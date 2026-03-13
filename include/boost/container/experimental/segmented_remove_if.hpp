//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_IF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_IF_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
#include <boost/container/experimental/segmented_remove_copy_if.hpp>

#include <boost/move/utility_core.hpp>

namespace boost {
namespace container {


//! Removes all elements for which \c pred returns true from [first, last),
//! moving retained elements forward. Returns iterator to new end.
template <class FwdIt, class Sent, class Predicate>
inline FwdIt segmented_remove_if(FwdIt first, Sent last, Predicate pred)
{
   first = segmented_find_if(first, last, pred);
   if (first == last)
      return last;

   FwdIt next = first;
   ++next;
   return segmented_remove_copy_if(next, last, first, pred);
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_REMOVE_IF_HPP
