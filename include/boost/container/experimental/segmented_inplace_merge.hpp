//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_INPLACE_MERGE_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_INPLACE_MERGE_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

//! Merges two consecutive sorted ranges [first, middle) and [middle, last)
//! in-place using operator<.
template <class BidirIt>
void segmented_inplace_merge(BidirIt first, BidirIt middle, BidirIt last)
{
   typedef typename boost::container::iterator_traits<BidirIt>::value_type value_type;
   if(first == middle || middle == last) return;
   for(BidirIt second = middle; second != last; ++second) {
      value_type val = boost::move(*second);
      BidirIt hole = second;
      BidirIt prev = hole; --prev;
      while(hole != first) {
         if(!(val < *prev)) break;
         *hole = boost::move(*prev);
         hole = prev;
         if(hole != first) --prev;
      }
      *hole = boost::move(val);
   }
}

//! Merges two consecutive sorted ranges [first, middle) and [middle, last)
//! in-place using the provided comparator \c comp.
template <class BidirIt, class Compare>
void segmented_inplace_merge(BidirIt first, BidirIt middle, BidirIt last, Compare comp)
{
   typedef typename boost::container::iterator_traits<BidirIt>::value_type value_type;
   if(first == middle || middle == last) return;
   for(BidirIt second = middle; second != last; ++second) {
      value_type val = boost::move(*second);
      BidirIt hole = second;
      BidirIt prev = hole; --prev;
      while(hole != first) {
         if(!comp(val, *prev)) break;
         *hole = boost::move(*prev);
         hole = prev;
         if(hole != first) --prev;
      }
      *hole = boost::move(val);
   }
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_INPLACE_MERGE_HPP
