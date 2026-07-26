//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SET_UNION_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SET_UNION_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_copy.hpp>
#include <boost/container/detail/iterator.hpp>
#include <cstddef>

namespace boost {
namespace container {

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter, class Comp>
OutIter segmented_set_union
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp);

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter>
OutIter segmented_set_union
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result);

namespace detail_algo {

//////////////////////////////////////////////////////////////////////////////
// set_union_dst_bounded: leaf kernel that unions [first1, last1) and
// [first2, last2) into [dst_first, dst_last), stopping when source 1,
// source 2, or destination is exhausted.  When dst_last is
// unreachable_sentinel_t the destination-full check is optimised away.
// No residue draining is performed; the caller handles that.
//////////////////////////////////////////////////////////////////////////////

template <class Iter1, class Sent1, class Iter2, class Sent2, class DstIter, class DstSent,
          class Comp, class DstTag, class SrcCat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<!DstTag::value, segtrio<Iter1, Iter2, DstIter> >::type
set_union_dst_bounded
   (Iter1 first1, Sent1 last1, Iter2 first2, Sent2 last2,
    DstIter dst_first, DstSent dst_last, Comp comp, DstTag, SrcCat)
{
   while(first1 != last1 && first2 != last2 && dst_first != dst_last) {
      if      (comp(*first1, *first2)) { *dst_first = *first1;  ++first1;  }
      else if (comp(*first2, *first1)) { *dst_first = *first2; ++first2; }
      else                             { *dst_first = *first1;  ++first1; ++first2; }
      ++dst_first;
   }
   return segtrio<Iter1, Iter2, DstIter>(first1, first2, dst_first);
}

template <std::size_t BlockSize, class RAIter1, class RAIter2, class DstIter,
          class DstSent, class Comp>
BOOST_CONTAINER_FORCEINLINE segtrio<RAIter1, RAIter2, DstIter>
set_union_blocks
   (RAIter1 first1, RAIter1 last1, RAIter2 first2, RAIter2 last2,
    DstIter dst_first, DstSent dst_last, Comp comp)
{
   typedef typename iterator_traits<RAIter1>::difference_type difference_type;
   const difference_type block_size = static_cast<difference_type>(BlockSize);
   while( (seg_srcs_dst_room_at_least)
            (first1, last1, first2, last2, dst_first, dst_last, block_size)) {
      BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL
      for(difference_type chunk = block_size; chunk; ) {
         --chunk;
         if      (comp(*first1, *first2)) { *dst_first = *first1;  ++first1;  }
         else if (comp(*first2, *first1)) { *dst_first = *first2; ++first2; }
         else                             { *dst_first = *first1;  ++first1; ++first2; }
         ++dst_first;
      }
   }
   return segtrio<RAIter1, RAIter2, DstIter>(first1, first2, dst_first);
}

template <class RAIter1, class RAIter2, class DstIter, class DstSent,
          class Comp, class DstTag>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c
   < !DstTag::value && seg_is_ra_iterator<RAIter2>::value
      && seg_is_ra_iterator<DstIter>::value
   , segtrio<RAIter1, RAIter2, DstIter> >::type
set_union_dst_bounded
   (RAIter1 first1, RAIter1 last1, RAIter2 first2, RAIter2 last2,
    DstIter dst_first, DstSent dst_last, Comp comp, DstTag dst_tag,
    const std::random_access_iterator_tag &)
{
   segtrio<RAIter1, RAIter2, DstIter> r = (set_union_blocks<32>)
      (first1, last1, first2, last2, dst_first, dst_last, comp);
   return (set_union_dst_bounded)
      (r.first, last1, r.second, last2, r.third, dst_last, comp, dst_tag, int());
}

//////////////////////////////////////////////////////////////////////////////
// set_union_until_exhausts: writes the union into result until src1 or src2
// is exhausted.  No residue draining.
//
// Non-segmented destination: single bounded call with unreachable_sentinel.
// Segmented destination: walk dst segments, calling dst_bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <class Iter1, class Sent1, class Iter2, class Sent2, class DstIter,
          class Comp, class Cat, class Tag>
BOOST_CONTAINER_FORCEINLINE
segtrio<Iter1, Iter2, DstIter> set_union_until_exhausts
   (Iter1 first1, Sent1 last1, Iter2 first2, Sent2 last2, DstIter result, Comp comp,
    const Tag &, const Cat &src1_cat)
{
   return (set_union_dst_bounded)
      (first1, last1, first2, last2, result, unreachable_sentinel_t(),
       comp, non_segmented_iterator_tag(), src1_cat);
}

template <class Iter1, class Sent1, class Iter2, class Sent2, class SegDstIter,
          class Comp, class Cat>
segtrio<Iter1, Iter2, SegDstIter> set_union_until_exhausts
   (Iter1 first1, Sent1 last1, Iter2 first2, Sent2 last2, SegDstIter result, Comp comp,
    const segmented_iterator_tag &, const Cat &src1_cat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;
   typedef segtrio<Iter1, Iter2, dst_local_iterator>  bounded_t;
   typedef segtrio<Iter1, Iter2, SegDstIter>          result_t;

   if(first1 == last1 || first2 == last2)
      return result_t(first1, first2, result);

   dst_segment_iterator dst_seg   = dst_traits::segment(result);
   dst_local_iterator   dst_local = dst_traits::local(result);

   while(1) {
      const dst_local_iterator dst_end = dst_traits::end(dst_seg);
      const bounded_t r = (set_union_dst_bounded)
         (first1, last1, first2, last2, dst_local, dst_end, comp,
          dst_is_local_seg_t(), src1_cat);
      first1    = r.first;
      first2    = r.second;
      dst_local = r.third;

      // Stop on source exhaustion, not on "the segment did not fill": when
      // the output ends exactly on a segment boundary both hold at once, and
      // stepping dst_seg then walks off the end of the destination.  compose()
      // normalises a local iterator sitting on the segment end, the same way
      // segmented_copy_dst_dispatch relies on.
      if(BOOST_CONTAINER_SEG_UNLIKELY(first1 == last1 || first2 == last2)) {
         return result_t(first1, first2, dst_traits::compose(dst_seg, dst_local));
      }
      // dst segment full and both sources still live; advance to the next.
      ++dst_seg;
      dst_local = dst_traits::begin(dst_seg);
   }
}

//////////////////////////////////////////////////////////////////////////////
// set_union_seg2_dispatch: exploits segmentation of range 2.
//
// Non-segmented src2: dispatches on output segmentation (guarded).
// Segmented src2: walks src2 segments, recurses on local src2.
//////////////////////////////////////////////////////////////////////////////

template <class Iter1, class Sent1, class Iter2, class Sent2, class OutIter, class Comp, class Cat>
BOOST_CONTAINER_FORCEINLINE segtrio<Iter1, Iter2, OutIter> set_union_seg2_dispatch
   (Iter1 first1, Sent1 last1, Iter2 first2, Sent2 last2, OutIter result, Comp comp,
    non_segmented_iterator_tag, const Cat& src1_cat)
{
   typedef segmented_iterator_traits<OutIter>  out_traits;
   typedef typename out_traits::is_segmented_iterator is_out_seg_t;
   return (set_union_until_exhausts)
      (first1, last1, first2, last2, result, comp, is_out_seg_t(), src1_cat);
}

template <class Iter1, class Sent1, class SegIter2, class OutIter, class Comp, class Cat>
segtrio<Iter1, SegIter2, OutIter> set_union_seg2_dispatch
   (Iter1 first1, Sent1 last1, SegIter2 first2, SegIter2 last2, OutIter result, Comp comp,
    segmented_iterator_tag, const Cat & cat)
{
   typedef segmented_iterator_traits<SegIter2>          src2_traits;
   typedef typename src2_traits::local_iterator         src2_local_iterator;
   typedef typename src2_traits::segment_iterator       src2_segment_iterator;
   typedef typename segmented_iterator_traits
      <src2_local_iterator>::is_segmented_iterator      src2_is_local_seg_t;
   typedef segtrio<Iter1, src2_local_iterator, OutIter> local_result_t;
   typedef segtrio<Iter1, SegIter2, OutIter>            result_t;

   if(first1 == last1 || first2 == last2)
      return result_t(first1, first2, result);

   src2_segment_iterator       sf2 = src2_traits::segment(first2);
   const src2_segment_iterator sl2 = src2_traits::segment(last2);
   src2_local_iterator         lf2 = src2_traits::local(first2);

   if(BOOST_CONTAINER_SEG_LIKELY(sf2 != sl2)) {
      {
         const local_result_t r = (set_union_seg2_dispatch)
            (first1, last1, lf2, src2_traits::end(sf2), result, comp,
             src2_is_local_seg_t(), cat);
         first1 = r.first;
         result = r.third;
         if (BOOST_CONTAINER_SEG_UNLIKELY(first1 == last1))
            return result_t(first1, src2_traits::compose(sf2, r.second), result);
      }

      for(++sf2; sf2 != sl2; ++sf2) {
         const local_result_t r = (set_union_seg2_dispatch)
            (first1, last1, src2_traits::begin(sf2), src2_traits::end(sf2),
             result, comp, src2_is_local_seg_t(), cat);
         first1 = r.first;
         result = r.third;
         if(BOOST_CONTAINER_SEG_UNLIKELY(first1 == last1))
            return result_t(first1, src2_traits::compose(sf2, r.second), result);
      }

      lf2 = src2_traits::begin(sl2);
   }
   const local_result_t r = (set_union_seg2_dispatch)
      (first1, last1, lf2, src2_traits::local(last2), result, comp,
       src2_is_local_seg_t(), cat);
   return result_t(r.first, src2_traits::compose(sf2, r.second), r.third);
}

//////////////////////////////////////////////////////////////////////////////
// set_union_scan: exploits segmentation of range 1.
//
// Non-segmented src1: dispatches on src2 segmentation (guarded).
// Segmented src1: walks src1 segments, recurses on local src1.
//////////////////////////////////////////////////////////////////////////////

template <class FwdIt, class Sent, class InIter2, class Sent2, class OutIter, class Comp>
BOOST_CONTAINER_FORCEINLINE segtrio<FwdIt, InIter2, OutIter> set_union_scan
   (FwdIt first1, Sent last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp,
    non_segmented_iterator_tag)
{
   typedef sent_filter<FwdIt, Sent> sf1;
   typedef sent_filter<InIter2, Sent2> sf2;

   return (set_union_seg2_dispatch)
      (first1, last1, first2, last2, result, comp,
       typename sf2::seg_t(),
       typename sf1::cat_t());
}

template <class SegIt, class InIter2, class Sent2, class OutIter, class Comp>
segtrio<SegIt, InIter2, OutIter> set_union_scan
   (SegIt first, SegIt last, InIter2 first2, Sent2 last2, OutIter result, Comp comp,
    segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt>   traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef segtrio<local_iterator, InIter2, OutIter> local_result_t;
   typedef segtrio<SegIt, InIter2, OutIter>          result_t;

   if(first == last || first2 == last2)
      return result_t(first, first2, result);

   segment_iterator       scur  = traits::segment(first);
   segment_iterator const slast = traits::segment(last);
   local_iterator         lcur  = traits::local(first);

   if(BOOST_CONTAINER_SEG_LIKELY(scur != slast)) {
      {
         const local_result_t r = set_union_scan
            (lcur, traits::end(scur), first2, last2, result, comp, is_local_seg_t());
         first2 = r.second;
         result = r.third;
         if(BOOST_CONTAINER_SEG_UNLIKELY(first2 == last2))
            return result_t(traits::compose(scur, r.first), first2, result);
      }

      for(++scur; scur != slast; ++scur) {
         const local_result_t r = set_union_scan
            (traits::begin(scur), traits::end(scur), first2, last2, result, comp, is_local_seg_t());
         first2 = r.second;
         result = r.third;
         if(BOOST_CONTAINER_SEG_UNLIKELY(first2 == last2))
            return result_t(traits::compose(scur, r.first), first2, result);
      }

      lcur = traits::begin(slast);
   }
   const local_result_t r = set_union_scan
      (lcur, traits::local(last), first2, last2, result, comp, is_local_seg_t());
   return result_t(traits::compose(scur, r.first), r.second, r.third);
}

} // namespace detail_algo

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter, class Comp>
inline OutIter segmented_set_union
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp)
{
   typedef detail_algo::sent_filter<InIter1, Sent1> sf;
   segtrio<InIter1, InIter2, OutIter> r = detail_algo::set_union_scan
      (first1, last1, first2, last2, result, comp, typename sf::seg_t());
   // Drain both source residues; at most one of these is non-empty.
   result = r.third;
   if(r.first != last1)
      result = (segmented_copy)(r.first, last1, result);
   return (r.second == last2) ? result : (segmented_copy)(r.second, last2, result);
}

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter>
inline OutIter segmented_set_union
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result)
{
   return boost::container::segmented_set_union
      (first1, last1, first2, last2, result, detail_algo::segmented_default_less());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SET_UNION_HPP
