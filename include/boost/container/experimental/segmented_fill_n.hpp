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
#include <boost/container/detail/iterators.hpp>
#include <boost/container/detail/std_fwd.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Size, class T>
FwdIt segmented_fill_n(FwdIt first, Size count, const T& value);

namespace detail_algo {

template <class OutIter, class Size, class T>
OutIter fill_n_scan_non_segmented(OutIter first, OutIter last, Size& count, const T& value, const std::random_access_iterator_tag &)
{
   std::size_t range_sz = static_cast<std::size_t>(last - first);
   const Size local_count = (std::size_t)count < range_sz ? count : (Size)range_sz;

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)
   Size cnt = local_count;
   while(cnt >= Size(4)) {
      *first = value; ++first;
      *first = value; ++first;
      *first = value; ++first;
      *first = value; ++first;
      cnt -= Size(4);
   }

   switch(cnt) {
      case 3:
         *first = value; ++first;
         BOOST_FALLTHROUGH;
      case 2:
         *first = value; ++first;
         BOOST_FALLTHROUGH;
      case 1:
         *first = value; ++first;
         BOOST_FALLTHROUGH;
      default:
         break;
   }
#else
   for(Size cnt = local_count; cnt; ++first, --cnt)
      *first = value;
#endif

   count -= local_count;
   return first;
}

template <class OutIter, class Size, class T, class Tag>
OutIter fill_n_scan_non_segmented(OutIter first, OutIter last, Size& count, const T& value, Tag)
{
   Size local_count = count;

   for (; local_count > 0 && first != last; ++first, --local_count)
      *first = value;

   count = local_count;

   return first;
}

template <class OutIter, class Size, class T>
BOOST_CONTAINER_FORCEINLINE
OutIter fill_n_scan(OutIter first, OutIter last, Size& count, const T& value, non_segmented_iterator_tag)
{
   return (fill_n_scan_non_segmented)(first, last, count, value, typename iterator_traits<OutIter>::iterator_category());
}

template <class SegIt, class Size, class T>
SegIt fill_n_scan(SegIt first, SegIt last, Size& count, const T& value, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator       scur  = traits::segment(first);
   segment_iterator const slast = traits::segment(last);

   if(scur == slast) {
      const local_iterator ll = traits::local(last);
      const local_iterator r = (fill_n_scan)(traits::local(first), ll, count, value, is_local_seg_t());
      return (r != ll) ? traits::compose(scur, r) : last;
   }
   else {
      local_iterator r = fill_n_scan(traits::local(first), traits::end(scur), count, value, is_local_seg_t());
      if (!count)
         return traits::compose(scur, r);

      for (++scur; scur != slast; ++scur) {
         r = fill_n_scan(traits::begin(scur), traits::end(scur), count, value, is_local_seg_t());
         if (!count)
            return traits::compose(scur, r);
      }
      const local_iterator ll = traits::local(last);
      r = fill_n_scan(traits::begin(slast), ll, count, value, is_local_seg_t());
      return (r != ll) ? traits::compose(scur, r) : last;
   }
}

template <class SegIter, class Size, class T>
SegIter segmented_fill_n_ref
   (SegIter first, Size count, const T& value, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator scur = traits::segment(first);
   local_iterator   lcur = traits::local(first);

   while(1) {
      lcur = fill_n_scan(lcur, traits::end(scur), count, value, is_local_seg_t());

      if(count == 0)
         break;
      ++scur;
      lcur = traits::begin(scur);
   }
   return traits::compose(scur, lcur);
}

template <class OutIt, class Size, class T>
OutIt segmented_fill_n_ref
   (OutIt first, Size count, const T& value, non_segmented_iterator_tag)
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
BOOST_CONTAINER_FORCEINLINE
FwdIt segmented_fill_n(FwdIt first, Size count, const T& value)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_fill_n_ref(first, count, value,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_N_HPP
