//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
//segmented_find_if hands its dispatch a value-initialised iterator category,
//and std_fwd.hpp only forward-declares those, so the candidate scan needs the
//tags complete even though nothing here names one.
#include <iterator>

namespace boost {
namespace container {

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
FwdIt1 segmented_search
   (FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last);

namespace detail_algo {

//////////////////////////////////////////////////////////////////////////////
// Find-then-verify.  segmented_find_if locates every candidate (it walks the
// haystack recursively and reaches the unrolled leaves), and the verify below
// checks the rest of the needle at each candidate.
//
// The verify advances the haystack cursor one element at a time and recurses
// over the needle's segments.  A counted walk over the haystack instead would
// have to produce the remaining haystack length, the remaining needle length
// and their minimum before the first comparison; almost every candidate dies
// on its second element, so that prologue is paid in full for a comparison it
// never reaches.  Leaving it out also keeps the candidate loop small, which
// is what the surrounding segmented_find_if scan is sensitive to.
//
// equal_to_deref keeps the search proxy-safe: *s_first is re-evaluated on
// every comparison, so a prvalue proxy never outlives its call.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Verify leaf: lock-step walk of [it, last) against the non-segmented needle
// range [s_it, s_last).  Returns the haystack cursor plus whether the needle
// range was consumed; the cursor is what tells a false start (cursor still
// inside the haystack) from an exhausted haystack.
//
// Entered with it != last and s_it != s_last, so the comparison comes first
// and the two bound tests only run after an element has matched.
//////////////////////////////////////////////////////////////////////////////
template <class FwdIt1, class Sent1, class FwdIt2, class Sent2, class NdlTag>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c
   <!NdlTag::value || is_sentinel<Sent2, FwdIt2>::value, segduo<FwdIt1, bool> >::type
segmented_search_verify(FwdIt1 it, Sent1 last, FwdIt2 s_it, Sent2 s_last, NdlTag)
{
   for(;;) {
      if(!(*it == *s_it))
         return segduo<FwdIt1, bool>(it, false);
      ++it;
      ++s_it;
      if(s_it == s_last)
         return segduo<FwdIt1, bool>(it, true);
      if(it == last)
         return segduo<FwdIt1, bool>(it, false);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Segmented needle: walk the needle's segments, feeding each local range to
// the verify above, which recurses when the local iterator is itself
// segmented.  The haystack cursor is threaded through untouched, so a
// haystack of any segmentation depth is handled by the leaf.
//
// A needle segment can be empty and the haystack can run out between two
// needle segments, so the haystack bound is tested right before each call
// rather than after it: only a call that still has needle left to compare
// turns an exhausted haystack into a rejected candidate.
//////////////////////////////////////////////////////////////////////////////
template <class FwdIt1, class Sent1, class SegIt2>
segduo<FwdIt1, bool> segmented_search_verify
   (FwdIt1 it, Sent1 last, SegIt2 s_it, SegIt2 s_last, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt2>       ndl_traits;
   typedef typename ndl_traits::local_iterator     local_iterator;
   typedef typename ndl_traits::segment_iterator   segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef segduo<FwdIt1, bool>                    result_t;

   segment_iterator       sfirst = ndl_traits::segment(s_it);
   const segment_iterator slast  = ndl_traits::segment(s_last);

   local_iterator lb = ndl_traits::local(s_it);

   for(;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = sfirst == slast;
      const local_iterator le = last_seg ? ndl_traits::local(s_last) : ndl_traits::end(sfirst);
      if(lb != le) {
         if(BOOST_UNLIKELY(it == last))
            return result_t(it, false);
         const result_t r = (segmented_search_verify)(it, last, lb, le, is_local_seg_t());
         if(!r.second)
            return r;
         it = r.first;
      }
      if(BOOST_UNLIKELY(last_seg))
         return result_t(it, true);

      //Middle segments keep their own call site: begin/end both come from
      //sfirst, so the leaf can be specialised for full segments
      for(++sfirst; sfirst != slast; ++sfirst) {
         const local_iterator mb = ndl_traits::begin(sfirst);
         const local_iterator me = ndl_traits::end(sfirst);
         if(mb == me)
            continue;
         if(BOOST_UNLIKELY(it == last))
            return result_t(it, false);
         const result_t r = (segmented_search_verify)(it, last, mb, me, is_local_seg_t());
         if(!r.second)
            return r;
         it = r.first;
      }

      lb = ndl_traits::begin(sfirst);
   }
}

//////////////////////////////////////////////////////////////////////////////
// True when both ranges are random access and closed by an iterator of their
// own type, which is what the clamped dispatch below needs to measure them.
//
// seg_is_ra_iterator is not usable here: it asks is_convertible about the
// category tags, and that needs them complete, whereas this header must stay
// compilable against the forward declarations of std_fwd.hpp alone.
// iterator_enable_if_tag only compares tag types.
//////////////////////////////////////////////////////////////////////////////
template <class It, class Enable = void>
struct search_is_ra_it
{  static const bool value = false;  };

template <class It>
struct search_is_ra_it
   <It, typename iterator_enable_if_tag<It, std::random_access_iterator_tag>::type>
{  static const bool value = true;  };

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
struct search_clampable
{
   static const bool value = false &&
      dtl::is_same<Sent1, FwdIt1>::value && dtl::is_same<Sent2, FwdIt2>::value &&
      search_is_ra_it<FwdIt1>::value && search_is_ra_it<FwdIt2>::value;
};

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
typename algo_enable_if_c
   <!search_clampable<FwdIt1, Sent1, FwdIt2, Sent2>::value, FwdIt1>::type
segmented_search_dispatch(FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last)
{
   typedef typename segmented_iterator_traits<FwdIt2>::is_segmented_iterator ndl_tag_t;

   if(BOOST_UNLIKELY(s_first == s_last))
      return first;

   equal_to_deref<FwdIt2> eq(s_first);

   while(first != last) {
      first = boost::container::segmented_find_if(first, last, eq);
      if(first == last)
         return last;

      //Verification starts one past both cursors: segmented_find_if has just
      //matched the candidate's first element, and comparing it again would
      //over-apply by one, which for a one-element needle already exceeds the
      //(last1 - first1) * (last2 - first2) applications [alg.search] allows.
      FwdIt1 it = first;
      ++it;
      FwdIt2 s_it = s_first;
      ++s_it;
      if(s_it == s_last)
         return first;          // one-element needle -> already matched
      if(it == last)
         return last;           // source exhausted before needle

      const segduo<FwdIt1, bool> r = (segmented_search_verify)
         (it, last, s_it, s_last, ndl_tag_t());
      if(r.second)
         return first;          // full needle consumed -> match
      if(r.first == last)
         return last;           // source exhausted before needle
      ++first;
   }
   return last;
}

//////////////////////////////////////////////////////////////////////////////
// Random-access dispatch: clamp the candidate scan to the last position where
// a whole needle still fits.  No candidate can then run off the end mid-needle,
// so the verify runs against an unreachable haystack end and its bound test
// folds away, taking the last per-element test out of the lock-step loop.
//////////////////////////////////////////////////////////////////////////////
template <class RAIt1, class RAIt2>
typename algo_enable_if_c
   <search_clampable<RAIt1, RAIt1, RAIt2, RAIt2>::value, RAIt1>::type
segmented_search_dispatch(RAIt1 first, RAIt1 last, RAIt2 s_first, RAIt2 s_last)
{
   typedef typename iterator_traits<RAIt1>::difference_type difference_type;
   typedef typename segmented_iterator_traits<RAIt2>::is_segmented_iterator ndl_tag_t;

   const difference_type n2 = difference_type(s_last - s_first);
   if(BOOST_UNLIKELY(!n2))
      return first;
   if(BOOST_UNLIKELY((last - first) < n2))
      return last;

   const RAIt1 scan_last = last - (n2 - 1);

   equal_to_deref<RAIt2> eq(s_first);

   while(first != scan_last) {
      first = boost::container::segmented_find_if(first, scan_last, eq);
      if(first == scan_last)
         return last;

      //As above: the candidate's first element has already been matched by
      //segmented_find_if and must not be compared a second time.
      RAIt1 it = first;
      ++it;
      RAIt2 s_it = s_first;
      ++s_it;
      if(s_it == s_last)
         return first;          // one-element needle -> already matched

      const segduo<RAIt1, bool> r = (segmented_search_verify)
         (it, unreachable_sentinel_t(), s_it, s_last, ndl_tag_t());
      if(r.second)
         return first;
      ++first;
   }
   return last;
}

} // namespace detail_algo

//! Finds the first occurrence of the subsequence [s_first, s_last) in [first, last).
//! Returns an iterator to the beginning of the found subsequence, or \c last if not found.
//! Exploits segmentation recursively on both ranges.
template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
BOOST_CONTAINER_FORCEINLINE
FwdIt1 segmented_search(FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last)
{
   return detail_algo::segmented_search_dispatch(first, last, s_first, s_last);
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_HPP
