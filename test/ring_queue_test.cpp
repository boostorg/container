/*
 *             Copyright Andrey Semashev 2019.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          https://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file libs/container/test/ring_queue_test.cpp
 * \date 2019
 * \author Andrey Semashev
 *
 * \brief The file contains tests for \c ring_queue and \c small_ring_queue
 */

#include <boost/container/ring_queue.hpp>
#include <boost/container/small_ring_queue.hpp>

#include <algorithm>
#include <boost/core/lightweight_test.hpp>

constexpr unsigned int small_storage_count = 4u;

struct my_struct
{
    unsigned int x, y;

    my_struct() : x(1u), y(2u) {}
    my_struct(unsigned int x, unsigned int y) : x(x), y(y) {}
};

struct my_struct_non_default_constructible
{
    unsigned int x, y;

    my_struct_non_default_constructible() = delete;
    my_struct_non_default_constructible(unsigned int x, unsigned int y) : x(x), y(y) {}
};

template< template< typename > class Queue >
struct ring_queue_tests
{
    //! Runs all tests
    static void test()
    {
        test_size();
        test_capacity();

        test_element_passing(0u);
        test_element_passing(1u);
        test_element_passing(small_storage_count + 1u);

        test_emplace();
        test_swap();
    }

    //! Tests size and capacity related methods
    static void test_size()
    {
        Queue< int > q;

        BOOST_TEST_EQ(q.empty(), true);
        BOOST_TEST_EQ(q.size(), 0u);
        BOOST_TEST_EQ(q.capacity(), Queue< int >::static_capacity);

        q.push(10);

        BOOST_TEST_EQ(q.empty(), false);
        BOOST_TEST_EQ(q.size(), 1u);
        BOOST_TEST_GE(q.capacity(), q.size());

        typename Queue< int >::size_type old_capacity = q.capacity();
        q.pop();

        BOOST_TEST_EQ(q.empty(), true);
        BOOST_TEST_EQ(q.size(), 0u);
        BOOST_TEST_EQ(q.capacity(), old_capacity);

        q.push(10);
        q.push(20);

        BOOST_TEST_EQ(q.empty(), false);
        BOOST_TEST_EQ(q.size(), 2u);
        BOOST_TEST_GE(q.capacity(), q.size());

        old_capacity = q.capacity();
        q.clear();

        BOOST_TEST_EQ(q.empty(), true);
        BOOST_TEST_EQ(q.size(), 0u);
        BOOST_TEST_EQ(q.capacity(), old_capacity);
    }

    //! Tests capacity management methods
    static void test_capacity()
    {
        {
            Queue< int > q;

            BOOST_TEST_EQ(q.capacity(), Queue< int >::static_capacity);

            q.reserve(small_storage_count * 2u);

            BOOST_TEST_EQ(q.capacity(), small_storage_count * 2u);
        }
        {
            Queue< int > q;

            BOOST_TEST_EQ(q.capacity(), Queue< int >::static_capacity);

            q.push(10);
            q.reserve(small_storage_count * 2u);

            BOOST_TEST_EQ(q.size(), 1u);
            BOOST_TEST_EQ(q.capacity(), small_storage_count * 2u);
            BOOST_TEST_EQ(q.front(), 10);

            q.shrink_to_fit();

            BOOST_TEST_EQ(q.size(), 1u);
            BOOST_TEST_EQ(q.capacity(), std::max(q.size(), Queue< int >::static_capacity));
            BOOST_TEST_EQ(q.front(), 10);
        }
        {
            Queue< int > q;

            BOOST_TEST_EQ(q.capacity(), Queue< int >::static_capacity);

            q.push(10);
            q.reserve(small_storage_count * 2u);

            BOOST_TEST_EQ(q.size(), 1u);
            BOOST_TEST_EQ(q.capacity(), small_storage_count * 2u);
            BOOST_TEST_EQ(q.front(), 10);

            typename Queue< int >::size_type const extra_capacity = 2u;
            q.shrink_to_fit(extra_capacity);

            BOOST_TEST_EQ(q.size(), 1u);
            BOOST_TEST_EQ(q.capacity(), std::max(q.size() + extra_capacity, Queue< int >::static_capacity));
            BOOST_TEST_EQ(q.front(), 10);
        }
    }

    //! Tests push/pop/front/back
    static void test_element_passing(unsigned int window_size)
    {
        unsigned int send = 0u, receive = 0u;
        Queue< unsigned int > q;

        for (unsigned int i = 0u; i < window_size; ++i)
            q.push(send++);

        BOOST_TEST_EQ(q.size(), window_size);
        BOOST_TEST_GE(q.capacity(), q.size());

        for (unsigned int i = 0u; i < 10u; ++i)
        {
            q.push(send);

            BOOST_TEST_EQ(q.back(), send);
            BOOST_TEST_EQ(q.front(), receive);

            q.pop();
            ++send;
            ++receive;
        }

        unsigned int i = 0u;
        while (!q.empty())
        {
            BOOST_TEST_EQ(q.front(), receive);

            q.pop();
            ++receive;
            ++i;
        }

        BOOST_TEST_EQ(send, receive);
        BOOST_TEST_EQ(i, window_size);
    }

    //! Tests emplace
    static void test_emplace()
    {
        {
            Queue< my_struct > q;

            q.emplace();

            BOOST_TEST_EQ(q.empty(), false);
            BOOST_TEST_EQ(q.size(), 1u);

            {
                my_struct const& elem = q.back();
                BOOST_TEST_EQ(elem.x, 1u);
                BOOST_TEST_EQ(elem.y, 2u);
            }

            q.emplace(10u, 20u);

            BOOST_TEST_EQ(q.empty(), false);
            BOOST_TEST_EQ(q.size(), 2u);

            {
                my_struct const& elem = q.back();
                BOOST_TEST_EQ(elem.x, 10u);
                BOOST_TEST_EQ(elem.y, 20u);
            }
        }

        {
            Queue< my_struct_non_default_constructible > q;

            q.emplace(10u, 20u);

            BOOST_TEST_EQ(q.empty(), false);
            BOOST_TEST_EQ(q.size(), 1u);

            {
                my_struct_non_default_constructible const& elem = q.back();
                BOOST_TEST_EQ(elem.x, 10u);
                BOOST_TEST_EQ(elem.y, 20u);
            }
        }
    }

    //! Tests swap
    static void test_swap()
    {
        // Swap empty queues
        {
            Queue< unsigned int > q1, q2;

            BOOST_TEST_EQ(q1.size(), 0u);
            BOOST_TEST_EQ(q2.size(), 0u);

            q1.swap(q2);

            BOOST_TEST_EQ(q1.size(), 0u);
            BOOST_TEST_EQ(q2.size(), 0u);

            swap(q1, q2);

            BOOST_TEST_EQ(q1.size(), 0u);
            BOOST_TEST_EQ(q2.size(), 0u);
        }

        // Swap empty with non-empty queue using internal storage
        {
            Queue< unsigned int > q1, q2;

            q1.push(10u);
            q1.push(10u);
            // Make sure the reading position is not 0
            q1.pop();

            BOOST_TEST_EQ(q1.size(), 1u);
            BOOST_TEST_EQ(q2.size(), 0u);

            q1.swap(q2);

            BOOST_TEST_EQ(q1.size(), 0u);
            BOOST_TEST_EQ(q2.size(), 1u);
            BOOST_TEST_EQ(q2.front(), 10u);
        }

        // Swap empty with non-empty queue using internal storage while making sure the deep swap needs to rollover the internal storage
        if (Queue< unsigned int >::static_capacity > 0)
        {
            Queue< unsigned int > q1, q2;

            q1.push(0u);
            q1.push(0u);
            q1.pop();
            q2.push(0u);

            for (unsigned int i = 0u; i < Queue< unsigned int >::static_capacity - 1u; ++i)
            {
                q1.push(i);
                q2.push(100u + i);
            }

            q1.pop();
            q2.pop();

            q1.push(Queue< unsigned int >::static_capacity - 1u);
            q2.push(100u + Queue< unsigned int >::static_capacity - 1u);

            BOOST_TEST_EQ(q1.size(), Queue< unsigned int >::static_capacity);
            BOOST_TEST_EQ(q2.size(), Queue< unsigned int >::static_capacity);

            q1.swap(q2);

            BOOST_TEST_EQ(q1.size(), Queue< unsigned int >::static_capacity);
            BOOST_TEST_EQ(q2.size(), Queue< unsigned int >::static_capacity);

            for (unsigned int i = 0u; i < Queue< unsigned int >::static_capacity; ++i)
            {
                BOOST_TEST_EQ(q1.front(), 100u + i);
                BOOST_TEST_EQ(q2.front(), i);
                q1.pop();
                q2.pop();
            }
        }

        // Swap two non-empty queues using internal storage
        {
            Queue< unsigned int > q1, q2;

            q1.push(10u);
            q1.push(10u);
            q1.push(20u);
            // Make sure the reading position is not 0
            q1.pop();

            q2.push(100u);

            BOOST_TEST_EQ(q1.size(), 2u);
            BOOST_TEST_EQ(q2.size(), 1u);

            q1.swap(q2);

            BOOST_TEST_EQ(q1.size(), 1u);
            BOOST_TEST_EQ(q2.size(), 2u);
            BOOST_TEST_EQ(q1.front(), 100u);
            BOOST_TEST_EQ(q2.front(), 10u);
            q2.pop();
            BOOST_TEST_EQ(q2.front(), 20u);
        }

        // Swap a non-empty queue using internal storage with a queue using dynamic storage
        {
            Queue< unsigned int > q1, q2;

            for (unsigned int i = 0u; i < Queue< unsigned int >::static_capacity + 1u; ++i)
                q1.push(i);

            q2.push(100u);
            q2.push(100u);
            // Make sure the reading position is not 0
            q2.pop();

            BOOST_TEST_EQ(q1.size(), Queue< unsigned int >::static_capacity + 1u);
            BOOST_TEST_EQ(q2.size(), 1u);

            q1.swap(q2);

            BOOST_TEST_EQ(q1.size(), 1u);
            BOOST_TEST_EQ(q2.size(), Queue< unsigned int >::static_capacity + 1u);
            BOOST_TEST_EQ(q1.front(), 100u);
            for (unsigned int i = 0u; i < Queue< unsigned int >::static_capacity + 1u; ++i)
            {
                BOOST_TEST_EQ(q2.front(), i);
                q2.pop();
            }
        }

        // Swap two queues using dynamic storage
        {
            Queue< unsigned int > q1, q2;

            for (unsigned int i = 0u; i < Queue< unsigned int >::static_capacity + 1u; ++i)
            {
                q1.push(i);
            }

            for (unsigned int i = 0u; i < Queue< unsigned int >::static_capacity + 5u; ++i)
            {
                q2.push(100u + i);
            }

            BOOST_TEST_EQ(q1.size(), Queue< unsigned int >::static_capacity + 1u);
            BOOST_TEST_EQ(q2.size(), Queue< unsigned int >::static_capacity + 5u);

            q1.swap(q2);

            BOOST_TEST_EQ(q1.size(), Queue< unsigned int >::static_capacity + 5u);
            BOOST_TEST_EQ(q2.size(), Queue< unsigned int >::static_capacity + 1u);

            for (unsigned int i = 0u; i < Queue< unsigned int >::static_capacity + 5u; ++i)
            {
                BOOST_TEST_EQ(q1.front(), 100u + i);
                q1.pop();
            }

            for (unsigned int i = 0u; i < Queue< unsigned int >::static_capacity + 1u; ++i)
            {
                BOOST_TEST_EQ(q2.front(), i);
                q2.pop();
            }
        }
    }
};

template< typename T >
using ring_queue = boost::container::ring_queue< T >;

template< typename T >
using small_ring_queue = boost::container::small_ring_queue< T, small_storage_count >;

int main()
{
    ring_queue_tests< ring_queue >::test();
    ring_queue_tests< small_ring_queue >::test();

    return boost::report_errors();
}
