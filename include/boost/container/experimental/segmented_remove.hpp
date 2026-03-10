//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_HPP

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

namespace boost {
namespace container {

template <class FwdIt, class Sent, class T>
FwdIt segmented_remove(FwdIt first, Sent last, const T& value);

namespace detail_algo {

template <class FwdIt, class OutIter, class T>
void remove_scan(FwdIt first, FwdIt last, OutIter& result, const T& value, non_segmented_iterator_tag)
{
   for(; first != last; ++first) {
      if(!(*first == value)) {
         *result = boost::move(*first);
         ++result;
      }
   }
}

template <class SegIt, class OutIter, class T>
void remove_scan(SegIt first, SegIt last, OutIter& result, const T& value, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt> traits;
   typedef typename traits::local_iterator local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typename traits::segment_iterator scur = traits::segment(first);
   typename traits::segment_iterator slast = traits::segment(last);
   local_iterator lcur = traits::local(first);
   if(scur == slast) {
      remove_scan(lcur, traits::local(last), result, value, is_local_seg_t());
   }
   else {
      remove_scan(lcur, traits::end(scur), result, value,
         is_local_seg_t());
      for(++scur; scur != slast; ++scur)
         remove_scan(traits::begin(scur), traits::end(scur), result, value,
            is_local_seg_t());
      remove_scan(traits::begin(scur), traits::local(last), result, value,
         is_local_seg_t());
   }
}

template <class SegIter, class T>
SegIter segmented_remove_dispatch
   (SegIter first, SegIter last, const T& value, segmented_iterator_tag)
{
   SegIter result = first;
   remove_scan(first, last, result, value, segmented_iterator_tag());
   return result;
}

template <class FwdIt, class Sent, class T, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value, FwdIt>::type
segmented_remove_dispatch
   (FwdIt first, Sent last, const T& value, Tag)
{
   FwdIt result = first;
   for(; first != last; ++first) {
      if(!(*first == value)) {
         if(result != first)
            *result = boost::move(*first);
         ++result;
      }
   }
   return result;
}

} // namespace detail_algo

//! Removes all elements equal to \c value from [first, last),
//! moving retained elements forward. Returns iterator to new end.
template <class FwdIt, class Sent, class T>
inline FwdIt segmented_remove(FwdIt first, Sent last, const T& value)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_remove_dispatch(first, last, value,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_HPP
