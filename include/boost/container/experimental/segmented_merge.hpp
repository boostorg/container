//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MERGE_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MERGE_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_copy.hpp>

namespace boost {
namespace container {

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter, class Comp>
OutIter segmented_merge
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp);

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter>
OutIter segmented_merge
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result);

namespace detail_algo {

struct merge_default_less
{
   template <class T>
   bool operator()(const T& a, const T& b) const { return a < b; }
};

template <class FwdIt, class Sent, class InIter2, class Sent2, class OutIter, class Comp>
OutIter merge_scan(FwdIt first, Sent last, InIter2& first2_out, Sent2 last2, OutIter result, Comp comp,
   non_segmented_iterator_tag)
{
   InIter2 first2 = first2_out;
   while(first != last && first2 != last2) {
      if(comp(*first2, *first)) {
         *result = *first2;
         ++first2;
      }
      else {
         *result = *first;
         ++first;
      }
      ++result;
   }
   first2_out = first2;
   return (segmented_copy)(first, last, result);
}

template <class SegIt, class InIter2, class Sent2, class OutIter, class Comp>
OutIter merge_scan(SegIt first, SegIt last, InIter2& first2, Sent2 last2, OutIter result, Comp comp,
   segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt>   traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename segmented_iterator_traits
      <local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator scur  = traits::segment(first);
   segment_iterator slast = traits::segment(last);
   local_iterator   lcur  = traits::local(first);

   if(scur == slast) {
      return merge_scan(lcur, traits::local(last), first2, last2, result, comp, is_local_seg_t());
   }
   else {
      result = merge_scan(lcur, traits::end(scur), first2, last2, result, comp, is_local_seg_t());

      for(++scur; scur != slast; ++scur)
         result = merge_scan(traits::begin(scur), traits::end(scur), first2, last2, result, comp, is_local_seg_t());

      return merge_scan(traits::begin(scur), traits::local(last), first2, last2, result, comp, is_local_seg_t());
   }
}

template <class SegIter, class InIter2, class Sent2, class OutIter, class Comp>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_merge_dispatch
   (SegIter first1, SegIter last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp, segmented_iterator_tag)
{
   result = merge_scan(first1, last1, first2, last2, result, comp, segmented_iterator_tag());
   return (segmented_copy)(first2, last2, result);
}

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter, class Comp, class Tag>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent1, InIter1>::value, OutIter>::type
segmented_merge_dispatch
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp, Tag)
{
   result = merge_scan(first1, last1, first2, last2, result, comp, non_segmented_iterator_tag());
   return   (segmented_copy)(first2, last2, result);
}

} // namespace detail_algo

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter, class Comp>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_merge
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp)
{
   typedef segmented_iterator_traits<InIter1> traits;
   return detail_algo::segmented_merge_dispatch
      (first1, last1, first2, last2, result, comp, typename traits::is_segmented_iterator());
}

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter>
BOOST_CONTAINER_FORCEINLINE
OutIter segmented_merge (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result)
{
   return boost::container::segmented_merge
      (first1, last1, first2, last2, result, detail_algo::merge_default_less());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_MERGE_HPP
