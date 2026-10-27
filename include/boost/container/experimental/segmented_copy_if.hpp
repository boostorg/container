//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_IF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_IF_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/detail/segmented_common_algo.hpp>
#include <boost/container/detail/iterator.hpp>
#include <cstddef>

namespace boost {
namespace container {

template <class InIter, class Sent, class OutIter, class Pred>
OutIter segmented_copy_if(InIter first, Sent last, OutIter result, Pred pred);

namespace detail_algo {

//////////////////////////////////////////////////////////////////////////////
// Bounded destination helper: copies the elements that satisfy pred from the source into
// [dst_first, dst_last).  It stops when the source is exhausted, or when an
// element must be written and the destination is full: then it returns the
// "carry" flag (third member) and `first` points to that element.  The caller
// passes the flag back with the next destination range, and the element is
// written without testing it again (pred is applied exactly last - first times).
//
// When dst_last is unreachable_sentinel_t the destination-full check
// is optimised away, giving the same code as an unbounded loop.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class DstIter, class DstSent, class Pred, class DstTag, class SrcCat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<!DstTag::value, segtrio<SrcIter, DstIter, bool> >::type
segmented_copy_if_dst_bounded
   (SrcIter first, Sent last, DstIter dst_first, DstSent dst_last, Pred pred, bool carry, DstTag, SrcCat)
{
   //The caller gives a carry only with a range that has room
   if(carry) {
      *dst_first = *first;
      ++dst_first;
      ++first;
      carry = false;
   }
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   while(first != last) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(dst_first == dst_last)) {
            carry = true;
            break;
         }
         *dst_first = *first;
         ++dst_first;
      }
      ++first;
   }
   return segtrio<SrcIter, DstIter, bool>(first, dst_first, carry);
}

#if BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS
template <class RASrcIter, class RADstIter, class Pred>
BOOST_CONTAINER_FORCEINLINE typename iterator_enable_if_tag
   <RADstIter, std::random_access_iterator_tag, segtrio<RASrcIter, RADstIter, bool> >::type
segmented_copy_if_dst_bounded
   (RASrcIter first, RASrcIter last, RADstIter dst_first, RADstIter dst_last, Pred pred, bool carry,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   //Same carry contract as the generic leaf
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;
   typedef segtrio<RASrcIter, RADstIter, bool> result_t;
   const difference_type block_size = 16;

   if(carry) {
      *dst_first = *first;
      ++dst_first;
      ++first;
      carry = false;
   }
   difference_type n = last - first;

   //Avoid destination check if both input and output ranges are big enough
   while(n >= block_size &&
         static_cast<difference_type>(dst_last - dst_first) >= block_size) {
      n -= block_size;
      BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL
      for(difference_type chunk = block_size; chunk; ) {
         --chunk;
         if(pred(*first)) {
            *dst_first = *first;
            ++dst_first;
         }
         ++first;
      }
   }

   //Remaining elements
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   while(n) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(dst_first == dst_last)) {
            carry = true;
            break;
         }
         *dst_first = *first;
         ++dst_first;
      }
      ++first;
      --n;
   }
   return result_t(first, dst_first, carry);
}
#endif   // BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS

template <class SrcIter, class Sent, class SegDstIter, class Pred, class SrcCat>
segtrio<SrcIter, SegDstIter, bool> segmented_copy_if_dst_bounded
   (SrcIter first, Sent last, SegDstIter dst_first, SegDstIter dst_last, Pred pred, bool carry,
    segmented_iterator_tag, SrcCat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   dst_segment_iterator       dsfirst = dst_traits::segment(dst_first);
   const dst_segment_iterator dslast  = dst_traits::segment(dst_last);

   dst_local_iterator db = dst_traits::local(dst_first);

   for(;;) {
      const bool last_seg = dsfirst == dslast;
      const dst_local_iterator de = last_seg ? dst_traits::local(dst_last) : dst_traits::end(dsfirst);
      //Segments are never empty, but the range can end at the start of the last one
      if(BOOST_UNLIKELY(carry && db == de))
         return segtrio<SrcIter, SegDstIter, bool>(first, dst_traits::compose(dsfirst, db), true);
      const segtrio<SrcIter, dst_local_iterator, bool> r = (segmented_copy_if_dst_bounded)
         (first, last, db, de, pred, carry, dst_is_local_seg_t(), SrcCat());
      first = r.first;
      carry = r.third;
      if(!carry || last_seg)
         return segtrio<SrcIter, SegDstIter, bool>(first, dst_traits::compose(dsfirst, r.second), carry);
      ++dsfirst;
      db = dst_traits::begin(dsfirst);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Destination dispatch: routes to bounded helper.
// Non-segmented destination: single unbounded call (unreachable_sentinel_t).
// Segmented destination: loop over destination segments, bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class DstIter, class Pred, class Cat>
BOOST_CONTAINER_FORCEINLINE DstIter segmented_copy_if_dst_dispatch
   (SrcIter first, Sent last, DstIter result, Pred pred,
    const non_segmented_iterator_tag &, Cat)
{
   return (segmented_copy_if_dst_bounded)
      (first, last, result, unreachable_sentinel_t(), pred, false, non_segmented_iterator_tag(), Cat()).second;
}

template <class SrcIter, class Sent, class SegDstIter, class Pred, class Cat>
SegDstIter segmented_copy_if_dst_dispatch
   (SrcIter first, Sent last, SegDstIter result, Pred pred,
    const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   dst_segment_iterator dst_seg   = dst_traits::segment(result);
   dst_local_iterator   dst_local = dst_traits::local(result);
   //The first range is empty: end() of the first segment is computed only when
   //an element must be written.  If nothing is written, the segment can be
   //singular (e.g. begin() of an empty deque).
   dst_local_iterator   dst_end   = dst_local;
   bool bounded = false;
   bool carry   = false;

   for(;;) {
      const segtrio<SrcIter, dst_local_iterator, bool> r = (segmented_copy_if_dst_bounded)
         (first, last, dst_local, dst_end, pred, carry, dst_is_local_seg_t(), Cat());
      first = r.first;
      carry = r.third;
      if(BOOST_LIKELY(!carry))
         return dst_traits::compose(dst_seg, r.second);
      //An element must be written: use the rest of the first segment, then the next ones
      if(!bounded) {
         bounded = true;
      }
      else {
         ++dst_seg;
         dst_local = dst_traits::begin(dst_seg);
      }
      dst_end = dst_traits::end(dst_seg);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Source dispatch: walks the source (read pointer) segments
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class OutIter, class Pred, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE 
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, SrcIter>::value, OutIter>::type
   segmented_copy_if_dispatch(SrcIter first, Sent last, OutIter result, Pred pred, Tag, Cat)
{
   typedef segmented_iterator_traits<OutIter> dst_traits;
   return (segmented_copy_if_dst_dispatch)
      (first, last, result, pred, typename dst_traits::is_segmented_iterator(), Cat());
}

template <class SegIter, class OutIter, class Pred, class Cat>
OutIter segmented_copy_if_dispatch
   (SegIter first, SegIter last, OutIter result, Pred pred, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>  src_traits;
   typedef typename src_traits::local_iterator   src_local_iterator;
   typedef typename src_traits::segment_iterator src_segment_iterator;
   typedef typename segmented_iterator_traits<src_local_iterator>::is_segmented_iterator src_is_local_seg_t;
   typedef typename iterator_traits<src_local_iterator>::iterator_category src_local_cat_t;

   src_segment_iterator sfirst = src_traits::segment(first);
   const src_segment_iterator slast  = src_traits::segment(last);

   src_local_iterator lb = src_traits::local(first);

   for(;;) {
      const bool last_seg = sfirst == slast;
      const src_local_iterator le = last_seg ? src_traits::local(last) : src_traits::end(sfirst);
      result = (segmented_copy_if_dispatch)
         (lb, le, result, pred, src_is_local_seg_t(), src_local_cat_t());
      if(BOOST_UNLIKELY(last_seg))
         return result;

      for(++sfirst; sfirst != slast; ++sfirst)
         result = (segmented_copy_if_dispatch)
            (src_traits::begin(sfirst), src_traits::end(sfirst), result, pred, src_is_local_seg_t(), src_local_cat_t());

      lb = src_traits::begin(sfirst);
   }
}

} // namespace detail_algo

//! Copies elements satisfying \c pred from [first, last) to the range
//! beginning at \c result. Segmentation is exploited on both input and
//! output ranges.
template <class InIter, class Sent, class OutIter, class Pred>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_copy_if(InIter first, Sent last, OutIter result, Pred pred)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_copy_if_dispatch(first, last, result, pred,
      typename traits::is_segmented_iterator(), typename iterator_traits<InIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_IF_HPP
