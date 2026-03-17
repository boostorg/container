//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/move/adl_move_swap.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Sent, class Pred>
FwdIt segmented_partition(FwdIt first, Sent last, Pred pred);

namespace detail_algo {

template <class FwdIt, class OutIter, class Pred>
OutIter partition_scan(FwdIt first, FwdIt last, OutIter result, Pred pred, non_segmented_iterator_tag)
{
   for(; first != last; ++first) {
      if(pred(*first)) {
         boost::adl_move_swap(*result, *first);
         ++result;
      }
   }
   return result;
}

template <class SegIt, class OutIter, class Pred>
OutIter partition_scan(SegIt first, SegIt last, OutIter result, Pred pred, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;

   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);
   local_iterator   lcur  = traits::local(first);

   if(scur == slast) {
      return partition_scan(lcur, traits::local(last), result, pred, is_local_seg_t());
   }
   else {
      result = partition_scan(lcur, traits::end(scur), result, pred, is_local_seg_t());

      for(++scur; scur != slast; ++scur)
         result = partition_scan(traits::begin(scur), traits::end(scur), result, pred, is_local_seg_t());

      return partition_scan(traits::begin(scur), traits::local(last), result, pred, is_local_seg_t());
   }
}

template <class SegIter, class Pred>
BOOST_CONTAINER_FORCEINLINE SegIter segmented_partition_dispatch
   (SegIter first, SegIter last, Pred pred, segmented_iterator_tag)
{
   return partition_scan(first, last, first, pred, segmented_iterator_tag());
}

template <class FwdIt, class Sent, class Pred, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value, FwdIt>::type
segmented_partition_dispatch
   (FwdIt first, Sent last, Pred pred, Tag)
{
   FwdIt result = first;
   for(; first != last; ++first) {
      if(pred(*first)) {
         boost::adl_move_swap(*result, *first);
         ++result;
      }
   }
   return result;
}

} // namespace detail_algo

//! Reorders elements in [first, last) so that elements satisfying
//! \c pred come before those that do not (Lomuto-style partition).
//! Returns an iterator to the partition point.
template <class FwdIt, class Sent, class Pred>
BOOST_CONTAINER_FORCEINLINE FwdIt segmented_partition(FwdIt first, Sent last, Pred pred)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_partition_dispatch(first, last, pred,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_HPP
