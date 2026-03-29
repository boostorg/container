//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_POINT_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_POINT_HPP

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

namespace boost {
namespace container {

template <class FwdIt, class Sent, class Predicate>
FwdIt segmented_partition_point(FwdIt first, Sent last, Predicate pred);

namespace detail_algo {

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RAIter, class Predicate>
RAIter segmented_partition_point_dispatch
   (RAIter first, RAIter last, Predicate pred, const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RAIter>::difference_type difference_type;

   difference_type n = last - first;
   while(n >= difference_type(4)) {
      if(!pred(*first))
         goto final_result;
      ++first;
      if(!pred(*first))
         goto final_result;
      ++first;
      if(!pred(*first))
         goto final_result;
      ++first;
      if(!pred(*first))
         goto final_result;
      ++first;
      n -= 4;
   }

   switch(n) {
      case 3:
         if(!pred(*first))
            goto final_result;
         ++first;
         if(!pred(*first))
            goto final_result;
         ++first;
         if(!pred(*first))
            goto final_result;
         ++first;
         break;
      case 2:
         if(!pred(*first))
            goto final_result;
         ++first;
         if(!pred(*first))
            goto final_result;
         ++first;
         break;
      case 1:
         if(!pred(*first))
            goto final_result;
         ++first;
         break;
      default:
         break;
   }
   final_result:
   return first;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class FwdIt, class Sent, class Predicate, class Tag, class Cat>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value, FwdIt>::type
segmented_partition_point_dispatch
   (FwdIt first, Sent last, Predicate pred, Tag, Cat)
{
   for(; first != last; ++first)
      if(!pred(*first))
         return first;
   return last;
}

template <class SegIter, class Predicate, class Cat>
SegIter segmented_partition_point_dispatch
   (SegIter first, SegIter last, Predicate pred, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>   traits;
   typedef typename traits::local_iterator      local_iterator;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator   is_local_seg_t;

   segment_iterator        scur = traits::segment(first);
   segment_iterator const slast = traits::segment(last);

   if(scur == slast) {
      return traits::compose(scur, 
         (segmented_partition_point_dispatch)(traits::local(first), traits::local(last), pred, is_local_seg_t(), Cat()));
   }
   else {
      {
         local_iterator lcur =
            (segmented_partition_point_dispatch)(traits::local(first), traits::end(scur), pred, is_local_seg_t(), Cat());
         if (lcur != traits::end(scur))
            return traits::compose(scur, lcur);
      }

      for(++scur; scur != slast; ++scur) {
         local_iterator lcur =
            (segmented_partition_point_dispatch)(traits::begin(scur), traits::end(scur), pred, is_local_seg_t(), Cat());
         if(lcur != traits::end(scur))
            return traits::compose(scur, lcur);
      }
      return traits::compose(slast, 
         (segmented_partition_point_dispatch)(traits::begin(scur), traits::local(last), pred, is_local_seg_t(), Cat()));
   }
   return last;
}

} // namespace detail_algo

//! Note: This version is suboptimal, and does not fulfill OlogN comparisons
//! 
//! Finds the partition point in [first, last): the first element
//! for which \c pred returns false. The range must be partitioned
//! with respect to \c pred.
template <class FwdIt, class Sent, class Predicate>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_partition_point(FwdIt first, Sent last, Predicate pred)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_partition_point_dispatch
      (first, last, pred, typename traits::is_segmented_iterator(), typename iterator_traits<FwdIt>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_POINT_HPP
