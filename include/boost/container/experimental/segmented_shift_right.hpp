//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SHIFT_RIGHT_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SHIFT_RIGHT_HPP

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

//! Shifts elements in [first, last) right by \c n positions.
//! Returns iterator to new begin.
template <class BidirIt>
BidirIt segmented_shift_right
   (BidirIt first, BidirIt last,
    typename boost::container::iterator_traits<BidirIt>::difference_type n)
{
   typedef typename boost::container::iterator_traits<BidirIt>::difference_type difference_type;
   if(n <= 0) return first;

   BidirIt mid = last;
   for(difference_type i = 0; i < n; ++i) {
      if(mid == first) return last;
      --mid;
   }

   BidirIt result = last;
   BidirIt src = mid;
   while(src != first) {
      --src; --result;
      *result = boost::move(*src);
   }
   return result;
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SHIFT_RIGHT_HPP
