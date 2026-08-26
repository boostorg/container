//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_TEST_INTERMODULE_TEST_TYPES_HPP
#define BOOST_CONTAINER_TEST_INTERMODULE_TEST_TYPES_HPP

//Types shared by the three modules of the intermodule test (the executable
//and both shared libraries). All of them include this header, so all of them
//instantiate the very same intermodule_globals<T, Options> and must end up
//sharing one object per T/Options pair.
//
//This test deliberately uses none of the library's own singletons: the
//dlmalloc heap is covered by dlmalloc_multi_tu and the polymorphic resources
//by pmr_multi_tu. What is under test here is intermodule_globals<> itself.

#include <boost/config.hpp>
#include <boost/container/detail/intermodule_globals.hpp>
#include <cstddef>

namespace bc_test {

//How many times *this module's* copy of the constructor and destructor ran.
//Each module keeps its own count (internal linkage), and the counts summed
//over every module say how many objects were really created: whichever
//module attaches first constructs, the rest only attach.
static unsigned module_ctor_calls = 0;
static unsigned module_dtor_calls = 0;

//BOOST_SYMBOL_VISIBLE is not decoration: on ELF the visibility of a template
//instantiation is the minimum over the template and every one of its
//arguments, so a hidden T or Options would make the instantiation hidden and
//hand each module a private copy of the object.
struct BOOST_SYMBOL_VISIBLE shared_globals
{
   //Constructed on zero-initialized storage, so every member starts at 0
   //whether or not this constructor assigns it.
   shared_globals()
      : magic(magic_value), value(0), self(this)
   {  ++module_ctor_calls;  }

   ~shared_globals()
   {  ++module_dtor_calls;  }

   //An enum, not a static const member: reading it can never turn into an
   //ODR-use that would want an out-of-line definition.
   enum { magic_value = 0x5EED };

   int    magic;
   int    value;
   //Records the address the constructor actually ran on, so a module that
   //somehow got a copy rather than the shared object is detectable.
   void  *self;
};

//Three keys over the same T: the object identity follows the T/Options pair,
//not T alone, so these must be three distinct objects.
struct BOOST_SYMBOL_VISIBLE options_alpha
{  static const char *name() { return "bctim_alpha"; }  };

struct BOOST_SYMBOL_VISIBLE options_beta
{  static const char *name() { return "bctim_beta"; }  };

//An intentionally immortal instance: its destructor is never run, the OS
//reclaims it at process exit.
struct BOOST_SYMBOL_VISIBLE options_immortal
{
   static const char *name() { return "bctim_imm"; }
   static const bool destroy_at_exit = false;
};

#if !defined(BOOST_NO_ALIGNMENT)

//Alignment beyond what the platform allocator guarantees, so the backend has
//to reserve extra bytes and place the object at an aligned address inside the
//block - and every module must agree on where that is.
struct BOOST_SYMBOL_VISIBLE overaligned_globals
{
   overaligned_globals() : magic(magic_value) {}

   enum { magic_value = 0x0A11, alignment = 64 };

   BOOST_ALIGNMENT(64) int magic;
   char pad[64];
};

struct BOOST_SYMBOL_VISIBLE options_overaligned
{  static const char *name() { return "bctim_ovr"; }  };

#endif   //!defined(BOOST_NO_ALIGNMENT)

}  //namespace bc_test {

//Each shared library defines its exported accessors with this macro, so all
//of them go through the identical instantiations with no chance of the
//modules drifting apart.  Every accessor reads or writes through the calling
//module's *own* instantiation, which is what makes the checks meaningful.
#define BOOST_CONTAINER_TEST_INTERMODULE_LIB(prefix)                             \
                                                                                 \
BOOST_SYMBOL_EXPORT ::bc_test::shared_globals *prefix##_alpha()                  \
{  return &::boost::container::dtl::intermodule_globals                          \
            < ::bc_test::shared_globals, ::bc_test::options_alpha>();  }          \
                                                                                 \
BOOST_SYMBOL_EXPORT ::bc_test::shared_globals *prefix##_beta()                   \
{  return &::boost::container::dtl::intermodule_globals                          \
            < ::bc_test::shared_globals, ::bc_test::options_beta>();  }           \
                                                                                 \
BOOST_SYMBOL_EXPORT ::bc_test::shared_globals *prefix##_immortal()               \
{  return &::boost::container::dtl::intermodule_globals                          \
            < ::bc_test::shared_globals, ::bc_test::options_immortal>();  }       \
                                                                                 \
BOOST_SYMBOL_EXPORT int prefix##_alpha_value()                                   \
{  return ::boost::container::dtl::intermodule_globals                           \
            < ::bc_test::shared_globals, ::bc_test::options_alpha>().value;  }    \
                                                                                 \
BOOST_SYMBOL_EXPORT void prefix##_set_alpha_value(int v)                         \
{  ::boost::container::dtl::intermodule_globals                                  \
      < ::bc_test::shared_globals, ::bc_test::options_alpha>().value = v;  }      \
                                                                                 \
BOOST_SYMBOL_EXPORT unsigned prefix##_ctor_calls()                               \
{  return ::bc_test::module_ctor_calls;  }                                        \
                                                                                 \
BOOST_SYMBOL_EXPORT unsigned prefix##_dtor_calls()                               \
{  return ::bc_test::module_dtor_calls;  }                                        \
                                                                                 \
BOOST_CONTAINER_TEST_INTERMODULE_LIB_OVERALIGNED(prefix)

#if !defined(BOOST_NO_ALIGNMENT)
#define BOOST_CONTAINER_TEST_INTERMODULE_LIB_OVERALIGNED(prefix)                 \
BOOST_SYMBOL_EXPORT void *prefix##_overaligned()                                 \
{  return &::boost::container::dtl::intermodule_globals                          \
            < ::bc_test::overaligned_globals, ::bc_test::options_overaligned>();  }
#else
#define BOOST_CONTAINER_TEST_INTERMODULE_LIB_OVERALIGNED(prefix)
#endif

//Declarations of what the two libraries export, for the executable.
#define BOOST_CONTAINER_TEST_INTERMODULE_LIB_DECL(prefix)                        \
   ::bc_test::shared_globals *prefix##_alpha();                                  \
   ::bc_test::shared_globals *prefix##_beta();                                   \
   ::bc_test::shared_globals *prefix##_immortal();                               \
   int      prefix##_alpha_value();                                              \
   void     prefix##_set_alpha_value(int v);                                     \
   unsigned prefix##_ctor_calls();                                               \
   unsigned prefix##_dtor_calls();                                               \
   BOOST_CONTAINER_TEST_INTERMODULE_LIB_DECL_OVERALIGNED(prefix)

#if !defined(BOOST_NO_ALIGNMENT)
#define BOOST_CONTAINER_TEST_INTERMODULE_LIB_DECL_OVERALIGNED(prefix)            \
   void *prefix##_overaligned();
#else
#define BOOST_CONTAINER_TEST_INTERMODULE_LIB_DECL_OVERALIGNED(prefix)
#endif

#endif   //BOOST_CONTAINER_TEST_INTERMODULE_TEST_TYPES_HPP
