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

template <class InpIter1, class Sent, class InpIter2, class BinaryPred>
bool mismatch_scan(InpIter1& first1_out, Sent last1, InpIter2& first2_out, BinaryPred pred)
{
   InpIter1 first1 = first1_out;
   InpIter2 first2 = first2_out;
   bool all_match = true;

   for (; first1 != last1; ++first1, ++first2) {
      if (!pred(*first1, *first2)) {
         all_match = false;
         break;
      }
   }

   first1_out = first1;
   first2_out = first2;
   return all_match;
}

template <class SegIter, class InpIter2, class OutIter1, class BinaryPred>
bool segmented_mismatch_ref
   (SegIter first1, SegIter last1, InpIter2& first2, OutIter1& mismatch1, BinaryPred pred, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;

   segment_iterator sfirst = traits::segment(first1);
   segment_iterator slast  = traits::segment(last1);

   if(sfirst == slast) {
      local_iterator lf = traits::local(first1);
      if(!(mismatch_scan)(lf, traits::local(last1), first2, pred)) {
         mismatch1 = traits::compose(sfirst, lf);
         return false;
      }
   }
   else {
      {
         local_iterator lf = traits::local(first1);
         if(!(mismatch_scan)(lf, traits::end(sfirst), first2, pred)) {
            mismatch1 = traits::compose(sfirst, lf);
            return false;
         }
      }
      for(++sfirst; sfirst != slast; ++sfirst) {
         local_iterator lb = traits::begin(sfirst);
         if(!(mismatch_scan)(lb, traits::end(sfirst), first2, pred)) {
            mismatch1 = traits::compose(sfirst, lb);
            return false;
         }
      }
      {
         local_iterator lb = traits::begin(sfirst);
         if(!(mismatch_scan)(lb, traits::local(last1), first2, pred)) {
            mismatch1 = traits::compose(sfirst, lb);
            return false;
         }
      }
   }
   return true;
}

template <class InpIter1, class Sent, class InpIter2, class OutIter1, class BinaryPred, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InpIter1>::value, bool>::type
segmented_mismatch_ref(InpIter1 first1, Sent last1, InpIter2& first2_out, OutIter1& mismatch1, BinaryPred pred, Tag)
{
   InpIter2 first2 = first2_out;
   bool all_match = true;

   for (; first1 != last1; ++first1, ++first2) {
      if (!pred(*first1, *first2)) {
         mismatch1 = first1;
         all_match = false;
         break;
      }
   }
   mismatch1 = first1;
   first2_out = first2;
   return all_match;
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
   InpIter1 mismatch1(last1);
   detail_algo::segmented_mismatch_ref
      (first1, last1, first2, mismatch1, pred, typename traits::is_segmented_iterator());
   return std::pair<InpIter1, InpIter2>(mismatch1, first2);
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
