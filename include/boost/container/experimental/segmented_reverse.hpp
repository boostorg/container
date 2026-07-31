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
#include <boost/container/experimental/detail/segmented_common_algo.hpp>
#include <boost/move/adl_move_swap.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

namespace detail_algo {

//Same-segment reverse: simply a reverse loop with move-swaps. No segmentation
template <class BidirIt, class Cat>
BOOST_CONTAINER_FORCEINLINE
void segmented_reverse_dispatch(BidirIt first, BidirIt last, non_segmented_iterator_tag, const Cat &)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   while(first != last && first != --last) {
      boost::adl_move_swap(*first, *last);
      ++first;
   }
}

template <class RAIter>
BOOST_CONTAINER_FORCEINLINE
void segmented_reverse_dispatch(RAIter first, RAIter last, non_segmented_iterator_tag, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RAIter>::difference_type difference_type;
   difference_type pairs = (last - first) / difference_type(2);

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; pairs; --pairs) {
      --last;
      boost::adl_move_swap(*first, *last);
      ++first;
   }
}

//////////////////////////////////////////////
// segmented_reverse_disjoint_ranges: swaps elements between
// [f, f_end) (forward) and [l_beg, l) (backward).
// Returns segduo with the final positions of f and l.
// At least one side is fully consumed on return.
//////////////////////////////////////////////

template <class It, class Cat>
BOOST_CONTAINER_FORCEINLINE
segduo<It, It> segmented_reverse_disjoint_ranges
   (It f, It const f_end, It const l_beg, It l, non_segmented_iterator_tag, const Cat &)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   while (f != f_end && l != l_beg) {
      --l;
      boost::adl_move_swap(*f, *l);
      ++f;
   }
   return segduo<It, It>(f, l);
}

template <class It>
BOOST_CONTAINER_FORCEINLINE
segduo<It, It> segmented_reverse_disjoint_ranges
   (It f, It const f_end, It const l_beg, It l, non_segmented_iterator_tag, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<It>::difference_type difference_type;

   difference_type n_f = f_end - f;
   difference_type n_l = l - l_beg;
   difference_type n = n_f < n_l ? n_f : n_l;

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; n; --n) {
      --l;
      boost::adl_move_swap(*f, *l);
      ++f;
   }

   return segduo<It, It>(f, l);
}

template <class It, class Cat>
segduo<It, It> segmented_reverse_disjoint_ranges(It f, It f_end, It l_beg, It l, segmented_iterator_tag, const Cat&)
{
   typedef segmented_iterator_traits<It>        traits;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename traits::local_iterator      local_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator   is_local_seg_t;
   typedef typename iterator_traits
      <local_iterator>::iterator_category       local_cat_t;

   //Nothing to swap here if a range is empty
   if (BOOST_UNLIKELY(f == f_end || l == l_beg))
      return segduo<It, It>(f, l);

   segment_iterator       fs     = traits::segment(f);
   const segment_iterator fs_end = traits::segment(f_end);
   local_iterator         fi     = traits::local(f);

   segment_iterator       ls     = traits::segment(l);
   const segment_iterator ls_beg = traits::segment(l_beg);
   local_iterator         li     = traits::local(l);

   while (true) {
      const local_iterator fi_end = (fs == fs_end) ? traits::local(f_end) : traits::end(fs);
      const local_iterator li_beg = (ls == ls_beg) ? traits::local(l_beg) : traits::begin(ls);
      {
         const segduo<local_iterator, local_iterator> r =
            segmented_reverse_disjoint_ranges(fi, fi_end, li_beg, li, is_local_seg_t(), local_cat_t());
         fi = r.first;
         li = r.second;
      }

      if (fi == fi_end) {
         if (fs == fs_end)
            break;
         ++fs;
         fi = traits::begin(fs);
      }
      if (li == li_beg) {
         if (ls == ls_beg)
            break;
         --ls;
         li = traits::end(ls);
      }
   }

   return segduo<It, It>(traits::compose(fs, fi), traits::compose(ls, li));
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

   while (sf != sl) {
      const local_iterator f_end = traits::end(sf);
      const local_iterator l_beg = traits::begin(sl);

      const segduo<local_iterator, local_iterator> r =
         segmented_reverse_disjoint_ranges(f_loc, f_end, l_beg, l_loc, is_local_seg_t(), local_cat_t());
      f_loc = r.first;
      l_loc = r.second;

      if (f_loc == f_end) {
         ++sf;
         f_loc = traits::begin(sf);
         if (sf == sl)
            break;
      }
      if (l_loc == l_beg) {
         --sl;
         l_loc = traits::end(sl);
      }
   }

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
