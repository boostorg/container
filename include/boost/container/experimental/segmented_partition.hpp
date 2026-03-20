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
#include <boost/container/experimental/segmented_find_if_not.hpp>
#include <boost/move/adl_move_swap.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Sent, class Pred>
FwdIt segmented_partition(FwdIt first, Sent last, Pred pred);

namespace detail_algo {

//////////////////////////////////////////////
// Forward (Lomuto-style) partition
//////////////////////////////////////////////

template <class FwdIt, class Sent, class OutIter, class Pred>
OutIter partition_scan(FwdIt first, Sent last, OutIter result, Pred pred, non_segmented_iterator_tag, const std::forward_iterator_tag &)
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
OutIter partition_scan(SegIt first, SegIt last, OutIter result, Pred pred, segmented_iterator_tag, const std::forward_iterator_tag & cat)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;

   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);
   local_iterator   lcur  = traits::local(first);

   if(scur == slast) {
      return partition_scan(lcur, traits::local(last), result, pred, is_local_seg_t(), cat);
   }
   else {
      result = partition_scan(lcur, traits::end(scur), result, pred, is_local_seg_t(), cat);

      for(++scur; scur != slast; ++scur)
         result = partition_scan(traits::begin(scur), traits::end(scur), result, pred, is_local_seg_t(), cat);

      return partition_scan(traits::begin(scur), traits::local(last), result, pred, is_local_seg_t(), cat);
   }
}

//////////////////////////////////////////////
// Bidirectional (Hoare-style) partition
//////////////////////////////////////////////

template <class BidirIt, class Pred>
BidirIt partition_scan(BidirIt first, BidirIt last, Pred pred, non_segmented_iterator_tag, const std::bidirectional_iterator_tag&)
{
   while(true) {
      while(first != last && pred(*first))
         ++first;
      if(first == last)
         return first;
      --last;
      while(first != last && !pred(*last))
         --last;
      if(first == last)
         return first;
      boost::adl_move_swap(*first, *last);
      ++first;
   }
}

template <class SegIt, class Pred>
SegIt partition_scan(SegIt first, SegIt last, Pred pred, segmented_iterator_tag, const std::bidirectional_iterator_tag& cat)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   if(first == last) return first;

   segment_iterator sf = traits::segment(first);
   segment_iterator sl = traits::segment(last);

   if(sf == sl) {
      local_iterator r = partition_scan(traits::local(first), traits::local(last), pred, is_local_seg_t(), cat);
      return traits::compose(sf, r);
   }
   else {
      local_iterator f_loc = traits::local(first);
      local_iterator f_end = traits::end(sf);
      local_iterator l_loc = traits::local(last);
      local_iterator l_beg = traits::begin(sl);

      for (;;) {
         // Phase 1: advance front to find element NOT satisfying pred
         f_loc = boost::container::segmented_find_if_not(f_loc, f_end, pred);

         if (f_loc == f_end) {
            ++sf;
            if (sf == sl) {
               local_iterator r = partition_scan(l_beg, l_loc, pred, is_local_seg_t(), cat);
               return traits::compose(sf, r);
            }
            f_loc = traits::begin(sf);
            f_end = traits::end(sf);
            continue;
         }

         // Phase 2: retreat back to find element satisfying pred
         for (;;) {
            if (l_loc == l_beg) {
               --sl;
               if (sf == sl) {
                  local_iterator r = partition_scan(f_loc, f_end, pred, is_local_seg_t(), cat);
                  return traits::compose(sf, r);
               }
               l_beg = traits::begin(sl);
               l_loc = traits::end(sl);
            }
            --l_loc;
            if (pred(*l_loc)) break;
         }

         boost::adl_move_swap(*f_loc, *l_loc);
         ++f_loc;
      }
   }
}

//////////////////////////////////////////////
// Top-level dispatch
//////////////////////////////////////////////

template <class FwdIt, class Sent, class Pred, class Tag>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_partition_dispatch(FwdIt first, Sent last, Pred pred, Tag tag, const std::forward_iterator_tag &cat)
{
   first = (segmented_find_if_not)(first, last, pred);
   if (first == last)
      return first;
   FwdIt next = first;
   ++next;
   return (partition_scan)(next, last, first, pred, tag, cat);
}

template <class FwdIt, class Sent, class Pred, class Tag>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_partition_dispatch(FwdIt first, Sent last, Pred pred, Tag tag, const std::bidirectional_iterator_tag& cat)
{
   return (partition_scan)(first, last, pred, tag, cat);
}

//Sentinels may only be used with forward iterators, so we can ignore the segmentation tag in that case.
template<class It, class Sent, class Seg, class Tag>
struct sent_filter
{
   typedef std::forward_iterator_tag   cat_t;
   typedef non_segmented_iterator_tag  seg_t;
};

template<class It, class Seg, class Tag>
struct sent_filter<It, It, Seg, Tag>
{
   typedef Tag cat_t;
   typedef Seg seg_t;
};

} // namespace detail_algo

//! Reorders elements in [first, last) so that elements satisfying
//! \c pred come before those that do not.
//! For forward iterators, uses a Lomuto-style scan.
//! For bidirectional (or stronger) iterators, uses a Hoare-style
//! partition that swaps from both ends, reducing the number of swaps.
//! Returns an iterator to the partition point.
template <class FwdIt, class Sent, class Pred>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_partition(FwdIt first, Sent last, Pred pred)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   typedef typename boost::container::iterator_traits<FwdIt>::iterator_category cat_t;
   typedef typename traits::is_segmented_iterator seg_t;

   typedef detail_algo::sent_filter<FwdIt, Sent, seg_t, cat_t> sent_filter_t;
   return detail_algo::segmented_partition_dispatch
      ( first, last, pred
      , typename sent_filter_t::seg_t()
      , typename sent_filter_t::cat_t());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_HPP
