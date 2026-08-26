//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_PMR_MONOTONIC_BUFFER_RESOURCE_HPP
#define BOOST_CONTAINER_PMR_MONOTONIC_BUFFER_RESOURCE_HPP

#if defined (_MSC_VER)
#  pragma once 
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/container_fwd.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/detail/block_slist.hpp>

//Inline implementation
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/detail/min_max.hpp>
#include <boost/intrusive/detail/math.hpp>
#include <boost/cstdint.hpp>

#include <cstddef>

namespace boost {
namespace container {
namespace pmr {

//! A monotonic_buffer_resource is a special-purpose memory resource intended for
//! very fast memory allocations in situations where memory is used to build up a
//! few objects and then is released all at once when the memory resource object
//! is destroyed. It has the following qualities:
//! 
//! - A call to deallocate has no effect, thus the amount of memory consumed
//!   increases monotonically until the resource is destroyed.
//! 
//! - The program can supply an initial buffer, which the allocator uses to satisfy
//!   memory requests.
//! 
//! - When the initial buffer (if any) is exhausted, it obtains additional buffers
//!   from an upstream memory resource supplied at construction. Each additional
//!   buffer is larger than the previous one, following a geometric progression.
//! 
//! - It is intended for access from one thread of control at a time. Specifically,
//!   calls to allocate and deallocate do not synchronize with one another.
//! 
//! - It owns the allocated memory and frees it on destruction, even if deallocate has
//!   not been called for some of the allocated blocks.
#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

namespace mbr_dtl {

#ifdef BOOST_HAS_INTPTR_T
typedef boost::uintptr_t   uintptr_type;
#else
typedef std::size_t        uintptr_type;
#endif

BOOST_CONTAINER_CONSTANT_VAR std::size_t minimum_buffer_size = 2*sizeof(void*);

}  //namespace mbr_dtl {

#endif   //BOOST_CONTAINER_DOXYGEN_INVOKED

class monotonic_buffer_resource
   : public memory_resource
{
   block_slist       m_memory_blocks;
   void *            m_current_buffer;
   std::size_t       m_current_buffer_size;
   std::size_t       m_next_buffer_size;
   void * const      m_initial_buffer;
   std::size_t const m_initial_buffer_size;

   /// @cond
   void increase_next_buffer();
   void increase_next_buffer_at_least_to(std::size_t minimum_size);
   void *allocate_from_current(std::size_t aligner, std::size_t bytes);
   /// @endcond

   public:

   //! The number of bytes that will be requested by the default in the first call
   //! to the upstream allocator
   //!
   //! <b>Note</b>: Non-standard extension.
   BOOST_STATIC_CONSTEXPR std::size_t initial_next_buffer_size = 32u*sizeof(void*);

   //! <b>Requires</b>: `upstream` shall be the address of a valid memory resource or `nullptr`
   //!
   //! <b>Effects</b>: If `upstream` is not nullptr, sets the internal resource to `upstream`,
   //!   to get_default_resource() otherwise.
   //!   Sets the internal `current_buffer` to `nullptr` and the internal `next_buffer_size` to an
   //!   implementation-defined size.
   explicit monotonic_buffer_resource(memory_resource* upstream = 0) BOOST_NOEXCEPT;

   //! <b>Requires</b>: `upstream` shall be the address of a valid memory resource or `nullptr`
   //!   and `initial_size` shall be greater than zero.
   //!
   //! <b>Effects</b>: If `upstream` is not nullptr, sets the internal resource to `upstream`,
   //!   to get_default_resource() otherwise. Sets the internal `current_buffer` to `nullptr` and
   //!   `next_buffer_size` to at least `initial_size`.
   explicit monotonic_buffer_resource(std::size_t initial_size, memory_resource* upstream = 0) BOOST_NOEXCEPT;

   //! <b>Requires</b>: `upstream` shall be the address of a valid memory resource or `nullptr`,
   //!   `buffer_size` shall be no larger than the number of bytes in buffer.
   //!
   //! <b>Effects</b>: If `upstream` is not nullptr, sets the internal resource to `upstream`,
   //!   to get_default_resource() otherwise. Sets the internal `current_buffer` to `buffer`,
   //!   and `next_buffer_size` to `buffer_size` (but not less than an implementation-defined size),
   //!   then increases `next_buffer_size` by an implementation-defined growth factor (which need not be integral).
   monotonic_buffer_resource(void* buffer, std::size_t buffer_size, memory_resource* upstream = 0) BOOST_NOEXCEPT;

   #if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   monotonic_buffer_resource(const monotonic_buffer_resource&) = delete;
   monotonic_buffer_resource operator=(const monotonic_buffer_resource&) = delete;
   #else
   private:
   monotonic_buffer_resource          (const monotonic_buffer_resource&);
   monotonic_buffer_resource operator=(const monotonic_buffer_resource&);
   public:
   #endif

   //! <b>Effects</b>: Calls
   //!   `this->release()`.
   ~monotonic_buffer_resource() BOOST_OVERRIDE;

   //! <b>Effects</b>: `upstream_resource()->deallocate()` as necessary to release all allocated memory.
   //!   Resets *this to its initial state at construction.
   //!   [Note: memory is released back to `upstream_resource()` even if some blocks that were allocated
   //!   from this have not been deallocated from this. - end note]
   void release() BOOST_NOEXCEPT;

   //! <b>Returns</b>: The value of
   //!   the internal resource.
   memory_resource* upstream_resource() const BOOST_NOEXCEPT;

   //! <b>Returns</b>:
   //!   The number of bytes of storage available for the specified alignment and
   //!   the number of bytes wasted due to the requested alignment.
   //!
   //! <b>Note</b>: Non-standard extension.
   std::size_t remaining_storage(std::size_t alignment, std::size_t &wasted_due_to_alignment) const BOOST_NOEXCEPT;
   
   //! <b>Returns</b>:
   //!   The number of bytes of storage available for the specified alignment.
   //!
   //! <b>Note</b>: Non-standard extension.
   std::size_t remaining_storage(std::size_t alignment = 1u) const BOOST_NOEXCEPT;

   //! <b>Returns</b>:
   //!   The address pointing to the start of the current free storage.
   //!
   //! <b>Note</b>: Non-standard extension.
   const void *current_buffer() const BOOST_NOEXCEPT;

   //! <b>Returns</b>:
   //!   The number of bytes that will be requested for the next buffer once the
   //!   current one is exhausted.
   //!
   //! <b>Note</b>: Non-standard extension.
   std::size_t next_buffer_size() const BOOST_NOEXCEPT;

   protected:

   //! <b>Returns</b>: A pointer to allocated storage with a size of at least `bytes`. The size
   //!   and alignment of the allocated memory shall meet the requirements for a class derived
   //!   from `memory_resource`.
   //!
   //! <b>Effects</b>: If the unused space in the internal `current_buffer` can fit a block with the specified
   //!   bytes and alignment, then allocate the return block from the internal `current_buffer`; otherwise sets
   //!   the internal `current_buffer` to `upstream_resource()->allocate(n, m)`, where `n` is not less than
   //!   `max(bytes, next_buffer_size)` and `m` is not less than alignment, and increase
   //!   `next_buffer_size` by an implementation-defined growth factor (which need not be integral),
   //!   then allocate the return block from the newly-allocated internal `current_buffer`.
   //!
   //! <b>Throws</b>: Nothing unless `upstream_resource()->allocate()` throws.
   virtual void* do_allocate(std::size_t bytes, std::size_t alignment) BOOST_OVERRIDE;

   //! <b>Effects</b>: None
   //!
   //! <b>Throws</b>: Nothing
   //!
   //! <b>Remarks</b>: Memory used by this resource increases monotonically until its destruction.
   virtual void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) BOOST_NOEXCEPT BOOST_OVERRIDE;

   //! <b>Returns</b>:
   //!   `this == dynamic_cast<const monotonic_buffer_resource*>(&other)`.
   virtual bool do_is_equal(const memory_resource& other) const BOOST_NOEXCEPT BOOST_OVERRIDE;
};

#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

//////////////////////////////////////////////////////////////////////////////
//
//    Inline implementation (formerly src/monotonic_buffer_resource.cpp)
//
//////////////////////////////////////////////////////////////////////////////

inline void monotonic_buffer_resource::increase_next_buffer()
{
   m_next_buffer_size = (std::size_t(-1)/2 < m_next_buffer_size) ? std::size_t(-1) : m_next_buffer_size*2;
}

inline void monotonic_buffer_resource::increase_next_buffer_at_least_to(std::size_t minimum_size)
{
   if(m_next_buffer_size < minimum_size){
      if(bi::detail::is_pow2(minimum_size)){
         m_next_buffer_size = minimum_size;
      }
      else if(std::size_t(-1)/2 < minimum_size){
         m_next_buffer_size = minimum_size;
      }
      else{
         m_next_buffer_size = bi::detail::ceil_pow2(minimum_size);
      }
   }
}

inline monotonic_buffer_resource::monotonic_buffer_resource(memory_resource* upstream) BOOST_NOEXCEPT
   : m_memory_blocks(upstream ? *upstream : *get_default_resource())
   , m_current_buffer(0)
   , m_current_buffer_size(0u)
   , m_next_buffer_size(initial_next_buffer_size)
   , m_initial_buffer(0)
   , m_initial_buffer_size(0u)
{}

inline monotonic_buffer_resource::monotonic_buffer_resource(std::size_t initial_size, memory_resource* upstream) BOOST_NOEXCEPT
   : m_memory_blocks(upstream ? *upstream : *get_default_resource())
   , m_current_buffer(0)
   , m_current_buffer_size(0u)
   , m_next_buffer_size(mbr_dtl::minimum_buffer_size)
   , m_initial_buffer(0)
   , m_initial_buffer_size(0u)
{                                         //In case initial_size is zero
   this->increase_next_buffer_at_least_to(initial_size + !initial_size);
}

inline monotonic_buffer_resource::monotonic_buffer_resource(void* buffer, std::size_t buffer_size, memory_resource* upstream) BOOST_NOEXCEPT
   : m_memory_blocks(upstream ? *upstream : *get_default_resource())
   , m_current_buffer(buffer)
   , m_current_buffer_size(buffer_size)
   , m_next_buffer_size
      (bi::detail::previous_or_equal_pow2
         (boost::container::dtl::max_value(buffer_size, std::size_t(initial_next_buffer_size))))
   , m_initial_buffer(buffer)
   , m_initial_buffer_size(buffer_size)
{  this->increase_next_buffer(); }

inline monotonic_buffer_resource::~monotonic_buffer_resource()
{  this->release();  }

inline void monotonic_buffer_resource::release() BOOST_NOEXCEPT
{
   m_memory_blocks.release();
   m_current_buffer = m_initial_buffer;
   m_current_buffer_size = m_initial_buffer_size;
   m_next_buffer_size = initial_next_buffer_size;
}

inline memory_resource* monotonic_buffer_resource::upstream_resource() const BOOST_NOEXCEPT
{  return &m_memory_blocks.upstream_resource();   }

inline std::size_t monotonic_buffer_resource::remaining_storage(std::size_t alignment, std::size_t &wasted_due_to_alignment) const BOOST_NOEXCEPT
{
   typedef mbr_dtl::uintptr_type uintptr_type;
   const uintptr_type up_alignment_minus1 = alignment - 1u;
   const uintptr_type up_alignment_mask = ~up_alignment_minus1;
   const uintptr_type up_addr = uintptr_type(m_current_buffer);
   const uintptr_type up_aligned_addr = (up_addr + up_alignment_minus1) & up_alignment_mask;
   wasted_due_to_alignment = std::size_t(up_aligned_addr - up_addr);
   return m_current_buffer_size <= wasted_due_to_alignment ? 0u : m_current_buffer_size - wasted_due_to_alignment;
}

inline std::size_t monotonic_buffer_resource::remaining_storage(std::size_t alignment) const BOOST_NOEXCEPT
{
   std::size_t ignore_this;
   return this->remaining_storage(alignment, ignore_this);
}

inline const void *monotonic_buffer_resource::current_buffer() const BOOST_NOEXCEPT
{  return m_current_buffer;  }

inline std::size_t monotonic_buffer_resource::next_buffer_size() const BOOST_NOEXCEPT
{  return m_next_buffer_size;  }

inline void *monotonic_buffer_resource::allocate_from_current(std::size_t aligner, std::size_t bytes)
{
   char * p = (char*)m_current_buffer + aligner;
   m_current_buffer = p + bytes;
   m_current_buffer_size -= aligner + bytes;
   return p;
}

inline void* monotonic_buffer_resource::do_allocate(std::size_t bytes, std::size_t alignment)
{
   BOOST_ASSERT((alignment & (alignment - 1u)) == 0u);  //Alignment must be a power of two

   //See if there is room in the current buffer. remaining_storage() reports the
   //alignment padding through "aligner"; note it clamps its return value to zero
   //when that padding alone does not fit, so "m_current_buffer_size < aligner" is
   //also tested to route the empty-block case below and to avoid underflowing
   //m_current_buffer_size in allocate_from_current().
   std::size_t aligner = 0u;
   if(!m_current_buffer || this->remaining_storage(alignment, aligner) < bytes || m_current_buffer_size < aligner){
      //The block obtained from the upstream resource is only guaranteed to be
      //max_align-aligned, so for an over-aligned request reserve enough extra
      //space to be able to realign the returned block inside the new buffer.
      const std::size_t extra_for_alignment = (alignment > memory_resource::max_align) ? (alignment - 1u) : 0u;
      //Update next_buffer_size to at least bytes plus the realignment slack
      this->increase_next_buffer_at_least_to(bytes + extra_for_alignment);
      //Now allocate and update internal data
      m_current_buffer = (char*)m_memory_blocks.allocate(m_next_buffer_size);
      m_current_buffer_size = m_next_buffer_size;
      this->increase_next_buffer();
      //Recompute the alignment padding for the freshly obtained buffer (which is
      //only max_align-aligned). This is zero unless the request is over-aligned,
      //in which case the reserved slack above guarantees "aligner + bytes" fits.
      this->remaining_storage(alignment, aligner);
   }
   //Enough internal storage, extract from it. For a zero-sized block this returns
   //the current aligned position without consuming bytes, so repeated zero-sized
   //allocations that already meet the alignment yield the same address (following
   //the libc++/MSVC behavior).
   return this->allocate_from_current(aligner, bytes);
}

inline void monotonic_buffer_resource::do_deallocate(void* p, std::size_t bytes, std::size_t alignment) BOOST_NOEXCEPT
{  (void)p; (void)bytes;  (void)alignment;  }

inline bool monotonic_buffer_resource::do_is_equal(const memory_resource& other) const BOOST_NOEXCEPT
{  return this == &other;  }

#endif   //!defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

}  //namespace pmr {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_PMR_MONOTONIC_BUFFER_RESOURCE_HPP
