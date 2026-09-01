//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//intermodule_globals<> when one global depends on another, which is the case
//that re-enters attach() while it is still running.
//
//Two halves:
//
//  cross-module - the executable and two shared libraries each inline the
//     same root -> mid -> leaf chain. Whichever module attaches first builds
//     the whole chain, nesting two attach() calls inside the first one; the
//     rest only attach. All three must end up on one object per type, and the
//     chain must have been constructed exactly three times in total.
//
//  same-module - a second chain, private to this executable and mortal, whose
//     leaf destructor asks for the root again. The leaf is constructed first,
//     being innermost, so it is destroyed last, when the root has already
//     detached. That re-attach nests inside detach(), which is the one
//     nesting this test can force deterministically: the order of the
//     constructor chain depends on when each registrar runs, and nothing
//     specifies that.
//
//A backend that held a single lock over its whole registry across a
//constructor would ask for that same lock on the nested attach and hang, so
//failure here shows up as a hung test rather than a failed check.
#include "intermodule_chain_types.hpp"
#include "../lightweight_test.hpp"
#include <cstdio>
#include <cstdlib>

namespace dtl = boost::container::dtl;

//Exported by intermodule_chain_lib_a.cpp / intermodule_chain_lib_b.cpp
BOOST_CONTAINER_TEST_CHAIN_LIB_DECL(lib_a)
BOOST_CONTAINER_TEST_CHAIN_LIB_DECL(lib_b)

//////////////////////////////////////////////////////////////////////////////
//    cross-module: one chain shared by the executable and both libraries
//////////////////////////////////////////////////////////////////////////////

//Every module must reach the same object for each link of the chain, even
//though two of the three attaches happened nested inside another one.
void test_chain_same_object_in_every_module()
{
   BOOST_TEST(&bc_test::get_chain_root() == lib_a_chain_root());
   BOOST_TEST(&bc_test::get_chain_root() == lib_b_chain_root());
   BOOST_TEST(&bc_test::get_chain_mid()  == lib_a_chain_mid());
   BOOST_TEST(&bc_test::get_chain_mid()  == lib_b_chain_mid());
   BOOST_TEST(&bc_test::get_chain_leaf() == lib_a_chain_leaf());
   BOOST_TEST(&bc_test::get_chain_leaf() == lib_b_chain_leaf());
}

//Each constructor ran on the object every module now reads, so no module got
//a private copy that merely looks right.
void test_chain_constructed_in_place()
{
   BOOST_TEST(bc_test::get_chain_root().self == &bc_test::get_chain_root());
   BOOST_TEST(bc_test::get_chain_mid().self  == &bc_test::get_chain_mid());
   BOOST_TEST(bc_test::get_chain_leaf().self == &bc_test::get_chain_leaf());
}

//The values prove the chain really composed: each level added its own bits to
//the level below, rather than reading a zeroed or half-built object.
void test_chain_values_composed()
{
   BOOST_TEST(bc_test::get_chain_leaf().v == 0x100u);
   BOOST_TEST(bc_test::get_chain_mid().v  == 0x300u);
   BOOST_TEST(bc_test::get_chain_root().v == 0x700u);

   //And every module reads the same numbers through its own instantiation
   BOOST_TEST(lib_a_chain_root_value() == 0x700u);
   BOOST_TEST(lib_b_chain_root_value() == 0x700u);
   BOOST_TEST(lib_a_chain_mid_value()  == 0x300u);
   BOOST_TEST(lib_b_chain_mid_value()  == 0x300u);
   BOOST_TEST(lib_a_chain_leaf_value() == 0x100u);
   BOOST_TEST(lib_b_chain_leaf_value() == 0x100u);
}

//Three objects, so three constructor calls in the whole process, whichever
//module the loader happened to send through attach() first.
void test_chain_constructed_once()
{
   const unsigned total = bc_test::chain_ctor_calls
                        + lib_a_chain_ctor_calls()
                        + lib_b_chain_ctor_calls();
   BOOST_TEST(total == 3u);
}

//////////////////////////////////////////////////////////////////////////////
//    cross-module: constructions really landing in different modules
//////////////////////////////////////////////////////////////////////////////
//
//owner_a exists only in library A and owner_b only in library B, so each
//library must build its own, and only one of them also builds the shared leaf.

//Each library reaches its own object, and they are different objects.
void test_fan_owners_are_per_library()
{
   BOOST_TEST(lib_a_owner() != 0);
   BOOST_TEST(lib_b_owner() != 0);
   BOOST_TEST(lib_a_owner() != lib_b_owner());
   //Both nested through to the one shared leaf
   BOOST_TEST(lib_a_fan_leaf() == lib_b_fan_leaf());
}

//The values prove each owner really read the shared leaf during construction,
//and for one of the two that leaf had been built by the other module.
void test_fan_values_composed()
{
   BOOST_TEST(lib_a_fan_leaf_value() == 0x1000u);
   BOOST_TEST(lib_b_fan_leaf_value() == 0x1000u);
   BOOST_TEST(lib_a_owner_value() == 0x10u + 0x1000u);
   BOOST_TEST(lib_b_owner_value() == 0x20u + 0x1000u);
}

//Constructions really happened in different
//modules. Each owner constructor is defined out of line in its own library,
//so its counter is that library's own. Each library built
//exactly its own owner and both of those constructors reached
//the same shared leaf, which only one of the two can have created.
void test_fan_constructions_are_distributed()
{
   BOOST_TEST(lib_a_owner_ctor_calls() == 1u);
   BOOST_TEST(lib_b_owner_ctor_calls() == 1u);
}

//////////////////////////////////////////////////////////////////////////////
//    same-module: a mortal chain whose teardown re-enters attach()
//////////////////////////////////////////////////////////////////////////////

namespace {

//Private to this translation unit, so the libraries never attach them and the
//destruction order below is this module's alone.
struct BOOST_SYMBOL_VISIBLE mortal_opts
{  static const bool destroy_at_exit = true;  };

int  g_depth        = 0;   //how deep we are inside these constructors
int  g_nested_ctor  = 0;   //a constructor entered while another was running
bool g_teardown_ran = false;

struct BOOST_SYMBOL_VISIBLE m_root;

struct BOOST_SYMBOL_VISIBLE m_leaf
{
   m_leaf() : v(0x11u)
   {  if(g_depth) ++g_nested_ctor;  }
   ~m_leaf();        //asks for m_root: a nested attach from inside detach()
   unsigned v;
};

struct BOOST_SYMBOL_VISIBLE m_mid
{
   m_mid();
   unsigned v;
};

struct BOOST_SYMBOL_VISIBLE m_root
{
   m_root();
   unsigned v;
};

m_leaf &get_m_leaf()
{  return dtl::intermodule_globals<m_leaf, mortal_opts>();  }
m_mid  &get_m_mid()
{  return dtl::intermodule_globals<m_mid,  mortal_opts>();  }
m_root &get_m_root()
{  return dtl::intermodule_globals<m_root, mortal_opts>();  }

m_mid::m_mid() : v(0)
{
   if(g_depth) ++g_nested_ctor;
   ++g_depth;
   v = 0x22u + get_m_leaf().v;
   --g_depth;
}

m_root::m_root() : v(0)
{
   if(g_depth) ++g_nested_ctor;
   ++g_depth;
   v = 0x44u + get_m_mid().v;
   --g_depth;
}

//Runs after main, so BOOST_TEST can no longer report anything: a wrong result
//here has to fail the process outright.
//
//Whether this re-attach nests depends on which registrar the implementation
//destroys last, and nothing specifies that. When the root is already gone
//this rebuilds the whole chain from inside detach(), which is the case worth
//having; when it is still alive the call is just a lookup. Either way the
//value has to come back right, and either way the process must not hang.
m_leaf::~m_leaf()
{
   g_teardown_ran = true;
   const unsigned root_v = get_m_root().v;   //may re-attach, nested in detach
   if(root_v != 0x77u){
      std::printf("intermodule_chain_test: teardown re-attach gave %x, "
                  "expected 77\n", root_v);
      std::abort();
   }
}

}  //namespace

//Build the mortal chain and check it composed. Its teardown runs after main.
//
//Nesting is not asserted here. Each type's registrar attaches it during
//static initialization, in an order nothing specifies
void test_same_module_chain()
{
   BOOST_TEST(get_m_root().v == 0x77u);
   BOOST_TEST(get_m_mid().v  == 0x33u);
   BOOST_TEST(get_m_leaf().v == 0x11u);
   BOOST_TEST(!g_teardown_ran);
   std::printf("intermodule_chain_test: nested constructions on this run: %d\n",
               g_nested_ctor);
}

//The teardown below runs after main and can hang if a backend serializes its
//whole registry across a constructor. Flush first, or a hung run prints
//nothing at all and looks like a failure to even start.
void flush_before_teardown()
{
   std::printf("intermodule_chain_test: checks done, entering teardown\n");
   std::fflush(stdout);
}

int main()
{
   test_chain_same_object_in_every_module();
   test_chain_constructed_in_place();
   test_chain_values_composed();
   test_chain_constructed_once();
   test_fan_owners_are_per_library();
   test_fan_values_composed();
   test_fan_constructions_are_distributed();
   test_same_module_chain();
   const int errors = boost::report_errors();
   flush_before_teardown();
   return errors;
}
