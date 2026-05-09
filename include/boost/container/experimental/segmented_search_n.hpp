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
   (FwdIt first, Sent last, Size count, const T& value);

namespace detail_algo {

template <class RAIter, class Size, class T>
BOOST_CONTAINER_FORCEINLINE
RAIter segmented_search_n_dispatch
   (RAIter first, RAIter last, Size count, const T& value,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RAIter>::difference_type difference_type;

   const difference_type len = last - first;
   const difference_type dcount = static_cast<difference_type>(count);
   if (dcount > len)
      return last;

   const RAIter scan_end = last - (dcount - 1);

   for (; first < scan_end; ++first) {
      if (*first == value) {
         RAIter candidate = first;
         const RAIter run_end = first + dcount;
         ++first;
         BOOST_CONTAINER_SEGMENTED_UNROLL(4)
         for (; first != run_end; ++first) {
            if (!(*first == value))
               goto continue_scan;
         }
         return candidate;
         continue_scan:;
      }
   }
   return last;
}

template <class FwdIt, class Sent, class Size, class T, class Tag, class Cat>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value, FwdIt>::type
segmented_search_n_dispatch
   (FwdIt first, Sent last, Size count, const T& value, Tag, Cat)
{
   for(; first != last; ++first) {
      if(*first == value) {
         FwdIt candidate = first;
         Size matched = 1;
         ++first;
         BOOST_CONTAINER_SEGMENTED_UNROLL(4)
         for(; matched < count; ++matched, ++first) {
            if(first == last)
               return last;
            if(!(*first == value))
               goto continue_scan;
         }
         return candidate;
         continue_scan:;
      }
   }
   return last;
}

// Scans local range [lcur, lend) for search_n, updating cross-segment state.
// Returns true if a run of 'count' consecutive matches has been completed.
// On return, lcur is past the last element processed, consecutive/match_start
// are updated to reflect any partial match at the segment tail.
template <class LocalIter, class Size, class T, class SegIter, class SegmentIter>
bool search_n_scan_segment
   (LocalIter& lcur, LocalIter lend,
    Size& consecutive, Size count, const T& value,
    SegIter& match_start, SegmentIter scur,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &src_tag)
{
   typedef segmented_iterator_traits<SegIter> seg_traits;
   typedef typename iterator_traits<LocalIter>::difference_type difference_type;

   while(lcur != lend) {
      if(consecutive == 0) {
         // Use find to skip non-matching elements (benefits from RA/vectorization)
         lcur = (segmented_find_dispatch)(lcur, lend, value, non_segmented_iterator_tag(), src_tag);
         if(lcur == lend)
            return false;

         match_start = seg_traits::compose(scur, lcur);
         consecutive = 1;
         if(consecutive == count)
            return true;
         ++lcur;
      }
      else {
         // Verify consecutive matches; we know exactly how many to check
         const difference_type need   = static_cast<difference_type>(count - consecutive);
         const difference_type avail  = lend - lcur;
         const difference_type ncheck = need < avail ? need : avail;
         const LocalIter check_end = lcur + ncheck;

         for(; lcur != check_end; ++lcur) {
            if(!(*lcur == value)) {
               consecutive = 0;
               ++lcur;
               break;
            }
            ++consecutive;
            if(consecutive == count)
               return true;
         }
      }
   }
   return false;
}

template <class LocalIter, class Size, class T, class SegIter, class SegmentIter, class Tag, class Cat>
typename algo_enable_if_c<!Tag::value, bool>::type
search_n_scan_segment
   (LocalIter& lcur, LocalIter lend,
    Size& consecutive, Size count, const T& value,
    SegIter& match_start, SegmentIter scur, Tag, Cat)
{
   typedef segmented_iterator_traits<SegIter> seg_traits;

   for(; lcur != lend; ++lcur) {
      if(*lcur == value) {
         if(consecutive == 0)
            match_start = seg_traits::compose(scur, lcur);
         ++consecutive;
         if(consecutive == count)
            return true;
      }
      else {
         consecutive = 0;
      }
   }
   return false;
}

// Recursively segmented local iterators: decompose into sub-segments
// so that the innermost (non-segmented) level gets RA optimization.
// Uses lend as sentinel to detect whether local_match was set by inner calls
// (a match position is always before lend).
template <class LocalIter, class Size, class T, class SegIter, class SegmentIter, class Cat>
bool search_n_scan_segment
   (LocalIter& lcur, LocalIter lend,
    Size& consecutive, Size count, const T& value,
    SegIter& match_start, SegmentIter scur,
    segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>     outer_traits;
   typedef segmented_iterator_traits<LocalIter>   local_traits;
   typedef typename local_traits::segment_iterator  sub_seg_iter;
   typedef typename local_traits::local_iterator    sub_local_iter;
   typedef typename segmented_iterator_traits<sub_local_iter>::is_segmented_iterator sub_is_seg_t;
   typedef typename iterator_traits<sub_local_iter>::iterator_category sub_cat_t;

   sub_seg_iter   sub_scur  = local_traits::segment(lcur);
   sub_seg_iter   sub_slast = local_traits::segment(lend);
   sub_local_iter sub_lcur  = local_traits::local(lcur);

   LocalIter local_match = lend;

   if(sub_scur == sub_slast) {
      sub_local_iter sub_lend = local_traits::local(lend);
      if(search_n_scan_segment(sub_lcur, sub_lend, consecutive, count, value,
                               local_match, sub_scur, sub_is_seg_t(), sub_cat_t())) {
         if(local_match != lend)
            match_start = outer_traits::compose(scur, local_match);
         lcur = lend;
         return true;
      }
   }
   else {
      {
         sub_local_iter sub_lend = local_traits::end(sub_scur);
         if(search_n_scan_segment(sub_lcur, sub_lend, consecutive, count, value,
                                  local_match, sub_scur, sub_is_seg_t(), sub_cat_t())) {
            if(local_match != lend)
               match_start = outer_traits::compose(scur, local_match);
            lcur = lend;
            return true;
         }
      }
      for(++sub_scur; sub_scur != sub_slast; ++sub_scur) {
         sub_lcur = local_traits::begin(sub_scur);
         sub_local_iter sub_le = local_traits::end(sub_scur);
         if(search_n_scan_segment(sub_lcur, sub_le, consecutive, count, value,
                                  local_match, sub_scur, sub_is_seg_t(), sub_cat_t())) {
            if(local_match != lend)
               match_start = outer_traits::compose(scur, local_match);
            lcur = lend;
            return true;
         }
      }
      {
         sub_lcur = local_traits::begin(sub_scur);
         sub_local_iter sub_ll = local_traits::local(lend);
         if(search_n_scan_segment(sub_lcur, sub_ll, consecutive, count, value,
                                  local_match, sub_scur, sub_is_seg_t(), sub_cat_t())) {
            if(local_match != lend)
               match_start = outer_traits::compose(scur, local_match);
            lcur = lend;
            return true;
         }
      }
   }
   if(local_match != lend)
      match_start = outer_traits::compose(scur, local_match);
   lcur = lend;
   return false;
}

// Non-recursive: must carry cross-segment state (consecutive count,
// match_start) to track runs that span segment boundaries.
template <class SegIter, class Size, class T, class Cat>
SegIter segmented_search_n_dispatch
   (SegIter first, SegIter last, Size count, const T& value, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   Size consecutive = 0;
   SegIter match_start = first;

   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);

   local_iterator lcur  = traits::local(first);

   if(scur == slast) {
      local_iterator lend = traits::local(last);
      if(search_n_scan_segment(lcur, lend, consecutive, count, value,
                               match_start, scur, is_local_seg_t(), local_cat_t()))
         return match_start;
   }
   else {
      {
         local_iterator lend = traits::end(scur);
         if(search_n_scan_segment(lcur, lend, consecutive, count, value,
                                  match_start, scur, is_local_seg_t(), local_cat_t()))
            return match_start;
      }
      for(++scur; scur != slast; ++scur) {
         lcur = traits::begin(scur);
         local_iterator le = traits::end(scur);
         if(search_n_scan_segment(lcur, le, consecutive, count, value,
                                  match_start, scur, is_local_seg_t(), local_cat_t()))
            return match_start;
      }
      {
         lcur = traits::begin(scur);
         local_iterator ll = traits::local(last);
         if(search_n_scan_segment(lcur, ll, consecutive, count, value,
                                  match_start, scur, is_local_seg_t(), local_cat_t()))
            return match_start;
      }
   }
   return last;
}

} // namespace detail_algo

//! Finds the first occurrence of \c count consecutive elements equal to \c value
//! in [first, last). Returns an iterator to the start of the run, or \c last if not found.
template <class FwdIt, class Sent, class Size, class T>
inline FwdIt segmented_search_n
   (FwdIt first, Sent last, Size count, const T& value)
{
   if (BOOST_UNLIKELY(count <= 0))
      return first;
   else if(BOOST_UNLIKELY(count == 1))
      return (segmented_find)(first, last, value);

   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_search_n_dispatch
      (first, last, count, value, typename traits::is_segmented_iterator(),
       typename iterator_traits<FwdIt>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_N_HPP
