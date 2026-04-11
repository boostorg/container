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

struct default_less
{
   template <class T>
   bool operator()(const T& a, const T& b) const { return a < b; }
};

// Non-recursive: must carry cross-segment state (prev_local) to compare
// the last element of one segment with the first of the next.
template <class SegIter, class Comp, class Cat>
SegIter segmented_is_sorted_until_dispatch
   (SegIter first, SegIter last, Comp comp, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>   traits;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename traits::local_iterator      local_iterator;

   if(first == last)
      return last;

   segment_iterator sfirst = traits::segment(first);
   segment_iterator slast  = traits::segment(last);

   local_iterator prev_local = traits::local(first);
   local_iterator lcur;

   lcur = prev_local;
   ++lcur;

   if(sfirst == slast) {
      local_iterator lend = traits::local(last);
      for(; lcur != lend; ++lcur) {
         if(comp(*lcur, *prev_local))
            return traits::compose(sfirst, lcur);
         prev_local = lcur;
      }
      return last;
   }
   else {
      // First segment: from local(first) to end(sfirst)
      {
         local_iterator lend = traits::end(sfirst);

         for (; lcur != lend; ++lcur) {
            if (comp(*lcur, *prev_local))
               return traits::compose(sfirst, lcur);
            prev_local = lcur;
         }
      }

      // Middle segments
      for (++sfirst; sfirst != slast; ++sfirst) {
         local_iterator lb = traits::begin(sfirst);
         local_iterator le = traits::end(sfirst);
         if (lb != le) {
            if (comp(*lb, *prev_local))
               return traits::compose(sfirst, lb);
            prev_local = lb;
            lcur = lb;
            ++lcur;
            for (; lcur != le; ++lcur) {
               if (comp(*lcur, *prev_local))
                  return traits::compose(sfirst, lcur);
               prev_local = lcur;
            }
         }
      }

      // Last segment: from begin(slast) to local(last)
      {
         local_iterator lb = traits::begin(sfirst);
         local_iterator ll = traits::local(last);
         if (lb != ll) {
            if (comp(*lb, *prev_local))
               return traits::compose(sfirst, lb);
            prev_local = lb;
            lcur = lb;
            ++lcur;
            for (; lcur != ll; ++lcur) {
               if (comp(*lcur, *prev_local))
                  return traits::compose(sfirst, lcur);
               prev_local = lcur;
            }
         }
      }
   }

   return last;
}

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RAIter, class Comp>
RAIter segmented_is_sorted_until_dispatch
   (RAIter first, RAIter last, Comp comp, const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RAIter>::difference_type difference_type;

   difference_type n = last - first;
   if(n <= difference_type(1))
      return last;

   RAIter prev = first;
   ++first;
   --n;

   while(n >= difference_type(4)) {
      if(comp(*first, *prev))
         goto final_result;
      prev = first; ++first;
      if(comp(*first, *prev))
         goto final_result;
      prev = first; ++first;
      if(comp(*first, *prev))
         goto final_result;
      prev = first; ++first;
      if(comp(*first, *prev))
         goto final_result;
      prev = first; ++first;
      n -= 4;
   }

   switch(n) {
      case 3:
         if(comp(*first, *prev))
            goto final_result;
         prev = first; ++first;
         BOOST_FALLTHROUGH;
      case 2:
         if(comp(*first, *prev))
            goto final_result;
         prev = first; ++first;
         BOOST_FALLTHROUGH;
      case 1:
         if(comp(*first, *prev))
            goto final_result;
         ++first;
         BOOST_FALLTHROUGH;
      default:
         break;
   }

   final_result:
   return first;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class FwdIt, class Sent, class Comp, class Tag, class Cat>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value, FwdIt>::type
segmented_is_sorted_until_dispatch(FwdIt first, Sent last, Comp comp, Tag, Cat)
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
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_is_sorted_until(FwdIt first, Sent last, Comp comp)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_is_sorted_until_dispatch
      (first, last, comp, typename traits::is_segmented_iterator(), typename iterator_traits<FwdIt>::iterator_category());
}

//! Returns an iterator to the first element that is less than its predecessor
//! in [first, last), using operator<.
//! Returns \c last if the range is sorted.
template <class FwdIt, class Sent>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_is_sorted_until(FwdIt first, Sent last)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_is_sorted_until_dispatch
      (first, last, detail_algo::default_less(), typename traits::is_segmented_iterator(), typename iterator_traits<FwdIt>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_IS_SORTED_UNTIL_HPP
