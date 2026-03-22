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
#include <boost/container/experimental/segmented_mismatch.hpp>

namespace boost {
namespace container {

//! Returns \c true if elements in [first1, last1) are equal to the
//! range starting at \c first2. Exploits segmentation on the first range.
template <class InpIter1, class Sent, class InpIter2>
BOOST_CONTAINER_FORCEINLINE
bool segmented_equal(InpIter1 first1, Sent last1, InpIter2 first2)
{
   return (segmented_mismatch)(first1, last1, first2).first == last1;
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_EQUAL_HPP
