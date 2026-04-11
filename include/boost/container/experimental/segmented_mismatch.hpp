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

namespace detail_algo {

struct mismatch_equal
{
   template <class T, class U>
   BOOST_CONTAINER_FORCEINLINE bool operator()(const T& a, const U& b) const { return a == b; }
};

//////////////////////////////////////////////////////////////////////////////
// Bounded iter2 helper: compares source [first1, last1) against
// [iter2_first, iter2_last), stopping when source, iter2, or a mismatch
// is encountered.
// Advances first1_out (by reference) so the caller knows how far we got.
// Advances iter2_first (by reference) for the same reason.
// Recursively walks iter2 segments when iter2 is segmented.
//
// Returns true if no mismatch was found in the bounded region
// (i.e. stopped because first1==last1 or first2==iter2_last).
// Returns false if a mismatch was found.
//
// When iter2_last is unreachable_sentinel_t the segment-boundary check
// is optimised away, giving the same code as an unbounded loop.
//////////////////////////////////////////////////////////////////////////////

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RASrcIter, class Iter2, class Iter2Sent, class BinaryPred>
bool segmented_mismatch_iter2_bounded
   (RASrcIter &first1_out, RASrcIter last1, Iter2 &iter2_first, Iter2Sent iter2_last, BinaryPred pred,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;
   RASrcIter first1 = first1_out;
   Iter2 first2 = iter2_first;

   difference_type n = last1 - first1;

   while(n >= difference_type(4)) {
      if(first2 == iter2_last) goto out_path; if(!pred(*first1, *first2)) goto mismatch_path; ++first1; ++first2;
      if(first2 == iter2_last) goto out_path; if(!pred(*first1, *first2)) goto mismatch_path; ++first1; ++first2;
      if(first2 == iter2_last) goto out_path; if(!pred(*first1, *first2)) goto mismatch_path; ++first1; ++first2;
      if(first2 == iter2_last) goto out_path; if(!pred(*first1, *first2)) goto mismatch_path; ++first1; ++first2;
      n -= 4;
   }

   switch(n) {
      case 3:
         if(first2 == iter2_last) goto out_path; if(!pred(*first1, *first2)) goto mismatch_path; ++first1; ++first2;
         BOOST_FALLTHROUGH;
      case 2:
         if(first2 == iter2_last) goto out_path; if(!pred(*first1, *first2)) goto mismatch_path; ++first1; ++first2;
         BOOST_FALLTHROUGH;
      case 1:
         if(first2 == iter2_last) goto out_path; if(!pred(*first1, *first2)) goto mismatch_path; ++first1; ++first2;
         BOOST_FALLTHROUGH;
      default:
         break;
   }
   out_path:
   first1_out = first1;
   iter2_first = first2;
   return true;

   mismatch_path:
   first1_out = first1;
   iter2_first = first2;
   return false;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class SrcIter, class Sent, class Iter2, class Iter2Sent, class BinaryPred, class Iter2Tag, class SrcCat>
typename algo_enable_if_c<!Iter2Tag::value, bool>::type
segmented_mismatch_iter2_bounded
   (SrcIter &first1_out, Sent last1, Iter2 &iter2_first, Iter2Sent iter2_last, BinaryPred pred, Iter2Tag, SrcCat)
{
   SrcIter first1 = first1_out;
   Iter2 first2 = iter2_first;

   for(; first1 != last1; ++first1) {
      if(first2 == iter2_last)
         goto out_path;
      if(!pred(*first1, *first2)) {
         first1_out = first1;
         iter2_first = first2;
         return false;
      }
      ++first2;
   }
   out_path:
   first1_out = first1;
   iter2_first = first2;
   return true;
}

template <class SrcIter, class Sent, class SegIter2, class BinaryPred, class SrcCat>
bool segmented_mismatch_iter2_bounded
   (SrcIter &first1, Sent last1, SegIter2 &iter2_first_out, SegIter2 iter2_last, BinaryPred pred,
    segmented_iterator_tag, SrcCat)
{
   typedef segmented_iterator_traits<SegIter2>  iter2_traits;
   typedef typename iter2_traits::local_iterator    iter2_local_iterator;
   typedef typename iter2_traits::segment_iterator  iter2_segment_iterator;
   typedef typename segmented_iterator_traits<iter2_local_iterator>::is_segmented_iterator iter2_is_local_seg_t;

   iter2_segment_iterator       sfirst = iter2_traits::segment(iter2_first_out);
   const iter2_segment_iterator slast  = iter2_traits::segment(iter2_last);

   if(sfirst == slast) {
      iter2_local_iterator loc2 = iter2_traits::local(iter2_first_out);
      bool r = (segmented_mismatch_iter2_bounded)
         (first1, last1, loc2, iter2_traits::local(iter2_last), pred, iter2_is_local_seg_t(), SrcCat());
      iter2_first_out = iter2_traits::compose(sfirst, loc2);
      return r;
   }
   else {
      iter2_local_iterator loc2 = iter2_traits::local(iter2_first_out);
      if(!(segmented_mismatch_iter2_bounded)
            (first1, last1, loc2, iter2_traits::end(sfirst), pred, iter2_is_local_seg_t(), SrcCat())) {
         iter2_first_out = iter2_traits::compose(sfirst, loc2);
         return false;
      }
      if(first1 == last1) {
         iter2_first_out = iter2_traits::compose(sfirst, loc2);
         return true;
      }

      for(++sfirst; sfirst != slast; ++sfirst) {
         loc2 = iter2_traits::begin(sfirst);
         if(!(segmented_mismatch_iter2_bounded)
               (first1, last1, loc2, iter2_traits::end(sfirst), pred, iter2_is_local_seg_t(), SrcCat())) {
            iter2_first_out = iter2_traits::compose(sfirst, loc2);
            return false;
         }
         if(first1 == last1) {
            iter2_first_out = iter2_traits::compose(sfirst, loc2);
            return true;
         }
      }

      loc2 = iter2_traits::begin(slast);
      bool r = (segmented_mismatch_iter2_bounded)
         (first1, last1, loc2, iter2_traits::local(iter2_last), pred, iter2_is_local_seg_t(), SrcCat());
      iter2_first_out = iter2_traits::compose(sfirst, loc2);
      return r;
   }
}

//////////////////////////////////////////////////////////////////////////////
// Iter2 dispatch: routes to bounded helper.
// Non-segmented iter2: single unbounded call (unreachable_sentinel_t).
// Segmented iter2: loop over iter2 segments, bounded per segment.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class InpIter2, class BinaryPred, class Cat>
BOOST_CONTAINER_FORCEINLINE bool segmented_mismatch_iter2_dispatch
   (SrcIter &first1, Sent last1, InpIter2 &first2, BinaryPred pred,
    const non_segmented_iterator_tag &, Cat)
{
   return (segmented_mismatch_iter2_bounded)
      (first1, last1, first2, unreachable_sentinel_t(), pred, non_segmented_iterator_tag(), Cat());
}

template <class SrcIter, class Sent, class SegIter2, class BinaryPred, class Cat>
bool segmented_mismatch_iter2_dispatch
   (SrcIter &first1, Sent last1, SegIter2 &first2_out, BinaryPred pred,
    const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegIter2>  iter2_traits;
   typedef typename iter2_traits::local_iterator    iter2_local_iterator;
   typedef typename iter2_traits::segment_iterator  iter2_segment_iterator;
   typedef typename segmented_iterator_traits<iter2_local_iterator>::is_segmented_iterator iter2_is_local_seg_t;

   if(first1 == last1)
      return true;

   iter2_segment_iterator seg2 = iter2_traits::segment(first2_out);
   iter2_local_iterator   loc2 = iter2_traits::local(first2_out);

   while(first1 != last1) {
      iter2_local_iterator end2 = iter2_traits::end(seg2);
      if(!(segmented_mismatch_iter2_bounded)
            (first1, last1, loc2, end2, pred, iter2_is_local_seg_t(), Cat())) {
         first2_out = iter2_traits::compose(seg2, loc2);
         return false;
      }
      if(first1 != last1) {
         ++seg2;
         loc2 = iter2_traits::begin(seg2);
      }
   }
   first2_out = iter2_traits::compose(seg2, loc2);
   return true;
}

//////////////////////////////////////////////////////////////////////////////
// Source dispatch: walks the source (first1) segments.
// Returns std::pair<Iter1, Iter2>.
// Internally uses the iter2 dispatch which returns bool and updates first2
// by reference.  The source dispatch reconstructs the composed first1
// position when a mismatch is found.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIter, class Sent, class InpIter2, class BinaryPred, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c
   < !Tag::value || is_sentinel<Sent, SrcIter>::value
   , std::pair<SrcIter, InpIter2>
   >::type
segmented_mismatch_dispatch(SrcIter first1, Sent last1, InpIter2 first2, BinaryPred pred, Tag, Cat)
{
#if !defined(BOOST_CONTAINER_DISABLE_SEGMENTED_OUTPUT)
   typedef segmented_iterator_traits<InpIter2> iter2_traits;
   bool ok = (segmented_mismatch_iter2_dispatch)
      (first1, last1, first2, pred, typename iter2_traits::is_segmented_iterator(), Cat());
#else
   bool ok = (segmented_mismatch_iter2_dispatch)
      (first1, last1, first2, pred, non_segmented_iterator_tag(), Cat());
#endif
   (void)ok;
   return std::pair<SrcIter, InpIter2>(first1, first2);
}

template <class SegIter, class InpIter2, class BinaryPred, class Cat>
std::pair<SegIter, InpIter2> segmented_mismatch_dispatch
   (SegIter first1, SegIter last1, InpIter2 first2, BinaryPred pred, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>  traits;
   typedef typename traits::local_iterator     local_iterator;
   typedef typename traits::segment_iterator   segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   typedef std::pair<SegIter, InpIter2>        return_t;
   typedef std::pair<local_iterator, InpIter2> local_return_t;

   segment_iterator       sfirst = traits::segment(first1);
   segment_iterator const slast  = traits::segment(last1);

   if(sfirst == slast) {
      const local_iterator ll = traits::local(last1);
      const local_return_t r = (segmented_mismatch_dispatch)(traits::local(first1), ll, first2, pred, is_local_seg_t(), local_cat_t());
      return return_t((r.first != ll) ? traits::compose(sfirst, r.first) : last1, r.second);
   }
   else {
      local_iterator le = traits::end(sfirst);
      local_return_t r = (segmented_mismatch_dispatch)(traits::local(first1), le, first2, pred, is_local_seg_t(), local_cat_t());
      if (r.first != le)
         return return_t(traits::compose(sfirst, r.first), r.second);

      for (++sfirst; sfirst != slast; ++sfirst) {
         le = traits::end(sfirst);
         r = (segmented_mismatch_dispatch)(traits::begin(sfirst), le, r.second, pred, is_local_seg_t(), local_cat_t());
         if (r.first != le)
            return return_t(traits::compose(sfirst, r.first), r.second);
      }

      le = traits::local(last1);
      r = (segmented_mismatch_dispatch)(traits::begin(slast), le, r.second, pred, is_local_seg_t(), local_cat_t());
      return return_t((r.first != le) ? traits::compose(sfirst, r.first) : last1, r.second);
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
   return detail_algo::segmented_mismatch_dispatch
      (first1, last1, first2, pred, typename traits::is_segmented_iterator(), typename iterator_traits<InpIter1>::iterator_category());
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

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MISMATCH_HPP
