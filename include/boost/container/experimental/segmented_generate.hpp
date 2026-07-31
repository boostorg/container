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
#include <boost/container/experimental/detail/segmented_common_algo.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/move/utility_core.hpp>

namespace boost {
namespace container {

template <class FwdIt, class Sent, class Generator>
void segmented_generate(FwdIt first, Sent last, Generator gen);

namespace detail_algo {

template <class FwdIt, class Sent, class Generator, class Tag, class Cat>
 BOOST_CONTAINER_FORCEINLINE typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value>::type
segmented_generate_dispatch(FwdIt first, const Sent last, Generator & BOOST_RESTRICT gen, Tag, Cat)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first)
      *first = gen();
}

template <class SegIter, class Generator, class Cat>
void segmented_generate_dispatch
   (SegIter first, SegIter last, Generator & BOOST_RESTRICT gen, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>   traits;
   typedef typename traits::local_iterator      local_iterator;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   segment_iterator sfirst = traits::segment(first);
   segment_iterator slast  = traits::segment(last);

   local_iterator lb = traits::local(first);

   for(;;) {
      const bool last_seg = sfirst == slast;
      const local_iterator le = last_seg ? traits::local(last) : traits::end(sfirst);
      (segmented_generate_dispatch)(lb, le, gen, is_local_seg_t(), local_cat_t());
      if(BOOST_UNLIKELY(last_seg))
         break;

      for(++sfirst; sfirst != slast; ++sfirst)
         (segmented_generate_dispatch)(traits::begin(sfirst), traits::end(sfirst), gen, is_local_seg_t(), local_cat_t());

      lb = traits::begin(sfirst);
   }
}

} // namespace detail_algo

//! Assigns the result of successive calls to \c gen to each
//! element in [first, last).
//! Generator state is preserved across segment boundaries.
template <class FwdIt, class Sent, class Generator>
BOOST_CONTAINER_FORCEINLINE
void segmented_generate(FwdIt first, Sent last, Generator gen)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   detail_algo::segmented_generate_dispatch
      (first, last, gen, typename traits::is_segmented_iterator(), typename iterator_traits<FwdIt>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_HPP
