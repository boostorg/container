//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_END_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_END_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
#include <boost/container/experimental/segmented_find_last_if.hpp>

namespace boost {
namespace container {

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2, class BinaryPred>
FwdIt1 segmented_find_end
   (FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last, BinaryPred pred);

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
FwdIt1 segmented_find_end
   (FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last);

namespace detail_algo {

struct find_end_equal
{
   template <class T, class U>
   BOOST_CONTAINER_FORCEINLINE bool operator()(const T& a, const U& b) const { return a == b; }
};

//! Proxy-safe "pred(x, *it)" adaptor.  The needle iterator is held by value
//! and re-dereferenced on every application, so a prvalue proxy returned by
//! operator*() never outlives the comparison it was built for.
template <class Iter, class BinaryPred>
struct find_end_pred_deref
{
   Iter it_;
   BinaryPred pred_;

   BOOST_CONTAINER_FORCEINLINE find_end_pred_deref(Iter it, BinaryPred p)
      : it_(it), pred_(p)
   {}

   template <class U>
   BOOST_CONTAINER_FORCEINLINE bool operator()(const U &u) const
   {  return pred_(u, *it_);   }
};

//////////////////////////////////////////////////////////////////////////////
// Candidate verification: walks [it, last) against [s_it, s_last) in
// lock-step and stops at the first of mismatch, source exhaustion or needle
// exhaustion.  Returns segduo{final source, final needle}; the caller reads a
// full match off second == s_last.
//
// The searched range is walked segment-wise, the needle flat: only the former
// is a segmented container in practice, and the needle is at most as long as
// the part of a candidate still to be checked, so recursing on it would cost
// more set-up than it saves.
//////////////////////////////////////////////////////////////////////////////

template <class FwdIt1, class FwdIt2, class Sent2, class BinaryPred, class Tag>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<!Tag::value, segtrio<FwdIt1, FwdIt2, bool> >::type
find_end_verify
   (FwdIt1 it, FwdIt1 last, FwdIt2 s_it, Sent2 s_last, BinaryPred pred, Tag)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; it != last; ++it) {
      if(s_it == s_last)
         break;
      if(!pred(*it, *s_it))
         break;
      ++s_it;
   }
   return segtrio<FwdIt1, FwdIt2, bool>(it, s_it, it != last || s_it == s_last);
}

template <class SegIt, class FwdIt2, class Sent2, class BinaryPred>
segtrio<SegIt, FwdIt2, bool> find_end_verify
   (SegIt it, SegIt last, FwdIt2 s_it, Sent2 s_last, BinaryPred pred, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   typedef segtrio<SegIt, FwdIt2, bool>          return_t;
   typedef segtrio<local_iterator, FwdIt2, bool> local_return_t;

   segment_iterator       sfirst = traits::segment(it);
   const segment_iterator slast  = traits::segment(last);

   local_iterator lb = traits::local(it);

   for(;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = sfirst == slast;
      const local_iterator le = last_seg ? traits::local(last) : traits::end(sfirst);
      {
         const local_return_t r = (find_end_verify)(lb, le, s_it, s_last, pred, is_local_seg_t());
         s_it = r.second;
         if(BOOST_UNLIKELY(r.third))
            return return_t(traits::compose(sfirst, r.first), s_it, true);
         if(BOOST_UNLIKELY(last_seg))
            return return_t(last, s_it, false);
      }

      //Middle segments keep their own call site: begin/end both come from
      //sfirst, so the leaf can be specialised for full segments
      for(++sfirst; sfirst != slast; ++sfirst) {
         const local_iterator me = traits::end(sfirst);
         const local_return_t r = (find_end_verify)
            (traits::begin(sfirst), me, s_it, s_last, pred, is_local_seg_t());
         s_it = r.second;
         if(BOOST_UNLIKELY(r.third))
            return return_t(traits::compose(sfirst, r.first), s_it, true);
      }

      lb = traits::begin(sfirst);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Forward scan: no way back, so every candidate start is visited in order and
// the last one that verifies wins.  Both bounds the scan needs are produced
// by the same lock-step walk, which also materialises the end of the range
// when it was given as a sentinel.
//////////////////////////////////////////////////////////////////////////////

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2, class BinaryPred>
FwdIt1 find_end_scan
   (FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last, BinaryPred pred,
    const std::forward_iterator_tag&)
{
   typedef typename segmented_iterator_traits<FwdIt1>::is_segmented_iterator tag_t;

   FwdIt1 lead = first;

   FwdIt2 s_next = s_first;
   if(BOOST_UNLIKELY(s_next == s_last)) {
      //An empty needle occurs at the end of the range, not at its start:
      //[alg.find.end] returns last1 where [alg.search] returns first1
      while(lead != last)
         ++lead;
      return lead;
   }
   ++s_next;

   //lead runs one needle length ahead of hi, so hi ends up one past the last
   //position the needle still fits at.  Positions after it are never tested,
   //which is what keeps the total inside the S * (N - S + 1) applications
   //[alg.find.end] allows.
   {
      FwdIt2 s_it = s_next;
      for(; s_it != s_last; ++s_it) {
         if(lead == last)
            return lead;
         ++lead;
      }
   }

   FwdIt1 hi = first;
   while(lead != last) {
      ++lead;
      ++hi;
   }
   const FwdIt1 real_last = lead;

   find_end_pred_deref<FwdIt2, BinaryPred> eq(s_first, pred);

   FwdIt1 result = real_last;
   FwdIt1 cur    = first;

   for(;;) {
      cur = boost::container::segmented_find_if(cur, hi, eq);
      if(cur == hi)
         return result;

      if(s_next == s_last) {
         result = cur;              //one-element needle -> already matched
      }
      else {
         FwdIt1 it = cur;
         ++it;
         const segtrio<FwdIt1, FwdIt2, bool> r = (find_end_verify)
            (it, real_last, s_next, s_last, pred, tag_t());
         if(r.second == s_last)
            result = cur;
      }
      ++cur;
   }
}

//////////////////////////////////////////////////////////////////////////////
// Bidirectional scan: candidate starts are visited from the end, so the first
// one that verifies is the answer and everything before it is left untouched.
//////////////////////////////////////////////////////////////////////////////

template <class FwdIt1, class FwdIt2, class Sent2, class BinaryPred>
FwdIt1 find_end_scan
   (FwdIt1 first, FwdIt1 last, FwdIt2 s_first, Sent2 s_last, BinaryPred pred,
    const std::bidirectional_iterator_tag&)
{
   typedef typename segmented_iterator_traits<FwdIt1>::is_segmented_iterator tag_t;

   FwdIt2 s_next = s_first;
   if(BOOST_UNLIKELY(s_next == s_last))
      return last;
   ++s_next;

   //Stepping back one needle length puts hi one past the last position the
   //needle still fits at.  The tail is never scanned, which is what keeps the
   //total inside the S * (N - S + 1) applications [alg.find.end] allows.
   FwdIt1 hi = last;
   {
      FwdIt2 s_it = s_next;
      for(; s_it != s_last; ++s_it) {
         if(hi == first)
            return last;
         --hi;
      }
   }

   find_end_pred_deref<FwdIt2, BinaryPred> eq(s_first, pred);

   for(;;) {
      const FwdIt1 cand = boost::container::segmented_find_last_if(first, hi, eq);
      if(cand == hi)
         return last;

      if(s_next == s_last)
         return cand;               //one-element needle -> already matched

      FwdIt1 it = cand;
      ++it;
      const segtrio<FwdIt1, FwdIt2, bool> r = (find_end_verify)
         (it, last, s_next, s_last, pred, tag_t());
      if(r.second == s_last)
         return cand;

      //Rejected candidates are dropped from the scanned range, so no start
      //position is ever looked at twice.
      hi = cand;
   }
}

} // namespace detail_algo

//! Finds the last occurrence of the subsequence [s_first, s_last) in
//! [first, last), comparing elements with \c pred.
//! Returns an iterator to the beginning of the found subsequence, or \c last
//! if there is none.  An empty subsequence yields \c last.
//! Exploits segmentation of the searched range.
//! For bidirectional iterators the candidate starts are scanned backwards, so
//! the search stops at the first one that verifies.
template <class FwdIt1, class Sent1, class FwdIt2, class Sent2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE
FwdIt1 segmented_find_end
   (FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last, BinaryPred pred)
{
   typedef detail_algo::sent_filter<FwdIt1, Sent1> sf;
   return detail_algo::find_end_scan
      (first, last, s_first, s_last, pred, typename sf::cat_t());
}

//! Finds the last occurrence of the subsequence [s_first, s_last) in
//! [first, last).
//! Returns an iterator to the beginning of the found subsequence, or \c last
//! if there is none.  An empty subsequence yields \c last.
//! Exploits segmentation of the searched range.
template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
BOOST_CONTAINER_FORCEINLINE
FwdIt1 segmented_find_end(FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last)
{
   return boost::container::segmented_find_end
      (first, last, s_first, s_last, detail_algo::find_end_equal());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_END_HPP
