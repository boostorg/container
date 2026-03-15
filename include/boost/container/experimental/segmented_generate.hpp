//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_HPP

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

template <class FwdIt, class Sent, class Generator>
void segmented_generate(FwdIt first, Sent last, Generator gen);

namespace detail_algo {

// Uses a by-reference inner loop to preserve generator state
// across segments (std::generate takes generators by value).
template <class FwdIt, class Sent, class Generator>
void generate_ref(FwdIt first, const Sent last, Generator& gen)
{
   for(; first != last; ++first)
      *first = gen();
}

template <class SegIter, class Generator>
void segmented_generate_ref
   (SegIter first, SegIter last, Generator& gen, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);
   if(sfirst == slast) {
      detail_algo::generate_ref(traits::local(first), traits::local(last), gen);
   }
   else {
      detail_algo::generate_ref(traits::local(first), traits::end(sfirst), gen);
      for(++sfirst; sfirst != slast; ++sfirst)
         detail_algo::generate_ref(traits::begin(sfirst), traits::end(sfirst), gen);
      detail_algo::generate_ref(traits::begin(sfirst), traits::local(last), gen);
   }
}

template <class FwdIt, class Sent, class Generator, class Tag>
 BOOST_CONTAINER_FORCEINLINE typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value>::type
segmented_generate_ref(FwdIt first, Sent last, Generator& gen, Tag)
{
   return (generate_ref)(first, last, gen);
}

} // namespace detail_algo

//! Assigns the result of successive calls to \c gen to each
//! element in [first, last).
//! Generator state is preserved across segment boundaries.
template <class FwdIt, class Sent, class Generator>
inline void segmented_generate(FwdIt first, Sent last, Generator gen)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   detail_algo::segmented_generate_ref(first, last, gen,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_HPP
