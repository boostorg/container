/*
 *             Copyright Andrey Semashev 2019.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          https://www.boost.org/LICENSE_1_0.txt)
 */
/*
 * \file boost/container/ring_queue.hpp
 * \date 2019
 * \author Andrey Semashev
 *
 * \brief The header defines a FIFO queue class with a circular buffer implementation
 *
 * The \c ring_queue class is a specialization of \c small_ring_queue, which has no
 * internal storage for a small number of elements and always uses allocator to allocate storage.
 */

#ifndef BOOST_CONTAINER_RING_QUEUE_HPP_INCLUDED_
#define BOOST_CONTAINER_RING_QUEUE_HPP_INCLUDED_

#include <boost/container/ring_queue_fwd.hpp>
#include <boost/container/small_ring_queue.hpp>

#if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
namespace boost {
namespace container {
/*!
 * \brief FIFO queue with circular buffer implementation
 *
 * \c ring_queue is a specialization of \c small_ring_queue which has no internal storage for elements
 * and always uses the allocator to dynamically allocate storage. See \c small_ring_queue documentation.
 *
 * \tparam T The type of the element of the queue.
 * \tparam Allocator The allocator to use to allocate dynamic memory. By default, \c std::allocator will be used.
 */
template< typename T, typename Allocator >
class ring_queue :
    public small_ring_queue< T, 0u, Allocator >
{
};
}} // namespace boost::container
#endif // defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

#endif // BOOST_CONTAINER_RING_QUEUE_HPP_INCLUDED_
