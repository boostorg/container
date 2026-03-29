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
#include <boost/container/experimental/segmented_find_last_if.hpp>
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

//////////////////////////////////////////////
// Bidirectional (Hoare-style) partition
//////////////////////////////////////////////

template <class BidirIt, class Pred, class Cat>
BidirIt partition_scan(BidirIt first, BidirIt last, Pred pred, non_segmented_iterator_tag, const Cat&)
{
   while(true) {
      bool cond = true;
      while(first != last && (cond = pred(*first)))
         ++first;
      if(cond)
         return first;
      --last;
      cond = true;
      while(first != last && (cond = !pred(*last)))
         --last;
      if(cond)
         return first;
      boost::adl_move_swap(*first, *last);
      ++first;
   }
}

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RAIter, class Pred>
RAIter partition_scan(RAIter first, RAIter last, Pred pred, non_segmented_iterator_tag, const std::random_access_iterator_tag&)
{
   typedef typename iterator_traits<RAIter>::difference_type difference_type;

   while(true) {
      bool cond = true;
      difference_type nfront = last - first;
      while(nfront >= difference_type(4)) {
         if(!(cond = pred(*first)))
            goto front_found;
         ++first;
         if(!(cond = pred(*first)))
            goto front_found;
         ++first;
         if(!(cond = pred(*first)))
            goto front_found;
         ++first;
         if(!(cond = pred(*first)))
            goto front_found;
         ++first;
         nfront -= 4;
      }
      switch(nfront) {
         case 3:
            if(!(cond = pred(*first)))
               goto front_found;
            ++first;
            if(!(cond = pred(*first)))
               goto front_found;
            ++first;
            if(!(cond = pred(*first)))
               goto front_found;
            ++first;
            break;
         case 2:
            if(!(cond = pred(*first)))
               goto front_found;
            ++first;
            if(!(cond = pred(*first)))
               goto front_found;
            ++first;
            break;
         case 1:
            if(!(cond = pred(*first)))
               goto front_found;
            ++first;
            break;
         default:
            break;
      }

      front_found:
      if(cond)
         return first;

      --last;
      cond = true;
      difference_type nback = last - first;
      while(nback >= difference_type(4)) {
         if(!(cond = !pred(*last)))
            goto back_found;
         --last;
         if(!(cond = !pred(*last)))
            goto back_found;
         --last;
         if(!(cond = !pred(*last)))
            goto back_found;
         --last;
         if(!(cond = !pred(*last)))
            goto back_found;
         --last;
         nback -= 4;
      }
      switch(nback) {
         case 3:
            if(!(cond = !pred(*last)))
               goto back_found;
            --last;
            if(!(cond = !pred(*last)))
               goto back_found;
            --last;
            if(!(cond = !pred(*last)))
               goto back_found;
            --last;
            break;
         case 2:
            if(!(cond = !pred(*last)))
               goto back_found;
            --last;
            if(!(cond = !pred(*last)))
               goto back_found;
            --last;
            break;
         case 1:
            if(!(cond = !pred(*last)))
               goto back_found;
            --last;
            break;
         default:
            break;
      }

      back_found:
      if(cond)
         return first;

      boost::adl_move_swap(*first, *last);
      ++first;
   }
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class SegIt, class OutIter, class Pred, class Cat>
OutIter partition_scan(SegIt first, SegIt last, OutIter result, Pred pred, segmented_iterator_tag, const Cat & cat)
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

template <class SegIt, class Pred>
SegIt partition_scan(SegIt first, SegIt last, Pred pred, segmented_iterator_tag, const std::bidirectional_iterator_tag& cat)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator sf = traits::segment(first);
   segment_iterator sl = traits::segment(last);
   local_iterator f_loc = traits::local(first);
   local_iterator l_loc = traits::local(last);

   if(sf != sl) {
      local_iterator f_end = traits::end(sf);
      local_iterator l_beg = traits::begin(sl);

      while (1) {
         // Phase 1: advance front segment by segment to find an element NOT satisfying pred
         while (1) {
            f_loc = boost::container::segmented_find_if_not(f_loc, f_end, pred);

            if (f_loc == f_end) {
               ++sf;
               if (sf == sl) {
                  f_loc = l_beg;
                  goto same_segment_partition_step;
               }
               f_loc = traits::begin(sf);
               f_end = traits::end(sf);
            }
            else
               break;   //Found element not satisfying pred, now find one from the back satisfying pred to swap with.
         }

         // Phase 2: retreat back segment by segment to find an element satisfying pred
         while(1) {
            const local_iterator l_loc_orig = l_loc;
            l_loc = boost::container::segmented_find_last_if(l_beg, l_loc, pred);
            if (l_loc == l_loc_orig) {   //Segment exhausted, move to previous
               --sl;
               if (sf == sl) {
                  l_loc = f_end;
                  goto same_segment_partition_step;
               }
               l_beg = traits::begin(sl);
               l_loc = traits::end(sl);
            }
            else
               break;
         }

         boost::adl_move_swap(*f_loc, *l_loc);
         ++f_loc;
      }
   }

   same_segment_partition_step:
   return traits::compose(sf, partition_scan(f_loc, l_loc, pred, is_local_seg_t(), cat));
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

template <class FwdIt, class Sent, class Pred, class Tag>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_partition_dispatch(FwdIt first, Sent last, Pred pred, Tag tag, const std::random_access_iterator_tag& cat)
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
