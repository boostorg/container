//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_STRING_HPP
#define BOOST_CONTAINER_STRING_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/container_fwd.hpp>
// container
#include <boost/container/allocator_traits.hpp>
#include <boost/container/new_allocator.hpp> //new_allocator
#include <boost/container/throw_exception.hpp>
#include <boost/container/options.hpp>
// container/detail
#include <boost/container/detail/alloc_helpers.hpp>
#include <boost/container/detail/allocation_type.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/container/detail/iterators.hpp>
#include <boost/container/detail/min_max.hpp>
#include <boost/container/detail/mpl.hpp>
#include <boost/container/detail/next_capacity.hpp>
#include <boost/move/detail/to_raw_pointer.hpp>
#include <boost/move/detail/iterator_to_raw_pointer.hpp>
#include <boost/container/detail/version_type.hpp>
#include <boost/container/detail/placement_new.hpp>
#include <boost/container/detail/type_traits.hpp>
#include <boost/container/detail/algorithm.hpp>
#include <boost/container/detail/minimal_char_traits_header.hpp>  // for char_traits
//intrusive
#include <boost/intrusive/pointer_traits.hpp>
#include <boost/intrusive/detail/hash_combine.hpp>
#include <boost/move/detail/launder.hpp>
//move
#include <boost/move/utility_core.hpp>
#include <boost/move/adl_move_swap.hpp>
#include <boost/move/traits.hpp>
#include <boost/move/algo/detail/rotate.hpp>

#include <iosfwd> 
#include <istream>   //
#include <ostream>
#include <ios>
#include <locale>
#include <cstddef>
#include <climits>

//std
#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
#include <initializer_list>   //for std::initializer_list
#endif

//GCC 12 has a regression for array-bounds warnings
#if defined(BOOST_GCC) && (BOOST_GCC >= 120000) && (BOOST_GCC < 130000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif


namespace boost {
namespace container {

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

template<class Options, class AllocatorSizeType>
struct get_string_opt
{
   typedef string_opt< typename default_if_void<typename Options::growth_factor_type, growth_factor_60>::type
                     , typename default_if_void<typename Options::stored_size_type, AllocatorSizeType>::type
                     , Options::inline_chars
                     > type;
};

template<class AllocatorSizeType>
struct get_string_opt<void, AllocatorSizeType>
{
   typedef string_opt<growth_factor_60, AllocatorSizeType, 0u> type;
};

namespace dtl {

//Traits::copy, move and compare usually call memcpy, memmove and memcmp, which
//require non-null pointers even if the count is zero. A string or a view can
//have a null pointer if its size is zero, so these helpers do not call Traits
//for a zero count. GCC 12 still reports -Wnonnull for constant null pointers on
//paths where the count is zero and the call can not run: a false positive.
#if defined(BOOST_GCC) && (BOOST_GCC >= 40600)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
#endif

template<class Traits, class CharT>
BOOST_CONTAINER_FORCEINLINE void string_traits_copy(CharT* dst, const CharT* src, std::size_t n)
{
   BOOST_ASSERT((dst && src) || !n);
   if (BOOST_LIKELY(n != 0))
      Traits::copy(dst, src, n);
}

template<class Traits, class CharT>
BOOST_CONTAINER_FORCEINLINE void string_traits_move(CharT* dst, const CharT* src, std::size_t n)
{
   BOOST_ASSERT((dst && src) || !n);
   if (BOOST_LIKELY(n != 0))
      Traits::move(dst, src, n);
}

template<class Traits, class CharT>
BOOST_CONTAINER_FORCEINLINE int string_traits_compare(const CharT* a, const CharT* b, std::size_t n)
{
   BOOST_ASSERT((a && b) || !n);
   if (BOOST_LIKELY(n != 0))
      return Traits::compare(a, b, n);
   return 0;
}

#if defined(BOOST_GCC) && (BOOST_GCC >= 40600)
#pragma GCC diagnostic pop
#endif

// ------------------------------------------------------------
// Class basic_string_base.

// basic_string_base is a helper class that makes it it easier to write
// an exception-safe version of basic_string.  The constructor allocates,
// but does not initialize, a block of memory.  The destructor
// deallocates, but does not destroy elements within, a block of
// memory. The destructor assumes that the memory either is the internal buffer,
// or else points to a block of memory that was allocated using string_base's
// allocator and whose size is this->m_storage.
template <class Allocator, class Options>
class basic_string_base
{
   basic_string_base & operator=(const basic_string_base &);
   basic_string_base(const basic_string_base &);

   typedef Allocator allocator_type;
 public:
   typedef allocator_traits<allocator_type>                 allocator_traits_type;
   typedef allocator_type                                   stored_allocator_type;
   typedef typename allocator_traits_type::pointer          pointer;
   typedef typename allocator_traits_type::value_type       value_type;
   typedef typename allocator_traits_type::size_type        size_type;
   typedef typename allocator_traits_type::difference_type  difference_type;
   typedef typename boost::container::
      allocator_traits<allocator_type>::size_type           alloc_size_type;
   typedef typename get_string_opt
      <Options, alloc_size_type>::type                      options_type;
   typedef typename options_type::growth_factor_type        growth_factor_type;
   typedef typename options_type::stored_size_type          stored_size_type;
   BOOST_CONTAINER_STATIC_ASSERT_MSG( (stored_size_type(-1) > stored_size_type(0))
                                    , "stored_size must be an unsigned integer type");

   typedef ::boost::intrusive::pointer_traits<pointer> pointer_traits;

   static const std::size_t inline_chars = options_type::inline_chars;

   inline basic_string_base()
      : members_()
   {}

   inline explicit basic_string_base(const allocator_type& a)
      : members_(a)
   {}

   inline explicit basic_string_base(BOOST_RV_REF(allocator_type) a)
      :  members_(boost::move(a))
   {}

   inline basic_string_base(const allocator_type& a, size_type n)
      : members_(a)
   {
      this->allocate_initial_block(n);
   }

   inline explicit basic_string_base(size_type n)
      : members_()
   {
      this->allocate_initial_block(n);
   }

   inline ~basic_string_base()
   {
      if(!this->is_short()){
         this->deallocate(this->priv_long_addr(), this->priv_long_storage());
      }
   }

   private:

   #if defined(BOOST_GCC) && (BOOST_GCC >= 40600)
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
   #endif

   //This is the structure controlling a long string
   struct long_t
   {
      stored_size_type  is_short  : 1;
      stored_size_type  length    : (sizeof(stored_size_type)*CHAR_BIT - 1);
      stored_size_type  storage;
      pointer           start;

      inline long_t()
         : is_short(0)
      {}

      inline long_t(size_type len, size_type stor, pointer ptr)
         : is_short(0), length(len), storage(stor), start(ptr)
      {}

      inline long_t(const long_t &other)
      {
         this->is_short = false;
         length   = other.length;
         storage  = other.storage;
         start    = other.start;
      }

      inline long_t &operator= (const long_t &other)
      {
         length   = other.length;
         storage  = other.storage;
         start    = other.start;
         return *this;
      }
   };

   #if defined(BOOST_GCC) && (BOOST_GCC >= 40600)
   #pragma GCC diagnostic pop
   #endif


   //This type is the first part of the structure controlling a short string
   //The "data" member stores
   struct short_header
   {
      unsigned char  is_short  : 1;
      unsigned char  length    : (CHAR_BIT - 1);
   };

   //This type has the same alignment and size as long_t but it's POD
   //so, unlike long_t, it can be placed in a union
   typedef typename dtl::aligned_storage
      <sizeof(long_t), dtl::alignment_of<long_t>::value>::type   long_raw_t;

   template <std::size_t NChar>
   union short_type_test
   {
      long_raw_t l;
      struct short_t
      {
         short_header   h;
         value_type     data[NChar+1u];
      } s;
   };

   protected:
   //Due to internal data representation constraints, inline chars cannot be bigger than this
   BOOST_STATIC_CONSTEXPR size_type  MaxInlineChars = 127u;
   BOOST_CONTAINER_STATIC_ASSERT(inline_chars <= MaxInlineChars);

   BOOST_STATIC_CONSTEXPR size_type  SizeOfValueType = sizeof(value_type);
   BOOST_STATIC_CONSTEXPR size_type  ShortDataOffset = ((sizeof(short_header)-1)/SizeOfValueType+1)*SizeOfValueType;
   BOOST_STATIC_CONSTEXPR size_type  ZeroCostInternalStorage = (sizeof(long_t) - ShortDataOffset)/sizeof(value_type);
   BOOST_STATIC_CONSTEXPR size_type  InlineCharsStorage = (sizeof(short_type_test<inline_chars>) - ShortDataOffset)/sizeof(value_type);

   //Select the biggest internal buffer between zero-cost SSO and user-requested SSO
   BOOST_STATIC_CONSTEXPR size_type  CandidateInternalStorageChars =
      ZeroCostInternalStorage > InlineCharsStorage ? ZeroCostInternalStorage : InlineCharsStorage;

   //Clamp internal buffer chars to MaxInlineChars
   BOOST_STATIC_CONSTEXPR size_type  MaxInlineStorage = MaxInlineChars+1u;
   BOOST_STATIC_CONSTEXPR size_type  FinalInternalStorage =
      CandidateInternalStorageChars < MaxInlineStorage ? CandidateInternalStorageChars : MaxInlineStorage;

   struct short_t
   {
      short_header   h;
      value_type     data[FinalInternalStorage];
   };

   union repr_t_size_t
   {
      long_raw_t  r;
      short_t     s;
   };

   union repr_t
   {
      long_raw_t  r_aligner;
      short_t     s_aligner;
      unsigned char data[sizeof(repr_t_size_t)];
   };

   struct members_holder
      :  public allocator_type
   {
      inline void init()
      {
         short_t &s = *::new(&this->m_repr, boost_container_new_t()) short_t;
         s.h.is_short = 1;
         s.h.length = 0;
      }

      inline members_holder()
         : allocator_type()
      { this->init(); }

      template<class AllocatorConvertible>
      inline explicit members_holder(BOOST_FWD_REF(AllocatorConvertible) a)
         :  allocator_type(boost::forward<AllocatorConvertible>(a))
      { this->init(); }

      inline const short_t * pshort_repr() const
      {  return move_detail::launder_cast<const short_t*>(&m_repr);  }

      inline const long_t * plong_repr() const
      {  return move_detail::launder_cast<const long_t*>(&m_repr);  }

      inline short_t * pshort_repr()
      {  return move_detail::launder_cast<short_t*>(&m_repr);  }

      inline long_t * plong_repr()
      {  return move_detail::launder_cast<long_t*>(&m_repr);  }

      repr_t m_repr;
   } members_;

   inline const allocator_type &alloc() const
   {  return members_;  }

   inline allocator_type &alloc()
   {  return members_;  }

   BOOST_STATIC_CONSTEXPR size_type InternalBufferChars = FinalInternalStorage;

   //The internal buffer (inline_chars) must not be bigger than the biggest dynamic buffer that
   //stored_size can represent (one bit of stored_size is used for the short/long flag)
   BOOST_CONTAINER_STATIC_ASSERT_MSG
      ( (InternalBufferChars <= (size_type(stored_size_type(-1)) >> 1u))
      , "inline_chars is too big for the stored_size option");

   private:

   BOOST_STATIC_CONSTEXPR size_type MinAllocation = InternalBufferChars*2;

   protected:
   inline bool is_short() const
   {
      //Access and copy (to avoid UB) the first byte of the union to know if the
      //active representation is short or long
      short_header hdr;
      BOOST_CONTAINER_STATIC_ASSERT((sizeof(short_header) == 1));
      *(unsigned char*)&hdr = *static_cast<const unsigned char*>(static_cast<const void *>(&this->members_.m_repr));
      return hdr.is_short != 0;
   }

   inline short_t * construct_short()
   {
      short_t *ps = ::new(&this->members_.m_repr, boost_container_new_t()) short_t;
      ps->h.is_short = 1;
      return ps;
   }

   inline void destroy_short()
   {
      BOOST_ASSERT(this->is_short());
      this->members_.pshort_repr()->~short_t();
   }

   //Precondition: !is_short(). Frees the dynamic buffer and changes to the short representation.
   //The size and the characters of the short representation are not initialized.
   void priv_long_to_short()
   {
      this->deallocate(this->priv_long_addr(), this->priv_long_storage());
      this->destroy_long();
      this->construct_short();
   }

   inline long_t * construct_long()
   {
      long_t *pl = ::new(&this->members_.m_repr, boost_container_new_t()) long_t;
      //is_short flag is written in the constructor
      return pl;
   }

   inline void destroy_long()
   {
      BOOST_ASSERT(!this->is_short());
      this->members_.plong_repr()->~long_t();
   }

   //Stores a new dynamic buffer. If was_short (the value of is_short() read by the caller), changes
   //to the long representation, else frees the old dynamic buffer.
   void priv_set_long_buffer(bool was_short, pointer addr, size_type sz, size_type storage)
   {
      BOOST_ASSERT(was_short == this->is_short());
      if (was_short){
         this->destroy_short();
         this->construct_long();
      }
      else{
         this->deallocate(this->priv_long_addr(), this->priv_long_storage());
      }
      this->priv_long_addr(addr);
      this->priv_long_size(sz);
      this->priv_long_storage(storage);
   }

   protected:

   typedef dtl::integral_constant<unsigned,
      boost::container::dtl::version<allocator_type>::value> alloc_version;

   pointer allocation_command(allocation_type command,
                         size_type limit_size,
                         size_type &prefer_in_recvd_out_size,
                         pointer &reuse)
   {
      //Test the command first: if it has no expansion bits (often a constant), is_short() is not read
      if((command & (expand_fwd | expand_bwd)) && this->is_short()){
         reuse = 0;
         command &= ~(expand_fwd | expand_bwd);
      }
      return this->priv_allocation_command(alloc_version(), command, limit_size, prefer_in_recvd_out_size, reuse);
   }

   pointer priv_allocation_command(version_1, allocation_type command,
                         size_type,
                         size_type &prefer_in_recvd_out_size,
                         pointer &reuse)
   {
      (void)command;
      BOOST_ASSERT( (command & allocate_new));
      BOOST_ASSERT(!(command & nothrow_allocation));
      pointer const p = this->alloc().allocate(prefer_in_recvd_out_size);
      reuse = pointer();
      return p;
   }

   pointer priv_allocation_command(version_2, allocation_type command,
                         size_type limit_size,
                         size_type &prefer_in_recvd_out_size,
                         pointer &reuse)
   {
      pointer const p = this->alloc().allocation_command(command, limit_size, prefer_in_recvd_out_size, reuse);
      //The received size can be bigger than requested: never store a storage bigger than the maximum stored_size
      const size_type max_storage = this->priv_max_storage();
      if (prefer_in_recvd_out_size > max_storage)
         prefer_in_recvd_out_size = max_storage;
      return p;
   }

   size_type priv_max_storage() const
   {
      size_type max_storage = allocator_traits_type::max_size(this->alloc());
      //We need a bit to store the short/ong bit so we can only use half of the stored_size_type capacity
      (clamp_by_half_stored_size_type<size_type>)(max_storage, stored_size_type());
      return max_storage;
   }

   static size_type next_storage
      (size_type additional_objects, size_type current_size, size_type current_storage, size_type max_storage)
   {
      const size_type cur_cap = size_type(current_storage - 1u);
      const size_type spare_cap = size_type(cur_cap - current_size);
      BOOST_ASSERT(additional_objects > spare_cap);
      const size_type max_cap = max_storage - 1u;  //we need to save space for null terminator
      const size_type min_additional_obj = size_type(additional_objects - spare_cap);

      //cur_cap is bigger than max_cap only if the allocator max_size is smaller than the current buffer
      if ( BOOST_UNLIKELY(cur_cap > max_cap || size_type(max_cap - cur_cap) < min_additional_obj) )
         boost::container::throw_length_error("get_next_capacity, allocator or storage_size_type's max size reached");

      return growth_factor_type()
            ( current_storage, min_additional_obj, max_storage);
   }

   void deallocate(pointer p, size_type n)
   {
      if (p && (n > InternalBufferChars))
         this->alloc().deallocate(p, n);
   }

   void destroy(pointer p, size_type n)
   {
      value_type *raw_p = boost::movelib::to_raw_pointer(p);
      for(; n--; ++raw_p){
         allocator_traits_type::destroy( this->alloc(), raw_p);
      }
   }

   inline void destroy(pointer p)
   {
      allocator_traits_type::destroy
         ( this->alloc()
         , boost::movelib::to_raw_pointer(p)
         );
   }

   //n is the number of characters, without the null terminator
   void allocate_initial_block(size_type n)
   {
      if (n <= this->max_size()) {
         if(n >= InternalBufferChars){
            size_type new_storage = n + 1u;
            pointer reuse = 0;
            pointer p = this->allocation_command(allocate_new, n, new_storage, reuse);
            BOOST_ASSERT(this->is_short());
            this->construct_long();
            this->priv_long_addr(p);
            this->priv_long_size(0);
            this->priv_long_storage(new_storage);
         }
      }
      else {
         throw_length_error("basic_string::allocate_initial_block max_size() exceeded");
      }
   }

   //The allocator and the stored_size limits, minus the null terminator. The internal
   //buffer does not use the allocator, so it is always available.
   inline size_type max_size() const
   {
      const size_type max_storage = this->priv_max_storage();
      return size_type((max_storage > InternalBufferChars ? max_storage : InternalBufferChars) - 1u);
   }

   protected:
   inline size_type priv_capacity() const
   { return this->priv_storage() - 1; }

   inline pointer priv_short_addr() const
   {  return pointer_traits::pointer_to(const_cast<value_type&>(this->members_.pshort_repr()->data[0]));  }

   //GCC seems a bit confused about uninitialized accesses
   #if defined(BOOST_GCC) && (BOOST_GCC >= 40700)
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
   #endif

   inline pointer priv_long_addr() const
   {  return this->members_.plong_repr()->start;  }

   inline pointer priv_addr() const
   {
      return this->is_short()
         ? priv_short_addr()
         : priv_long_addr()
         ;
   }

   inline pointer priv_end_addr() const
   {
      return this->is_short()
         ? this->priv_short_addr() + difference_type(this->priv_short_size())
         : this->priv_long_addr()  + difference_type(this->priv_long_size())
         ;
   }

   inline void priv_long_addr(pointer addr)
   {  this->members_.plong_repr()->start = addr;  }

   inline size_type priv_storage() const
   {  return this->is_short() ? priv_short_storage() : priv_long_storage();  }

   inline size_type priv_short_storage() const
   {  return InternalBufferChars;  }

   inline size_type priv_long_storage() const
   {  return this->members_.plong_repr()->storage;  }

   inline void priv_long_storage(size_type storage)
   {
      this->members_.plong_repr()->storage = static_cast<stored_size_type>(storage);
   }

   inline size_type priv_size() const
   {  return this->is_short() ? this->priv_short_size() : this->priv_long_size();  }

   inline size_type priv_short_size() const
   {  return this->members_.pshort_repr()->h.length;  }

   inline size_type priv_long_size() const
   {  return this->members_.plong_repr()->length;  }

   //Sets the size when the caller knows the representation
   inline void priv_size(bool short_repr, size_type sz)
   {
      if(short_repr)
         this->priv_short_size(sz);
      else
         this->priv_long_size(sz);
   }

   //Reads the address, size and storage of the buffer with only one is_short() test.
   //Returns is_short().
   BOOST_CONTAINER_FORCEINLINE bool priv_repr_data(pointer &addr, size_type &sz, size_type &storage) const
   {
      if(this->is_short()){
         addr    = this->priv_short_addr();
         sz      = this->priv_short_size();
         storage = this->priv_short_storage();
         return true;
      }
      else{
         addr    = this->priv_long_addr();
         sz      = this->priv_long_size();
         storage = this->priv_long_storage();
         return false;
      }
   }

   inline void priv_short_size(size_type sz)
   {
      typedef unsigned char uchar_type;
      BOOST_ASSERT(sz < uchar_type(-1));
      BOOST_STATIC_CONSTEXPR uchar_type mask = uchar_type(uchar_type(-1) >> 1U);
      BOOST_ASSERT( sz <= mask );
      //Make -Wconversion happy
      this->members_.pshort_repr()->h.length = uchar_type(uchar_type(sz) & mask);
   }

   inline void priv_long_size(size_type sz)
   {
      BOOST_STATIC_CONSTEXPR stored_size_type mask = static_cast<stored_size_type>(stored_size_type(-1) >> 1U);
      BOOST_ASSERT( sz <= mask );
      //Make -Wconversion happy
      this->members_.plong_repr()->length = static_cast<stored_size_type>(sz & mask);
   }
   #if defined(BOOST_GCC) && (BOOST_GCC >= 40700)
   #pragma GCC diagnostic pop
   #endif

   void swap_data(basic_string_base& other)
   {
      if(this->is_short()){
         if(other.is_short()){
            repr_t tmp(this->members_.m_repr);
            this->members_.m_repr = other.members_.m_repr;
            other.members_.m_repr = tmp;
         }
         else{
            short_t short_backup(*this->members_.pshort_repr());
            this->members_.pshort_repr()->~short_t();
            ::new(this->members_.plong_repr(), boost_container_new_t()) long_t(*other.members_.plong_repr());
            other.members_.plong_repr()->~long_t();
            ::new(other.members_.pshort_repr(), boost_container_new_t()) short_t(short_backup);
         }
      }
      else{
         if(other.is_short()){
            short_t short_backup(*other.members_.pshort_repr());
            other.members_.pshort_repr()->~short_t();
            ::new(other.members_.plong_repr(), boost_container_new_t()) long_t(*this->members_.plong_repr());
            this->members_.plong_repr()->~long_t();
            ::new(this->members_.pshort_repr(), boost_container_new_t()) short_t(short_backup);
         }
         else{
            boost::adl_move_swap(*this->members_.plong_repr(), *other.members_.plong_repr());
         }
      }
   }
};

}  //namespace dtl {

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//! The basic_string class represents a Sequence of characters. It contains all the
//! usual operations of a Sequence, and, additionally, it contains standard string
//! operations such as search and concatenation.
//!
//! The basic_string class is parameterized by character type, and by that type's
//! Character Traits.
//!
//! This class has performance characteristics very much like vector<>, meaning,
//! for example, that it does not perform reference-count or copy-on-write, and that
//! concatenation of two strings is an O(N) operation.
//!
//! Some of basic_string's member functions use an unusual method of specifying positions
//! and ranges. In addition to the conventional method using iterators, many of
//! basic_string's member functions use a single value pos of type size_type to represent a
//! position (in which case the position is begin() + pos, and many of basic_string's
//! member functions use two values, pos and n, to represent a range. In that case pos is
//! the beginning of the range and n is its size. That is, the range is
//! [begin() + pos, begin() + pos + n).
//!
//! Note that the C++ standard does not specify the complexity of basic_string operations.
//! In this implementation, basic_string has performance characteristics very similar to
//! those of vector: access to a single character is O(1), while copy and concatenation
//! are O(N).
//!
//! In this implementation, begin(),
//! end(), rbegin(), rend(), operator[], c_str(), and data() do not invalidate iterators.
//! In this implementation, iterators are only invalidated by member functions that
//! explicitly change the string's contents.
//!
//! \tparam CharT The type of character it contains.
//! \tparam Traits The Character Traits type, which encapsulates basic character operations
//! \tparam Allocator The allocator, used for internal memory management. Use void
//!   for the default allocator. An allocator whose \c value_type is \c void
//!   is rebound to \c CharT.
//! \tparam Options A type produced from \c boost::container::string_options.
#ifdef BOOST_CONTAINER_DOXYGEN_INVOKED
template <class CharT, class Traits = std::char_traits<CharT>, class Allocator = void, class Options = void >
#else
template <class CharT, class Traits, class Allocator, class Options>
#endif
class basic_string
   :  private dtl::basic_string_base<typename real_allocator<CharT, Allocator>::type, Options>
{
   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   private:
   BOOST_COPYABLE_AND_MOVABLE(basic_string)
   typedef dtl::basic_string_base<typename real_allocator<CharT, Allocator>::type, Options> base_t;
   typedef typename base_t::allocator_traits_type allocator_traits_type;

   BOOST_STATIC_CONSTEXPR typename base_t::size_type InternalBufferChars = base_t::InternalBufferChars;

   private:
   // Allocator helper class to use a char_traits as a function object.

   template <class Tr>
   struct Eq_traits
   {
      //Compatibility with std::binary_function
      typedef typename Tr::char_type   first_argument_type;
      typedef typename Tr::char_type   second_argument_type;
      typedef bool   result_type;

      bool operator()(const first_argument_type& x, const second_argument_type& y) const
         { return Tr::eq(x, y); }
   };

   template <class Tr>
   struct Not_within_traits
   {
      typedef typename Tr::char_type   argument_type;
      typedef bool                     result_type;

      typedef const typename Tr::char_type* Pointer;
      const Pointer m_first;
      const Pointer m_last;

      Not_within_traits(Pointer f, Pointer l)
         : m_first(f), m_last(l) {}

      bool operator()(const typename Tr::char_type& x) const
      {
         return boost::container::find_if(m_first, m_last,
                        boost::container::bind1st(Eq_traits<Tr>(), x)) == m_last;
      }
   };
   #endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   public:
   //////////////////////////////////////////////
   //
   //                    types
   //
   //////////////////////////////////////////////
   typedef Traits                                                                         traits_type;
   typedef CharT                                                                          value_type;
   typedef typename real_allocator<CharT, Allocator>::type                                allocator_type;
   typedef typename boost::container::allocator_traits<allocator_type>::pointer         pointer;
   typedef typename boost::container::allocator_traits<allocator_type>::const_pointer   const_pointer;
   typedef typename boost::container::allocator_traits<allocator_type>::reference       reference;
   typedef typename boost::container::allocator_traits<allocator_type>::const_reference const_reference;
   typedef typename boost::container::allocator_traits<allocator_type>::size_type       size_type;
   typedef typename boost::container::allocator_traits<allocator_type>::difference_type difference_type;
   typedef BOOST_CONTAINER_IMPDEF(allocator_type)                                         stored_allocator_type;
   typedef BOOST_CONTAINER_IMPDEF(pointer)                                                iterator;
   typedef BOOST_CONTAINER_IMPDEF(const_pointer)                                          const_iterator;
   typedef BOOST_CONTAINER_IMPDEF(boost::container::reverse_iterator<iterator>)           reverse_iterator;
   typedef BOOST_CONTAINER_IMPDEF(boost::container::reverse_iterator<const_iterator>)     const_reverse_iterator;
   static const size_type npos = size_type(-1);

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   typedef typename base_t::alloc_size_type                                               alloc_size_type;
   typedef typename base_t::options_type                                                  options_type;
   typedef typename base_t::growth_factor_type                                            growth_factor_type;
   typedef typename base_t::stored_size_type                                              stored_size_type;

   private:

   //`allocator_type::value_type` must match container's `value type`. If this
   //assertion fails, please review your allocator definition. 
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<value_type, typename allocator_traits<allocator_type>::value_type>::value));

   typedef constant_iterator<CharT> cvalue_iterator;
   typedef typename base_t::alloc_version  alloc_version;
   typedef ::boost::intrusive::pointer_traits<pointer> pointer_traits;
   #endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   public:                         // Constructor, destructor, assignment.
   //////////////////////////////////////////////
   //
   //          construct/copy/destroy
   //
   //////////////////////////////////////////////
   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   struct reserve_t {};

   basic_string(reserve_t, size_type n,
                const allocator_type& a = allocator_type())
      //Select allocator as in copy constructor as reserve_t-based constructors
      //are two step copies optimized for capacity
      : base_t( allocator_traits_type::select_on_container_copy_construction(a)
              , n)
   { this->priv_terminate_string(); }

   #if !defined( BOOST_NO_CXX11_NULLPTR )
   BOOST_DELETED_FUNCTION(basic_string( decltype(nullptr) ))
   public:
   #endif   //!defined( BOOST_NO_CXX11_NULLPTR )

   #endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

   //! <b>Effects</b>: Default constructs a basic_string.
   //!
   //! <b>Throws</b>: If allocator_type's default constructor throws.
   basic_string() BOOST_NOEXCEPT_IF(dtl::is_nothrow_default_constructible<allocator_type>::value)
      : base_t()
   { this->priv_short_terminate(); }


   //! <b>Effects</b>: Constructs a basic_string taking the allocator as parameter.
   //!
   //! <b>Throws</b>: Nothing
   explicit basic_string(const allocator_type& a) BOOST_NOEXCEPT_OR_NOTHROW
      : base_t(a)
   { this->priv_short_terminate(); }

   //! <b>Effects</b>: Copy constructs a basic_string.
   //!
   //! <b>Postcondition</b>: x == *this.
   //!
   //! <b>Throws</b>: If allocator_type's default constructor or allocation throws.
   basic_string(const basic_string& s)
      :  base_t(allocator_traits_type::select_on_container_copy_construction(s.alloc()))
   {
      this->priv_short_terminate();
      this->assign(s.begin(), s.end());
   }

   //! <b>Effects</b>: Same as basic_string(sv.data(), sv.size(), a).
   //!
   //! <b>Throws</b>: If allocator_type's default constructor or allocation throws.
   template<template <class, class> class BasicStringView>
   explicit basic_string(BasicStringView<CharT, Traits> sv, const allocator_type& a = allocator_type())
      :  base_t(allocator_traits_type::select_on_container_copy_construction(a))
   {
      this->priv_short_terminate();
      this->assign(sv);
   }

   //! <b>Effects</b>: Move constructor. Moves s's resources to *this.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   basic_string(BOOST_RV_REF(basic_string) s) BOOST_NOEXCEPT_OR_NOTHROW
      : base_t(boost::move(s.alloc()))
   {
      this->priv_short_terminate();
      if(s.alloc() == this->alloc()){
         this->swap_data(s);
      }
      else{
         this->assign(s.begin(), s.end());
      }
   }

   //! <b>Effects</b>: Copy constructs a basic_string using the specified allocator.
   //!
   //! <b>Postcondition</b>: x == *this.
   //!
   //! <b>Throws</b>: If allocation throws.
   basic_string(const basic_string& s, const BOOST_CONTAINER_DOC1ST(allocator_type, typename dtl::type_identity<allocator_type>::type) &a)
      :  base_t(a)
   {
      this->priv_short_terminate();
      this->assign(s.begin(), s.end());
   }

   //! <b>Effects</b>: Move constructor using the specified allocator.
   //!                 Moves s's resources to *this.
   //!
   //! <b>Throws</b>: Nothing if allocator_traits_type::is_always_equal::value is true.
   //!   Otherwise, if allocation throws.
   //!
   //! <b>Complexity</b>: Constant if a == s.get_allocator(), linear otherwise.
   basic_string(BOOST_RV_REF(basic_string) s, const BOOST_CONTAINER_DOC1ST(allocator_type, typename dtl::type_identity<allocator_type>::type) &a)
      BOOST_NOEXCEPT_IF(allocator_traits_type::is_always_equal::value)
      : base_t(a)
   {
      this->priv_short_terminate();
      if(s.alloc() == this->alloc()){
         this->swap_data(s);
      }
      else{
         this->assign(s.begin(), s.end());
      }
   }

   //! <b>Effects</b>: Constructs a basic_string with a default-constructed allocator,
   //!   and is initialized by a specific number of characters of the s string.
   basic_string(const basic_string& s, size_type pos, size_type n = npos)
      : base_t()
   {
      this->priv_short_terminate();
      if (pos > s.size())
         throw_out_of_range("basic_string::basic_string out of range position");
      else
         this->assign
            (s.begin() + pos, s.begin() + pos + dtl::min_value(n, s.size() - pos));
   }

   //! <b>Effects</b>: Constructs a basic_string taking the allocator as parameter,
   //!   and is initialized by a specific number of characters of the s string.
   basic_string(const basic_string& s, size_type pos, size_type n, const allocator_type& a)
      : base_t(a)
   {
      this->priv_short_terminate();
      if (pos > s.size())
         throw_out_of_range("basic_string::basic_string out of range position");
      else
         this->assign
            (s.begin() + pos, s.begin() + pos + dtl::min_value(n, s.size() - pos));
   }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT.
   //!   If n is zero, s may be null.
   //!
   //! <b>Effects</b>: Constructs a basic_string taking a default-constructed allocator,
   //!   and is initialized by the first n characters of the array pointed to by s.
   basic_string(const CharT* s, size_type n)
      : base_t()
   {
      this->priv_short_terminate();
      this->assign(s, n);  // assign checks for s != 0 when n != 0
   }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT.
   //!   If n is zero, s may be null.
   //!
   //! <b>Effects</b>: Constructs a basic_string taking the allocator as parameter,
   //!   and is initialized by the first n characters of the array pointed to by s.
   basic_string(const CharT* s, size_type n, const allocator_type& a)
      : base_t(a)
   {
      this->priv_short_terminate();
      this->assign(s, n);  // assign checks for s != 0 when n != 0
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Effects</b>: Constructs a basic_string with a default-constructed allocator,
   //!   and is initialized by the null-terminated s c-string.
   basic_string(const CharT* s)
      : base_t()
   {
      BOOST_ASSERT(s != 0);
      this->priv_short_terminate();
      this->assign(s, s + Traits::length(s));
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Effects</b>: Constructs a basic_string taking the allocator as parameter,
   //!   and is initialized by the null-terminated s c-string.
   basic_string(const CharT* s, const allocator_type& a)
      : base_t(a)
   {
      BOOST_ASSERT(s != 0);
      this->priv_short_terminate();
      this->assign(s, s + Traits::length(s));
   }


   //! <b>Effects</b>: Constructs a basic_string with a default-constructed allocator,
   //!   and is initialized by n copies of c.
   basic_string(size_type n, CharT c)
      : base_t()
   {
      this->priv_short_terminate();
      this->assign(n, c);
   }

   //! <b>Effects</b>: Constructs a basic_string taking the allocator as parameter,
   //!   and is initialized by n copies of c.
   basic_string(size_type n, CharT c, const allocator_type& a)
      : base_t(a)
   {
      this->priv_short_terminate();
      this->assign(n, c);
   }

   //! <b>Effects</b>: Constructs a basic_string with a default-constructed allocator,
   //!   and is initialized by n default-initialized characters.
   basic_string(size_type n, default_init_t)
      : base_t(n)
   {
      this->priv_size_and_terminate(n);
   }

   //! <b>Effects</b>: Constructs a basic_string taking the allocator as parameter,
   //!   and is initialized by n default-initialized characters.
   basic_string(size_type n, default_init_t, const allocator_type& a)
      : base_t(a, n)
   {
      this->priv_size_and_terminate(n);
   }

   //! <b>Effects</b>: Constructs a basic_string with a default-constructed allocator,
   //!   and a range of iterators.
   template <class InputIterator>
   basic_string(InputIterator f, InputIterator l
      BOOST_CONTAINER_DOCIGN(BOOST_MOVE_I typename dtl::disable_if_c
         < dtl::is_convertible<InputIterator BOOST_MOVE_I size_type>::value
         BOOST_MOVE_I dtl::nat >::type * = 0))
      : base_t()
   {
      this->priv_short_terminate();
      this->assign(f, l);
   }

   //! <b>Effects</b>: Constructs a basic_string taking the allocator as parameter,
   //!   and a range of iterators.
   template <class InputIterator>
   basic_string(InputIterator f, InputIterator l, const allocator_type& a
      BOOST_CONTAINER_DOCIGN(BOOST_MOVE_I typename dtl::disable_if_c
         < dtl::is_convertible<InputIterator BOOST_MOVE_I size_type>::value
         BOOST_MOVE_I dtl::nat >::type * = 0))
      : base_t(a)
   {
      this->priv_short_terminate();
      this->assign(f, l);
   }

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   //! <b>Effects</b>: Same as basic_string(il.begin(), il.end(), a).
   //!
   basic_string(std::initializer_list<value_type> il, const allocator_type& a = allocator_type())
      : base_t(a)
   {
      this->priv_short_terminate();
      this->assign(il.begin(), il.end());
   }
   #endif

   //! <b>Effects</b>: Destroys the basic_string. All used memory is deallocated.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   ~basic_string() BOOST_NOEXCEPT_OR_NOTHROW
   {}

   //! <b>Effects</b>: Copy constructs a string.
   //!
   //! <b>Postcondition</b>: x == *this.
   //!
   //! <b>Complexity</b>: Linear to the elements x contains.
   basic_string& operator=(BOOST_COPY_ASSIGN_REF(basic_string) x)
   {
      if (BOOST_LIKELY(this != &x)) {
         allocator_type &this_alloc     = this->alloc();
         const allocator_type &x_alloc  = x.alloc();
         dtl::bool_<allocator_traits_type::
            propagate_on_container_copy_assignment::value> flag;
         if(flag && this_alloc != x_alloc){
            if(!this->is_short()){
               this->priv_long_to_short();
               Traits::assign(*this->priv_short_addr(), CharT(0));
               this->priv_short_size(0);
            }
         }
         dtl::assign_alloc(this->alloc(), x.alloc(), flag);
         this->assign(x.begin(), x.end());
      }
      return *this;
   }

   //! <b>Effects</b>: Move constructor. Moves x's resources to *this.
   //!
   //! <b>Throws</b>: If allocator_traits_type::propagate_on_container_move_assignment
   //!   is false and allocation throws
   //!
   //! <b>Complexity</b>: Constant if allocator_traits_type::
   //!   propagate_on_container_move_assignment is true or
   //!   this->get>allocator() == x.get_allocator(). Linear otherwise.
   basic_string& operator=(BOOST_RV_REF(basic_string) x)
      BOOST_NOEXCEPT_IF(allocator_traits_type::propagate_on_container_move_assignment::value
                                  || allocator_traits_type::is_always_equal::value)
   {
      if (BOOST_LIKELY(this != &x)) {
         //We know resources can be transferred at comiple time if both allocators are
         //always equal or the allocator is going to be propagated
         const bool can_steal_resources_alloc
            =  allocator_traits_type::propagate_on_container_move_assignment::value
            || allocator_traits_type::is_always_equal::value;
         dtl::bool_<can_steal_resources_alloc> flag;
         this->priv_move_assign(boost::move(x), flag);
      }
      return *this;
   }

   //! <b>Effects</b>: Assignment from a null-terminated c-string.
   //!
   basic_string& operator=(const CharT* s)
   {
      return this->assign(s); // assign checks for s != 0
   }

   //! <b>Effects</b>: Returns *this = basic_string(1, c).
   //!
   basic_string& operator=(CharT c)
   { return this->assign(static_cast<size_type>(1), c); }

   //! <b>Effects</b>: Equivalent to return assign(sv).
   //!
   template<template <class, class> class BasicStringView>
   basic_string& operator=(BasicStringView<CharT, Traits> sv)
   { return this->assign(sv.data(), sv.size()); }

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   //! <b>Effects</b>: Returns *this = basic_string(il);
   //!
   basic_string& operator=(std::initializer_list<CharT> il)
   {
      return this->assign(il.begin(), il.end());
   }
   #endif

   //! <b>Effects</b>: Returns a copy of the internal allocator.
   //!
   //! <b>Throws</b>: If allocator's copy constructor throws.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      allocator_type get_allocator() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->alloc(); }

   //! <b>Effects</b>: Returns a reference to the internal allocator.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Complexity</b>: Constant.
   //!
   //! <b>Note</b>: Non-standard extension.
   BOOST_CONTAINER_NODISCARD inline
      stored_allocator_type &get_stored_allocator() BOOST_NOEXCEPT_OR_NOTHROW
   {  return this->alloc(); }

   //! <b>Effects</b>: Returns a reference to the internal allocator.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Complexity</b>: Constant.
   //!
   //! <b>Note</b>: Non-standard extension.
   BOOST_CONTAINER_NODISCARD inline
      const stored_allocator_type &get_stored_allocator() const BOOST_NOEXCEPT_OR_NOTHROW
   {  return this->alloc(); }

   //////////////////////////////////////////////
   //
   //                iterators
   //
   //////////////////////////////////////////////

   //! <b>Effects</b>: Returns an iterator to the first element contained in the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      iterator begin() BOOST_NOEXCEPT_OR_NOTHROW
   { return this->priv_addr(); }

   //! <b>Effects</b>: Returns a const_iterator to the first element contained in the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_iterator begin() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->priv_addr(); }

   //! <b>Effects</b>: Returns an iterator to the end of the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      iterator end() BOOST_NOEXCEPT_OR_NOTHROW
   { return this->priv_end_addr(); }

   //! <b>Effects</b>: Returns a const_iterator to the end of the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_iterator end() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->priv_end_addr(); }

   //! <b>Effects</b>: Returns a reverse_iterator pointing to the beginning
   //! of the reversed vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      reverse_iterator rbegin()  BOOST_NOEXCEPT_OR_NOTHROW
   { return reverse_iterator(this->priv_end_addr()); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the beginning
   //! of the reversed vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_reverse_iterator rbegin() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->crbegin(); }

   //! <b>Effects</b>: Returns a reverse_iterator pointing to the end
   //! of the reversed vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      reverse_iterator rend()  BOOST_NOEXCEPT_OR_NOTHROW
   { return reverse_iterator(this->priv_addr()); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the end
   //! of the reversed vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_reverse_iterator rend() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->crend(); }

   //! <b>Effects</b>: Returns a const_iterator to the first element contained in the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_iterator cbegin() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->priv_addr(); }

   //! <b>Effects</b>: Returns a const_iterator to the end of the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_iterator cend() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->priv_end_addr(); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the beginning
   //! of the reversed vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_reverse_iterator crbegin() const BOOST_NOEXCEPT_OR_NOTHROW
   { return const_reverse_iterator(this->priv_end_addr()); }

   //! <b>Effects</b>: Returns a const_reverse_iterator pointing to the end
   //! of the reversed vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_reverse_iterator crend() const BOOST_NOEXCEPT_OR_NOTHROW
   { return const_reverse_iterator(this->priv_addr()); }

   //////////////////////////////////////////////
   //
   //                capacity
   //
   //////////////////////////////////////////////

   //! <b>Effects</b>: Returns true if the vector contains no elements.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      bool empty() const BOOST_NOEXCEPT_OR_NOTHROW
   { return !this->priv_size(); }

   //! <b>Effects</b>: Returns the number of the elements contained in the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      size_type size() const    BOOST_NOEXCEPT_OR_NOTHROW
   { return this->priv_size(); }

   //! <b>Effects</b>: Returns the number of the elements contained in the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      size_type length() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->size(); }

   //! <b>Effects</b>: Returns the largest possible size of the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant
   BOOST_CONTAINER_NODISCARD inline
      size_type max_size() const BOOST_NOEXCEPT_OR_NOTHROW
   { return base_t::max_size(); }

   //! <b>Effects</b>: Inserts or erases elements at the end such that
   //!   the size becomes n. New elements are copy constructed from x.
   //!
   //! <b>Throws</b>: If memory allocation throws
   //!
   //! <b>Complexity</b>: Linear to the difference between size() and new_size.
   void resize(size_type n, CharT c)
   {
      if (n <= this->size())
         this->erase(this->begin() + difference_type(n), this->end());
      else
         this->append(n - this->size(), c);
   }

   //! <b>Effects</b>: Inserts or erases elements at the end such that
   //!   the size becomes n. New elements are value initialized.
   //!
   //! <b>Throws</b>: If memory allocation throws
   //!
   //! <b>Complexity</b>: Linear to the difference between size() and new_size.
   void resize(size_type n)
   { resize(n, CharT()); }

   //! <b>Effects</b>: Inserts or erases elements at the end such that
   //!   the size becomes n. New elements are uninitialized.
   //!
   //! <b>Throws</b>: If memory allocation throws
   //!
   //! <b>Complexity</b>: Linear to the difference between size() and new_size.
   //!
   //! <b>Note</b>: Non-standard extension
   void resize(size_type n, default_init_t)
   {
      if (n <= this->size())
         this->erase(this->begin() + difference_type(n), this->end());
      else{
         this->priv_reserve_no_null_end(n);
         this->priv_size_and_terminate(n);
      }
   }

   //! <b>Effects</b>: Number of elements for which memory has been allocated.
   //!   capacity() is always greater than or equal to size().
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      size_type capacity() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->priv_capacity(); }

   //! <b>Effects</b>: If n is less than or equal to capacity(), this call has no
   //!   effect. Otherwise, it is a request for allocation of additional memory.
   //!   If the request is successful, then capacity() is greater than or equal to
   //!   n; otherwise, capacity() is unchanged. In either case, size() is unchanged.
   //!
   //! <b>Throws</b>: If memory allocation allocation throws
   void reserve(size_type res_arg)
   {  this->priv_reserve(res_arg);  }

   //! <b>Effects</b>: Tries to deallocate the excess of memory created
   //!   with previous allocations. The size of the string is unchanged
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Complexity</b>: Linear to size().
   void shrink_to_fit()
   {
      //Check if shrinking is possible: only a dynamically allocated buffer can be shrunk
      if(!this->is_short()){
         //Dynamically allocated buffer attributes
         const size_type long_size = this->priv_long_size();
         //Check if we should pass from dynamically allocated buffer
         //to the internal storage
         if(long_size < InternalBufferChars){
            pointer   long_addr    = this->priv_long_addr();
            size_type long_storage = this->priv_long_storage();
            //Shrink from allocated buffer to the internal one, including trailing null
            Traits::copy( boost::movelib::to_raw_pointer(this->priv_short_addr())
                        , boost::movelib::to_raw_pointer(long_addr)
                        , long_size+1);
            BOOST_ASSERT(!this->is_short());
            this->destroy_long();
            this->construct_short();
            this->alloc().deallocate(long_addr, long_storage);
         }
         else{
            //Shrinking in dynamic buffer
            this->priv_shrink_to_fit_dynamic_buffer(alloc_version());
         }
      }
   }

   //////////////////////////////////////////////
   //
   //               element access
   //
   //////////////////////////////////////////////

   //! <b>Requires</b>: !empty()
   //!
   //! <b>Effects</b>: Returns a reference to the first
   //!   element of the container.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      reference         front() BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(!this->empty());
      return *this->priv_addr();
   }

   //! <b>Requires</b>: !empty()
   //!
   //! <b>Effects</b>: Returns a const reference to the first
   //!   element of the container.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_reference   front() const BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(!this->empty());
      return *this->priv_addr();
   }

   //! <b>Requires</b>: !empty()
   //!
   //! <b>Effects</b>: Returns a reference to the last
   //!   element of the container.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      reference         back() BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(!this->empty());
      return *(this->priv_addr() + (this->size() - 1u) );
   }

   //! <b>Requires</b>: !empty()
   //!
   //! <b>Effects</b>: Returns a const reference to the last
   //!   element of the container.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_reference   back()  const BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(!this->empty());
      return *(this->priv_addr() + (this->size() - 1u) );
   }

   //! <b>Requires</b>: size() >= n.
   //!
   //! <b>Effects</b>: Returns a reference to the nth element
   //!   from the beginning of the container.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      reference operator[](size_type n) BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(this->size() >= n);
      return *(this->priv_addr() + difference_type(n));
   }

   //! <b>Requires</b>: size() >= n.
   //!
   //! <b>Effects</b>: Returns a const reference to the nth element
   //!   from the beginning of the container.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_reference operator[](size_type n) const BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(this->size() >= n);
      return *(this->priv_addr() + difference_type(n));
   }

   //! <b>Requires</b>: size() > n.
   //!
   //! <b>Effects</b>: Returns a reference to the nth element
   //!   from the beginning of the container.
   //!
   //! <b>Throws</b>: range_error if n >= size()
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      reference at(size_type n)
   {
      if (n >= this->size())
         throw_out_of_range("basic_string::at invalid subscript");
      return *(this->priv_addr() + difference_type(n));
   }

   //! <b>Requires</b>: size() > n.
   //!
   //! <b>Effects</b>: Returns a const reference to the nth element
   //!   from the beginning of the container.
   //!
   //! <b>Throws</b>: range_error if n >= size()
   //!
   //! <b>Complexity</b>: Constant.
   BOOST_CONTAINER_NODISCARD inline
      const_reference at(size_type n) const {
      if (n >= this->size())
         throw_out_of_range("basic_string::at invalid subscript");
      return *(this->priv_addr() + difference_type(n));
   }

   //////////////////////////////////////////////
   //
   //                modifiers
   //
   //////////////////////////////////////////////

   //! <b>Effects</b>: Calls append(str.data, str.size()).
   //!
   //! <b>Returns</b>: *this
   basic_string& operator+=(const basic_string& s)
   {  return this->append(s); }

   //! <b>Effects</b>: Same as `return append(sv)`.
   //!
   template<template<class, class> class BasicStringView>
   basic_string& operator+=(BasicStringView<CharT, Traits> sv)
   {
      return this->append(sv);
   }

   //! <b>Effects</b>: Calls append(s).
   //!
   //! <b>Returns</b>: *this
   basic_string& operator+=(const CharT* s)
   {
      return this->append(s); //append checks for s != 0
   }

   //! <b>Effects</b>: Calls append(1, c).
   //!
   //! <b>Returns</b>: *this
   basic_string& operator+=(CharT c)
   {  this->push_back(c); return *this;   }

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   //! <b>Effects</b>: Returns append(il)
   //!
   basic_string& operator+=(std::initializer_list<CharT> il)
   {
      return this->append(il);
   }
   #endif

   //! <b>Effects</b>: Calls append(str.data(), str.size()).
   //!
   //! <b>Returns</b>: *this
   basic_string& append(const basic_string& s)
   {  return this->append(s.begin(), s.end());  }

   //! <b>Effects</b>: Same as return append(sv.data(), sv.size()).
   //!
   template<template<class, class> class BasicStringView>
   basic_string& append(BasicStringView<CharT, Traits> sv)
   {  return this->append(sv.data(), sv.size());  }

   //! <b>Requires</b>: pos <= str.size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to append
   //! as the smaller of n and str.size() - pos and calls append(str.data() + pos, rlen).
   //!
   //! <b>Throws</b>: If memory allocation throws and out_of_range if pos > str.size()
   //!
   //! <b>Returns</b>: *this
   basic_string& append(const basic_string& s, size_type pos, size_type n = npos)
   {
      if (pos > s.size())
         throw_out_of_range("basic_string::append out of range position");
      return this->append(s.begin() + pos,
                          s.begin() + pos + dtl::min_value(n, s.size() - pos));
   }

   //! <b>Requires</b>: pos <= sv.size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to append
   //! as the smaller of n and sv.size() - pos and calls append(sv.data() + pos, rlen).
   //!
   //! <b>Throws</b>: If memory allocation throws and out_of_range if pos > sv.size()
   //!
   //! <b>Returns</b>: *this
   template<template<class, class> class BasicStringView>   
   basic_string& append(BasicStringView<CharT, Traits> sv, size_type pos, size_type n = npos)
   {
      if (pos > sv.size())
         throw_out_of_range("basic_string::append out of range position");
      return this->append(sv.begin() + pos,
                          sv.begin() + pos + dtl::min_value(n, sv.size() - pos));
   }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT.
   //!   If n is zero, s may be null.
   //!
   //! <b>Effects</b>: The function replaces the string controlled by *this with
   //!   a string of length size() + n whose irst size() elements are a copy of the
   //!   original string controlled by *this and whose remaining
   //!   elements are a copy of the initial n elements of s.
   //!
   //! <b>Throws</b>: If memory allocation throws length_error if size() + n > max_size().
   //!
   //! <b>Returns</b>: *this
   basic_string& append(const CharT* s, size_type n)
   {
      BOOST_ASSERT(s != 0 || n == 0);
      return this->append(s, s + difference_type(n));
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Effects</b>: Calls append(s, traits::length(s)).
   //!
   //! <b>Returns</b>: *this
   basic_string& append(const CharT* s)
   {
      BOOST_ASSERT(s != 0);
      return this->append(s, s + Traits::length(s));
   }

   //! <b>Effects</b>: Equivalent to append(basic_string(n, c)).
   //!
   //! <b>Returns</b>: *this
   basic_string& append(size_type n, CharT c)
   {  return this->append(cvalue_iterator(c, n), cvalue_iterator()); }

   //! <b>Requires</b>: [first,last) is a valid range.
   //!
   //! <b>Effects</b>: Equivalent to append(basic_string(first, last)).
   //!
   //! <b>Throws</b>: If memory allocation throws, length_error if the new size is greater than
   //!   max_size(), or if an operation on the iterators throws.
   //!   If an exception is thrown, there are no effects (strong exception guarantee).
   //!
   //! <b>Returns</b>: *this
   template <class InputIter>
   basic_string& append(InputIter first, InputIter last)
   {  this->insert(this->end(), first, last);   return *this;  }

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   //! <b>Effects</b>: Returns append(il.begin(), il.end()).
   //!
   //! <b>Throws</b>: If memory allocation throws or length_error if the new size is greater than
   //!   max_size(). If an exception is thrown, there are no effects (strong exception guarantee).
   basic_string& append(std::initializer_list<CharT> il)
   {
      return this->append(il.begin(), il.end());
   }
   #endif

   //! <b>Effects</b>: Equivalent to append(static_cast<size_type>(1), c).
   //!
   void push_back(CharT c)
   {
      pointer   addr;
      size_type old_size, storage;
      const bool short_repr = this->priv_repr_data(addr, old_size, storage);
      if (old_size < size_type(storage - 1u)){
         this->priv_construct_null(addr + difference_type(old_size + 1u));
         Traits::assign(addr[difference_type(old_size)], c);
         this->priv_size(short_repr, old_size+1u);
      }
      else{
         //No enough memory, insert a new object at the end
         this->append(size_type(1), c);
      }
   }

   //! <b>Effects</b>: return *this = s;
   //!
   //! <b>Returns</b>: *this
   basic_string& assign(const basic_string& s)
   {  return this->operator=(s); }

   //! <b>Effects</b>: return *this = move(s);
   //!
   //! <b>Returns</b>: *this
   basic_string& assign(BOOST_RV_REF(basic_string) s) BOOST_NOEXCEPT_OR_NOTHROW
   {  return this->operator=(boost::move(s)); }

   //! <b>Effects</b>: Equivalent to return assign(sv.data(), sv.size()).
   //!
   //! <b>Returns</b>: *this
   template<template <class, class> class BasicStringView>
   basic_string& assign(BasicStringView<CharT, Traits> sv)
   {  return this->operator=(sv); }

   //! <b>Requires</b>: pos <= sv.size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to assign as
   //!   the smaller of n and sv.size() - pos and calls assign(sv.data() + pos rlen).
   //!
   //! <b>Throws</b>: If memory allocation throws or out_of_range if pos > sv.size().
   //!
   //! <b>Returns</b>: *this
   template<template <class, class> class BasicStringView>
   basic_string& assign(BasicStringView<CharT, Traits> sv, size_type pos, size_type n = npos)
   {
      if (pos > sv.size())
         throw_out_of_range("basic_string::assign out of range position");
      return this->assign(sv.begin() + pos,
                          sv.begin() + pos + dtl::min_value(n, sv.size() - pos));
   }

   //! <b>Requires</b>: pos <= str.size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to assign as
   //!   the smaller of n and str.size() - pos and calls assign(str.data() + pos rlen).
   //!
   //! <b>Throws</b>: If memory allocation throws or out_of_range if pos > str.size().
   //!
   //! <b>Returns</b>: *this
   basic_string& assign(const basic_string& s, size_type pos, size_type n = npos)
   {
      if (pos > s.size())
         throw_out_of_range("basic_string::assign out of range position");
      return this->assign(s.begin() + pos,
                          s.begin() + pos + dtl::min_value(n, s.size() - pos));
   }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT.
   //!   If n is zero, s may be null.
   //!
   //! <b>Effects</b>: Replaces the string controlled by *this with a string of
   //! length n whose elements are a copy of those pointed to by s.
   //!
   //! <b>Throws</b>: If memory allocation throws or length_error if n > max_size().
   //!
   //! <b>Returns</b>: *this
   basic_string& assign(const CharT* s, size_type n)
   {
      BOOST_ASSERT(s != 0 || n == 0);
      if(n == 0){
         this->clear();
         return *this;
      }
      return this->assign(s, s + difference_type(n));
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Effects</b>: Calls assign(s, traits::length(s)).
   //!
   //! <b>Returns</b>: *this
   basic_string& assign(const CharT* s)
   {
      BOOST_ASSERT(s != 0);
      return this->assign(s, s + Traits::length(s));
   }

   //! <b>Effects</b>: Equivalent to assign(basic_string(n, c)).
   //!
   //! <b>Returns</b>: *this
   basic_string& assign(size_type n, CharT c)
   {  return this->assign(cvalue_iterator(c, n), cvalue_iterator()); }

   //! <b>Effects</b>: Equivalent to assign(basic_string(first, last)).
    //!
    //! <b>Returns</b>: *this
    basic_string& assign(const CharT* first, const CharT* last)
    {
       size_type n = static_cast<size_type>(last - first);
       this->reserve(n);
       pointer   addr;
       size_type old_size, storage;
       const bool short_repr = this->priv_repr_data(addr, old_size, storage);
       CharT* const ptr = boost::movelib::to_raw_pointer(addr);
       //[first, last) can be a part of *this (e.g. assign(s, pos, n)): if so, n <= size(),
       //reserve() does not reallocate and the ranges can overlap.
       dtl::string_traits_move<Traits>(ptr, first, n);
       this->priv_construct_null(ptr + difference_type(n));
       this->priv_size(short_repr, n);
       return *this;
    }

   //! <b>Effects</b>: Equivalent to assign(basic_string(first, last)).
   //!
   //! <b>Throws</b>: If memory allocation throws, length_error if the new size is greater than
   //!   max_size(), or if an operation on the iterators throws.
   //!   If an exception is thrown, there are no effects (strong exception guarantee).
   //!
   //! <b>Returns</b>: *this
   template <class InputIter>
   basic_string& assign(InputIter first, InputIter last
      #if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
      , typename dtl::disable_if_or
         < void
         , dtl::is_convertible<InputIter, size_type>
         , dtl::is_not_input_iterator<InputIter>
         >::type * = 0
      #endif
      )
   {
      const size_type old_size = this->size();
      //Append the new characters after the old ones, then erase the old ones
      BOOST_CONTAINER_TRY{
         for ( ; first != last; ++first) {
            this->push_back(*first);
         }
      }
      BOOST_CONTAINER_CATCH(...){
         //No effects: erase the new characters
         this->erase(this->begin() + difference_type(old_size), this->end());
         BOOST_CONTAINER_RETHROW
      }
      BOOST_CONTAINER_CATCH_END
      this->erase(this->begin(), this->begin() + difference_type(old_size));
      return *this;
   }

   #if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   template <class ForwardIter>
   basic_string& assign(ForwardIter first, ForwardIter last
      , typename dtl::disable_if_or
         < void
         , dtl::is_convertible<ForwardIter, size_type>
         , dtl::is_input_iterator<ForwardIter>
         >::type * = 0
      )
   {
      this->priv_replace_range(0u, this->size(), first, last, boost::container::iterator_udistance(first, last));
      return *this;
   }
   #endif

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   //! <b>Effects</b>: Returns assign(il.begin(), il.end()).
   //!
   //! <b>Throws</b>: If memory allocation throws or length_error if the new size is greater than
   //!   max_size(). If an exception is thrown, there are no effects (strong exception guarantee).
   basic_string& assign(std::initializer_list<CharT> il)
   {
      return this->assign(il.begin(), il.end());
   }
   #endif

   //! <b>Requires</b>: pos <= size().
   //!
   //! <b>Effects</b>: Calls insert(pos, str.data(), str.size()).
   //!
   //! <b>Throws</b>: If memory allocation throws or out_of_range if pos > size().
   //!
   //! <b>Returns</b>: *this
   basic_string& insert(size_type pos, const basic_string& s)
   {
      const size_type sz = this->size();
      if (pos > sz)
         throw_out_of_range("basic_string::insert out of range position");
      if (sz > this->max_size() - s.size())
         throw_length_error("basic_string::insert max_size() exceeded");
      this->insert(this->priv_addr() + pos, s.begin(), s.end());
      return *this;
   }

   //! <b>Requires</b>: pos1 <= size() and pos2 <= str.size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to insert as
   //!   the smaller of n and str.size() - pos2 and calls insert(pos1, str.data() + pos2, rlen).
   //!
   //! <b>Throws</b>: If memory allocation throws or out_of_range if pos1 > size() or pos2 > str.size().
   //!
   //! <b>Returns</b>: *this
   basic_string& insert(size_type pos1, const basic_string& s, size_type pos2, size_type n = npos)
   {
      const size_type sz = this->size();
      const size_type str_size = s.size();
      if (pos1 > sz || pos2 > str_size)
         throw_out_of_range("basic_string::insert out of range position");
      size_type len = dtl::min_value(n, str_size - pos2);
      if (sz > this->max_size() - len)
         throw_length_error("basic_string::insert max_size() exceeded");
      const CharT *beg_ptr = boost::movelib::to_raw_pointer(s.begin()) + pos2;
      const CharT *end_ptr = beg_ptr + len;
      this->insert(this->priv_addr() + pos1, beg_ptr, end_ptr);
      return *this;
   }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT and pos <= size().
   //!   If n is zero, s may be null.
   //!
   //! <b>Effects</b>: Replaces the string controlled by *this with a string of length size() + n
   //!   whose first pos elements are a copy of the initial elements of the original string
   //!   controlled by *this and whose next n elements are a copy of the elements in s and whose
   //!   remaining elements are a copy of the remaining elements of the original string controlled by *this.
   //!
   //! <b>Throws</b>: If memory allocation throws, out_of_range if pos > size() or
   //!   length_error if size() + n > max_size().
   //!
   //! <b>Returns</b>: *this
   basic_string& insert(size_type pos, const CharT* s, size_type n)
   {
      BOOST_ASSERT(s != 0 || n == 0);
      if (pos > this->size())
         throw_out_of_range("basic_string::insert out of range position");
      if (this->size() > this->max_size() - n)
         throw_length_error("basic_string::insert max_size() exceeded");
      this->insert(this->priv_addr() + pos, s, s + difference_type(n));
      return *this;
   }

   //! <b>Requires</b>: pos <= size() and s points to an array of at least traits::length(s) + 1 elements of CharT
   //!
   //! <b>Effects</b>: Calls insert(pos, s, traits::length(s)).
   //!
   //! <b>Throws</b>: If memory allocation throws, out_of_range if pos > size()
   //!   length_error if size() > max_size() - Traits::length(s)
   //!
   //! <b>Returns</b>: *this
   basic_string& insert(size_type pos, const CharT* s)
   {
      BOOST_ASSERT(s != 0);
      if (pos > this->size())
         throw_out_of_range("basic_string::insert out of range position");
      size_type len = Traits::length(s);
      if (this->size() > this->max_size() - len)
         throw_length_error("basic_string::insert max_size() exceeded");
      this->insert(this->priv_addr() + pos, s, s + len);
      return *this;
   }

   //! <b>Effects</b>: Equivalent to insert(pos, basic_string(n, c)).
   //!
   //! <b>Throws</b>: If memory allocation throws, out_of_range if pos > size()
   //!   length_error if size() > max_size() - n
   //!
   //! <b>Returns</b>: *this
   basic_string& insert(size_type pos, size_type n, CharT c)
   {
      if (pos > this->size())
         throw_out_of_range("basic_string::insert out of range position");
      if (this->size() > this->max_size() - n)
         throw_length_error("basic_string::insert max_size() exceeded");
      this->insert(const_iterator(this->priv_addr() + pos), n, c);
      return *this;
   }

   //! <b>Effects</b>: Same as `return insert(pos, sv.data(), sv.size())`.
   //!
   template<template<class, class> class BasicStringView>
   basic_string& insert(size_type pos, BasicStringView<CharT, Traits> sv)
   {  return this->insert(pos, sv.data(), sv.size());  }


   //! <b>Requires</b>: pos1 <= size() and pos2 <= sv.size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to insert as
   //!   the smaller of n and sv.size() - pos2 and calls insert(pos1, sv.data() + pos2, rlen).
   //!
   //! <b>Throws</b>: If memory allocation throws or out_of_range if pos1 > size() or pos2 > sv.size().
   //!
   //! <b>Returns</b>: *this
   template<template<class, class> class BasicStringView>
   basic_string& insert(size_type pos1, BasicStringView<CharT, Traits> sv, size_type pos2, size_type n = npos)
   {
      const size_type sz = this->size();
      const size_type str_size = sv.size();
      if (pos1 > sz || pos2 > str_size)
         throw_out_of_range("basic_string::insert out of range position");
      size_type len = dtl::min_value(n, str_size - pos2);
      if (sz > this->max_size() - len)
         throw_length_error("basic_string::insert max_size() exceeded");
      const CharT *beg_ptr = boost::movelib::to_raw_pointer(sv.begin()) + pos2;
      const CharT *end_ptr = beg_ptr + len;
      this->insert(this->priv_addr() + pos1, beg_ptr, end_ptr);
      return *this;
   }

   //! <b>Requires</b>: p is a valid iterator on *this.
   //!
   //! <b>Effects</b>: inserts a copy of c before the character referred to by p.
   //!
   //! <b>Returns</b>: An iterator which refers to the copy of the inserted character.
   iterator insert(const_iterator p, CharT c)
   {
      size_type new_offset = size_type(p - this->priv_addr());
      this->insert(p, cvalue_iterator(c, 1), cvalue_iterator());
      return this->priv_addr() + new_offset;
   }

   //! <b>Requires</b>: p is a valid iterator on *this.
   //!
   //! <b>Effects</b>: Inserts n copies of c before the character referred to by p.
   //!
   //! <b>Returns</b>: an iterator to the first inserted element or p if n is 0.
   iterator insert(const_iterator p, size_type n, CharT c)
   {  return this->insert(p, cvalue_iterator(c, n), cvalue_iterator());  }

   //! <b>Requires</b>: p is a valid iterator on *this. [first,last) is a valid range.
   //!
   //! <b>Effects</b>: Equivalent to insert(p - begin(), basic_string(first, last)).
   //!
   //! <b>Throws</b>: If memory allocation throws, length_error if the new size is greater than
   //!   max_size(), or if an operation on the iterators throws.
   //!   If an exception is thrown, there are no effects (strong exception guarantee).
   //!
   //! <b>Returns</b>: an iterator to the first inserted element or p if first == last.
   template <class InputIter>
   iterator insert(const_iterator p, InputIter first, InputIter last
      #if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
      , typename dtl::disable_if_or
         < void
         , dtl::is_convertible<InputIter, size_type>
         , dtl::is_not_input_iterator<InputIter>
         >::type * = 0
      #endif
      )
   {
      const size_type n_pos = size_type(p - this->cbegin());
      const size_type old_size = this->size();
      //Append the new characters and rotate them
      BOOST_CONTAINER_TRY{
         for ( ; first != last; ++first) {
            this->push_back(*first);
         }
      }
      BOOST_CONTAINER_CATCH(...){
         this->erase(this->begin() + difference_type(old_size), this->end());
         BOOST_CONTAINER_RETHROW
      }
      BOOST_CONTAINER_CATCH_END
      boost::movelib::rotate_gcd
         (this->begin() + difference_type(n_pos), this->begin() + difference_type(old_size), this->end());
      return this->begin() + difference_type(n_pos);
   }

   #if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   template <class ForwardIter>
   iterator insert(const_iterator p, ForwardIter first, ForwardIter last
      , typename dtl::disable_if_or
         < void
         , dtl::is_convertible<ForwardIter, size_type>
         , dtl::is_input_iterator<ForwardIter>
         >::type * = 0
      )
   {
      const size_type n_pos = size_type(p - this->cbegin());
      if (first != last) {
         this->priv_replace_range(n_pos, 0u, first, last, boost::container::iterator_udistance(first, last));
      }
      return this->begin() + difference_type(n_pos);
   }
   #endif

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   //! <b>Effects</b>: As if by insert(p, il.begin(), il.end()).
   //!
   //! <b>Throws</b>: If memory allocation throws or length_error if the new size is greater than
   //!   max_size(). If an exception is thrown, there are no effects (strong exception guarantee).
   //!
   //! <b>Returns</b>: An iterator which refers to the copy of the first inserted
   //!   character, or p if i1 is empty.
   inline iterator insert(const_iterator p, std::initializer_list<CharT> il)
   {
      return this->insert(p, il.begin(), il.end());
   }
   #endif

   //! <b>Effects</b>: Removes the last element from the container.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Constant time.
   void pop_back() BOOST_NOEXCEPT_OR_NOTHROW
   {
      BOOST_ASSERT(!this->empty());
      iterator p = this->end();
      this->erase(--p);
   }

   //! <b>Requires</b>: pos <= size()
   //!
   //! <b>Effects</b>: Determines the effective length xlen of the string to be removed as the smaller of n and size() - pos.
   //!   The function then replaces the string controlled by *this with a string of length size() - xlen
   //!   whose first pos elements are a copy of the initial elements of the original string controlled by *this,
   //!   and whose remaining elements are a copy of the elements of the original string controlled by *this
   //!   beginning at position pos + xlen.
   //!
   //! <b>Throws</b>: out_of_range if pos > size().
   //!
   //! <b>Returns</b>: *this
   basic_string& erase(size_type pos = 0, size_type n = npos)
   {
      if (pos > this->size())
         throw_out_of_range("basic_string::erase out of range position");
      const pointer addr = this->priv_addr();
      erase(addr + difference_type(pos), addr + difference_type(pos) + dtl::min_value(n, this->size() - pos));
      return *this;
   }

   //! <b>Effects</b>: Removes the character referred to by p.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: An iterator which points to the element immediately following p prior to the element being
   //!    erased. If no such element exists, end() is returned.
   iterator erase(const_iterator p) BOOST_NOEXCEPT_OR_NOTHROW
   {
      // The move includes the terminating null.
      CharT * const ptr = const_cast<CharT*>(boost::movelib::to_raw_pointer(p));
      pointer   addr;
      size_type old_size, storage;
      const bool short_repr = this->priv_repr_data(addr, old_size, storage);
      Traits::move(ptr,
                   boost::movelib::to_raw_pointer(p + 1),
                   old_size - size_type(p - addr));
      this->priv_size(short_repr, old_size-1u);
      return iterator(ptr);
   }

   //! <b>Requires</b>: first and last are valid iterators on *this, defining a range [first,last).
   //!
   //! <b>Effects</b>: Removes the characters in the range [first,last).
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: An iterator which points to the element pointed to by last prior to
   //!   the other elements being erased. If no such element exists, end() is returned.
   iterator erase(const_iterator first, const_iterator last) BOOST_NOEXCEPT_OR_NOTHROW
   {
      CharT * f = const_cast<CharT*>(boost::movelib::to_raw_pointer(first));
      if (first != last) { // The move includes the terminating null.
         const size_type num_erased = size_type(last - first);
         pointer   addr;
         size_type old_size, storage;
         const bool short_repr = this->priv_repr_data(addr, old_size, storage);
         Traits::move(f,
                      boost::movelib::to_raw_pointer(last),
                      old_size + 1u - size_type(last - addr));
         const size_type new_length = old_size - num_erased;
         this->priv_size(short_repr, new_length);
      }
      return iterator(f);
   }

   //! <b>Effects</b>: Erases all the elements of the vector.
   //!
   //! <b>Throws</b>: Nothing.
   //!
   //! <b>Complexity</b>: Linear to the number of elements in the vector.
   void clear() BOOST_NOEXCEPT_OR_NOTHROW
   {
       if(this->is_short()) {
           Traits::assign(*this->priv_short_addr(), CharT(0));
           this->priv_short_size(0);
       }
       else {
           Traits::assign(*this->priv_long_addr(), CharT(0));
           this->priv_long_size(0);
       }
   }

   //! <b>Requires</b>: pos1 <= size().
   //!
   //! <b>Effects</b>: Calls replace(pos1, n1, str.data(), str.size()).
   //!
   //! <b>Throws</b>: if memory allocation throws or out_of_range if pos1 > size().
   //!
   //! <b>Returns</b>: *this
   basic_string& replace(size_type pos1, size_type n1, const basic_string& str)
   {
      if (pos1 > this->size())
         throw_out_of_range("basic_string::replace out of range position");
      const size_type len = dtl::min_value(n1, this->size() - pos1);
      if (this->size() - len > this->max_size() - str.size())
         throw_length_error("basic_string::replace max_size() exceeded");
      const pointer addr = this->priv_addr();
      return this->replace( const_iterator(addr + difference_type(pos1))
                          , const_iterator(addr + difference_type(pos1 + len))
                          , str.begin(), str.end());
   }

   //! <b>Effects</b>: Calls `return replace(pos1, n1, sv.data(), sv.size());`.
   //!
   template<template<class, class> class BasicStringView>
   inline basic_string& replace(size_type pos1, size_type n1, BasicStringView<CharT, Traits> sv)
   {
      return this->replace(pos1, n1, sv.data(), sv.size());
   }

   //! <b>Requires</b>: pos1 <= size() and pos2 <= str.size().
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to be
   //!   inserted as the smaller of n2 and str.size() - pos2 and calls
   //!   replace(pos1, n1, str.data() + pos2, rlen).
   //!
   //! <b>Throws</b>: if memory allocation throws, out_of_range if pos1 > size() or pos2 > str.size().
   //!
   //! <b>Returns</b>: *this
   basic_string& replace(size_type pos1, size_type n1,
                         const basic_string& str, size_type pos2, size_type n2 = npos)
   {
      if (pos2 > str.size())
         throw_out_of_range("basic_string::replace out of range position");
      return this->replace(pos1, n1, str.data()+pos2, dtl::min_value(n2, str.size() - pos2));
   }

   //! <b>Throws</b>: out_of_range if pos1 > size() or pos2 > sv.size().
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to be inserted as the
   //!   smaller of n2 and sv.size() - pos2 and calls `replace(pos1, n1, sv.data() + pos2, rlen)`.
   //!
   //! <b>Returns</b>: *this.
   template<template<class, class> class BasicStringView>
   basic_string& replace(size_type pos1, size_type n1, BasicStringView<CharT, Traits> sv,
                         size_type pos2, size_type n2 = npos)
   {
      if (pos2 > sv.size())
         throw_out_of_range("basic_string::replace out of range position");
      return this->replace(pos1, n1, sv.data()+pos2, dtl::min_value(n2, sv.size() - pos2));
   }

   //! <b>Requires</b>: pos1 <= size() and s points to an array of at least n2 elements of CharT.
   //!   If n2 is zero, s may be null.
   //!
   //! <b>Effects</b>: Determines the effective length xlen of the string to be removed as the
   //!   smaller of n1 and size() - pos1. If size() - xlen > max_size() - n2 throws length_error.
   //!   Otherwise, the function replaces the string controlled by *this with a string of
   //!   length size() - xlen + n2 whose first pos1 elements are a copy of the initial elements
   //!   of the original string controlled by *this, whose next n2 elements are a copy of the
   //!   initial n2 elements of s, and whose remaining elements are a copy of the elements of
   //!   the original string controlled by *this beginning at position pos + xlen.
   //!
   //! <b>Throws</b>: if memory allocation throws, out_of_range if pos1 > size() or length_error
   //!   if the length of the resulting string would exceed max_size()
   //!
   //! <b>Returns</b>: *this
   basic_string& replace(size_type pos1, size_type n1, const CharT* s, size_type n2)
   {
      BOOST_ASSERT(s != 0 || n2 == 0);
      if (pos1 > this->size())
         throw_out_of_range("basic_string::replace out of range position");
      const size_type len = dtl::min_value(n1, this->size() - pos1);
      const size_type max_sz = this->max_size();
      if (n2 > max_sz || (this->size() - len) > (max_sz - n2))
         throw_length_error("basic_string::replace max_size() exceeded");
      const pointer addr = this->priv_addr() + pos1;
      return this->replace(addr, addr + difference_type(len), s, s + difference_type(n2));
   }

   //! <b>Requires</b>: pos1 <= size() and s points to an array of at least n2 elements of CharT.
   //!
   //! <b>Effects</b>: Determines the effective length xlen of the string to be removed as the smaller
   //! of n1 and size() - pos1. If size() - xlen > max_size() - n2 throws length_error. Otherwise,
   //! the function replaces the string controlled by *this with a string of length size() - xlen + n2
   //! whose first pos1 elements are a copy of the initial elements of the original string controlled
   //! by *this, whose next n2 elements are a copy of the initial n2 elements of s, and whose
   //! remaining elements are a copy of the elements of the original string controlled by *this
   //! beginning at position pos + xlen.
   //!
   //! <b>Throws</b>: if memory allocation throws, out_of_range if pos1 > size() or length_error
   //!   if the length of the resulting string would exceed max_size()
   //!
   //! <b>Returns</b>: *this
   inline basic_string& replace(size_type pos, size_type n1, const CharT* s)
   {
      BOOST_ASSERT(s != 0);
      return this->replace(pos, n1, s, Traits::length(s));
   }

   //! <b>Requires</b>: pos1 <= size().
   //!
   //! <b>Effects</b>: Equivalent to replace(pos1, n1, basic_string(n2, c)).
   //!
   //! <b>Throws</b>: if memory allocation throws, out_of_range if pos1 > size() or length_error
   //!   if the length of the  resulting string would exceed max_size()
   //!
   //! <b>Returns</b>: *this
   basic_string& replace(size_type pos1, size_type n1, size_type n2, CharT c)
   {
      if (pos1 > this->size())
         throw_out_of_range("basic_string::replace out of range position");
      const size_type len = dtl::min_value(n1, this->size() - pos1);
      if (n2 > this->max_size() || this->size() - len > this->max_size() - n2)
         throw_length_error("basic_string::replace max_size() exceeded");
      const pointer addr    = this->priv_addr();
      return this->replace(addr + difference_type(pos1), addr + difference_type(pos1 + len), n2, c);
   }

   //! <b>Requires</b>: [begin(),i1) and [i1,i2) are valid ranges.
   //!
   //! <b>Effects</b>: Calls replace(i1 - begin(), i2 - i1, str).
   //!
   //! <b>Throws</b>: if memory allocation throws
   //!
   //! <b>Returns</b>: *this
   inline basic_string& replace(const_iterator i1, const_iterator i2, const basic_string& str)
   { return this->replace(i1, i2, str.data(), str.data()+str.size()); }

   //! <b>Requires</b>: [begin(),i1) and [i1,i2) are valid ranges and
   //!   s points to an array of at least n elements
   //!
   //! <b>Effects</b>: Calls replace(i1 - begin(), i2 - i1, s, n).
   //!
   //! <b>Throws</b>: if memory allocation throws
   //!
   //! <b>Returns</b>: *this
   inline basic_string& replace(const_iterator i1, const_iterator i2, const CharT* s, size_type n)
   {
      BOOST_ASSERT(s != 0 || n == 0);
      return this->replace(i1, i2, s, s + difference_type(n));
   }

   //! <b>Requires</b>: [begin(),i1) and [i1,i2) are valid ranges and s points to an
   //!   array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Effects</b>: Calls replace(i1 - begin(), i2 - i1, s, traits::length(s)).
   //!
   //! <b>Throws</b>: if memory allocation throws
   //!
   //! <b>Returns</b>: *this
   inline basic_string& replace(const_iterator i1, const_iterator i2, const CharT* s)
   {
      BOOST_ASSERT(s != 0);
      return this->replace(i1, i2, s, s + Traits::length(s));
   }

   //! <b>Requires</b>: [begin(),i1) and [i1,i2) are valid ranges.
   //!
   //! <b>Effects</b>: Calls replace(i1 - begin(), i2 - i1, basic_string(n, c)).
   //!
   //! <b>Throws</b>: if memory allocation throws
   //!
   //! <b>Returns</b>: *this
   basic_string& replace(const_iterator i1, const_iterator i2, size_type n, CharT c)
   {
      const size_type len = static_cast<size_type>(i2 - i1);
      if (len >= n) {
         Traits::assign(const_cast<CharT*>(boost::movelib::to_raw_pointer(i1)), n, c);
         erase(i1 + difference_type(n), i2);
      }
      else {
         Traits::assign(const_cast<CharT*>(boost::movelib::to_raw_pointer(i1)), len, c);
         insert(i2, n - len, c);
      }
      return *this;
   }

   //! <b>Requires</b>: [begin(),i1), [i1,i2) and [j1,j2) are valid ranges.
   //!
   //! <b>Effects</b>: Calls replace(i1 - begin(), i2 - i1, basic_string(j1, j2)).
   //!
   //! <b>Throws</b>: If memory allocation throws, length_error if the new size is greater than
   //!   max_size(), or if an operation on the iterators throws.
   //!   If an exception is thrown, there are no effects (strong exception guarantee).
   //!
   //! <b>Returns</b>: *this
   template <class InputIter>
   basic_string& replace(const_iterator i1, const_iterator i2, InputIter j1, InputIter j2
      #if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
      , typename dtl::disable_if_or
         < void
         , dtl::is_convertible<InputIter, size_type>
         , dtl::is_not_input_iterator<InputIter>
         >::type * = 0
      #endif
      )
   {
      //Insert the new characters after [i1, i2) (linear and with no effects if an
      //exception is thrown), then erase [i1, i2), which can't throw
      const difference_type pos = i1 - this->cbegin();
      const difference_type n1  = i2 - i1;
      this->insert(i2, j1, j2);
      this->erase(this->begin() + pos, this->begin() + (pos + n1));
      return *this;
   }

   #if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   template <class ForwardIter>
   basic_string& replace(const_iterator i1, const_iterator i2, ForwardIter j1, ForwardIter j2
      , typename dtl::disable_if_or
         < void
         , dtl::is_convertible<ForwardIter, size_type>
         , dtl::is_input_iterator<ForwardIter>
         >::type * = 0
      )
   {
      this->priv_replace_range
         ( size_type(i1 - this->cbegin()), size_type(i2 - i1)
         , j1, j2, boost::container::iterator_udistance(j1, j2));
      return *this;
   }
   #endif

   //! <b>Requires</b>: [begin(), i1) and [i1, i2) are valid ranges.
   //!
   //! <b>Effects</b>: Calls `replace(i1 - begin(), i2 - i1, sv).`.
   //!
   //! <b>Returns</b>: *this.
   template<template <class, class> class BasicStringView>
   inline basic_string& replace(const_iterator i1, const_iterator i2, BasicStringView<CharT, Traits> sv)
   {
      return this->replace( static_cast<size_type>(i1 - this->cbegin())
                          , static_cast<size_type>(i2 - i1), sv);
   }

   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   //! <b>Requires</b>: [begin(), i1) and [i1, i2) are valid ranges.
   //!
   //! <b>Effects</b>: Calls replace(i1, i2, il.begin(), il.end()).
   //!
   //! <b>Throws</b>: If memory allocation throws or length_error if the new size is greater than
   //!   max_size(). If an exception is thrown, there are no effects (strong exception guarantee).
   //!
   //! <b>Returns</b>: *this.
   inline basic_string& replace(const_iterator i1, const_iterator i2, std::initializer_list<CharT> il)
   {
      return this->replace(i1, i2, il.begin(), il.end());
   }
   #endif

   //! <b>Requires</b>: pos <= size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to copy as the
   //!   smaller of n and size() - pos. s shall designate an array of at least rlen elements.
   //!   The function then replaces the string designated by s with a string of length rlen
   //!   whose elements are a copy of the string controlled by *this beginning at position pos.
   //!   The function does not append a null object to the string designated by s.
   //!
   //! <b>Throws</b>: if memory allocation throws, out_of_range if pos > size().
   //!
   //! <b>Returns</b>: rlen
   size_type copy(CharT* s, size_type n, size_type pos = 0) const
   {
      BOOST_ASSERT(s != 0 || n == 0);
      if (pos > this->size())
         throw_out_of_range("basic_string::copy out of range position");
      const size_type len = dtl::min_value(n, this->size() - pos);
      dtl::string_traits_copy<Traits>(s, boost::movelib::to_raw_pointer(this->priv_addr() + pos), len);
      return len;
   }

   //! <b>Effects</b>: *this contains the same sequence of characters that was in s,
   //!   s contains the same sequence of characters that was in *this.
   //!
   //! <b>Throws</b>: Nothing
   void swap(basic_string& x)
      BOOST_NOEXCEPT_IF(allocator_traits_type::propagate_on_container_swap::value
                               || allocator_traits_type::is_always_equal::value)
   {
      this->base_t::swap_data(x);
      dtl::bool_<allocator_traits_type::propagate_on_container_swap::value> flag;
      dtl::swap_alloc(this->alloc(), x.alloc(), flag);
   }

   //////////////////////////////////////////////
   //
   //                 data access
   //
   //////////////////////////////////////////////

   //! <b>Requires</b>: The program shall not alter any of the values stored in the character array.
   //!
   //! <b>Returns</b>: A pointer p such that p + i == &operator[](i) for each i in [0,size()].
   //!
   //! <b>Complexity</b>: constant time.
   BOOST_CONTAINER_NODISCARD inline
      const CharT* c_str() const BOOST_NOEXCEPT_OR_NOTHROW
   {  return boost::movelib::to_raw_pointer(this->priv_addr()); }

   //! <b>Requires</b>: The program shall not alter any of the values stored in the character array.
   //!
   //! <b>Returns</b>: A pointer p such that p + i == &operator[](i) for each i in [0,size()].
   //!
   //! <b>Complexity</b>: constant time.
   BOOST_CONTAINER_NODISCARD inline
      const CharT* data()  const BOOST_NOEXCEPT_OR_NOTHROW
   {  return boost::movelib::to_raw_pointer(this->priv_addr()); }

   //! <b>Returns</b>: A pointer p such that p + i == &operator[](i) for each i in [0,size()].
   //!
   //! <b>Complexity</b>: constant time.
   BOOST_CONTAINER_NODISCARD inline
      CharT* data()  BOOST_NOEXCEPT_OR_NOTHROW
   {  return boost::movelib::to_raw_pointer(this->priv_addr()); }

   #ifndef BOOST_CONTAINER_TEMPLATED_CONVERSION_OPERATOR_BROKEN
   //! <b>Returns</b>: a string_view to the characters in the string.
   //!
   //! <b>Complexity</b>: constant time.
   template<template <class, class> class BasicStringView>
   inline operator BasicStringView<CharT, Traits>() const BOOST_NOEXCEPT_OR_NOTHROW
   { return this->to_view< BasicStringView<CharT, Traits> >(); }
   #endif

   //! <b>Returns</b>: a string_view to the characters in the string.
   //!
   //! <b>Complexity</b>: constant time.
   //!
   //! <b>Note</b>: This function is available to write portable code for compilers
   //!   that don't support templated conversion operators.
   template<class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      BasicStringView to_view() const BOOST_NOEXCEPT_OR_NOTHROW
   { return BasicStringView(this->data(), this->size()); }

   //////////////////////////////////////////////
   //
   //             string operations
   //
   //////////////////////////////////////////////

   //! <b>Effects</b>: Determines the lowest position xpos, if possible, such that both
   //!   of the following conditions hold:
   //!   1) pos <= xpos and xpos + str.size() <= size();
   //!   2) traits::eq(at(xpos+I), str.at(I)) for all elements I of the string controlled by str.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   BOOST_CONTAINER_NODISCARD inline
      size_type find(const basic_string& s, size_type pos = 0) const
   { return find(s.c_str(), pos, s.size()); }

   //! <b>Effects</b>: Determines the lowest position xpos, if possible, such that both
   //!   of the following conditions hold:
   //!   1) pos <= xpos and xpos + sv.size() <= size();
   //!   2) traits::eq(at(xpos+I), sv.at(I)) for all elements I of the string controlled by sv.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      size_type find(BasicStringView<CharT, Traits> sv, size_type pos = 0) const
   { return this->find(sv.data(), pos, sv.size()); }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT.
   //!   If n is zero, s may be null.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find(basic_string<CharT,traits,allocator_type>(s,n),pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find(const CharT* s, size_type pos, size_type n) const
   {
      BOOST_ASSERT(s != 0 || n == 0);
      if (pos > this->size() || n > this->size() - pos)
         return npos;
      else if (!n)   //An empty string is found at pos
         return pos;
      else {
         const pointer addr = this->priv_addr();
         pointer finish = addr + difference_type(this->priv_size());
         const const_iterator result =
            boost::container::search(boost::movelib::to_raw_pointer(addr + difference_type(pos)),
                   boost::movelib::to_raw_pointer(finish),
                   s, s + difference_type(n), Eq_traits<Traits>());
         return result != finish ? size_type(result - begin()) : npos;
      }
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find(basic_string(s), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find(const CharT* s, size_type pos = 0) const
   {
      BOOST_ASSERT(s != 0);
      return this->find(s, pos, Traits::length(s));
   }

   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find(basic_string<CharT,traits,allocator_type>(1,c), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find(CharT c, size_type pos = 0) const
   {
      const size_type sz = this->size();
      if (pos >= sz)
         return npos;
      else {
         const pointer addr    = this->priv_addr();
         pointer finish = addr + difference_type(sz);
         const const_iterator result =
            boost::container::find_if(addr + difference_type(pos), finish,
                  boost::container::bind2nd(Eq_traits<Traits>(), c));
         return result != finish ? size_type(result - begin()) : npos;
      }
   }

   //! <b>Effects</b>: Determines the highest position xpos, if possible, such
   //!   that both of the following conditions obtain:
   //!   a) xpos <= pos and xpos + str.size() <= size();
   //!   b) traits::eq(at(xpos+I), str.at(I)) for all elements I of the string controlled by str.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   BOOST_CONTAINER_NODISCARD inline
      size_type rfind(const basic_string& str, size_type pos = npos) const
      { return this->rfind(str.c_str(), pos, str.size()); }

   //! <b>Effects</b>: Determines the highest position xpos, if possible, such
   //!   that both of the following conditions obtain:
   //!   a) xpos <= pos and xpos + sv.size() <= size();
   //!   b) traits::eq(at(xpos+I), sv.at(I)) for all elements I of the string controlled by sv.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      size_type rfind(BasicStringView<CharT, Traits> sv, size_type pos = npos) const
      { return this->rfind(sv.data(), pos, sv.size()); }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT.
   //!   If n is zero, s may be null.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: rfind(basic_string(s, n), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type rfind(const CharT* s, size_type pos, size_type n) const
   {
      BOOST_ASSERT(s != 0 || n == 0);
      const size_type len = this->size();

      if (n > len)
         return npos;
      else if (n == 0)
         return dtl::min_value(len, pos);
      else {
         const const_iterator last = begin() + difference_type(dtl::min_value(len - n, pos) + n);
         const const_iterator result = boost::container::find_end
            (begin(), last, s, s + difference_type(n), Eq_traits<Traits>());
         return result != last ? size_type(result - begin()) : npos;
      }
   }

   //! <b>Requires</b>: pos <= size() and s points to an array of at least
   //!   traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: rfind(basic_string(s), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type rfind(const CharT* s, size_type pos = npos) const
   {
      BOOST_ASSERT(s != 0);
      return this->rfind(s, pos, Traits::length(s));
   }

   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: rfind(basic_string<CharT,traits,allocator_type>(1,c),pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type rfind(CharT c, size_type pos = npos) const
   {
      const size_type len = this->size();

      if (len < 1)
         return npos;
      else {
         const const_iterator last = begin() + dtl::min_value(len - 1, pos) + 1;
         const_reverse_iterator rresult =
            boost::container::find_if(const_reverse_iterator(last), rend(),
                  boost::container::bind2nd(Eq_traits<Traits>(), c));
         return rresult != rend() ? size_type((rresult.base() - 1) - begin()) : npos;
      }
   }

   //! <b>Effects</b>: Determines the lowest position xpos, if possible, such that both of the
   //!   following conditions obtain: a) pos <= xpos and xpos < size();
   //!   b) traits::eq(at(xpos), str.at(I)) for some element I of the string controlled by str.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_of(const basic_string& str, size_type pos = 0) const
      { return this->find_first_of(str.c_str(), pos, str.size()); }

   //! <b>Effects</b>: Determines the lowest position xpos, if possible, such that both of the
   //!   following conditions obtain: a) pos <= xpos and xpos < size();
   //!   b) traits::eq(at(xpos), sv.at(I)) for some element I of the string controlled by sv.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_of(BasicStringView<CharT, Traits> sv, size_type pos = 0) const
      { return this->find_first_of(sv.data(), pos, sv.size()); }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT.
   //!   If n is zero, s may be null.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_first_of(basic_string(s, n), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_of(const CharT* s, size_type pos, size_type n) const
   {
      BOOST_ASSERT(s != 0 || n == 0);
      const size_type sz = this->size();
      if (pos >= sz)
         return npos;
      else {
         const pointer addr    = this->priv_addr();
         pointer finish = addr + difference_type(sz);
         const_iterator result = boost::container::find_first_of
            (addr + difference_type(pos), finish, s, s + difference_type(n), Eq_traits<Traits>());
         return result != finish ? size_type(result - this->begin()) : npos;
      }
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_first_of(basic_string(s), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_of(const CharT* s, size_type pos = 0) const
   {
      BOOST_ASSERT(s != 0);
      return this->find_first_of(s, pos, Traits::length(s));
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_first_of(basic_string<CharT,traits,allocator_type>(1,c), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_of(CharT c, size_type pos = 0) const
    { return this->find(c, pos); }

   //! <b>Effects</b>: Determines the highest position xpos, if possible, such that both of
   //!   the following conditions obtain: a) xpos <= pos and xpos < size(); b)
   //!   traits::eq(at(xpos), str.at(I)) for some element I of the string controlled by str.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_of(const basic_string& str, size_type pos = npos) const
      { return this->find_last_of(str.c_str(), pos, str.size()); }

   //! <b>Effects</b>: Determines the highest position xpos, if possible, such that both of
   //!   the following conditions obtain: a) xpos <= pos and xpos < size(); b)
   //!   traits::eq(at(xpos), str.at(I)) for some element I of the string controlled by str.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_of(BasicStringView<CharT, Traits> sv, size_type pos = npos) const
      { return this->find_last_of(sv.data(), pos, sv.size()); }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT.
   //!   If n is zero, s may be null.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_last_of(basic_string(s, n), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_of(const CharT* s, size_type pos, size_type n) const
   {
      BOOST_ASSERT(s != 0 || n == 0);
      const size_type len = this->size();

      if (len < 1)
         return npos;
      else {
         const pointer addr    = this->priv_addr();
         const const_iterator last = addr + difference_type(dtl::min_value(len - 1, pos) + 1);
         const const_reverse_iterator rresult =
            boost::container::find_first_of(const_reverse_iterator(last), rend(),
                               s, s + difference_type(n), Eq_traits<Traits>());
         return rresult != rend() ? size_type((rresult.base() - 1) - addr) : npos;
      }
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_last_of(basic_string<CharT,traits,allocator_type>(1,c),pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_of(const CharT* s, size_type pos = npos) const
   {
      BOOST_ASSERT(s != 0);
      return this->find_last_of(s, pos, Traits::length(s));
   }

   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_last_of(basic_string(s), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_of(CharT c, size_type pos = npos) const
      {  return this->rfind(c, pos);   }

   //! <b>Effects</b>: Determines the lowest position xpos, if possible, such that
   //!   both of the following conditions obtain:
   //!   a) pos <= xpos and xpos < size(); b) traits::eq(at(xpos), str.at(I)) for no
   //!   element I of the string controlled by str.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_not_of(const basic_string& str, size_type pos = 0) const
      { return this->find_first_not_of(str.c_str(), pos, str.size()); }

   //! <b>Effects</b>: Determines the lowest position xpos, if possible, such that
   //!   both of the following conditions obtain:
   //!   a) pos <= xpos and xpos < size(); b) traits::eq(at(xpos), sv.at(I)) for no
   //!   element I of the string controlled by sv.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_not_of(BasicStringView<CharT, Traits> sv, size_type pos = 0) const
      { return this->find_first_not_of(sv.data(), pos, sv.size()); }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_first_not_of(basic_string(s, n), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_not_of(const CharT* s, size_type pos, size_type n) const
   {
      BOOST_ASSERT(s != 0 || n == 0);
      if (pos > this->size())
         return npos;
      else {
         const pointer addr   = this->priv_addr();
         const pointer finish = addr + difference_type(this->priv_size());
         const const_iterator result = boost::container::find_if
            (addr + difference_type(pos), finish, Not_within_traits<Traits>(s, s + difference_type(n)));
         return result != finish ? size_type(result - addr) : npos;
      }
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_first_not_of(basic_string(s), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_not_of(const CharT* s, size_type pos = 0) const
   {
      BOOST_ASSERT(s != 0);
      return this->find_first_not_of(s, pos, Traits::length(s));
   }

   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_first_not_of(basic_string(1, c), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_first_not_of(CharT c, size_type pos = 0) const
   {
      if (pos > this->size())
         return npos;
      else {
         const pointer addr   = this->priv_addr();
         const pointer finish = addr + difference_type(this->priv_size());
         const const_iterator result
            = boost::container::find_if(addr + difference_type(pos), finish,
                     boost::container::not1(boost::container::bind2nd(Eq_traits<Traits>(), c)));
         return result != finish ? size_type(result - begin()) : npos;
      }
   }

   //! <b>Effects</b>: Determines the highest position xpos, if possible, such that
   //!   both of the following conditions obtain: a) xpos <= pos and xpos < size();
   //!   b) traits::eq(at(xpos), str.at(I)) for no element I of the string controlled by str.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_not_of(const basic_string& str, size_type pos = npos) const
      { return this->find_last_not_of(str.c_str(), pos, str.size()); }

   //! <b>Effects</b>: Determines the highest position xpos, if possible, such that
   //!   both of the following conditions obtain: a) xpos <= pos and xpos < size();
   //!   b) traits::eq(at(xpos), sv.at(I)) for no element I of the string controlled by sv.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: xpos if the function can determine such a value for xpos. Otherwise, returns npos.
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_not_of(BasicStringView<CharT, Traits> sv, size_type pos = npos) const
      { return this->find_last_not_of(sv.data(), pos, sv.size()); }

   //! <b>Requires</b>: s points to an array of at least n elements of CharT.
   //!   If n is zero, s may be null.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_last_not_of(basic_string(s, n), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_not_of(const CharT* s, size_type pos, size_type n) const
   {
      BOOST_ASSERT(s != 0 || n == 0);
      const size_type len = this->size();

      if (len < 1)
         return npos;
      else {
         const const_iterator last = begin() + dtl::min_value(len - 1, pos) + 1;
         const const_reverse_iterator rresult =
            boost::container::find_if(const_reverse_iterator(last), rend(),
                    Not_within_traits<Traits>(s, s + difference_type(n)));
         return rresult != rend() ? size_type((rresult.base() - 1) - begin()) : npos;
      }
   }

   //! <b>Requires</b>: s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_last_not_of(basic_string(s), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_not_of(const CharT* s, size_type pos = npos) const
   {
      BOOST_ASSERT(s != 0);
      return this->find_last_not_of(s, pos, Traits::length(s));
   }

   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: find_last_not_of(basic_string(1, c), pos).
   BOOST_CONTAINER_NODISCARD inline
      size_type find_last_not_of(CharT c, size_type pos = npos) const
   {
      const size_type len = this->size();

      if (len < 1)
         return npos;
      else {
         const const_iterator last = begin() + dtl::min_value(len - 1, pos) + 1;
         const const_reverse_iterator rresult =
            boost::container::find_if(const_reverse_iterator(last), rend(),
                  boost::container::not1(boost::container::bind2nd(Eq_traits<Traits>(), c)));
         return rresult != rend() ? size_type((rresult.base() - 1) - begin()) : npos;
      }
   }

   //! <b>Requires</b>: Requires: pos <= size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to copy as
   //!   the smaller of n and size() - pos.
   //!
   //! <b>Throws</b>: If memory allocation throws or out_of_range if pos > size().
   //!
   //! <b>Returns</b>: basic_string<CharT,traits,allocator_type>(data()+pos,rlen).
   BOOST_CONTAINER_NODISCARD inline
      basic_string substr(size_type pos = 0, size_type n = npos) const
   {
      if (pos > this->size())
         throw_out_of_range("basic_string::substr out of range position");
      const pointer addr = this->priv_addr();
      return basic_string(addr + difference_type(pos),
                          addr + difference_type(pos + dtl::min_value(n, size() - pos)), this->alloc());
   }

   //! <b>Effects</b>: Determines the effective length rlen of the string to compare as
   //!   the smaller of size() and str.size(). The function then compares the two strings by
   //!   calling traits::compare(data(), str.data(), rlen).
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: The nonzero result if the result of the comparison is nonzero.
   //!   Otherwise, returns a value < 0 if size() < str.size(), a 0 value if size() == str.size(),
   //!   and value > 0 if size() > str.size()
   BOOST_CONTAINER_NODISCARD inline
      int compare(const basic_string& str) const
   {
      const pointer addr     = this->priv_addr();
      const pointer str_addr = str.priv_addr();
      return this->s_compare(addr, addr + difference_type(this->priv_size()), str_addr, str_addr + difference_type(str.priv_size()));
   }

   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: compare(basic_string(sv)).
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      int compare(BasicStringView<CharT,Traits> sv) const
   {
      const pointer addr = this->priv_addr();
      return this->s_compare(addr, addr + difference_type(this->priv_size()), sv.data(), sv.data() + difference_type(sv.size()));
   }

   //! <b>Requires</b>: pos1 <= size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to compare as
   //!   the smaller of (this->size() - pos1), n1 and str.size(). The function then compares the two strings by
   //!   calling traits::compare(data()+pos1, str.data(), rlen).
   //!
   //! <b>Throws</b>: out_of_range if pos1 > size()
   //!
   //! <b>Returns</b>:basic_string(*this,pos1,n1).compare(str).
   BOOST_CONTAINER_NODISCARD inline
      int compare(size_type pos1, size_type n1, const basic_string& str) const
   {
      if (pos1 > this->size())
         throw_out_of_range("basic_string::compare out of range position");
      const pointer addr    = this->priv_addr();
      const pointer str_addr = str.priv_addr();
      return this->s_compare(addr + difference_type(pos1),
                        addr + difference_type(pos1 + dtl::min_value(n1, this->size() - pos1)),
                        str_addr, str_addr + difference_type(str.priv_size()));
   }

   //! <b>Requires</b>: pos1 <= size()
   //!
   //! <b>Throws</b>: out_of_range if pos1 > size()
   //!
   //! <b>Returns</b>:basic_string(*this,pos1,n1).compare(sv).
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      int compare(size_type pos1, size_type n1, BasicStringView<CharT,Traits> sv) const
   {
      if (pos1 > this->size())
         throw_out_of_range("basic_string::compare out of range position");
      const pointer addr    = this->priv_addr() + pos1;
      const CharT* const str_addr = sv.data();
      return this->s_compare(addr, addr + difference_type(dtl::min_value(n1, this->size() - pos1)),
                       str_addr, str_addr + sv.size());
   }

   //! <b>Requires</b>: pos1 <= size() and pos2 <= str.size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to copy as
   //!   the smaller of
   //!
   //! <b>Throws</b>: out_of_range if pos1 > size() or pos2 > str.size()
   //!
   //! <b>Returns</b>: basic_string(*this, pos1, n1).compare(basic_string(str, pos2, n2)).
   BOOST_CONTAINER_NODISCARD inline
      int compare(size_type pos1, size_type n1, const basic_string& str, size_type pos2, size_type n2 = npos) const
   {
      if (pos1 > this->size() || pos2 > str.size())
         throw_out_of_range("basic_string::compare out of range position");
      const pointer addr     = this->priv_addr() + pos1;
      const pointer str_addr = str.priv_addr() + pos2;
      return this->s_compare(addr, addr + difference_type(dtl::min_value(n1, this->size() - pos1)),
                             str_addr, str_addr + difference_type(dtl::min_value(n2, str.size() - pos2)));
   }

   //! <b>Requires</b>: pos1 <= size() and pos2 <= str.size()
   //!
   //! <b>Effects</b>: Determines the effective length rlen of the string to copy as
   //!   the smaller of
   //!
   //! <b>Throws</b>: out_of_range if pos1 > size() or pos2 > sv.size()
   //!
   //! <b>Returns</b>: basic_string(*this, pos1, n1).compare(BasicStringView<CharT, Traits>(sv, pos2, n2)).
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      int compare(size_type pos1, size_type n1, BasicStringView<CharT,Traits> sv, size_type pos2, size_type n2 = npos) const
   {
      if (pos1 > this->size() || pos2 > sv.size())
         throw_out_of_range("basic_string::compare out of range position");
      const pointer addr     = this->priv_addr() + pos1;
      const CharT * str_addr = sv.data() + pos2;
      return this->s_compare(addr, addr + difference_type(dtl::min_value(n1, this->size() - pos1)),
                       str_addr, str_addr + difference_type(dtl::min_value(n2, sv.size() - pos2)));
   }

   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: compare(basic_string(s)).
   BOOST_CONTAINER_NODISCARD inline
      int compare(const CharT* s) const
   {
      BOOST_ASSERT(s != 0);
      const_pointer const addr = this->priv_addr();
      return this->s_compare(addr, addr + difference_type(this->priv_size()), s, s + Traits::length(s));
   }

   //! <b>Requires</b>: pos1 <= size() and s points to an array of at least n2 elements of CharT.
   //!   If n2 is zero, s may be null.
   //!
   //! <b>Throws</b>: out_of_range if pos1 > size()
   //!
   //! <b>Returns</b>: basic_string(*this, pos, n1).compare(basic_string(s, n2)).
   BOOST_CONTAINER_NODISCARD inline
      int compare(size_type pos1, size_type n1, const CharT* s, size_type n2) const
   {
      BOOST_ASSERT(s != 0 || n2 == 0);
      if (pos1 > this->size())
         throw_out_of_range("basic_string::compare out of range position");
      const pointer addr = this->priv_addr();
      return this->s_compare( addr + difference_type(pos1),
                        addr + difference_type(pos1 + dtl::min_value(n1, this->size() - pos1)),
                        s, s + difference_type(n2));
   }

   //! <b>Requires</b>: pos1 > size() and s points to an array of at least traits::length(s) + 1 elements of CharT.
   //!
   //! <b>Throws</b>: out_of_range if pos1 > size()
   //!
   //! <b>Returns</b>: basic_string(*this, pos, n1).compare(basic_string(s, n2)).
   BOOST_CONTAINER_NODISCARD inline
      int compare(size_type pos1, size_type n1, const CharT* s) const
   {
      BOOST_ASSERT(s != 0);
      return this->compare(pos1, n1, s, Traits::length(s));
   }

   //! <b>Effects</b>: Equivalent to find(sv) != npos
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: true if the string contains the provided substring, false otherwise.
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      bool contains(BasicStringView<CharT, Traits> sv) const BOOST_NOEXCEPT
   {  return this->find(sv) != npos; }

   //! <b>Effects</b>: Equivalent to find(c) != npos
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: true if the string contains the provided substring, false otherwise.
   BOOST_CONTAINER_NODISCARD inline
      bool contains(CharT c) const BOOST_NOEXCEPT
   {  return this->find(c) != npos; }

   //! <b>Effects</b>: Equivalent to find(c) != npos
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: true if the string contains the provided substring, false otherwise.
   BOOST_CONTAINER_NODISCARD inline
      bool contains(const CharT* s) const BOOST_NOEXCEPT
   {
      return this->find(s) != npos; //find checks s != 0
   }

   //! <b>Effects</b>: Checks if the string begins with the given prefix
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: true if the string begins with the provided prefix, false otherwise.
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      bool starts_with(BasicStringView<CharT, Traits> sv) const BOOST_NOEXCEPT
   {
      const std::size_t s_sz = sv.size();
      const std::size_t t_sz = this->size();
      return t_sz >= s_sz && dtl::string_traits_compare<Traits>(this->data(), sv.data(), s_sz) == 0;
   }

   //! <b>Effects</b>: Checks if the string begins with the given prefix
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: true if the string begins with the provided prefix, false otherwise.
   BOOST_CONTAINER_NODISCARD inline
      bool starts_with(CharT c) const BOOST_NOEXCEPT
   {  return !empty() && Traits::eq(front(), c);  }

   //! <b>Effects</b>: Checks if the string begins with the given prefix
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: true if the string begins with the provided prefix, false otherwise.
   BOOST_CONTAINER_NODISCARD inline
      bool starts_with(const CharT* s) const BOOST_NOEXCEPT
   {
      BOOST_ASSERT(s != 0);
      const std::size_t s_sz = Traits::length(s);
      const std::size_t t_sz = this->size();
      return t_sz >= s_sz && Traits::compare(this->data(), s, s_sz) == 0;
   }

   //! <b>Effects</b>: Checks if the string begins with the given suffix
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: true if the string begins with the provided suffix, false otherwise.
   template<template <class, class> class BasicStringView>
   BOOST_CONTAINER_NODISCARD inline
      bool ends_with(BasicStringView<CharT, Traits> sv) const BOOST_NOEXCEPT
   {
      const std::size_t s_sz = sv.size();
      const std::size_t t_sz = this->size();
      return t_sz >= s_sz && dtl::string_traits_compare<Traits>(this->data() + std::ptrdiff_t(t_sz - s_sz), sv.data(), s_sz) == 0;
   }

   //! <b>Effects</b>: Checks if the string begins with the given suffix
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: true if the string begins with the provided suffix, false otherwise.
   BOOST_CONTAINER_NODISCARD inline
      bool ends_with(CharT c) const BOOST_NOEXCEPT
   {  return !empty() && Traits::eq(back(), c);  }

   //! <b>Effects</b>: Checks if the string begins with the given suffix
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Returns</b>: true if the string begins with the provided suffix, false otherwise.
   BOOST_CONTAINER_NODISCARD inline
      bool ends_with(const CharT* s) const BOOST_NOEXCEPT
   {
      BOOST_ASSERT(s != 0);
      const std::size_t s_sz = Traits::length(s);
      const std::size_t t_sz = this->size();
      return t_sz >= s_sz && Traits::compare(this->data() + std::ptrdiff_t(t_sz - s_sz), s, s_sz) == 0;
   }

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   private:
   void priv_move_assign(BOOST_RV_REF(basic_string) x, dtl::bool_<true> /*steal_resources*/)
   {
      if(!this->is_short()){
         this->priv_long_to_short();  //Free the memory with its current allocator
      }
      Traits::assign(*this->priv_short_addr(), CharT(0));
      this->priv_short_size(0);
      //Move allocator if needed
      dtl::bool_<allocator_traits_type::
         propagate_on_container_move_assignment::value> flag;
      dtl::move_alloc(this->alloc(), x.alloc(), flag);
      //Nothrow swap
      this->swap_data(x);
   }

   void priv_move_assign(BOOST_RV_REF(basic_string) x, dtl::bool_<false> /*steal_resources*/)
   {
      //We can't guarantee a compile-time equal allocator or propagation so fallback to runtime
      //Resources can be transferred if both allocators are equal
      if (this->alloc() == x.alloc()) {
         this->priv_move_assign(boost::move(x), dtl::true_());
      }
      else {
         this->assign(x.begin(), x.end());
      }
   }

   bool priv_reserve_no_null_end(size_type res_arg)
   {
      if (res_arg > this->max_size()){
         throw_length_error("basic_string::reserve max_size() exceeded");
      }

      pointer   addr;
      size_type old_size, old_storage;
      const bool was_short = this->priv_repr_data(addr, old_size, old_storage);
      const bool do_alloc = size_type(old_storage - 1u) < res_arg;
      if (do_alloc){
         size_type n = res_arg + 1;
         //next_storage takes the number of characters to add to the current size
         size_type new_storage = this->next_storage(size_type(res_arg - old_size), old_size, old_storage, this->priv_max_storage());
         pointer reuse = 0;
         pointer new_start = this->allocation_command(allocate_new, n, new_storage, reuse);

         size_type new_length = priv_uninitialized_copy
            (addr, addr + difference_type(old_size), new_start);
         this->priv_set_long_buffer(was_short, new_start, new_length, new_storage);
      }
      return do_alloc;
   }

   void priv_reserve(size_type res_arg)
   {
      //After a reallocation the buffer is always dynamic
      if(this->priv_reserve_no_null_end(res_arg))
         this->priv_construct_null(this->priv_long_addr() + difference_type(this->priv_long_size()));
   }

   template<class It1, class It2>
   static int s_compare(It1 f1, It1 l1, It2 f2, It2 l2)
   {
      const std::size_t n1 = std::size_t(l1 - f1);
      const std::size_t n2 = std::size_t(l2 - f2);
      const int cmp = dtl::string_traits_compare<Traits>
         (boost::movelib::to_raw_pointer(f1), boost::movelib::to_raw_pointer(f2), dtl::min_value(n1, n2));
      return cmp != 0 ? cmp : (n1 < n2 ? -1 : (n1 > n2 ? 1 : 0));
   }

   template<class AllocVersion>
   void priv_shrink_to_fit_dynamic_buffer
      ( AllocVersion
      , typename dtl::enable_if<dtl::is_same<AllocVersion, version_1> >::type* = 0)
   {
      //Allocate a new buffer.
      size_type real_cap = 0;
      const pointer   long_addr    = this->priv_long_addr();
      const size_type long_size    = this->priv_long_size();
      const size_type long_storage = this->priv_long_storage();
      //We can make this nothrow as chars are always NoThrowCopyables
      BOOST_CONTAINER_TRY{
         pointer reuse = 0;
         real_cap = long_size+1;
         const pointer ret = this->allocation_command(allocate_new, long_size+1, real_cap, reuse);
         //Copy and update
         Traits::copy( boost::movelib::to_raw_pointer(ret)
                     , boost::movelib::to_raw_pointer(long_addr)
                     , long_size+1);
         this->priv_long_addr(ret);
         this->priv_long_storage(real_cap);
         //And release old buffer
         this->alloc().deallocate(long_addr, long_storage);
      }
      BOOST_CONTAINER_CATCH(...){
         return;
      }
      BOOST_CONTAINER_CATCH_END
   }

   template<class AllocVersion>
   void priv_shrink_to_fit_dynamic_buffer
      ( AllocVersion
      , typename dtl::enable_if<dtl::is_same<AllocVersion, version_2> >::type* = 0)
   {
      size_type received_size = this->priv_long_size()+1;
      pointer hint = this->priv_long_addr();
      if(this->alloc().allocation_command
         ( shrink_in_place | nothrow_allocation, this->priv_long_storage(), received_size, hint)){
         this->priv_long_storage(received_size);
      }
   }

   inline void priv_construct_null(pointer p)
   {  traits_type::assign(*p, CharT());  }

   // Helper functions used by constructors.  It is a severe error for
   // any of them to be called anywhere except from within constructors.
   inline void priv_terminate_string()
   {  this->priv_construct_null(this->priv_end_addr());  }

   inline void priv_short_terminate()
   {
      BOOST_ASSERT(this->is_short());
      this->priv_construct_null(this->priv_short_addr() + difference_type(this->priv_short_size()));
   }

   //Sets the size to n and writes the null terminator with only one is_short() test.
   //Precondition: n <= capacity()
   inline void priv_size_and_terminate(size_type n)
   {
      pointer   addr;
      size_type old_size, storage;
      const bool short_repr = this->priv_repr_data(addr, old_size, storage);
      BOOST_ASSERT(n < storage);
      this->priv_size(short_repr, n);
      this->priv_construct_null(addr + difference_type(n));
   }

   //nvc++ (NVIDIA HPC SDK, at least up to 26.5) miscompiles the loop below when
   //it is inlined at -O2/-fast. See:
   //https://forums.developer.nvidia.com/t/nvc-26-5-o2-miscompilation-wrong-final-value-for-a-loop-counter-when-a-copy-loop-is-inlined/381529
   #if defined(__NVCOMPILER)
   template<class InpIt, class FwdIt> BOOST_NOINLINE
   #else
   template<class InpIt, class FwdIt> inline
   #endif
   size_type priv_uninitialized_copy(InpIt first, InpIt last, FwdIt dest)
   {
      //Save initial destination position
      size_type constructed = 0;

      for (; first != last; ++dest, ++first, ++constructed){
         traits_type::assign(*dest, *first);
      }
      return constructed;
   }

   template <class InputIterator, class OutIterator>
   static void priv_copy(InputIterator first, InputIterator last, OutIterator result)
   {
      for ( ; first != last; ++first, ++result)
         Traits::assign(*result, *first);
   }

   static inline void priv_copy( const CharT* first, const CharT* last, CharT* result)
   {  dtl::string_traits_copy<Traits>(result, first, std::size_t(last - first));  }

   //True if reading [first, last) can throw or [first, last) is a part of *this.
   //Then the characters must be read before the old characters move.
   template<class ForwardIter>
   static bool priv_is_throwing_or_aliased_source(ForwardIter, size_type, const CharT *, size_type)
   {  return true;  }

   static bool priv_is_throwing_or_aliased_source(const cvalue_iterator &, size_type, const CharT *, size_type)
   {  return false;  }

   static bool priv_is_throwing_or_aliased_source(const CharT *first, size_type n, const CharT *buf, size_type buf_size)
   {  return n && first < buf + difference_type(buf_size + 1u) && buf < first + difference_type(n);  }

   //True if [first, last) can be read through a const CharT pointer: contiguous iterators
   //(including the allocator's pointer types) whose value type is CharT
   template<class ForwardIter>
   struct is_char_contiguous_iterator
   {
      BOOST_STATIC_CONSTEXPR bool value =
         ( boost::movelib::is_contiguous_iterator<ForwardIter>::value
         || dtl::is_same<ForwardIter, pointer>::value
         || dtl::is_same<ForwardIter, const_pointer>::value )
         && dtl::is_same<typename boost::container::iterator_traits<ForwardIter>::value_type, CharT>::value;
   };

   //Replaces the n1 characters at pos with the n characters of [first, last).
   //Strong exception guarantee: *this is changed only after all characters are read and the
   //memory is allocated. [first, last) can be a part of *this.
   template<class ForwardIter>
   BOOST_CONTAINER_FORCEINLINE void priv_replace_range(size_type pos, size_type n1, ForwardIter first, ForwardIter last, size_type n)
   {
      this->priv_replace_range(pos, n1, first, last, n, dtl::bool_<is_char_contiguous_iterator<ForwardIter>::value>());
   }

   //Contiguous iterators: read the characters through a pointer, which can't throw and
   //can be checked for aliasing. Only an element (not an end iterator) is converted
   template<class ForwardIter>
   BOOST_CONTAINER_FORCEINLINE void priv_replace_range(size_type pos, size_type n1, ForwardIter first, ForwardIter, size_type n, dtl::true_)
   {
      const CharT * const f = n ? boost::movelib::iterator_to_raw_pointer(first) : static_cast<const CharT *>(0);
      this->priv_replace_range_core(pos, n1, f, f + difference_type(n), n);
   }

   template<class ForwardIter>
   BOOST_CONTAINER_FORCEINLINE void priv_replace_range(size_type pos, size_type n1, ForwardIter first, ForwardIter last, size_type n, dtl::false_)
   {
      this->priv_replace_range_core(pos, n1, first, last, n);
   }

   //This function has only the in-place code, so that it can be inlined.
   template<class ForwardIter>
   BOOST_CONTAINER_FORCEINLINE void priv_replace_range_core(size_type pos, size_type n1, ForwardIter first, ForwardIter last, size_type n)
   {
      pointer   old_start;
      size_type old_size, old_storage;
      const bool was_short = this->priv_repr_data(old_start, old_size, old_storage);
      BOOST_ASSERT(pos <= old_size && n1 <= old_size - pos);
      const size_type max_sz = this->max_size();
      if (n > max_sz || (old_size - n1) > (max_sz - n))
         throw_length_error("basic_string: max_size() exceeded");
      const size_type new_size = old_size - n1 + n;
      CharT * const old_buf    = boost::movelib::to_raw_pointer(old_start);
      const bool throwing_or_aliased = priv_is_throwing_or_aliased_source(first, n, old_buf, old_size);
      //Position of the temporary copy of [first, last): after both null terminators (old and new)
      const size_type tmp_copy_pos = (new_size > old_size ? new_size : old_size) + 1u;
      //A throwing or aliased source needs space for the temporary copy of the n characters at
      //tmp_copy_pos. Other sources need space only for new_size characters and the null terminator
      const size_type min_storage = throwing_or_aliased ? size_type(tmp_copy_pos + n) : size_type(new_size + 1u);
      const bool fits_in_place = min_storage <= old_storage;

      if (!fits_in_place && this->priv_replace_range_grow(pos, n1, first, last, n, min_storage, throwing_or_aliased)) {
         return;
      }

      //In place (maybe after a forward expansion: the characters did not move)
      const size_type tail = old_size - pos - n1;
      if (throwing_or_aliased) {
         //Make the temporary copy in the free capacity: if this throws, *this is not changed
         priv_uninitialized_copy(first, last, old_buf + difference_type(tmp_copy_pos));
      }
      //Nothing below can throw
      if (n != n1) {
         Traits::move(old_buf + difference_type(pos + n), old_buf + difference_type(pos + n1), tail);
      }
      if (throwing_or_aliased) {
         Traits::copy(old_buf + difference_type(pos), old_buf + difference_type(tmp_copy_pos), n);
      }
      else {
         priv_copy(first, last, old_buf + difference_type(pos));
      }
      //A forward expansion is done only on a dynamic buffer, so the representation did not change
      this->priv_size(was_short, new_size);
      this->priv_construct_null(old_start + difference_type(new_size));
   }

   //Part of priv_replace_range when the free capacity is too small: the buffer has less than
   //min_storage characters. Expands the buffer or allocates a new buffer. Returns false if the
   //buffer was expanded forward (the characters did not move and the replacement must be done in
   //place), else does the replacement and returns true.
   template<class ForwardIter>
   BOOST_CONTAINER_NOINLINE bool priv_replace_range_grow
      ( size_type pos, size_type n1, ForwardIter first, ForwardIter last, size_type n
      , size_type min_storage, bool throwing_or_aliased)
   {
      pointer   old_start;
      size_type old_size, old_storage;
      const bool was_short = this->priv_repr_data(old_start, old_size, old_storage);
      const size_type new_size = old_size - n1 + n;
      const size_type tail     = old_size - pos - n1;
      CharT * const old_buf    = boost::movelib::to_raw_pointer(old_start);
      //Grow from min_storage, so that the next operations also have free capacity
      size_type new_storage = this->next_storage
         (size_type(min_storage - 1u - old_size), old_size, old_storage, this->priv_max_storage());
      if (new_storage <= InternalBufferChars)
         new_storage = InternalBufferChars + 1u;
      //The internal buffer can not be expanded.
      //Backward expansion moves the old characters before [first, last) is read, so it is
      //not used if reading [first, last) can throw or [first, last) is a part of *this
      const allocation_type command = was_short ? allocate_new
         : throwing_or_aliased ? (allocate_new | expand_fwd) : (allocate_new | expand_fwd | expand_bwd);
      pointer reuse = was_short ? pointer() : old_start;
      const pointer new_start = this->allocation_command(command, min_storage, new_storage, reuse);
      BOOST_IF_CONSTEXPR(alloc_version::value != 1){
         if (reuse) {
            if (new_start == old_start) {
               //Expanded forward: the buffer has at least min_storage characters
               this->priv_long_storage(new_storage);
               return false;
            }
            //Expanded backward (and maybe forward): nothing can throw.
            //Move the prefix first: the new place of the tail can overlap the old prefix
            CharT * const new_buf = boost::movelib::to_raw_pointer(new_start);
            Traits::move(new_buf, old_buf, pos);
            Traits::move(new_buf + difference_type(pos + n), old_buf + difference_type(pos + n1), tail);
            priv_copy(first, last, new_buf + difference_type(pos));
            this->priv_construct_null(new_start + difference_type(new_size));
            this->priv_long_addr(new_start);
            this->priv_long_size(new_size);
            this->priv_long_storage(new_storage);
            return true;
         }
      }
      //New buffer: [first, last) is read before the old buffer is freed
      CharT * const new_buf = boost::movelib::to_raw_pointer(new_start);
      BOOST_CONTAINER_TRY{
         priv_uninitialized_copy(first, last, new_buf + difference_type(pos));
      }
      BOOST_CONTAINER_CATCH(...){
         this->deallocate(new_start, new_storage);
         BOOST_CONTAINER_RETHROW
      }
      BOOST_CONTAINER_CATCH_END
      Traits::copy(new_buf, old_buf, pos);
      Traits::copy(new_buf + difference_type(pos + n), old_buf + difference_type(pos + n1), tail);
      this->priv_construct_null(new_start + difference_type(new_size));
      this->priv_set_long_buffer(was_short, new_start, new_size, new_storage);
      return true;
   }

   //Like priv_copy, but [first, last) and the destination can overlap
   template <class InputIterator, class OutIterator>
   static void priv_copy_overlapping(InputIterator first, InputIterator last, OutIterator result)
   {  priv_copy(first, last, result);  }

   static inline void priv_copy_overlapping( const CharT* first, const CharT* last, CharT* result)
   {  dtl::string_traits_move<Traits>(result, first, std::size_t(last - first));  }

   static inline void priv_copy_overlapping( CharT* first, CharT* last, CharT* result)
   {  dtl::string_traits_move<Traits>(result, first, std::size_t(last - first));  }

   //Debug checks of the precondition "a range must not refer to characters of the
   //string" (see "Arguments that refer to elements of the same container" in the
   //documentation). Only ranges of raw pointers are checked.
   template <class Iterator>
   static bool priv_range_overlaps(Iterator, Iterator, const CharT*, const CharT*)
   {  return false;  }

   static bool priv_range_overlaps(const CharT *f, const CharT *l, const CharT *b, const CharT *e)
   {
      //Compare addresses as integers: the pointers can point to different objects
      return reinterpret_cast<std::size_t>(f) < reinterpret_cast<std::size_t>(e)
          && reinterpret_cast<std::size_t>(b) < reinterpret_cast<std::size_t>(l);
   }

   static bool priv_range_overlaps(CharT *f, CharT *l, const CharT *b, const CharT *e)
   {  return priv_range_overlaps(const_cast<const CharT*>(f), const_cast<const CharT*>(l), b, e);  }

   template <class Integer>
   inline basic_string& priv_replace_dispatch(const_iterator first, const_iterator last,
                                       Integer n, Integer x,
                                       dtl::true_)
   {  return this->replace(first, last, (size_type) n, (CharT) x);   }

   template <class InputIter>
   inline basic_string& priv_replace_dispatch(const_iterator first, const_iterator last,
                                       InputIter f, InputIter l,
                                       dtl::false_)
   {
      typedef typename boost::container::iterator_traits<InputIter>::iterator_category Category;
      return this->priv_replace(first, last, f, l, Category());
   }

   #endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
};

#ifndef BOOST_CONTAINER_NO_CXX17_CTAD

//! <b>Deduction guide</b>: allows a `basic_string` to be constructed from the
//! iterator range <code>[first, last)</code>, deducing the character type from the
//! value type of `InputIterator` and using the default allocator.
template <typename InputIterator>
basic_string(InputIterator, InputIterator) ->
   basic_string<typename iterator_traits<InputIterator>::value_type>;

//! <b>Deduction guide</b>: allows a `basic_string` to be constructed from the
//! iterator range <code>[first, last)</code>, deducing the character type from the
//! value type of `InputIterator` and taking the allocator type from the supplied allocator.
template <typename InputIterator, typename Allocator>
basic_string(InputIterator, InputIterator, Allocator const&) ->
   basic_string<typename iterator_traits<InputIterator>::value_type, Allocator>;

#endif

#ifdef BOOST_CONTAINER_DOXYGEN_INVOKED

//!Typedef for a basic_string of
//!narrow characters
typedef basic_string
   <char
   ,std::char_traits<char>
   ,new_allocator<char> >
string;

//!Typedef for a basic_string of
//!narrow characters
typedef basic_string
   <wchar_t
   ,std::char_traits<wchar_t>
   ,new_allocator<wchar_t> >
wstring;

#else

template <class CharT, class Traits, class Allocator, class Options>
const typename basic_string<CharT, Traits, Allocator, Options>::size_type
    basic_string<CharT, Traits, Allocator, Options>::npos;

template<class S>
struct is_string
{
   BOOST_STATIC_CONSTEXPR bool value = false;
};

template<class C, class T, class A>
struct is_string< basic_string<C, T, A> >
{
   BOOST_STATIC_CONSTEXPR bool value = true;
};

#endif

// ------------------------------------------------------------
// Non-member functions.

// Operator+

template <class CharT, class Traits, class Allocator, class Options> inline
   basic_string<CharT,Traits,Allocator,Options>
   operator+(const basic_string<CharT,Traits,Allocator,Options>& x
            ,const basic_string<CharT,Traits,Allocator,Options>& y)
{
   typedef basic_string<CharT,Traits,Allocator,Options> str_t;
   typedef typename str_t::reserve_t reserve_t;
   reserve_t reserve;
   str_t result(reserve, x.size() + y.size(), x.get_stored_allocator());
   result.append(x);
   result.append(y);
   return result;
}

template <class CharT, class Traits, class Allocator, class Options> inline
   basic_string<CharT, Traits, Allocator, Options> operator+
      ( BOOST_RV_REF_BEG basic_string<CharT, Traits, Allocator, Options> BOOST_RV_REF_END x
      , BOOST_RV_REF_BEG basic_string<CharT, Traits, Allocator, Options> BOOST_RV_REF_END y)
{
   x += y;
   return boost::move(x);
}

template <class CharT, class Traits, class Allocator, class Options> inline
   basic_string<CharT, Traits, Allocator, Options> operator+
      ( BOOST_RV_REF_BEG basic_string<CharT, Traits, Allocator, Options> BOOST_RV_REF_END x
      , const basic_string<CharT,Traits,Allocator,Options>& y)
{
   x += y;
   return boost::move(x);
}

template <class CharT, class Traits, class Allocator, class Options> inline
   basic_string<CharT, Traits, Allocator, Options> operator+
      (const basic_string<CharT,Traits,Allocator,Options>& x
      ,BOOST_RV_REF_BEG basic_string<CharT, Traits, Allocator, Options> BOOST_RV_REF_END y)
{
   y.insert(y.begin(), x.begin(), x.end());
   return boost::move(y);
}

template <class CharT, class Traits, class Allocator, class Options> inline
   basic_string<CharT, Traits, Allocator, Options> operator+
      (const CharT* s, basic_string<CharT, Traits, Allocator, Options> y)
{
   BOOST_ASSERT(s != 0);
   y.insert(y.begin(), s, s + Traits::length(s));
   return y;
}

template <class CharT, class Traits, class Allocator, class Options> inline
   basic_string<CharT,Traits,Allocator,Options> operator+
      (basic_string<CharT,Traits,Allocator,Options> x, const CharT* s)
{
   x += s;  //operator+= checks s != 0
   return x;
}

template <class CharT, class Traits, class Allocator, class Options> inline
   basic_string<CharT,Traits,Allocator,Options> operator+
      (CharT c, basic_string<CharT,Traits,Allocator,Options> y)
{
   y.insert(y.begin(), c);
   return y;
}

template <class CharT, class Traits, class Allocator, class Options> inline
   basic_string<CharT,Traits,Allocator,Options> operator+
      (basic_string<CharT,Traits,Allocator,Options> x, const CharT c)
{
   x += c;
   return x;
}

// Operator== and operator!=

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator==(const basic_string<CharT,Traits,Allocator,Options>& x, const basic_string<CharT,Traits,Allocator,Options>& y)
{
   return x.size() == y.size() &&
          Traits::compare(x.data(), y.data(), x.size()) == 0;
}

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator==(const CharT* s, const basic_string<CharT,Traits,Allocator,Options>& y)
{
   BOOST_ASSERT(s != 0);
   typename basic_string<CharT,Traits,Allocator,Options>::size_type n = Traits::length(s);
   return n == y.size() && Traits::compare(s, y.data(), n) == 0;
}

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator==(const basic_string<CharT,Traits,Allocator,Options>& x, const CharT* s)
{
   BOOST_ASSERT(s != 0);
   typename basic_string<CharT,Traits,Allocator,Options>::size_type n = Traits::length(s);
   return x.size() == n && Traits::compare(x.data(), s, n) == 0;
}

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline 
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
      operator==( BasicStringView<CharT,Traits> x, const basic_string<CharT,Traits,Allocator,Options>& y)
{
   return x.size() == y.size() &&
          dtl::string_traits_compare<Traits>(x.data(), y.data(), x.size()) == 0;
}

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
      operator==( const basic_string<CharT,Traits,Allocator,Options>& x, BasicStringView<CharT,Traits> y)
{
   return x.size() == y.size() &&
          dtl::string_traits_compare<Traits>(x.data(), y.data(), x.size()) == 0;
}

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator!=(const basic_string<CharT,Traits,Allocator,Options>& x, const basic_string<CharT,Traits,Allocator,Options>& y)
   {  return !(x == y);  }

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator!=(const CharT* s, const basic_string<CharT,Traits,Allocator,Options>& y)
{
   return !(s == y);
}

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator!=(const basic_string<CharT,Traits,Allocator,Options>& x, const CharT* s)
{
   return !(x == s);
}

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator!=( BasicStringView<CharT,Traits> x, const basic_string<CharT,Traits,Allocator,Options>& y)
   {  return !(x == y);  }

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator!=( const basic_string<CharT,Traits,Allocator,Options>& x, BasicStringView<CharT,Traits> y)
   {  return !(x == y);  }

// Operator< (and also >, <=, and >=).
template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator<(const basic_string<CharT,Traits,Allocator,Options>& x, const basic_string<CharT,Traits,Allocator,Options>& y)
{
   return x.compare(y) < 0;
}

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator<(const CharT* s, const basic_string<CharT,Traits,Allocator,Options>& y)
{
   return y.compare(s) > 0;
}

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator<(const basic_string<CharT,Traits,Allocator,Options>& x, const CharT* s)
{
   return x.compare(s) < 0;
}

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator<( BasicStringView<CharT,Traits> x, const basic_string<CharT,Traits,Allocator,Options>& y)
   {  return y.compare(x) > 0;  }

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator<(  const basic_string<CharT,Traits,Allocator,Options>& x, BasicStringView<CharT,Traits> y)
   {  return x.compare(y) < 0;  }

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator>(const basic_string<CharT,Traits,Allocator,Options>& x, const basic_string<CharT,Traits,Allocator,Options>& y)
{  return y < x;  }

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator>(const CharT* s, const basic_string<CharT,Traits,Allocator,Options>& y)
{  return y < s;  }

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator>(const basic_string<CharT,Traits,Allocator,Options>& x, const CharT* s)
{  return s < x;  }

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator>( BasicStringView<CharT,Traits> x, const basic_string<CharT,Traits,Allocator,Options>& y)
   {  return y < x;  }

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator>( const basic_string<CharT,Traits,Allocator,Options>& x, BasicStringView<CharT,Traits> y)
   {  return y < x;  }

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator<=(const basic_string<CharT,Traits,Allocator,Options>& x, const basic_string<CharT,Traits,Allocator,Options>& y)
{
  return !(y < x);
}

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator<=(const CharT* s, const basic_string<CharT,Traits,Allocator,Options>& y)
   {  return !(y < s);  }

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator<=(const basic_string<CharT,Traits,Allocator,Options>& x, const CharT* s)
   {  return !(s < x);  }


template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator<=( BasicStringView<CharT,Traits> x, const basic_string<CharT,Traits,Allocator,Options>& y)
   {  return !(y < x);  }

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator<=( const basic_string<CharT,Traits,Allocator,Options>& x, BasicStringView<CharT,Traits> y)
   {  return !(y < x);  }

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator>=(const basic_string<CharT,Traits,Allocator,Options>& x,
           const basic_string<CharT,Traits,Allocator,Options>& y)
   {  return !(x < y);  }

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator>=(const CharT* s, const basic_string<CharT,Traits,Allocator,Options>& y)
   {  return !(s < y);  }

template <class CharT, class Traits, class Allocator, class Options>
inline bool
operator>=(const basic_string<CharT,Traits,Allocator,Options>& x, const CharT* s)
   {  return !(x < s);  }

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator>=( BasicStringView<CharT,Traits> x, const basic_string<CharT,Traits,Allocator,Options>& y)
   {  return !(x < y);  }

template <class CharT, class Traits, class Allocator, class Options, template <class, class> class BasicStringView>
inline
   BOOST_CONTAINER_DOC1ST( bool, 
                           typename dtl::disable_if
                              <is_string< BasicStringView<CharT BOOST_MOVE_I Traits> > BOOST_MOVE_I bool >::type)
operator>=( const basic_string<CharT,Traits,Allocator,Options>& x, BasicStringView<CharT,Traits> y)
   {  return !(x < y);  }

// Swap.
template <class CharT, class Traits, class Allocator, class Options>
inline void swap(basic_string<CharT,Traits,Allocator,Options>& x, basic_string<CharT,Traits,Allocator,Options>& y)
    BOOST_NOEXCEPT_IF(BOOST_NOEXCEPT(x.swap(y)))
{  x.swap(y);  }

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
// I/O.
namespace dtl {

template <class CharT, class Traits>
inline bool
string_fill(std::basic_ostream<CharT, Traits>& os,
                  std::basic_streambuf<CharT, Traits>* buf,
                  std::size_t n)
{
   CharT f = os.fill();
   std::size_t i;
   bool ok = true;

   for (i = 0; i < n; i++)
      ok = ok && !Traits::eq_int_type(buf->sputc(f), Traits::eof());
   return ok;
}

}  //namespace dtl {
#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

template <class CharT, class Traits, class Allocator, class Options>
std::basic_ostream<CharT, Traits>&
operator<<(std::basic_ostream<CharT, Traits>& os, const basic_string<CharT,Traits,Allocator,Options>& s)
{
   typename std::basic_ostream<CharT, Traits>::sentry sentry(os);
   bool ok = false;

   if (sentry) {
      ok = true;
      typename basic_string<CharT,Traits,Allocator,Options>::size_type n = s.size();
      typename basic_string<CharT,Traits,Allocator,Options>::size_type pad_len = 0;
      const bool left = (os.flags() & std::ios::left) != 0;
      const std::size_t w = static_cast<std::size_t>(os.width(0));
      std::basic_streambuf<CharT, Traits>* buf = os.rdbuf();

      if (w != 0 && n < w)
         pad_len = w - n;

      if (!left)
         ok = dtl::string_fill(os, buf, pad_len);

      ok = ok &&
            buf->sputn(s.data(), std::streamsize(n)) == std::streamsize(n);

      if (left)
         ok = ok && dtl::string_fill(os, buf, pad_len);
   }

   if (!ok)
      os.setstate(std::ios_base::failbit);

   return os;
}


template <class CharT, class Traits, class Allocator, class Options>
std::basic_istream<CharT, Traits>&
operator>>(std::basic_istream<CharT, Traits>& is, basic_string<CharT,Traits,Allocator,Options>& s)
{
   typename std::basic_istream<CharT, Traits>::sentry sentry(is);

   if (sentry) {
      std::basic_streambuf<CharT, Traits>* buf = is.rdbuf();
      const std::ctype<CharT>& ctype = std::use_facet<std::ctype<CharT> >(is.getloc());

      s.clear();
      std::size_t n = static_cast<std::size_t>(is.width(0));
      if (n == 0)
         n = static_cast<std::size_t>(-1);
      else
         s.reserve(n);

      while (n-- > 0) {
         typename Traits::int_type c1 = buf->sbumpc();

         if (Traits::eq_int_type(c1, Traits::eof())) {
            is.setstate(std::ios_base::eofbit);
            break;
         }
         else {
            CharT c = Traits::to_char_type(c1);

            if (ctype.is(std::ctype<CharT>::space, c)) {
               if (Traits::eq_int_type(buf->sputbackc(c), Traits::eof()))
                  is.setstate(std::ios_base::failbit);
               break;
            }
            else
               s.push_back(c);
         }
      }

      // If we have read no characters, then set failbit.
      if (s.size() == 0)
         is.setstate(std::ios_base::failbit);
   }
   else
      is.setstate(std::ios_base::failbit);

   return is;
}

template <class CharT, class Traits, class Allocator, class Options>
std::basic_istream<CharT, Traits>&
getline(std::istream& is, basic_string<CharT,Traits,Allocator,Options>& s,CharT delim)
{
   typename basic_string<CharT,Traits,Allocator,Options>::size_type nread = 0;
   typename std::basic_istream<CharT, Traits>::sentry sentry(is, true);
   if (sentry) {
      std::basic_streambuf<CharT, Traits>* buf = is.rdbuf();
      s.clear();

      while (nread < s.max_size()) {
         int c1 = buf->sbumpc();
         if (Traits::eq_int_type(c1, Traits::eof())) {
            is.setstate(std::ios_base::eofbit);
            break;
         }
         else {
            ++nread;
            CharT c = Traits::to_char_type(c1);
            if (!Traits::eq(c, delim))
               s.push_back(c);
            else
               break;              // Character is extracted but not appended.
         }
      }
   }
   if (nread == 0 || nread >= s.max_size())
      is.setstate(std::ios_base::failbit);

   return is;
}

template <class CharT, class Traits, class Allocator, class Options>
inline std::basic_istream<CharT, Traits>&
getline(std::basic_istream<CharT, Traits>& is, basic_string<CharT,Traits,Allocator,Options>& s)
{
   return getline(is, s, '\n');
}

//! <b>Effects</b>: Erases all elements that compare equal to v from the container c.
//!
//! <b>Complexity</b>: Linear.
template <class T, class Tr, class A, class U>
inline typename basic_string<T, Tr, A>::size_type erase(basic_string<T, Tr, A>& c, const U& v)
{
   typename basic_string<T, Tr, A>::size_type old_size = c.size();
   c.erase(boost::container::remove(c.begin(), c.end(), v), c.end());
   return old_size - c.size();
}

//! <b>Effects</b>: Erases all elements that satisfy the predicate pred from the container c.
//!
//! <b>Complexity</b>: Linear.
template <class T, class Tr, class A, class Pred>
inline typename basic_string<T, Tr, A>::size_type erase_if(basic_string<T, Tr, A>& c, Pred pred)
{
   typename basic_string<T, Tr, A>::size_type old_size = c.size();
   c.erase(boost::container::remove_if(c.begin(), c.end(), pred), c.end());
   return old_size - c.size();
}

}}

//GCC 12 has a regression for array-bounds warnings
#if defined(BOOST_GCC) && (BOOST_GCC >= 120000) && (BOOST_GCC < 130000)
#pragma GCC diagnostic pop
#endif


#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

namespace boost {

//!has_trivial_destructor_after_move<> specialization.
template <class C, class T, class Allocator, class Options>
struct has_trivial_destructor_after_move<boost::container::basic_string<C, T, Allocator, Options> >
{
   typedef typename boost::container::basic_string<C, T, Allocator, Options>::allocator_type allocator_type;
   typedef boost::container::allocator_traits<allocator_type> allocator_traits_type;
   typedef typename allocator_traits_type::pointer pointer;
   BOOST_STATIC_CONSTEXPR bool value =
      allocator_traits_type::is_always_equal::value &&
      ::boost::has_trivial_destructor_after_move<allocator_type>::value &&
      ::boost::has_trivial_destructor_after_move<pointer>::value;
};

}  //namespace boost {

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_STRING_HPP
