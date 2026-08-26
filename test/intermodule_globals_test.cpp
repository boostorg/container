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
#if defined(BOOST_CONTAINER_INTERMODULE_BACKEND_WINAPI) && !defined(BOOST_NO_TYPEID)
#include <typeinfo>
#endif

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

struct BOOST_SYMBOL_VISIBLE options_a
{
   static const char *name() { return "testa"; }
};

//An immortal instance (dlmalloc-style lifetime policy)
struct BOOST_SYMBOL_VISIBLE options_b
{
   static const char *name() { return "testb"; }
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

struct BOOST_SYMBOL_VISIBLE options_overaligned
{
   static const char *name() { return "testovr"; }
};

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
   static const char *name() { return "testall"; }
   static const bool destroy_at_exit = false;
   static const bool pin_constructing_module = true;
};

}  //namespace

BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_destroy_at_exit<options_all_set>::value == false));
BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_pin_constructing_module<options_all_set>::value == true));

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

struct BOOST_SYMBOL_VISIBLE options_mt
{
   static const char *name() { return "testmt"; }
};

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

#if defined(BOOST_CONTAINER_INTERMODULE_BACKEND_WINAPI) && !defined(BOOST_NO_TYPEID)

namespace {

//No name(): the registry keys this instance on typeid(T).name()
struct BOOST_SYMBOL_VISIBLE unnamed_globals
{
   unnamed_globals() : magic(0xC0DE) {}
   int magic;
};

struct BOOST_SYMBOL_VISIBLE options_no_name
{
   static const bool destroy_at_exit = true;
};

}  //namespace

BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_has_name<options_no_name>::value == false));
BOOST_CONTAINER_STATIC_ASSERT
   ((dtl::intermodule_opt_has_name<options_a>::value == true));

static void test_typeid_key()
{
   unnamed_globals &g =
      dtl::intermodule_globals<unnamed_globals, options_no_name>();
   BOOST_TEST(g.magic == 0xC0DE);
   //Same instance on every call, i.e. the typeid key is stable
   unnamed_globals &g2 =
      dtl::intermodule_globals<unnamed_globals, options_no_name>();
   BOOST_TEST(&g == &g2);
   //And it really is the type's name, stored whole
   BOOST_TEST(0 == std::strcmp
      ( (dtl::intermodule_rendezvous_key<unnamed_globals, options_no_name>())
      , typeid(unnamed_globals).name()));
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
   #if !defined(BOOST_NO_CXX11_HDR_THREAD)
   test_threaded_access();
   #endif
   #if defined(BOOST_CONTAINER_INTERMODULE_BACKEND_WINAPI) && !defined(BOOST_NO_TYPEID)
   test_typeid_key();
   #endif
   #if !defined(BOOST_NO_ALIGNMENT)
   test_overaligned();
   #endif
   return boost::report_errors();
}
