//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REPLACE_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REPLACE_HPP

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

template <class FwdIt, class Sent, class T>
void segmented_replace(FwdIt first, Sent last, const T& old_val, const T& new_val);

namespace detail_algo {

template <class SegIter, class T>
void segmented_replace_dispatch
   (SegIter first, SegIter last, const T& old_val, const T& new_val, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);
   if(sfirst == slast) {
      boost::container::segmented_replace(traits::local(first), traits::local(last), old_val, new_val);
   }
   else {
      boost::container::segmented_replace(traits::local(first), traits::end(sfirst), old_val, new_val);
      for(++sfirst; sfirst != slast; ++sfirst)
         boost::container::segmented_replace(traits::begin(sfirst), traits::end(sfirst), old_val, new_val);
      boost::container::segmented_replace(traits::begin(sfirst), traits::local(last), old_val, new_val);
   }
}

template <class FwdIt, class Sent, class T, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value>::type
segmented_replace_dispatch
   (FwdIt first, Sent last, const T& old_val, const T& new_val, Tag)
{
   for(; first != last; ++first)
      if(*first == old_val)
         *first = new_val;
}

} // namespace detail_algo

//! Replaces every occurrence of \c old_val with \c new_val in [first, last).
template <class FwdIt, class Sent, class T>
inline void segmented_replace
   (FwdIt first, Sent last, const T& old_val, const T& new_val)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   detail_algo::segmented_replace_dispatch(first, last, old_val, new_val,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REPLACE_HPP
