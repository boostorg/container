//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_N_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_N_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/std_fwd.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

template <class InIter, class Size, class OutIter>
OutIter segmented_copy_n(InIter first, Size count, OutIter result);

namespace detail_algo {

//////////////////////////////////////////////////////////////////////////////
// Bounded destination helper: copies from source count elements into
// [dst_first, dst_last), stopping when count reaches zero or destination
// is full.  Returns segduo<SrcIter, DstIter> with the final positions of
// both iterators.  count is advanced by reference so the caller knows
// how many elements remain.
// Recursively walks destination segments when dst is segmented.
//
// When dst_last is unreachable_sentinel_t the destination-full check
// is optimised away, giving the same code as an unbounded loop.
//////////////////////////////////////////////////////////////////////////////

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RASrcIter, class Size, class DstIter, class DstSent>
segduo<RASrcIter, DstIter> segmented_copy_n_dst_bounded
   (RASrcIter first, Size& count, DstIter dst_first, DstSent dst_last,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   while(count >= Size(4)) {
      if(dst_first == dst_last) goto out_path;
      *dst_first = *first; ++first; ++dst_first; --count;
      if(dst_first == dst_last) goto out_path;
      *dst_first = *first; ++first; ++dst_first; --count;
      if(dst_first == dst_last) goto out_path;
      *dst_first = *first; ++first; ++dst_first; --count;
      if(dst_first == dst_last) goto out_path;
      *dst_first = *first; ++first; ++dst_first; --count;
   }

   switch(count) {
      case 3:
         if(dst_first == dst_last) goto out_path;
         *dst_first = *first; ++first; ++dst_first; --count;
         BOOST_FALLTHROUGH;
      case 2:
         if(dst_first == dst_last) goto out_path;
         *dst_first = *first; ++first; ++dst_first; --count;
         BOOST_FALLTHROUGH;
      case 1:
         if(dst_first == dst_last) goto out_path;
         *dst_first = *first; ++first; ++dst_first; --count;
         BOOST_FALLTHROUGH;
      default:
         break;
   }
   out_path:
   return segduo<RASrcIter, DstIter>(first, dst_first);
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class SrcIter, class Size, class DstIter, class DstSent, class DstTag, class SrcCat>
typename algo_enable_if_c<!DstTag::value, segduo<SrcIter, DstIter> >::type
segmented_copy_n_dst_bounded
   (SrcIter first, Size& count, DstIter dst_first, DstSent dst_last, DstTag, SrcCat)
{
   for(; count > 0; ++first, --count) {
      if(dst_first == dst_last)
         goto out_path;
      *dst_first = *first;
      ++dst_first;
   }
   out_path:
   return segduo<SrcIter, DstIter>(first, dst_first);
}

template <class SrcIter, class Size, class SegDstIter, class SrcCat>
segduo<SrcIter, SegDstIter> segmented_copy_n_dst_bounded
   (SrcIter first, Size& count, SegDstIter dst_first, SegDstIter dst_last,
    segmented_iterator_tag, SrcCat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   dst_segment_iterator       sfirst = dst_traits::segment(dst_first);
   const dst_segment_iterator slast  = dst_traits::segment(dst_last);

   if(sfirst == slast) {
      segduo<SrcIter, dst_local_iterator> r = (segmented_copy_n_dst_bounded)
         (first, count, dst_traits::local(dst_first), dst_traits::local(dst_last), dst_is_local_seg_t(), SrcCat());
      return segduo<SrcIter, SegDstIter>(r.first, dst_traits::compose(sfirst, r.second));
   }
   else {
      segduo<SrcIter, dst_local_iterator> r = (segmented_copy_n_dst_bounded)
         (first, count, dst_traits::local(dst_first), dst_traits::end(sfirst), dst_is_local_seg_t(), SrcCat());
      first = r.first;
      if(count == 0)
         return segduo<SrcIter, SegDstIter>(first, dst_traits::compose(sfirst, r.second));

      for(++sfirst; sfirst != slast; ++sfirst) {
         r = (segmented_copy_n_dst_bounded)
            (first, count, dst_traits::begin(sfirst), dst_traits::end(sfirst), dst_is_local_seg_t(), SrcCat());
         first = r.first;
         if(count == 0)
            return segduo<SrcIter, SegDstIter>(first, dst_traits::compose(sfirst, r.second));
      }

      r = (segmented_copy_n_dst_bounded)
         (first, count, dst_traits::begin(slast), dst_traits::local(dst_last), dst_is_local_seg_t(), SrcCat());
      return segduo<SrcIter, SegDstIter>(r.first, dst_traits::compose(sfirst, r.second));
   }
}

//////////////////////////////////////////////////////////////////////////////
// Destination dispatch: routes to bounded helper.
// Non-segmented destination: single unbounded call (unreachable_sentinel_t).
// Segmented destination: loop over destination segments, bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Size, class DstIter, class Cat>
BOOST_CONTAINER_FORCEINLINE DstIter segmented_copy_n_dst_dispatch
   (SrcIter first, Size& count, DstIter result,
    const non_segmented_iterator_tag &, Cat)
{
   return (segmented_copy_n_dst_bounded)
      (first, count, result, unreachable_sentinel_t(), non_segmented_iterator_tag(), Cat()).second;
}

template <class SrcIter, class Size, class SegDstIter, class Cat>
SegDstIter segmented_copy_n_dst_dispatch
   (SrcIter first, Size& count, SegDstIter result,
    const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   if(count <= 0)
      return result;

   dst_segment_iterator dst_seg   = dst_traits::segment(result);
   dst_local_iterator   dst_local = dst_traits::local(result);

   while(count > 0) {
      const dst_local_iterator dst_end = dst_traits::end(dst_seg);
      const segduo<SrcIter, dst_local_iterator> r = (segmented_copy_n_dst_bounded)
         (first, count, dst_local, dst_end, dst_is_local_seg_t(), Cat());
      first = r.first;
      if(count > 0) {
         ++dst_seg;
         dst_local = dst_traits::begin(dst_seg);
      }
      else
         dst_local = r.second;
   }
   return dst_traits::compose(dst_seg, dst_local);
}

//////////////////////////////////////////////////////////////////////////////
// copy_n_scan: scans through source segments, clipping to both segment
// boundaries and count, then delegates to the destination dispatch layer.
//////////////////////////////////////////////////////////////////////////////

template <class InIter, class Size, class OutIter>
BOOST_CONTAINER_FORCEINLINE
OutIter copy_n_scan_non_segmented
   (InIter first, InIter last, Size& count, OutIter result, const std::random_access_iterator_tag &)
{
   Size range_sz = Size(last - first);
   Size local_count = count < range_sz ? count : range_sz;
   result = (segmented_copy_n)(first, local_count, result);
   count -= local_count;
   return result;
}

template <class InIter, class Size, class OutIter, class Tag>
OutIter copy_n_scan_non_segmented(InIter first, InIter last, Size& count, OutIter result, Tag)
{
   Size local_count = count;

   for(; local_count > 0 && first != last; ++first, ++result, --local_count)
      *result = *first;

   count = local_count;

   return result;
}

template <class InIter, class Size, class OutIter>
BOOST_CONTAINER_FORCEINLINE
OutIter copy_n_scan(InIter first, InIter last, Size& count, OutIter result, non_segmented_iterator_tag)
{
   return (copy_n_scan_non_segmented)(first, last, count, result, typename iterator_traits<InIter>::iterator_category());
}

template <class SegIt, class Size, class OutIter>
OutIter copy_n_scan(SegIt first, SegIt last, Size& count, OutIter result, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);
   local_iterator   lcur  = traits::local(first);

   if(scur == slast) {
      return copy_n_scan(lcur, traits::local(last), count, result, is_local_seg_t());
   }
   else {
      result = copy_n_scan(lcur, traits::end(scur), count, result, is_local_seg_t());

      if (!count)
         return result;

      for (++scur; scur != slast; ++scur) {
         result = copy_n_scan(traits::begin(scur), traits::end(scur), count, result, is_local_seg_t());
         if (!count)
            return result;
      }

      return copy_n_scan(traits::begin(scur), traits::local(last), count, result, is_local_seg_t());
   }
}

//////////////////////////////////////////////////////////////////////////////
// Source dispatch: walks the source (read pointer) segments
//////////////////////////////////////////////////////////////////////////////

template <class SegIter, class Size, class OutIter, class Cat>
OutIter segmented_copy_n_dispatch
   (SegIter first, Size count, OutIter result, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   if(count <= 0) return result;

   segment_iterator scur = traits::segment(first);
   local_iterator   lcur = traits::local(first);

   while(1) {
      result = copy_n_scan(lcur, traits::end(scur), count, result, is_local_seg_t());

      if(count == 0)
         break;
      ++scur;
      lcur = traits::begin(scur);
   }
   return result;
}

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RAIter, class Size, class OutIter>
BOOST_CONTAINER_FORCEINLINE OutIter segmented_copy_n_dispatch
   (RAIter first, Size count, OutIter result, non_segmented_iterator_tag, const std::random_access_iterator_tag &)
{
#if !defined(BOOST_CONTAINER_DISABLE_MULTI_SEGMENTED_ALGO)
   typedef segmented_iterator_traits<OutIter> dst_traits;
   return (segmented_copy_n_dst_dispatch)
      (first, count, result, typename dst_traits::is_segmented_iterator(), std::random_access_iterator_tag());
#else
   return (segmented_copy_n_dst_dispatch)
      (first, count, result, non_segmented_iterator_tag(), std::random_access_iterator_tag());
#endif
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class InIter, class Size, class OutIter, class Cat>
OutIter segmented_copy_n_dispatch
   (InIter first, Size count, OutIter result, non_segmented_iterator_tag, Cat)
{
#if !defined(BOOST_CONTAINER_DISABLE_MULTI_SEGMENTED_ALGO)
   typedef segmented_iterator_traits<OutIter> dst_traits;
   return (segmented_copy_n_dst_dispatch)
      (first, count, result, typename dst_traits::is_segmented_iterator(), Cat());
#else
   return (segmented_copy_n_dst_dispatch)
      (first, count, result, non_segmented_iterator_tag(), Cat());
#endif
}

} // namespace detail_algo

//! Copies \c count elements from the range beginning at \c first to
//! the range beginning at \c result. Exploits segmentation on both
//! input and output.
template <class InIter, class Size, class OutIter>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_copy_n(InIter first, Size count, OutIter result)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_copy_n_dispatch(first, count, result,
      typename traits::is_segmented_iterator(), typename iterator_traits<InIter>::iterator_category());
}


} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_N_HPP
