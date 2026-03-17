//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_generate.hpp>

namespace boost {
namespace container {

//! Assigns \c value to every element in [first, last).
//! When \c Iter is a segmented iterator, exploits segmentation
//! to reduce per-element overhead.
template <class FwdIt, class Sent, class T>
BOOST_CONTAINER_FORCEINLINE
void segmented_fill(FwdIt first, Sent last, const T& value)
{
   (segmented_generate)(first, last, detail_algo::constref_generator<T>(value));
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_HPP
