//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_N_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_N_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_find.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Sent, class Size, class T>
FwdIt segmented_search_n
   (FwdIt first, Sent last, Size count, const T& BOOST_RESTRICT value);

namespace detail_algo {

// search_n_scan_segment scans a single (sub-)segment for a run of `count`
// consecutive elements equal to `value`, threading cross-segment state
// through the `consecutive` parameter. The result is a segtrio:
//
//   first  : true if the full run was completed inside this scan
//   second : current consecutive count at end of scan (carries to next segment)
//   third  : start of the run when first==true; otherwise the position where
//            a new run started inside this scan, or `lend` (sentinel) when
//            no new run started in this scan (i.e. the in-progress run, if
//            any, started in a previous segment).
//
// Used both as the recursive worker driving segmented walks and as the
// single entry point invoked by segmented_search_n.

// (non-segmented, random-access, same-typed iterators)
// Boyer-Moore-style "skip-by-count" scan with cross-segment carry-over.
//
// Phase 1: if `consecutive` carries a partial run from the previous
//   segment, probe at `lcur + ncheck - 1` (the last position needed to
//   complete the carry-over, clamped by the segment) and verify
//   backward on a hit. On success return; on a mismatch reset
//   `consecutive` to 0 and fall through to phase 2 from the position
//   past the mismatch -- with `consecutive > 0`, no in-segment run can
//   start at or before the probe, so the entire range up to the probe
//   can be skipped without inspection on a probe miss.
// Phase 2: probe at `lcur + (count - 1)`. On match, verify backward to
//   find the run start; on mismatch advance the probe by `count`
//   positions instead of 1 (Boyer-Moore). The mismatched element cannot
//   belong to any complete run that fits in [probe - count + 1, probe],
//   so the next candidate run can only start past it. This makes the
//   scan O(n / count) on the value-sparse portion of the range.
// Phase 3: once `probe >= lend`, scan the tail to detect a partial
//   trailing run that the next segment may extend. Bounded by phase 2's
//   exit position: `*(probe - count) != value` is known, so the scan
//   range is (probe - count, lend) of length `lend - probe + count - 1`,
//   which is in [0, count - 1] -- often well below count - 1, and
//   exactly 0 when the last probe landed at `lend - 1` and missed.
template <class LocalIter, class Size, class T>
segtrio<bool, Size, LocalIter> search_n_scan_segment
   (LocalIter lcur, LocalIter lend,
    Size consecutive, Size count, const T& BOOST_RESTRICT value,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef segtrio<bool, Size, LocalIter> result_t;
   typedef typename iterator_traits<LocalIter>::difference_type difference_type;

   LocalIter match_start = lend;

   // Phase 1: extend a partial run carried from the previous segment.
   //
   // Probe at `lcur + ncheck - 1` (the last position the carry-over needs to
   // cover) and, on match, verify backward to `lcur`. This lets us discard
   // the entire `[lcur, probe - 1]` range on a single mismatch at `probe`:
   //
   // - When ncheck == need (segment can host the completion), every run that
   //   could complete the carry-over covers position `probe`, and so do all
   //   in-segment runs starting at `s in [lcur, probe]` (because the smallest
   //   in-segment start whose run does NOT cover `probe` would be at
   //   `s = probe - count + 1 = lcur - consecutive`, which is impossible
   //   when consecutive > 0). So `*probe != value` rules them all out and we
   //   may jump `lcur` to `probe + 1` without inspecting `[lcur_old, probe - 1]`.
   //
   // - When ncheck == avail < need (segment too short to complete), `probe`
   //   is `lend - 1`, and `*probe != value` makes any phase-3 trailing run
   //   length 0 (the very last element is non-matching), which is what
   //   forward Phase 1 would have observed anyway.
   if(consecutive > 0) {
      const difference_type need   = static_cast<difference_type>(count - consecutive);
      const difference_type avail  = lend - lcur;
      const difference_type ncheck = need < avail ? need : avail;
      if(ncheck == 0)
         // Empty segment; carry-over preserved for the next one.
         return result_t(false, consecutive, match_start);
      const LocalIter probe = lcur + (ncheck - 1);
      if(*probe == value) {
         // Backward verify ncheck - 1 positions before probe.
         LocalIter back = probe;
         Size left = static_cast<Size>(ncheck - 1);
         BOOST_CONTAINER_UNROLL(4)
         while(left > 0) {
            --back;
            if(!(*back == value)) {
               // Mismatch at `back`. Carry-over broken; reset and let
               // phase 2 resume from the position past `back`. Using
               // `++back` (then `lcur = back`) instead of `back + 1`
               // lets iterator types with a fast in-step increment
               // avoid the generic `it + n` arithmetic path.
               consecutive = 0;
               ++back;
               lcur = back;
               goto mismatch;
            }
            --left;
         }
         // Full backward verify: every position in [lcur, probe] matches.
         consecutive = static_cast<Size>(consecutive + ncheck);
         return result_t(consecutive == count, consecutive, match_start);
      }
      else {
         // *probe != value. Skip past the entire check range without
         // inspecting [lcur, probe - 1] (justification above).
         // `lcur = probe; ++lcur` instead of `probe + 1` for the same
         // increment-vs-arithmetic reason as above.
         consecutive = 0;
         lcur = probe;
         ++lcur;
      }
      mismatch:   ;
   }

   const difference_type dcount    = static_cast<difference_type>(count);
   const difference_type remaining = lend - lcur;

   // Phase 3 bound. Default: scan the whole segment from `lcur`. This is
   // only used when phase 2 is skipped because the segment is shorter than
   // `count`; otherwise phase 2 tightens it further on exit.
   difference_type tail_max = (dcount - 1) < remaining ? (dcount - 1) : remaining;

   // Phase 2: skip-by-count probing -- only meaningful if a full run fits.
   //
   // Driven by a running probe iterator, advanced in place by `dcount` on
   // a miss and by `left` after a backward-verify mismatch. For chunked
   // local iterators (e.g. raw pointer into a deque chunk) the in-chunk
   // advance collapses to a single register add, with chunk-boundary work
   // amortized over chunk_size/count probes; an index-based form
   // `probe = lcur + pidx` would force the chunk-base load on the critical
   // path of every probe.
   if(dcount <= remaining) {
      // dcount <= remaining => probe = lcur + (dcount - 1) is in [lcur, lend),
      // so the loop body always executes at least once: use do/while.
      LocalIter probe = lcur + (dcount - 1);
      do {
         if(*probe == value) {
            // Backward verify count - 1 elements before probe.
            LocalIter back = probe;
            Size left = static_cast<Size>(count - 1);
            BOOST_CONTAINER_UNROLL(4)
            do {  //count is always >= 2 because it's tested in the outer function
               --back;
               if(!(*back == value))
                  goto back_mismatch;
               --left;
            } while (left > 0);
            match_start = back;
            return result_t(true, count, match_start);

            // Mismatch at `back`; smallest possible new candidate start is
            // back + 1, so next probe is back + count. With back at
            // probe - (count - left), advancing by `left` lands the running
            // probe at the next candidate's verification position.
            back_mismatch:
            probe += static_cast<difference_type>(left);
            continue;
         }
         else {
            probe += dcount;
         }
      } while(probe < lend);
      // Phase-2 exit invariant: the last formed probe was at `probe - dcount`
      // (no-match branch) or at `probe - count` (back-mismatch branch); both
      // carry a confirmed `*!= value`. Therefore no partial trailing run can
      // start at or before that position, and phase 3 only needs to scan a
      // tail of length `(lend - probe) + (dcount - 1)`, which is always in
      // [0, count - 1].
      tail_max = (lend - probe) + (dcount - 1);
   }

   // Phase 3: partial trailing run that could extend into the next segment.
   {
      const LocalIter tail_lo = lend - tail_max;
      LocalIter       tail    = lend;
      BOOST_CONTAINER_UNROLL(4)
      while(tail != tail_lo) {
         --tail;
         if (!(*tail == value)) {
            ++tail;
            break;
         }
      }
      const Size tail_run = static_cast<Size>(lend - tail);
      if(tail_run > 0)
         match_start = tail;
      return result_t(false, tail_run, match_start);
   }
}

// Initial value for `match_start` used by the forward scan below.
//
// When invoked with same-typed iterators (the recursive segmented caller's
// path) we return `lend` so the caller can detect "no new run started in
// this scan" via `r.third != lend`. With a real sentinel this trick isn't
// available (Sent cannot be assigned to LocalIter), but it isn't needed
// either: that path is reached only at the top level, where the caller only
// consults `r.third` when `r.first == true`, in which case `match_start`
// has been set to a real position inside the loop.
template <class LocalIter>
BOOST_CONTAINER_FORCEINLINE
LocalIter search_n_init_match_start(LocalIter, LocalIter lend) { return lend; }

template <class LocalIter, class Sent>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<is_sentinel<Sent, LocalIter>::value, LocalIter>::type
search_n_init_match_start(LocalIter lcur, Sent) { return lcur; }

// Forward scan: handles both the same-typed (recursive segmented caller) and
// the real-sentinel (top-level) cases with one body. The only behavioral
// difference -- the initial value of `match_start` -- is folded into the
// search_n_init_match_start helper above.
template <class LocalIter, class Sent, class Size, class T, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<!Tag::value || is_sentinel<Sent, LocalIter>::value,
                          segtrio<bool, Size, LocalIter> >::type
search_n_scan_segment
   (LocalIter lcur, Sent lend,
    Size consecutive, Size count, const T& BOOST_RESTRICT value, Tag, Cat)
{
   typedef segtrio<bool, Size, LocalIter> result_t;

   LocalIter match_start = search_n_init_match_start(lcur, lend);

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; lcur != lend; ++lcur) {
      if(*lcur == value) {
         if(consecutive == 0)
            match_start = lcur;
         ++consecutive;
         if(consecutive == count)
            return result_t(true, consecutive, match_start);
      }
      else {
         consecutive = 0;
      }
   }
   return result_t(false, consecutive, match_start);
}

// (segmented, same-typed iterators)
// Walks sub-segments and recursively delegates to search_n_scan_segment so
// that every level of segmentation is exploited and the innermost level
// gets the RA fast path.  Cross-segment state (consecutive count and
// match_start) is threaded through the segtrio result.
template <class SegIter, class Size, class T, class Cat>
segtrio<bool, Size, SegIter> search_n_scan_segment
   (SegIter first, SegIter last,
    Size consecutive, Size count, const T& BOOST_RESTRICT value,
    segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>     traits;
   typedef typename traits::segment_iterator      segment_iterator;
   typedef typename traits::local_iterator        local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   typedef segtrio<bool, Size, local_iterator>  sub_result_t;
   typedef segtrio<bool, Size, SegIter>         result_t;

   SegIter match_start = last;

   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);

   if(scur == slast) {
      local_iterator lend = traits::local(last);
      const sub_result_t r = search_n_scan_segment(traits::local(first), lend,
                               consecutive, count, value, is_local_seg_t(), local_cat_t());
      if(r.third != lend)
         match_start = traits::compose(scur, r.third);
      return result_t(r.first, r.second, match_start);
   }
   else {
      local_iterator lend = traits::end(scur);
      sub_result_t r = search_n_scan_segment(traits::local(first), lend,
                               consecutive, count, value, is_local_seg_t(), local_cat_t());
      if(r.third != lend)
         match_start = traits::compose(scur, r.third);
      if(r.first)
         return result_t(true, r.second, match_start);

      for(++scur; scur != slast; ++scur) {
         lend = traits::end(scur);
         r = search_n_scan_segment(traits::begin(scur), lend,
                               r.second, count, value, is_local_seg_t(), local_cat_t());
         if(r.third != lend)
            match_start = traits::compose(scur, r.third);
         if(r.first)
            return result_t(true, r.second, match_start);
      }
      {
         lend = traits::local(last);
         r = search_n_scan_segment(traits::begin(scur), lend,
                               r.second, count, value, is_local_seg_t(), local_cat_t());
         if(r.third != lend)
            match_start = traits::compose(scur, r.third);
         return result_t(r.first, r.second, match_start);
      }
   }
}

// Cheap O(1) size guard available when first/last are a same-typed RA pair.
template <class FwdIt, class Size>
BOOST_CONTAINER_FORCEINLINE
bool search_n_range_shorter_than
   (FwdIt first, FwdIt last, Size count, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<FwdIt>::difference_type difference_type;
   return (last - first) < static_cast<difference_type>(count);
}

template <class FwdIt, class Sent, class Size, class Cat>
BOOST_CONTAINER_FORCEINLINE
bool search_n_range_shorter_than
   (FwdIt, Sent, Size, Cat)
{
   return false;
}

// Top-level dispatch for the count >= 2 case.
//
// Generic fallback: delegates to the recursive walker via search_n_scan_segment
// and unwraps the segtrio.
template <class FwdIt, class Sent, class Size, class T, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
FwdIt search_n_top_dispatch
   (FwdIt first, Sent last, Size count, const T& BOOST_RESTRICT value, Tag tag, Cat cat)
{
   const segtrio<bool, Size, FwdIt> r = search_n_scan_segment
      (first, last, (Size)0, count, value, tag, cat);
   if (r.first)
      return r.third;
   return last;
}

// Top-level non-segmented RA fast path. Phase 1 (carry-over from a previous
// segment) is dead because `consecutive == 0` at the top level. Phase 3
// (trailing partial run that the next segment may extend) is dead because
// there is no next segment. Reduces to pure Boyer-Moore skip-by-count
// probing, returning the iterator directly without going through the
// segtrio packaging. Picked over the generic overload by partial ordering
// when the iterator is non-segmented, RA, and same-typed (FwdIt == Sent).
//
// Driven by a running probe iterator advanced in place: for chunked
// iterators (e.g. wrapped deque_iterator) this collapses to a single
// register add inside a chunk, with chunk-boundary work amortized over
// chunk_size/count probes -- vastly cheaper than recomputing
// `first + pidx` (which forces a chunk-base load on the critical path of
// every probe).
template <class FwdIt, class Size, class T>
BOOST_CONTAINER_FORCEINLINE
FwdIt search_n_top_dispatch
   (FwdIt first, FwdIt last, Size count, const T& BOOST_RESTRICT value,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<FwdIt>::difference_type difference_type;
   const difference_type dcount = static_cast<difference_type>(count);
   // dcount <= remaining guaranteed by the search_n_range_shorter_than guard.
   FwdIt probe = first + (dcount - 1);
   do {
      if (*probe == value) {
         FwdIt back = probe;
         Size left = static_cast<Size>(count - 1);
         BOOST_CONTAINER_UNROLL(4)
         do {
            --back;
            if (!(*back == value))
               goto back_mismatch;
            --left;
         } while (left > 0);
         return back;
         back_mismatch:
         probe += static_cast<difference_type>(left);
         continue;
      }
      else {
         probe += dcount;
      }
   } while (probe < last);
   return last;
}

} // namespace detail_algo

//! Finds the first occurrence of \c count consecutive elements equal to \c value
//! in [first, last). Returns an iterator to the start of the run, or \c last if not found.
//!
//! Trivial counts (<= 0 or == 1) are handled directly. Otherwise an O(1) size
//! guard is applied when available, then the work is delegated to
//! search_n_scan_segment with consecutive=0. Partial ordering routes to the
//! right overload by tag (segmented walker, non-seg RA skip-by-count, or
//! forward/sentinel scan), and the segtrio is unwrapped to the public
//! iterator return.
template <class FwdIt, class Sent, class Size, class T>
inline FwdIt segmented_search_n
   (FwdIt first, Sent last, Size count, const T& BOOST_RESTRICT value)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   typedef typename traits::is_segmented_iterator              is_seg_t;
   typedef typename iterator_traits<FwdIt>::iterator_category  cat_t;

   if (BOOST_UNLIKELY(count <= 0))
      return first;
   else if (BOOST_UNLIKELY(detail_algo::search_n_range_shorter_than(first, last, count, cat_t())))
      return last;   
   else if (count == 1) {
      typedef segmented_iterator_traits<FwdIt> traits;
      return detail_algo::segmented_find_dispatch(first, last, value, is_seg_t(), cat_t());
   }
   else {
      // count >= 2. Tag-dispatched: non-segmented RA top-level calls take a Phase-2-only
      // fast path (Phase 1 is dead because `consecutive == 0` and Phase 3
      // is dead because there is no next segment). Otherwise (segmented,
      // forward-only, sentinel) the generic overload delegates to
      // search_n_scan_segment + segtrio unwrap.
      return detail_algo::search_n_top_dispatch(first, last, count, value, is_seg_t(), cat_t());
   }
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_N_HPP
