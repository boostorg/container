//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_COPY_IF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_COPY_IF_HPP

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
#include <boost/move/utility_core.hpp>
#include <cstddef>

namespace boost {
namespace container {

template <class InIter, class Sent, class OutIter, class Pred>
OutIter segmented_remove_copy_if(InIter first, Sent last, OutIter result, Pred pred);

namespace detail_algo {

//////////////////////////////////////////////////////////////////////////////
// Bounded destination helper: copies non-matching elements from source into
// [dst_first, dst_last), stopping when source is exhausted or destination
// is full.  Returns segduo<SrcIter, DstIter> with the final positions of
// both iterators.  Recursively walks destination segments when dst is
// segmented.
//
// When dst_last is unreachable_sentinel_t the destination-full check
// is optimised away, giving the same code as an unbounded loop.
//
// Move (first template parameter): true = move, false = copy.
//////////////////////////////////////////////////////////////////////////////

template <bool Move, class SrcIter, class Sent, class DstIter, class DstSent, class Pred, class DstTag, class SrcCat>
BOOST_CONTAINER_FORCEINLINE typename algo_enable_if_c<!DstTag::value, segduo<SrcIter, DstIter> >::type
segmented_remove_copy_if_dst_bounded
   (SrcIter first, Sent last, DstIter dst_first, DstSent dst_last, Pred pred, DstTag, SrcCat)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(!pred(*first)) {
         if(BOOST_UNLIKELY(dst_first == dst_last))
            goto out_path;
         transfer_op<Move>::apply(*dst_first, *first);
         ++dst_first;
      }
   }
   out_path:
   return segduo<SrcIter, DstIter>(first, dst_first);
}

template <std::size_t BlockSize, bool Move, class RASrcIter, class RADstIter,
          class Pred, class Diff>
BOOST_CONTAINER_FORCEINLINE segtrio<RASrcIter, RADstIter, Diff>
remove_copy_if_cleanup_blocks
   (RASrcIter cur, RADstIter dst_cur, RADstIter dst_last,
    Pred pred, Diff avail)
{
   const Diff block_size = static_cast<Diff>(BlockSize);
   while(avail >= block_size &&
         static_cast<Diff>(dst_last - dst_cur) >= block_size) {
      avail -= block_size;
      BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL
      for(Diff chunk = block_size; chunk; ) {
         --chunk;
         if(!pred(*cur)) {
            transfer_op<Move>::apply(*dst_cur, *cur);
            ++dst_cur;
         }
         ++cur;
      }
   }
   return segtrio<RASrcIter, RADstIter, Diff>(cur, dst_cur, avail);
}

template <bool Move, class RASrcIter, class RADstIter, class Pred>
typename iterator_enable_if_tag
   <RADstIter, std::random_access_iterator_tag, segduo<RASrcIter, RADstIter> >::type
segmented_remove_copy_if_dst_bounded
   (RASrcIter first, RASrcIter last, RADstIter dst_first, RADstIter dst_last, Pred pred,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &src_tag)
{
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;

   (void)src_tag;
   segtrio<RASrcIter, RADstIter, difference_type> r =
      (remove_copy_if_cleanup_blocks<32, Move>)
         (first, dst_first, dst_last, pred, last - first);

   return (segmented_remove_copy_if_dst_bounded<Move>)
      (r.first, last, r.second, dst_last, pred, non_segmented_iterator_tag(), int());
}

template <bool Move, class SrcIter, class Sent, class SegDstIter, class Pred, class SrcCat>
segduo<SrcIter, SegDstIter> segmented_remove_copy_if_dst_bounded
   (SrcIter first, Sent last, SegDstIter dst_first, SegDstIter dst_last, Pred pred,
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
         const segduo<SrcIter, dst_local_iterator> r = (segmented_remove_copy_if_dst_bounded<Move>)
            (first, last, db, de, pred, dst_is_local_seg_t(), SrcCat());
         first = r.first;
         if(last_seg || BOOST_UNLIKELY(first == last))
            return segduo<SrcIter, SegDstIter>(first, dst_traits::compose(sfirst, r.second));
      }

      for(++sfirst; sfirst != slast; ++sfirst) {
         const segduo<SrcIter, dst_local_iterator> r = (segmented_remove_copy_if_dst_bounded<Move>)
            (first, last, dst_traits::begin(sfirst), dst_traits::end(sfirst), pred, dst_is_local_seg_t(), SrcCat());
         first = r.first;
         if(BOOST_UNLIKELY(first == last))
            return segduo<SrcIter, SegDstIter>(first, dst_traits::compose(sfirst, r.second));
      }

      db = dst_traits::begin(sfirst);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Destination dispatch: routes to bounded helper.
// Non-segmented destination: single unbounded call (unreachable_sentinel_t).
// Segmented destination: loop over destination segments, bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <bool Move, class SrcIter, class Sent, class DstIter, class Pred, class Cat>
BOOST_CONTAINER_FORCEINLINE DstIter segmented_remove_copy_if_dst_dispatch
   (SrcIter first, Sent last, DstIter result, Pred pred,
    const non_segmented_iterator_tag &, Cat)
{
   return (segmented_remove_copy_if_dst_bounded<Move>)
      (first, last, result, unreachable_sentinel_t(), pred, non_segmented_iterator_tag(), Cat()).second;
}

template <bool Move, class SrcIter, class Sent, class SegDstIter, class Pred, class Cat>
SegDstIter segmented_remove_copy_if_dst_dispatch
   (SrcIter first, Sent last, SegDstIter result, Pred pred,
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
      const segduo<SrcIter, dst_local_iterator> r = (segmented_remove_copy_if_dst_bounded<Move>)
         (first, last, dst_local, dst_end, pred, dst_is_local_seg_t(), Cat());
      first = r.first;
      if(BOOST_LIKELY(first != last)) {
         ++dst_seg;
         dst_local = dst_traits::begin(dst_seg);
      }
      else {
         dst_local = r.second;
         break;
      }
   }
   return dst_traits::compose(dst_seg, dst_local);
}

//////////////////////////////////////////////////////////////////////////////
// Source dispatch: walks the source (read pointer) segments
//////////////////////////////////////////////////////////////////////////////

template <bool Move, class SrcIter, class Sent, class OutIter, class Pred, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE 
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, SrcIter>::value, OutIter>::type
segmented_remove_copy_if_dispatch
   (SrcIter first, Sent last, OutIter result, Pred pred, Tag, Cat)
{
   typedef segmented_iterator_traits<OutIter> dst_traits;
   return (segmented_remove_copy_if_dst_dispatch<Move>)
      (first, last, result, pred, typename dst_traits::is_segmented_iterator(), Cat());
}

template <bool Move, class SegIter, class OutIter, class Pred, class Cat>
OutIter segmented_remove_copy_if_dispatch
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
      result = (segmented_remove_copy_if_dispatch<Move>)
         (lb, le, result, pred, src_is_local_seg_t(), src_local_cat_t());
      if(BOOST_UNLIKELY(last_seg))
         return result;

      for(++sfirst; sfirst != slast; ++sfirst)
         result = (segmented_remove_copy_if_dispatch<Move>)
            (src_traits::begin(sfirst), src_traits::end(sfirst), result, pred, src_is_local_seg_t(), src_local_cat_t());

      lb = src_traits::begin(sfirst);
   }
}

} // namespace detail_algo

//! Copies elements from [first, last) to the range beginning at \c result,
//! skipping elements satisfying \c pred. Returns the output iterator past
//! the last element written.
template <class InIter, class Sent, class OutIter, class Pred>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_remove_copy_if(InIter first, Sent last, OutIter result, Pred pred)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_remove_copy_if_dispatch<false>
      (first, last, result, pred, typename traits::is_segmented_iterator(), typename iterator_traits<InIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_COPY_IF_HPP
