//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_EQUAL_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_EQUAL_HPP

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

template <class InpIter1, class Sent, class InpIter2>
bool segmented_equal(InpIter1 first1, Sent last1, InpIter2 first2);

namespace detail_algo {

template <class InpIter1, class Sent, class InpIter2>
bool equal_ref(InpIter1 first1, Sent last1, InpIter2& first2)
{
   for(; first1 != last1; ++first1, ++first2)
      if(!(*first1 == *first2))
         return false;
   return true;
}

template <class SegIter, class InpIter2>
bool segmented_equal_ref
   (SegIter first1, SegIter last1, InpIter2& first2, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typename traits::segment_iterator sfirst = traits::segment(first1);
   typename traits::segment_iterator slast  = traits::segment(last1);
   if(sfirst == slast) {
      return detail_algo::equal_ref(traits::local(first1), traits::local(last1), first2);
   }
   else {
      if(!detail_algo::equal_ref(traits::local(first1), traits::end(sfirst), first2))
         return false;
      for(++sfirst; sfirst != slast; ++sfirst)
         if(!detail_algo::equal_ref(traits::begin(sfirst), traits::end(sfirst), first2))
            return false;
      return detail_algo::equal_ref(traits::begin(sfirst), traits::local(last1), first2);
   }
}

template <class InpIter1, class Sent, class InpIter2, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InpIter1>::value, bool>::type
segmented_equal_ref
   (InpIter1 first1, Sent last1, InpIter2& first2, Tag)
{
   return detail_algo::equal_ref(first1, last1, first2);
}

} // namespace detail_algo

//! Returns \c true if elements in [first1, last1) are equal to the
//! range starting at \c first2. Exploits segmentation on the first range.
template <class InpIter1, class Sent, class InpIter2>
inline bool segmented_equal(InpIter1 first1, Sent last1, InpIter2 first2)
{
   typedef segmented_iterator_traits<InpIter1> traits;
   return detail_algo::segmented_equal_ref(first1, last1, first2,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_EQUAL_HPP
