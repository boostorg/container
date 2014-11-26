//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright 2007, 2008 Steven Watanabe, Joseph Gauterin, Niels Dekker
// (C) Copyright Ion Gaztanaga 2005-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_SWAP_HPP
#define BOOST_CONTAINER_DETAIL_SWAP_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

//Based on Boost.Core's swap, but defaulting to a move-enabled swap.
//
// Note: the implementation of this utility contains various workarounds:
// - swap_impl is put outside the boost namespace, to avoid infinite
// recursion (causing stack overflow) when swapping objects of a primitive
// type.
// - swap_impl has a using-directive, rather than a using-declaration,
// because some compilers (including MSVC 7.1, Borland 5.9.3, and
// Intel 8.1) don't do argument-dependent lookup when it has a
// using-declaration instead.
// - boost::swap has two template arguments, instead of one, to
// avoid ambiguity when swapping objects of a Boost type that does
// not have its own boost::swap overload.

#include <cstddef> //for std::size_t
#include <boost/move/utility_core.hpp> //for boost::move

namespace boost_container_swap_move {
   template<class T>
   void swap(T& left, T& right)
   {
      T tmp(::boost::move(left));
      left = ::boost::move(right);
      right = ::boost::move(tmp);
   }
}  //namespace boost_container_swap_move {

namespace boost_container_swap
{
   template<class T>
   void adl_swap_impl(T& left, T& right)
   {
      //use boost_container_swap_move::swap if argument dependent lookup fails
      using namespace boost_container_swap_move;
      swap(left,right);
   }

   template<class T, std::size_t N>
   void adl_swap_impl(T (& left)[N], T (& right)[N])
   {
      for (std::size_t i = 0; i < N; ++i){
         ::boost_container_swap::adl_swap_impl(left[i], right[i]);
      }
   }
}  //namespace boost_container_swap {

namespace boost{
namespace container{

template<class T1, class T2>
void adl_swap(T1& left, T2& right)
{
   ::boost_container_swap::adl_swap_impl(left, right);
}

}  //namespace container{
}  //namespace boost{

#endif   //#ifndef BOOST_CONTAINER_DETAIL_SWAP_HPP
