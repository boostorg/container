//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REVERSE_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REVERSE_HPP

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

namespace detail_algo {

template <class FwdIt, class BidirIt, class Size>
BidirIt reverse_scan(FwdIt first, FwdIt last, BidirIt back, Size& count, non_segmented_iterator_tag)
{
   for(; first != last && count > 0; ++first, --count) {
      --back;
      boost::adl_move_swap(*first, *back);
   }
   return back;
}

template <class SegIt, class BidirIt, class Size>
BidirIt reverse_scan(SegIt first, SegIt last, BidirIt back, Size& count, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt> traits;
   typedef typename traits::local_iterator local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typename traits::segment_iterator scur = traits::segment(first);
   typename traits::segment_iterator slast = traits::segment(last);
   local_iterator lcur = traits::local(first);
   if(scur == slast) {
      back = reverse_scan(lcur, traits::local(last), back, count, is_local_seg_t());
   }
   else {
      back = reverse_scan(lcur, traits::end(scur), back, count,
         is_local_seg_t());
      for(++scur; scur != slast && count > 0; ++scur)
         back = reverse_scan(traits::begin(scur), traits::end(scur), back, count,
            is_local_seg_t());
      if(count > 0)
         back = reverse_scan(traits::begin(scur), traits::local(last), back, count,
            is_local_seg_t());
   }
   return back;
}

template <class SegIter>
void segmented_reverse_dispatch(SegIter first, SegIter last, segmented_iterator_tag)
{
   typedef typename boost::container::iterator_traits<SegIter>::difference_type diff_t;
   diff_t n = 0;
   for(SegIter it = first; it != last; ++it)
      ++n;
   diff_t swaps = n / 2;
   if(swaps <= 0) return;
   SegIter back = last;
   reverse_scan(first, last, back, swaps, segmented_iterator_tag());
}

template <class BidirIt>
void segmented_reverse_dispatch(BidirIt first, BidirIt last, non_segmented_iterator_tag)
{
   while(first != last) {
      --last;
      if(first == last) break;
      boost::adl_move_swap(*first, *last);
      ++first;
   }
}

} // namespace detail_algo

//! Reverses the order of elements in [first, last).
//! When the iterator is segmented, exploits segmentation on the
//! forward side to reduce per-element overhead.
template <class BidirIter>
void segmented_reverse(BidirIter first, BidirIter last)
{
   typedef segmented_iterator_traits<BidirIter> traits;
   detail_algo::segmented_reverse_dispatch(first, last,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REVERSE_HPP
