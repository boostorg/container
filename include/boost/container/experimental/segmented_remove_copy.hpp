//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_COPY_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_COPY_HPP

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

template <class InIter, class Sent, class OutIter, class T>
OutIter segmented_remove_copy(InIter first, Sent last, OutIter result, const T& value);

namespace detail_algo {

template <class SegIter, class OutIter, class T>
OutIter segmented_remove_copy_dispatch
   (SegIter first, SegIter last, OutIter result, const T& value, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);
   if(sfirst == slast) {
      result = boost::container::segmented_remove_copy(traits::local(first), traits::local(last), result, value);
   }
   else {
      result = boost::container::segmented_remove_copy(traits::local(first), traits::end(sfirst), result, value);
      for(++sfirst; sfirst != slast; ++sfirst)
         result = boost::container::segmented_remove_copy(traits::begin(sfirst), traits::end(sfirst), result, value);
      result = boost::container::segmented_remove_copy(traits::begin(sfirst), traits::local(last), result, value);
   }
   return result;
}

template <class InIter, class Sent, class OutIter, class T, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InIter>::value, OutIter>::type
segmented_remove_copy_dispatch
   (InIter first, Sent last, OutIter result, const T& value, Tag)
{
   for(; first != last; ++first) {
      if(!(*first == value)) {
         *result = *first;
         ++result;
      }
   }
   return result;
}

} // namespace detail_algo

//! Copies elements from [first, last) to the range beginning at \c result,
//! skipping elements equal to \c value. Returns the output iterator past
//! the last element written.
template <class InIter, class Sent, class OutIter, class T>
inline OutIter segmented_remove_copy(InIter first, Sent last, OutIter result, const T& value)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_remove_copy_dispatch(first, last, result, value,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_COPY_HPP
