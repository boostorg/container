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

namespace boost {
namespace container {

template <class FwdIt, class Sent, class Size, class T>
FwdIt segmented_search_n
   (FwdIt first, Sent last, Size count, const T& value);

namespace detail_algo {

// Non-recursive: must carry cross-segment state (consecutive count,
// match_start) to track runs that span segment boundaries.
template <class SegIter, class Size, class T>
SegIter segmented_search_n_dispatch
   (SegIter first, SegIter last, Size count, const T& value, segmented_iterator_tag)
{
   if(count <= 0) return first;

   typedef segmented_iterator_traits<SegIter> traits;
   Size consecutive = 0;
   SegIter match_start = first;

   typename traits::segment_iterator scur  = traits::segment(first);
   typename traits::segment_iterator slast = traits::segment(last);
   typename traits::local_iterator   lcur  = traits::local(first);

   if(scur == slast) {
      typename traits::local_iterator lend = traits::local(last);
      for(; lcur != lend; ++lcur) {
         if(*lcur == value) {
            if(consecutive == 0)
               match_start = traits::compose(scur, lcur);
            ++consecutive;
            if(consecutive == count)
               return match_start;
         }
         else {
            consecutive = 0;
         }
      }
   }
   else {
      {
         typename traits::local_iterator lend = traits::end(scur);
         for(; lcur != lend; ++lcur) {
            if(*lcur == value) {
               if(consecutive == 0)
                  match_start = traits::compose(scur, lcur);
               ++consecutive;
               if(consecutive == count)
                  return match_start;
            }
            else {
               consecutive = 0;
            }
         }
      }
      for(++scur; scur != slast; ++scur) {
         typename traits::local_iterator lb = traits::begin(scur);
         typename traits::local_iterator le = traits::end(scur);
         for(lcur = lb; lcur != le; ++lcur) {
            if(*lcur == value) {
               if(consecutive == 0)
                  match_start = traits::compose(scur, lcur);
               ++consecutive;
               if(consecutive == count)
                  return match_start;
            }
            else {
               consecutive = 0;
            }
         }
      }
      {
         typename traits::local_iterator lb = traits::begin(scur);
         typename traits::local_iterator ll = traits::local(last);
         for(lcur = lb; lcur != ll; ++lcur) {
            if(*lcur == value) {
               if(consecutive == 0)
                  match_start = traits::compose(scur, lcur);
               ++consecutive;
               if(consecutive == count)
                  return match_start;
            }
            else {
               consecutive = 0;
            }
         }
      }
   }
   return last;
}

template <class FwdIt, class Sent, class Size, class T, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value, FwdIt>::type
segmented_search_n_dispatch
   (FwdIt first, Sent last, Size count, const T& value, Tag)
{
   if(count <= 0)
      return first;

   for(; first != last; ++first) {
      if(*first == value) {
         FwdIt candidate = first;
         Size matched = 1;
         FwdIt it = first;
         ++it;
         for(; matched < count; ++matched, ++it) {
            if(it == last)
               return last;
            if(!(*it == value))
               break;
         }
         if(matched == count)
            return candidate;
      }
   }
   return last;
}

} // namespace detail_algo

//! Finds the first occurrence of \c count consecutive elements equal to \c value
//! in [first, last). Returns an iterator to the start of the run, or \c last if not found.
template <class FwdIt, class Sent, class Size, class T>
BOOST_CONTAINER_FORCEINLINE FwdIt segmented_search_n
   (FwdIt first, Sent last, Size count, const T& value)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_search_n_dispatch(first, last, count, value,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_N_HPP
