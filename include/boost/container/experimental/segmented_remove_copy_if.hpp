//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_COPY_IF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_COPY_IF_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

template <class InIter, class Sent, class OutIter, class Pred>
OutIter segmented_remove_copy_if(InIter first, Sent last, OutIter result, Pred pred);

namespace detail_algo {

#if defined(BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING)

template <class InIter, class OutIter, class Pred>
OutIter segmented_remove_copy_if_dispatch
   (InIter first, InIter last, OutIter result, Pred pred, const non_segmented_iterator_tag &, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<InIter>::difference_type difference_type;
   difference_type n = last - first;

   while(n >= difference_type(4)) {
      if(!pred(*first)) { *result = *first; ++result; } ++first;
      if(!pred(*first)) { *result = *first; ++result; } ++first;
      if(!pred(*first)) { *result = *first; ++result; } ++first;
      if(!pred(*first)) { *result = *first; ++result; } ++first;
      n -= 4;
   }

   switch(n) {
      case 3:
         if(!pred(*first)) { *result = *first; ++result; } ++first;
         if(!pred(*first)) { *result = *first; ++result; } ++first;
         if(!pred(*first)) { *result = *first; ++result; }
         break;
      case 2:
         if(!pred(*first)) { *result = *first; ++result; } ++first;
         if(!pred(*first)) { *result = *first; ++result; }
         break;
      case 1:
         if(!pred(*first)) { *result = *first; ++result; }
         break;
      default:
         break;
   }
   return result;
}

#endif   //BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING

template <class InIter, class Sent, class OutIter, class Pred, class Tag, class Cat>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InIter>::value, OutIter>::type
segmented_remove_copy_if_dispatch
   (InIter first, Sent last, OutIter result, Pred pred, Tag, Cat)
{
   for(; first != last; ++first) {
      if(!pred(*first)) {
         *result = *first;
         ++result;
      }
   }
   return result;
}

template <class SegIter, class OutIter, class Pred, class Cat>
OutIter segmented_remove_copy_if_dispatch
   (SegIter first, SegIter last, OutIter result, Pred pred, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator sfirst = traits::segment(first);
   segment_iterator slast  = traits::segment(last);

   if(sfirst == slast) {
      return (segmented_remove_copy_if_dispatch)(traits::local(first), traits::local(last), result, pred, is_local_seg_t(), Cat());
   }
   else {
      result = (segmented_remove_copy_if_dispatch)(traits::local(first), traits::end(sfirst), result, pred, is_local_seg_t(), Cat());

      for(++sfirst; sfirst != slast; ++sfirst)
         result = (segmented_remove_copy_if_dispatch)(traits::begin(sfirst), traits::end(sfirst), result, pred, is_local_seg_t(), Cat());

      return (segmented_remove_copy_if_dispatch)(traits::begin(sfirst), traits::local(last), result, pred, is_local_seg_t(), Cat());
   }
}

} // namespace detail_algo

//! Copies elements from [first, last) to the range beginning at \c result,
//! skipping elements satisfying \c pred. Returns the output iterator past
//! the last element written.
template <class InIter, class Sent, class OutIter, class Pred>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_remove_copy_if(InIter first, Sent last, OutIter result, Pred pred)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_remove_copy_if_dispatch
      (first, last, result, pred, typename traits::is_segmented_iterator(), typename iterator_traits<InIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_COPY_IF_HPP
