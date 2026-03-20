//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MISMATCH_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MISMATCH_HPP

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

template <class InpIter1, class Sent, class InpIter2, class BinaryPred>
std::pair<InpIter1, InpIter2> segmented_mismatch(InpIter1 first1, Sent last1, InpIter2 first2, BinaryPred pred);

template <class InpIter1, class Sent, class InpIter2>
std::pair<InpIter1, InpIter2> segmented_mismatch(InpIter1 first1, Sent last1, InpIter2 first2);

namespace detail_algo {

struct mismatch_equal
{
   template <class T, class U>
   BOOST_CONTAINER_FORCEINLINE bool operator()(const T& a, const U& b) const { return a == b; }
};

template <class SegIter, class InpIter2, class BinaryPred>
std::pair<SegIter, InpIter2> segmented_mismatch_dispatch
   (SegIter first1, SegIter last1, InpIter2 first2, BinaryPred pred, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;

   typedef std::pair<SegIter, InpIter2>        return_t;
   typedef std::pair<local_iterator, InpIter2> local_return_t;

   segment_iterator sfirst = traits::segment(first1);
   segment_iterator slast  = traits::segment(last1);

   if(sfirst == slast) {
      const local_iterator lf = traits::local(first1);
      const local_iterator ll = traits::local(last1);
      const local_return_t r = (segmented_mismatch)(lf, ll, first2, pred);
      return return_t(traits::compose(sfirst, r.first), r.second);
   }

   // First segment
   {
      const local_iterator lf = traits::local(first1);
      const local_iterator le = traits::end(sfirst);
      const local_return_t r = (segmented_mismatch)(lf, le, first2, pred);
      if (r.first != le)
         return return_t(traits::compose(sfirst, r.first), r.second);
      first2 = r.second;
   }
   // Middle segments
   for(++sfirst; sfirst != slast; ++sfirst) {
      const local_iterator lb = traits::begin(sfirst);
      const local_iterator le = traits::end(sfirst);
      const local_return_t r = (segmented_mismatch)(lb, le, first2, pred);
      if (r.first != le)
         return return_t(traits::compose(sfirst, r.first), r.second);
      first2 = r.second;
   }
   // Last segment
   {
      const local_iterator lb = traits::begin(sfirst);
      const local_iterator ll = traits::local(last1);
      const local_return_t r = (segmented_mismatch)(lb, ll, first2, pred);
      return return_t(traits::compose(sfirst, r.first), r.second);
   }
}

template <class InpIter1, class Sent, class InpIter2, class BinaryPred, class Tag>
typename algo_enable_if_c
   < !Tag::value || is_sentinel<Sent, InpIter1>::value
   , std::pair<InpIter1, InpIter2>
   >::type
segmented_mismatch_dispatch(InpIter1 first1, Sent last1, InpIter2 first2, BinaryPred pred, Tag)
{
   while (first1 != last1 && pred(*first1, *first2)) {
      ++first1, ++first2;
   }

   return std::pair<InpIter1, InpIter2>(first1, first2);
}

} // namespace detail_algo

//! Returns a pair of iterators to the first elements where
//! \c pred(*it1, *it2) is false in [first1, last1) and the range
//! starting at \c first2, or {last1, first2 + N} if all match.
template <class InpIter1, class Sent, class InpIter2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE std::pair<InpIter1, InpIter2>
segmented_mismatch(InpIter1 first1, Sent last1, InpIter2 first2, BinaryPred pred)
{
   typedef segmented_iterator_traits<InpIter1> traits;
   return detail_algo::segmented_mismatch_dispatch
      (first1, last1, first2, pred, typename traits::is_segmented_iterator());
}

//! Returns a pair of iterators to the first mismatching elements
//! in [first1, last1) and the range starting at \c first2, or
//! {last1, first2 + N} if all elements match.
template <class InpIter1, class Sent, class InpIter2>
BOOST_CONTAINER_FORCEINLINE std::pair<InpIter1, InpIter2>
segmented_mismatch(InpIter1 first1, Sent last1, InpIter2 first2)
{
   return boost::container::segmented_mismatch(first1, last1, first2, detail_algo::mismatch_equal());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MISMATCH_HPP
