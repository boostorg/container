//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_TRANSFORM_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_TRANSFORM_HPP

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

template <class InIter, class Sent, class OutIter, class UnaryOp>
OutIter segmented_transform(InIter first, Sent last, OutIter result, UnaryOp op);

namespace detail_algo {

template <class SegIter, class OutIter, class UnaryOp>
OutIter segmented_transform_dispatch
   (SegIter first, SegIter last, OutIter result, UnaryOp op, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);

   if(sfirst == slast) {
      return boost::container::segmented_transform(traits::local(first), traits::local(last), result, op);
   }
   else {
      result = boost::container::segmented_transform(traits::local(first), traits::end(sfirst), result, op);

      for(++sfirst; sfirst != slast; ++sfirst)
         result = boost::container::segmented_transform(traits::begin(sfirst), traits::end(sfirst), result, op);

      return boost::container::segmented_transform(traits::begin(sfirst), traits::local(last), result, op);
   }
}

template <class InIter, class Sent, class OutIter, class UnaryOp, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InIter>::value, OutIter>::type
segmented_transform_dispatch
   (InIter first, Sent last, OutIter result, UnaryOp op, Tag)
{
   for(; first != last; ++first, ++result)
      *result = op(*first);
   return result;
}

} // namespace detail_algo

//! Applies \c op to each element in [first, last) and writes the result
//! to the range beginning at \c result.
//! Segmentation is exploited on the input range only.
template <class InIter, class Sent, class OutIter, class UnaryOp>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_transform (InIter first, Sent last, OutIter result, UnaryOp op)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_transform_dispatch
      (first, last, result, op, typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_TRANSFORM_HPP
