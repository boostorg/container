//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_N_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_N_HPP

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

template <class OutIt, class Size, class Generator>
OutIt segmented_generate_n(OutIt first, Size count, Generator gen);

namespace detail_algo {

template <class OutIter, class Size, class Generator>
OutIter generate_n_scan_non_segmented(OutIter first, OutIter last, Size& count, Generator &gen, const std::random_access_iterator_tag &)
{
   std::size_t range_sz = static_cast<std::size_t>(last - first);
   const Size local_count = (std::size_t)count < range_sz ? count : (Size)range_sz;

   for(Size cnt = local_count; cnt; ++first, --cnt)
      *first = gen();

   count -= local_count;
   return first;
}

template <class OutIter, class Size, class Generator, class Tag>
OutIter generate_n_scan_non_segmented(OutIter first, OutIter last, Size& count, Generator &gen, Tag)
{
   Size local_count = count;  //Avoid aliasing the count parameter

   for (; local_count > 0 && first != last; ++first, --local_count)
      *first = gen();

   count = local_count;  //Restore the count parameter

   return first;
}

template <class OutIter, class Size, class Generator>
BOOST_CONTAINER_FORCEINLINE
OutIter generate_n_scan(OutIter first, OutIter last, Size& count, Generator &gen, non_segmented_iterator_tag)
{
   return (generate_n_scan_non_segmented)(first, last, count, gen, typename iterator_traits<OutIter>::iterator_category());
}

template <class SegIt, class Size, class Generator>
SegIt generate_n_scan(SegIt first, SegIt last, Size& count, Generator& gen, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);

   if(scur == slast) {
      return traits::compose(scur, generate_n_scan(traits::local(first), traits::local(last), count, gen, is_local_seg_t()));
   }
   else {
      local_iterator lcur = generate_n_scan(traits::local(first), traits::end(scur), count, gen, is_local_seg_t());
      for(++scur; scur != slast && count > 0; ++scur)
         lcur = generate_n_scan(traits::begin(scur), traits::end(scur), count, gen, is_local_seg_t());
      if(count > 0)
         lcur = generate_n_scan(traits::begin(slast), traits::local(last), count, gen, is_local_seg_t());
      return traits::compose(scur, lcur);
   }
   
}

template <class SegIter, class Size, class Generator>
SegIter segmented_generate_n_ref
   (SegIter first, Size count, Generator& gen, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator scur = traits::segment(first);
   local_iterator   lcur = traits::local(first);

   //Iterate through the segments, until the count is 0
   while(1) {
      lcur = generate_n_scan(lcur, traits::end(scur), count, gen, is_local_seg_t());

      if(count == 0)
         break;
      ++scur;
      lcur = traits::begin(scur);
   }
   return traits::compose(scur, lcur);
}

template <class OutIt, class Size, class Generator>
OutIt segmented_generate_n_ref
   (OutIt first, Size count, Generator& gen, non_segmented_iterator_tag)
{
   for(; count > 0; ++first, --count)
      *first = gen();
   return first;
}

} // namespace detail_algo

//! Assigns the result of successive calls to \c gen to the first
//! \c count elements starting at \c first. Generator state is
//! preserved across segment boundaries.
//! Returns an iterator past the last generated element.
template <class OutIt, class Size, class Generator>
BOOST_CONTAINER_FORCEINLINE
OutIt segmented_generate_n(OutIt first, Size count, Generator gen)
{
   typedef segmented_iterator_traits<OutIt> traits;
   return detail_algo::segmented_generate_n_ref(first, count, gen,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_N_HPP
