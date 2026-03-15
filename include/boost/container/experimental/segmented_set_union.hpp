//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SET_UNION_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SET_UNION_HPP

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

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter, class Comp>
OutIter segmented_set_union
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp);

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter>
OutIter segmented_set_union
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result);

namespace detail_algo {

struct set_union_default_less
{
   template <class T>
   bool operator()(const T& a, const T& b) const { return a < b; }
};

template <class FwdIt, class InIter2, class Sent2, class OutIter, class Comp>
InIter2 set_union_scan(FwdIt first, FwdIt last, InIter2 first2, Sent2 last2, OutIter& result, Comp comp,
   non_segmented_iterator_tag)
{
   while(first != last && first2 != last2) {
      if(comp(*first, *first2))      { *result = *first;  ++first; }
      else if(comp(*first2, *first)) { *result = *first2; ++first2; }
      else                           { *result = *first;  ++first; ++first2; }
      ++result;
   }
   for(; first != last; ++first, ++result)
      *result = *first;
   return first2;
}

template <class SegIt, class InIter2, class Sent2, class OutIter, class Comp>
InIter2 set_union_scan(SegIt first, SegIt last, InIter2 first2, Sent2 last2, OutIter& result, Comp comp,
   segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIt> traits;
   typedef typename traits::local_iterator local_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typename traits::segment_iterator scur  = traits::segment(first);
   typename traits::segment_iterator slast = traits::segment(last);
   local_iterator lcur = traits::local(first);
   if(scur == slast) {
      first2 = set_union_scan(lcur, traits::local(last), first2, last2, result, comp, is_local_seg_t());
   }
   else {
      first2 = set_union_scan(lcur, traits::end(scur), first2, last2, result, comp, is_local_seg_t());
      for(++scur; scur != slast; ++scur)
         first2 = set_union_scan(traits::begin(scur), traits::end(scur), first2, last2, result, comp, is_local_seg_t());
      first2 = set_union_scan(traits::begin(scur), traits::local(last), first2, last2, result, comp, is_local_seg_t());
   }
   return first2;
}

template <class SegIter, class InIter2, class Sent2, class OutIter, class Comp>
OutIter segmented_set_union_dispatch
   (SegIter first1, SegIter last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp, segmented_iterator_tag)
{
   first2 = set_union_scan(first1, last1, first2, last2, result, comp, segmented_iterator_tag());
   for(; first2 != last2; ++first2, ++result)
      *result = *first2;
   return result;
}

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter, class Comp, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent1, InIter1>::value, OutIter>::type
segmented_set_union_dispatch
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp, Tag)
{
   while(first1 != last1 && first2 != last2) {
      if(comp(*first1, *first2))      { *result = *first1; ++first1; }
      else if(comp(*first2, *first1)) { *result = *first2; ++first2; }
      else                            { *result = *first1; ++first1; ++first2; }
      ++result;
   }
   for(; first1 != last1; ++first1, ++result) *result = *first1;
   for(; first2 != last2; ++first2, ++result) *result = *first2;
   return result;
}

} // namespace detail_algo

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter, class Comp>
inline OutIter segmented_set_union
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result, Comp comp)
{
   typedef segmented_iterator_traits<InIter1> traits;
   return detail_algo::segmented_set_union_dispatch(first1, last1, first2, last2, result, comp,
      typename traits::is_segmented_iterator());
}

template <class InIter1, class Sent1, class InIter2, class Sent2, class OutIter>
inline OutIter segmented_set_union
   (InIter1 first1, Sent1 last1, InIter2 first2, Sent2 last2, OutIter result)
{
   return boost::container::segmented_set_union(first1, last1, first2, last2, result,
      detail_algo::set_union_default_less());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SET_UNION_HPP
