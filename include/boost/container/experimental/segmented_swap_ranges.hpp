//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SWAP_RANGES_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SWAP_RANGES_HPP

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
#include <boost/move/adl_move_swap.hpp>

namespace boost {
namespace container {

template <class FwdIt1, class Sent, class FwdIt2>
FwdIt2 segmented_swap_ranges(FwdIt1 first1, Sent last1, FwdIt2 first2);

namespace detail_algo {

template <class SrcIter, class Sent, class DstIter, class DstSent, class DstTag, class SrcCat>
BOOST_CONTAINER_FORCEINLINE typename algo_enable_if_c<!DstTag::value, segduo<SrcIter, DstIter> >::type
segmented_swap_ranges_dst_bounded
   (SrcIter first, Sent last, DstIter dst_first, DstSent dst_last, DstTag, SrcCat)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(BOOST_CONTAINER_SEG_UNLIKELY(dst_first == dst_last))
         goto out_path;
      boost::adl_move_swap(*first, *dst_first);
      ++dst_first;
   }
   out_path:
   return segduo<SrcIter, DstIter>(first, dst_first);
}

template <class RASrcIter, class RADstIter>
BOOST_CONTAINER_FORCEINLINE
typename iterator_enable_if_tag
   <RADstIter, std::random_access_iterator_tag, segduo<RASrcIter, RADstIter> >::type
segmented_swap_ranges_dst_bounded
   (RASrcIter first, RASrcIter last, RADstIter dst_first, RADstIter dst_last,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &src_tag)
{
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;
   const difference_type src_n = last - first;
   const difference_type dst_n = difference_type(dst_last - dst_first);
   const difference_type n = src_n < dst_n ? src_n : dst_n;
   return (segmented_swap_ranges_dst_bounded)(first, first + n, dst_first, unreachable_sentinel_t(),
      non_segmented_iterator_tag(), src_tag);
}

template <class SrcIter, class Sent, class SegDstIter, class SrcCat>
segduo<SrcIter, SegDstIter> segmented_swap_ranges_dst_bounded
   (SrcIter first, Sent last, SegDstIter dst_first, SegDstIter dst_last,
    segmented_iterator_tag, SrcCat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   dst_segment_iterator       sfirst = dst_traits::segment(dst_first);
   const dst_segment_iterator slast  = dst_traits::segment(dst_last);

   dst_local_iterator db = dst_traits::local(dst_first);

   if(BOOST_CONTAINER_SEG_LIKELY(sfirst != slast)) {
      {
         const segduo<SrcIter, dst_local_iterator> r = (segmented_swap_ranges_dst_bounded)
            (first, last, db, dst_traits::end(sfirst), dst_is_local_seg_t(), SrcCat());
         first = r.first;
         if(BOOST_CONTAINER_SEG_UNLIKELY(first == last))
            return segduo<SrcIter, SegDstIter>(first, dst_traits::compose(sfirst, r.second));
      }

      for(++sfirst; sfirst != slast; ++sfirst) {
         const segduo<SrcIter, dst_local_iterator> r = (segmented_swap_ranges_dst_bounded)
            (first, last, dst_traits::begin(sfirst), dst_traits::end(sfirst), dst_is_local_seg_t(), SrcCat());
         first = r.first;
         if(BOOST_CONTAINER_SEG_UNLIKELY(first == last))
            return segduo<SrcIter, SegDstIter>(first, dst_traits::compose(sfirst, r.second));
      }

      db = dst_traits::begin(slast);
   }
   const segduo<SrcIter, dst_local_iterator> r = (segmented_swap_ranges_dst_bounded)
      (first, last, db, dst_traits::local(dst_last), dst_is_local_seg_t(), SrcCat());
   return segduo<SrcIter, SegDstIter>(r.first, dst_traits::compose(sfirst, r.second));
}

//////////////////////////////////////////////////////////////////////////////
// Second-range dispatch: routes to bounded helper.
// Non-segmented second range: single unbounded call (unreachable_sentinel_t).
// Segmented second range: loop over its segments, bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class DstIter, class Cat>
BOOST_CONTAINER_FORCEINLINE DstIter segmented_swap_ranges_dst_dispatch
   (SrcIter first, Sent last, DstIter result,
    const non_segmented_iterator_tag &, Cat)
{
   return (segmented_swap_ranges_dst_bounded)
      (first, last, result, unreachable_sentinel_t(), non_segmented_iterator_tag(), Cat()).second;
}

template <class SrcIter, class Sent, class SegDstIter, class Cat>
SegDstIter segmented_swap_ranges_dst_dispatch
   (SrcIter first, Sent last, SegDstIter result,
    const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   if(first == last)
      return result;

   dst_segment_iterator dst_seg   = dst_traits::segment(result);
   dst_local_iterator   dst_local = dst_traits::local(result);

   while(1) {
      const dst_local_iterator dst_end = dst_traits::end(dst_seg);
      const segduo<SrcIter, dst_local_iterator> r = (segmented_swap_ranges_dst_bounded)
         (first, last, dst_local, dst_end, dst_is_local_seg_t(), Cat());
      first = r.first;
      if(BOOST_CONTAINER_SEG_LIKELY(first != last)) {
         ++dst_seg;
         dst_local = dst_traits::begin(dst_seg);
      }
      else {
         return dst_traits::compose(dst_seg, r.second);
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// First-range dispatch: walks the first (read pointer) range segments
//////////////////////////////////////////////////////////////////////////////

template <class FwdIt1, class Sent, class FwdIt2, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt1>::value, FwdIt2>::type
segmented_swap_ranges_dispatch (FwdIt1 first1, Sent last1, FwdIt2 first2, Tag, Cat)
{
   typedef segmented_iterator_traits<FwdIt2> dst_traits;
   return (segmented_swap_ranges_dst_dispatch)
      (first1, last1, first2, typename dst_traits::is_segmented_iterator(), Cat());
}

template <class SegIter, class FwdIt2, class Cat>
FwdIt2 segmented_swap_ranges_dispatch (SegIter first1, SegIter last1, FwdIt2 first2, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>    src_traits;
   typedef typename src_traits::local_iterator   src_local_iterator;
   typedef typename src_traits::segment_iterator src_segment_iterator;
   typedef typename segmented_iterator_traits<src_local_iterator>::is_segmented_iterator src_is_local_seg_t;
   typedef typename iterator_traits<src_local_iterator>::iterator_category src_local_cat_t;

   src_segment_iterator sfirst = src_traits::segment(first1);
   const src_segment_iterator slast  = src_traits::segment(last1);

   src_local_iterator lb = src_traits::local(first1);

   if(BOOST_CONTAINER_SEG_LIKELY(sfirst != slast)) {
      first2 = (segmented_swap_ranges_dispatch)
         (lb, src_traits::end(sfirst), first2, src_is_local_seg_t(), src_local_cat_t());

      for(++sfirst; sfirst != slast; ++sfirst)
         first2 = (segmented_swap_ranges_dispatch)
            (src_traits::begin(sfirst), src_traits::end(sfirst), first2, src_is_local_seg_t(), src_local_cat_t());

      lb = src_traits::begin(slast);
   }
   return (segmented_swap_ranges_dispatch)
      (lb, src_traits::local(last1), first2, src_is_local_seg_t(), src_local_cat_t());
}

} // namespace detail_algo

//! Swaps elements in [first1, last1) with the range starting at \c first2.
//! Returns an iterator past the last swapped element in the second range.
//! Segmentation is exploited on both ranges.
template <class FwdIt1, class Sent, class FwdIt2>
BOOST_CONTAINER_FORCEINLINE
FwdIt2 segmented_swap_ranges(FwdIt1 first1, Sent last1, FwdIt2 first2)
{
   typedef segmented_iterator_traits<FwdIt1> traits;
   return detail_algo::segmented_swap_ranges_dispatch
      (first1, last1, first2, typename traits::is_segmented_iterator()
      , typename iterator_traits<FwdIt1>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SWAP_RANGES_HPP
