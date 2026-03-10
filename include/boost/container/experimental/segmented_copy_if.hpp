//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_IF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_IF_HPP

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

template <class InIter, class Sent, class OutIter, class Pred>
OutIter segmented_copy_if(InIter first, Sent last, OutIter result, Pred pred);

namespace detail_algo {

template <class SegIter, class OutIter, class Pred>
OutIter segmented_copy_if_dispatch
   (SegIter first, SegIter last, OutIter result, Pred pred, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);
   if(sfirst == slast) {
      result = boost::container::segmented_copy_if(traits::local(first), traits::local(last), result, pred);
   }
   else {
      result = boost::container::segmented_copy_if(traits::local(first), traits::end(sfirst), result, pred);
      for(++sfirst; sfirst != slast; ++sfirst)
         result = boost::container::segmented_copy_if(traits::begin(sfirst), traits::end(sfirst), result, pred);
      result = boost::container::segmented_copy_if(traits::begin(sfirst), traits::local(last), result, pred);
   }
   return result;
}

template <class InIter, class Sent, class OutIter, class Pred, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InIter>::value, OutIter>::type
segmented_copy_if_dispatch
   (InIter first, Sent last, OutIter result, Pred pred, Tag)
{
   for(; first != last; ++first)
      if(pred(*first)) {
         *result = *first;
         ++result;
      }
   return result;
}

} // namespace detail_algo

//! Copies elements satisfying \c pred from [first, last) to the range
//! beginning at \c result. Segmentation is exploited on the input range.
template <class InIter, class Sent, class OutIter, class Pred>
inline OutIter segmented_copy_if(InIter first, Sent last, OutIter result, Pred pred)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_copy_if_dispatch(first, last, result, pred,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COPY_IF_HPP
