//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_BLOCK_LIST_HEADER
#define BOOST_CONTAINER_DETAIL_BLOCK_LIST_HEADER

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
#include <boost/intrusive/circular_list_algorithms.hpp>
#include <boost/move/detail/type_traits.hpp>
#include <boost/assert.hpp>
#include <boost/container/detail/placement_new.hpp>

#include <cstddef>

namespace boost {
namespace container {
namespace pmr {

struct list_node
{
   list_node *next;
   list_node *previous;
};

struct list_node_traits
{
   typedef list_node         node;
   typedef list_node*        node_ptr;
   typedef const list_node*  const_node_ptr;

   static node_ptr get_next(const_node_ptr n)
   {  return n->next;  }

   static node_ptr get_previous(const_node_ptr n)
   {  return n->previous;  }

   static void set_next(const node_ptr & n, const node_ptr & next)
   {  n->next = next;  }

   static void set_previous(const node_ptr & n, const node_ptr & previous)
   {  n->previous = previous;  }
};

struct block_list_header
   : public list_node
{
   std::size_t size;       //bytes allocated from the upstream resource
   std::size_t alignment;  //alignment passed to the upstream resource
};

typedef bi::circular_list_algorithms<list_node_traits> list_algo;


//block_list_base allocates blocks from an upstream memory_resource and links them in
//a circular doubly linked list. A block can be unlinked in constant time (deallocate)
//and all blocks can be returned to the upstream resource at once (release).
//
//Layout of each upstream allocation:
//
//  base: returned by upstream.allocate(header.size, header.alignment)
//  |
//  v
//  +-------------+------------------------------+------------------------------+
//  | padding     | DerivedFromBlockListHeader   | block                        |
//  | (only if    | (derives from                | ("size" bytes, aligned to    |
//  |  alignment  |  block_list_header:          |  "alignment")                |
//  |  >          |  next, previous,             |                              |
//  |  max_align) |  size, alignment)            |                              |
//  +-------------+------------------------------+------------------------------+
//  <------------------- offset ----------------> ^
//                <-------- header_size --------> |
//                                                returned by allocate()
//
//  header_size      = sizeof(DerivedFromBlockListHeader) rounded up to max_align
//  offset           = header_size rounded up to "alignment"
//  header.size      = offset + size   (passed to upstream.deallocate)
//  header.alignment = alignment       (passed to upstream.deallocate)
//
//The headers are the nodes of the list. m_list is the sentinel node:
//
//  m_list <-> header(block 1) <-> header(block 2) <-> ... <-> header(block N) <-> m_list
//    ^                                                                             |
//    +-----------------------------------------------------------------------------+
//
//DerivedFromBlockListHeader must derive from block_list_header, and can add members.
template<class DerivedFromBlockListHeader = block_list_header>
class block_list_base
{
   list_node m_list;

   BOOST_STATIC_CONSTEXPR std::size_t MaxAlignMinus1 = memory_resource::max_align-1u;

   public:

   BOOST_STATIC_CONSTEXPR std::size_t header_size = std::size_t(sizeof(DerivedFromBlockListHeader) + MaxAlignMinus1) & std::size_t(~MaxAlignMinus1);

   explicit block_list_base()
   {  list_algo::init_header(&m_list);  }

   #if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   block_list_base(const block_list_base&) = delete;
   block_list_base operator=(const block_list_base&) = delete;
   #else
   private:
   block_list_base          (const block_list_base&);
   block_list_base operator=(const block_list_base&);
   public:
   #endif

   //Returns a block of size bytes aligned to alignment (at least max_align). The header
   //is placed just before the block. The upstream allocation starts "offset" bytes
   //before the block, so that the block keeps the alignment of the upstream allocation.
   void *allocate(std::size_t size, memory_resource &mr, std::size_t alignment = memory_resource::max_align)
   {
      if(alignment < memory_resource::max_align)
         alignment = memory_resource::max_align;
      const std::size_t offset = priv_offset(alignment);
      if(BOOST_UNLIKELY((size_t(-1) - offset) < size))
         throw_bad_alloc();
      char *const base = static_cast<char*>(mr.allocate(size + offset, alignment));
      char *const block = base + offset;
      block_list_header &mb  = *::new((void*)(block - header_size), boost_container_new_t()) DerivedFromBlockListHeader;
      mb.size = size + offset;
      mb.alignment = alignment;
      list_algo::link_after(&m_list, &mb);
      return block;
   }

   void deallocate(void *p, memory_resource &mr) BOOST_NOEXCEPT
   {
      DerivedFromBlockListHeader *pheader = static_cast<DerivedFromBlockListHeader*>
         (static_cast<void*>((char*)p - header_size));
      list_algo::unlink(pheader);
      priv_deallocate(*pheader, mr);
   }

   void release(memory_resource &mr) BOOST_NOEXCEPT
   {
      list_node *n = list_algo::node_traits::get_next(&m_list);
      while(n != &m_list){
         DerivedFromBlockListHeader &d = static_cast<DerivedFromBlockListHeader&>(*n);
         n = list_algo::node_traits::get_next(n);
         priv_deallocate(d, mr);
      }
      list_algo::init_header(&m_list);
   }

   private:
   //Bytes from the start of the upstream allocation to the block: header_size rounded to the alignment
   static std::size_t priv_offset(std::size_t alignment) BOOST_NOEXCEPT
   {  return (header_size + alignment - 1u) & ~(alignment - 1u);  }

   static void priv_deallocate(DerivedFromBlockListHeader &header, memory_resource &mr) BOOST_NOEXCEPT
   {
      const std::size_t size      = header.size;
      const std::size_t alignment = header.alignment;
      char *const base = reinterpret_cast<char*>(&header) + header_size - priv_offset(alignment);
      header.~DerivedFromBlockListHeader();
      mr.deallocate(base, size, alignment);
   }
};

}  //namespace pmr {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_DETAIL_BLOCK_LIST_HEADER
