//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_N_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_N_HPP

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

template <class InIter, class Size, class OutIter>
OutIter segmented_copy_n(InIter first, Size count, OutIter result);

namespace detail_algo {

template <class InIter, class Size, class OutIter>
OutIter copy_n_scan(InIter first, InIter last, Size& count, OutIter result, non_segmented_iterator_tag)
{
   Size local_count = count;  //Avoid aliasing the count parameter
   for(; first != last && local_count > 0; ++first, ++result, --local_count)
      *result = *first;
   count = local_count;  //Restore the count parameter
   return result;
}

template <class SegIt, class Size, class OutIter>
OutIter copy_n_scan(SegIt first, SegIt last, Size& count, OutIter result, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt> traits;
   typedef typename traits::local_iterator local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   typename traits::segment_iterator scur  = traits::segment(first);
   typename traits::segment_iterator slast = traits::segment(last);
   local_iterator lcur = traits::local(first);

   if(scur == slast) {
      result = copy_n_scan(lcur, traits::local(last), count, result, is_local_seg_t());
   }
   else {
      result = copy_n_scan(lcur, traits::end(scur), count, result, is_local_seg_t());
      for(++scur; scur != slast && count > 0; ++scur)
         result = copy_n_scan(traits::begin(scur), traits::end(scur), count, result, is_local_seg_t());
      if(count > 0)
         result = copy_n_scan(traits::begin(scur), traits::local(last), count, result, is_local_seg_t());
   }
   return result;
}

template <class SegIter, class Size, class OutIter>
OutIter segmented_copy_n_dispatch
   (SegIter first, Size count, OutIter result, segmented_iterator_tag)
{
   if(count <= 0) return result;

   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   typename traits::segment_iterator scur = traits::segment(first);
   local_iterator lcur = traits::local(first);

   //Iterate through the segments, until the count is 0
   while(1) {
      result = copy_n_scan(lcur, traits::end(scur), count, result, is_local_seg_t());

      if(count == 0)
         break;
      ++scur;
      lcur = traits::begin(scur);
   }
   return result;
}

template <class InIter, class Size, class OutIter>
OutIter segmented_copy_n_dispatch
   (InIter first, Size count, OutIter result, non_segmented_iterator_tag)
{
   for(; count > 0; ++first, ++result, --count)
      *result = *first;
   return result;
}

} // namespace detail_algo

//! Copies \c count elements from the range beginning at \c first to
//! the range beginning at \c result. Exploits segmentation on input.
template <class InIter, class Size, class OutIter>
inline OutIter segmented_copy_n(InIter first, Size count, OutIter result)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_copy_n_dispatch(first, count, result,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_N_HPP
