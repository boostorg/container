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

template <class FwdIt, class Sent, class Predicate, class Tag, class Cat>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value, FwdIt>::type
segmented_partition_point_dispatch
   (FwdIt first, Sent last, Predicate pred, Tag, Cat)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
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
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   segment_iterator        scur = traits::segment(first);
   segment_iterator const slast = traits::segment(last);

   local_iterator lb = traits::local(first);

   for(;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = scur == slast;
      const local_iterator le = last_seg ? traits::local(last) : traits::end(scur);
      const local_iterator r = (segmented_partition_point_dispatch)
         (lb, le, pred, is_local_seg_t(), local_cat_t());
      if (r != le)
         return traits::compose(scur, r);
      if(BOOST_CONTAINER_SEG_UNLIKELY(last_seg))
         return last;

      //middle segments keep their own call site: begin/end both come from
      //scur, so the leaf can be specialised for full segments
      for(++scur; scur != slast; ++scur) {
         const local_iterator me = traits::end(scur);
         const local_iterator mr = (segmented_partition_point_dispatch)
            (traits::begin(scur), me, pred, is_local_seg_t(), local_cat_t());
         if (mr != me)
            return traits::compose(scur, mr);
      }
      lb = traits::begin(scur);
   }
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
