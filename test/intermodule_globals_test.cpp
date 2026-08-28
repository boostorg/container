//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#include <boost/container/detail/intermodule_globals.hpp>
#include "lightweight_test.hpp"
#include <cstdio>
#include <cstring>

#if !defined(BOOST_NO_CXX11_HDR_THREAD)
#include <thread>
#include <vector>

#endif

using namespace boost::container;

namespace {

unsigned constructor_calls = 0;
unsigned destructor_calls  = 0;

//Used like a global variable: its own constructor and destructor are what
//the utility runs, no construct()/destroy() callbacks involved.
struct BOOST_SYMBOL_VISIBLE test_globals
{
   test_globals()
      : magic(0xB005), self(this)
   {  ++constructor_calls;  }

   ~test_globals()
   {  ++destructor_calls;  }

   int         magic;
   void       *self;
};

struct BOOST_SYMBOL_VISIBLE options_a {};

//An immortal instance (dlmalloc-style lifetime policy)
struct BOOST_SYMBOL_VISIBLE options_b
{
   static const bool destroy_at_exit = false;
};

#if !defined(BOOST_NO_ALIGNMENT)

//Over-aligned payload: its alignment exceeds what the platform allocator
//guarantees, so the backend must reserve extra space and place the object
//at a suitably aligned address inside the block.
//The alignment is requested on a MEMBER, not on the class: GCC up to 12
//cannot parse a GNU __attribute__ (which is what BOOST_SYMBOL_VISIBLE is on
//ELF) together with C++11 alignas in a class head, in either order - it
//reports "expected identifier before 'alignas'". Over-aligning a member
//raises the alignment of the whole class just the same, which is all this
//test needs, and it keeps the default visibility the ELF backend requires.
struct BOOST_SYMBOL_VISIBLE overaligned_globals
{
   overaligned_globals() : magic(0xA11) {}
   BOOST_ALIGNMENT(64) int magic;
   char pad[64];
};

struct BOOST_SYMBOL_VISIBLE options_overaligned {};

#endif   //!defined(BOOST_NO_ALIGNMENT)

//Runs during static destruction, in unordered position relative to the
//intermodule registrars: whether it observes the original object or a
//phoenix-reconstructed one, the access must be valid and consistent.
struct static_destruction_prober
{
   ~static_destruction_prober()
   {
      test_globals &g = dtl::intermodule_globals<test_globals, options_a>();
      if(g.magic != 42 && g.magic != 0xB005){
         std::printf("ERROR: inconsistent globals during static destruction\n");
      }
      test_globals &gb = dtl::intermodule_globals<test_globals, options_b>();
      if(gb.magic != 0xB005){
         std::printf("ERROR: immortal globals lost during static destruction\n");
      }
   }
};

static_destruction_prober prober;

}  //namespace

//Option detection: members present in the Options type win, absent ones
//fall back to intermodule_options_defaults
BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_destroy_at_exit<options_a>::value == true));
BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_destroy_at_exit<options_b>::value == false));
BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_pin_constructing_module<options_a>::value == false));
BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_pin_constructing_module<options_b>::value == false));

namespace {

//A type that sets every option explicitly, to check the non-default branch
//of each detector
struct BOOST_SYMBOL_VISIBLE options_all_set
{
   static const bool destroy_at_exit = false;
   static const bool pin_constructing_module = true;
};

}  //namespace

//void is the empty option set: every option takes its default
BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_destroy_at_exit<void>::value == true));
BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_pin_constructing_module<void>::value == false));

BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_destroy_at_exit<options_all_set>::value == false));
BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_pin_constructing_module<options_all_set>::value == true));

//Options defaults to void, so these two spellings are the same object - and
//a different one from any named Options over the same T
static void test_default_options()
{
   test_globals &v1 = dtl::intermodule_globals<test_globals>();
   test_globals &v2 = dtl::intermodule_globals<test_globals, void>();
   BOOST_TEST(&v1 == &v2);
   BOOST_TEST(v1.magic == 0xB005);
   BOOST_TEST(v1.self == (void *)&v1);
   BOOST_TEST((&v1 != &dtl::intermodule_globals<test_globals, options_a>()));

   //Still one object on a later call, and mutations survive
   v1.magic = 0x0DDu;
   BOOST_TEST(dtl::intermodule_globals<test_globals>().magic == 0x0DDu);
   v1.magic = 0xB005;
}

static void test_basic()
{
   //T's constructor must have run BEFORE main() started (this function runs
   //before any in-main get() call). Two instances exist by now: options_a
   //and options_b, both touched by the static destruction prober above.
   BOOST_TEST(constructor_calls >= 1u);
   BOOST_TEST(destructor_calls == 0u);
   const unsigned calls_before = constructor_calls;

   test_globals &g1 = dtl::intermodule_globals<test_globals, options_a>();
   BOOST_TEST(g1.magic == 0xB005);
   //The constructor ran on the very storage we are handed
   BOOST_TEST(g1.self == (void *)&g1);

   //Second call returns the very same object and does not re-construct
   test_globals &g2 = dtl::intermodule_globals<test_globals, options_a>();
   BOOST_TEST(&g1 == &g2);
   BOOST_TEST(constructor_calls == calls_before);

   //Different options rendezvous to a different instance
   test_globals &gb = dtl::intermodule_globals<test_globals, options_b>();
   BOOST_TEST(&gb != &g1);
   BOOST_TEST(gb.magic == 0xB005);

   //Mutations are visible through later calls (same object)
   g1.magic = 42;
   test_globals &g3 = dtl::intermodule_globals<test_globals, options_a>();
   BOOST_TEST(g3.magic == 42);
   BOOST_TEST(gb.magic == 0xB005);
}

#if !defined(BOOST_NO_CXX11_HDR_THREAD)

namespace {

struct BOOST_SYMBOL_VISIBLE options_mt {};

}  //namespace

static void test_threaded_access()
{
   const unsigned num_threads = 8;
   const unsigned calls_before = constructor_calls;
   std::vector<std::thread> threads;
   std::vector<test_globals *> results(num_threads, (test_globals *)0);
   for(unsigned i = 0; i != num_threads; ++i){
      threads.push_back(std::thread([i, &results]{
         test_globals &g = dtl::intermodule_globals<test_globals, options_mt>();
         results[i] = &g;
      }));
   }
   for(unsigned i = 0; i != num_threads; ++i){
      threads[i].join();
   }
   //All threads saw the same, once-constructed object
   for(unsigned i = 0; i != num_threads; ++i){
      BOOST_TEST(results[i] != 0);
      BOOST_TEST(results[i] == results[0]);
   }
   //options_mt was already built by its registrar before main()
   BOOST_TEST(constructor_calls == calls_before);
   BOOST_TEST(results[0]->magic == 0xB005);
}

#endif   //!defined(BOOST_NO_CXX11_HDR_THREAD)

#if defined(BOOST_CONTAINER_INTERMODULE_BACKEND_WINAPI)

namespace {

//A second payload type, so the key can be checked to depend on T as well as
//on Options
struct BOOST_SYMBOL_VISIBLE other_globals
{
   other_globals() : magic(0xC0DE) {}
   int magic;
};

}  //namespace

static void test_rendezvous_key()
{
   //The key is the signature of a template instantiated on the T/Options
   //pair, so it is non-empty and stable...
   //(the extra parentheses keep the commas of the template argument lists
   //from splitting BOOST_TEST's argument)
   const char *const k = dtl::intermodule_rendezvous_key<test_globals, options_a>();
   BOOST_TEST(k != 0 && k[0] != 0);
   BOOST_TEST((0 == std::strcmp
      (k, dtl::intermodule_rendezvous_key<test_globals, options_a>())));

   //...and it tells apart a different Options over the same T...
   BOOST_TEST((0 != std::strcmp
      (k, dtl::intermodule_rendezvous_key<test_globals, options_b>())));
   //...as well as the same Options over a different T
   BOOST_TEST((0 != std::strcmp
      (k, dtl::intermodule_rendezvous_key<other_globals, options_a>())));

   //Which is what keeps the two pairs on two separate records
   other_globals &g = dtl::intermodule_globals<other_globals, options_a>();
   BOOST_TEST(g.magic == 0xC0DE);
   BOOST_TEST((&g == &dtl::intermodule_globals<other_globals, options_a>()));
   BOOST_TEST(((void *)&g !=
      (void *)&dtl::intermodule_globals<test_globals, options_a>()));
}

#endif   //BOOST_CONTAINER_INTERMODULE_BACKEND_WINAPI

#if !defined(BOOST_NO_ALIGNMENT)

static void test_overaligned()
{
   overaligned_globals &g =
      dtl::intermodule_globals<overaligned_globals, options_overaligned>();
   BOOST_TEST(g.magic == 0xA11);
   //The object must sit at an address honouring its (over-)alignment
   BOOST_TEST(0 == (reinterpret_cast<std::size_t>(&g) % 64u));
   //Same instance on every call
   overaligned_globals &g2 =
      dtl::intermodule_globals<overaligned_globals, options_overaligned>();
   BOOST_TEST(&g == &g2);
}

#endif   //!defined(BOOST_NO_ALIGNMENT)

int main()
{
   test_basic();
   test_default_options();
   #if !defined(BOOST_NO_CXX11_HDR_THREAD)
   test_threaded_access();
   #endif
   #if defined(BOOST_CONTAINER_INTERMODULE_BACKEND_WINAPI)
   test_rendezvous_key();
   #endif
   #if !defined(BOOST_NO_ALIGNMENT)
   test_overaligned();
   #endif
   return boost::report_errors();
}
