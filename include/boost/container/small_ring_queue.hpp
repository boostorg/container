/*
 *             Copyright Andrey Semashev 2019.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          https://www.boost.org/LICENSE_1_0.txt)
 */
/*
 * \file boost/container/small_ring_queue.hpp
 * \date 2019
 * \author Andrey Semashev
 *
 * \brief The header defines a FIFO queue class with a circular buffer implementation
 *
 * The \c small_ring_queue class template defined in this header is similar to \c std::queue but uses a circular
 * buffer implementation internally, which is more optimal in case if the average number of queued elements remains
 * constant over the course of the queue lifetime. The internal circular buffer allows to avoid dynamic memory
 * allocation as the queue elements are enqueued and dequeued.
 *
 * Additionally, \c small_ring_queue allows to allocate inline storage for a (small) number of elements
 * and avoid dynamic memory allocations altogether.
 */

#ifndef BOOST_CONTAINER_SMALL_RING_QUEUE_HPP_INCLUDED_
#define BOOST_CONTAINER_SMALL_RING_QUEUE_HPP_INCLUDED_

#include <cstddef>
#include <memory>
#include <utility>
#include <stdexcept>
#include <type_traits>
#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
#include <boost/container/ring_queue_fwd.hpp>

namespace boost {
namespace container {
namespace detail {

template< typename T, typename Allocator >
struct ring_queue_make_allocator_type
{
    typedef typename std::allocator_traits< Allocator >::template rebind_alloc< T > type;
};

template< typename T >
struct ring_queue_make_allocator_type< T, void >
{
    typedef std::allocator< T > type;
};

template< typename Allocator >
struct ring_queue_allocator_types
{
    typedef Allocator allocator_type;
    typedef typename std::allocator_traits< allocator_type >::value_type value_type;
    typedef typename std::allocator_traits< allocator_type >::pointer pointer;
    typedef typename std::allocator_traits< allocator_type >::const_pointer const_pointer;
    typedef typename std::allocator_traits< allocator_type >::size_type size_type;
    typedef typename std::allocator_traits< allocator_type >::difference_type difference_type;
};

template< typename Allocator, bool = std::is_empty< Allocator >::value && !std::is_final< Allocator >::value >
class ring_queue_allocator_holder :
    private Allocator
{
private:
    typedef ring_queue_allocator_types< Allocator > allocator_types;

public:
    typedef typename allocator_types::allocator_type allocator_type;
    typedef typename allocator_types::value_type value_type;
    typedef typename allocator_types::pointer pointer;
    typedef typename allocator_types::const_pointer const_pointer;
    typedef typename allocator_types::size_type size_type;
    typedef typename allocator_types::difference_type difference_type;

protected:
    ring_queue_allocator_holder() noexcept(std::is_nothrow_default_constructible< allocator_type >::value) = default;
    ring_queue_allocator_holder(ring_queue_allocator_holder const& that) noexcept : allocator_type(that.get_allocator_ref()) {}
    explicit ring_queue_allocator_holder(allocator_type const& alloc) noexcept : allocator_type(alloc) {}
    ring_queue_allocator_holder& operator= (ring_queue_allocator_holder const& that) noexcept
    {
        get_allocator_ref() = that.get_allocator_ref();
        return *this;
    }

public:
    allocator_type get_allocator() const noexcept { return *static_cast< const allocator_type* >(this); }

protected:
    allocator_type& get_allocator_ref() noexcept { return *static_cast< allocator_type* >(this); }
    allocator_type const& get_allocator_ref() const noexcept { return *static_cast< const allocator_type* >(this); }
};

template< typename Allocator >
class ring_queue_allocator_holder< Allocator, false >
{
private:
    typedef ring_queue_allocator_types< Allocator > allocator_types;

public:
    typedef typename allocator_types::allocator_type allocator_type;
    typedef typename allocator_types::value_type value_type;
    typedef typename allocator_types::pointer pointer;
    typedef typename allocator_types::const_pointer const_pointer;
    typedef typename allocator_types::size_type size_type;
    typedef typename allocator_types::difference_type difference_type;

private:
    allocator_type m_allocator;

protected:
    ring_queue_allocator_holder() noexcept(std::is_nothrow_default_constructible< allocator_type >::value) = default;
    ring_queue_allocator_holder(ring_queue_allocator_holder const& that) noexcept : m_allocator(that.get_allocator_ref()) {}
    explicit ring_queue_allocator_holder(allocator_type const& alloc) noexcept : m_allocator(alloc) {}
    ring_queue_allocator_holder& operator= (ring_queue_allocator_holder const& that) noexcept
    {
        m_allocator = that.get_allocator_ref();
        return *this;
    }

public:
    allocator_type get_allocator() const noexcept { return m_allocator; }

protected:
    allocator_type& get_allocator_ref() noexcept { return m_allocator; }
    allocator_type const& get_allocator_ref() const noexcept { return m_allocator; }
};

template< typename T, std::size_t StaticCapacity, typename Allocator >
class small_ring_queue_storage
{
private:
    typedef ring_queue_allocator_types< Allocator > allocator_types;

public:
    typedef typename allocator_types::allocator_type allocator_type;
    typedef typename allocator_types::value_type value_type;
    typedef typename allocator_types::pointer pointer;
    typedef typename allocator_types::const_pointer const_pointer;
    typedef typename allocator_types::size_type size_type;
    typedef typename allocator_types::difference_type difference_type;

    //! Amount of storage available without dynamic memory allocation
    static constexpr size_type static_capacity = StaticCapacity;

private:
    typedef typename std::aligned_storage< sizeof(value_type), alignof(value_type) >::type value_storage_type;

private:
    value_storage_type m_storage[static_capacity];

protected:
    pointer get_internal_storage() noexcept { return reinterpret_cast< pointer >(m_storage); }
    const_pointer get_internal_storage() const noexcept { return reinterpret_cast< const_pointer >(m_storage); }
};

template< typename T, std::size_t StaticCapacity, typename Allocator >
constexpr typename small_ring_queue_storage< T, StaticCapacity, Allocator >::size_type small_ring_queue_storage< T, StaticCapacity, Allocator >::static_capacity;

template< typename T, typename Allocator >
class small_ring_queue_storage< T, 0u, Allocator >
{
private:
    typedef ring_queue_allocator_types< Allocator > allocator_types;

public:
    typedef typename allocator_types::allocator_type allocator_type;
    typedef typename allocator_types::value_type value_type;
    typedef typename allocator_types::pointer pointer;
    typedef typename allocator_types::const_pointer const_pointer;
    typedef typename allocator_types::size_type size_type;
    typedef typename allocator_types::difference_type difference_type;

    //! Amount of storage available without dynamic memory allocation
    static constexpr size_type static_capacity = 0u;

protected:
    pointer get_internal_storage() noexcept { return nullptr; }
    const_pointer get_internal_storage() const noexcept { return nullptr; }
};

template< typename T, typename Allocator >
constexpr typename small_ring_queue_storage< T, 0u, Allocator >::size_type small_ring_queue_storage< T, 0u, Allocator >::static_capacity;

} // namespace detail

/*!
 * \brief Base class for \c small_ring_queue
 *
 * This class implements partial interface of \c small_ring_queue which does not depend on the size of the internal storage.
 * It can be used to dequeue elements from the queue and observe properties like size and capacity. Since \c small_ring_queue_base
 * does not depend on the size of the internal storage, it can be used to write queue element consumers that do not depend on
 * the internal storage size.
 *
 * <pre>
 * void consume_elements(small_ring_queue_base< Foo >& q);
 *
 * void produce_elements()
 * {
 *     small_ring_queue< Foo, 4 > q1;
 *     consume_elements(q1);
 *
 *     small_ring_queue< Foo, 10 > q2;
 *     consume_elements(q2);
 * }
 * </pre>
 *
 * \note Users should not attempt to destroy, copy, move, assign or swap queues through \c small_ring_queue_base.
 */
template< typename T, typename Allocator >
class small_ring_queue_base
#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
    : private container::detail::ring_queue_allocator_holder< typename container::detail::ring_queue_make_allocator_type< T, Allocator >::type >
#endif
{
#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
private:
    typedef container::detail::ring_queue_allocator_holder< typename container::detail::ring_queue_make_allocator_type< T, Allocator >::type > allocator_holder;
#endif

public:
    typedef typename allocator_holder::allocator_type allocator_type;
    typedef typename allocator_holder::value_type value_type;
    typedef typename allocator_holder::pointer pointer;
    typedef typename allocator_holder::const_pointer const_pointer;
    typedef typename allocator_holder::size_type size_type;
    typedef typename allocator_holder::difference_type difference_type;
    typedef value_type& reference;
    typedef value_type const& const_reference;

#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
protected:
    //! Pointer to the storage
    pointer m_data;
    //! Buffer capacity
    size_type m_capacity;
    //! Number of enqueued elements
    size_type m_size;
    //! Dequeueing position
    size_type m_read_pos;

protected:
    small_ring_queue_base(pointer data, size_type capacity) noexcept(std::is_nothrow_default_constructible< allocator_type >::value) :
        m_data(data),
        m_capacity(capacity),
        m_size(0u),
        m_read_pos(0u)
    {
    }

    small_ring_queue_base(allocator_type const& alloc, pointer data, size_type capacity) noexcept :
        allocator_holder(alloc),
        m_data(data),
        m_capacity(capacity),
        m_size(0u),
        m_read_pos(0u)
    {
    }

    ~small_ring_queue_base() noexcept = default;
#endif // !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

public:
    small_ring_queue_base(small_ring_queue_base const& that) = delete;
    small_ring_queue_base& operator= (small_ring_queue_base const& that) = delete;

    /*!
     * <b>Effects</b>: Returns underlying allocator.
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
    using allocator_holder::get_allocator;
#else
    allocator_type get_allocator() const noexcept;
#endif

    /*!
     * <b>Effects</b>: Returns \c true if the queue is empty, otherwise \c false.
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
    bool empty() const noexcept
    {
        return m_size == 0u;
    }

    /*!
     * <b>Effects</b>: Returns the number of enqueued elements.
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
    size_type size() const noexcept
    {
        return m_size;
    }

    /*!
     * <b>Effects</b>: Returns the allocated storage capacity, in the number of elements.
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
    size_type capacity() const noexcept
    {
        return m_capacity;
    }

    /*!
     * <b>Effects</b>: Returns maximum queue size, in the number of elements.
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
    size_type max_size() const noexcept
    {
        return std::allocator_traits< allocator_type >::max_size(allocator_holder::get_allocator_ref());
    }

    /*!
     * <b>Effects</b>: Destroys all elements in the queue.
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(N).
     */
    void clear() noexcept
    {
        if (m_size > 0u)
        {
            destroy_elements(allocator_holder::get_allocator_ref(), m_data, m_read_pos, m_capacity, m_size);
            m_size = 0u;
            m_read_pos = 0u;
        }
    }

    /*!
     * <b>Effects</b>: Returns a reference to the element at the front of the queue (i.e. the oldest enqueued element).
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
    reference front() noexcept
    {
        BOOST_ASSERT(!empty());
        return m_data[m_read_pos];
    }

    /*!
     * <b>Effects</b>: Returns a reference to the element at the front of the queue (i.e. the oldest enqueued element).
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
    const_reference front() const noexcept
    {
        BOOST_ASSERT(!empty());
        return m_data[m_read_pos];
    }

    /*!
     * <b>Effects</b>: Returns a reference to the element at the back of the queue (i.e. the last enqueued element).
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
    reference back() noexcept
    {
        BOOST_ASSERT(!empty());
        size_type pos = m_read_pos + m_size - 1u;
        if (pos >= m_capacity)
            pos -= m_capacity;
        return m_data[pos];
    }

    /*!
     * <b>Effects</b>: Returns a reference to the element at the back of the queue (i.e. the last enqueued element).
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
    const_reference back() const noexcept
    {
        BOOST_ASSERT(!empty());
        size_type pos = m_read_pos + m_size - 1u;
        if (pos >= m_capacity)
            pos -= m_capacity;
        return m_data[pos];
    }

    /*!
     * <b>Effects</b>: Destroys the front element of the queue (i.e. the oldest enqueued element).
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
    void pop() noexcept
    {
        BOOST_ASSERT(!empty());
        std::allocator_traits< allocator_type >::destroy(allocator_holder::get_allocator_ref(), m_data + m_read_pos);
        --m_size;
        ++m_read_pos;
        if (m_read_pos >= m_capacity)
            m_read_pos = 0u;
    }

#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
protected:
    using allocator_holder::get_allocator_ref;

    //! Performs a shallow swap operation. Assumes the elements are allocated in the dynamically allocated memory.
    void shallow_swap(small_ring_queue_base& that) noexcept(std::is_nothrow_swappable< allocator_type >::value)
    {
        this->swap_allocators(that);
        std::swap(m_data, that.m_data);
        std::swap(m_capacity, that.m_capacity);
        std::swap(m_size, that.m_size);
        std::swap(m_read_pos, that.m_read_pos);
    }

    //! Swaps allocators
    void swap_allocators(small_ring_queue_base& that) noexcept(std::is_nothrow_swappable< allocator_type >::value)
    {
        using namespace std;
        swap(this->allocator_holder::get_allocator_ref(), that.allocator_holder::get_allocator_ref());
    }

    //! Copy-constructs elements in a new storage. The newly constructed elements are linearized at the beginning of the storage.
    static void copy_construct_elements_linearized(const_pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type size)
        noexcept(std::is_nothrow_copy_constructible< value_type >::value)
    {
        copy_construct_elements_linearized(read_p, read_capacity, read_pos, alloc, write_p, write_capacity, size, std::is_nothrow_copy_constructible< value_type >());
    }

    //! Copy-constructs elements in a new storage. The newly constructed elements are linearized at the beginning of the storage.
    static BOOST_FORCEINLINE void copy_construct_elements_linearized(const_pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type size, std::true_type) noexcept
    {
        size_type write_pos = 0u;
        copy_construct_elements_linearized_impl(read_p, read_capacity, read_pos, alloc, write_p, write_capacity, size, write_pos);
    }

    //! Copy-constructs elements in a new storage. The newly constructed elements are linearized at the beginning of the storage.
    static BOOST_FORCEINLINE void copy_construct_elements_linearized(const_pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type size, std::false_type)
    {
        size_type write_pos = 0u;
        try
        {
            copy_construct_elements_linearized_impl(read_p, read_capacity, read_pos, alloc, write_p, write_capacity, size, write_pos);
        }
        catch (...)
        {
            while (write_pos > 0u)
            {
                --write_pos;
                std::allocator_traits< allocator_type >::destroy(alloc, write_p + write_pos);
            }

            throw;
        }
    }

    //! Copy-constructs elements in a new storage. The newly constructed elements are linearized at the beginning of the storage.
    static BOOST_FORCEINLINE void copy_construct_elements_linearized_impl(const_pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type size, size_type& write_pos)
        noexcept(std::is_nothrow_copy_constructible< value_type >::value)
    {
        size_type n = read_pos + size;
        if (n > read_capacity)
            n = read_capacity;

        for (; read_pos < n; ++write_pos, ++read_pos)
        {
            std::allocator_traits< allocator_type >::construct(alloc, write_p + write_pos, read_p[read_pos]);
        }

        read_pos = 0u;
        for (; write_pos < size; ++read_pos, ++write_pos)
        {
            std::allocator_traits< allocator_type >::construct(alloc, write_p + write_pos, read_p[read_pos]);
        }
    }

    //! Move-constructs elements in a new storage. The newly constructed elements are linearized at the beginning of the storage.
    static void move_construct_elements_linearized(pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type size)
        noexcept(std::is_nothrow_move_constructible< value_type >::value)
    {
        move_construct_elements_linearized(read_p, read_capacity, read_pos, alloc, write_p, write_capacity, size, std::is_nothrow_move_constructible< value_type >());
    }

    //! Move-constructs elements in a new storage. The newly constructed elements are linearized at the beginning of the storage.
    static BOOST_FORCEINLINE void move_construct_elements_linearized(pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type size, std::true_type) noexcept
    {
        size_type write_pos = 0u;
        move_construct_elements_linearized_impl(read_p, read_capacity, read_pos, alloc, write_p, write_capacity, size, write_pos);
    }

    //! Move-constructs elements in a new storage. The newly constructed elements are linearized at the beginning of the storage.
    static BOOST_FORCEINLINE void move_construct_elements_linearized(pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type size, std::false_type)
    {
        size_type write_pos = 0u;
        try
        {
            move_construct_elements_linearized_impl(read_p, read_capacity, read_pos, alloc, write_p, write_capacity, size, write_pos);
        }
        catch (...)
        {
            while (write_pos > 0u)
            {
                --write_pos;
                std::allocator_traits< allocator_type >::destroy(alloc, write_p + write_pos);
            }

            throw;
        }
    }

    //! Move-constructs elements in a new storage. The newly constructed elements are linearized at the beginning of the storage.
    static BOOST_FORCEINLINE void move_construct_elements_linearized_impl(pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type size, size_type& write_pos)
        noexcept(std::is_nothrow_move_constructible< value_type >::value)
    {
        size_type n = read_pos + size;
        if (n > read_capacity)
            n = read_capacity;

        for (; read_pos < n; ++write_pos, ++read_pos)
        {
            std::allocator_traits< allocator_type >::construct(alloc, write_p + write_pos, std::move(read_p[read_pos]));
        }

        read_pos = 0u;
        for (; write_pos < size; ++read_pos, ++write_pos)
        {
            std::allocator_traits< allocator_type >::construct(alloc, write_p + write_pos, std::move(read_p[read_pos]));
        }
    }

    //! Move-constructs elements in a new storage
    static void move_construct_elements(pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type write_pos, size_type size)
        noexcept(std::is_nothrow_move_constructible< value_type >::value)
    {
        move_construct_elements(read_p, read_capacity, read_pos, alloc, write_p, write_capacity, write_pos, size, std::is_nothrow_move_constructible< value_type >());
    }

    //! Move-constructs elements in a new storage
    static BOOST_FORCEINLINE void move_construct_elements(pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type write_pos, size_type size, std::true_type) noexcept
    {
        size_type i = 0u;
        move_construct_elements_impl(read_p, read_capacity, read_pos, alloc, write_p, write_capacity, write_pos, size, i);
    }

    //! Move-constructs elements in a new storage
    static BOOST_FORCEINLINE void move_construct_elements(pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type write_pos, size_type size, std::false_type)
    {
        size_type moved_count = 0u;
        try
        {
            move_construct_elements_impl(read_p, read_capacity, read_pos, alloc, write_p, write_capacity, write_pos, size, moved_count);
        }
        catch (...)
        {
            while (moved_count > 0u)
            {
                --moved_count;
                if (write_pos == 0u)
                    write_pos = write_capacity;
                --write_pos;
                std::allocator_traits< allocator_type >::destroy(alloc, write_p + write_pos);
            }

            throw;
        }
    }

    //! Move-constructs elements in a new storage
    static BOOST_FORCEINLINE void move_construct_elements_impl(pointer read_p, size_type read_capacity, size_type read_pos, allocator_type& alloc, pointer write_p, size_type write_capacity, size_type write_pos, size_type size, size_type& moved_count)
        noexcept(std::is_nothrow_move_constructible< value_type >::value)
    {
        size_type n = size, m = read_capacity - read_pos;
        if (n > m)
            n = m;
        m = write_capacity - write_pos;
        if (n > m)
            n = m;

        if (n < size)
        {
            for (size_type i = 0u; i < n; ++moved_count, ++read_pos, ++write_pos, ++i)
            {
                std::allocator_traits< allocator_type >::construct(alloc, write_p + write_pos, std::move(read_p[read_pos]));
            }

            size -= n;

            if (read_pos >= read_capacity)
            {
                read_pos = 0u;
                n = size;
                m = write_capacity - write_pos;
                if (n > m)
                    n = m;
            }

            if (write_pos >= write_capacity)
            {
                write_pos = 0u;
                n = size;
                m = read_capacity - read_pos;
                if (n > m)
                    n = m;
            }

            for (size_type i = 0u; i < n; ++moved_count, ++read_pos, ++write_pos, ++i)
            {
                std::allocator_traits< allocator_type >::construct(alloc, write_p + write_pos, std::move(read_p[read_pos]));
            }

            size -= n;

            if (read_pos >= read_capacity)
                read_pos = 0u;
            if (write_pos >= write_capacity)
                write_pos = 0u;
        }

        for (size_type i = 0u; i < size; ++moved_count, ++read_pos, ++write_pos, ++i)
        {
            std::allocator_traits< allocator_type >::construct(alloc, write_p + write_pos, std::move(read_p[read_pos]));
        }
    }

    //! Swaps elements in two storages
    static void swap_elements(pointer left_p, size_type left_capacity, size_type left_pos, pointer right_p, size_type right_capacity, size_type right_pos, size_type size)
        noexcept(std::is_nothrow_swappable< value_type >::value)
    {
        using namespace std;

        size_type n = size, m = left_capacity - left_pos;
        if (n > m)
            n = m;
        m = right_capacity - right_pos;
        if (n > m)
            n = m;

        if (n < size)
        {
            for (size_type i = 0u; i < n; ++left_pos, ++right_pos, ++i)
            {
                swap(left_p[left_pos], right_p[right_pos]);
            }

            size -= n;

            if (left_pos >= left_capacity)
            {
                left_pos = 0u;
                n = size;
                m = right_capacity - right_pos;
                if (n > m)
                    n = m;
            }

            if (right_pos >= right_capacity)
            {
                right_pos = 0u;
                n = size;
                m = left_capacity - left_pos;
                if (n > m)
                    n = m;
            }

            for (size_type i = 0u; i < n; ++left_pos, ++right_pos, ++i)
            {
                swap(left_p[left_pos], right_p[right_pos]);
            }

            size -= n;

            if (left_pos >= left_capacity)
                left_pos = 0u;
            if (right_pos >= right_capacity)
                right_pos = 0u;
        }

        for (size_type i = 0u; i < size; ++left_pos, ++right_pos, ++i)
        {
            swap(left_p[left_pos], right_p[right_pos]);
        }
    }

    //! Destroys elements
    static void destroy_elements(allocator_type& alloc, pointer p, size_type pos, size_type capacity, size_type size) noexcept
    {
        size_type n = pos + size;
        if (n > capacity)
            n = capacity;

        size_type i = pos;
        for (; i < n; ++i)
        {
            std::allocator_traits< allocator_type >::destroy(alloc, p + i);
        }

        size -= i - pos;
        for (i = 0u; i < size; ++i)
        {
            std::allocator_traits< allocator_type >::destroy(alloc, p + i);
        }
    }
#endif // !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
};

/*!
 * \brief FIFO queue with circular buffer implementation
 *
 * \c small_ring_queue is a FIFO queue, similar to \c std::queue, with two essential differences:
 *
 * - the queue uses an internal circular buffer to store elements instead of a container, and
 * - the queue allows to allocate internal storage for a (small) number of elements.
 *
 * These features make \c small_ring_queue a better choice compared to \c std::queue in cases where
 * the average number of enqueued elements usually does not exceed some threshold and dynamic memory
 * allocations are undesirable.
 *
 * The usage of the queue is very similar to \c std::queue. Producers enqueue elements by calling \c push
 * or \c emplace. Consumers can access elements by calling \c front, \c back and remove elements by calling \c pop.
 * Additionally, the queue provides methods for managing buffer capacity, such as \c reserve and \c shrink_to_fit,
 * which have the same semantics as those for \c std::vector.
 *
 * \tparam T The type of the element of the queue.
 * \tparam StaticCapacity The number of elements, for which to allocate internal storage. Affects queue object size.
 * \tparam Allocator The allocator to use to allocate dynamic memory. By default, \c std::allocator will be used.
 */
template< typename T, std::size_t StaticCapacity, typename Allocator >
class small_ring_queue :
    public small_ring_queue_base< T, Allocator >
#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
    , private container::detail::small_ring_queue_storage<
        T,
        StaticCapacity,
        typename container::detail::ring_queue_make_allocator_type< T, Allocator >::type
    >
#endif
{
#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
private:
    typedef small_ring_queue_base< T, Allocator > base_type;
    typedef container::detail::small_ring_queue_storage<
        T,
        StaticCapacity,
        typename container::detail::ring_queue_make_allocator_type< T, Allocator >::type
    > storage_base_type;
#endif

public:
    typedef typename base_type::value_type value_type;
    typedef typename base_type::allocator_type allocator_type;
    typedef typename base_type::pointer pointer;
    typedef typename base_type::const_pointer const_pointer;
    typedef typename base_type::reference reference;
    typedef typename base_type::const_reference const_reference;
    typedef typename base_type::size_type size_type;
    typedef typename base_type::difference_type difference_type;

    //! Amount of storage available without dynamic memory allocation
#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
    using storage_base_type::static_capacity;
#else
    static constexpr size_type static_capacity = StaticCapacity;
#endif

public:
    /*!
     * <b>Effects</b>: Constructs an empty queue.
     *
     * <b>Throws</b>: Nothing, unless default constructor of \c allocator_type throws.
     *
     * <b>Complexity</b>: Constant.
     */
    constexpr small_ring_queue() noexcept(std::is_nothrow_default_constructible< allocator_type >::value) :
        base_type(storage_base_type::get_internal_storage(), storage_base_type::static_capacity)
    {
    }

    /*!
     * <b>Effects</b>: Constructs an empty queue, taking the allocator as a parameter.
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: Constant.
     */
    explicit small_ring_queue(allocator_type const& alloc) noexcept :
        base_type(alloc, storage_base_type::get_internal_storage(), storage_base_type::static_capacity)
    {
    }

    /*!
     * <b>Effects</b>: Move-constructs a queue. If the move source uses internal storage to store its elements,
     *                 the elements are moved into the new storage by calling \c value_type's move constructor.
     *
     * <b>Postcondition</b>: \c *this contains moved elements of \c that.
     *
     * <b>Throws</b>: Nothing, unless move constructor of \c value_type throws.
     *
     * <b>Complexity</b>: O(N), if the move source uses internal storage, O(1) otherwise.
     */
    small_ring_queue(small_ring_queue&& that) noexcept(std::is_nothrow_move_constructible< value_type >::value) :
        base_type(that.base_type::get_allocator_ref(), storage_base_type::get_internal_storage(), storage_base_type::static_capacity)
    {
        if (!that.uses_internal_storage())
        {
            this->m_data = that.m_data;
            this->m_capacity = that.m_capacity;
            this->m_size = that.m_size;
            this->m_read_pos = that.m_read_pos;

            that.m_data = that.storage_base_type::get_internal_storage();
            that.m_capacity = storage_base_type::static_capacity;
            that.m_size = 0u;
            that.m_read_pos = 0u;
        }
        else
        {
            base_type::move_construct_elements(that.m_data, that.m_capacity, that.m_read_pos, this->base_type::get_allocator_ref(), this->m_data, this->m_capacity, that.m_size);
            that.clear();
        }
    }

    /*!
     * <b>Effects</b>: Copy-constructs a queue. The elements are copied into the new storage by calling
     *                 \c value_type's copy constructor.
     *
     * <b>Postcondition</b>: \c *this contains copied elements of \c that.
     *
     * <b>Throws</b>: If \c allocator_type::allocate or copy constructor of \c value_type throws.
     *
     * <b>Complexity</b>: O(N).
     */
    small_ring_queue(small_ring_queue const& that) :
        base_type(that.base_type::get_allocator_ref(), storage_base_type::get_internal_storage(), storage_base_type::static_capacity)
    {
        const size_type size = that.m_size;
        reserve(size);
        try
        {
            base_type::copy_construct_elements_linearized(that.m_data, that.m_capacity, that.m_read_pos, this->base_type::get_allocator_ref(), this->m_data, this->m_capacity, size);
        }
        catch (...)
        {
            if (!uses_internal_storage())
                std::allocator_traits< allocator_type >::deallocate(this->base_type::get_allocator_ref(), this->m_data, this->m_capacity);
            throw;
        }
        this->m_size = size;
    }

    /*!
     * <b>Effects</b>: Destroys enqueued elements and the queue, releasing any allocated memory.
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(N).
     */
    ~small_ring_queue() noexcept
    {
        this->clear();

        if (!uses_internal_storage())
            std::allocator_traits< allocator_type >::deallocate(this->base_type::get_allocator_ref(), this->m_data, this->m_capacity);
    }

    /*!
     * <b>Effects</b>: Assigns to the queue. If the assignment source uses internal storage to store its elements,
     *                 the elements are moved into the new storage by calling \c value_type's move constructor or swapping.
     *
     * <b>Postcondition</b>: \c *this contains moved or copied elements of \c that.
     *
     * <b>Throws</b>: If \c allocator_type::allocate or swapping of \c value_type throws.
     *
     * <b>Complexity</b>: O(N), if the move source uses internal storage, O(1) otherwise.
     *
     * <b>Note</b>: The assignment source is copy- or move-constructed first, before assignment. See copy- and move-constructor
     *              documentation for additional complexity and sources of exceptions.
     */
    small_ring_queue& operator= (small_ring_queue that) noexcept(noexcept(that.swap(that)))
    {
        this->swap(that);
        return *this;
    }

    /*!
     * <b>Effects</b>: Returns underlying allocator.
     *
     * <b>Throws</b>: Nothing.
     *
     * <b>Complexity</b>: O(1).
     */
#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
    using base_type::get_allocator;
#else
    allocator_type get_allocator() const noexcept;
#endif

    /*!
     * <b>Effects</b>: Reserves memory for the given number of elements. Does not reduce the already allocated memory.
     *
     * <b>Postcondition</b>: <tt>this->capacity() >= capacity</tt>.
     *
     * <b>Throws</b>: If <tt>capacity > this->max_size()</tt>, throws \c std::length_error. Otherwise, if \c allocator_type::allocate
     *                or copy constructor of \c value_type throws.
     *
     * <b>Complexity</b>: O(N) if memory reallocation happens.
     */
    void reserve(size_type capacity)
    {
        if (this->m_capacity < capacity)
            do_reserve(capacity);
    }

    /*!
     * <b>Effects</b>: Shrinks allocated buffer to the current queue size plus an extra headroom. Does not reduce the capacity
     *                 below internal storage capacity.
     *
     * <b>Postcondition</b>: <tt>this->capacity() >= this->size() + extra</tt>.
     *
     * <b>Throws</b>: If <tt>this->size() + extra > this->max_size()</tt>, throws \c std::length_error. Otherwise, if \c allocator_type::allocate
     *                or copy constructor of \c value_type throws.
     *
     * <b>Complexity</b>: O(N) if memory reallocation happens.
     *
     * <b>Note</b>: The \a extra argument is a non-standard extension, compared to \c std::vector::shrink_to_fit.
     */
    void shrink_to_fit(size_type extra = 0u)
    {
        size_type new_capacity = this->m_size + extra;
        if (BOOST_UNLIKELY(new_capacity < this->m_size))
            BOOST_THROW_EXCEPTION(std::length_error("Too many extra elements requested in boost::container::small_ring_queue::shrink_to_fit"));
        if (new_capacity < storage_base_type::static_capacity)
            new_capacity = storage_base_type::static_capacity;

        if (new_capacity == 0u)
        {
            // This case is only possible if there is no internal storage
            if (this->m_data)
            {
                std::allocator_traits< allocator_type >::deallocate(this->base_type::get_allocator_ref(), this->m_data, this->m_capacity);
                this->m_data = nullptr;
                this->m_capacity = 0u;
                this->m_read_pos = 0u;
            }
        }
        else if (new_capacity == storage_base_type::static_capacity)
        {
            // This case is only possible if there is internal storage
            if (!uses_internal_storage())
            {
                allocator_type& alloc = this->base_type::get_allocator_ref();
                if constexpr(std::is_nothrow_move_constructible< value_type >::value)
                {
                    base_type::move_construct_elements_linearized(this->m_data, this->m_capacity, this->m_read_pos, alloc, this->storage_base_type::get_internal_storage(), storage_base_type::static_capacity, this->m_size);
                }
                else
                {
                    base_type::copy_construct_elements_linearized(this->m_data, this->m_capacity, this->m_read_pos, alloc, this->storage_base_type::get_internal_storage(), storage_base_type::static_capacity, this->m_size);
                }

                base_type::destroy_elements(alloc, this->m_data, this->m_read_pos, this->m_capacity, this->m_size);
                std::allocator_traits< allocator_type >::deallocate(alloc, this->m_data, this->m_capacity);

                this->m_data = this->storage_base_type::get_internal_storage();
                this->m_capacity = storage_base_type::static_capacity;
                // The above move_construct_elements_linearized/copy_construct_elements_linearized wrote elements linearized starting at the beginning of the internal storage
                this->m_read_pos = 0u;
            }
        }
        else if (new_capacity != this->m_capacity)
        {
            do_reserve(new_capacity);
        }
    }

    /*!
     * <b>Effects</b>: Constructs a new element at the back of the queue using \c args.
     *
     * <b>Returns</b>: A reference to the constructed element.
     *
     * <b>Throws</b>: If \c allocator_type::allocate, copy constructor of \c value_type or initializing constructor from \c args throws.
     *
     * <b>Complexity</b>: O(N) if memory reallocation happens, otherwise O(1).
     */
    template< typename... Args >
    reference emplace(Args&&... args)
    {
        const size_type new_size = this->m_size + 1u;
        if (this->m_capacity < new_size)
        {
            size_type new_capacity = this->m_capacity == 0u ? static_cast< size_type >(2u) : (this->m_capacity + this->m_capacity / 2u);
            if (new_capacity < new_size)
                new_capacity = new_size;
            if (BOOST_UNLIKELY(new_capacity < this->m_capacity))
                BOOST_THROW_EXCEPTION(std::length_error("Too many elements in a boost::container::small_ring_queue"));
            do_reserve(new_capacity);
        }

        size_type pos = this->m_read_pos + this->m_size;
        if (pos >= this->m_capacity)
            pos -= this->m_capacity;

        pointer p = this->m_data + pos;
        std::allocator_traits< allocator_type >::construct(this->base_type::get_allocator_ref(), p, std::forward< Args >(args)...);
        ++(this->m_size);

        return *p;
    }

    /*!
     * <b>Effects</b>: Copy-constructs a new element at the back of the queue.
     *
     * <b>Throws</b>: If \c allocator_type::allocate or copy constructor of \c value_type throws.
     *
     * <b>Complexity</b>: O(N) if memory reallocation happens, otherwise O(1).
     */
    void push(const_reference elem)
    {
        this->emplace(elem);
    }

    /*!
     * <b>Effects</b>: Move-constructs a new element at the back of the queue.
     *
     * <b>Throws</b>: If \c allocator_type::allocate, copy or move constructor of \c value_type throws.
     *
     * <b>Complexity</b>: O(N) if memory reallocation happens, otherwise O(1).
     */
    void push(value_type&& elem)
    {
        this->emplace(std::move(elem));
    }

    /*!
     * <b>Effects</b>: Swaps two queues. If one of the two queues uses internal storage, the operation is
     *                 performed element-wise by calling unqualified \c swap or \c value_type's move constructor.
     *
     * <b>Throws</b>: If \c swap or move constructor of \c value_type throws.
     *
     * <b>Complexity</b>: O(N) if either of the queues uses internal storage, otherwise O(1).
     */
    void swap(small_ring_queue& that) noexcept(std::is_nothrow_swappable< allocator_type >::value && std::is_nothrow_swappable< value_type >::value && std::is_nothrow_move_constructible< value_type >::value)
    {
        if (uses_internal_storage())
        {
            if (that.uses_internal_storage())
                this->deep_swap(that);
            else
                that.semideep_swap(*this);

        }
        else if (that.uses_internal_storage())
        {
            this->semideep_swap(that);
        }
        else
        {
            base_type::shallow_swap(static_cast< base_type& >(that));
        }
    }

    /*!
     * <b>Effects</b>: Swaps two queues. If one of the two queues uses internal storage, the operation is
     *                 performed element-wise by calling unqualified \c swap or \c value_type's move constructor.
     *
     * <b>Throws</b>: If \c swap or move constructor of \c value_type throws.
     *
     * <b>Complexity</b>: O(N) if either of the queues uses internal storage, otherwise O(1).
     */
    friend void swap(small_ring_queue& left, small_ring_queue& right) noexcept(noexcept(left.swap(right)))
    {
        left.swap(right);
    }

#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
private:
    //! Returns \c true if the queue uses internal storage
    BOOST_FORCEINLINE bool uses_internal_storage() const noexcept
    {
        return storage_base_type::static_capacity > 0u && this->m_data == this->storage_base_type::get_internal_storage();
    }

    //! Reserves memory for the given number of elements
    void do_reserve(size_type capacity)
    {
        if (BOOST_UNLIKELY(capacity > this->max_size()))
            BOOST_THROW_EXCEPTION(std::length_error("Too many elements in a boost::container::small_ring_queue"));

        allocator_type& alloc = this->base_type::get_allocator_ref();
        const pointer write_p = std::allocator_traits< allocator_type >::allocate(alloc, capacity);
        const pointer read_p = this->m_data;
        const size_type read_capacity = this->m_capacity;
        const size_type size = this->m_size;
        if (size > 0u)
        {
            size_type read_pos = this->m_read_pos;

            try
            {
                if constexpr(std::is_nothrow_move_constructible< value_type >::value)
                {
                    base_type::move_construct_elements_linearized(read_p, read_capacity, read_pos, alloc, write_p, capacity, size);
                }
                else
                {
                    base_type::copy_construct_elements_linearized(read_p, read_capacity, read_pos, alloc, write_p, capacity, size);
                }
            }
            catch (...)
            {
                std::allocator_traits< allocator_type >::deallocate(alloc, write_p, capacity);
                throw;
            }

            base_type::destroy_elements(alloc, read_p, read_pos, read_capacity, size);
        }

        if (!uses_internal_storage())
            std::allocator_traits< allocator_type >::deallocate(alloc, read_p, read_capacity);

        this->m_data = write_p;
        this->m_capacity = capacity;
        // The above move_construct_elements_linearized/copy_construct_elements_linearized wrote elements linearized starting at the beginning of the internal storage
        this->m_read_pos = 0u;
    }

    //! Performs a element-wise swap operation
    void deep_swap(small_ring_queue& that) noexcept(std::is_nothrow_swappable< allocator_type >::value && std::is_nothrow_swappable< value_type >::value && std::is_nothrow_move_constructible< value_type >::value)
    {
        BOOST_ASSERT(uses_internal_storage() && that.uses_internal_storage());

        size_type swap_size = this->m_size;
        if (swap_size > that.m_size)
            swap_size = that.m_size;

        base_type::swap_elements(this->m_data, this->m_capacity, this->m_read_pos, that.m_data, that.m_capacity, that.m_read_pos, swap_size);

        if (swap_size < this->m_size)
        {
            size_type move_size = this->m_size - swap_size;
            size_type read_pos = this->m_read_pos + swap_size;
            if (read_pos >= this->m_capacity)
                read_pos -= this->m_capacity;

            size_type write_pos = that.m_read_pos + swap_size;
            if (write_pos >= that.m_capacity)
                write_pos -= that.m_capacity;

            base_type::move_construct_elements(this->m_data, this->m_capacity, read_pos, that.base_type::get_allocator_ref(), that.m_data, that.m_capacity, write_pos, move_size);
            base_type::destroy_elements(this->base_type::get_allocator_ref(), this->m_data, read_pos, this->m_capacity, move_size);
        }
        else if (swap_size < that.m_size)
        {
            size_type move_size = that.m_size - swap_size;
            size_type read_pos = that.m_read_pos + swap_size;
            if (read_pos >= that.m_capacity)
                read_pos -= that.m_capacity;

            size_type write_pos = this->m_read_pos + swap_size;
            if (write_pos >= this->m_capacity)
                write_pos -= this->m_capacity;

            base_type::move_construct_elements(that.m_data, that.m_capacity, read_pos, this->base_type::get_allocator_ref(), this->m_data, this->m_capacity, write_pos, move_size);
            base_type::destroy_elements(that.base_type::get_allocator_ref(), that.m_data, read_pos, that.m_capacity, move_size);
        }

        this->base_type::swap_allocators(that);
        std::swap(this->m_size, that.m_size);
    }

    //! Performs a element-wise move operation in one direction and moves the dynamic storage in the other
    void semideep_swap(small_ring_queue& that) noexcept(std::is_nothrow_swappable< allocator_type >::value && std::is_nothrow_swappable< value_type >::value && std::is_nothrow_move_constructible< value_type >::value)
    {
        BOOST_ASSERT(!uses_internal_storage() && that.uses_internal_storage());

        pointer write_p = this->storage_base_type::get_internal_storage();
        allocator_type& alloc = that.base_type::get_allocator_ref();
        base_type::move_construct_elements_linearized(that.m_data, that.m_capacity, that.m_read_pos, alloc, write_p, storage_base_type::static_capacity, that.m_size);
        base_type::destroy_elements(alloc, that.m_data, that.m_read_pos, that.m_capacity, that.m_size);

        this->base_type::swap_allocators(that);

        that.m_data = this->m_data;
        this->m_data = write_p;

        std::swap(this->m_capacity, that.m_capacity);
        std::swap(this->m_size, that.m_size);

        that.m_read_pos = this->m_read_pos;
        this->m_read_pos = 0u;
    }
#endif // !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
};

}} // namespace boost::container

#endif // BOOST_CONTAINER_SMALL_RING_QUEUE_HPP_INCLUDED_
