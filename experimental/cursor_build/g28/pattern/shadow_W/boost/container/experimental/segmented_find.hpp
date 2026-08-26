//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_HPP
#define BOOST_CONTAINER_G28_SHADOW_FIND 'W'

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/iterators.hpp>
#include <boost/container/experimental/segmented_walk.hpp>

namespace boost {
namespace container {

template <class InpIter, class Sent, class T>
InpIter segmented_find(InpIter first, Sent last, const T& BOOST_RESTRICT value);

namespace detail_algo {

template <class InpIter, class Sent, class T, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InpIter>::value, InpIter>::type
segmented_find_dispatch(InpIter first, Sent last, const T& BOOST_RESTRICT value, Tag, Cat)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first)
      if(*first == value)
         break;
   return first;
}

template <class T>
struct find_walk_fn
{
   const T* pvalue;

   BOOST_CONTAINER_FORCEINLINE explicit find_walk_fn(const T& v) : pvalue(&v) {}

   template <class It, class Sent>
   BOOST_CONTAINER_FORCEINLINE It operator()(It first, Sent last) const
   {
      return (segmented_find_dispatch)(first, last, *pvalue,
         non_segmented_iterator_tag(), typename iterator_traits<It>::iterator_category());
   }
};

template <class SegIter, class T, class Cat>
SegIter segmented_find_dispatch
   (SegIter first, SegIter last, const T& BOOST_RESTRICT value, segmented_iterator_tag, Cat)
{
   find_walk_fn<T> f(value);
   SegIter out;
   if((segmented_walk_until)(first, last, f, out, segmented_iterator_tag(), Cat()))
      return out;
   return last;
}

} // namespace detail_algo

//! Returns an iterator to the first element equal to \c value
//! in [first, last), or \c last if not found.
template <class InpIter, class Sent, class T>
BOOST_CONTAINER_FORCEINLINE
InpIter segmented_find(InpIter first, Sent last, const T& BOOST_RESTRICT value)
{
   typedef segmented_iterator_traits<InpIter> traits;
   return detail_algo::segmented_find_dispatch
      (first, last, value, typename traits::is_segmented_iterator(), typename iterator_traits<InpIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_HPP
