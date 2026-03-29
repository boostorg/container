//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_LAST_IF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_LAST_IF_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Sent, class Pred>
FwdIt segmented_find_last_if(FwdIt first, Sent last, Pred pred);

namespace detail_algo {

//////////////////////////////////////////////
// Non-segmented scans
//////////////////////////////////////////////

template <class FwdIt, class Pred>
FwdIt find_last_if_scan(FwdIt first, FwdIt last, Pred pred,
                        non_segmented_iterator_tag, const std::forward_iterator_tag&)
{
   FwdIt result = last;
   for (; first != last; ++first)
      if (pred(*first))
         result = first;
   return result;
}

template <class BidirIt, class Pred>
BidirIt find_last_if_scan(BidirIt first, BidirIt last, Pred pred,
                          non_segmented_iterator_tag, const std::bidirectional_iterator_tag&)
{
   BidirIt cur = last;
   while (cur != first) {
      --cur;
      if (pred(*cur))
         return cur;
   }
   return last;
}

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RAIter, class Pred>
RAIter find_last_if_scan(RAIter first, RAIter last, Pred pred,
                         non_segmented_iterator_tag, const std::random_access_iterator_tag&)
{
   typedef typename iterator_traits<RAIter>::difference_type difference_type;

   const RAIter not_found = last;
   RAIter cur = last;
   difference_type n = cur - first;
   while (n >= difference_type(4)) {
      --cur;
      if (pred(*cur))
         return cur;
      --cur;
      if (pred(*cur))
         return cur;
      --cur;
      if (pred(*cur))
         return cur;
      --cur;
      if (pred(*cur))
         return cur;
      n -= 4;
   }

   switch(n) {
      case 3:
         --cur;
         if (pred(*cur))
            return cur;
         --cur;
         if (pred(*cur))
            return cur;
         --cur;
         if (pred(*cur))
            return cur;
         break;
      case 2:
         --cur;
         if (pred(*cur))
            return cur;
         --cur;
         if (pred(*cur))
            return cur;
         break;
      case 1:
         --cur;
         if (pred(*cur))
            return cur;
         break;
      default:
         break;
   }
   return not_found;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

//////////////////////////////////////////////
// Segmented forward scan
//////////////////////////////////////////////

template <class SegIt, class Pred>
SegIt find_last_if_scan(SegIt first, SegIt last, Pred pred,
                        segmented_iterator_tag, const std::forward_iterator_tag&)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category               local_cat_t;

   SegIt result = last;
   segment_iterator       sfirst = traits::segment(first);
   const segment_iterator slast  = traits::segment(last);

   if (sfirst == slast) {
      return traits::compose(sfirst, find_last_if_scan(traits::local(first), traits::local(last), pred, is_local_seg_t(), local_cat_t()));
   }
   else {
      {  // First segment
         const local_iterator le = traits::end(sfirst);
         const local_iterator r = find_last_if_scan(traits::local(first), le, pred, is_local_seg_t(), local_cat_t());
         if (r != le)
            result = traits::compose(sfirst, r);
      }
         // Middle segments
      for (++sfirst; sfirst != slast; ++sfirst) {
         const local_iterator le = traits::end(sfirst);
         const local_iterator r = find_last_if_scan(traits::begin(sfirst), le, pred, is_local_seg_t(), local_cat_t());
         if (r != le)
            result = traits::compose(sfirst, r);
      }
      // Last segment
      return traits::compose(sfirst, find_last_if_scan(traits::begin(slast), traits::local(last), pred, is_local_seg_t(), local_cat_t()));
   }
}

//////////////////////////////////////////////
// Segmented bidirectional scan
//////////////////////////////////////////////

template <class SegIt, class Pred>
SegIt find_last_if_scan(SegIt first, SegIt last, Pred pred, segmented_iterator_tag, const std::bidirectional_iterator_tag&)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   segment_iterator sfirst = traits::segment(first);
   segment_iterator slast  = traits::segment(last);
   const local_iterator ll = traits::local(last);

   if (sfirst == slast) {
      const local_iterator r =
         find_last_if_scan(traits::local(first), ll, pred, is_local_seg_t(), local_cat_t());
      return traits::compose(sfirst, r);
   }

   // Last segment (partial): [begin(slast), local(last))
   local_iterator r = find_last_if_scan(traits::begin(slast), ll, pred, is_local_seg_t(), local_cat_t());
   if (r != ll)
      return traits::compose(slast, r);

   // Middle segments in reverse
   for (--slast; slast != sfirst; --slast) {
      const local_iterator le = traits::end(slast);
      r = find_last_if_scan(traits::begin(slast), le, pred, is_local_seg_t(), local_cat_t());
      if (r != le)
         return traits::compose(slast, r);
   }

   // First segment (partial): [local(first), end(sfirst))
   const local_iterator le = traits::end(sfirst);
   r  = find_last_if_scan(traits::local(first), le, pred, is_local_seg_t(), local_cat_t());
   if (r != le)
      return traits::compose(sfirst, r);

   return last;
}

//////////////////////////////////////////////
// Sentinel / generic fallback
//////////////////////////////////////////////

template <class FwdIt, class Sent, class Pred, class SegTag, class CatTag>
typename algo_enable_if_c<is_sentinel<Sent, FwdIt>::value, FwdIt>::type
   find_last_if_scan(FwdIt first, Sent last, Pred pred, SegTag, CatTag)
{
   FwdIt result = first;
   bool found = false;
   for (; first != last; ++first) {
      if (pred(*first)) { result = first; found = true; }
   }
   return found ? result : first;
}

//////////////////////////////////////////////
// sent_filter: sentinel ⇒ forward + flat
//////////////////////////////////////////////

template<class It, class Sent, class Seg, class Tag>
struct find_last_sent_filter
{
   typedef std::forward_iterator_tag   cat_t;
   typedef non_segmented_iterator_tag  seg_t;
};

template<class It, class Seg, class Tag>
struct find_last_sent_filter<It, It, Seg, Tag>
{
   typedef Tag cat_t;
   typedef Seg seg_t;
};

} // namespace detail_algo

//! Returns an iterator to the last element satisfying \c pred
//! in [first, last), or \c last if not found.
//! For bidirectional iterators, scans backward for early exit.
//! For forward iterators, scans the entire range and remembers
//! the last match.
template <class FwdIt, class Sent, class Pred>
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_find_last_if(FwdIt first, Sent last, Pred pred)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   typedef typename boost::container::iterator_traits<FwdIt>::iterator_category cat_t;
   typedef typename traits::is_segmented_iterator seg_t;
   typedef detail_algo::find_last_sent_filter<FwdIt, Sent, seg_t, cat_t> sf;
   return detail_algo::find_last_if_scan
      ( first, last, pred, typename sf::seg_t(), typename sf::cat_t());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_LAST_IF_HPP
