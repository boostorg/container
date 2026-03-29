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

//Same-segment reverse: simply a reverse loop with move-swaps. No segmentation
template <class BidirIt, class Cat>
void segmented_reverse_dispatch(BidirIt first, BidirIt last, non_segmented_iterator_tag, const Cat &)
{
   while(first != last && first != --last) {
      boost::adl_move_swap(*first, *last);
      ++first;
   }
}

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RAIter>
void segmented_reverse_dispatch(RAIter first, RAIter last, non_segmented_iterator_tag, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RAIter>::difference_type difference_type;
   difference_type pairs = (last - first) / difference_type(2);

   while(pairs >= difference_type(4)) {
      --last; boost::adl_move_swap(*first, *last); ++first;
      --last; boost::adl_move_swap(*first, *last); ++first;
      --last; boost::adl_move_swap(*first, *last); ++first;
      --last; boost::adl_move_swap(*first, *last); ++first;
      pairs -= 4;
   }

   switch (pairs % 4) {
      case 3:
         --last; boost::adl_move_swap(*first, *last); ++first;
      BOOST_FALLTHROUGH;
      case 2:
         --last; boost::adl_move_swap(*first, *last); ++first;
      BOOST_FALLTHROUGH;
      case 1:
         --last; boost::adl_move_swap(*first, *last); //No need to increment first since we're done after this.
      BOOST_FALLTHROUGH;
      default:
         break;
   }
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

//////////////////////////////////////////////
// segmented_reverse_disjoint_ranges: swaps elements between
// [f, f_end) (forward) and [l_beg, l) (backward).
// Updates f and l to their final positions.
// At least one side is fully consumed on return.
//////////////////////////////////////////////

template <class It, class Cat>
void segmented_reverse_disjoint_ranges
   (It& f_out, It const f_end, It const l_beg, It& l_out, non_segmented_iterator_tag, const Cat &)
{
   It f = f_out;
   It l = l_out;

   while (f != f_end && l != l_beg) {
      --l;
      boost::adl_move_swap(*f, *l);
      ++f;
   }
   f_out = f;
   l_out = l;
}

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class It>
void segmented_reverse_disjoint_ranges
   (It& f_out, It const f_end, It const l_beg, It& l_out, non_segmented_iterator_tag, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<It>::difference_type difference_type;

   It f = f_out;
   It l = l_out;
   difference_type n_f = f_end - f;
   difference_type n_l = l - l_beg;
   difference_type n = n_f < n_l ? n_f : n_l;

   while(n >= difference_type(4)) {
      --l; boost::adl_move_swap(*f, *l); ++f;
      --l; boost::adl_move_swap(*f, *l); ++f;
      --l; boost::adl_move_swap(*f, *l); ++f;
      --l; boost::adl_move_swap(*f, *l); ++f;
      n -= 4;
   }

   switch (n % 4) {
      case 3:
         --l; boost::adl_move_swap(*f, *l); ++f;
      BOOST_FALLTHROUGH;
      case 2:
         --l; boost::adl_move_swap(*f, *l); ++f;
      BOOST_FALLTHROUGH;
      case 1:
         --l; boost::adl_move_swap(*f, *l); ++f;
      BOOST_FALLTHROUGH;
      default:
         break;
   }

   f_out = f;
   l_out = l;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class It, class Cat>
void segmented_reverse_disjoint_ranges(It& f, It f_end, It l_beg, It& l, segmented_iterator_tag, const Cat&)
{
   typedef segmented_iterator_traits<It>        traits;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename traits::local_iterator      local_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator   is_local_seg_t;
   typedef typename iterator_traits
      <local_iterator>::iterator_category       local_cat_t;

   //Nothing to swap here if a range is empty
   if (f == f_end || l == l_beg)
      return;

   segment_iterator       fs     = traits::segment(f);
   const segment_iterator fs_end = traits::segment(f_end);
   local_iterator         fi     = traits::local(f);
   local_iterator         fi_end = (fs == fs_end) ? traits::local(f_end) : traits::end(fs);

   segment_iterator       ls     = traits::segment(l);
   const segment_iterator ls_beg = traits::segment(l_beg);
   local_iterator         li     = traits::local(l);
   local_iterator         li_beg = (ls == ls_beg) ? traits::local(l_beg) : traits::begin(ls);

   //At least one of the two sides is guaranteed to be fully consumed after this loop
   //since the ranges are guaranteed not to overlap
   while (true) {
      //Reverse the front and back segments recursively
      segmented_reverse_disjoint_ranges(fi, fi_end, li_beg, li, is_local_seg_t(), local_cat_t());

      //Independent advancement of forward and backward segments since ranges do not to overlap

      //Check if the forward segment was fully consumed
      if (fi == fi_end) {
         //Check if there are no more segments to advance on the forward side, in which case we are done
         if (fs == fs_end) {
            f = f_end;
            l = traits::compose(ls, li);
            return;
         }
         //Advance the forward segment
         ++fs;
         fi = traits::begin(fs);
         fi_end = (fs == fs_end) ? traits::local(f_end) : traits::end(fs);
      }

      //Check if the backward segment was fully consumed
      if (li == li_beg) {
         //Check if there are no more segments to retreat on the backward side, in which case we are done
         if (ls == ls_beg) {
            f = traits::compose(fs, fi);
            l = l_beg;
            return;
         }
         //Retreat the backward segment
         --ls;
         li = traits::end(ls);
         li_beg = (ls == ls_beg) ? traits::local(l_beg) : traits::begin(ls);
      }
   }
}

template <class SegIt, class Cat>
void segmented_reverse_dispatch(SegIt first, SegIt last, segmented_iterator_tag, const Cat &)
{
   typedef segmented_iterator_traits<SegIt>     traits;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename traits::local_iterator      local_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator   is_local_seg_t;
   typedef typename iterator_traits
      <local_iterator>::iterator_category       local_cat_t;

   segment_iterator sf = traits::segment(first);
   segment_iterator sl = traits::segment(last);
   local_iterator f_loc = traits::local(first);
   local_iterator l_loc = traits::local(last);

   if (sf != sl) {
      local_iterator f_end = traits::end(sf);
      local_iterator l_beg = traits::begin(sl);

      while (true) {
         segmented_reverse_disjoint_ranges(f_loc, f_end, l_beg, l_loc, is_local_seg_t(), local_cat_t());

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
   segmented_reverse_dispatch(f_loc, l_loc, is_local_seg_t(), local_cat_t());
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
      ( first, last
      , typename traits::is_segmented_iterator()
      , typename iterator_traits<BidirIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REVERSE_HPP
