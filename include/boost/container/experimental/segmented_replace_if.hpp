//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REPLACE_IF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REPLACE_IF_HPP

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

template <class FwdIt, class Sent, class Pred, class T>
void segmented_replace_if(FwdIt first, Sent last, Pred pred, const T& new_val);

namespace detail_algo {

template <class FwdIt, class Sent, class Pred, class T, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, FwdIt>::value>::type
segmented_replace_if_dispatch(FwdIt first, Sent last, Pred pred, const T& new_val, Tag)
{
   for(; first != last; ++first)
      if(pred(*first))
         *first = new_val;
}

template <class SegIter, class Pred, class T>
void segmented_replace_if_dispatch
   (SegIter first, SegIter last, Pred pred, const T& new_val, segmented_iterator_tag)
{

   typedef segmented_iterator_traits<SegIter>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator sfirst = traits::segment(first);
   segment_iterator slast  = traits::segment(last);

   if(sfirst == slast) {
      (segmented_replace_if_dispatch)(traits::local(first), traits::local(last), pred, new_val, is_local_seg_t());
   }
   else {
      (segmented_replace_if_dispatch)(traits::local(first), traits::end(sfirst), pred, new_val, is_local_seg_t());

      for(++sfirst; sfirst != slast; ++sfirst)
         (segmented_replace_if_dispatch)(traits::begin(sfirst), traits::end(sfirst), pred, new_val, is_local_seg_t());

      (segmented_replace_if_dispatch)(traits::begin(sfirst), traits::local(last), pred, new_val, is_local_seg_t());
   }
}

} // namespace detail_algo

//! Replaces every element satisfying \c pred with \c new_val in [first, last).
template <class FwdIt, class Sent, class Pred, class T>
BOOST_CONTAINER_FORCEINLINE
void segmented_replace_if(FwdIt first, Sent last, Pred pred, const T& new_val)
{
   typedef segmented_iterator_traits<FwdIt> traits;
   detail_algo::segmented_replace_if_dispatch
      (first, last, pred, new_val, typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REPLACE_IF_HPP
