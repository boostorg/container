//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_BLOCK_SLIST_HEADER
#define BOOST_CONTAINER_DETAIL_BLOCK_SLIST_HEADER

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif


#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/container_fwd.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/throw_exception.hpp>
#include <boost/container/detail/placement_new.hpp>

#include <boost/move/detail/type_traits.hpp>
#include <boost/intrusive/linear_slist_algorithms.hpp>
#include <boost/intrusive/detail/math.hpp>
#include <boost/assert.hpp>

#include <cstddef>
#include <climits>

namespace boost {
namespace container {
namespace pmr {

struct slist_node
{
   slist_node *next;
};

struct slist_node_traits
{
   typedef slist_node         node;
   typedef slist_node*        node_ptr;
   typedef const slist_node*  const_node_ptr;

   static node_ptr get_next(const_node_ptr n)
   {  return n->next;  }

   static void set_next(const node_ptr & n, const node_ptr & next)
   {  n->next = next;  }
};

struct block_slist_header
   : public slist_node
{
   std::size_t size;
};

typedef bi::linear_slist_algorithms<slist_node_traits> slist_algo;

template<class DerivedFromBlockSlistHeader = block_slist_header>
class block_slist_base
{
   slist_node m_slist;

   BOOST_STATIC_CONSTEXPR std::size_t MaxAlignMinus1 = memory_resource::max_align-1u;

   public:

   BOOST_STATIC_CONSTEXPR std::size_t header_size = std::size_t(sizeof(DerivedFromBlockSlistHeader) + MaxAlignMinus1) & std::size_t(~MaxAlignMinus1);

   explicit block_slist_base()
   {  slist_algo::init_header(&m_slist);  }

   #if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   block_slist_base(const block_slist_base&) = delete;
   block_slist_base operator=(const block_slist_base&) = delete;
   #else
   private:
   block_slist_base          (const block_slist_base&);
   block_slist_base operator=(const block_slist_base&);
   public:
   #endif

   void *allocate(std::size_t size, memory_resource &mr)
   {
      if(BOOST_UNLIKELY((size_t(-1) - header_size) < size))
         throw_bad_alloc();
      void *p = mr.allocate(size+header_size);
      block_slist_header &mb  = *::new((void*)p, boost_container_new_t()) DerivedFromBlockSlistHeader;
      mb.size = size+header_size;
      slist_algo::link_after(&m_slist, &mb);
      return (char *)p + header_size;
   }

   void release(memory_resource &mr) BOOST_NOEXCEPT
   {
      slist_node *n = slist_algo::node_traits::get_next(&m_slist);
      while(n){
         DerivedFromBlockSlistHeader &d = static_cast<DerivedFromBlockSlistHeader&>(*n);
         n = slist_algo::node_traits::get_next(n);
         std::size_t size = d.block_slist_header::size;
         d.~DerivedFromBlockSlistHeader();
         mr.deallocate(reinterpret_cast<char*>(&d), size, memory_resource::max_align);         
      }
      slist_algo::init_header(&m_slist);
   }
};

//Metadata for block_slist, placed at the end of the block
struct block_slist_footer
   : public slist_node
{
   //Includes:
   //Size of the block: (multiple of the granularity of block_slist) << 1u
   //Base-2 logarithm of the alignment (low bits)
   std::size_t size_and_alignment_log2;
};

//  Layout (upstream allocation of "size" bytes):
//
//  |<------------------------------ size ------------------------------>|
//  +---------------------------------------------+-----------------------+
//  | payload (returned address, aligned to the   | block_slist_footer    |
//  |  alignment requested from upstream)         | (next, size|lg2 align)|
//  +---------------------------------------------+-----------------------+
//  ^ upstream address                            ^ upstream address + size - footer_size

class block_slist
{
   slist_node        m_slist;
   memory_resource & m_upstream_rsrc;

   //Bytes of each block used by the footer
   BOOST_STATIC_CONSTEXPR std::size_t footer_size = sizeof(block_slist_footer);

   //Block sizes are rounded up to a multiple of this value. We take advantage
   //of tha fact that 2^(size_bits-1) is not representable by std::ptrdiff_t
   BOOST_STATIC_CONSTEXPR std::size_t granularity = sizeof(std::size_t)*CHAR_BIT/2u;

   //Sanity checks
   BOOST_CONTAINER_STATIC_ASSERT((granularity & (granularity - 1u)) == 0u);
   BOOST_CONTAINER_STATIC_ASSERT(granularity >= footer_size);
   BOOST_CONTAINER_STATIC_ASSERT((granularity % boost::move_detail::alignment_of<block_slist_footer>::value) == 0u);

   BOOST_STATIC_CONSTEXPR std::size_t granularity_mask = granularity - 1u;
   BOOST_STATIC_CONSTEXPR std::size_t alignment_log2_mask = 2u*granularity - 1u;

   //The size (not bigger than max_size) shifted left by one bit has
   //its log2(2*granularity) low bits clear: they store the logarithm of the alignment
   static std::size_t priv_encode_size_and_alignment(std::size_t size, std::size_t alignment) BOOST_NOEXCEPT
   {
      //Alignment sanity
      const std::size_t alignment_log2 = bi::detail::floor_log2(alignment);
      BOOST_ASSERT(alignment_log2 <= alignment_log2_mask);
      //Size sanity
      BOOST_ASSERT(size <= max_size && (size & granularity_mask) == 0u);
      return (size << 1u) | alignment_log2;
   }

   static std::size_t priv_decode_size(std::size_t v) BOOST_NOEXCEPT
   {  return (v >> 1u) & ~granularity_mask;  }

   static std::size_t priv_decode_alignment(std::size_t v) BOOST_NOEXCEPT
   {  return std::size_t(1u) << (v & alignment_log2_mask);  }

   //Largest block size that can be requested, can't be bigger than PTRDIFF_MAX
   BOOST_STATIC_CONSTEXPR std::size_t max_size = (std::size_t(-1)/2u) & ~granularity_mask;

   //Returns the size (footer included) of the smallest block whose usable bytes are not
   //less than "payload": "payload" plus the footer rounded up to a multiple of granularity.
   //Returns zero if the result is bigger than max_size.
   static std::size_t priv_block_size(std::size_t payload) BOOST_NOEXCEPT
   {
      if(BOOST_UNLIKELY(payload > max_size - footer_size))
         return 0u;
      return (payload + footer_size + granularity_mask) & ~granularity_mask;
   }

   //Returns the payload size (usable bytes) of a block
   static std::size_t priv_payload_size(std::size_t block_size) BOOST_NOEXCEPT
   {  return block_size - footer_size;  }

   public:

   explicit block_slist(memory_resource &upstream_rsrc)
      : m_slist(), m_upstream_rsrc(upstream_rsrc)
   {  slist_algo::init_header(&m_slist);  }

   #if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   block_slist(const block_slist&) = delete;
   block_slist operator=(const block_slist&) = delete;
   #else
   private:
   block_slist          (const block_slist&);
   block_slist operator=(const block_slist&);
   public:
   #endif

   ~block_slist()
   {  this->release();  }

   //Allocates from upstream a block whose usable bytes are not less than "min_payload", with
   //alignment max(alignment, max_align), and returns the start of its usable bytes. On return,
   //"payload" is the number of usable bytes (it can be bigger than "min_payload"). Throws
   //bad_alloc if the size of the block (more than "min_payload") would be bigger than
   //PTRDIFF_MAX. If the upstream resource throws, there are no effects.
   void *allocate(std::size_t min_payload, std::size_t alignment, std::size_t &payload)
   {
      BOOST_ASSERT((alignment & (alignment - 1u)) == 0u);
      if(alignment < memory_resource::max_align)
         alignment = memory_resource::max_align;

      const std::size_t bsize = priv_block_size(min_payload);
      if(BOOST_UNLIKELY(!bsize))
         throw_bad_alloc();

      char *const p = static_cast<char*>(m_upstream_rsrc.allocate(bsize, alignment));

      payload = priv_payload_size(bsize);
      block_slist_footer &f = *::new((void*)(p + payload), boost_container_new_t()) block_slist_footer;
      f.size_and_alignment_log2 = priv_encode_size_and_alignment(bsize, alignment);
      slist_algo::link_after(&m_slist, &f);
      return p;
   }

   void release() BOOST_NOEXCEPT
   {
      slist_node *n = slist_algo::node_traits::get_next(&m_slist);
      while(n){
         block_slist_footer &f = static_cast<block_slist_footer&>(*n);
         n = slist_algo::node_traits::get_next(n);
         const std::size_t size      = priv_decode_size(f.size_and_alignment_log2);
         const std::size_t alignment = priv_decode_alignment(f.size_and_alignment_log2);
         char *const p = reinterpret_cast<char*>(&f) + footer_size - size;
         f.~block_slist_footer();
         m_upstream_rsrc.deallocate(p, size, alignment);
      }
      slist_algo::init_header(&m_slist);
   }

   memory_resource& upstream_resource() const BOOST_NOEXCEPT
   {  return m_upstream_rsrc;   }
};

}  //namespace pmr {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_DETAIL_BLOCK_SLIST_HEADER
