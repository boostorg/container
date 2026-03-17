//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REVERSE_COPY_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REVERSE_COPY_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>

namespace boost {
namespace container {

template <class BidirIter, class OutIter>
OutIter segmented_reverse_copy(BidirIter first, BidirIter last, OutIter result);

namespace detail_algo {

template <class SegIter, class OutIter>
OutIter segmented_reverse_copy_dispatch
   (SegIter first, SegIter last, OutIter result, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::segment_iterator segment_iterator;

   if(first == last)
      return result;

   segment_iterator sfirst = traits::segment(first);
   segment_iterator slast  = traits::segment(last);

   if(sfirst == slast) {
      return boost::container::segmented_reverse_copy(
         traits::local(first), traits::local(last), result);
   }

   result = boost::container::segmented_reverse_copy(
      traits::begin(slast), traits::local(last), result);

   segment_iterator s = slast;
   for(--s; s != sfirst; --s)
      result = boost::container::segmented_reverse_copy(
         traits::begin(s), traits::end(s), result);

   result = boost::container::segmented_reverse_copy(
      traits::local(first), traits::end(sfirst), result);

   return result;
}

template <class BidirIter, class OutIter>
OutIter segmented_reverse_copy_dispatch
   (BidirIter first, BidirIter last, OutIter result, non_segmented_iterator_tag)
{
   while(first != last) {
      --last;
      *result = *last;
      ++result;
   }
   return result;
}

} // namespace detail_algo

//! Copies elements from [first, last) to the range beginning at \c result
//! in reverse order. When the source range uses segmented iterators,
//! exploits segmentation by walking segments in reverse order, processing
//! each local range without per-element segment-boundary overhead.
//! Returns the output iterator past the last element written.
template <class BidirIter, class OutIter>
BOOST_CONTAINER_FORCEINLINE OutIter segmented_reverse_copy(BidirIter first, BidirIter last, OutIter result)
{
   typedef segmented_iterator_traits<BidirIter> traits;
   return detail_algo::segmented_reverse_copy_dispatch(first, last, result,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REVERSE_COPY_HPP
