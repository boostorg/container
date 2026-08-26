//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_HPP
#define BOOST_CONTAINER_G28_SHADOW_FILL 'W'

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/container/experimental/segmented_fill_n.hpp>
#include <boost/container/experimental/segmented_walk.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Sent, class T>
void segmented_fill(FwdIt first, Sent last, const T& value);

namespace detail_algo {

template <class FwdIt, class Sent, class T, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value>::type
segmented_fill_range(FwdIt first, Sent last, const T& value, Tag, Cat)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first)
      *first = value;
}

template <class T>
struct fill_walk_fn
{
   const T* pvalue;

   BOOST_CONTAINER_FORCEINLINE explicit fill_walk_fn(const T& v) : pvalue(&v) {}

   template <class It, class Sent>
   BOOST_CONTAINER_FORCEINLINE void operator()(It first, Sent last) const
   {
      (segmented_fill_range)(first, last, *pvalue, non_segmented_iterator_tag(),
         typename iterator_traits<It>::iterator_category());
   }
};

template <class SegIter, class T, class Cat>
void segmented_fill_range
   (SegIter first, SegIter last, const T& value, segmented_iterator_tag, Cat)
{
   fill_walk_fn<T> f(value);
   (segmented_walk)(first, last, f, segmented_iterator_tag(), Cat());
}

template <class FwdIt, class Sent, class T, class Cat>
BOOST_CONTAINER_FORCEINLINE
void segmented_fill_dispatch(FwdIt first, Sent last, const T& value, const Cat &)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   (segmented_fill_range)(first, last, value, typename traits::is_segmented_iterator(), Cat());
}

} // namespace detail_algo

//! Assigns \c value to every element in [first, last).
//! When \c Iter is a segmented iterator, exploits segmentation
//! to reduce per-element overhead.
//! When \c Sent is the same type as \c FwdIt and the iterator
//! category is random access, derives to \c segmented_fill_n.
template <class FwdIt, class Sent, class T>
BOOST_CONTAINER_FORCEINLINE
void segmented_fill(FwdIt first, Sent last, const T& value)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   (detail_algo::segmented_fill_range)( first, last, value
                                      , typename traits::is_segmented_iterator()
                                      , typename iterator_traits<FwdIt>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_HPP
