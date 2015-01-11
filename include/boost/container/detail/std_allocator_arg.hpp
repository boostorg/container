#ifndef BOOST_CONTAINER_DETAIL_ALLOCATOR_ARG_HPP
#define BOOST_CONTAINER_DETAIL_ALLOCATOR_ARG_HPP
///////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
///////////////////////////////////////////////////////////////////////////////

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/std_fwd.hpp>

namespace boost { namespace container {

template<int Dummy = 0>
struct alloc_arg
{
   static const std::allocator_arg_t &get()  {  return *palloc_arg;  }
   static std::allocator_arg_t *palloc_arg;
};

template<int Dummy>
std::allocator_arg_t *alloc_arg<Dummy>::palloc_arg;

}} //namespace boost { namespace container {

#endif   //BOOST_CONTAINER_DETAIL_ALLOCATOR_ARG_HPP
