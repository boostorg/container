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
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

template <class BidirIter, class OutIter>
OutIter segmented_reverse_copy(BidirIter first, BidirIter last, OutIter result);

namespace detail_algo {

template <class BidirIter, class OutIter, class Cat>
OutIter segmented_reverse_copy_dispatch
   (BidirIter first, BidirIter last, OutIter result, non_segmented_iterator_tag, const Cat &)
{
   while(first != last) {
      --last;
      *result = *last;
      ++result;
   }
   return result;
}

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RAIter, class OutIter>
OutIter segmented_reverse_copy_dispatch
   (RAIter first, RAIter last, OutIter result, non_segmented_iterator_tag, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RAIter>::difference_type difference_type;
   difference_type n = last - first;

   while(n >= difference_type(4)) {
      --last; *result = *last; ++result;
      --last; *result = *last; ++result;
      --last; *result = *last; ++result;
      --last; *result = *last; ++result;
      n -= 4;
   }

   switch (n % 4) {
      case 3:
         --last; *result = *last; ++result;
      BOOST_FALLTHROUGH;
      case 2:
         --last; *result = *last; ++result;
      BOOST_FALLTHROUGH;
      case 1:
         --last; *result = *last; ++result;
      BOOST_FALLTHROUGH;
      default:
         break;
   }
   return result;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class SegIter, class OutIter, class Cat>
OutIter segmented_reverse_copy_dispatch
   (SegIter first, SegIter last, OutIter result, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>   traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator const sfirst = traits::segment(first);
   segment_iterator       slast  = traits::segment(last);

   if(sfirst == slast) {
      return (segmented_reverse_copy_dispatch)(traits::local(first), traits::local(last), result, is_local_seg_t(), Cat());
   }
   else {
      result = (segmented_reverse_copy_dispatch)(traits::begin(slast), traits::local(last), result, is_local_seg_t(), Cat());

      for (--slast; slast != sfirst; --slast)
         result = (segmented_reverse_copy_dispatch)(traits::begin(slast), traits::end(slast), result, is_local_seg_t(), Cat());

      return (segmented_reverse_copy_dispatch)(traits::local(first), traits::end(sfirst), result, is_local_seg_t(), Cat());
   }
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
   return detail_algo::segmented_reverse_copy_dispatch
      (first, last, result, typename traits::is_segmented_iterator(), typename iterator_traits<BidirIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REVERSE_COPY_HPP
