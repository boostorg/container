//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FOR_EACH_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FOR_EACH_HPP

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

template <class InpIter, class Sent, class F>
F segmented_for_each(InpIter first, Sent last, F f);

namespace detail_algo {

template <class SegIter, class F>
F segmented_for_each_dispatch
   (SegIter first, SegIter last, F f, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);
   if(sfirst == slast) {
      f = boost::container::segmented_for_each(traits::local(first), traits::local(last), f);
   }
   else {
      f = boost::container::segmented_for_each(traits::local(first), traits::end(sfirst), f);
      for(++sfirst; sfirst != slast; ++sfirst)
         f = boost::container::segmented_for_each(traits::begin(sfirst), traits::end(sfirst), f);
      f = boost::container::segmented_for_each(traits::begin(sfirst), traits::local(last), f);
   }
   return f;
}

template <class InpIter, class Sent, class F, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InpIter>::value, F>::type
segmented_for_each_dispatch
   (InpIter first, Sent last, F f, Tag)
{
   for(; first != last; ++first)
      f(*first);
   return f;
}

} // namespace detail_algo

//! Applies \c f to every element in [first, last).
//! Returns the (possibly modified) function object.
//! When \c Iter is a segmented iterator, exploits segmentation
//! to reduce per-element overhead.
template <class InpIter, class Sent, class F>
inline F segmented_for_each(InpIter first, Sent last, F f)
{
   typedef segmented_iterator_traits<InpIter> traits;
   return detail_algo::segmented_for_each_dispatch(first, last, f,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FOR_EACH_HPP
