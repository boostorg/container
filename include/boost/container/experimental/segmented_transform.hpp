//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_TRANSFORM_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_TRANSFORM_HPP

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

template <class InIter, class Sent, class OutIter, class UnaryOp>
OutIter segmented_transform(InIter first, Sent last, OutIter result, UnaryOp op);

namespace detail_algo {

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class InIter, class OutIter, class UnaryOp>
OutIter segmented_transform_dispatch
   (InIter first, InIter last, OutIter result, UnaryOp op, const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<InIter>::difference_type difference_type;
   difference_type n = last - first;

   while(n >= difference_type(4)) {
      *result = op(*first); ++first; ++result;
      *result = op(*first); ++first; ++result;
      *result = op(*first); ++first; ++result;
      *result = op(*first); ++first; ++result;
      n -= 4;
   }

   switch(n) {
      case 3:
         *result = op(*first); ++first; ++result;
         *result = op(*first); ++first; ++result;
         *result = op(*first); ++first; ++result;
         break;
      case 2:
         *result = op(*first); ++first; ++result;
         *result = op(*first); ++first; ++result;
         break;
      case 1:
         *result = op(*first); ++first; ++result;
         break;
      default:
         break;
   }
   return result;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING


template <class InIter, class Sent, class OutIter, class UnaryOp, class Tag, class Cat>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InIter>::value, OutIter>::type
segmented_transform_dispatch
   (InIter first, Sent last, OutIter result, UnaryOp op, Tag, Cat)
{
   for(; first != last; ++first, ++result)
      *result = op(*first);
   return result;
}

template <class SegIter, class OutIter, class UnaryOp, class Cat>
OutIter segmented_transform_dispatch
   (SegIter first, SegIter last, OutIter result, UnaryOp op, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator       sfirst = traits::segment(first);
   segment_iterator const slast  = traits::segment(last);

   if(sfirst == slast) {
      return (segmented_transform_dispatch)(traits::local(first), traits::local(last), result, op, is_local_seg_t(), Cat());
   }
   else {
      result = (segmented_transform_dispatch)(traits::local(first), traits::end(sfirst), result, op, is_local_seg_t(), Cat());

      for(++sfirst; sfirst != slast; ++sfirst)
         result = (segmented_transform_dispatch)(traits::begin(sfirst), traits::end(sfirst), result, op, is_local_seg_t(), Cat());

      return (segmented_transform_dispatch)(traits::begin(sfirst), traits::local(last), result, op, is_local_seg_t(), Cat());
   }
}

} // namespace detail_algo

//! Applies \c op to each element in [first, last) and writes the result
//! to the range beginning at \c result.
//! Segmentation is exploited on the input range only.
template <class InIter, class Sent, class OutIter, class UnaryOp>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_transform (InIter first, Sent last, OutIter result, UnaryOp op)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_transform_dispatch
      (first, last, result, op, typename traits::is_segmented_iterator(), typename iterator_traits<InIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_TRANSFORM_HPP
