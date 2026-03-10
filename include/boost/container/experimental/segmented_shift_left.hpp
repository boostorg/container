//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SHIFT_LEFT_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SHIFT_LEFT_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Sent>
FwdIt segmented_shift_left
   (FwdIt first, Sent last,
    typename boost::container::iterator_traits<FwdIt>::difference_type n);

namespace detail_algo {

template <class FwdIt, class OutIter>
void shift_scan(FwdIt first, FwdIt last, OutIter& result, non_segmented_iterator_tag)
{
   for(; first != last; ++first) {
      *result = boost::move(*first);
      ++result;
   }
}

template <class SegIt, class OutIter>
void shift_scan(SegIt first, SegIt last, OutIter& result, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt> traits;
   typedef typename traits::local_iterator local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typename traits::segment_iterator scur = traits::segment(first);
   typename traits::segment_iterator slast = traits::segment(last);
   local_iterator lcur = traits::local(first);
   if(scur == slast) {
      shift_scan(lcur, traits::local(last), result, is_local_seg_t());
   }
   else {
      shift_scan(lcur, traits::end(scur), result,
         is_local_seg_t());
      for(++scur; scur != slast; ++scur)
         shift_scan(traits::begin(scur), traits::end(scur), result,
            is_local_seg_t());
      shift_scan(traits::begin(scur), traits::local(last), result,
         is_local_seg_t());
   }
}

template <class SegIter>
SegIter segmented_shift_left_dispatch
   (SegIter first, SegIter last,
    typename boost::container::iterator_traits<SegIter>::difference_type n,
    segmented_iterator_tag)
{
   typedef typename boost::container::iterator_traits<SegIter>::difference_type difference_type;
   if(n <= 0) return last;

   SegIter mid = first;
   for(difference_type i = 0; i < n; ++i) {
      if(mid == last) return first;
      ++mid;
   }

   SegIter result = first;
   shift_scan(mid, last, result, segmented_iterator_tag());
   return result;
}

template <class FwdIt, class Sent, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value, FwdIt>::type
segmented_shift_left_dispatch
   (FwdIt first, Sent last,
    typename boost::container::iterator_traits<FwdIt>::difference_type n,
    Tag)
{
   typedef typename boost::container::iterator_traits<FwdIt>::difference_type difference_type;
   if(n <= 0) return last;

   FwdIt mid = first;
   for(difference_type i = 0; i < n; ++i) {
      if(mid == last) return first;
      ++mid;
   }

   FwdIt result = first;
   for(; mid != last; ++mid, ++result)
      *result = boost::move(*mid);
   return result;
}

} // namespace detail_algo

//! Shifts elements in [first, last) left by \c n positions.
//! Returns iterator to the new end.
template <class FwdIt, class Sent>
inline FwdIt segmented_shift_left
   (FwdIt first, Sent last,
    typename boost::container::iterator_traits<FwdIt>::difference_type n)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_shift_left_dispatch(first, last, n,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SHIFT_LEFT_HPP
