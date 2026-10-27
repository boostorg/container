//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Benedek Thaler 2015-2016
// (C) Copyright Ion Gaztanaga 2019-2020. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DEVECTOR_HPP
#define BOOST_CONTAINER_DEVECTOR_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>

#include <cstring> // memcpy

#include <boost/assert.hpp>

#include <boost/container/detail/copy_move_algo.hpp>
#include <boost/container/new_allocator.hpp> //new_allocator
#include <boost/container/allocator_traits.hpp> //allocator_traits
#include <boost/container/detail/algorithm.hpp> //equal()
#include <boost/container/throw_exception.hpp>
#include <boost/container/options.hpp>

#include <boost/container/detail/guards_dended.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/container/detail/iterators.hpp>
#include <boost/container/detail/destroyers.hpp>
#include <boost/container/detail/min_max.hpp>
#include <boost/container/detail/next_capacity.hpp>
#include <boost/container/detail/alloc_helpers.hpp>
#include <boost/container/detail/advanced_insert_int.hpp>
#include <boost/container/detail/addressof.hpp>

// move
#if defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
#include <boost/move/detail/fwd_macros.hpp>
#endif
#include <boost/move/detail/move_helpers.hpp>
#include <boost/move/adl_move_swap.hpp>
#include <boost/move/iterator.hpp>
#include <boost/move/traits.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/move/detail/to_raw_pointer.hpp>
#include <boost/move/algo/detail/rotate.hpp>

//std
#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
#include <initializer_list>    //for std::initializer_list
#endif

namespace boost {
namespace container {

#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

struct growth_factor_60;

template<class Options, class AllocatorSizeType>
struct get_devector_opt
{
    typedef devector_opt< typename default_if_void<typename Options::growth_factor_type, growth_factor_60>::type
                        , typename default_if_void<typename Options::stored_size_type, AllocatorSizeType>::type
                        , default_if_zero<Options::free_fraction, relocate_on_90::value>::value
                        > type;
};

template<class AllocatorSizeType>
struct get_devector_opt<void, AllocatorSizeType>
{
    typedef devector_opt< growth_factor_60, AllocatorSizeType, relocate_on_90::value> type;
};

namespace dtl {

//The buffer and the indices of a devector, and the members that only use them.
//They only depend on the pointer and the size types, so they are compiled once
//per pointer and size type instead of once per devector type.
template<class Pointer, class SizeType, class StoredSizeType>
struct devector_base
{
   typedef Pointer                                                          pointer;
   typedef typename boost::intrusive::pointer_traits<Pointer>::element_type value_type;
   typedef typename boost::intrusive::pointer_traits<Pointer>::template
      rebind_pointer<const value_type>::type                                const_pointer;
   typedef value_type &                                                     reference;
   typedef const value_type &                                               const_reference;
   typedef SizeType                                                         size_type;
   typedef pointer                                                          iterator;
   typedef const_pointer                                                    const_iterator;
   typedef boost::container::reverse_iterator<iterator>                     reverse_iterator;
   typedef boost::container::reverse_iterator<const_iterator>               const_reverse_iterator;

   devector_base()
      : m_buffer(), m_front_idx(), m_back_idx(), m_capacity()
      #ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS
      , m_capacity_alloc_count(0)
      #endif
   {}

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
      bool empty() const BOOST_NOEXCEPT
   {  return m_front_idx == m_back_idx;  }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
      SizeType size() const BOOST_NOEXCEPT
   {  return SizeType(m_back_idx - m_front_idx);  }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
      SizeType front_free_capacity() const BOOST_NOEXCEPT
   {  return m_front_idx;  }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
      SizeType back_free_capacity() const BOOST_NOEXCEPT
   {  return SizeType(m_capacity - m_back_idx);  }

   BOOST_CONTAINER_FORCEINLINE SizeType front_capacity() const
   {  return m_back_idx;  }

   BOOST_CONTAINER_FORCEINLINE SizeType back_capacity() const
   {  return SizeType(m_capacity - m_front_idx);  }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
         iterator begin() BOOST_NOEXCEPT
   {
      return this->m_buffer + this->m_front_idx;
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
    const_iterator begin() const BOOST_NOEXCEPT
   {
      return this->m_buffer + this->m_front_idx;
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
         iterator end() BOOST_NOEXCEPT
   {
      return this->m_buffer + this->m_back_idx;
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
    const_iterator end() const BOOST_NOEXCEPT
   {
      return this->m_buffer + this->m_back_idx;
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
    reverse_iterator rbegin() BOOST_NOEXCEPT
   {
      return reverse_iterator(this->m_buffer + this->m_back_idx);
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
    const_reverse_iterator rbegin() const BOOST_NOEXCEPT
   {
      return const_reverse_iterator(this->m_buffer + this->m_back_idx);
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
    reverse_iterator rend() BOOST_NOEXCEPT
   {
      return reverse_iterator(this->m_buffer + this->m_front_idx);
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
    const_reverse_iterator rend() const BOOST_NOEXCEPT
   {
      return const_reverse_iterator(this->m_buffer + this->m_front_idx);
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
    const_iterator cbegin() const BOOST_NOEXCEPT
   {
      return this->m_buffer + this->m_front_idx;
   }

   BOOST_CONTAINER_FORCEINLINE const_iterator cend() const BOOST_NOEXCEPT
   {
      return this->m_buffer + this->m_back_idx;
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
    const_reverse_iterator crbegin() const BOOST_NOEXCEPT
   {
      return const_reverse_iterator(this->m_buffer + this->m_back_idx);
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
    const_reverse_iterator crend() const BOOST_NOEXCEPT
   {
      return const_reverse_iterator(this->m_buffer + this->m_front_idx);
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
   reference operator[](size_type n) BOOST_NOEXCEPT
   {
      BOOST_ASSERT(n < size());
      return this->m_buffer[this->m_front_idx + n];
   }

   BOOST_CONTAINER_NODISCARD inline
   const_reference operator[](size_type n) const BOOST_NOEXCEPT
   {
      BOOST_ASSERT(n < size());
      return this->m_buffer[this->m_front_idx + n];
   }

   BOOST_CONTAINER_NODISCARD inline
      iterator nth(size_type n) BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(n <= size());
      return iterator(this->m_buffer + (this->m_front_idx + n));
   }

   BOOST_CONTAINER_NODISCARD inline
      const_iterator nth(size_type n) const BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(n <= size());
      return const_iterator(this->m_buffer + (this->m_front_idx + n));
   }

   BOOST_CONTAINER_NODISCARD inline
      size_type index_of(iterator p) BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(p >= begin());
      BOOST_ASSERT(p <= end());
      return static_cast<size_type>(p - this->begin());
   }

   BOOST_CONTAINER_NODISCARD inline
      size_type index_of(const_iterator p) const BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(p >= cbegin());
      BOOST_ASSERT(p <= cend());
      return static_cast<size_type>(p - this->cbegin());
   }

   BOOST_CONTAINER_NODISCARD inline
      reference at(size_type n)
   {
      if (BOOST_UNLIKELY(size() <= n))
            throw_out_of_range("devector::at out of range");
      return (*this)[n];
   }

   BOOST_CONTAINER_NODISCARD inline
      const_reference at(size_type n) const
   {
      if (BOOST_UNLIKELY(size() <= n))
            throw_out_of_range("devector::at out of range");
      return (*this)[n];
   }

   BOOST_CONTAINER_NODISCARD inline
      reference front() BOOST_NOEXCEPT
   {
      BOOST_ASSERT(!empty());
      return this->m_buffer[this->m_front_idx];
   }

   BOOST_CONTAINER_NODISCARD inline
      const_reference front() const BOOST_NOEXCEPT
   {
      BOOST_ASSERT(!empty());
      return this->m_buffer[this->m_front_idx];
   }

   BOOST_CONTAINER_NODISCARD inline
      reference back() BOOST_NOEXCEPT
   {
      BOOST_ASSERT(!empty());
      return this->m_buffer[this->m_back_idx - 1u];
   }

   BOOST_CONTAINER_NODISCARD inline
      const_reference back() const BOOST_NOEXCEPT
   {
      BOOST_ASSERT(!empty());
      return this->m_buffer[this->m_back_idx - 1u];
   }

   BOOST_CONTAINER_NODISCARD inline
      value_type* data() BOOST_NOEXCEPT
   {
      return boost::movelib::to_raw_pointer(this->m_buffer) + this->m_front_idx;
   }

   BOOST_CONTAINER_NODISCARD inline
      const value_type* data() const BOOST_NOEXCEPT
   {
      return boost::movelib::to_raw_pointer(this->m_buffer) + this->m_front_idx;
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
   size_type pos_to_index(const_iterator i) const 
   {
      return static_cast<size_type>(i - cbegin());
   }

   BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE
   bool should_move_back(const_iterator i) const 
   {
      return static_cast<size_type>(this->pos_to_index(i)) >= this->size()/2u;
   }

   inline static iterator unconst_iterator(const_iterator i)
   {
      return boost::intrusive::pointer_traits<pointer>::const_cast_from(i);
   }

   BOOST_CONTAINER_FORCEINLINE value_type* priv_raw_begin() BOOST_NOEXCEPT
   {   return boost::movelib::to_raw_pointer(this->m_buffer) + this->m_front_idx;   }

   BOOST_CONTAINER_FORCEINLINE value_type* priv_raw_end() BOOST_NOEXCEPT
   {   return boost::movelib::to_raw_pointer(this->m_buffer) + this->m_back_idx;    }

   bool priv_is_element_address(const void *p) const
   {
      const value_type *const b = boost::movelib::to_raw_pointer(this->data());
      const std::size_t a = reinterpret_cast<std::size_t>(p);
      return reinterpret_cast<std::size_t>(b) <= a
          && a < reinterpret_cast<std::size_t>(b + this->size());
   }

   bool priv_insertion_moves_elements(const_iterator pos, size_type n) const
   {
      return n != 0u
         && !(pos == this->cend()   && n <= this->back_free_capacity())
         && !(pos == this->cbegin() && n <= this->front_free_capacity());
   }

   bool invariants_ok()
   {
      return  (! this->m_capacity || this->m_buffer )
              && this->m_front_idx <= this->m_back_idx
              && this->m_back_idx <= this->m_capacity;
   }

   #ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS
   void reset_alloc_stats()
   {
      this->m_capacity_alloc_count = 0;
   }

   size_type get_alloc_count() const
   {
      return this->m_capacity_alloc_count;
   }
   #endif // ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS

   BOOST_CONTAINER_FORCEINLINE void set_back_idx(SizeType bi)
   {  m_back_idx = static_cast<StoredSizeType>(bi);  }

   BOOST_CONTAINER_FORCEINLINE void set_front_idx(SizeType fi)
   {  m_front_idx = static_cast<StoredSizeType>(fi);  }

   BOOST_CONTAINER_FORCEINLINE void set_capacity(SizeType c)
   {  m_capacity = static_cast<StoredSizeType>(c);  }

   Pointer         m_buffer;
   StoredSizeType  m_front_idx;
   StoredSizeType  m_back_idx;
   StoredSizeType  m_capacity;
   #ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS
   SizeType        m_capacity_alloc_count;
   #endif
};

//Holds the allocator of a devector as a base, so that an empty allocator takes no
//space. devector derives privately from it, so only devector can reach the allocator.
template<class Allocator>
struct devector_alloc_holder
   : public Allocator
{
   BOOST_CONTAINER_DEFAULTED_DEFAULT_CTOR(devector_alloc_holder(), {})

   template<class A>
   BOOST_CONTAINER_FORCEINLINE explicit devector_alloc_holder(BOOST_FWD_REF(A) a)
      : Allocator(::boost::forward<A>(a))
   {}
};

template<class T, class A, class Options>
struct devector_base_of
{
   typedef allocator_traits<typename real_allocator<T, A>::type> alloc_traits_t;
   typedef typename alloc_traits_t::size_type size_type;
   typedef devector_base
      < typename alloc_traits_t::pointer
      , size_type
      , typename get_devector_opt<Options, size_type>::type::stored_size_type> type;
};

}  //namespace dtl {

#endif    //#if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

struct reserve_only_tag_t {};
struct reserve_uninitialized_t {};

//struct unsafe_uninitialized_tag_t {};

/**
 * A vector-like sequence container providing front and back operations
 * (e.g: `push_front`/`pop_front`/`push_back`/`pop_back`) with amortized constant complexity.
 *
 * Models the [SequenceContainer], [ReversibleContainer], and [AllocatorAwareContainer] concepts.
 *
 * **Requires**:
 *   - `T` shall be [MoveInsertable] into the devector.
 *   - `T` shall be [Erasable] from any `devector<T, allocator_type, GP>`.
 *   - `GrowthFactor`, and `Allocator` must model the concepts with the same names or be void.
 *
 * **Definition**: `T` is `NothrowConstructible` if it's either nothrow move constructible or
 * nothrow copy constructible.
 *
 * **Definition**: `T` is `NothrowAssignable` if it's either nothrow move assignable or
 * nothrow copy assignable.
 *
 * **Exceptions**: The exception specifications assume `T` is nothrow [Destructible].
 *
 * Most methods providing the strong exception guarantee assume that the move constructor of `T`
 * does not throw. Like `vector`, `devector` moves the elements when it reallocates or relocates
 * them (it does not use `move_if_noexcept`), also when `T` is [CopyInsertable]. If the move
 * constructor throws, only the basic exception guarantee is given.
 *
 * In addition to the exceptions specified in the **Throws** clause, the following operations
 * of `T` can throw when any of the specified concept is required:
 *   - [DefaultInsertable][]: Default constructor
 *   - [MoveInsertable][]: Move constructor
 *   - [CopyInsertable][]: Copy constructor
 *   - [DefaultConstructible][]: Default constructor
 *   - [EmplaceConstructible][]: Constructor selected by the given arguments
 *   - [MoveAssignable][]: Move assignment operator
 *   - [CopyAssignable][]: Copy assignment operator
 *
 * Furthermore, not `noexcept` methods throws whatever the allocator throws
 * if memory allocation fails. Such methods also throw `length_error` if the capacity
 * exceeds `max_size()`.
 *
 * **Remark**: If a method invalidates some iterators, it also invalidates references
 * and pointers to the elements pointed by the invalidated iterators.
 *
 *! \tparam Options A type produced from \c boost::container::devector_options.
 *
 * [SequenceContainer]: http://en.cppreference.com/w/cpp/concept/SequenceContainer
 * [ReversibleContainer]: http://en.cppreference.com/w/cpp/concept/ReversibleContainer
 * [AllocatorAwareContainer]: http://en.cppreference.com/w/cpp/concept/AllocatorAwareContainer
 * [DefaultInsertable]: http://en.cppreference.com/w/cpp/concept/DefaultInsertable
 * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
 * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
 * [Erasable]: http://en.cppreference.com/w/cpp/concept/Erasable
 * [DefaultConstructible]: http://en.cppreference.com/w/cpp/concept/DefaultConstructible
 * [Destructible]: http://en.cppreference.com/w/cpp/concept/Destructible
 * [EmplaceConstructible]: http://en.cppreference.com/w/cpp/concept/EmplaceConstructible
 * [MoveAssignable]: http://en.cppreference.com/w/cpp/concept/MoveAssignable
 * [CopyAssignable]: http://en.cppreference.com/w/cpp/concept/CopyAssignable
 */
template < typename T, class A BOOST_CONTAINER_DOCONLY(= void), class Options BOOST_CONTAINER_DOCONLY(= void)>
class devector
   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   : private dtl::devector_base_of<T, A, Options>::type
   , private dtl::devector_alloc_holder<typename real_allocator<T, A>::type>
   #endif
{
   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   typedef boost::container::allocator_traits
      <typename real_allocator<T, A>::type>                                      allocator_traits_type;
   typedef typename allocator_traits_type::size_type                             alloc_size_type;
   typedef typename get_devector_opt<Options, alloc_size_type>::type             options_type;
   typedef typename options_type::growth_factor_type                             growth_factor_type;
   typedef typename options_type::stored_size_type                               stored_size_type;
   BOOST_CONTAINER_STATIC_ASSERT_MSG( (stored_size_type(-1) > stored_size_type(0))
                                    , "stored_size must be an unsigned integer type");
   BOOST_STATIC_CONSTEXPR std::size_t devector_min_free_fraction =
      options_type::free_fraction;
   BOOST_STATIC_CONSTEXPR bool devector_never_relocates =
      devector_min_free_fraction == std::size_t(-1);
   BOOST_CONTAINER_STATIC_ASSERT_MSG( devector_min_free_fraction >= 2u
                                    , "relocate_on<Fraction> needs Fraction >= 2");
   //The buffer, the indices and the members that only use them
   typedef typename dtl::devector_base_of<T, A, Options>::type                   base_t;
   typedef dtl::devector_alloc_holder<typename real_allocator<T, A>::type>       alloc_holder_t;

   #endif // ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   public:
   // Standard Interface Types:
   typedef T                                                                     value_type;
   typedef BOOST_CONTAINER_IMPDEF
      (typename real_allocator<T BOOST_MOVE_I A>::type)                          allocator_type;
   typedef allocator_type                                                        stored_allocator_type;
   typedef typename    allocator_traits<allocator_type>::pointer                 pointer;
   typedef typename    allocator_traits<allocator_type>::const_pointer           const_pointer;
   typedef typename    allocator_traits<allocator_type>::reference               reference;
   typedef typename    allocator_traits<allocator_type>::const_reference         const_reference;
   typedef typename    allocator_traits<allocator_type>::size_type               size_type;
   typedef typename    allocator_traits<allocator_type>::difference_type         difference_type;
   typedef pointer                                                               iterator;
   typedef const_pointer                                                         const_iterator;
   typedef BOOST_CONTAINER_IMPDEF
      (boost::container::reverse_iterator<iterator>)                             reverse_iterator;
   typedef BOOST_CONTAINER_IMPDEF
      (boost::container::reverse_iterator<const_iterator>)                       const_reverse_iterator;

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   private:

   //`allocator_type::value_type` must match container's `value type`. If this
   //assertion fails, please review your allocator definition. 
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<value_type, typename allocator_traits<allocator_type>::value_type>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<base_t, dtl::devector_base<pointer, size_type, stored_size_type> >::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename base_t::const_pointer, const_pointer>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename base_t::reference, reference>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename base_t::const_reference, const_reference>::value));

   BOOST_COPYABLE_AND_MOVABLE(devector)

   //Every member of the base that devector uses is declared here, so that a
   //member of the allocator with the same name can not make the name ambiguous
   using base_t::m_buffer;
   using base_t::m_front_idx;
   using base_t::m_back_idx;
   using base_t::m_capacity;
   #ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS
   using base_t::m_capacity_alloc_count;
   #endif
   using base_t::set_front_idx;
   using base_t::set_back_idx;
   using base_t::set_capacity;

   // Guard to deallocate buffer on exception
   typedef typename detail::allocation_guard<allocator_type> allocation_guard;

   // Random access pseudo iterator always yielding to the same result
   typedef constant_iterator<T> cvalue_iterator;

   //The allocator limit, clamped to the biggest value of stored_size_type
   static size_type max_size_for(const allocator_type &a) BOOST_NOEXCEPT
   {
      return dtl::limit_by_stored_size_type<size_type, stored_size_type>::clamp
         (allocator_traits_type::max_size(a));
   }

   //Internal capacity that leaves the minimum free fraction after desired_capacity
   //elements. If that is bigger than max, returns max: a buffer of max_size()
   //elements can not be reallocated, so all of it can be used.
   static size_type to_internal_capacity(size_type desired_capacity, size_type max)
   {
      if (BOOST_UNLIKELY(desired_capacity > max)) {
         boost::container::throw_length_error("devector: requested capacity greater than max_size()");
      }

      BOOST_IF_CONSTEXPR (devector_never_relocates) {
         return desired_capacity;
      }
      else {
         const size_type divisor = devector_min_free_fraction - 1u;
         //ceil(desired_capacity/divisor), without overflow
         const size_type q = size_type(desired_capacity / divisor + size_type(desired_capacity % divisor != 0u));
         if (q > max / devector_min_free_fraction) {
            return max;
         }
         size_type const nc = size_type(q * devector_min_free_fraction);
         BOOST_ASSERT(desired_capacity <= (nc - nc / devector_min_free_fraction));
         return nc;
      }
   }

   #endif // ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   // Standard Interface
   public:
   // construct/copy/destroy

   /**
   * **Effects**: Constructs an empty devector.
   *
   * **Postcondition**: `empty() && front_free_capacity() == 0
   * && back_free_capacity() == 0`.
   *
   * **Throws**: If allocator_type's default constructor throws.
   *
   * **Complexity**: Constant.
   */
   devector() BOOST_NOEXCEPT_IF(dtl::is_nothrow_default_constructible<allocator_type>::value)
      : alloc_holder_t()
   {}

   /**
   * **Effects**: Constructs an empty devector, using the specified allocator.
   *
   * **Postcondition**: `empty() && front_free_capacity() == 0
   * && back_free_capacity() == 0`.
   *
   * **Complexity**: Constant.
   */
   explicit devector(const allocator_type& a) BOOST_NOEXCEPT
      : alloc_holder_t(a)
   {}

   /**
   * **Effects**: Constructs an empty devector, using the specified allocator
   * and reserves `n` slots as if `reserve(n)` was called.
   *
   * **Postcondition**: `empty() && capacity() >= n`.
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Constant.
   */
   devector(size_type n, reserve_only_tag_t, const allocator_type& a = allocator_type())
      : alloc_holder_t(a)
   {
      const size_type c = to_internal_capacity(n, max_size_for(a));
      this->priv_reserve(c, size_type(c/2u), size_type(c/2u));
   }

   /**
   * **Effects**: Constructs an empty devector, using the specified allocator
   * and reserves at least `front_free_cap + back_free_cap` slots as if `reserve_front(front_cap)` and
   * `reserve_back(back_cap)` was called.
   *
   * **Postcondition**: `empty() && front_free_capacity() >= front_free_cap
   * && back_free_capacity() >= back_free_cap`.
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Constant.
   */
   devector(size_type front_free_cap, size_type back_free_cap, reserve_only_tag_t, const allocator_type& a = allocator_type())
      : alloc_holder_t(a)
   {
      this->priv_reserve(checked_capacity_sum(front_free_cap, back_free_cap), front_free_cap, front_free_cap);
   }

   /**
   * [DefaultInsertable]: http://en.cppreference.com/w/cpp/concept/DefaultInsertable
   *
   * **Effects**: Constructs a devector with `n` value_initialized elements using the specified allocator.
   *
   * **Requires**: `T` shall be [DefaultInsertable] into `*this`.
   *
   * **Postcondition**: `size() == n`.
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Linear in `n`.
   */
   explicit devector(size_type n, const allocator_type& a = allocator_type())
      : alloc_holder_t(a)
   {
      this->priv_reserve(n, 0u, n);
      allocator_type &al = static_cast<allocator_type&>(*this);
      allocation_guard buffer_guard(this->m_buffer, this->m_capacity, al);
      boost::container::uninitialized_value_init_alloc_n(al, n, this->priv_raw_begin());
      buffer_guard.release();
      BOOST_ASSERT(invariants_ok());
   }

   /**
   * **Effects**: Constructs a devector with `n` default-initialized elements using the specified allocator.
   *
   * **Requires**: `T` shall be [DefaultInsertable] into `*this`.
   *
   * **Postcondition**: `size() == n`.
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Linear in `n`.
   */
   explicit devector(size_type n, default_init_t, const allocator_type& a = allocator_type())
      : alloc_holder_t(a)
   {
      this->priv_reserve(n, 0u, n);
      allocator_type &al = static_cast<allocator_type&>(*this);
      allocation_guard buffer_guard(this->m_buffer, this->m_capacity, al);
      boost::container::uninitialized_default_init_alloc_n(al, n, this->priv_raw_begin());
      buffer_guard.release();
      BOOST_ASSERT(invariants_ok());
   }

   /**
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   *
   * **Effects**: Constructs a devector with `n` copies of `value`, using the specified allocator.
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this`.
   *
   * **Postcondition**: `size() == n`.
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Linear in `n`.
   */
   devector(size_type n, const T& value, const allocator_type& a = allocator_type())
      : alloc_holder_t(a)
   {
      this->priv_reserve(n, 0u, n);
      construct_from_range(cvalue_iterator(value, n), cvalue_iterator());
      BOOST_ASSERT(invariants_ok());
   }

   /**
   * **Effects**: Constructs a devector equal to the range `[first,last)`, using the specified allocator.
   *
   * **Requires**: `T` shall be [EmplaceConstructible] into `*this` from `*first`. If the specified
   * iterator does not meet the forward iterator requirements, `T` shall also be [MoveInsertable]
   * into `*this`.
   *
   * **Postcondition**: `size() == boost::container::iterator_distance(first, last)`
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Makes only `N` calls to the copy constructor of `T` (where `N` is the distance between `first`
   * and `last`), at most one allocation and no reallocations if iterators first and last are of forward,
   * bidirectional, or random access categories. It makes `O(N)` calls to the copy constructor of `T`
   * and `O(log(N))` reallocations if they are just input iterators.
   *
   * **Remarks**: Each iterator in the range `[first,last)` shall be dereferenced exactly once,
   * unless an exception is thrown.
   *
   * [EmplaceConstructible]: http://en.cppreference.com/w/cpp/concept/EmplaceConstructible
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   */
   template <class InputIterator>
   devector(InputIterator first, InputIterator last, const allocator_type& a = allocator_type()
      //Input iterators
      BOOST_CONTAINER_DOCIGN(BOOST_MOVE_I typename dtl::disable_if_or
            < void
            BOOST_MOVE_I dtl::is_convertible<InputIterator BOOST_MOVE_I size_type>
            BOOST_MOVE_I dtl::is_not_input_iterator<InputIterator>
            >::type * = 0)
      )
      : alloc_holder_t(a)
   {
      BOOST_CONTAINER_TRY{
         while (first != last) {
            this->emplace_back(*first++);
         }
         BOOST_ASSERT(invariants_ok());
      }
      BOOST_CONTAINER_CATCH(...){
         this->destroy_elements(this->m_buffer + this->m_front_idx, this->m_buffer + this->m_back_idx);
         this->deallocate_buffer();
         BOOST_CONTAINER_RETHROW
      }
      BOOST_CONTAINER_CATCH_END
   }

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   template <class ForwardIterator>
   devector(ForwardIterator first, ForwardIterator last, const allocator_type& a = allocator_type()
      //Other iterators
      BOOST_CONTAINER_DOCIGN(BOOST_MOVE_I typename dtl::disable_if_or
            < void
            BOOST_MOVE_I dtl::is_convertible<ForwardIterator BOOST_MOVE_I size_type>
            BOOST_MOVE_I dtl::is_input_iterator<ForwardIterator>
            >::type * = 0)
      )
      : alloc_holder_t(a)
   {
      const size_type n = boost::container::iterator_udistance(first, last);
      this->priv_reserve(n, 0u, n);
      this->construct_from_range(first, last);
      BOOST_ASSERT(invariants_ok());
   }

   #endif // ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   /**
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   *
   * **Effects**: Copy constructs a devector.
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this`.
   *
   * **Postcondition**: `this->size() == x.size()`.
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Linear in the size of `x`.
   */
   devector(const devector& x)
      : alloc_holder_t(allocator_traits_type::select_on_container_copy_construction(static_cast<const allocator_type&>(x)))
   {
      const size_type n = x.size();
      this->priv_reserve(n, 0u, n);
      this->construct_from_range(x.begin(), x.end());
      BOOST_ASSERT(invariants_ok());
   }

   /**
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   *
   * **Effects**: Copy constructs a devector, using the specified allocator.
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this`.
   *
   * **Postcondition**: `*this == x`.
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Linear in the size of `x`.
   */
   devector(const devector& x, const BOOST_CONTAINER_DOC1ST(allocator_type, typename dtl::type_identity<allocator_type>::type)& a)
      : alloc_holder_t(a)
   {
      const size_type n = x.size();
      this->priv_reserve(n, 0u, n);
      this->construct_from_range(x.begin(), x.end());
      BOOST_ASSERT(invariants_ok());
   }

   /**
   * **Effects**: Moves `rhs`'s resources to `*this`.
   *
   * **Throws**: Nothing.
   *
   * **Postcondition**: *this has the same value `rhs` had before the operation.
   *                    `rhs` is left in an unspecified but valid state.
   *
   * **Exceptions**: Strong exception guarantee if not `noexcept`.
   *
   * **Complexity**: Constant.
   */
   devector(BOOST_RV_REF(devector) rhs) BOOST_NOEXCEPT_OR_NOTHROW
      : base_t(rhs), alloc_holder_t(::boost::move(static_cast<allocator_type&>(rhs)))
   {
      // buffer is already acquired, reset rhs
      #ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS
      this->m_capacity_alloc_count = 0;
      #endif
      rhs.m_buffer = pointer();
      rhs.set_front_idx(0u);
      rhs.set_back_idx(0u);
      rhs.set_capacity(0u);
      BOOST_ASSERT(      invariants_ok());
      BOOST_ASSERT(rhs.invariants_ok());
   }

   /**
   * **Effects**: Moves `rhs`'s resources to `*this`, using the specified allocator.
   *
   * **Throws**: Nothing if `allocator_traits_type::is_always_equal::value` is true.
   *   Otherwise, if allocation or T's move constructor throws.
   *
   * **Postcondition**: *this has the same value `rhs` had before the operation.
   *                    `rhs` is left in an unspecified but valid state.
   *
   * **Exceptions**: Strong exception guarantee if not `noexcept`.
   *
   * **Complexity**: Linear if allocator != rhs.get_allocator(), otherwise constant.
   */
   devector(BOOST_RV_REF(devector) rhs, const BOOST_CONTAINER_DOC1ST(allocator_type, typename dtl::type_identity<allocator_type>::type)& a)
      BOOST_NOEXCEPT_IF(allocator_traits_type::is_always_equal::value)
      : alloc_holder_t(a)
   {
      if (allocator_traits_type::is_always_equal::value || static_cast<allocator_type&>(*this) == static_cast<allocator_type&>(rhs)) {
         //Equal allocators: take the buffer of rhs
         this->m_buffer = rhs.m_buffer;
         this->set_front_idx(rhs.m_front_idx);
         this->set_back_idx(rhs.m_back_idx);
         this->set_capacity(rhs.m_capacity);
         rhs.m_capacity = 0u;
         rhs.m_buffer = pointer();
         rhs.m_front_idx = 0;
         rhs.m_back_idx = 0;
      }
      else if (const size_type n = rhs.size()) {
         //Unequal allocators, we can take rhs's so we need to allocate
         this->m_buffer = this->allocate(n);
         this->set_capacity(n);
         this->set_back_idx(n);
         this->construct_from_range(boost::make_move_iterator(rhs.begin()), boost::make_move_iterator(rhs.end()));
      }
      BOOST_ASSERT(      invariants_ok());
      BOOST_ASSERT(rhs.invariants_ok());
   }

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   /**
   * **Equivalent to**: `devector(il.begin(), il.end(), allocator)`.
   */
   devector(const std::initializer_list<T>& il, const allocator_type& a = allocator_type())
      : alloc_holder_t(a)
   {
      const size_type n = il.size();
      this->priv_reserve(n, 0u, n);
      this->construct_from_range(il.begin(), il.end());
      BOOST_ASSERT(invariants_ok());
   }
   #endif

/**
   * **Effects**: Destroys the devector. All stored values are destroyed and
   * used memory, if any, deallocated.
   *
   * **Complexity**: Linear in the size of `*this`.
   */
~devector() BOOST_NOEXCEPT
{
   this->destroy_elements(this->m_buffer + this->m_front_idx, this->m_buffer + this->m_back_idx);
   this->deallocate_buffer();
}

/**
   * **Effects**: Copies elements of `x` to `*this`. Previously
   * held elements get copy assigned to or destroyed.
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this`.
   *
   * **Postcondition**: `this->size() == x.size()`, the elements of
   * `*this` are copies of elements in `x` in the same order.
   *
   * **Returns**: `*this`.
   *
   * **Exceptions**: Strong exception guarantee if `T` is `NothrowConstructible`
   * and the allocator is allowed to be propagated
   * ([propagate_on_container_copy_assignment] is true),
   * Basic exception guarantee otherwise.
   *
   * **Complexity**: Linear in the size of `x` and `*this`.
   *
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   * [propagate_on_container_copy_assignment]: http://en.cppreference.com/w/cpp/memory/allocator_traits
   */

   inline devector& operator=(BOOST_COPY_ASSIGN_REF(devector) rhs)
   {
      const devector &x = rhs;
      if (this == &x) { return *this; } // skip self

      const bool do_propagate = allocator_traits_type::propagate_on_container_copy_assignment::value;
      BOOST_IF_CONSTEXPR(do_propagate)
      {
         allocator_type &this_alloc = static_cast<allocator_type&>(*this);
         const allocator_type &other_alloc = static_cast<const allocator_type&>(x);
         if (this_alloc != other_alloc)
         {
            // new allocator cannot free existing storage
            this->clear();
            this->deallocate_buffer();
            //Leave a valid empty devector in case the copy below throws
            this->m_capacity = 0u;
            this->m_buffer = pointer();
            this->m_front_idx = 0u;
            this->set_back_idx(0u);
         }
         dtl::bool_<do_propagate> flag;
         dtl::assign_alloc(this_alloc, other_alloc, flag);
      }

      size_type n = x.size();
      if (this->m_capacity >= n) {
         this->overwrite_buffer(x.begin(), x.end());
      }
      else {
         this->allocate_and_copy_range(x.begin(), x.end());
      }

      BOOST_ASSERT(invariants_ok());

      return *this;
   }

   /**
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   *
   * **Effects**: Moves elements of `x` to `*this`. Previously
   * held elements get move/copy assigned to or destroyed.
   *
   * **Requires**: `T` shall be [MoveInsertable] into `*this`.
   *
   * **Postcondition**: `x` is left in an unspecified but valid state.
   *
   * **Returns**: `*this`.
   *
   * **Exceptions**: Basic exception guarantee if not `noexcept`.
   *
   * **Complexity**: Constant if allocator_traits_type::
   *   propagate_on_container_move_assignment is true or
   *   this->get>allocator() == x.get_allocator(). Linear otherwise.
   */
   devector& operator=(BOOST_RV_REF(devector) x)
      BOOST_NOEXCEPT_IF(allocator_traits_type::propagate_on_container_move_assignment::value
                                 || allocator_traits_type::is_always_equal::value)
   {
      if (BOOST_LIKELY(this != &x)) {
         //We know resources can be transferred at comiple time if both allocators are
         //always equal or the allocator is going to be propagated
         const bool can_steal_resources_alloc
            = allocator_traits_type::propagate_on_container_move_assignment::value
            || allocator_traits_type::is_always_equal::value;
         dtl::bool_<can_steal_resources_alloc> flag;
         this->priv_move_assign(boost::move(x), flag);
      }
      BOOST_ASSERT(invariants_ok());
      return *this;
   }

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   /**
   * **Effects**: Copies elements of `il` to `*this`. Previously
   * held elements get copy assigned to or destroyed.
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this` and [CopyAssignable].
   *
   * **Postcondition**: `this->size() == il.size()`, the elements of
   * `*this` are copies of elements in `il` in the same order.
   *
   * **Exceptions**: Strong exception guarantee if `T` is nothrow copy assignable
   * from `T` and `NothrowConstructible`, Basic exception guarantee otherwise.
   *
   * **Returns**: `*this`.
   *
   * **Complexity**: Linear in the size of `il` and `*this`.
   *
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   * [CopyAssignable]: http://en.cppreference.com/w/cpp/concept/CopyAssignable
   */
   inline devector& operator=(std::initializer_list<T> il)
   {
      this->assign(il.begin(), il.end());
      return *this;
   }
   #endif

   /**
   * **Effects**: Replaces elements of `*this` with a copy of `[first,last)`.
   * Previously held elements get copy assigned to or destroyed.
   *
   * **Requires**: `T` shall be [EmplaceConstructible] from `*first`. If the specified iterator
   * does not meet the forward iterator requirements, `T` shall be also [MoveInsertable] into `*this`.
   *
   * **Precondition**: `first` and `last` are not iterators into `*this`.
   *
   * **Postcondition**: `size() == N`, where `N` is the distance between `first` and `last`.
   *
   * **Exceptions**: Strong exception guarantee if `T` is nothrow copy assignable
   * from `*first` and `NothrowConstructible`, Basic exception guarantee otherwise.
   *
   * **Complexity**: Linear in the distance between `first` and `last`.
   * Makes a single reallocation at most if the iterators `first` and `last`
   * are of forward, bidirectional, or random access categories. It makes
   * `O(log(N))` reallocations if they are just input iterators.
   *
   * **Remarks**: Each iterator in the range `[first,last)` shall be dereferenced exactly once,
   * unless an exception is thrown.
   *
   * [EmplaceConstructible]: http://en.cppreference.com/w/cpp/concept/EmplaceConstructible
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   */
   template <class InputIterator>
   void assign(InputIterator first, InputIterator last
      //Input iterators
      BOOST_CONTAINER_DOCIGN(BOOST_MOVE_I typename dtl::disable_if_or
            < void
            BOOST_MOVE_I dtl::is_convertible<InputIterator BOOST_MOVE_I size_type>
            BOOST_MOVE_I dtl::is_not_input_iterator<InputIterator>
            >::type * = 0)
      )
   {
      first = overwrite_buffer_impl(first, last, dtl::false_());
      while (first != last)
      {
         this->emplace_back(*first++);
      }
   }

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   template <class ForwardIterator>
   void assign(ForwardIterator first, ForwardIterator last
      //Other iterators
      BOOST_CONTAINER_DOCIGN(BOOST_MOVE_I typename dtl::disable_if_or
            < void
            BOOST_MOVE_I dtl::is_convertible<ForwardIterator BOOST_MOVE_I size_type>
            BOOST_MOVE_I dtl::is_input_iterator<ForwardIterator>
            >::type * = 0)
      )
   {
      const size_type n = boost::container::iterator_udistance(first, last);

      if (this->m_capacity >= n) {
         this->overwrite_buffer(first, last);
      }
      else {
         this->allocate_and_copy_range(first, last);
      }

      BOOST_ASSERT(invariants_ok());
   }

   #endif // ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   /**
    * **Effects**: Replaces elements of `*this` with `n` copies of `u`.
    * Previously held elements get copy assigned to or destroyed.
    *
    * **Requires**: `T` shall be [CopyInsertable] into `*this` and
    * [CopyAssignable].
    *
    * **Precondition**: `u` is not a reference into `*this`.
    *
    * **Postcondition**: `size() == n` and the elements of
    * `*this` are copies of `u`.
    *
    * **Exceptions**: Strong exception guarantee if `T` is nothrow copy assignable
    * from `u` and `NothrowConstructible`, Basic exception guarantee otherwise.
    *
    * **Complexity**: Linear in `n` and the size of `*this`.
    *
    * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
    * [CopyAssignable]: http://en.cppreference.com/w/cpp/concept/CopyAssignable
    */
   inline void assign(size_type n, const T& u)
   {
      BOOST_ASSERT(!this->priv_is_element_address(dtl::addressof(u)));
      cvalue_iterator first(u, n);
      cvalue_iterator last;
      this->assign(first, last);
   }

    #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
    /** **Equivalent to**: `assign(il.begin(), il.end())`. */
   inline void assign(std::initializer_list<T> il)
    {
         this->assign(il.begin(), il.end());
    }
    #endif

   /**
    * **Returns**: A copy of the allocator associated with the container.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD inline
    allocator_type get_allocator() const BOOST_NOEXCEPT
   {
      return static_cast<const allocator_type&>(*this);
   }

   BOOST_CONTAINER_NODISCARD inline
    const allocator_type &get_stored_allocator() const BOOST_NOEXCEPT
   {
      return static_cast<const allocator_type&>(*this);
   }

   BOOST_CONTAINER_NODISCARD inline
         allocator_type &get_stored_allocator() BOOST_NOEXCEPT
   {
      return static_cast<allocator_type&>(*this);
   }

   // iterators

   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   /**
    * **Returns**: A iterator pointing to the first element in the devector,
    * or the past the end iterator if the devector is empty.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD iterator begin() BOOST_NOEXCEPT;

   /**
    * **Returns**: A constant iterator pointing to the first element in the devector,
    * or the past the end iterator if the devector is empty.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD const_iterator begin() const BOOST_NOEXCEPT;

   /**
    * **Returns**: An iterator pointing past the last element of the container.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD iterator end() BOOST_NOEXCEPT;

   /**
    * **Returns**: A constant iterator pointing past the last element of the container.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD const_iterator end() const BOOST_NOEXCEPT;

   /**
    * **Returns**: A reverse iterator pointing to the first element in the reversed devector,
    * or the reverse past the end iterator if the devector is empty.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD reverse_iterator rbegin() BOOST_NOEXCEPT;

   /**
    * **Returns**: A constant reverse iterator
    * pointing to the first element in the reversed devector,
    * or the reverse past the end iterator if the devector is empty.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD const_reverse_iterator rbegin() const BOOST_NOEXCEPT;

   /**
    * **Returns**: A reverse iterator pointing past the last element in the
    * reversed container, or to the beginning of the reversed container if it's empty.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD reverse_iterator rend() BOOST_NOEXCEPT;

   /**
    * **Returns**: A constant reverse iterator pointing past the last element in the
    * reversed container, or to the beginning of the reversed container if it's empty.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD const_reverse_iterator rend() const BOOST_NOEXCEPT;

   /**
    * **Returns**: A constant iterator pointing to the first element in the devector,
    * or the past the end iterator if the devector is empty.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD const_iterator cbegin() const BOOST_NOEXCEPT;

   /**
    * **Returns**: A constant iterator pointing past the last element of the container.
    *
    * **Complexity**: Constant.
    */
   const_iterator cend() const BOOST_NOEXCEPT;

   /**
    * **Returns**: A constant reverse iterator
    * pointing to the first element in the reversed devector,
    * or the reverse past the end iterator if the devector is empty.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD const_reverse_iterator crbegin() const BOOST_NOEXCEPT;

   /**
    * **Returns**: A constant reverse iterator pointing past the last element in the
    * reversed container, or to the beginning of the reversed container if it's empty.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD const_reverse_iterator crend() const BOOST_NOEXCEPT;
   #else
   using base_t::begin;
   using base_t::end;
   using base_t::rbegin;
   using base_t::rend;
   using base_t::cbegin;
   using base_t::cend;
   using base_t::crbegin;
   using base_t::crend;
   #endif

   // capacity

   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   /**
    * **Returns**: True, if `size() == 0`, false otherwise.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD bool empty() const BOOST_NOEXCEPT;

   /**
    * **Returns**: The number of elements the devector contains.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD size_type size() const BOOST_NOEXCEPT;
   #else
   using base_t::empty;
   using base_t::size;
   #endif

   /**
    * **Returns**: The maximum number of elements the devector could possibly hold.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD inline
    size_type max_size() const BOOST_NOEXCEPT
   {
      return max_size_for(static_cast<const allocator_type&>(*this));
   }

   /**
   * **Returns**: The *minimum* number of elements that can be inserted into devector using
   *   position-based insertions without requiring a reallocation. Note that, unlike in
   *   typical sequence containers like `vector`, `capacity()` can be smaller than `size()`.
   *   This can happen if a user inserts elements in a particular way (usually inserting at
   *   front up to front_free_capacity() and at back up to back_free_capacity()).
   *
   *   With the `relocate_never` option, `capacity()` is the size of the buffer, and it does not
   *   tell how many elements can be inserted without a reallocation. The elements are never moved
   *   inside the buffer, so an insertion avoids a reallocation only if there is enough free space at the
   *   end where the space for the new elements is made: the back for insertions at `end()`,
   *   the front for insertions at `begin()` and the nearer end for other positions.
   *   See front_free_capacity() and back_free_capacity().
   *
   * **Complexity**: Constant.
   */
   BOOST_CONTAINER_NODISCARD inline
   size_type capacity() const BOOST_NOEXCEPT
   {
      //A buffer of max_size() elements can not be reallocated, so all of it can be used
      if (BOOST_UNLIKELY(this->m_capacity >= this->max_size()))
         return this->m_capacity;
      size_type const cap_reserve = devector_never_relocates ? 0u : this->m_capacity/devector_min_free_fraction;
      return this->m_capacity > cap_reserve ? (this->m_capacity - cap_reserve) : 0u;
   }

   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   /**
    * **Returns**: The total number of elements that can be pushed to the front of the
    * devector without requiring reallocation.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD size_type front_free_capacity() const BOOST_NOEXCEPT;

   /**
    * **Returns**: The total number of elements that can be pushed to the back of the
    * devector without requiring reallocation.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD size_type back_free_capacity() const BOOST_NOEXCEPT;
   #else
   using base_t::front_free_capacity;
   using base_t::back_free_capacity;
   #endif

   /**
   * **Effects**: Same as resize_back(sz): if `sz` is greater than the size of `*this`,
   * additional value-initialized elements are inserted at the back. Invalidates iterators
   * if reallocation is needed. If `sz` is smaller than than the size of `*this`,
   * elements are erased from the back.
   *
   * **Requires**: T shall be [MoveInsertable] into *this and [DefaultConstructible].
   *
   * **Postcondition**: `sz == size()`.
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Linear in the size of `*this` and `sz`.
   *
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   * [DefaultConstructible]: http://en.cppreference.com/w/cpp/concept/DefaultConstructible
   */
   inline void resize(size_type sz)
   {
      this->resize_back(sz);
   }

   /**
    * **Effects**: Same as resize(sz) but creates default-initialized
    * elements instead of value-initialized elements.
    */
   inline void resize(size_type sz, default_init_t)
   {
      this->resize_back(sz, default_init);
   }

   /**
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   *
   * **Effects**: Same as resize_back(sz, c): if `sz` is greater than the size of `*this`,
   * copies of `c` are inserted at the back.
   * If `sz` is smaller than than the size of `*this`,
   * elements are popped from the back.
   *
   * **Postcondition**: `sz == size()`.
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this`.
   *
   * **Exceptions**: Strong exception guarantee.
   *
   * **Complexity**: Linear in the size of `*this` and `sz`.
   */
   inline void resize(size_type sz, const T& c)
   {
      this->resize_back(sz, c);
   }

   /**
    * **Effects**: If `sz` is greater than the size of `*this`,
    * additional value-initialized elements are inserted
    * to the front. Invalidates iterators if reallocation is needed.
    * If `sz` is smaller than than the size of `*this`,
    * elements are popped from the front.
    *
    * **Requires**: T shall be [MoveInsertable] into *this and [DefaultConstructible].
    *
    * **Postcondition**: `sz == size()`.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Complexity**: Linear in the size of `*this` and `sz`.
    *
    * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
    * [DefaultConstructible]: http://en.cppreference.com/w/cpp/concept/DefaultConstructible
    */
   inline void resize_front(size_type sz)
   {
      resize_front_impl(sz, dtl::insert_value_initialized_n_proxy<allocator_type>());
      BOOST_ASSERT(invariants_ok());
   }

   /**
    * **Effects**: If `sz` is greater than the size of `*this`,
    * additional value-initialized elements are inserted
    * to the front. Invalidates iterators if reallocation is needed.
    * If `sz` is smaller than than the size of `*this`,
    * elements are popped from the front.
    *
    * **Requires**: T shall be [MoveInsertable] into *this and default_initializable.
    *
    * **Postcondition**: `sz == size()`.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Complexity**: Linear in the size of `*this` and `sz`.
    *
    * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
    */
   inline void resize_front(size_type sz, default_init_t)
   {
      resize_front_impl(sz, dtl::insert_default_initialized_n_proxy<allocator_type>());
      BOOST_ASSERT(invariants_ok());
   }

   /**
    * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
    *
    * **Effects**: If `sz` is greater than the size of `*this`,
    * copies of `c` are inserted to the front.
    * Invalidates iterators if reallocation is needed.
    * If `sz` is smaller than than the size of `*this`,
    * elements are popped from the front.
    *
    * **Postcondition**: `sz == size()`.
    *
    * **Requires**: `T` shall be [CopyInsertable] into `*this`.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Complexity**: Linear in the size of `*this` and `sz`.
    */
   inline void resize_front(size_type sz, const T& c)
   {
      resize_front_impl(sz, dtl::insert_n_copies_proxy<allocator_type>(c));
      BOOST_ASSERT(invariants_ok());
   }

   /**
    * **Effects**: If `sz` is greater than the size of `*this`,
    * additional value-initialized elements are inserted
    * to the back. Invalidates iterators if reallocation is needed.
    * If `sz` is smaller than than the size of `*this`,
    * elements are popped from the back.
    *
    * **Requires**: T shall be [MoveInsertable] into *this and [DefaultConstructible].
    *
    * **Postcondition**: `sz == size()`.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Complexity**: Linear in the size of `*this` and `sz`.
    *
    * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
    * [DefaultConstructible]: http://en.cppreference.com/w/cpp/concept/DefaultConstructible
    */
   inline void resize_back(size_type sz)
   {
      resize_back_impl(sz, dtl::insert_value_initialized_n_proxy<allocator_type>());
      BOOST_ASSERT(invariants_ok());
   }

   /**
    * **Effects**: If `sz` is greater than the size of `*this`,
    * additional value-initialized elements are inserted
    * to the back. Invalidates iterators if reallocation is needed.
    * If `sz` is smaller than than the size of `*this`,
    * elements are popped from the back.
    *
    * **Requires**: T shall be [MoveInsertable] into *this and default initializable.
    *
    * **Postcondition**: `sz == size()`.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Complexity**: Linear in the size of `*this` and `sz`.
    *
    * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
    */
   inline void resize_back(size_type sz, default_init_t)
   {
      resize_back_impl(sz, dtl::insert_default_initialized_n_proxy<allocator_type>());
      BOOST_ASSERT(invariants_ok());
   }

   /**
    * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
    *
    * **Effects**: If `sz` is greater than the size of `*this`,
    * copies of `c` are inserted to the back.
    * If `sz` is smaller than than the size of `*this`,
    * elements are popped from the back.
    *
    * **Postcondition**: `sz == size()`.
    *
    * **Requires**: `T` shall be [CopyInsertable] into `*this`.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Complexity**: Linear in the size of `*this` and `sz`.
    */
   inline void resize_back(size_type sz, const T& c)
   {
      resize_back_impl(sz, dtl::insert_n_copies_proxy<allocator_type>(c));
      BOOST_ASSERT(invariants_ok());
   }

   /**
    * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
    *
    * **Effects**: Ensures that at least `n` elements can be inserted
    * without requiring reallocation, where `n` is `new_capacity - size()`,
    * if `n` is positive. Otherwise, there are no effects.
    * Invalidates iterators if reallocation is needed.
    *
    * **Note**: With the `relocate_never` option, the elements are never moved inside the buffer
    * and the free space is divided between the front and the back. Then only front_free_capacity()
    * elements can be inserted at the front and back_free_capacity() elements at the back without a
    * reallocation. Use reserve_front() or reserve_back() to get free space at one end.
    *
    * **Requires**: `T` shall be [MoveInsertable] into `*this`.
    *
    * **Complexity**: Linear in the size of *this.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Throws**: length_error if `new_capacity > max_size()`.
    */
   inline void reserve(size_type new_capacity)
   {
      if (this->capacity() < new_capacity) {
         if (new_capacity > this->max_size()) {
            boost::container::throw_length_error("devector::reserve max_size() exceeded");
         }
         size_type const nc = to_internal_capacity(new_capacity, this->max_size());
         size_type const sz = this->size();
         reallocate_at(nc, (nc-sz)/2u);
      }
      BOOST_ASSERT(invariants_ok());
   }

   /**
    * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
    *
    * **Effects**: Ensures that `n` elements can be pushed to the front
    * without requiring reallocation, where `n` is `new_capacity - size()`,
    * if `n` is positive. Otherwise, there are no effects.
    * Invalidates iterators if reallocation is needed.
    *
    * **Requires**: `T` shall be [MoveInsertable] into `*this`.
    *
    * **Complexity**: Linear in the size of *this.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Throws**: `length_error` if `new_capacity > max_size()`.
    */
   inline void reserve_front(size_type new_capacity)
   {
      if (front_capacity() >= new_capacity) { return; }

      if (new_capacity > this->max_size() - back_free_capacity()) {
         boost::container::throw_length_error("devector::reserve_front max_size() exceeded");
      }
      reallocate_at(new_capacity + back_free_capacity(), new_capacity - size());

      BOOST_ASSERT(invariants_ok());
   }

   /**
    * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
    *
    * **Effects**: Ensures that `n` elements can be pushed to the back
    * without requiring reallocation, where `n` is `new_capacity - size()`,
    * if `n` is positive. Otherwise, there are no effects.
    * Invalidates iterators if reallocation is needed.
    *
    * **Requires**: `T` shall be [MoveInsertable] into `*this`.
    *
    * **Complexity**: Linear in the size of *this.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Throws**: length_error if `new_capacity > max_size()`.
    */
   inline void reserve_back(size_type new_capacity)
   {
      if (back_capacity() >= new_capacity) { return; }

      if (new_capacity > this->max_size() - front_free_capacity()) {
         boost::container::throw_length_error("devector::reserve_back max_size() exceeded");
      }
      reallocate_at(new_capacity + front_free_capacity(), this->m_front_idx);

      BOOST_ASSERT(invariants_ok());
   }

   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   size_type front_capacity() const;

   size_type back_capacity() const;
   #else
   using base_t::front_capacity;
   using base_t::back_capacity;
   #endif

   /**
    * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
    *
    * **Effects**: Reduces `capacity()` to `size()`. Invalidates iterators.
    *
    * **Requires**: `T` shall be [MoveInsertable] into `*this`.
    *
    * **Exceptions**: Strong exception guarantee.
    *
    * **Complexity**: Linear in the size of *this.
    */
   inline void shrink_to_fit()
   {
      if(this->front_capacity() || this->back_capacity())
            this->reallocate_at(size(), 0);
   }

   // element access:

   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   /**
    * **Returns**: A reference to the `n`th element in the devector.
    *
    * **Precondition**: `n < size()`.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD reference operator[](size_type n) BOOST_NOEXCEPT;

   /**
    * **Returns**: A constant reference to the `n`th element in the devector.
    *
    * **Precondition**: `n < size()`.
    *
    * **Complexity**: Constant.
    */
   BOOST_CONTAINER_NODISCARD const_reference operator[](size_type n) const BOOST_NOEXCEPT;

   //! <b>Requires</b>: size() >= n.
   //!
   //! <b>Effects</b>: Returns an iterator to the nth element
   //!   from the beginning of the container. Returns end()
   //!   if n == size().
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   //!
   //! <b>Note</b>: Non-standard extension
   BOOST_CONTAINER_NODISCARD iterator nth(size_type n) BOOST_NOEXCEPT_OR_NOTHROW;

   //! <b>Requires</b>: size() >= n.
   //!
   //! <b>Effects</b>: Returns a const_iterator to the nth element
   //!   from the beginning of the container. Returns end()
   //!   if n == size().
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   //!
   //! <b>Note</b>: Non-standard extension
   BOOST_CONTAINER_NODISCARD const_iterator nth(size_type n) const BOOST_NOEXCEPT_OR_NOTHROW;

   //! <b>Requires</b>: begin() <= p <= end().
   //!
   //! <b>Effects</b>: Returns the index of the element pointed by p
   //!   and size() if p == end().
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   //!
   //! <b>Note</b>: Non-standard extension
   BOOST_CONTAINER_NODISCARD size_type index_of(iterator p) BOOST_NOEXCEPT_OR_NOTHROW;

   //! <b>Requires</b>: begin() <= p <= end().
   //!
   //! <b>Effects</b>: Returns the index of the element pointed by p
   //!   and size() if p == end().
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   //!
   //! <b>Note</b>: Non-standard extension
   BOOST_CONTAINER_NODISCARD size_type index_of(const_iterator p) const BOOST_NOEXCEPT_OR_NOTHROW;

   /**
   * **Returns**: A reference to the `n`th element in the devector.
   *
   * **Throws**: `out_of_range`, if `n >= size()`.
   *
   * **Complexity**: Constant.
   */
   BOOST_CONTAINER_NODISCARD reference at(size_type n);

   /**
   * **Returns**: A constant reference to the `n`th element in the devector.
   *
   * **Throws**: `out_of_range`, if `n >= size()`.
   *
   * **Complexity**: Constant.
   */
   BOOST_CONTAINER_NODISCARD const_reference at(size_type n) const;

   /**
   * **Returns**: A reference to the first element in the devector.
   *
   * **Precondition**: `!empty()`.
   *
   * **Complexity**: Constant.
   */
   BOOST_CONTAINER_NODISCARD reference front() BOOST_NOEXCEPT;

   /**
   * **Returns**: A constant reference to the first element in the devector.
   *
   * **Precondition**: `!empty()`.
   *
   * **Complexity**: Constant.
   */
   BOOST_CONTAINER_NODISCARD const_reference front() const BOOST_NOEXCEPT;

   /**
   * **Returns**: A reference to the last element in the devector.
   *
   * **Precondition**: `!empty()`.
   *
   * **Complexity**: Constant.
   */
   BOOST_CONTAINER_NODISCARD reference back() BOOST_NOEXCEPT;

   /**
   * **Returns**: A constant reference to the last element in the devector.
   *
   * **Precondition**: `!empty()`.
   *
   * **Complexity**: Constant.
   */
   BOOST_CONTAINER_NODISCARD const_reference back() const BOOST_NOEXCEPT;

   /**
   * **Returns**: A pointer to the underlying array serving as element storage.
   * The range `[data(); data() + size())` is always valid. For a non-empty devector,
   * `data() == &front()`.
   *
   * **Complexity**: Constant.
   */
   BOOST_CONTAINER_NODISCARD T* data() BOOST_NOEXCEPT;

   /**
   * **Returns**: A constant pointer to the underlying array serving as element storage.
   * The range `[data(); data() + size())` is always valid. For a non-empty devector,
   * `data() == &front()`.
   *
   * **Complexity**: Constant.
   */
   BOOST_CONTAINER_NODISCARD const T* data() const BOOST_NOEXCEPT;
   #else
   using base_t::operator[];
   using base_t::nth;
   using base_t::index_of;
   using base_t::at;
   using base_t::front;
   using base_t::back;
   using base_t::data;
   #endif

   // modifiers:

   /**
   * **Effects**: Pushes a new element to the front of the devector.
   * The element is constructed in-place, using the perfect forwarded `args`
   * as constructor arguments. Invalidates iterators if reallocation is needed.
   * (`front_free_capacity() == 0`)
   *
   * **Requires**: `T` shall be [EmplaceConstructible] from `args` and [MoveInsertable] into `*this`.
   *
   * **Exceptions**: Basic exception guarantee: if the elements are relocated inside the
   * buffer (`front_free_capacity() == 0` and there is free capacity at the other end) and the
   * construction of the new element throws, the devector can lose its elements.
   * Strong exception guarantee if `front_free_capacity() > 0` or if the `relocate_never`
   * option is used (the elements are never relocated).
   *
   * **Complexity**: Amortized constant in the size of `*this`.
   * (Constant, if `front_free_capacity() > 0`)
   *
   * [EmplaceConstructible]: http://en.cppreference.com/w/cpp/concept/EmplaceConstructible
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   */
   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED) || !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
   template <class... Args>
   reference emplace_front(Args&&... args)
   {
      if (BOOST_LIKELY(front_free_capacity() != 0)) // fast path
      {
         pointer const p = this->m_buffer + (this->m_front_idx - 1u);
         this->alloc_construct(p, boost::forward<Args>(args)...);
         --this->m_front_idx;
         BOOST_ASSERT(invariants_ok());
         return *p;
      }
      else
      {
         typedef dtl::insert_emplace_proxy<allocator_type, Args...> proxy_t;
         return *this->insert_range_slow_path(this->begin(), 1, proxy_t(::boost::forward<Args>(args)...));
      }
   }

   #else //defined(BOOST_CONTAINER_DOXYGEN_INVOKED) || !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   #define BOOST_CONTAINER_DEVECTOR_EMPLACE_FRONT(N) \
   BOOST_MOVE_TMPL_LT##N BOOST_MOVE_CLASS##N BOOST_MOVE_GT##N \
   inline reference emplace_front(BOOST_MOVE_UREF##N)\
   {\
      if (front_free_capacity())\
      {\
         pointer const p = this->m_buffer + (this->m_front_idx - 1u);\
         this->alloc_construct(p BOOST_MOVE_I##N BOOST_MOVE_FWD##N);\
         --this->m_front_idx;\
         return *p;\
      }\
      else\
      {\
         typedef dtl::insert_emplace_proxy_arg##N<allocator_type BOOST_MOVE_I##N BOOST_MOVE_TARG##N> proxy_t;\
         return *this->insert_range_slow_path(this->begin(), 1, proxy_t(BOOST_MOVE_FWD##N));\
      }\
   }\
   //
   BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_DEVECTOR_EMPLACE_FRONT)
   #undef BOOST_CONTAINER_DEVECTOR_EMPLACE_FRONT

   #endif

   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   /**
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   *
   * **Effects**: Pushes the copy of `x` to the front of the devector.
   * Invalidates iterators if reallocation is needed.
   * (`front_free_capacity() == 0`)
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this`.
   *
   * **Exceptions**: Basic exception guarantee: if the elements are relocated inside the
   * buffer (`front_free_capacity() == 0` and there is free capacity at the other end) and the
   * construction of the new element throws, the devector can lose its elements.
   * Strong exception guarantee if `front_free_capacity() > 0` or if the `relocate_never`
   * option is used (the elements are never relocated).
   *
   * **Complexity**: Amortized constant in the size of `*this`.
   * (Constant, if `front_free_capacity() > 0`)
   */
   void push_front(const T& x);

   /**
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   *
   * **Effects**: Move constructs a new element at the front of the devector using `x`.
   * Invalidates iterators if reallocation is needed.
   * (`front_free_capacity() == 0`)
   *
   * **Requires**: `T` shall be [MoveInsertable] into `*this`.
   *
   * **Exceptions**: Basic exception guarantee: if the elements are relocated inside the
   * buffer (`front_free_capacity() == 0` and there is free capacity at the other end) and the
   * construction of the new element throws, the devector can lose its elements.
   * If `front_free_capacity() > 0` or if the `relocate_never` option is used, strong exception
   * guarantee, not regarding the state of `x`.
   *
   * **Complexity**: Amortized constant in the size of `*this`.
   * (Constant, if `front_free_capacity() > 0`)
   */
   void push_front(T&& x);

   #else
   BOOST_MOVE_CONVERSION_AWARE_CATCH(push_front, T, void, priv_push_front)
   #endif

   /**
   * **Effects**: Removes the first element of `*this`.
   *
   * **Precondition**: `!empty()`.
   *
   * **Postcondition**: `front_free_capacity()` is incremented by 1.
   *
   * **Complexity**: Constant.
   */
   void pop_front() BOOST_NOEXCEPT
   {
      BOOST_ASSERT(!empty());
      allocator_traits_type::destroy(static_cast<allocator_type&>(*this), boost::movelib::to_raw_pointer(this->m_buffer + this->m_front_idx));
      ++this->m_front_idx;
      BOOST_ASSERT(invariants_ok());
   }

   /**
   * **Effects**: Pushes a new element to the back of the devector.
   * The element is constructed in-place, using the perfect forwarded `args`
   * as constructor arguments. Invalidates iterators if reallocation is needed.
   * (`back_free_capacity() == 0`)
   *
   * **Requires**: `T` shall be [EmplaceConstructible] from `args` and [MoveInsertable] into `*this`,
   * and [MoveAssignable].
   *
   * **Exceptions**: Basic exception guarantee: if the elements are relocated inside the
   * buffer (`back_free_capacity() == 0` and there is free capacity at the other end) and the
   * construction of the new element throws, the devector can lose its elements.
   * Strong exception guarantee if `back_free_capacity() > 0` or if the `relocate_never`
   * option is used (the elements are never relocated).
   *
   * **Complexity**: Amortized constant in the size of `*this`.
   * (Constant, if `back_free_capacity() > 0`)
   *
   * [EmplaceConstructible]: http://en.cppreference.com/w/cpp/concept/EmplaceConstructible
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   * [MoveAssignable]: http://en.cppreference.com/w/cpp/concept/MoveAssignable
   */
   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED) || !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
   template <class... Args>
   inline reference emplace_back(Args&&... args)
   {
      if (BOOST_LIKELY(this->back_free_capacity() != 0)){
         pointer const p = this->m_buffer + this->m_back_idx;
         this->alloc_construct(p, boost::forward<Args>(args)...);
         ++this->m_back_idx;
         BOOST_ASSERT(invariants_ok());
         return *p;
      }
      else {
         typedef dtl::insert_emplace_proxy<allocator_type, Args...> proxy_t;
         return *this->insert_range_slow_path(this->end(), 1, proxy_t(::boost::forward<Args>(args)...));
      }
   }

   #else //defined(BOOST_CONTAINER_DOXYGEN_INVOKED) || !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   #define BOOST_CONTAINER_DEVECTOR_EMPLACE_BACK(N) \
   BOOST_MOVE_TMPL_LT##N BOOST_MOVE_CLASS##N BOOST_MOVE_GT##N \
   inline reference emplace_back(BOOST_MOVE_UREF##N)\
   {\
      if (this->back_free_capacity()){\
         pointer const p = this->m_buffer + this->m_back_idx;\
         this->alloc_construct(p BOOST_MOVE_I##N BOOST_MOVE_FWD##N);\
         ++this->m_back_idx;\
         return *p;\
      }\
      else {\
         typedef dtl::insert_emplace_proxy_arg##N<allocator_type BOOST_MOVE_I##N BOOST_MOVE_TARG##N> proxy_t;\
         return *this->insert_range_slow_path(this->end(), 1, proxy_t(BOOST_MOVE_FWD##N));\
      }\
   }\
   //
   BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_DEVECTOR_EMPLACE_BACK)
   #undef BOOST_CONTAINER_DEVECTOR_EMPLACE_BACK

   #endif    //defined(BOOST_CONTAINER_DOXYGEN_INVOKED) || !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)


   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   /**
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   *
   * **Effects**: Pushes the copy of `x` to the back of the devector.
   * Invalidates iterators if reallocation is needed.
   * (`back_free_capacity() == 0`)
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this`.
   *
   * **Exceptions**: Basic exception guarantee: if the elements are relocated inside the
   * buffer (`back_free_capacity() == 0` and there is free capacity at the other end) and the
   * construction of the new element throws, the devector can lose its elements.
   * Strong exception guarantee if `back_free_capacity() > 0` or if the `relocate_never`
   * option is used (the elements are never relocated).
   *
   * **Complexity**: Amortized constant in the size of `*this`.
   * (Constant, if `back_free_capacity() > 0`)
   */
   void push_back(const T& x);

   /**
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   *
   * **Effects**: Move constructs a new element at the back of the devector using `x`.
   * Invalidates iterators if reallocation is needed.
   * (`back_free_capacity() == 0`)
   *
   * **Requires**: `T` shall be [MoveInsertable] into `*this`.
   *
   * **Exceptions**: Basic exception guarantee: if the elements are relocated inside the
   * buffer (`back_free_capacity() == 0` and there is free capacity at the other end) and the
   * construction of the new element throws, the devector can lose its elements.
   * If `back_free_capacity() > 0` or if the `relocate_never` option is used, strong exception
   * guarantee, not regarding the state of `x`.
   *
   * **Complexity**: Amortized constant in the size of `*this`.
   * (Constant, if `back_free_capacity() > 0`)
   */
   void push_back(T&& x);

   #else
   BOOST_MOVE_CONVERSION_AWARE_CATCH(push_back, T, void, priv_push_back)
   #endif

   /**
   * **Effects**: Removes the last element of `*this`.
   *
   * **Precondition**: `!empty()`.
   *
   * **Postcondition**: `back_free_capacity()` is incremented by 1.
   *
   * **Complexity**: Constant.
   */
   void pop_back() BOOST_NOEXCEPT
   {
      BOOST_ASSERT(! empty());
      --this->m_back_idx;
      allocator_traits_type::destroy(static_cast<allocator_type&>(*this), boost::movelib::to_raw_pointer(this->m_buffer + this->m_back_idx));
      BOOST_ASSERT(invariants_ok());
   }

   /**
   * **Effects**: Constructs a new element before the element pointed by `position`.
   * The element is constructed in-place, using the perfect forwarded `args`
   * as constructor arguments. Invalidates iterators if reallocation is needed.
   *
   * **Requires**: `T` shall be [EmplaceConstructible], and [MoveInsertable] into `*this`,
   * and [MoveAssignable].
   *
   * **Returns**: Iterator pointing to the newly constructed element.
   *
   * **Exceptions**: Strong exception guarantee if `T` is `NothrowConstructible`
   * and `NothrowAssignable`, Basic exception guarantee otherwise.
   *
   * **Complexity**: Linear in the size of `*this`.
   *
   * [EmplaceConstructible]: http://en.cppreference.com/w/cpp/concept/EmplaceConstructible
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   * [MoveAssignable]: http://en.cppreference.com/w/cpp/concept/MoveAssignable
   */
   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED) || !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
   template <class... Args>
   iterator emplace(const_iterator position, Args&&... args)
   {
      BOOST_ASSERT(position >= begin());
      BOOST_ASSERT(position <= end());
      typedef dtl::insert_emplace_proxy<allocator_type, Args...> proxy_t;
      bool prefer_move_back;
      if (position == end()){
         if(back_free_capacity()) // fast path
         {
            pointer const p = this->m_buffer + this->m_back_idx;
            this->alloc_construct(p, boost::forward<Args>(args)...);
            ++this->m_back_idx;
            return iterator(p);
         }
         prefer_move_back = true;
      }
      else if (position == begin()){
         if(front_free_capacity()) // secondary fast path
         {
            pointer const p = this->m_buffer + (this->m_front_idx - 1);
            this->alloc_construct(p, boost::forward<Args>(args)...);
            --this->m_front_idx;
            return iterator(p);
         }
         prefer_move_back = false;
      }
      else{
         iterator nonconst_pos = unconst_iterator(position);
         prefer_move_back = should_move_back(position);
         allocator_type &al = static_cast<allocator_type&>(*this);

         if(prefer_move_back){
            if(back_free_capacity()){
               boost::container::expand_forward_and_insert_nonempty_middle_alloc
                  ( al
                  , boost::movelib::to_raw_pointer(nonconst_pos)
                  , this->priv_raw_end()
                  , 1, proxy_t(::boost::forward<Args>(args)...));
               ++this->m_back_idx;
               return nonconst_pos;
            }
         }
         else{
            if (front_free_capacity()){
               boost::container::expand_backward_and_insert_nonempty_middle_alloc
               (al
                  , this->priv_raw_begin()
                  , boost::movelib::to_raw_pointer(nonconst_pos)
                  , 1, proxy_t(::boost::forward<Args>(args)...));
               --this->m_front_idx;
               return --nonconst_pos;
            }
         }
      }
      return this->insert_range_slow_path(position, 1, proxy_t(::boost::forward<Args>(args)...));
   }

   #else //defined(BOOST_CONTAINER_DOXYGEN_INVOKED) || !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

   #define BOOST_CONTAINER_DEVECTOR_EMPLACE(N) \
   BOOST_MOVE_TMPL_LT##N BOOST_MOVE_CLASS##N BOOST_MOVE_GT##N \
   iterator emplace(const_iterator position BOOST_MOVE_I##N BOOST_MOVE_UREF##N)\
   {\
      BOOST_ASSERT(position >= begin());\
      BOOST_ASSERT(position <= end());\
      typedef dtl::insert_emplace_proxy_arg##N<allocator_type  BOOST_MOVE_I##N BOOST_MOVE_TARG##N> proxy_t;\
      bool prefer_move_back;\
      if (position == end()){\
         if(back_free_capacity())\
         {\
            pointer const p = this->m_buffer + this->m_back_idx;\
            this->alloc_construct(p BOOST_MOVE_I##N BOOST_MOVE_FWD##N);\
            ++this->m_back_idx;\
            return iterator(p);\
         }\
         prefer_move_back = true;\
      }\
      else if (position == begin()){\
         if(front_free_capacity())\
         {\
            pointer const p = this->m_buffer + (this->m_front_idx - 1);\
            this->alloc_construct(p BOOST_MOVE_I##N BOOST_MOVE_FWD##N);\
            --this->m_front_idx;\
            return iterator(p);\
         }\
         prefer_move_back = false;\
      }\
      else{\
         iterator nonconst_pos = unconst_iterator(position);\
         prefer_move_back = should_move_back(position);\
         allocator_type &al = static_cast<allocator_type&>(*this);\
         \
         if(prefer_move_back){\
            if(back_free_capacity()){\
               boost::container::expand_forward_and_insert_nonempty_middle_alloc\
                  ( al\
                  , boost::movelib::to_raw_pointer(nonconst_pos)\
                  , this->priv_raw_end()\
                  , 1, proxy_t(BOOST_MOVE_FWD##N));\
               ++this->m_back_idx;\
               return nonconst_pos;\
            }\
         }\
         else{\
            if (front_free_capacity()){\
               boost::container::expand_backward_and_insert_nonempty_middle_alloc\
               (al\
                  , this->priv_raw_begin()\
                  , boost::movelib::to_raw_pointer(nonconst_pos)\
                  , 1, proxy_t(BOOST_MOVE_FWD##N));\
               --this->m_front_idx;\
               return --nonconst_pos;\
            }\
         }\
      }\
      return this->insert_range_slow_path(position, 1, proxy_t(BOOST_MOVE_FWD##N));\
   }\
   //
   BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_DEVECTOR_EMPLACE)
   #undef BOOST_CONTAINER_DEVECTOR_EMPLACE

   #endif    //defined(BOOST_CONTAINER_DOXYGEN_INVOKED) || !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)


   #if defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   /**
   * **Effects**: Copy constructs a new element before the element pointed by `position`,
   * using `x` as constructor argument. Invalidates iterators if reallocation is needed.
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this` and and [CopyAssignable].
   *
   * **Returns**: Iterator pointing to the newly constructed element.
   *
   * **Exceptions**: Strong exception guarantee if `T` is `NothrowConstructible`
   * and `NothrowAssignable`, Basic exception guarantee otherwise.
   *
   * **Complexity**: Linear in the size of `*this`.
   *
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   * [CopyAssignable]: http://en.cppreference.com/w/cpp/concept/CopyAssignable
   */
   iterator insert(const_iterator position, const T &x);

   /**
   * **Effects**: Move constructs a new element before the element pointed by `position`,
   * using `x` as constructor argument. Invalidates iterators if reallocation is needed.
   *
   * **Requires**: `T` shall be [MoveInsertable] into `*this` and and [CopyAssignable].
   *
   * **Returns**: Iterator pointing to the newly constructed element.
   *
   * **Exceptions**: Strong exception guarantee if `T` is `NothrowConstructible`
   * and `NothrowAssignable` (not regarding the state of `x`),
   * Basic exception guarantee otherwise.
   *
   * **Complexity**: Linear in the size of `*this`.
   *
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   * [CopyAssignable]: http://en.cppreference.com/w/cpp/concept/CopyAssignable
   */
   iterator insert(const_iterator position, T &&x);
   #else
   BOOST_MOVE_CONVERSION_AWARE_CATCH_1ARG(insert, T, iterator, priv_insert, const_iterator, const_iterator)
   #endif

   /**
   * **Effects**: Copy constructs `n` elements before the element pointed by `position`,
   * using `x` as constructor argument. Invalidates iterators if reallocation is needed.
   *
   * **Requires**: `T` shall be [CopyInsertable] into `*this` and and [CopyAssignable].
   *
   * **Returns**: Iterator pointing to the first inserted element, or `position`, if `n` is zero.
   *
   * **Exceptions**: Strong exception guarantee if `T` is `NothrowConstructible`
   * and `NothrowAssignable`, Basic exception guarantee otherwise.
   *
   * **Complexity**: Linear in the size of `*this` and `n`.
   *
   * [CopyInsertable]: http://en.cppreference.com/w/cpp/concept/CopyInsertable
   * [CopyAssignable]: http://en.cppreference.com/w/cpp/concept/CopyAssignable
   */
   inline iterator insert(const_iterator position, size_type n, const T& x)
   {
      BOOST_ASSERT(!(this->priv_insertion_moves_elements(position, n) && this->priv_is_element_address(dtl::addressof(x))));
      cvalue_iterator first(x, n);
      cvalue_iterator last = first + n;
      return this->insert_range(position, first, last);
   }

   /**
   * **Effects**: Copy constructs elements before the element pointed by position
   * using each element in the range pointed by `first` and `last` as constructor arguments.
   * Invalidates iterators if reallocation is needed.
   *
   * **Requires**: `T` shall be [EmplaceConstructible] into `*this` from `*first`. If the specified iterator
   * does not meet the forward iterator requirements, `T` shall also be [MoveInsertable] into `*this`
   * and [MoveAssignable].
   *
   * **Precondition**: `first` and `last` are not iterators into `*this`.
   *
   * **Returns**: Iterator pointing to the first inserted element, or `position`, if `first == last`.
   *
   * **Complexity**: Linear in the size of `*this` and `N` (where `N` is the distance between `first` and `last`).
   * Makes only `N` calls to the constructor of `T` and no reallocations if iterators `first` and `last`
   * are of forward, bidirectional, or random access categories. It makes 2N calls to the copy constructor of `T`
   * and `O(log(N))` reallocations if they are just input iterators.
   *
   * **Exceptions**: Strong exception guarantee if `T` is `NothrowConstructible`
   * and `NothrowAssignable`, Basic exception guarantee otherwise.
   *
   * **Remarks**: Each iterator in the range `[first,last)` shall be dereferenced exactly once,
   * unless an exception is thrown.
   *
   * [EmplaceConstructible]: http://en.cppreference.com/w/cpp/concept/EmplaceConstructible
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   * [MoveAssignable]: http://en.cppreference.com/w/cpp/concept/MoveAssignable
   */
   template <class InputIterator>
   iterator insert(const_iterator position, InputIterator first, InputIterator last
      //Input iterators
      BOOST_CONTAINER_DOCIGN(BOOST_MOVE_I typename dtl::disable_if_or
            < void
            BOOST_MOVE_I dtl::is_convertible<InputIterator BOOST_MOVE_I size_type>
            BOOST_MOVE_I dtl::is_not_input_iterator<InputIterator>
            >::type * = 0)
      )
   {
      const size_type insert_index = static_cast<size_type>(position - this->cbegin());
      const size_type old_size = static_cast<size_type>(this->size());

      //Append the new elements, then rotate them to the position
      BOOST_CONTAINER_TRY{
         for (; first != last; ++first) {
            this->emplace_back(*first);
         }
      }
      BOOST_CONTAINER_CATCH(...){
         //No effects: destroy the new elements at the back
         while (this->size() > old_size) {
            this->pop_back();
         }
         BOOST_CONTAINER_RETHROW
      }
      BOOST_CONTAINER_CATCH_END
      iterator rit (this->begin() + difference_type(insert_index));
      boost::movelib::rotate_gcd(rit, this->begin() + difference_type(old_size), this->end());
      return rit;
   }

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   template <class ForwardIterator>
   inline iterator insert(const_iterator position, ForwardIterator first, ForwardIterator last
      //Other iterators
      BOOST_CONTAINER_DOCIGN(BOOST_MOVE_I typename dtl::disable_if_or
            < void
            BOOST_MOVE_I dtl::is_convertible<ForwardIterator BOOST_MOVE_I size_type>
            BOOST_MOVE_I dtl::is_input_iterator<ForwardIterator>
            >::type * = 0)
      )
   {
      return insert_range(position, first, last);
   }

   #endif // ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   /** **Equivalent to**: `insert(position, il.begin(), il.end())` */
   inline iterator insert(const_iterator position, std::initializer_list<T> il)
   {
      return this->insert(position, il.begin(), il.end());
   }
   #endif

   /**
   * [MoveAssignable]: http://en.cppreference.com/w/cpp/concept/MoveAssignable
   *
   * **Effects**: Destroys the element pointed by `position` and removes it from the devector.
   * Invalidates iterators.
   *
   * **Requires**: `T` shall be [MoveAssignable].
   *
   * **Precondition**: `position` must be in the range of `[begin(), end())`.
   *
   * **Returns**: Iterator pointing to the element immediately following the erased element
   * prior to its erasure. If no such element exists, `end()` is returned.
   *
   * **Exceptions**: Strong exception guarantee if `T` is `NothrowAssignable`,
   * Basic exception guarantee otherwise.
   *
   * **Complexity**: Linear in half the size of `*this`.
   */
   iterator erase(const_iterator position)
   {
      return erase(position, position + 1);
   }

   /**
   * [MoveAssignable]: http://en.cppreference.com/w/cpp/concept/MoveAssignable
   *
   * **Effects**: Destroys the range `[first,last)` and removes it from the devector.
   * Invalidates iterators.
   *
   * **Requires**: `T` shall be [MoveAssignable].
   *
   * **Precondition**: `[first,last)` must be in the range of `[begin(), end())`.
   *
   * **Returns**: Iterator pointing to the element pointed to by `last` prior to any elements
   * being erased. If no such element exists, `end()` is returned.
   *
   * **Exceptions**: Strong exception guarantee if `T` is `NothrowAssignable`,
   * Basic exception guarantee otherwise.
   *
   * **Complexity**: Linear in half the size of `*this`
   * plus the distance between `first` and `last`.
   */
   iterator erase(const_iterator first, const_iterator last)
   {
      iterator nc_first = unconst_iterator(first);
      iterator nc_last  = unconst_iterator(last);
      return erase(nc_first, nc_last);
   }

   /**
   * [MoveAssignable]: http://en.cppreference.com/w/cpp/concept/MoveAssignable
   *
   * **Effects**: Destroys the range `[first,last)` and removes it from the devector.
   * Invalidates iterators.
   *
   * **Requires**: `T` shall be [MoveAssignable].
   *
   * **Precondition**: `[first,last)` must be in the range of `[begin(), end())`.
   *
   * **Returns**: Iterator pointing to the element pointed to by `last` prior to any elements
   * being erased. If no such element exists, `end()` is returned.
   *
   * **Exceptions**: Strong exception guarantee if `T` is `NothrowAssignable`,
   * Basic exception guarantee otherwise.
   *
   * **Complexity**: Linear in half the size of `*this`.
   */
   iterator erase(iterator first, iterator last)
   {
      if (BOOST_UNLIKELY(first == last))  //An empty range is a no-op
         return first;
      size_type front_distance = pos_to_index(last);
      size_type back_distance  = size_type(end() - first);
      size_type n = boost::container::iterator_udistance(first, last);

      if (front_distance < back_distance) {
            // move n to the right
            boost::container::move_backward(begin(), first, last);

            for (iterator i = begin(); i != begin() + n; ++i) {
               allocator_traits_type::destroy(static_cast<allocator_type&>(*this), boost::movelib::to_raw_pointer(i));
            }
            //n is always less than max stored_size_type
            this->set_front_idx(this->m_front_idx + n);

            BOOST_ASSERT(invariants_ok());
            return last;
      }
      else {
            // move n to the left
            boost::container::move(last, end(), first);

            for (iterator i = end() - n; i != end(); ++i)
            {
               allocator_traits_type::destroy(static_cast<allocator_type&>(*this), boost::movelib::to_raw_pointer(i));
            }
            //n is always less than max stored_size_type
            this->set_back_idx(this->m_back_idx - n);

            BOOST_ASSERT(invariants_ok());
            return first;
      }
   }

   /**
   * [MoveInsertable]: http://en.cppreference.com/w/cpp/concept/MoveInsertable
   *
   * **Effects**: exchanges the contents of `*this` and `b`.
   *
   * **Requires**: instances of `T` must be swappable by unqualified call of `swap`
   * and `T` must be [MoveInsertable] into `*this`.
   *
   * **Precondition**: The allocators should allow propagation or should compare equal.
   *
   * **Exceptions**: Basic exceptions guarantee if not `noexcept`.
   *
   * **Complexity**: Constant.
   */
   void swap(devector& b)
      BOOST_NOEXCEPT_IF( allocator_traits_type::propagate_on_container_swap::value
                                 || allocator_traits_type::is_always_equal::value)
   {
      BOOST_CONSTEXPR_OR_CONST bool propagate_alloc = allocator_traits_type::propagate_on_container_swap::value;
      BOOST_ASSERT(propagate_alloc || static_cast<allocator_type&>(*this) == static_cast<allocator_type&>(b)); // else it's undefined behavior

      swap_big_big(*this, b);

      // swap indices
      boost::adl_move_swap(this->m_front_idx, b.m_front_idx);
      boost::adl_move_swap(this->m_back_idx, b.m_back_idx);

      //And now swap the allocator
      dtl::swap_alloc(static_cast<allocator_type&>(*this), static_cast<allocator_type&>(b), dtl::bool_<propagate_alloc>());

      BOOST_ASSERT(   invariants_ok());
      BOOST_ASSERT(b.invariants_ok());
   }

   /**
   * **Effects**: Destroys all elements in the devector.
   * Invalidates all references, pointers and iterators to the
   * elements of the devector.
   *
   * **Postcondition**: `empty() && front_free_capacity() == 0
   * && back_free_capacity() == old capacity`.
   *
   * **Complexity**: Linear in the size of `*this`.
   *
   * **Remarks**: Does not free memory.
   */
   void clear() BOOST_NOEXCEPT
   {
      destroy_elements(begin(), end());
      this->m_back_idx = this->m_front_idx;
   }

   BOOST_CONTAINER_NODISCARD inline
      friend bool operator==(const devector& x, const devector& y)
   {   return x.size() == y.size() && ::boost::container::algo_equal(x.begin(), x.end(), y.begin());   }

   BOOST_CONTAINER_NODISCARD inline
      friend bool operator!=(const devector& x, const devector& y)
   {   return !(x == y); }

   BOOST_CONTAINER_NODISCARD inline
      friend bool operator< (const devector& x, const devector& y)
   {   return boost::container::algo_lexicographical_compare(x.begin(), x.end(), y.begin(), y.end());   }

   BOOST_CONTAINER_NODISCARD inline
      friend bool operator>(const devector& x, const devector& y)
   {   return y < x;   }

   BOOST_CONTAINER_NODISCARD inline
      friend bool operator<=(const devector& x, const devector& y)
   {   return !(y < x);   }

   BOOST_CONTAINER_NODISCARD inline
      friend bool operator>=(const devector& x, const devector& y)
   {   return !(x < y);   }

   inline friend void swap(devector& x, devector& y)
      BOOST_NOEXCEPT_IF( allocator_traits_type::propagate_on_container_swap::value
                                 || allocator_traits_type::is_always_equal::value)
   {   x.swap(y);   }

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   //Functions for optimizations, not for users
   T *unused_storage(size_type &sz)
   {
      T *const storage_addr = boost::movelib::to_raw_pointer(this->m_buffer);
      if(this->empty()){
         sz = this->m_capacity;
         return storage_addr;
      }
      else if(this->back_free_capacity() > this->front_free_capacity()){
         sz = this->back_free_capacity();
         return storage_addr + this->m_back_idx;
      }
      else{
         sz = this->front_free_capacity();
         return storage_addr;
      }
   }

   private:

   void priv_move_assign(BOOST_RV_REF(devector) x, dtl::bool_<true> /*steal_resources*/)
   {
      this->clear();
      this->deallocate_buffer();

      //Move allocator if needed
      dtl::bool_<allocator_traits_type::
         propagate_on_container_move_assignment::value> flag;
      dtl::move_alloc(static_cast<allocator_type&>(*this), static_cast<allocator_type&>(x), flag);

      this->m_capacity = x.m_capacity;
      this->m_buffer = x.m_buffer;
      this->m_front_idx = x.m_front_idx;
      this->m_back_idx = x.m_back_idx;

      // leave x in valid state
      x.m_capacity = 0u;
      x.m_buffer = pointer();
      x.m_back_idx = x.m_front_idx = 0;
   }

   void priv_move_assign(BOOST_RV_REF(devector) x, dtl::bool_<false> /*steal_resources*/)
   {
      //We can't guarantee a compile-time equal allocator or propagation so fallback to runtime
      //Resources can be transferred if both allocators are equal
      if (static_cast<allocator_type&>(*this) == static_cast<allocator_type&>(x)) {
         this->priv_move_assign(boost::move(x), dtl::true_());
      }
      else {
         // We can't steal memory.
         move_iterator<iterator> xbegin = boost::make_move_iterator(x.begin());
         move_iterator<iterator> xend = boost::make_move_iterator(x.end());

         //Move allocator if needed
         dtl::bool_<allocator_traits_type::
            propagate_on_container_move_assignment::value> flag;
         dtl::move_alloc(static_cast<allocator_type&>(*this), static_cast<allocator_type&>(x), flag);

         if (this->m_capacity >= x.size()) {
            this->overwrite_buffer(xbegin, xend);
         }
         else {
            this->allocate_and_copy_range(xbegin, xend);
         }
      }
   }

   using base_t::pos_to_index;
   using base_t::should_move_back;
   using base_t::unconst_iterator;
   using base_t::priv_raw_begin;
   using base_t::priv_raw_end;


   template <class U>
   inline void priv_push_front(BOOST_FWD_REF(U) u)
   {
      BOOST_ASSERT(!(!this->front_free_capacity() && this->priv_is_element_address(dtl::addressof(u))));
      this->emplace_front(boost::forward<U>(u));
   }

   template <class U>
   inline void priv_push_back(BOOST_FWD_REF(U) u)
   {
      BOOST_ASSERT(!(!this->back_free_capacity() && this->priv_is_element_address(dtl::addressof(u))));
      this->emplace_back(boost::forward<U>(u));
   }

   template <class U>
   inline iterator priv_insert(const_iterator pos, BOOST_FWD_REF(U) u)
   {
      BOOST_ASSERT(!(this->priv_insertion_moves_elements(pos, 1u) && this->priv_is_element_address(dtl::addressof(u))));
      return this->emplace(pos, boost::forward<U>(u));
   }

   //Debug checks of the precondition "an argument must not refer to an element of
   //the container" (see "Arguments that refer to elements of the same container" in
   //the documentation). The checks are done only when the operation moves elements
   //before the argument is read: then the inserted value would be wrong.
   using base_t::priv_is_element_address;
   using base_t::priv_insertion_moves_elements;

   // allocator_type wrappers

   static pointer do_allocate(allocator_type &a, size_type cap)
   {
      if (cap) {
         //First detect overflow on smaller stored_size_types
         if (cap > stored_size_type(-1)){
               boost::container::throw_length_error("get_next_capacity, allocator's max size reached");
         }
         return allocator_traits_type::allocate(a, cap);
      }
      else {
         return pointer();
      }
   }

   //Returns ffc + bfc, or throws length_error if the sum overflows
   static size_type checked_capacity_sum(size_type const ffc, size_type const bfc)
   {
      if (BOOST_UNLIKELY(ffc > size_type(size_type(-1) - bfc))) {
         boost::container::throw_length_error("devector: requested capacity greater than max_size()");
      }
      return size_type(ffc + bfc);
   }

   //Allocates the buffer of a constructor with c elements of capacity. The
   //constructor builds the elements of [front_idx, back_idx), if any
   void priv_reserve(size_type c, size_type front_idx, size_type back_idx)
   {
      this->m_buffer = do_allocate(static_cast<allocator_type&>(*this), c);
      #ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS
      this->m_capacity_alloc_count = size_type(this->m_buffer != pointer());
      #endif
      //The setters cast the sizes, as the allocation function took care of overflows
      this->set_front_idx(front_idx);
      this->set_back_idx(back_idx);
      this->set_capacity(c);
   }

   pointer allocate(size_type cap)
   {
      pointer const p = do_allocate(static_cast<allocator_type&>(*this), cap);
      #ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS
      ++this->m_capacity_alloc_count;
      #endif // BOOST_CONTAINER_DEVECTOR_ALLOC_STATS
      return p;
   }

   void destroy_elements(pointer b, pointer e)
   {
      for (; b != e; ++b) {
         allocator_traits_type::destroy(static_cast<allocator_type&>(*this), boost::movelib::to_raw_pointer(b));
      }
   }

   void deallocate_buffer()
   {
      if (this->m_buffer) {
         allocator_traits_type::deallocate(static_cast<allocator_type&>(*this), this->m_buffer, this->m_capacity);
      }
   }

   #if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   template <typename... Args>
   inline void alloc_construct(pointer dst, Args&&... args)
   {
      allocator_traits_type::construct(
            static_cast<allocator_type&>(*this),
            boost::movelib::to_raw_pointer(dst),
            boost::forward<Args>(args)...
      );
   }

   #else //   !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

   #define BOOST_CONTAINER_DEVECTOR_ALLOC_CONSTRUCT(N) \
   BOOST_MOVE_TMPL_LT##N BOOST_MOVE_CLASS##N BOOST_MOVE_GT##N \
   inline void alloc_construct(pointer dst BOOST_MOVE_I##N BOOST_MOVE_UREF##N)\
   {\
      allocator_traits_type::construct(\
            static_cast<allocator_type&>(*this), boost::movelib::to_raw_pointer(dst) BOOST_MOVE_I##N BOOST_MOVE_FWD##N );\
   }
   //
   BOOST_MOVE_ITERATE_0TO9(BOOST_CONTAINER_DEVECTOR_ALLOC_CONSTRUCT)
   #undef BOOST_CONTAINER_DEVECTOR_ALLOC_CONSTRUCT

   #endif    //   !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

   size_type calculate_new_capacity(size_type requested_capacity)
   {
      size_type max = allocator_traits_type::max_size(static_cast<allocator_type&>(*this));
      (clamp_by_stored_size_type)(max, stored_size_type());
      const size_type remaining_additional_cap = max - size_type(this->m_capacity);
      const size_type min_additional_cap = requested_capacity - size_type(this->m_capacity);
      if ( remaining_additional_cap < min_additional_cap )
            boost::container::throw_length_error("devector: get_next_capacity, max size exceeded");

      return growth_factor_type()( size_type(this->m_capacity), min_additional_cap, max);
   }

   //The resize members construct the new elements with an insertion proxy: value-initialized,
   //default-initialized or copies
   template <class InsertionProxy>
   void resize_impl(size_type sz, InsertionProxy proxy)
   {
      const size_type old_sz = this->size();
      if (sz > old_sz)
      {
         const size_type n = sz - old_sz;

         if (sz <= this->m_capacity)
         {
            //Construct at back
            const size_type bfc = this->back_free_capacity();
            const size_type b = n < bfc ? n : bfc;
            proxy.uninitialized_copy_n_and_update(static_cast<allocator_type&>(*this), this->priv_raw_end(), b);
            this->set_back_idx(this->m_back_idx + b);

            //Construct remaining at front
            const size_type f = n - b;
            proxy.uninitialized_copy_n_and_update(static_cast<allocator_type&>(*this), this->priv_raw_begin() - f, f);
            this->set_front_idx(this->m_front_idx - f);
         }
         else
         {
            resize_back_slow_path(sz, n, proxy);
         }
      }
      else
      {
         const size_type n = old_sz - sz;
         const size_type new_bidx = this->m_back_idx - n;
         destroy_elements(this->m_buffer + new_bidx, this->m_buffer + this->m_back_idx);
         this->set_back_idx(new_bidx);
      }
   }

   template <class InsertionProxy>
   void resize_front_impl(size_type sz, InsertionProxy proxy)
   {
      const size_type old_sz = this->size();
      if (sz > old_sz)
      {
         const size_type n = sz - old_sz;

         if (sz <= this->front_capacity())
         {
            proxy.uninitialized_copy_n_and_update(static_cast<allocator_type&>(*this), this->priv_raw_begin() - n, n);
            this->set_front_idx(this->m_front_idx - n);
         }
         else
         {
            resize_front_slow_path(sz, n, proxy);
         }
      }
      else {
         const size_type n = old_sz - sz;
         const size_type new_fidx = this->m_front_idx + n;
         destroy_elements(this->m_buffer + this->m_front_idx, this->m_buffer + new_fidx);
         this->set_front_idx(new_fidx);
      }
   }

   //Not enough room at the front: as an insertion at begin(), the elements are
   //relocated if the buffer has enough free space, otherwise a new buffer is allocated
   template <class InsertionProxy>
   void resize_front_slow_path(size_type, size_type n, InsertionProxy proxy)
   {  this->insert_range_slow_path(this->cbegin(), n, proxy);  }

   template <class InsertionProxy>
   void resize_back_impl(size_type sz, InsertionProxy proxy)
   {
      const size_type old_sz = this->size();
      if (sz > old_sz)
      {
         const size_type n = sz - old_sz;

         if (sz <= this->back_capacity())
         {
            proxy.uninitialized_copy_n_and_update(static_cast<allocator_type&>(*this), this->priv_raw_end(), n);
            this->set_back_idx(this->m_back_idx + n);
         }
         else
         {
            resize_back_slow_path(sz, n, proxy);
         }
      }
      else
      {
         const size_type n = old_sz - sz;
         const size_type new_bidx = this->m_back_idx - n;
         destroy_elements(this->m_buffer + new_bidx, this->m_buffer + this->m_back_idx);
         this->set_back_idx(new_bidx);
      }
   }

   //Not enough room at the back: as an insertion at end(), the elements are
   //relocated if the buffer has enough free space, otherwise a new buffer is allocated
   template <class InsertionProxy>
   void resize_back_slow_path(size_type, size_type n, InsertionProxy proxy)
   {  this->insert_range_slow_path(this->cend(), n, proxy);  }

   void reallocate_at(size_type new_capacity, size_type buffer_offset)
   {
      pointer new_buffer = allocate(new_capacity);
      {
         allocator_type &al = static_cast<allocator_type&>(*this);
         allocation_guard new_buffer_guard(new_buffer, new_capacity, al);
         boost::container::uninitialized_move_alloc(al, this->begin(), this->end(), new_buffer + buffer_offset);
         new_buffer_guard.release();
      }
      this->destroy_elements(this->m_buffer + this->m_front_idx, this->m_buffer + this->m_back_idx);
      this->deallocate_buffer();

      this->m_buffer = new_buffer;
      //Safe cast, allocate() will handle stored_size_type overflow
      this->set_capacity(new_capacity);
      this->set_back_idx(size_type(this->m_back_idx - this->m_front_idx) + buffer_offset);
      this->set_front_idx(buffer_offset);

      BOOST_ASSERT(invariants_ok());
   }

   template <typename ForwardIterator>
   iterator insert_range(const_iterator position, ForwardIterator first, ForwardIterator last)
   {
      BOOST_ASSERT(position >= begin());
      BOOST_ASSERT(position <= end());
      typedef dtl::insert_range_proxy<allocator_type, ForwardIterator> proxy_t;

      size_type const n = boost::container::iterator_udistance(first, last);
      bool prefer_move_back;
      allocator_type &al = static_cast<allocator_type&>(*this);

      if (BOOST_UNLIKELY(!n)) {
         return begin() + size_type(position - cbegin());
      }
      else if (position == end()) {
         if(back_free_capacity() >= n) // fast path
         {
            iterator r(this->end());
            boost::container::uninitialized_copy_alloc(al, first, last, this->priv_raw_end());
            this->set_back_idx(this->m_back_idx + n);
            return r;
         }
         prefer_move_back = true;
      }
      else if (position == begin()) {
         if(front_free_capacity() >= n) {// secondary fast path
            boost::container::uninitialized_copy_alloc(al, first, last, this->priv_raw_begin() - n);
            this->set_front_idx(this->m_front_idx - n);
            return begin();
         }
         prefer_move_back = false;
      }
      else{
         iterator nonconst_pos = unconst_iterator(position);
         prefer_move_back = should_move_back(position);

         if(prefer_move_back){
            if(back_free_capacity() >= n){
               boost::container::expand_forward_and_insert_nonempty_middle_alloc
                  ( al
                  , boost::movelib::to_raw_pointer(nonconst_pos)
                  , this->priv_raw_end()
                  , n, proxy_t(first));
               this->set_back_idx(this->m_back_idx + n);
               return nonconst_pos;
            }
         }
         else{
            if (front_free_capacity() >= n){
               boost::container::expand_backward_and_insert_nonempty_middle_alloc
                  ( al
                  , this->priv_raw_begin()
                  , boost::movelib::to_raw_pointer(nonconst_pos)
                  , n, proxy_t(first));
               this->set_front_idx(this->m_front_idx - n);
               return (nonconst_pos -= n);
            }
         }
      }
      return this->insert_range_slow_path(position, n, proxy_t(first));
   }

   template <class InsertionProxy>
   BOOST_CONTAINER_NOINLINE iterator insert_range_slow_path
      (const_iterator p, const size_type n, const InsertionProxy proxy)
   {
      size_type const back_free_cap = back_free_capacity();
      size_type const front_free_cap = front_free_capacity();
      size_type const free_cap = front_free_cap + back_free_cap;
      size_type const index = size_type(p - cbegin());

      size_type const cap = this->m_capacity;
      allocator_type &al = static_cast<allocator_type&>(*this);

      //Test if enough free memory would be left. A buffer of max_size() elements
      //can not be reallocated, so then all the free memory can be used.
      //relocate_never never relocates: then reallocation throws length_error.
      if (!devector_never_relocates && free_cap >= n &&
          ((free_cap - n) >= cap/devector_min_free_fraction || cap >= this->max_size())) {
         size_type const old_size = this->size();
         T* const raw_pos = const_cast<T*>(boost::movelib::to_raw_pointer(p));
         size_type const new_size = old_size + n;
         size_type const new_front_idx = (cap - new_size) / 2u;

         T* const raw_beg = this->priv_raw_begin();
         T* const new_raw_beg = raw_beg - std::ptrdiff_t(this->m_front_idx - new_front_idx);
         this->m_back_idx = 0u;
         this->m_front_idx = 0u;
         boost::container::expand_backward_forward_and_insert_alloc
            (raw_beg, old_size, new_raw_beg, raw_pos, n, proxy, al);
         this->set_front_idx(new_front_idx);
         this->set_back_idx(new_front_idx + new_size);
      }
      else {
         // reallocate
         const size_type new_capacity = calculate_new_capacity(this->m_capacity + n);
         pointer new_buffer = allocate(new_capacity);

         // guard allocation
         allocation_guard new_buffer_guard(new_buffer, new_capacity, al);
         const size_type new_front_index = (new_capacity - this->size() - n) / 2u;

         //Construct the new objects first: if this throws, the old buffer is not changed.
         //The new objects can also refer to old elements that are not yet moved.
         T* const raw_new_pos = boost::movelib::to_raw_pointer(new_buffer) + (new_front_index + index);
         InsertionProxy insertion_proxy(proxy);
         insertion_proxy.uninitialized_copy_n_and_update(al, raw_new_pos, n);
         new_buffer_guard.release();
         this->priv_relocate_with_gap(new_buffer, new_capacity, new_front_index, index, n);
      }
      return begin() + index;
   }

   //Moves the elements to new_buffer, leaving before them new_front_index free
   //positions and after the first "index" elements a gap of n already
   //constructed objects. Then destroys the old elements, deallocates the old
   //buffer and adopts the new one. If a move throws, the old buffer is not
   //changed and the new buffer is destroyed and deallocated.
   BOOST_CONTAINER_NOINLINE void priv_relocate_with_gap
      (pointer new_buffer, size_type new_capacity, size_type new_front_index, size_type index, size_type n)
   {
      typedef typename array_destructor<allocator_type>::type array_destructor_t;
      allocator_type &al = static_cast<allocator_type&>(*this);
      allocation_guard new_buffer_guard(new_buffer, new_capacity, al);
      size_type const old_size = this->size();
      T* const raw_new_start = boost::movelib::to_raw_pointer(new_buffer) + new_front_index;
      T* const raw_new_pos = raw_new_start + index;
      T* const raw_pos = this->priv_raw_begin() + index;
      array_destructor_t new_values_destroyer(raw_new_pos, raw_new_pos + n, al);
      (void) ::boost::container::uninitialized_move_alloc(al, this->priv_raw_begin(), raw_pos, raw_new_start);
      new_values_destroyer.set_begin(raw_new_start);
      (void) ::boost::container::uninitialized_move_alloc(al, raw_pos, this->priv_raw_end(), raw_new_pos + n);
      new_values_destroyer.release();
      new_buffer_guard.release();

      // cleanup
      destroy_elements(begin(), end());
      deallocate_buffer();

      // rebind members
      this->set_capacity(new_capacity);
      this->m_buffer = new_buffer;
      this->set_back_idx(new_front_index + old_size + n);
      this->set_front_idx(new_front_index);
   }


   template <typename Iterator>
   void construct_from_range(Iterator b, Iterator e)
   {
      allocation_guard buffer_guard(this->m_buffer, this->m_capacity, static_cast<allocator_type&>(*this));
      boost::container::uninitialized_copy_alloc(static_cast<allocator_type&>(*this), b, e, this->m_buffer);
      buffer_guard.release();
   }

   template <typename ForwardIterator>
   void allocate_and_copy_range(ForwardIterator first, ForwardIterator last)
   {
      size_type n = boost::container::iterator_udistance(first, last);

      pointer new_buffer = n ? allocate(n) : pointer();
      allocator_type &al = static_cast<allocator_type&>(*this);
      allocation_guard new_buffer_guard(new_buffer, n, al);
      boost::container::uninitialized_copy_alloc(al, first, last, new_buffer);
      this->destroy_elements(begin(), end());
      this->deallocate_buffer();

      this->set_capacity(n);
      this->m_buffer = new_buffer;
      this->m_front_idx = 0;
      this->set_back_idx(n);

      new_buffer_guard.release();
   }

   static void swap_big_big(devector& a, devector& b) BOOST_NOEXCEPT
   {
      boost::adl_move_swap(a.m_capacity, b.m_capacity);
      boost::adl_move_swap(a.m_buffer, b.m_buffer);
   }

   template <typename ForwardIterator>
   void overwrite_buffer_impl(ForwardIterator first, ForwardIterator last, dtl::true_)
   {
      const size_type n = boost::container::iterator_udistance(first, last);

      BOOST_ASSERT(this->m_capacity >= n);
      boost::container::uninitialized_copy_alloc_n
            ( static_cast<allocator_type&>(*this), first
            , n, boost::movelib::to_raw_pointer(this->m_buffer));
      this->m_front_idx = 0;
      this->set_back_idx(n);
   }

   template <typename InputIterator>
   InputIterator overwrite_buffer_impl(InputIterator first, InputIterator last, dtl::false_)
   {
      pointer pos = this->m_buffer;
      allocator_type &al = static_cast<allocator_type&>(*this);
      detail::construction_guard<allocator_type> front_guard(pos, al);

      while (first != last && pos != begin()) {
         this->alloc_construct(pos++, *first++);
         front_guard.extend();
      }

      while (first != last && pos != end()) {
         *pos++ = *first++;
      }

      detail::construction_guard<allocator_type> back_guard(pos, al);

      iterator capacity_end = this->m_buffer + this->m_capacity;
      while (first != last && pos != capacity_end) {
         this->alloc_construct(pos++, *first++);
         back_guard.extend();
      }

      pointer destroy_after = dtl::min_value(dtl::max_value(begin(), pos), end());
      destroy_elements(destroy_after, end());

      front_guard.release();
      back_guard.release();

      this->m_front_idx = 0;
      this->set_back_idx(pos_to_index(pos));
      return first;
   }

   template <typename ForwardIterator>
   inline void overwrite_buffer(ForwardIterator first, ForwardIterator last)
   {
      this->overwrite_buffer_impl(first, last, 
            dtl::bool_<dtl::is_trivially_destructible<T>::value>());
   }

   using base_t::invariants_ok;

   #ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS
   public:
   using base_t::reset_alloc_stats;
   using base_t::get_alloc_count;
   #endif // ifdef BOOST_CONTAINER_DEVECTOR_ALLOC_STATS

   #endif // ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
};

//! <b>Effects</b>: Erases all elements that compare equal to v from the container c.
//!
//! <b>Complexity</b>: Linear.
template <class T, class A, class O, class U>
inline typename devector<T, A, O>::size_type erase(devector<T, A, O>& c, const U& v)
{
   typename devector<T, A, O>::size_type old_size = c.size();
   c.erase(boost::container::remove(c.begin(), c.end(), v), c.end());
   return old_size - c.size();
}

//! <b>Effects</b>: Erases all elements that satisfy the predicate pred from the container c.
//!
//! <b>Complexity</b>: Linear.
template <class T, class A, class O, class Pred>
inline typename devector<T, A, O>::size_type erase_if(devector<T, A, O>& c, Pred pred)
{
   typename devector<T, A, O>::size_type old_size = c.size();
   c.erase(boost::container::remove_if(c.begin(), c.end(), pred), c.end());
   return old_size - c.size();
}

}} // namespace boost::container

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

namespace boost {

//!has_trivial_destructor_after_move<> specialization for optimizations
template <class T, class Allocator, class Options>
struct has_trivial_destructor_after_move<boost::container::devector<T, Allocator, Options> >
{
    typedef typename boost::container::devector<T, Allocator, Options>::allocator_type allocator_type;
    typedef boost::container::allocator_traits<allocator_type> allocator_traits_type;
    typedef typename allocator_traits_type::pointer pointer;
    BOOST_STATIC_CONSTEXPR bool value =
      allocator_traits_type::is_always_equal::value &&
      ::boost::has_trivial_destructor_after_move<allocator_type>::value &&
      ::boost::has_trivial_destructor_after_move<pointer>::value;
};

}

#endif    //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_DEVECTOR_HPP
