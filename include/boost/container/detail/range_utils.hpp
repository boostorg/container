//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2024-2024. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_RANGE_UTILS_HPP
#define BOOST_CONTAINER_DETAIL_RANGE_UTILS_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/workaround.hpp>
#include <boost/container/detail/std_fwd.hpp>   //forward declares ::std::from_range_t

//! \file
//!   Defines boost::container::from_range_t and the boost::container::from_range
//!   tag, used to disambiguate the range-based members of the containers.
//!
//!   Following the same technique as boost::container::piecewise_construct_t and
//!   boost::container::allocator_arg_t, from_range_t is a reference to the
//!   standard ::std::from_range_t (only forward declared in <std_fwd.hpp>), so
//!   the tag is the very same type as the standard one and interoperates with
//!   ::std::from_range, yet the heavyweight <ranges> header is not required.

namespace boost {
namespace container {

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   template <int Dummy = 0>
   struct std_from_range_holder
   {
      static ::std::from_range_t *dummy;
   };

   template <int Dummy>                                            //Silence null-reference compiler warnings
   ::std::from_range_t *std_from_range_holder<Dummy>::dummy = reinterpret_cast< ::std::from_range_t * >(0x1234);

typedef const ::std::from_range_t & from_range_t;

#else

//! The from_range_t struct is an empty structure type used as a unique type to
//! disambiguate the constructors and member functions that build or replace the
//! contents of a container from a container-compatible range.
typedef unspecified from_range_t;

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//! A instance of type
//! from_range_t
static from_range_t from_range = BOOST_CONTAINER_DOC1ST(unspecified, *std_from_range_holder<>::dummy);

///@cond

namespace dtl {

struct from_range_construct_use
{
   //Avoid warnings of unused "from_range"
   from_range_construct_use()
   {  (void)&::boost::container::from_range;   }
};

}  //namespace dtl {

///@endcond

}  //namespace container {
}  //namespace boost {

#endif   //BOOST_CONTAINER_DETAIL_RANGE_UTILS_HPP
