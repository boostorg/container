//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//intermodule_globals<> across module boundaries: this executable and two
//shared libraries each inline their own copy of the utility, and each one
//instantiates it over the same test types, so every T/Options pair must
//resolve to a single process-wide object.
//
//Only the utility's own test types are used here. The library singletons
//built on top of it are tested where they belong: the dlmalloc heap in
//dlmalloc_multi_tu, the polymorphic resources in pmr_multi_tu.
#include "intermodule_test_types.hpp"
#include "../lightweight_test.hpp"

namespace dtl = boost::container::dtl;

//Exported by intermodule_lib_a.cpp / intermodule_lib_b.cpp
BOOST_CONTAINER_TEST_INTERMODULE_LIB_DECL(lib_a)
BOOST_CONTAINER_TEST_INTERMODULE_LIB_DECL(lib_b)

//This module's own accessors, so the executable reads through its own
//instantiation exactly as the libraries read through theirs.
namespace {

bc_test::shared_globals &exe_alpha()
{  return dtl::intermodule_globals<bc_test::shared_globals, bc_test::options_alpha>();  }

bc_test::shared_globals &exe_beta()
{  return dtl::intermodule_globals<bc_test::shared_globals, bc_test::options_beta>();  }

bc_test::shared_globals &exe_immortal()
{  return dtl::intermodule_globals<bc_test::shared_globals, bc_test::options_immortal>();  }

}  //namespace

//Every module must reach the same object for a given T/Options pair.
void test_same_object_in_every_module()
{
   BOOST_TEST(&exe_alpha()    == lib_a_alpha()    && &exe_alpha()    == lib_b_alpha());
   BOOST_TEST(&exe_beta()     == lib_a_beta()     && &exe_beta()     == lib_b_beta());
   BOOST_TEST(&exe_immortal() == lib_a_immortal() && &exe_immortal() == lib_b_immortal());
}

//Identity follows the T/Options pair, so the same T under three different
//keys must give three different objects.
void test_options_select_distinct_objects()
{
   BOOST_TEST(&exe_alpha() != &exe_beta());
   BOOST_TEST(&exe_alpha() != &exe_immortal());
   BOOST_TEST(&exe_beta()  != &exe_immortal());
}

//Each object was fully constructed before any module could observe it, and
//constructed in place rather than copied in.
void test_constructed_in_place()
{
   BOOST_TEST(exe_alpha().magic    == int(bc_test::shared_globals::magic_value));
   BOOST_TEST(exe_beta().magic     == int(bc_test::shared_globals::magic_value));
   BOOST_TEST(exe_immortal().magic == int(bc_test::shared_globals::magic_value));

   //The constructor recorded the address it ran on; a module holding a copy
   //would see self pointing somewhere else.
   BOOST_TEST(exe_alpha().self    == static_cast<void *>(&exe_alpha()));
   BOOST_TEST(exe_beta().self     == static_cast<void *>(&exe_beta()));
   BOOST_TEST(exe_immortal().self == static_cast<void *>(&exe_immortal()));
}

//Three objects exist, so exactly three constructor calls happened in the
//whole process however they are distributed over the modules. More would mean
//a module built its own copy instead of attaching to the shared one.
void test_constructed_exactly_once_each()
{
   const unsigned total = bc_test::module_ctor_calls
                        + lib_a_ctor_calls() + lib_b_ctor_calls();
   BOOST_TEST_EQ(total, 3u);

   //Nothing has been destroyed yet: the two destructible objects go at static
   //destruction time and the immortal one never does.
   const unsigned destroyed = bc_test::module_dtor_calls
                            + lib_a_dtor_calls() + lib_b_dtor_calls();
   BOOST_TEST_EQ(destroyed, 0u);
}

//The object is shared state, not a per-module copy: a write in one module is
//read back through another module's own instantiation.
void test_writes_cross_module()
{
   exe_alpha().value = 4242;
   BOOST_TEST_EQ(lib_a_alpha_value(), 4242);
   BOOST_TEST_EQ(lib_b_alpha_value(), 4242);

   lib_a_set_alpha_value(-7);
   BOOST_TEST_EQ(exe_alpha().value, -7);
   BOOST_TEST_EQ(lib_b_alpha_value(), -7);

   lib_b_set_alpha_value(0);
   BOOST_TEST_EQ(exe_alpha().value, 0);
   BOOST_TEST_EQ(lib_a_alpha_value(), 0);

   //The other keys are untouched by all of that
   BOOST_TEST_EQ(exe_beta().value, 0);
   BOOST_TEST_EQ(exe_immortal().value, 0);
}

#if !defined(BOOST_NO_ALIGNMENT)

//An over-aligned object lands on an aligned address, and every module agrees
//which address that is.
void test_overaligned_object()
{
   bc_test::overaligned_globals &g =
      dtl::intermodule_globals<bc_test::overaligned_globals, bc_test::options_overaligned>();
   BOOST_TEST(static_cast<void *>(&g) == lib_a_overaligned());
   BOOST_TEST(static_cast<void *>(&g) == lib_b_overaligned());
   BOOST_TEST(g.magic == int(bc_test::overaligned_globals::magic_value));

   const std::size_t align = std::size_t(bc_test::overaligned_globals::alignment);
   BOOST_TEST((reinterpret_cast<std::size_t>(&g) % align) == 0u);
}

#endif   //!defined(BOOST_NO_ALIGNMENT)

int main()
{
   test_same_object_in_every_module();
   test_options_select_distinct_objects();
   test_constructed_in_place();
   test_constructed_exactly_once_each();
   test_writes_cross_module();
   #if !defined(BOOST_NO_ALIGNMENT)
   test_overaligned_object();
   #endif
   return boost::report_errors();
}
