//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_HPP

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

template <class InIter, class Sent, class OutIter>
OutIter segmented_copy(InIter first, Sent last, OutIter result);

namespace detail_algo {

template <class SegIter, class OutIter>
OutIter segmented_copy_dispatch(SegIter first, SegIter last, OutIter result, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);

   if(sfirst == slast) {
      return boost::container::segmented_copy(traits::local(first), traits::local(last), result);
   }
   else {
      result = boost::container::segmented_copy(traits::local(first), traits::end(sfirst), result);

      for(++sfirst; sfirst != slast; ++sfirst)
         result = boost::container::segmented_copy(traits::begin(sfirst), traits::end(sfirst), result);

      return boost::container::segmented_copy(traits::begin(sfirst), traits::local(last), result);
   }
}

template <class InIter, class Sent, class OutIter, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InIter>::value, OutIter>::type
   segmented_copy_dispatch(InIter first, Sent last, OutIter result, Tag)
{
   for(; first != last; ++first, ++result)
      *result = *first;
   return result;
}

} // namespace detail_algo

//! Copies elements from [first, last) to the range beginning at \c result.
//! Segmentation is exploited on the input range only.
template <class InIter, class Sent, class OutIter>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_copy(InIter first, Sent last, OutIter result)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_copy_dispatch(first, last, result,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_HPP
