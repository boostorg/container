//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_IS_PARTITIONED_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_IS_PARTITIONED_HPP

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

template <class InpIter, class Sent, class Pred>
bool segmented_is_partitioned(InpIter first, Sent last, Pred pred);

namespace detail_algo {

template <class InpIter, class Sent, class Pred>
bool is_partitioned_with_state
   (InpIter first, Sent last, Pred pred, bool& in_true_part)
{
   for(; first != last; ++first) {
      if(in_true_part) {
         if(!pred(*first))
            in_true_part = false;
      }
      else {
         if(pred(*first))
            return false;
      }
   }
   return true;
}

// Non-recursive: must carry cross-segment state (in_true_part) to detect
// the transition from true to false partition across segment boundaries.
template <class SegIter, class Pred>
bool segmented_is_partitioned_dispatch
   (SegIter first, SegIter last, Pred pred, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);
   bool in_true_part = true;
   if(sfirst == slast) {
      return detail_algo::is_partitioned_with_state(
         traits::local(first), traits::local(last), pred, in_true_part);
   }
   else {
      if(!detail_algo::is_partitioned_with_state(
            traits::local(first), traits::end(sfirst), pred, in_true_part))
         return false;
      for(++sfirst; sfirst != slast; ++sfirst)
         if(!detail_algo::is_partitioned_with_state(
               traits::begin(sfirst), traits::end(sfirst), pred, in_true_part))
            return false;
      return detail_algo::is_partitioned_with_state(
         traits::begin(sfirst), traits::local(last), pred, in_true_part);
   }
}

template <class InpIter, class Sent, class Pred, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InpIter>::value, bool>::type
segmented_is_partitioned_dispatch
   (InpIter first, Sent last, Pred pred, Tag)
{
   bool in_true_part = true;
   return detail_algo::is_partitioned_with_state(first, last, pred, in_true_part);
}

} // namespace detail_algo

//! Returns \c true if all elements satisfying \c pred appear before
//! all elements that do not, i.e. the range [first, last) is
//! partitioned with respect to \c pred.
//! Exploits segmentation when available.
template <class InpIter, class Sent, class Pred>
inline bool segmented_is_partitioned(InpIter first, Sent last, Pred pred)
{
   typedef segmented_iterator_traits<InpIter> traits;
   return detail_algo::segmented_is_partitioned_dispatch(first, last, pred,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_IS_PARTITIONED_HPP
