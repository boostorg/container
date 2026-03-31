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
#include <boost/container/detail/iterators.hpp>
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

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class RAIter1, class RAIter2, class BinaryPred>
std::pair<RAIter1, RAIter2> segmented_mismatch_dispatch
   (RAIter1 first1, RAIter1 last1, RAIter2 first2, BinaryPred pred, const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RAIter1>::difference_type difference_type;

   difference_type n = last1 - first1;
   while(n >= difference_type(4)) {
      if(!pred(*first1, *first2))
         goto final_result;
      ++first1; ++first2;
      if(!pred(*first1, *first2))
         goto final_result;
      ++first1; ++first2;
      if(!pred(*first1, *first2))
         goto final_result;
      ++first1; ++first2;
      if(!pred(*first1, *first2))
         goto final_result;
      ++first1; ++first2;
      n -= 4;
   }

   switch(n) {
      case 3:
         if(!pred(*first1, *first2))
            goto final_result;
         ++first1; ++first2;
         if(!pred(*first1, *first2))
            goto final_result;
         ++first1; ++first2;
         if(!pred(*first1, *first2))
            goto final_result;
         ++first1; ++first2;
         break;
      case 2:
         if(!pred(*first1, *first2))
            goto final_result;
         ++first1; ++first2;
         if(!pred(*first1, *first2))
            goto final_result;
         ++first1; ++first2;
         break;
      case 1:
         if(!pred(*first1, *first2))
            goto final_result;
         ++first1; ++first2;
         break;
      default:
         break;
   }
   final_result:
   return std::pair<RAIter1, RAIter2>(first1, first2);
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class InpIter1, class Sent, class InpIter2, class BinaryPred, class Tag, class Cat>
typename algo_enable_if_c
   < !Tag::value || is_sentinel<Sent, InpIter1>::value
   , std::pair<InpIter1, InpIter2>
   >::type
segmented_mismatch_dispatch(InpIter1 first1, Sent last1, InpIter2 first2, BinaryPred pred, Tag, Cat)
{
   while (first1 != last1 && pred(*first1, *first2)) {
      ++first1, ++first2;
   }

   return std::pair<InpIter1, InpIter2>(first1, first2);
}

template <class SegIter, class InpIter2, class BinaryPred, class Cat>
std::pair<SegIter, InpIter2> segmented_mismatch_dispatch
   (SegIter first1, SegIter last1, InpIter2 first2, BinaryPred pred, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   typedef std::pair<SegIter, InpIter2>        return_t;
   typedef std::pair<local_iterator, InpIter2> local_return_t;

   segment_iterator       sfirst = traits::segment(first1);
   segment_iterator const slast  = traits::segment(last1);

   if(sfirst == slast) {
      const local_iterator ll = traits::local(last1);
      const local_return_t r = (segmented_mismatch_dispatch)(traits::local(first1), ll, first2, pred, is_local_seg_t(), local_cat_t());
      return return_t((r.first != ll) ? traits::compose(sfirst, r.first) : last1, r.second);
   }
   else {
      // First segment
      local_iterator le = traits::end(sfirst);
      local_return_t r = (segmented_mismatch_dispatch)(traits::local(first1), le, first2, pred, is_local_seg_t(), local_cat_t());
      if (r.first != le)
         return return_t(traits::compose(sfirst, r.first), r.second);

      // Middle segments
      for (++sfirst; sfirst != slast; ++sfirst) {
         le = traits::end(sfirst);
         r = (segmented_mismatch_dispatch)(traits::begin(sfirst), le, r.second, pred, is_local_seg_t(), local_cat_t());
         if (r.first != le)
            return return_t(traits::compose(sfirst, r.first), r.second);
      }

      // Last segment
      le = traits::local(last1);
      r = (segmented_mismatch_dispatch)(traits::begin(slast), le, r.second, pred, is_local_seg_t(), local_cat_t());
      return return_t((r.first != le) ? traits::compose(sfirst, r.first) : last1, r.second);
   }
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
      (first1, last1, first2, pred, typename traits::is_segmented_iterator(), typename iterator_traits<InpIter1>::iterator_category());
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
