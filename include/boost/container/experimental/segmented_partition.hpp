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
BOOST_CONTAINER_FORCEINLINE
OutIter partition_scan(FwdIt first, Sent last, OutIter result, Pred pred, non_segmented_iterator_tag, const std::forward_iterator_tag &)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         boost::adl_move_swap(*result, *first);
         ++result;
      }
   }
   return result;
}

template <class SegIt, class OutIter, class Pred, class Cat>
OutIter partition_scan(SegIt first, SegIt last, OutIter result, Pred pred, segmented_iterator_tag, const Cat &)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;

   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;
   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);
   local_iterator   lcur  = traits::local(first);

   for(;;) {
      const bool last_seg = scur == slast;
      const local_iterator le = last_seg ? traits::local(last) : traits::end(scur);
      result = partition_scan(lcur, le, result, pred, is_local_seg_t(), local_cat_t());
      if(BOOST_CONTAINER_SEG_UNLIKELY(last_seg))
         return result;

      for(++scur; scur != slast; ++scur)
         result = partition_scan(traits::begin(scur), traits::end(scur), result, pred, is_local_seg_t(), local_cat_t());

      lcur = traits::begin(scur);
   }
}

template <class FwdIt, class Sent, class Pred, class Tag>
FwdIt segmented_partition_dispatch(FwdIt first, Sent last, Pred pred, Tag tag, const std::forward_iterator_tag &cat)
{
   first = (segmented_find_if_not)(first, last, pred);
   if (first == last)
      return first;
   FwdIt next = first;
   ++next;
   return (partition_scan)(next, last, first, pred, tag, cat);
}

//////////////////////////////////////////////
// Bidirectional (Hoare-style) partition
//////////////////////////////////////////////

template <class BidirIt, class Pred, class Cat>
BOOST_CONTAINER_FORCEINLINE
BidirIt segmented_partition_dispatch(BidirIt first, BidirIt last, Pred pred, non_segmented_iterator_tag, const Cat&)
{
   while (first != last) {
      while (pred(*first)) {
         if (++first == last)
            goto first_ret;
      }

      do {
         if (first == --last)
            goto first_ret;
      } while (!pred(*last));
      boost::adl_move_swap(*first, *last);
      ++first;
   }
   first_ret:
   return first;
}

// Hoare-style partition on two non-overlapping ranges [f, f_end) (forward) and [l_beg, l) (backward).
// Returns segduo with the final positions of f and l. // At least one side is fully consumed on return.

template <class It, class Pred, class Cat>
BOOST_CONTAINER_FORCEINLINE
segduo<It, It> partition_disjoint_bidir_ranges
   (It f, It const f_end, It const l_beg, It l, Pred pred, non_segmented_iterator_tag, const Cat &)
{
   while (f != f_end) {
      while (pred(*f)) {
         if (++f == f_end)
            goto duo_ret;
      }

      do {
         if (l == l_beg)
            goto duo_ret;
         --l;
      } while (!pred(*l));

      boost::adl_move_swap(*f, *l);
      ++f;
   }

   duo_ret:
   return segduo<It, It>(f, l);
}

template <class It, class Pred, class Cat>
segduo<It, It> partition_disjoint_bidir_ranges(It f, It f_end, It l_beg, It l, Pred pred, segmented_iterator_tag, const Cat&)
{
   typedef segmented_iterator_traits<It>        traits;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename traits::local_iterator      local_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator   is_local_seg_t;
   typedef typename iterator_traits
      <local_iterator>::iterator_category       local_cat_t;

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
            partition_disjoint_bidir_ranges(fi, fi_end, li_beg, li, pred, is_local_seg_t(), local_cat_t());
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

template <class SegIt, class Pred>
SegIt segmented_partition_dispatch(SegIt first, SegIt last, Pred pred, segmented_iterator_tag, const std::bidirectional_iterator_tag&)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   segment_iterator sf = traits::segment(first);
   segment_iterator sl = traits::segment(last);
   local_iterator f_loc = traits::local(first);
   local_iterator l_loc = traits::local(last);

   while (sf != sl) {
      const local_iterator f_end = traits::end(sf);
      const local_iterator l_beg = traits::begin(sl);
      {
         const segduo<local_iterator, local_iterator> r =
            partition_disjoint_bidir_ranges(f_loc, f_end, l_beg, l_loc, pred, is_local_seg_t(), local_cat_t());
         f_loc = r.first;
         l_loc = r.second;
      }

      if (f_loc == f_end) {
         ++sf;
         f_loc = traits::begin(sf);
         //Advancing the forward cursor may make sf == sl; the backward cursor
         //must not then step past it.
         if (sf == sl)
            break;
      }
      if (l_loc == l_beg) {
         --sl;
         l_loc = traits::end(sl);
      }
   }

   return traits::compose(sf, segmented_partition_dispatch(f_loc, l_loc, pred, is_local_seg_t(), local_cat_t()));
}

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
   typedef detail_algo::sent_filter<FwdIt, Sent> sf;

   return detail_algo::segmented_partition_dispatch
      ( first, last, pred
      , typename sf::seg_t()
      , typename sf::cat_t());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_HPP
