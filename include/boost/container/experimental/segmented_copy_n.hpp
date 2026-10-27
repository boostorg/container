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
#include <boost/container/experimental/detail/segmented_common_algo.hpp>
#include <cstddef>
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

template <class SrcIter, class SrcSent, class DstIter, class DstSent>
BOOST_CONTAINER_FORCEINLINE segtrio<SrcIter, DstIter, std::size_t>
segmented_copy_n_src_dst_bounded
   (SrcIter first, SrcSent last, std::size_t n, DstIter dst_first, DstSent dst_last)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   while(n) {
      if(BOOST_UNLIKELY(dst_first == dst_last || first == last))
         break;
      *dst_first = *first;
      ++dst_first;
      ++first;
      --n;
   }

   return segtrio<SrcIter, DstIter, std::size_t>(first, dst_first, n);
}

#if BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS
template <class SrcIter, class SrcSent, class DstIter, class DstSent>
BOOST_CONTAINER_FORCEINLINE
   typename iterator_disable_if_tag
      <DstIter, std::random_access_iterator_tag, segtrio<SrcIter, DstIter, std::size_t> >::type
segmented_copy_n_dst_bounded
   (SrcIter first, SrcSent last, std::size_t count, DstIter dst_first, DstSent dst_last, const non_segmented_iterator_tag &)
{
   return (segmented_copy_n_src_dst_bounded)(first, last, count, dst_first, dst_last);
}

template <class RASrcIter, class SrcSent, class RADstIter>
BOOST_CONTAINER_FORCEINLINE
typename iterator_enable_if_tag
   <RADstIter, std::random_access_iterator_tag, segtrio<RASrcIter, RADstIter, std::size_t> >::type
segmented_copy_n_dst_bounded
   (RASrcIter first, SrcSent last, std::size_t count, RADstIter dst_first, RADstIter dst_last, const non_segmented_iterator_tag &)
{
   //count and the segment length are std::size_t (see segmented_copy_n): no truncation
   const std::size_t dst_n = static_cast<std::size_t>(dst_last - dst_first);
   const std::size_t n = count < dst_n ? count : dst_n;
   //The inner helper may exit early if first == last (bounded src), so
   //count is adjusted by the number of elements actually written.
   const segtrio<RASrcIter, RADstIter, std::size_t> r = (segmented_copy_n_src_dst_bounded)
      (first, last, n, dst_first, unreachable_sentinel_t());
   return segtrio<RASrcIter, RADstIter, std::size_t>(r.first, r.second, count - (n - r.third));
}
#else
template <class SrcIter, class SrcSent, class DstIter, class DstSent>
BOOST_CONTAINER_FORCEINLINE segtrio<SrcIter, DstIter, std::size_t>
segmented_copy_n_dst_bounded
   (SrcIter first, SrcSent last, std::size_t count, DstIter dst_first, DstSent dst_last, const non_segmented_iterator_tag &)
{
   return (segmented_copy_n_src_dst_bounded)(first, last, count, dst_first, dst_last);
}
#endif   // BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS

template <class SrcIter, class SrcSent, class SegDstIter>
segtrio<SrcIter, SegDstIter, std::size_t> segmented_copy_n_dst_bounded
   (SrcIter first, SrcSent last, std::size_t count, SegDstIter dst_first, SegDstIter dst_last, const segmented_iterator_tag &)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;
   typedef segtrio<SrcIter, dst_local_iterator, std::size_t> local_result_t;
   typedef segtrio<SrcIter, SegDstIter, std::size_t>         result_t;

   dst_segment_iterator       sdfirst = dst_traits::segment(dst_first);
   const dst_segment_iterator sdlast  = dst_traits::segment(dst_last);

   dst_local_iterator db = dst_traits::local(dst_first);

   for(;;) {
      const bool last_seg = sdfirst == sdlast;
      const dst_local_iterator de = last_seg ? dst_traits::local(dst_last) : dst_traits::end(sdfirst);
      {
         const local_result_t r = (segmented_copy_n_dst_bounded)
            (first, last, count, db, de, dst_is_local_seg_t());
         first = r.first;
         count = r.third;
         if (last_seg || BOOST_UNLIKELY(count == 0 || first == last))
            return result_t(first, dst_traits::compose(sdfirst, r.second), count);
      }

      for (++sdfirst; sdfirst != sdlast; ++sdfirst) {
         const local_result_t r = (segmented_copy_n_dst_bounded)
            (first, last, count, dst_traits::begin(sdfirst), dst_traits::end(sdfirst), dst_is_local_seg_t());
         first = r.first;
         count = r.third;
         if (BOOST_UNLIKELY(count == 0 || first == last))
            return result_t(first, dst_traits::compose(sdfirst, r.second), count);
      }

      db = dst_traits::begin(sdfirst);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Destination dispatch: routes to bounded helper and returns the destination
// position and the count of elements not copied.
// Non-segmented destination: single unbounded call (unreachable_sentinel_t).
// Segmented destination: loop over destination segments, bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class SrcSent, class DstIter>
BOOST_CONTAINER_FORCEINLINE segduo<DstIter, std::size_t> segmented_copy_n_dst_dispatch
   (SrcIter first, SrcSent last, std::size_t count, DstIter result, const non_segmented_iterator_tag &)
{
   const segtrio<SrcIter, DstIter, std::size_t> r = (segmented_copy_n_src_dst_bounded)
      (first, last, count, result, unreachable_sentinel_t());
   return segduo<DstIter, std::size_t>(r.second, r.third);
}

template <class SrcIter, class SrcSent, class SegDstIter>
segduo<SegDstIter, std::size_t> segmented_copy_n_dst_dispatch
   (SrcIter first, SrcSent last, std::size_t count, SegDstIter result, const segmented_iterator_tag &)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;

   if(BOOST_UNLIKELY(!count))
      return segduo<SegDstIter, std::size_t>(result, count);

   dst_segment_iterator dst_seg   = dst_traits::segment(result);
   dst_local_iterator   dst_local = dst_traits::local(result);

   while(1) {
      const segtrio<SrcIter, dst_local_iterator, std::size_t> r = (segmented_copy_n_dst_bounded)
         (first, last, count, dst_local, dst_traits::end(dst_seg), dst_is_local_seg_t());
      first = r.first;
      count = r.third;
      //Exit when count is satisfied or the source range is exhausted.
      //The src-exhaustion check matters when this routine is invoked with
      //a per-segment src range (count typically larger than the range);
      //it returns control to the outer source-segment loop.
      if(BOOST_UNLIKELY(count == 0 || first == last))
         return segduo<SegDstIter, std::size_t>(dst_traits::compose(dst_seg, r.second), count);
      ++dst_seg;
      dst_local = dst_traits::begin(dst_seg);
   }
}

//////////////////////////////////////////////////////////////////////////////
// copy_n_scan: scans through source segments, clipping to both segment
// boundaries and count, then delegates to the destination dispatch layer.
// Returns the destination position and the count of elements not copied.
//////////////////////////////////////////////////////////////////////////////

#if BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS
template <class InIter, class OutIter>
BOOST_CONTAINER_FORCEINLINE
segduo<OutIter, std::size_t> copy_n_scan
   ( InIter first, InIter last, std::size_t count
   , OutIter result, non_segmented_iterator_tag, const std::random_access_iterator_tag &)
{
   //count and the segment length are std::size_t (see segmented_copy_n): no truncation
   const std::size_t range_sz = static_cast<std::size_t>(last - first);
   const std::size_t min_count = count <= range_sz ? count : range_sz;
   typedef segmented_iterator_traits<OutIter> dst_traits;
   //The source is not bounded, so all min_count elements are copied
   result = (segmented_copy_n_dst_dispatch)
      (first, unreachable_sentinel_t(), min_count, result, typename dst_traits::is_segmented_iterator()).first;
   return segduo<OutIter, std::size_t>(result, count - min_count);
}
#endif   // BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS

template <class InIter, class OutIter, class Cat>
BOOST_CONTAINER_FORCEINLINE
segduo<OutIter, std::size_t> copy_n_scan
   (InIter first, InIter last, std::size_t count, OutIter result, non_segmented_iterator_tag, const Cat &)
{
   typedef segmented_iterator_traits<OutIter> dst_traits;
   return (segmented_copy_n_dst_dispatch)
      (first, last, count, result, typename dst_traits::is_segmented_iterator());
}

template <class SegIt, class OutIter, class Cat>
segduo<OutIter, std::size_t> copy_n_scan
   (SegIt first, SegIt last, std::size_t count, OutIter result, segmented_iterator_tag, const Cat &)
{
   typedef segmented_iterator_traits<SegIt>                                            traits;
   typedef typename traits::local_iterator                                             local_iterator;
   typedef typename traits::segment_iterator                                           segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator   is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category                 local_cat_t;
   typedef segduo<OutIter, std::size_t>                                                result_t;

   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);
   local_iterator   lcur  = traits::local(first);

   for(;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = scur == slast;
      const local_iterator le = last_seg ? traits::local(last) : traits::end(scur);
      {
         const result_t r = copy_n_scan(lcur, le, count, result, is_local_seg_t(), local_cat_t());
         result = r.first;
         count  = r.second;
      }

      if (last_seg || BOOST_UNLIKELY(!count))
         return result_t(result, count);

      //Middle segments keep their own call site: begin/end both come from
      //scur, so the leaf can be specialised for full segments
      for (++scur; scur != slast; ++scur) {
         const result_t r = copy_n_scan(traits::begin(scur), traits::end(scur), count, result, is_local_seg_t(), local_cat_t());
         result = r.first;
         count  = r.second;
         if (BOOST_UNLIKELY(!count))
            return result_t(result, count);
      }

      lcur = traits::begin(scur);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Source dispatch: walks the source (read pointer) segments
//////////////////////////////////////////////////////////////////////////////

template <class SegIter, class OutIter, class Cat>
OutIter segmented_copy_n_dispatch
   (SegIter first, std::size_t count, OutIter result, const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   if(BOOST_UNLIKELY(!count)) return result;

   segment_iterator scur = traits::segment(first);
   local_iterator   lcur = traits::local(first);

   while(1) {
      const segduo<OutIter, std::size_t> r = copy_n_scan(lcur, traits::end(scur), count, result, is_local_seg_t(), local_cat_t());
      result = r.first;
      count  = r.second;

      if(BOOST_UNLIKELY(count == 0))
         break;
      ++scur;
      lcur = traits::begin(scur);
   }
   return result;
}

template <class InIter, class OutIter, class Cat>
BOOST_CONTAINER_FORCEINLINE OutIter segmented_copy_n_dispatch
   (InIter first, std::size_t count, OutIter result, const non_segmented_iterator_tag &, const Cat&)
{
   typedef segmented_iterator_traits<OutIter> dst_traits;
   return (segmented_copy_n_dst_dispatch)
      (first, unreachable_sentinel_t(), count, result, typename dst_traits::is_segmented_iterator()).first;
}

} // namespace detail_algo

//! Copies \c count elements from the range beginning at \c first to
//! the range beginning at \c result. If \c count is not positive, does
//! nothing and returns \c result. Exploits segmentation on both
//! input and output.
template <class InIter, class Size, class OutIter>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_copy_n(InIter first, Size count, OutIter result)
{
   typedef segmented_iterator_traits<InIter> traits;
   if (BOOST_UNLIKELY(!(count > 0)))
      return result;
   //A valid count is not bigger than the range, so it fits in std::size_t. The internal
   //functions use only std::size_t: a segment length converted to a narrower Size is truncated.
   return detail_algo::segmented_copy_n_dispatch(first, static_cast<std::size_t>(count), result,
      typename traits::is_segmented_iterator(), typename iterator_traits<InIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_N_HPP
