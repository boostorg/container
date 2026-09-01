//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_TEST_INTERMODULE_CHAIN_TYPES_HPP
#define BOOST_CONTAINER_TEST_INTERMODULE_CHAIN_TYPES_HPP

//Types for the dependency-chain test: three intermodule globals where each
//constructor needs the next one, root -> mid -> leaf. Attaching root therefore
//runs attach() for mid from inside root's constructor, and attach() for leaf
//from inside mid's, so the utility must tolerate re-entering itself for a
//different T/Options pair.
//
//The executable and both shared libraries include this header, so all three
//inline their own copy of the chain and still have to end up with a single
//object per type, constructed exactly once in the whole process.

#include <boost/config.hpp>
#include <boost/container/detail/intermodule_globals.hpp>

namespace bc_test {

//How many chain constructors ran. Only the total over the three modules is
//meaningful, never the individual numbers: the constructors below are inline
//in this header, so the linker keeps one copy of each and every module's call
//lands in whichever module's counter that copy was bound to.
static unsigned chain_ctor_calls = 0;

//Immortal
struct BOOST_SYMBOL_VISIBLE chain_opts
{  static const bool destroy_at_exit = false;  };

//BOOST_SYMBOL_VISIBLE for ELF
struct BOOST_SYMBOL_VISIBLE chain_leaf
{
   chain_leaf();
   unsigned  v;
   void     *self;   //the address the constructor really ran on
};

struct BOOST_SYMBOL_VISIBLE chain_mid
{
   chain_mid();      //needs chain_leaf
   unsigned  v;
   void     *self;
};

struct BOOST_SYMBOL_VISIBLE chain_root
{
   chain_root();     //needs chain_mid, which needs chain_leaf
   unsigned  v;
   void     *self;
};

//Every module reaches the chain through these, so no module can drift onto a
//different instantiation than the others.
inline chain_leaf &get_chain_leaf()
{  return ::boost::container::dtl::intermodule_globals<chain_leaf, chain_opts>();  }

inline chain_mid &get_chain_mid()
{  return ::boost::container::dtl::intermodule_globals<chain_mid, chain_opts>();  }

inline chain_root &get_chain_root()
{  return ::boost::container::dtl::intermodule_globals<chain_root, chain_opts>();  }

//Each level adds its own bit pattern to the one below, so the final values
//prove the chain really composed and did not, say, read a zeroed object.
//leaf = 0x100, mid = 0x300, root = 0x700.
inline chain_leaf::chain_leaf()
   : v(0x100u), self(this)
{  ++chain_ctor_calls;  }

inline chain_mid::chain_mid()
   : v(0x200u + get_chain_leaf().v), self(this)
{  ++chain_ctor_calls;  }

inline chain_root::chain_root()
   : v(0x400u + get_chain_mid().v), self(this)
{  ++chain_ctor_calls;  }

//////////////////////////////////////////////////////////////////////////////
//
//    A fan so that different modules construct different objects
//
//////////////////////////////////////////////////////////////////////////////
//owner_a is instantiated only by library A and owner_b only by library B,
//while both constructors need the same fan_leaf.
//So each library certainly builds its own owner, and exactly one of them also
//builds the leaf; for the other, the nested attach inside its constructor finds an
//object that a different module created.

struct BOOST_SYMBOL_VISIBLE fan_leaf
{
   fan_leaf();
   unsigned  v;
   void     *self;
};

//Same shape, two distinct types, so each library gets an object of its own.
//Their constructors are deliberately NOT defined here: each library defines
//its own out of line, in its own translation unit, which is what makes the
//per-library counter below trustworthy. See the note on counting.
struct BOOST_SYMBOL_VISIBLE owner_a
{
   owner_a();        //defined by library A only
   unsigned  v;
   void     *self;
};

struct BOOST_SYMBOL_VISIBLE owner_b
{
   owner_b();        //defined by library B only
   unsigned  v;
   void     *self;
};

inline fan_leaf &get_fan_leaf()
{  return ::boost::container::dtl::intermodule_globals<fan_leaf, chain_opts>();  }

inline fan_leaf::fan_leaf()
   : v(0x1000u), self(this)
{}

}  //namespace bc_test {

//Both shared libraries export their accessors with this macro, so each one
//reads through its own instantiation.
//
//owner_type is this library's own fan global: library A passes owner_a and
//library B passes owner_b, so neither type is ever instantiated anywhere
//else, and the executable reaches them only through these accessors.
//
//The constructor is defined here, inside the library's own translation unit,
//not in the header. An out-of-line constructor has a
//single definition, in this module, so the counter it bumps
//really is this module's.
#define BOOST_CONTAINER_TEST_CHAIN_LIB(prefix, owner_type, own_bits)             \
                                                                                 \
static unsigned prefix##_owner_ctors = 0;                                        \
                                                                                 \
::bc_test::owner_type::owner_type()                                              \
   : v(own_bits + ::bc_test::get_fan_leaf().v), self(this)                       \
{  ++prefix##_owner_ctors;  }                                                    \
                                                                                 \
BOOST_SYMBOL_EXPORT unsigned prefix##_owner_ctor_calls()                         \
{  return prefix##_owner_ctors;  }                                               \
                                                                                 \
BOOST_SYMBOL_EXPORT void *prefix##_owner()                                       \
{  return &::boost::container::dtl::intermodule_globals                          \
            < ::bc_test::owner_type, ::bc_test::chain_opts>();  }                \
                                                                                 \
BOOST_SYMBOL_EXPORT unsigned prefix##_owner_value()                              \
{  return ::boost::container::dtl::intermodule_globals                           \
            < ::bc_test::owner_type, ::bc_test::chain_opts>().v;  }              \
                                                                                 \
BOOST_SYMBOL_EXPORT void *prefix##_fan_leaf()                                    \
{  return &::bc_test::get_fan_leaf();  }                                         \
                                                                                 \
BOOST_SYMBOL_EXPORT unsigned prefix##_fan_leaf_value()                           \
{  return ::bc_test::get_fan_leaf().v;  }                                        \
                                                                                 \
BOOST_SYMBOL_EXPORT void *prefix##_chain_root()                                  \
{  return &::bc_test::get_chain_root();  }                                       \
                                                                                 \
BOOST_SYMBOL_EXPORT void *prefix##_chain_mid()                                   \
{  return &::bc_test::get_chain_mid();  }                                        \
                                                                                 \
BOOST_SYMBOL_EXPORT void *prefix##_chain_leaf()                                  \
{  return &::bc_test::get_chain_leaf();  }                                       \
                                                                                 \
BOOST_SYMBOL_EXPORT unsigned prefix##_chain_root_value()                         \
{  return ::bc_test::get_chain_root().v;  }                                      \
                                                                                 \
BOOST_SYMBOL_EXPORT unsigned prefix##_chain_mid_value()                          \
{  return ::bc_test::get_chain_mid().v;  }                                       \
                                                                                 \
BOOST_SYMBOL_EXPORT unsigned prefix##_chain_leaf_value()                         \
{  return ::bc_test::get_chain_leaf().v;  }                                      \
                                                                                 \
BOOST_SYMBOL_EXPORT unsigned prefix##_chain_ctor_calls()                         \
{  return ::bc_test::chain_ctor_calls;  }

//Declarations of what the two libraries export, for the executable.
#define BOOST_CONTAINER_TEST_CHAIN_LIB_DECL(prefix)                              \
   void    *prefix##_owner();                                                    \
   unsigned prefix##_owner_value();                                              \
   void    *prefix##_fan_leaf();                                                 \
   unsigned prefix##_fan_leaf_value();                                           \
   unsigned prefix##_owner_ctor_calls();                                         \
   void    *prefix##_chain_root();                                               \
   void    *prefix##_chain_mid();                                                \
   void    *prefix##_chain_leaf();                                               \
   unsigned prefix##_chain_root_value();                                         \
   unsigned prefix##_chain_mid_value();                                          \
   unsigned prefix##_chain_leaf_value();                                         \
   unsigned prefix##_chain_ctor_calls();

#endif   //BOOST_CONTAINER_TEST_INTERMODULE_CHAIN_TYPES_HPP
