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

namespace boost {
namespace container {

namespace detail_algo {

struct default_less
{
   template <class T>
   bool operator()(const T& a, const T& b) const { return a < b; }
};

// Non-recursive: must carry cross-segment state (prev_local) to compare
// the last element of one segment with the first of the next.
template <class SegIter, class Comp>
SegIter segmented_is_sorted_until_dispatch
   (SegIter first, SegIter last, Comp comp, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   if(first == last)
      return last;

   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);

   typename traits::local_iterator prev_local = traits::local(first);
   typename traits::local_iterator lcur;

   if(sfirst == slast) {
      lcur = prev_local;
      ++lcur;
      typename traits::local_iterator lend = traits::local(last);
      for(; lcur != lend; ++lcur) {
         if(comp(*lcur, *prev_local))
            return traits::compose(sfirst, lcur);
         prev_local = lcur;
      }
      return last;
   }

   // First segment: from local(first) to end(sfirst)
   {
      typename traits::local_iterator lend = traits::end(sfirst);
      lcur = prev_local;
      ++lcur;
      for(; lcur != lend; ++lcur) {
         if(comp(*lcur, *prev_local))
            return traits::compose(sfirst, lcur);
         prev_local = lcur;
      }
   }

   // Middle segments
   for(++sfirst; sfirst != slast; ++sfirst) {
      typename traits::local_iterator lb = traits::begin(sfirst);
      typename traits::local_iterator le = traits::end(sfirst);
      if(lb != le) {
         if(comp(*lb, *prev_local))
            return traits::compose(sfirst, lb);
         prev_local = lb;
         lcur = lb;
         ++lcur;
         for(; lcur != le; ++lcur) {
            if(comp(*lcur, *prev_local))
               return traits::compose(sfirst, lcur);
            prev_local = lcur;
         }
      }
   }

   // Last segment: from begin(slast) to local(last)
   {
      typename traits::local_iterator lb = traits::begin(sfirst);
      typename traits::local_iterator ll = traits::local(last);
      if(lb != ll) {
         if(comp(*lb, *prev_local))
            return traits::compose(sfirst, lb);
         prev_local = lb;
         lcur = lb;
         ++lcur;
         for(; lcur != ll; ++lcur) {
            if(comp(*lcur, *prev_local))
               return traits::compose(sfirst, lcur);
            prev_local = lcur;
         }
      }
   }

   return last;
}

template <class FwdIt, class Sent, class Comp, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value, FwdIt>::type
segmented_is_sorted_until_dispatch
   (FwdIt first, Sent last, Comp comp, Tag)
{
   if(first == last)
      return last;
   FwdIt prev = first;
   ++first;
   for(; first != last; ++first) {
      if(comp(*first, *prev))
         return first;
      prev = first;
   }
   return last;
}

} // namespace detail_algo

//! Returns an iterator to the first element that is less than its predecessor
//! in [first, last), using \c comp for comparison.
//! Returns \c last if the range is sorted.
template <class FwdIt, class Sent, class Comp>
inline FwdIt segmented_is_sorted_until(FwdIt first, Sent last, Comp comp)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_is_sorted_until_dispatch(first, last, comp,
      typename traits::is_segmented_iterator());
}

//! Returns an iterator to the first element that is less than its predecessor
//! in [first, last), using operator<.
//! Returns \c last if the range is sorted.
template <class FwdIt, class Sent>
inline FwdIt segmented_is_sorted_until(FwdIt first, Sent last)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_is_sorted_until_dispatch(first, last,
      detail_algo::default_less(),
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_IS_SORTED_UNTIL_HPP
