//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_COPY_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_COPY_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <utility>

namespace boost {
namespace container {

template <class InIter, class Sent, class OutIter1, class OutIter2, class Pred>
std::pair<OutIter1, OutIter2>
segmented_partition_copy(InIter first, Sent last, OutIter1 out_true, OutIter2 out_false, Pred pred);

namespace detail_algo {

template <class SegIter, class OutIter1, class OutIter2, class Pred>
std::pair<OutIter1, OutIter2>
segmented_partition_copy_dispatch
   (SegIter first, SegIter last, OutIter1 out_true, OutIter2 out_false, Pred pred, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef std::pair<OutIter1, OutIter2> pair_t;

   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);

   if(sfirst == slast) {
      pair_t p = (segmented_partition_copy)(traits::local(first), traits::local(last), out_true, out_false, pred);
      out_true  = p.first;
      out_false = p.second;
   }
   else {
      {
         pair_t p = (segmented_partition_copy)(traits::local(first), traits::end(sfirst), out_true, out_false, pred);
         out_true  = p.first;
         out_false = p.second;
      }
      for(++sfirst; sfirst != slast; ++sfirst) {
         pair_t p = (segmented_partition_copy)(traits::begin(sfirst), traits::end(sfirst), out_true, out_false, pred);
         out_true  = p.first;
         out_false = p.second;
      }
      {
         pair_t p = (segmented_partition_copy)(traits::begin(sfirst), traits::local(last), out_true, out_false, pred);
         out_true  = p.first;
         out_false = p.second;
      }
   }
   return pair_t(out_true, out_false);
}

template <class InIter, class Sent, class OutIter1, class OutIter2, class Pred, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InIter>::value, std::pair<OutIter1, OutIter2> >::type
segmented_partition_copy_dispatch
   (InIter first, Sent last, OutIter1 out_true, OutIter2 out_false, Pred pred, Tag)
{
   for(; first != last; ++first) {
      if(pred(*first)) {
         *out_true = *first;
         ++out_true;
      }
      else {
         *out_false = *first;
         ++out_false;
      }
   }
   return std::pair<OutIter1, OutIter2>(out_true, out_false);
}

} // namespace detail_algo

//! Copies elements from [first, last) to one of two output ranges
//! depending on whether \c pred returns true or false.
//! Returns a pair of output iterators past the last elements written.
template <class InIter, class Sent, class OutIter1, class OutIter2, class Pred>
BOOST_CONTAINER_FORCEINLINE
std::pair<OutIter1, OutIter2>
segmented_partition_copy(InIter first, Sent last, OutIter1 out_true, OutIter2 out_false, Pred pred)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_partition_copy_dispatch
      (first, last, out_true, out_false, pred, typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_COPY_HPP
