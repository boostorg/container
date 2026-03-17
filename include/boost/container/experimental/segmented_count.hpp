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
#include <boost/container/detail/compare_functors.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/container/experimental/segmented_count_if.hpp>

namespace boost {
namespace container {

//! Returns the number of elements equal to \c value in [first, last).
template <class InpIter, class Sent, class T>
BOOST_CONTAINER_FORCEINLINE
typename boost::container::iterator_traits<InpIter>::difference_type
   segmented_count(InpIter first, Sent last, const T& value)
{
   return boost::container::segmented_count_if(first, last, equal_to_value<T>(value));
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_COUNT_HPP
