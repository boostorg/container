//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COUNT_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COUNT_HPP
#define BOOST_CONTAINER_G28_SHADOW_COUNT 'W'

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
#include <boost/container/experimental/segmented_walk.hpp>

namespace boost {
namespace container {

template <class InpIter, class Sent, class T>
typename boost::container::iterator_traits<InpIter>::difference_type
segmented_count(InpIter first, Sent last, const T& BOOST_RESTRICT value);

namespace detail_algo {

template <class InpIter, class Sent, class T, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InpIter>::value,
   typename boost::container::iterator_traits<InpIter>::difference_type>::type
segmented_count_dispatch
   (InpIter first, Sent last, const T& BOOST_RESTRICT value, Tag, Cat)
{
   typedef typename boost::container::iterator_traits<InpIter>::difference_type diff_t;
   diff_t n = 0;

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for (; first != last; ++first)
   #if defined(BOOST_CONTAINER_SEGMENTED_COUNT_BRANCHLESS)
      n += static_cast<diff_t>(*first == value);
   #else
      if (*first == value) ++n;
   #endif
   return n;
}

template <class T, class Diff>
struct count_walk_fn
{
   const T* pvalue;
   Diff result;

   BOOST_CONTAINER_FORCEINLINE explicit count_walk_fn(const T& v) : pvalue(&v), result(0) {}

   template <class It, class Sent>
   BOOST_CONTAINER_FORCEINLINE void operator()(It first, Sent last)
   {
      result += (segmented_count_dispatch)(first, last, *pvalue,
         non_segmented_iterator_tag(), typename iterator_traits<It>::iterator_category());
   }
};

template <class SegIter, class T, class Cat>
typename boost::container::iterator_traits<SegIter>::difference_type
   segmented_count_dispatch(SegIter first, SegIter last, const T& BOOST_RESTRICT value, segmented_iterator_tag, Cat)
{
   count_walk_fn<T, typename boost::container::iterator_traits<SegIter>::difference_type> f(value);
   (segmented_walk)(first, last, f, segmented_iterator_tag(), Cat());
   return f.result;
}

} // namespace detail_algo

//! Returns the number of elements equal to \c value in [first, last).
template <class InpIter, class Sent, class T>
BOOST_CONTAINER_FORCEINLINE
typename boost::container::iterator_traits<InpIter>::difference_type
   segmented_count(InpIter first, Sent last, const T& BOOST_RESTRICT value)
{
   typedef segmented_iterator_traits<InpIter> traits;
   return detail_algo::segmented_count_dispatch(first, last, value,
      typename traits::is_segmented_iterator(), typename iterator_traits<InpIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COUNT_HPP
