//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ROTATE_COPY_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ROTATE_COPY_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_copy.hpp>

namespace boost {
namespace container {

//! Copies [n_first, last) followed by [first, n_first) to the range
//! beginning at \c result. Returns the output iterator past the last
//! element written. Exploits segmentation through segmented_copy.
template <class FwdIt, class Sent, class OutIter>
inline OutIter segmented_rotate_copy
   (FwdIt first, FwdIt n_first, Sent last, OutIter result)
{
   result = boost::container::segmented_copy(n_first, last, result);
   return boost::container::segmented_copy(first, n_first, result);
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ROTATE_COPY_HPP
