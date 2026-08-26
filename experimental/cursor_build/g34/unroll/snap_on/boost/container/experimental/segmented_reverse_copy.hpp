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

//////////////////////////////////////////////////////////////////////////////
// Bounded destination helper: reverse-copies from [first, last) into
// [dst_first, dst_last), reading backward from last toward first,
// writing forward into dst.
// Returns segduo<BidirIter, DstIter> where .first is the updated source
// read position (last, moved backward) and .second is the updated dst.
// When dst_last is unreachable_sentinel_t the destination-full check
// is optimised away, giving the same code as an unbounded loop.
//////////////////////////////////////////////////////////////////////////////

template <class BidirIter, class DstIter, class DstSent, class DstTag, class SrcCat>
BOOST_CONTAINER_FORCEINLINE typename algo_enable_if_c<!DstTag::value, segduo<BidirIter, DstIter> >::type
segmented_reverse_copy_dst_bounded
   (BidirIter first, BidirIter last, DstIter dst_first, DstSent dst_last, DstTag, SrcCat)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   while(first != last) {
      if(BOOST_UNLIKELY(dst_first == dst_last))
         goto out_path;
      --last;
      *dst_first = *last;
      ++dst_first;
   }
   out_path:
   return segduo<BidirIter, DstIter>(last, dst_first);
}

template <class RASrcIter, class RADstIter>
BOOST_CONTAINER_FORCEINLINE
typename iterator_enable_if_tag
   <RADstIter, std::random_access_iterator_tag, segduo<RASrcIter, RADstIter> >::type
segmented_reverse_copy_dst_bounded
   (RASrcIter first, RASrcIter last, RADstIter dst_first, RADstIter dst_last,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &src_tag)
{
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;
   const difference_type src_n = last - first;
   const difference_type dst_n = difference_type(dst_last - dst_first);
   const difference_type n = src_n < dst_n ? src_n : dst_n;
   return (segmented_reverse_copy_dst_bounded)(last - n, last, dst_first, unreachable_sentinel_t(),
      non_segmented_iterator_tag(), src_tag);
}

template <class BidirIter, class SegDstIter, class SrcCat>
segduo<BidirIter, SegDstIter> segmented_reverse_copy_dst_bounded
   (BidirIter first, BidirIter last, SegDstIter dst_first, SegDstIter dst_last,
    segmented_iterator_tag, SrcCat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   dst_segment_iterator       sfirst = dst_traits::segment(dst_first);
   const dst_segment_iterator slast  = dst_traits::segment(dst_last);

   dst_local_iterator db = dst_traits::local(dst_first);

   for(;;) {
      const bool last_seg = sfirst == slast;
      const dst_local_iterator de = last_seg ? dst_traits::local(dst_last) : dst_traits::end(sfirst);
      {
         const segduo<BidirIter, dst_local_iterator> r = (segmented_reverse_copy_dst_bounded)
            (first, last, db, de, dst_is_local_seg_t(), SrcCat());
         last = r.first;
         if(last_seg || BOOST_UNLIKELY(first == last))
            return segduo<BidirIter, SegDstIter>(last, dst_traits::compose(sfirst, r.second));
      }

      for(++sfirst; sfirst != slast; ++sfirst) {
         const segduo<BidirIter, dst_local_iterator> r = (segmented_reverse_copy_dst_bounded)
            (first, last, dst_traits::begin(sfirst), dst_traits::end(sfirst), dst_is_local_seg_t(), SrcCat());
         last = r.first;
         if(BOOST_UNLIKELY(first == last))
            return segduo<BidirIter, SegDstIter>(last, dst_traits::compose(sfirst, r.second));
      }

      db = dst_traits::begin(sfirst);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Destination dispatch: routes to bounded helper.
// Non-segmented destination: single unbounded call (unreachable_sentinel_t).
// Segmented destination: loop over destination segments, bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <class BidirIter, class DstIter, class Cat>
BOOST_CONTAINER_FORCEINLINE DstIter segmented_reverse_copy_dst_dispatch
   (BidirIter first, BidirIter last, DstIter result,
    const non_segmented_iterator_tag &, Cat)
{
   return (segmented_reverse_copy_dst_bounded)
      (first, last, result, unreachable_sentinel_t(), non_segmented_iterator_tag(), Cat()).second;
}

template <class BidirIter, class SegDstIter, class Cat>
SegDstIter segmented_reverse_copy_dst_dispatch
   (BidirIter first, BidirIter last, SegDstIter result,
    const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   if(BOOST_UNLIKELY(first == last))
      return result;

   dst_segment_iterator dst_seg   = dst_traits::segment(result);
   dst_local_iterator   dst_local = dst_traits::local(result);

   while(1) {
      const dst_local_iterator dst_end = dst_traits::end(dst_seg);
      const segduo<BidirIter, dst_local_iterator> r = (segmented_reverse_copy_dst_bounded)
         (first, last, dst_local, dst_end, dst_is_local_seg_t(), Cat());
      last = r.first;
      if(BOOST_LIKELY(first != last)) {
         ++dst_seg;
         dst_local = dst_traits::begin(dst_seg);
      }
      else
         return dst_traits::compose(dst_seg, r.second);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Source dispatch: walks the source (read pointer) segments in reverse
//////////////////////////////////////////////////////////////////////////////

template <class BidirIter, class OutIter, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE 
typename algo_enable_if_c<!Tag::value, OutIter>::type
   segmented_reverse_copy_dispatch(BidirIter first, BidirIter last, OutIter result, Tag, Cat)
{
   typedef segmented_iterator_traits<OutIter> dst_traits;
   return (segmented_reverse_copy_dst_dispatch)
      (first, last, result, typename dst_traits::is_segmented_iterator(), Cat());
}

template <class SegIter, class OutIter, class Cat>
OutIter segmented_reverse_copy_dispatch
   (SegIter first, SegIter last, OutIter result, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>   traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   segment_iterator const sfirst = traits::segment(first);
   segment_iterator       slast  = traits::segment(last);

   local_iterator le = traits::local(last);

   for(;;) {
      //Partial segments (last and first) share this call site: the walk runs
      //backwards, so the fused bound is the *begin* of the copied range.
      const bool first_seg = sfirst == slast;
      const local_iterator lb = first_seg ? traits::local(first) : traits::begin(slast);
      result = (segmented_reverse_copy_dispatch)(lb, le, result, is_local_seg_t(), local_cat_t());
      if(BOOST_UNLIKELY(first_seg))
         return result;

      //Middle segments (in reverse) keep their own call site: begin/end both
      //come from slast, so the leaf can be specialised for full segments
      for (--slast; slast != sfirst; --slast)
         result = (segmented_reverse_copy_dispatch)(traits::begin(slast), traits::end(slast), result, is_local_seg_t(), local_cat_t());

      le = traits::end(sfirst);
   }
}

} // namespace detail_algo

//! Copies elements from [first, last) to the range beginning at \c result
//! in reverse order. When the source range uses segmented iterators,
//! exploits segmentation by walking segments in reverse order, processing
//! each local range without per-element segment-boundary overhead.
//! When the output range also uses segmented iterators, walks output
//! segments as well (dual-segmented / multi-segmented dispatch).
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
