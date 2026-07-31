//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_EQUAL_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_EQUAL_HPP

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

namespace boost {
namespace container {

template <class InpIter1, class Sent, class InpIter2, class BinaryPred>
bool segmented_equal(InpIter1 first1, Sent last1, InpIter2 first2, BinaryPred pred);

template <class InpIter1, class Sent, class InpIter2>
bool segmented_equal(InpIter1 first1, Sent last1, InpIter2 first2);

namespace detail_algo {

struct equal_pred
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
//
// third is true when the walk ended for a reason the caller cannot resume
// from (a mismatch was found, or the source was consumed), and false when
// only [first2, last2) ran out.  That last case is the only one in which a
// segmented caller advances to its next segment and calls again, so one flag
// is enough to drive the segment loop.  Reporting it here spares the caller
// the two comparisons it would otherwise need to re-derive it, which are the
// very comparisons this helper has just made.
//
// When last2 is unreachable_sentinel_t the segment-boundary check is
// optimised away, giving the same code as an unbounded loop, and third folds
// to a constant true.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class Iter2, class Iter2Sent, class BinaryPred, class Iter2Tag, class SrcCat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<!Iter2Tag::value, segtrio<SrcIter, Iter2, bool> >::type
segmented_equal_iter2_bounded
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
segmented_equal_iter2_bounded
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
segtrio<SrcIter, SegIter2, bool> segmented_equal_iter2_bounded
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
         const local_result_t r = (segmented_equal_iter2_bounded)
            (first1, last1, lb2, le2, pred, iter2_is_local_seg_t(), SrcCat());
         first1 = r.first;
         if(last_seg || BOOST_UNLIKELY(r.third))
            return result_t(first1, iter2_traits::compose(sfirst, r.second), r.third);
      }

      for(++sfirst; sfirst != slast; ++sfirst) {
         const local_result_t r = (segmented_equal_iter2_bounded)
            (first1, last1, iter2_traits::begin(sfirst), iter2_traits::end(sfirst), pred
            , iter2_is_local_seg_t(), SrcCat());
         first1 = r.first;
         if(BOOST_UNLIKELY(r.third))
            return result_t(first1, iter2_traits::compose(sfirst, r.second), true);
      }

      lb2 = iter2_traits::begin(sfirst);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Iter2 dispatch: routes to bounded helper.
// Non-segmented iter2: single unbounded call (unreachable_sentinel_t).
// Segmented iter2: loop over iter2 segments, bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class InpIter2, class BinaryPred, class Cat>
BOOST_CONTAINER_FORCEINLINE segduo<bool, InpIter2> segmented_equal_iter2_dispatch
   (SrcIter first1, Sent last1, InpIter2 first2, BinaryPred pred,
    const non_segmented_iterator_tag &, Cat)
{
   //r.third is a constant true for an unbounded iter2, so the result still
   //comes from the source position.
   const segtrio<SrcIter, InpIter2, bool> r = (segmented_equal_iter2_bounded)
      (first1, last1, first2, unreachable_sentinel_t(), pred, non_segmented_iterator_tag(), Cat());
   return segduo<bool, InpIter2>(r.first == last1, r.second);
}

template <class SrcIter, class Sent, class SegIter2, class BinaryPred, class Cat>
segduo<bool, SegIter2> segmented_equal_iter2_dispatch
   (SrcIter first1, Sent last1, SegIter2 first2, BinaryPred pred,
    const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegIter2>  iter2_traits;
   typedef typename iter2_traits::local_iterator    iter2_local_iterator;
   typedef typename iter2_traits::segment_iterator  iter2_segment_iterator;
   typedef typename segmented_iterator_traits<iter2_local_iterator>::is_segmented_iterator iter2_is_local_seg_t;

   if(BOOST_UNLIKELY(first1 == last1))
      return segduo<bool, SegIter2>(true, first2);

   iter2_segment_iterator seg2 = iter2_traits::segment(first2);
   iter2_local_iterator   loc2 = iter2_traits::local(first2);

   for(;;) {
      const segtrio<SrcIter, iter2_local_iterator, bool> r = (segmented_equal_iter2_bounded)
         (first1, last1, loc2, iter2_traits::end(seg2), pred, iter2_is_local_seg_t(), Cat());
      first1 = r.first;
      loc2   = r.second;
      if(BOOST_UNLIKELY(r.third))
         break;
      ++seg2;
      loc2 = iter2_traits::begin(seg2);
   }
   //A mismatch leaves first1 on the offending element, so only an exhausted
   //source means every element compared equal.
   return segduo<bool, SegIter2>(first1 == last1, iter2_traits::compose(seg2, loc2));
}

//////////////////////////////////////////////////////////////////////////////
// Source dispatch: walks the source (first1) segments
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class InpIter2, class BinaryPred, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c
   < !Tag::value || is_sentinel<Sent, SrcIter>::value
   , segduo<bool, InpIter2>
   >::type
segmented_equal_dispatch(SrcIter first1, Sent last1, InpIter2 first2, BinaryPred pred, Tag, Cat)
{
   typedef segmented_iterator_traits<InpIter2> iter2_traits;
   return (segmented_equal_iter2_dispatch)
      (first1, last1, first2, pred, typename iter2_traits::is_segmented_iterator(), Cat());
}

template <class SegIter, class InpIter2, class BinaryPred, class Cat>
segduo<bool, InpIter2> segmented_equal_dispatch
   (SegIter first1, SegIter last1, InpIter2 first2, BinaryPred pred, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>  traits;
   typedef typename traits::local_iterator     local_iterator;
   typedef typename traits::segment_iterator   segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   segment_iterator       sfirst = traits::segment(first1);
   segment_iterator const slast  = traits::segment(last1);

   local_iterator lb = traits::local(first1);

   for(;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = sfirst == slast;
      const local_iterator le = last_seg ? traits::local(last1) : traits::end(sfirst);
      // NOT converted to the scoped-const form: InpIter2 is a full segmented
      // iterator here, so carrying it through `first2` costs a copy per
      // iteration that threading `r.second` straight into the next call
      // avoids.  Measured +4.1% on this walker under GCC against -0.2% under
      // Clang, so direct threading stays.
      segduo<bool, InpIter2> r = (segmented_equal_dispatch)
         (lb, le, first2, pred, is_local_seg_t(), local_cat_t());
      if(last_seg || BOOST_UNLIKELY(!r.first))
         return r;

      //Middle segments keep their own call site: begin/end both come from
      //sfirst, so the leaf can be specialised for full segments
      for(++sfirst; sfirst != slast; ++sfirst) {
         r = (segmented_equal_dispatch)
            (traits::begin(sfirst), traits::end(sfirst), r.second, pred, is_local_seg_t(), local_cat_t());
         if(BOOST_UNLIKELY(!r.first))
            return r;
      }

      lb     = traits::begin(sfirst);
      first2 = r.second;
   }
}

} // namespace detail_algo

//! Returns \c true if elements in [first1, last1) are equal to the
//! range starting at \c first2 according to \c pred.
//! Exploits segmentation on both ranges.
template <class InpIter1, class Sent, class InpIter2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE
bool segmented_equal(InpIter1 first1, Sent last1, InpIter2 first2, BinaryPred pred)
{
   typedef segmented_iterator_traits<InpIter1> traits;
   return detail_algo::segmented_equal_dispatch
      (first1, last1, first2, pred, typename traits::is_segmented_iterator(), typename iterator_traits<InpIter1>::iterator_category()).first;
}

//! Returns \c true if elements in [first1, last1) are equal to the
//! range starting at \c first2. Exploits segmentation on both ranges.
template <class InpIter1, class Sent, class InpIter2>
BOOST_CONTAINER_FORCEINLINE
bool segmented_equal(InpIter1 first1, Sent last1, InpIter2 first2)
{
   return boost::container::segmented_equal(first1, last1, first2, detail_algo::equal_pred());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_EQUAL_HPP
