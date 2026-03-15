//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_N_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_N_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_generate_n.hpp>

namespace boost {
namespace container {

//! Assigns \c value to the first \c count elements starting at \c first.
//! Returns an iterator past the last filled element.
//! Exploits segmentation when available.
template <class FwdIt, class Size, class T>
BOOST_CONTAINER_FORCEINLINE FwdIt segmented_fill_n(FwdIt first, Size count, const T& value)
{
   return segmented_generate_n(first, count, detail_algo::constref_generator<T>(value));
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FILL_N_HPP
