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
#include <boost/container/detail/std_fwd.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

template <class InIter, class Size, class OutIter>
OutIter segmented_copy_n(InIter first, Size count, OutIter result);

namespace detail_algo {

template <class InIter, class Size, class OutIter>
BOOST_CONTAINER_FORCEINLINE
OutIter copy_n_scan_non_segmented
   (InIter first, InIter last, Size& count, OutIter result, const std::random_access_iterator_tag &)
{
   Size range_sz = Size(last - first);
   Size local_count = count < range_sz ? count : range_sz;
   result = (segmented_copy_n)(first, local_count, result);
   count -= local_count;
   return result;
}

template <class InIter, class Size, class OutIter, class Tag>
OutIter copy_n_scan_non_segmented(InIter first, InIter last, Size& count, OutIter result, Tag)
{
   Size local_count = count;

   for(; local_count > 0 && first != last; ++first, ++result, --local_count)
      *result = *first;

   count = local_count;

   return result;
}

template <class InIter, class Size, class OutIter>
BOOST_CONTAINER_FORCEINLINE
OutIter copy_n_scan(InIter first, InIter last, Size& count, OutIter result, non_segmented_iterator_tag)
{
   return (copy_n_scan_non_segmented)(first, last, count, result, typename iterator_traits<InIter>::iterator_category());
}

template <class SegIt, class Size, class OutIter>
OutIter copy_n_scan(SegIt first, SegIt last, Size& count, OutIter result, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);
   local_iterator   lcur  = traits::local(first);

   if(scur == slast) {
      return copy_n_scan(lcur, traits::local(last), count, result, is_local_seg_t());
   }
   else {
      result = copy_n_scan(lcur, traits::end(scur), count, result, is_local_seg_t());

      for(++scur; scur != slast && count > 0; ++scur)
         result = copy_n_scan(traits::begin(scur), traits::end(scur), count, result, is_local_seg_t());

      return count ? copy_n_scan(traits::begin(scur), traits::local(last), count, result, is_local_seg_t()) : result;
   }
}

template <class SegIter, class Size, class OutIter, class Cat>
OutIter segmented_copy_n_dispatch
   (SegIter first, Size count, OutIter result, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   if(count <= 0) return result;

   segment_iterator scur = traits::segment(first);
   local_iterator   lcur = traits::local(first);

   while(1) {
      result = copy_n_scan(lcur, traits::end(scur), count, result, is_local_seg_t());

      if(count == 0)
         break;
      ++scur;
      lcur = traits::begin(scur);
   }
   return result;
}

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RAIter, class Size, class OutIter>
OutIter segmented_copy_n_dispatch
   (RAIter first, Size count, OutIter result, non_segmented_iterator_tag, const std::random_access_iterator_tag &)
{
   while(count >= Size(4)) {
      *result = *first; ++first; ++result;
      *result = *first; ++first; ++result;
      *result = *first; ++first; ++result;
      *result = *first; ++first; ++result;
      count -= Size(4);
   }

   switch(count) {
      case 3:
         *result = *first; ++first; ++result;
         *result = *first; ++first; ++result;
         *result = *first; ++first; ++result;
         break;
      case 2:
         *result = *first; ++first; ++result;
         *result = *first; ++first; ++result;
         break;
      case 1:
         *result = *first; ++first; ++result;
         break;
      default:
         break;
   }
   return result;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class InIter, class Size, class OutIter, class Cat>
OutIter segmented_copy_n_dispatch
   (InIter first, Size count, OutIter result, non_segmented_iterator_tag, Cat)
{
   for(; count > 0; ++first, ++result, --count)
      *result = *first;
   return result;
}

} // namespace detail_algo

//! Copies \c count elements from the range beginning at \c first to
//! the range beginning at \c result. Exploits segmentation on input.
template <class InIter, class Size, class OutIter>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_copy_n(InIter first, Size count, OutIter result)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_copy_n_dispatch(first, count, result,
      typename traits::is_segmented_iterator(), typename iterator_traits<InIter>::iterator_category());
}


} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_N_HPP
