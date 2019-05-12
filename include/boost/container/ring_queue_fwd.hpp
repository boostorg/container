/*
 *             Copyright Andrey Semashev 2019.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          https://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file
 *
 * This header file forward declares \c boost::container::small_ring_queue_base, \c boost::container::small_ring_queue and \c boost::container::ring_queue.
 */

#ifndef BOOST_CONTAINER_RING_QUEUE_FWD_HPP_INCLUDED_
#define BOOST_CONTAINER_RING_QUEUE_FWD_HPP_INCLUDED_

// Doxygen incorrectly places ring_queue definition in this header for some reason
#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

#include <cstddef>

namespace boost {
namespace container {

template< typename T, typename Allocator = void >
class small_ring_queue_base;

template< typename T, std::size_t StaticCapacity, typename Allocator = void >
class small_ring_queue;

template< typename T, typename Allocator = void >
using ring_queue = small_ring_queue< T, 0u, Allocator >;

}} // namespace boost::container

#endif // !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

#endif // BOOST_CONTAINER_RING_QUEUE_FWD_HPP_INCLUDED_
