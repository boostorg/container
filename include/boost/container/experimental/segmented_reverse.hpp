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

namespace boost {
namespace container {

namespace detail_algo {

template <class BidirIt>
void segmented_reverse_dispatch(BidirIt first, BidirIt last, non_segmented_iterator_tag)
{
   while(first != last && first != --last) {
      boost::adl_move_swap(*first, *last);
      ++first;
   }
}


//Note: This implementation does not support recursively segmented iterators for the most part
//TODO: Add support for recursively segmented iterators, if needed. This will require a more complex
//cross-segment loop that exploits segmentation on both the forward and backward sides until the two sides meet.
template <class SegIt>
void segmented_reverse_dispatch(SegIt first, SegIt last, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt>     traits;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename traits::local_iterator      local_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator   is_local_seg_t;

   segment_iterator sf = traits::segment(first);
   segment_iterator sl = traits::segment(last);
   local_iterator f_loc = traits::local(first);
   local_iterator l_loc = traits::local(last);

   if (sf != sl) {
      local_iterator f_end = traits::end(sf);
      local_iterator l_beg = traits::begin(sl);

      while (true) {
         //Cross-segment reverse loop: stop when either side reaches its end
         while (f_loc != f_end && l_loc != l_beg) {
            --l_loc;
            boost::adl_move_swap(*f_loc, *l_loc);
            ++f_loc;
         }

         //Check if the backward side reached the end of its segment
         if (l_loc == l_beg) {
            //If both sides reached the end of their respective segments
            //advance the forward segment and retreat the backward segment
            if (f_loc == f_end) {
               ++sf;
               //If both segments were adjacent and we exhausted both, we are done
               if (sf == sl)
                  return;

               f_loc = traits::begin(sf);
               f_end = traits::end(sf);
            }

            --sl;
            if (sf == sl) {
               l_loc = f_end;
               break;
            }
            l_beg = traits::begin(sl);
            l_loc = traits::end(sl);
         }
         else {   //The forward side reached the end of its segment
            ++sf;
            if (sf == sl) {
               f_loc = l_beg;
               break;
            }
            f_loc = traits::begin(sf);
            f_end = traits::end(sf);
         }
      }
   }
   //Final reverse loop within the final segment
   segmented_reverse_dispatch(f_loc, l_loc, is_local_seg_t());
   return;
}

} // namespace detail_algo

//! Reverses the order of elements in [first, last).
//! When the iterator is segmented, exploits segmentation on both
//! the forward and backward sides to reduce per-element overhead.
template <class BidirIter>
BOOST_CONTAINER_FORCEINLINE
void segmented_reverse(BidirIter first, BidirIter last)
{
   typedef segmented_iterator_traits<BidirIter> traits;
   detail_algo::segmented_reverse_dispatch
      (first, last, typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REVERSE_HPP
