//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COUNT_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COUNT_HPP

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

template <class InpIter, class Sent, class T>
typename boost::container::iterator_traits<InpIter>::difference_type
segmented_count(InpIter first, Sent last, const T& value);

namespace detail_algo {

template <class SegIter, class T>
typename boost::container::iterator_traits<SegIter>::difference_type
segmented_count_dispatch
   (SegIter first, SegIter last, const T& value, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename boost::container::iterator_traits<SegIter>::difference_type diff_t;
   diff_t result = 0;

   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);
   if(sfirst == slast) {
      result = boost::container::segmented_count(traits::local(first), traits::local(last), value);
   }
   else {
      result += boost::container::segmented_count(traits::local(first), traits::end(sfirst), value);
      for(++sfirst; sfirst != slast; ++sfirst)
         result += boost::container::segmented_count(traits::begin(sfirst), traits::end(sfirst), value);
      result += boost::container::segmented_count(traits::begin(sfirst), traits::local(last), value);
   }
   return result;
}

template <class InpIter, class Sent, class T, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InpIter>::value,
   typename boost::container::iterator_traits<InpIter>::difference_type>::type
segmented_count_dispatch
   (InpIter first, Sent last, const T& value, Tag)
{
   typename boost::container::iterator_traits<InpIter>::difference_type n = 0;
   for(; first != last; ++first)
      if(*first == value)
         ++n;
   return n;
}

} // namespace detail_algo

//! Returns the number of elements equal to \c value in [first, last).
template <class InpIter, class Sent, class T>
inline typename boost::container::iterator_traits<InpIter>::difference_type
segmented_count(InpIter first, Sent last, const T& value)
{
   typedef segmented_iterator_traits<InpIter> traits;
   return detail_algo::segmented_count_dispatch(first, last, value,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COUNT_HPP
