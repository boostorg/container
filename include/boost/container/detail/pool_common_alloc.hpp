//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_POOL_COMMON_ALLOC_HPP
#define BOOST_CONTAINER_DETAIL_POOL_COMMON_ALLOC_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/throw_exception.hpp>

#include <boost/intrusive/slist.hpp>
#include <boost/container/detail/multiallocation_chain.hpp>
#include <boost/container/detail/pool_common.hpp>
#include <boost/container/detail/dlmalloc.hpp>
#include <boost/container/detail/intermodule_globals.hpp>
#include <cstddef>

namespace boost{
namespace container{
namespace dtl{

struct node_slist_helper
   : public boost::container::dtl::node_slist<void*>
{};

//It forwards to the global dlmalloc heap.
//All its functions are static
//Options of the pools shared by node_allocator and adaptive_pool (see
//intermodule_globals.hpp). BOOST_SYMBOL_VISIBLE: required for cross-module
//unification on ELF, as for the pool types.
struct BOOST_SYMBOL_VISIBLE shared_pool_globals_options
{
   //The pools are deliberately immortal: destructors of user global objects
   //(a global list<int, node_allocator<int> >) and atexit callbacks may still
   //return nodes after Boost.Container's own static objects are gone, and the
   //destructor of a pool frees its blocks. The blocks come from the immortal
   //dlmalloc heap, and the OS reclaims everything at process exit.
   static const bool destroy_at_exit = false;

   //pin_constructing_module defaults to false: a pool is plain state (block
   //lists, counters and a spin lock), without a vtable or a function pointer.
   //The rendezvous key defaults to the names of the pool type and of these
   //options, so every pool configuration has its own object.
};

BOOST_CONTAINER_INTERMODULE_ASSERT_VISIBLE(shared_pool_globals_options,
   "boost::container::dtl::shared_pool_globals_options must be declared BOOST_SYMBOL_VISIBLE");

//The process-wide instance of a shared pool type
template<class SharedPool>
BOOST_CONTAINER_FORCEINLINE SharedPool &shared_pool_instance()
{
   return intermodule_globals<SharedPool, shared_pool_globals_options>();
}

struct dlmalloc_segment_manager
{
   typedef void * void_pointer;
   BOOST_STATIC_CONSTEXPR std::size_t PayloadPerAllocation = dlmalloc::allocation_payload;

   typedef boost::container::dtl::
      basic_multiallocation_chain<void*>              multiallocation_chain;
   static void deallocate(void_pointer p)
   { dlmalloc_heap().deallocate(p); }

   static void deallocate_many(multiallocation_chain &chain)
   {
      dlmalloc_heap().multidealloc(chain);
   }

   typedef std::ptrdiff_t  difference_type;
   typedef std::size_t     size_type;

   static void *allocate_aligned(std::size_t nbytes, std::size_t alignment)
   {
      void *ret = dlmalloc_heap().allocate_aligned(alignment, nbytes);
      if(!ret)
         boost::container::throw_bad_alloc();
      return ret;
   }

};

}  //namespace boost{
}  //namespace container{
}  //namespace dtl{

namespace boost {
namespace container {
namespace dtl {

template<class T>
struct is_stateless_segment_manager;

template<>
struct is_stateless_segment_manager
   <boost::container::dtl::dlmalloc_segment_manager>
{
   BOOST_STATIC_CONSTEXPR bool value = true;
};

}  //namespace dtl {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_DETAIL_POOL_COMMON_ALLOC_HPP
