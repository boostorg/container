//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_NONE_OF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_NONE_OF_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>

namespace boost {
namespace container {

//! Returns \c true if \c pred returns false for all elements
//! in [first, last), or if the range is empty.
template <class InpIter, class Sent, class Pred>
BOOST_CONTAINER_FORCEINLINE
bool segmented_none_of(InpIter first, Sent last, Pred pred)
{
   return (segmented_find_if)(first, last, pred) == last;
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_NONE_OF_HPP
