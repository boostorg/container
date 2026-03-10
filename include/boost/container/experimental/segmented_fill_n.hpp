//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_N_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_N_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Size, class T>
FwdIt segmented_fill_n(FwdIt first, Size count, const T& value);

namespace detail_algo {

template <class FwdIt, class Size, class T>
FwdIt fill_n_scan(FwdIt first, FwdIt last, Size& count, const T& value, non_segmented_iterator_tag)
{
   for(; first != last && count > 0; ++first, --count)
      *first = value;
   return first;
}

template <class SegIt, class Size, class T>
SegIt fill_n_scan(SegIt first, SegIt last, Size& count, const T& value, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt> traits;
   typedef typename traits::local_iterator local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typename traits::segment_iterator scur = traits::segment(first);
   typename traits::segment_iterator slast = traits::segment(last);
   local_iterator lcur = traits::local(first);
   if(scur == slast) {
      lcur = fill_n_scan(lcur, traits::local(last), count, value, is_local_seg_t());
   }
   else {
      lcur = fill_n_scan(lcur, traits::end(scur), count, value,
         is_local_seg_t());
      for(++scur; scur != slast && count > 0; ++scur)
         lcur = fill_n_scan(traits::begin(scur), traits::end(scur), count, value,
            is_local_seg_t());
      if(count > 0)
         lcur = fill_n_scan(traits::begin(scur), traits::local(last), count, value,
            is_local_seg_t());
   }
   return traits::compose(scur, lcur);
}

template <class SegIter, class Size, class T>
SegIter segmented_fill_n_dispatch
   (SegIter first, Size count, const T& value, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   if(count <= 0) return first;
   typename traits::segment_iterator scur = traits::segment(first);
   typename traits::local_iterator lcur = traits::local(first);
   while(count > 0) {
      typename traits::local_iterator lend = traits::end(scur);
      lcur = fill_n_scan(lcur, lend, count, value,
         typename segmented_iterator_traits<typename traits::local_iterator>::is_segmented_iterator());
      if(count > 0) {
         ++scur;
         lcur = traits::begin(scur);
      }
   }
   return traits::compose(scur, lcur);
}

template <class FwdIt, class Size, class T>
FwdIt segmented_fill_n_dispatch
   (FwdIt first, Size count, const T& value, non_segmented_iterator_tag)
{
   for(; count > 0; ++first, --count)
      *first = value;
   return first;
}

} // namespace detail_algo

//! Assigns \c value to the first \c count elements starting at \c first.
//! Returns an iterator past the last filled element.
//! Exploits segmentation when available.
template <class FwdIt, class Size, class T>
inline FwdIt segmented_fill_n(FwdIt first, Size count, const T& value)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_fill_n_dispatch(first, count, value,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_N_HPP
