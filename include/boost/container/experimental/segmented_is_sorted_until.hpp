//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_IS_SORTED_UNTIL_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_IS_SORTED_UNTIL_HPP

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

namespace boost {
namespace container {

namespace detail_algo {

// Recursive dispatch for `segmented_is_sorted_until`.
//
// Cross-segment state is carried by reference:
//   - `prev`      : iterator (of the deepest non-segmented local type) pointing
//                   at the most recently examined element.  Valid only when
//                   `has_prev` is true.
//   - `has_prev`  : becomes true the first time any element is seen.
//
// The segmented overload is the workhorse: it decomposes a range into
// first / middle / last sub-ranges and dispatches each one back through
// `sorted_until_rec`, which recurses if the local iterator is itself
// segmented, until the non-segmented base case is reached.  `prev` and
// `has_prev` thread through every level of recursion unchanged, which is how
// the "last element of segment K" is compared against "first element of
// segment K+1" even when K and K+1 live at different levels of the nesting.

// (1) Non-segmented base case.
//     At this level FwdIt == DeepIt (the deepest type computed from the top).
template <class FwdIt, class Sent, class Comp, class DeepIt, class Cat>
FwdIt sorted_until_rec
   (FwdIt first, Sent last, Comp comp,
    DeepIt &prev_out, bool &has_prev,
    const non_segmented_iterator_tag &, Cat)
{
   DeepIt prev = prev_out;
   if (!has_prev) {
      if (first == last)
         return first;
      prev = first;
      has_prev = true;
      ++first;
   }
   for (; first != last; ++first) {
      if (comp(*first, *prev))
         goto final_update;
      prev = first;
   }
   final_update:
   prev_out = prev;
   return first;
}

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

// (1') Random-access unrolled variant of the non-segmented base case.
template <class RAIter, class Comp, class DeepIt>
RAIter sorted_until_rec
   (RAIter first, RAIter last, Comp comp,
    DeepIt &prev_out, bool &has_prev,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RAIter>::difference_type difference_type;

   difference_type n = last - first;
   if (n == difference_type(0))
      return last;

   DeepIt prev = prev_out;
   if (!has_prev) {
      prev = first;
      has_prev = true;
      ++first;
      --n;
   }

   while (n >= difference_type(4)) {
      if (comp(*first, *prev))
         goto final_result;
      prev = first; ++first;
      if (comp(*first, *prev))
         goto final_result;
      prev = first; ++first;
      if (comp(*first, *prev))
         goto final_result;
      prev = first; ++first;
      if (comp(*first, *prev))
         goto final_result;
      prev = first; ++first;
      n -= 4;
   }

   switch (n) {
      case 3:
         if (comp(*first, *prev))
            goto final_result;
         prev = first; ++first;
         BOOST_FALLTHROUGH;
      case 2:
         if (comp(*first, *prev))
            goto final_result;
         prev = first; ++first;
         BOOST_FALLTHROUGH;
      case 1:
         if (comp(*first, *prev))
            goto final_result;
         prev = first; ++first;
         BOOST_FALLTHROUGH;
      default:
         break;
   }

   final_result:
   prev_out = prev;
   return first;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

// (2) Segmented iterator with a heterogeneous sentinel as end.
//     Cannot locate `last`'s segment, so falls back to a simple linear loop
//     using a local iterator as previous.  The `DeepIt &` / `has_prev &` are
//     not used here because this overload is only ever invoked at the very
//     top level (no cross-level state to propagate).
template <class SegIter, class Sent, class Comp, class DeepIt, class Cat>
typename algo_enable_if_c<is_sentinel<Sent, SegIter>::value, SegIter>::type
sorted_until_rec
   (SegIter first, Sent last, Comp comp,
    DeepIt &, bool &,
    const segmented_iterator_tag &, Cat)
{
   if (first != last) {
      SegIter prev_local = first;
      for (; ++first != last; ) {
         if (comp(*first, *prev_local))
            return first;
         prev_local = first;
      }
   }
   return first;
}

// (3) Segmented iterator with a matching segmented iterator as end.
//     Recursive: decomposes the range and dispatches each sub-range back to
//     `sorted_until_rec`, so recursive segmentation is exploited at every
//     level.  `prev` / `has_prev` are threaded unchanged.
template <class SegIter, class Comp, class DeepIt, class Cat>
SegIter sorted_until_rec
   (SegIter first, SegIter last, Comp comp,
    DeepIt &prev, bool &has_prev,
    const segmented_iterator_tag &, Cat)
{
   typedef segmented_iterator_traits<SegIter>                             traits;
   typedef typename traits::segment_iterator                              segment_iterator;
   typedef typename traits::local_iterator                                local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator
                                                                          is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category    local_cat_t;

   segment_iterator       sfirst = traits::segment(first);
   const segment_iterator slast  = traits::segment(last);

   if (sfirst == slast) {
      const local_iterator ll = traits::local(last);
      const local_iterator lr = (sorted_until_rec)
         (traits::local(first), ll, comp, prev, has_prev, is_local_seg_t(), local_cat_t());
      if (lr != ll)
         return traits::compose(sfirst, lr);
   }
   else {
      // First segment: from local(first) to end(sfirst)
      {
         const local_iterator le = traits::end(sfirst);
         const local_iterator lr = (sorted_until_rec)
            (traits::local(first), le, comp, prev, has_prev, is_local_seg_t(), local_cat_t());
         if (lr != le)
            return traits::compose(sfirst, lr);
      }
      // Middle segments
      for (++sfirst; sfirst != slast; ++sfirst) {
         const local_iterator le = traits::end(sfirst);
         const local_iterator lr = (sorted_until_rec)
            (traits::begin(sfirst), le, comp, prev, has_prev, is_local_seg_t(), local_cat_t());
         if (lr != le)
            return traits::compose(sfirst, lr);
      }
      // Last segment: from begin(slast) to local(last)
      {
         const local_iterator ll = traits::local(last);
         const local_iterator lr = (sorted_until_rec)
            (traits::begin(slast), ll, comp, prev, has_prev, is_local_seg_t(), local_cat_t());
         if (lr != ll)
            return traits::compose(sfirst, lr);
      }
   }
   return last;
}

} // namespace detail_algo

//! Returns an iterator to the first element that is less than its predecessor
//! in [first, last), using \c comp for comparison.
//! Returns \c last if the range is sorted.
template <class FwdIt, class Sent, class Comp>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_is_sorted_until(FwdIt first, Sent last, Comp comp)
{
   typedef segmented_iterator_traits<FwdIt>                          traits;
   typedef typename detail_algo::deepest_local_iterator<FwdIt>::type deep_it;
   deep_it prev = deep_it();
   bool    has_prev = false;
   return detail_algo::sorted_until_rec
      (first, last, comp, prev, has_prev,
       typename traits::is_segmented_iterator(),
       typename iterator_traits<FwdIt>::iterator_category());
}

//! Returns an iterator to the first element that is less than its predecessor
//! in [first, last), using operator<.
//! Returns \c last if the range is sorted.
template <class FwdIt, class Sent>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_is_sorted_until(FwdIt first, Sent last)
{
   return (segmented_is_sorted_until)(first, last, detail_algo::segmented_default_less());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_IS_SORTED_UNTIL_HPP
