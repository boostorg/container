//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MISMATCH_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MISMATCH_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/iterators.hpp>
#include <utility>

namespace boost {
namespace container {

template <class InpIter1, class Sent, class InpIter2, class BinaryPred>
std::pair<InpIter1, InpIter2> segmented_mismatch(InpIter1 first1, Sent last1, InpIter2 first2, BinaryPred pred);

template <class InpIter1, class Sent, class InpIter2>
std::pair<InpIter1, InpIter2> segmented_mismatch(InpIter1 first1, Sent last1, InpIter2 first2);

template <class InpIter1, class Sent1, class InpIter2, class Sent2, class BinaryPred>
std::pair<InpIter1, InpIter2> segmented_mismatch(InpIter1 first1, Sent1 last1, InpIter2 first2, Sent2 last2, BinaryPred pred);

template <class InpIter1, class Sent1, class InpIter2>
std::pair<InpIter1, InpIter2> segmented_mismatch(InpIter1 first1, Sent1 last1, InpIter2 first2, InpIter2 last2);

namespace detail_algo {

struct mismatch_equal
{
   template <class T, class U>
   BOOST_CONTAINER_FORCEINLINE bool operator()(const T& a, const U& b) const { return a == b; }
};

//////////////////////////////////////////////////////////////////////////////
// Bounded iter2 helper: compares source [first1, last1) against
// [first2, last2), stopping when source, iter2, or a mismatch
// is encountered.
// Returns segtrio<SrcIter, Iter2, bool> with the final positions of both
// iterators and the reason the walk stopped.
// Recursively walks iter2 segments when iter2 is segmented.


template <class SrcIter, class Sent, class Iter2, class Iter2Sent, class BinaryPred, class Iter2Tag, class SrcCat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<!Iter2Tag::value, segtrio<SrcIter, Iter2, bool> >::type
segmented_mismatch_iter2_bounded
   (SrcIter first1, Sent last1, Iter2 first2, Iter2Sent last2, BinaryPred pred, Iter2Tag, SrcCat)
{
   typedef segtrio<SrcIter, Iter2, bool> result_t;

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first1 != last1; ++first1) {
      if(first2 == last2)
         return result_t(first1, first2, false);
      if(!pred(*first1, *first2))
         return result_t(first1, first2, true);
      ++first2;
   }
   return result_t(first1, first2, true);
}

template <class RASrcIter, class RAIter2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE
typename iterator_enable_if_tag
   <RAIter2, std::random_access_iterator_tag, segtrio<RASrcIter, RAIter2, bool> >::type
segmented_mismatch_iter2_bounded
   (RASrcIter first1, RASrcIter last1, RAIter2 first2, RAIter2 last2, BinaryPred pred,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef segtrio<RASrcIter, RAIter2, bool> result_t;
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;
   const difference_type src_n  = last1 - first1;
   const difference_type iter2_n = difference_type(last2 - first2);
   difference_type n = src_n < iter2_n ? src_n : iter2_n;

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   while(n) {
      --n;
      if(!pred(*first1, *first2))
         return result_t(first1, first2, true);
      ++first1;
      ++first2;
   }

   return result_t(first1, first2, first1 == last1);
}

template <class SrcIter, class Sent, class SegIter2, class BinaryPred, class SrcCat>
segtrio<SrcIter, SegIter2, bool> segmented_mismatch_iter2_bounded
   (SrcIter first1, Sent last1, SegIter2 first2, SegIter2 last2, BinaryPred pred,
    segmented_iterator_tag, SrcCat)
{
   typedef segmented_iterator_traits<SegIter2>  iter2_traits;
   typedef typename iter2_traits::local_iterator    iter2_local_iterator;
   typedef typename iter2_traits::segment_iterator  iter2_segment_iterator;
   typedef typename segmented_iterator_traits<iter2_local_iterator>::is_segmented_iterator iter2_is_local_seg_t;

   typedef segtrio<SrcIter, SegIter2, bool>              result_t;
   typedef segtrio<SrcIter, iter2_local_iterator, bool>  local_result_t;

   iter2_segment_iterator       sfirst = iter2_traits::segment(first2);
   const iter2_segment_iterator slast  = iter2_traits::segment(last2);

   iter2_local_iterator lb2 = iter2_traits::local(first2);

   for(;;) {
      const bool last_seg = sfirst == slast;
      const iter2_local_iterator le2 = last_seg ? iter2_traits::local(last2) : iter2_traits::end(sfirst);
      {
         const local_result_t r = (segmented_mismatch_iter2_bounded)
            (first1, last1, lb2, le2, pred, iter2_is_local_seg_t(), SrcCat());
         first1 = r.first;
         if(last_seg || BOOST_CONTAINER_SEG_UNLIKELY(r.third))
            return result_t(first1, iter2_traits::compose(sfirst, r.second), r.third);
      }

      for(++sfirst; sfirst != slast; ++sfirst) {
         const local_result_t r = (segmented_mismatch_iter2_bounded)
            (first1, last1, iter2_traits::begin(sfirst), iter2_traits::end(sfirst), pred
            , iter2_is_local_seg_t(), SrcCat());
         first1 = r.first;
         if(BOOST_CONTAINER_SEG_UNLIKELY(r.third))
            return result_t(first1, iter2_traits::compose(sfirst, r.second), true);
      }

      lb2 = iter2_traits::begin(sfirst);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Iter2 dispatch (segmented iter2 only): loops over iter2 segments,
// bounded per segment. Used as the body of the unbounded-iter2 shim
// of segmented_mismatch_iter2_bounded below.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class SegIter2, class BinaryPred, class Cat>
segduo<SrcIter, SegIter2> segmented_mismatch_iter2_dispatch
   (SrcIter first1, Sent last1, SegIter2 first2, BinaryPred pred,
    const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegIter2>  iter2_traits;
   typedef typename iter2_traits::local_iterator    iter2_local_iterator;
   typedef typename iter2_traits::segment_iterator  iter2_segment_iterator;
   typedef typename segmented_iterator_traits<iter2_local_iterator>::is_segmented_iterator iter2_is_local_seg_t;

   if(BOOST_UNLIKELY(first1 == last1))
      return segduo<SrcIter, SegIter2>(first1, first2);

   iter2_segment_iterator seg2 = iter2_traits::segment(first2);
   iter2_local_iterator   loc2 = iter2_traits::local(first2);

   for(;;) {
      const segtrio<SrcIter, iter2_local_iterator, bool> r = (segmented_mismatch_iter2_bounded)
         (first1, last1, loc2, iter2_traits::end(seg2), pred, iter2_is_local_seg_t(), Cat());
      first1 = r.first;
      loc2   = r.second;
      if(BOOST_CONTAINER_SEG_UNLIKELY(r.third))
         break;
      ++seg2;
      loc2 = iter2_traits::begin(seg2);
   }
   return segduo<SrcIter, SegIter2>(first1, iter2_traits::compose(seg2, loc2));
}

//////////////////////////////////////////////////////////////////////////////
// Shim: segmented_mismatch_iter2_bounded overload for the combination
// "segmented iter2 + unreachable_sentinel_t as last2". The primary
// segmented-iter2 overload (above) requires first2 and last2 to
// share the same type, which prevents passing unreachable_sentinel_t
// directly. This shim forwards to segmented_mismatch_iter2_dispatch, which
// walks iter2 segments without needing a global last2.
//
// With this shim in place, segmented_mismatch_bounded_dispatch can be
// reused as the single implementation for the unbounded case, simply by
// passing unreachable_sentinel_t as last2.
//
// third is always true: an unbounded iter2 can never be the reason the walk
// stopped, so a caller of this overload never has a segment to advance to.
//////////////////////////////////////////////////////////////////////////////
template <class SrcIter, class Sent, class SegIter2, class BinaryPred, class SrcCat>
BOOST_CONTAINER_FORCEINLINE segtrio<SrcIter, SegIter2, bool>
segmented_mismatch_iter2_bounded
   (SrcIter first1, Sent last1, SegIter2 first2, unreachable_sentinel_t,
    BinaryPred pred, segmented_iterator_tag, SrcCat)
{
   const segduo<SrcIter, SegIter2> r = (segmented_mismatch_iter2_dispatch)
      (first1, last1, first2, pred, segmented_iterator_tag(), SrcCat());
   return segtrio<SrcIter, SegIter2, bool>(r.first, r.second, true);
}

//////////////////////////////////////////////////////////////////////////////
// Fully bounded dispatch (two-range mismatch): walks [first1, last1) and
// [first2, last2) in lock-step, recursive on both sides. Stops at the first
// of: mismatch, source exhaustion (first1 == last1), or iter2 exhaustion
// (first2 == last2). Returns segduo{final source, final iter2}.
//
// - The non-segmented-source leaf forwards to segmented_mismatch_iter2_bounded,
//   which already recurses on iter2 segmentation and has the random-access
//   unrolled fast path.  Its stop flag is dropped here: it groups a mismatch
//   with source exhaustion, whereas the segmented-source walk below needs a
//   mismatch grouped with iter2 exhaustion instead.
// - The segmented-source overload mirrors the classic segmented walk but
//   threads last2 through the recursion and exits early when iter2 is
//   exhausted.
//
// This dispatch is also reused to implement the unbounded (three-iterator)
// segmented_mismatch_dispatch by passing unreachable_sentinel_t as last2
// (the shim above handles the segmented-iter2 case for that sentinel).
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class InpIter2, class Sent2, class BinaryPred, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, SrcIter>::value, segduo<SrcIter, InpIter2> >::type
segmented_mismatch_bounded_dispatch
   (SrcIter first1, Sent last1, InpIter2 first2, Sent2 last2, BinaryPred pred, Tag, Cat)
{
   typedef segmented_iterator_traits<InpIter2> iter2_traits;
   const segtrio<SrcIter, InpIter2, bool> r = (segmented_mismatch_iter2_bounded)
      (first1, last1, first2, last2, pred,
       typename iter2_traits::is_segmented_iterator(), Cat());
   return segduo<SrcIter, InpIter2>(r.first, r.second);
}

template <class SegIter, class InpIter2, class Sent2, class BinaryPred, class Cat>
segduo<SegIter, InpIter2> segmented_mismatch_bounded_dispatch
   (SegIter first1, SegIter last1, InpIter2 first2, Sent2 last2,
    BinaryPred pred, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>  traits;
   typedef typename traits::local_iterator     local_iterator;
   typedef typename traits::segment_iterator   segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   typedef segduo<SegIter, InpIter2>           return_t;
   typedef segduo<local_iterator, InpIter2>    local_return_t;

   segment_iterator       sfirst = traits::segment(first1);
   segment_iterator const slast  = traits::segment(last1);

   local_iterator lb = traits::local(first1);

   for (;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = sfirst == slast;
      const local_iterator le = last_seg ? traits::local(last1) : traits::end(sfirst);
      {
         const local_return_t r = (segmented_mismatch_bounded_dispatch)
            (lb, le, first2, last2, pred, is_local_seg_t(), local_cat_t());
         first2 = r.second;
         // Early exit: stopped inside segment (mismatch) or iter2 exhausted.
         if (BOOST_CONTAINER_SEG_UNLIKELY(r.first != le || first2 == last2))
            return return_t(traits::compose(sfirst, r.first), first2);
         if (BOOST_CONTAINER_SEG_UNLIKELY(last_seg))
            return return_t(last1, first2);
      }

      //Middle segments keep their own call site: begin/end both come from
      //sfirst, so the leaf can be specialised for full segments
      for (++sfirst; sfirst != slast; ++sfirst) {
         const local_iterator me = traits::end(sfirst);
         const local_return_t r = (segmented_mismatch_bounded_dispatch)
            (traits::begin(sfirst), me, first2, last2, pred, is_local_seg_t(), local_cat_t());
         first2 = r.second;
         if (BOOST_CONTAINER_SEG_UNLIKELY(r.first != me || first2 == last2))
            return return_t(traits::compose(sfirst, r.first), first2);
      }

      lb = traits::begin(sfirst);
   }
}

} // namespace detail_algo

//! Returns a pair of iterators to the first elements where
//! \c pred(*it1, *it2) is false in [first1, last1) and the range
//! starting at \c first2, or {last1, first2 + N} if all match.
//! Exploits segmentation on both ranges.
template <class InpIter1, class Sent, class InpIter2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE std::pair<InpIter1, InpIter2>
segmented_mismatch(InpIter1 first1, Sent last1, InpIter2 first2, BinaryPred pred)
{
   typedef segmented_iterator_traits<InpIter1> traits;

   segduo<InpIter1, InpIter2> r = (detail_algo::segmented_mismatch_bounded_dispatch)
      ( first1, last1, first2, unreachable_sentinel_t(), pred
      , typename traits::is_segmented_iterator()
      , typename iterator_traits<InpIter1>::iterator_category());
   return std::pair<InpIter1, InpIter2>(r.first, r.second);
}

//! Returns a pair of iterators to the first mismatching elements
//! in [first1, last1) and the range starting at \c first2, or
//! {last1, first2 + N} if all elements match.
//! Exploits segmentation on both ranges.
template <class InpIter1, class Sent, class InpIter2>
BOOST_CONTAINER_FORCEINLINE std::pair<InpIter1, InpIter2>
segmented_mismatch(InpIter1 first1, Sent last1, InpIter2 first2)
{
   return boost::container::segmented_mismatch(first1, last1, first2, detail_algo::mismatch_equal());
}

//! Returns a pair of iterators to the first elements where
//! \c pred(*it1, *it2) is false in [first1, last1) and [first2, last2),
//! or {last1, last2} if all elements in the common prefix match.
//! When the two ranges have different lengths, iteration stops at the end
//! of the shorter range; the returned pair then points to the end of the
//! shorter range and the corresponding position in the other range.
//! Exploits segmentation on both ranges.
template <class InpIter1, class Sent1, class InpIter2, class Sent2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE std::pair<InpIter1, InpIter2>
segmented_mismatch(InpIter1 first1, Sent1 last1, InpIter2 first2, Sent2 last2, BinaryPred pred)
{
   typedef segmented_iterator_traits<InpIter1> traits;
   segduo<InpIter1, InpIter2> r = detail_algo::segmented_mismatch_bounded_dispatch
      (first1, last1, first2, last2, pred,
       typename traits::is_segmented_iterator(),
       typename iterator_traits<InpIter1>::iterator_category());
   return std::pair<InpIter1, InpIter2>(r.first, r.second);
}

//! Returns a pair of iterators to the first mismatching elements in
//! [first1, last1) and [first2, last2), or {last1, last2} if all elements
//! in the common prefix match. When the two ranges have different lengths,
//! iteration stops at the end of the shorter range.
//! Exploits segmentation on both ranges.
//!
//! Note: \c last2 must have the same type as \c first2. To pass a sentinel
//! type for the end of the second range, use the overload with an explicit
//! predicate.
template <class InpIter1, class Sent1, class InpIter2>
BOOST_CONTAINER_FORCEINLINE std::pair<InpIter1, InpIter2>
segmented_mismatch(InpIter1 first1, Sent1 last1, InpIter2 first2, InpIter2 last2)
{
   return boost::container::segmented_mismatch
      (first1, last1, first2, last2, detail_algo::mismatch_equal());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MISMATCH_HPP
