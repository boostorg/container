//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_IF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_IF_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>

#include <boost/move/utility_core.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Sent, class Predicate>
FwdIt segmented_remove_if(FwdIt first, Sent last, Predicate pred);

namespace detail_algo {

//////////////////////////////////////////////////////////////////////////////
// Bounded result helper: fills destination range [dst_first, dst_last)
// from source elements not matching pred.  Advances next (by reference)
// until source is exhausted or destination is full.
// Recursively walks destination segments when dst is segmented.
// SrcCat is threaded through so the RA-optimized leaf is selected.
//
// When dst_last is unreachable_sentinel_t the destination-full check
// is optimised away, giving the same code as an unbounded loop.
//////////////////////////////////////////////////////////////////////////////

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RASrcIter, class DstIter, class DstSent, class Pred>
DstIter segmented_remove_if_dst_bounded
   (RASrcIter& next_out, RASrcIter last, DstIter dst_first, DstSent dst_last, Pred pred,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;
   RASrcIter next = next_out;

   difference_type n = last - next;

   while(n >= difference_type(4)) {
      if(!pred(*next)) { if(dst_first == dst_last) goto out_path; *dst_first = boost::move(*next); ++dst_first; } ++next;
      if(!pred(*next)) { if(dst_first == dst_last) goto out_path; *dst_first = boost::move(*next); ++dst_first; } ++next;
      if(!pred(*next)) { if(dst_first == dst_last) goto out_path; *dst_first = boost::move(*next); ++dst_first; } ++next;
      if(!pred(*next)) { if(dst_first == dst_last) goto out_path; *dst_first = boost::move(*next); ++dst_first; } ++next;
      n -= 4;
   }

   switch(n) {
      case 3:
         if(!pred(*next)) { if(dst_first == dst_last) goto out_path; *dst_first = boost::move(*next); ++dst_first; } ++next;
         BOOST_FALLTHROUGH;
      case 2:
         if(!pred(*next)) { if(dst_first == dst_last) goto out_path; *dst_first = boost::move(*next); ++dst_first; } ++next;
         BOOST_FALLTHROUGH;
      case 1:
         if(!pred(*next)) { if(dst_first == dst_last) goto out_path; *dst_first = boost::move(*next); ++dst_first; } ++next;
         BOOST_FALLTHROUGH;
      default:
         break;
   }
   out_path:
   next_out = next;
   return dst_first;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class SrcIter, class Sent, class DstIter, class DstSent, class Pred, class DstTag, class SrcCat>
typename algo_enable_if_c<!DstTag::value, DstIter>::type
segmented_remove_if_dst_bounded
   (SrcIter& next_out, Sent last, DstIter dst_first, DstSent dst_last, Pred pred, DstTag, SrcCat)
{
   SrcIter next = next_out;

   for(; next != last; ++next) {
      if(!pred(*next)) {
         if(dst_first == dst_last)
            goto out_path;
         *dst_first = boost::move(*next);
         ++dst_first;
      }
   }
   out_path:
   next_out = next;
   return dst_first;
}

template <class SrcIter, class Sent, class SegDstIter, class Pred, class SrcCat>
SegDstIter segmented_remove_if_dst_bounded
   (SrcIter& next, Sent last, SegDstIter dst_first, SegDstIter dst_last, Pred pred,
    segmented_iterator_tag, SrcCat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   dst_segment_iterator sfirst = dst_traits::segment(dst_first);
   const dst_segment_iterator slast = dst_traits::segment(dst_last);

   if(sfirst == slast) {
      dst_local_iterator r = (segmented_remove_if_dst_bounded)
         (next, last, dst_traits::local(dst_first), dst_traits::local(dst_last), pred, dst_is_local_seg_t(), SrcCat());
      return dst_traits::compose(sfirst, r);
   }
   else {
      dst_local_iterator dst_local = (segmented_remove_if_dst_bounded)
         (next, last, dst_traits::local(dst_first), dst_traits::end(sfirst), pred, dst_is_local_seg_t(), SrcCat());
      if(next == last)
         return dst_traits::compose(sfirst, dst_local);

      for(++sfirst; sfirst != slast; ++sfirst) {
         dst_local = (segmented_remove_if_dst_bounded)
            (next, last, dst_traits::begin(sfirst), dst_traits::end(sfirst), pred, dst_is_local_seg_t(), SrcCat());
         if(next == last)
            return dst_traits::compose(sfirst, dst_local);
      }

      dst_local = (segmented_remove_if_dst_bounded)
         (next, last, dst_traits::begin(slast), dst_traits::local(dst_last), pred, dst_is_local_seg_t(), SrcCat());
      return dst_traits::compose(sfirst, dst_local);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Result dispatch: routes to bounded helper.
// Non-segmented destination: single unbounded call (unreachable_sentinel_t).
// Segmented destination: loop over destination segments, bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class DstIter, class Pred, class Cat>
BOOST_CONTAINER_FORCEINLINE DstIter segmented_remove_if_dst_dispatch
   (SrcIter next, Sent last, DstIter first, Pred pred,
    const non_segmented_iterator_tag &, Cat)
{
   return (segmented_remove_if_dst_bounded)
      (next, last, first, unreachable_sentinel_t(), pred, non_segmented_iterator_tag(), Cat());
}

template <class SrcIter, class Sent, class SegDstIter, class Pred, class Cat>
SegDstIter segmented_remove_if_dst_dispatch
   (SrcIter next, Sent last, SegDstIter first, Pred pred,
    const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   if(next == last)
      return first;

   dst_segment_iterator dst_seg   = dst_traits::segment(first);
   dst_local_iterator   dst_local = dst_traits::local(first);

   while(next != last) {
      dst_local_iterator dst_end = dst_traits::end(dst_seg);
      dst_local = (segmented_remove_if_dst_bounded)
         (next, last, dst_local, dst_end, pred, dst_is_local_seg_t(), Cat());
      if(next != last) {
         ++dst_seg;
         dst_local = dst_traits::begin(dst_seg);
      }
   }
   return dst_traits::compose(dst_seg, dst_local);
}

//////////////////////////////////////////////////////////////////////////////
// Source dispatch: walks the source (read pointer) segments
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class DstIter, class Pred, class Tag, class Cat>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, SrcIter>::value, DstIter>::type
segmented_remove_if_dispatch
   (SrcIter next, Sent last, DstIter first, Pred pred, Tag, Cat)
{
   // No longer segmented source, now dispatch on destination segmentation.
   typedef segmented_iterator_traits<DstIter> dst_traits;
   return (segmented_remove_if_dst_dispatch)
      (next, last, first, pred,
       typename dst_traits::is_segmented_iterator(), Cat());
}

template <class SegSrcIter, class DstIter, class Pred, class Cat>
DstIter segmented_remove_if_dispatch
   (SegSrcIter next, SegSrcIter last, DstIter first, Pred pred,
    segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegSrcIter>  src_traits;
   typedef typename src_traits::local_iterator    src_local_iterator;
   typedef typename src_traits::segment_iterator  src_segment_iterator;
   typedef typename segmented_iterator_traits<src_local_iterator>::is_segmented_iterator src_is_local_seg_t;
   typedef typename iterator_traits<src_local_iterator>::iterator_category src_local_cat_t;

   src_segment_iterator sfirst = src_traits::segment(next);
   const src_segment_iterator slast  = src_traits::segment(last);

   if(sfirst == slast) {
      return (segmented_remove_if_dispatch)
         (src_traits::local(next), src_traits::local(last), first, pred,
          src_is_local_seg_t(), src_local_cat_t());
   }
   else {
      first = (segmented_remove_if_dispatch)
         (src_traits::local(next), src_traits::end(sfirst), first, pred,
          src_is_local_seg_t(), src_local_cat_t());

      for(++sfirst; sfirst != slast; ++sfirst)
         first = (segmented_remove_if_dispatch)
            (src_traits::begin(sfirst), src_traits::end(sfirst), first, pred,
             src_is_local_seg_t(), src_local_cat_t());

      return (segmented_remove_if_dispatch)
         (src_traits::begin(slast), src_traits::local(last), first, pred,
          src_is_local_seg_t(), src_local_cat_t());
   }
}

} // namespace detail_algo

//! Removes all elements for which \c pred returns true from [first, last),
//! moving retained elements forward. Returns iterator to new end.
template <class FwdIt, class Sent, class Predicate>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_remove_if(FwdIt first, Sent last, Predicate pred)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   first = segmented_find_if(first, last, pred);
   if(first == last)
      return last;

   FwdIt next = first;
   ++next;

   return detail_algo::segmented_remove_if_dispatch
      (next, last, first, pred,
       typename traits::is_segmented_iterator(),
       typename iterator_traits<FwdIt>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_IF_HPP
