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
#include <boost/container/experimental/detail/segmented_common_algo.hpp>
#include <boost/container/detail/iterators.hpp>
#include <boost/container/detail/mpl.hpp>
#include <boost/container/detail/type_traits.hpp>

namespace boost {
namespace container {

template <class InpIter1, class Sent, class InpIter2, class BinaryPred>
bool segmented_equal(InpIter1 first1, Sent last1, InpIter2 first2, BinaryPred pred);

template <class InpIter1, class Sent, class InpIter2>
bool segmented_equal(InpIter1 first1, Sent last1, InpIter2 first2);

template <class InpIter1, class Sent1, class InpIter2, class Sent2, class BinaryPred>
bool segmented_equal(InpIter1 first1, Sent1 last1, InpIter2 first2, Sent2 last2, BinaryPred pred);

template <class InpIter1, class Sent1, class InpIter2>
bool segmented_equal(InpIter1 first1, Sent1 last1, InpIter2 first2, InpIter2 last2);

namespace detail_algo {

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
   const segtrio<SrcIter, InpIter2, bool> r = (segmented_iter2_bounded)
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
      const segtrio<SrcIter, iter2_local_iterator, bool> r = (segmented_iter2_bounded)
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

//////////////////////////////////////////////////////////////////////////////
// Two-range source dispatch: same walk as segmented_equal_dispatch above but
// with last2 threaded through, so the walk also stops when [first2, last2)
// runs out.  Returns segduo<bool, InpIter2> whose first is true only when the
// source was consumed without a mismatch; the caller still has to check the
// final iter2 against last2 to tell "both ended together" from "the second
// range had elements left over".
//
// Reached only when the two ranges are not both sized; the sized case is
// answered by comparing the two lengths and then reusing the unbounded walk,
// which needs no per-element iter2 bound check at all.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class InpIter2, class Sent2, class BinaryPred, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c
   < !Tag::value || is_sentinel<Sent, SrcIter>::value
   , segduo<bool, InpIter2>
   >::type
segmented_equal_bounded_dispatch
   (SrcIter first1, Sent last1, InpIter2 first2, Sent2 last2, BinaryPred pred, Tag, Cat)
{
   //A true sentinel cannot be decomposed into a segment, so the iter2 walk is
   //downgraded to the flat leaf; sent_filter leaves the natural segmentation
   //in place whenever last2 is an iterator.
   typedef typename sent_filter<InpIter2, Sent2>::seg_t iter2_seg_t;
   const segtrio<SrcIter, InpIter2, bool> r = (segmented_iter2_bounded)
      (first1, last1, first2, last2, pred, iter2_seg_t(), Cat());
   //A mismatch and an exhausted iter2 both leave first1 short of last1, so
   //this single comparison covers every reason the walk could have stopped
   //other than a fully consumed source.
   return segduo<bool, InpIter2>(r.first == last1, r.second);
}

template <class SegIter, class InpIter2, class Sent2, class BinaryPred, class Cat>
segduo<bool, InpIter2> segmented_equal_bounded_dispatch
   (SegIter first1, SegIter last1, InpIter2 first2, Sent2 last2, BinaryPred pred,
    segmented_iterator_tag, Cat)
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
      segduo<bool, InpIter2> r = (segmented_equal_bounded_dispatch)
         (lb, le, first2, last2, pred, is_local_seg_t(), local_cat_t());
      if(last_seg || BOOST_UNLIKELY(!r.first))
         return r;

      //Middle segments keep their own call site: begin/end both come from
      //sfirst, so the leaf can be specialised for full segments
      for(++sfirst; sfirst != slast; ++sfirst) {
         r = (segmented_equal_bounded_dispatch)
            (traits::begin(sfirst), traits::end(sfirst), r.second, last2, pred
            , is_local_seg_t(), local_cat_t());
         if(BOOST_UNLIKELY(!r.first))
            return r;
      }

      lb     = traits::begin(sfirst);
      first2 = r.second;
   }
}

//////////////////////////////////////////////////////////////////////////////
// Two-range entry point, selected at compile time on whether both ranges are
// sized.  "Sized" here means each range is closed by an iterator of its own
// type (not a sentinel) whose category is random access, so that last - first
// is a constant-time expression.
//
// Segmented random-access iterators qualify: bc::deque's operator- is block
// arithmetic, a handful of instructions, which is cheaper than the segment
// decomposition the walk itself would have to do before comparing anything.
//
// The sized form answers unequal lengths without applying the predicate once,
// which is the whole point of the four-argument overload; when the lengths do
// match it drops the iter2 bound entirely and reuses the unbounded walk, so
// the common equal-length case costs the same as the three-argument call.
//////////////////////////////////////////////////////////////////////////////

template <class Iter1, class Sent1, class Iter2, class Sent2>
struct segmented_equal_is_sized
{
   static const bool value =
      dtl::is_same<Iter1, Sent1>::value && dtl::is_same<Iter2, Sent2>::value
      && seg_is_ra_iterator<Iter1>::value && seg_is_ra_iterator<Iter2>::value;
   typedef dtl::bool_<value> type;
};

template <class RAIter1, class RAIter2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE bool segmented_equal_sized_dispatch
   (RAIter1 first1, RAIter1 last1, RAIter2 first2, RAIter2 last2, BinaryPred pred, dtl::true_)
{
   typedef typename iterator_traits<RAIter1>::difference_type difference_type;
   if((last1 - first1) != difference_type(last2 - first2))
      return false;
   return boost::container::segmented_equal(first1, last1, first2, pred);
}

template <class InpIter1, class Sent1, class InpIter2, class Sent2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE bool segmented_equal_sized_dispatch
   (InpIter1 first1, Sent1 last1, InpIter2 first2, Sent2 last2, BinaryPred pred, dtl::false_)
{
   typedef segmented_iterator_traits<InpIter1> traits;
   const segduo<bool, InpIter2> r = (segmented_equal_bounded_dispatch)
      (first1, last1, first2, last2, pred, typename traits::is_segmented_iterator()
      , typename iterator_traits<InpIter1>::iterator_category());
   return r.first && r.second == last2;
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
   return boost::container::segmented_equal
      (first1, last1, first2, detail_algo::segmented_default_equal_to());
}

//! Returns \c true if [first1, last1) and [first2, last2) have the same
//! length and every pair of corresponding elements satisfies \c pred.
//! Ranges of different lengths compare unequal.
//! Exploits segmentation on both ranges.
//!
//! When both ranges are closed by a random-access iterator of their own type
//! the two lengths are compared first, so unequal lengths are reported
//! without applying \c pred at all.  Otherwise \c pred is applied at most
//! \c min(last1 - first1, last2 - first2) times.
template <class InpIter1, class Sent1, class InpIter2, class Sent2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE
bool segmented_equal(InpIter1 first1, Sent1 last1, InpIter2 first2, Sent2 last2, BinaryPred pred)
{
   typedef detail_algo::segmented_equal_is_sized<InpIter1, Sent1, InpIter2, Sent2> sized_t;
   return detail_algo::segmented_equal_sized_dispatch
      (first1, last1, first2, last2, pred, typename sized_t::type());
}

//! Returns \c true if [first1, last1) and [first2, last2) have the same
//! length and compare equal element by element.
//! Ranges of different lengths compare unequal.
//! Exploits segmentation on both ranges.
//!
//! Note: \c last2 must have the same type as \c first2. To pass a sentinel
//! type for the end of the second range, use the overload with an explicit
//! predicate.
template <class InpIter1, class Sent1, class InpIter2>
BOOST_CONTAINER_FORCEINLINE
bool segmented_equal(InpIter1 first1, Sent1 last1, InpIter2 first2, InpIter2 last2)
{
   return boost::container::segmented_equal
      (first1, last1, first2, last2, detail_algo::segmented_default_equal_to());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_EQUAL_HPP
