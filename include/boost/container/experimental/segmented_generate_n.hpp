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

namespace boost {
namespace container {

template <class FwdIt, class Size, class Generator>
FwdIt segmented_generate_n(FwdIt first, Size count, Generator gen);

namespace detail_algo {

template <class FwdIt, class Size, class Generator>
FwdIt generate_n_scan(FwdIt first, FwdIt last, Size& count, Generator& gen, non_segmented_iterator_tag)
{
   for(; first != last && count > 0; ++first, --count)
      *first = gen();
   return first;
}

template <class SegIt, class Size, class Generator>
SegIt generate_n_scan(SegIt first, SegIt last, Size& count, Generator& gen, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt> traits;
   typedef typename traits::local_iterator local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typename traits::segment_iterator scur = traits::segment(first);
   typename traits::segment_iterator slast = traits::segment(last);
   local_iterator lcur = traits::local(first);
   if(scur == slast) {
      lcur = generate_n_scan(lcur, traits::local(last), count, gen, is_local_seg_t());
   }
   else {
      lcur = generate_n_scan(lcur, traits::end(scur), count, gen,
         is_local_seg_t());
      for(++scur; scur != slast && count > 0; ++scur)
         lcur = generate_n_scan(traits::begin(scur), traits::end(scur), count, gen,
            is_local_seg_t());
      if(count > 0)
         lcur = generate_n_scan(traits::begin(scur), traits::local(last), count, gen,
            is_local_seg_t());
   }
   return traits::compose(scur, lcur);
}

template <class SegIter, class Size, class Generator>
SegIter segmented_generate_n_ref
   (SegIter first, Size count, Generator& gen, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   if(count <= 0) return first;
   typename traits::segment_iterator scur = traits::segment(first);
   typename traits::local_iterator lcur = traits::local(first);
   while(count > 0) {
      typename traits::local_iterator lend = traits::end(scur);
      lcur = generate_n_scan(lcur, lend, count, gen,
         typename segmented_iterator_traits<typename traits::local_iterator>::is_segmented_iterator());
      if(count > 0) {
         ++scur;
         lcur = traits::begin(scur);
      }
   }
   return traits::compose(scur, lcur);
}

template <class FwdIt, class Size, class Generator>
FwdIt segmented_generate_n_ref
   (FwdIt first, Size count, Generator& gen, non_segmented_iterator_tag)
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
template <class FwdIt, class Size, class Generator>
inline FwdIt segmented_generate_n(FwdIt first, Size count, Generator gen)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   return detail_algo::segmented_generate_n_ref(first, count, gen,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_N_HPP
