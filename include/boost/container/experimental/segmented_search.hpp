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

namespace boost {
namespace container {

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
FwdIt1 segmented_search
   (FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last);

namespace detail_algo {

template <class FwdIt1, class Sent, class FwdIt2, class Sent2>
bool matches_at(FwdIt1 pos, Sent last, FwdIt2 s_first, Sent2 s_last)
{
   for(; s_first != s_last; ++s_first, ++pos)
      if(pos == last || !(*pos == *s_first))
         return false;
   return true;
}

// Non-recursive: match verification (matches_at) must span across segment
// boundaries using the full segmented iterator.
template <class SegIter, class FwdIt2, class Sent2>
SegIter segmented_search_dispatch
   (SegIter first, SegIter last, FwdIt2 s_first, Sent2 s_last, segmented_iterator_tag)
{
   if(s_first == s_last) return first;

   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   
   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);
   local_iterator   lcur  = traits::local(first);

   if(scur == slast) {
      local_iterator lend = traits::local(last);
      for(; lcur != lend; ++lcur) {
         if(*lcur == *s_first) {
            SegIter pos = traits::compose(scur, lcur);
            if(matches_at(pos, last, s_first, s_last))
               return pos;
         }
      }
   }
   else {
      {
         local_iterator lend = traits::end(scur);
         for(; lcur != lend; ++lcur) {
            if(*lcur == *s_first) {
               SegIter pos = traits::compose(scur, lcur);
               if(matches_at(pos, last, s_first, s_last))
                  return pos;
            }
         }
      }
      for(++scur; scur != slast; ++scur) {
         local_iterator lb = traits::begin(scur);
         local_iterator le = traits::end(scur);
         for(lcur = lb; lcur != le; ++lcur) {
            if(*lcur == *s_first) {
               SegIter pos = traits::compose(scur, lcur);
               if(matches_at(pos, last, s_first, s_last))
                  return pos;
            }
         }
      }
      {
         local_iterator lb = traits::begin(scur);
         local_iterator ll = traits::local(last);
         for(lcur = lb; lcur != ll; ++lcur) {
            if(*lcur == *s_first) {
               SegIter pos = traits::compose(scur, lcur);
               if(matches_at(pos, last, s_first, s_last))
                  return pos;
            }
         }
      }
   }
   return last;
}

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent1, FwdIt1>::value, FwdIt1>::type
segmented_search_dispatch
   (FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last, Tag)
{
   if(s_first == s_last)
      return first;

   for(; first != last; ++first) {
      FwdIt1 it = first;
      FwdIt2 s_it = s_first;
      for(;;) {
         if(s_it == s_last)
            return first;
         if(it == last)
            return last;
         if(!(*it == *s_it))
            break;
         ++it;
         ++s_it;
      }
   }
   return last;
}

} // namespace detail_algo

//! Finds the first occurrence of the subsequence [s_first, s_last) in [first, last).
//! Returns an iterator to the beginning of the found subsequence, or \c last if not found.
template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
BOOST_CONTAINER_FORCEINLINE
FwdIt1 segmented_search(FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last)
{
   typedef segmented_iterator_traits<FwdIt1> traits;
   return detail_algo::segmented_search_dispatch
      (first, last, s_first, s_last, typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_HPP
