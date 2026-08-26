//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_POOL_RESOURCE_HPP
#define BOOST_CONTAINER_POOL_RESOURCE_HPP

#if defined (_MSC_VER)
#  pragma once 
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/detail/block_list.hpp>
#include <boost/container/pmr/pool_options.hpp>

//Inline implementation
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/detail/block_slist.hpp>
#include <boost/container/detail/min_max.hpp>
#include <boost/container/detail/placement_new.hpp>
#include <boost/intrusive/linear_slist_algorithms.hpp>
#include <boost/intrusive/detail/math.hpp>

#include <cstddef>

namespace boost {
namespace container {
namespace pmr {

#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

class pool_data_t;

BOOST_CONTAINER_CONSTANT_VAR std::size_t pool_options_minimum_max_blocks_per_chunk = 1u;
BOOST_CONTAINER_CONSTANT_VAR std::size_t pool_options_default_max_blocks_per_chunk = 32u;
BOOST_CONTAINER_CONSTANT_VAR std::size_t pool_options_minimum_largest_required_pool_block =
   memory_resource::max_align > 2*sizeof(void*) ? memory_resource::max_align : 2*sizeof(void*);
BOOST_CONTAINER_CONSTANT_VAR std::size_t pool_options_default_largest_required_pool_block =
   pool_options_minimum_largest_required_pool_block > 4096u
      ? pool_options_minimum_largest_required_pool_block : 4096u;

#endif   //BOOST_CONTAINER_DOXYGEN_INVOKED

class pool_resource
{
   typedef block_list_base<> block_list_base_t;

   pool_options m_options;
   memory_resource&   m_upstream;
   block_list_base_t  m_oversized_list;
   pool_data_t *m_pool_data;
   std::size_t  m_pool_count;

   static void priv_limit_option(std::size_t &val, std::size_t min, std::size_t max);
   static std::size_t priv_pool_index(std::size_t block_size);
   static std::size_t priv_pool_block(std::size_t index);

   void priv_fix_options();
   void priv_init_pools();
   void priv_constructor_body();

   public:

   //! <b>Requires</b>: `upstream` is the address of a valid memory resource.
   //!
   //! <b>Effects</b>: Constructs a pool resource object that will obtain memory
   //!   from upstream whenever the pool resource is unable to satisfy a memory
   //!   request from its own internal data structures. The resulting object will hold
   //!   a copy of upstream, but will not own the resource to which upstream points.
   //!   [ Note: The intention is that calls to upstream->allocate() will be
   //!   substantially fewer than calls to this->allocate() in most cases. - end note 
   //!   The behavior of the pooling mechanism is tuned according to the value of
   //!   the opts argument.
   //!
   //! <b>Throws</b>: Nothing unless upstream->allocate() throws. It is unspecified if
   //!   or under what conditions this constructor calls upstream->allocate().
   pool_resource(const pool_options& opts, memory_resource* upstream) BOOST_NOEXCEPT;

   //! <b>Effects</b>: Same as
   //!   `pool_resource(pool_options(), get_default_resource())`.
   pool_resource() BOOST_NOEXCEPT;

   //! <b>Effects</b>: Same as
   //!   `pool_resource(pool_options(), upstream)`.
   explicit pool_resource(memory_resource* upstream) BOOST_NOEXCEPT;

   //! <b>Effects</b>: Same as
   //!   `pool_resource(opts, get_default_resource())`.
   explicit pool_resource(const pool_options& opts) BOOST_NOEXCEPT;

   #if !defined(BOOST_NO_CXX11_DELETED_FUNCTIONS) || defined(BOOST_CONTAINER_DOXYGEN_INVOKED)
   pool_resource(const pool_resource&) = delete;
   pool_resource operator=(const pool_resource&) = delete;
   #else
   private:
   pool_resource          (const pool_resource&);
   pool_resource operator=(const pool_resource&);
   public:
   #endif

   //! <b>Effects</b>: Calls
   //!   `this->release()`.
   ~pool_resource();

   //! <b>Effects</b>: Calls Calls `upstream_resource()->deallocate()` as necessary
   //!   to release all allocated memory. [ Note: memory is released back to
   //!   `upstream_resource()` even if deallocate has not been called for some
   //!   of the allocated blocks. - end note ]
   void release();

   //! <b>Returns</b>: The value of the upstream argument provided to the
   //!   constructor of this object.
   memory_resource* upstream_resource() const;

   //! <b>Returns</b>: The options that control the pooling behavior of this resource.
   //!   The values in the returned struct may differ from those supplied to the pool
   //!   resource constructor in that values of zero will be replaced with
   //!   implementation-defined defaults and sizes may be rounded to unspecified granularity.
   pool_options options() const;

   public:  //public so that [un]synchronized_pool_resource can use them

   //! <b>Returns</b>: A pointer to allocated storage with a size of at least `bytes`.
   //!   The size and alignment of the allocated memory shall meet the requirements for
   //!   a class derived from `memory_resource`.
   //!
   //! <b>Effects</b>: If the pool selected for a block of size bytes is unable to
   //!   satisfy the memory request from its own internal data structures, it will call
   //!   `upstream_resource()->allocate()` to obtain more memory. If `bytes` is larger
   //!   than that which the largest pool can handle, then memory will be allocated
   //!   using `upstream_resource()->allocate()`.
   //!
   //! <b>Throws</b>: Nothing unless `upstream_resource()->allocate()` throws.
   void* do_allocate(std::size_t bytes, std::size_t alignment);

   //! <b>Effects</b>: Return the memory at p to the pool. It is unspecified if or under
   //!   what circumstances this operation will result in a call to
   //!   `upstream_resource()->deallocate()`.
   //!
   //! <b>Throws</b>: Nothing.
   void do_deallocate(void* p, std::size_t bytes, std::size_t alignment);

   //Non-standard observers
   public:
   //! <b>Returns</b>: The number of pools that will be used in the pool resource.
   //!
   //! <b>Note</b>: Non-standard extension.
   std::size_t pool_count() const;

   //! <b>Returns</b>: The index of the pool that will be used to serve the allocation of `bytes`.
   //!   from the pool specified by `pool_index`. Returns `pool_count()` if `bytes` is bigger
   //!   than `options().largest_required_pool_block` (no pool will be used to serve this).
   //!
   //! <b>Note</b>: Non-standard extension.
   std::size_t pool_index(std::size_t bytes) const;

   //! <b>Requires</b>: `pool_idx < pool_index()`
   //!
   //! <b>Returns</b>: The number blocks that will be allocated in the next chunk
   //!   from the pool specified by `pool_idx`.
   //!
   //! <b>Note</b>: Non-standard extension.
   std::size_t pool_next_blocks_per_chunk(std::size_t pool_idx) const;

   //! <b>Requires</b>: `pool_idx < pool_index()`
   //!
   //! <b>Returns</b>: The number of bytes of the block that the specified `pool_idx` pool manages.
   //!
   //! <b>Note</b>: Non-standard extension.
   std::size_t pool_block(std::size_t pool_idx) const;

   //! <b>Requires</b>: `pool_idx < pool_index()`
   //!
   //! <b>Returns</b>: The number of blocks that the specified `pool_idx` pool has cached
   //!   and will be served without calling the upstream_allocator.
   //!
   //! <b>Note</b>: Non-standard extension.
   std::size_t pool_cached_blocks(std::size_t pool_idx) const;
};

#if !defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

//////////////////////////////////////////////////////////////////////////////
//
//    Inline implementation (formerly src/pool_resource.cpp)
//
//////////////////////////////////////////////////////////////////////////////

//pool_data_t

class pool_data_t
   : public block_slist_base<>
{
   typedef block_slist_base<> block_slist_base_t;

   public:
   explicit pool_data_t(std::size_t initial_blocks_per_chunk)
      : block_slist_base_t(), next_blocks_per_chunk(initial_blocks_per_chunk)
   {  slist_algo::init_header(&free_slist);  }

   void *allocate_block() BOOST_NOEXCEPT
   {
      if(slist_algo::unique(&free_slist)){
         return 0;
      }
      slist_node *pv = slist_algo::node_traits::get_next(&free_slist);
      slist_algo::unlink_after(&free_slist);
      pv->~slist_node();
      return pv;
   }

   void deallocate_block(void *p) BOOST_NOEXCEPT
   {
      slist_node *pv = ::new(p, boost_container_new_t()) slist_node();
      slist_algo::link_after(&free_slist, pv);
   }

   void release(memory_resource &upstream)
   {
      slist_algo::init_header(&free_slist);
      this->block_slist_base_t::release(upstream);
      next_blocks_per_chunk = pool_options_minimum_max_blocks_per_chunk;
   }

   void replenish(memory_resource &mr, std::size_t pool_block, std::size_t max_blocks_per_chunk)
   {
      //Limit max value
      std::size_t blocks_per_chunk = boost::container::dtl::min_value(max_blocks_per_chunk, next_blocks_per_chunk);
      //Avoid overflow
      blocks_per_chunk = boost::container::dtl::min_value(blocks_per_chunk, std::size_t(-1)/pool_block);

      //Minimum block size is at least max_align, so all pools allocate sizes that are multiple of max_align,
      //meaning that all blocks are max_align-aligned.
      char *p = static_cast<char *>(block_slist_base_t::allocate(blocks_per_chunk*pool_block, mr));

      //Create header types. This is no-throw
      for(std::size_t i = 0, max = blocks_per_chunk; i != max; ++i){
         slist_node *const pv = ::new(p, boost_container_new_t()) slist_node();
         slist_algo::link_after(&free_slist, pv);
         p += pool_block;
      }

      //Update next block per chunk
      next_blocks_per_chunk = max_blocks_per_chunk/2u < blocks_per_chunk  ? max_blocks_per_chunk : blocks_per_chunk*2u;
   }

   std::size_t cache_count() const
   {  return slist_algo::count(&free_slist) - 1u;  }

   slist_node  free_slist;
   std::size_t next_blocks_per_chunk;
};

//pool_resource

//Detect overflow in ceil_pow2
BOOST_CONTAINER_STATIC_ASSERT(pool_options_default_max_blocks_per_chunk <= (std::size_t(-1)/2u+1u));
//Sanity checks
BOOST_CONTAINER_STATIC_ASSERT(bi::detail::static_is_pow2<pool_options_default_max_blocks_per_chunk>::value);
BOOST_CONTAINER_STATIC_ASSERT(bi::detail::static_is_pow2<pool_options_minimum_largest_required_pool_block>::value);

inline void pool_resource::priv_limit_option(std::size_t &val, std::size_t min, std::size_t max) //static
{
   if(!val){
      val = max;
   }
   else{
      val = val < min ? min : boost::container::dtl::min_value(val, max);
   }
}

inline std::size_t pool_resource::priv_pool_index(std::size_t block_size) //static
{
   //For allocations equal or less than pool_options_minimum_largest_required_pool_block
   //the smallest pool is used
   block_size = boost::container::dtl::max_value(block_size, pool_options_minimum_largest_required_pool_block);
   return bi::detail::ceil_log2(block_size)
      - bi::detail::ceil_log2(pool_options_minimum_largest_required_pool_block);
}

inline std::size_t pool_resource::priv_pool_block(std::size_t index)  //static
{
   //For allocations equal or less than pool_options_minimum_largest_required_pool_block
   //the smallest pool is used
   return pool_options_minimum_largest_required_pool_block << index;
}

inline void pool_resource::priv_fix_options()
{
   priv_limit_option(m_options.max_blocks_per_chunk
                     , pool_options_minimum_max_blocks_per_chunk
                     , pool_options_default_max_blocks_per_chunk);
   priv_limit_option
      ( m_options.largest_required_pool_block
      , pool_options_minimum_largest_required_pool_block
      , pool_options_default_largest_required_pool_block);
   m_options.largest_required_pool_block = bi::detail::ceil_pow2(m_options.largest_required_pool_block);
}

inline void pool_resource::priv_init_pools()
{
   const std::size_t num_pools = priv_pool_index(m_options.largest_required_pool_block)+1u;
   //Otherwise, just use the default alloc (zero pools)
   void *p = 0;
   //This can throw
   p = m_upstream.allocate(sizeof(pool_data_t)*num_pools);
   //This is nothrow
   m_pool_data = static_cast<pool_data_t *>(p);
   for(std::size_t i = 0, max = num_pools; i != max; ++i){
      ::new(&m_pool_data[i], boost_container_new_t()) pool_data_t(pool_options_minimum_max_blocks_per_chunk);
   }
   m_pool_count = num_pools;
}

inline void pool_resource::priv_constructor_body()
{
   this->priv_fix_options();
}

inline pool_resource::pool_resource(const pool_options& opts, memory_resource* upstream) BOOST_NOEXCEPT
   : m_options(opts), m_upstream(*upstream), m_oversized_list(), m_pool_data(), m_pool_count()
{  this->priv_constructor_body();  }

inline pool_resource::pool_resource() BOOST_NOEXCEPT
   : m_options(), m_upstream(*get_default_resource()), m_oversized_list(), m_pool_data(), m_pool_count()
{  this->priv_constructor_body();  }

inline pool_resource::pool_resource(memory_resource* upstream) BOOST_NOEXCEPT
   : m_options(), m_upstream(*upstream), m_oversized_list(), m_pool_data(), m_pool_count()
{  this->priv_constructor_body();  }

inline pool_resource::pool_resource(const pool_options& opts) BOOST_NOEXCEPT
   : m_options(opts), m_upstream(*get_default_resource()), m_oversized_list(), m_pool_data(), m_pool_count()
{  this->priv_constructor_body();  }

inline pool_resource::~pool_resource()
{
   this->release();

   for(std::size_t i = 0, max = m_pool_count; i != max; ++i){
      m_pool_data[i].~pool_data_t();
   }
   if(m_pool_data){
      m_upstream.deallocate((void*)m_pool_data, sizeof(pool_data_t)*m_pool_count);
   }
}

inline void pool_resource::release()
{
   m_oversized_list.release(m_upstream);
   for(std::size_t i = 0, max = m_pool_count; i != max; ++i)
   {
      m_pool_data[i].release(m_upstream);
   }
}

inline memory_resource* pool_resource::upstream_resource() const
{  return &m_upstream;  }

inline pool_options pool_resource::options() const
{  return m_options; }

inline void* pool_resource::do_allocate(std::size_t bytes, std::size_t alignment)
{
   if(!m_pool_data){
      this->priv_init_pools();
   }
   (void)alignment;  //alignment ignored here, max_align is used by pools
   if(bytes > m_options.largest_required_pool_block){
      return m_oversized_list.allocate(bytes, m_upstream);
   }
   else{
      const std::size_t pool_idx = priv_pool_index(bytes);
      pool_data_t & pool = m_pool_data[pool_idx];
      void *p = pool.allocate_block();
      if(!p){
         pool.replenish(m_upstream, priv_pool_block(pool_idx), m_options.max_blocks_per_chunk);
         p = pool.allocate_block();
      }
      return p;
   }
}

inline void pool_resource::do_deallocate(void* p, std::size_t bytes, std::size_t alignment)
{
   (void)alignment;  //alignment ignored here, max_align is used by pools
   if(bytes > m_options.largest_required_pool_block){
      //Just cached
      return m_oversized_list.deallocate(p, m_upstream);
   }
   else{
      const std::size_t pool_idx = priv_pool_index(bytes);
      return m_pool_data[pool_idx].deallocate_block(p);
   }
}

inline std::size_t pool_resource::pool_count() const
{
   if(BOOST_LIKELY((0 != m_pool_data))){
      return m_pool_count;
   }
   else{
      return priv_pool_index(m_options.largest_required_pool_block)+1u;
   }
}

inline std::size_t pool_resource::pool_index(std::size_t bytes) const
{
   if(bytes > m_options.largest_required_pool_block){
      return pool_count();
   }
   else{
      return priv_pool_index(bytes);
   }
}

inline std::size_t pool_resource::pool_next_blocks_per_chunk(std::size_t pool_idx) const
{
   if(BOOST_LIKELY((m_pool_data && pool_idx < m_pool_count))){
      return m_pool_data[pool_idx].next_blocks_per_chunk;
   }
   else{
      return 1u;
   }
}

inline std::size_t pool_resource::pool_block(std::size_t pool_idx) const
{  return priv_pool_block(pool_idx);  }

inline std::size_t pool_resource::pool_cached_blocks(std::size_t pool_idx) const
{
   if(BOOST_LIKELY((m_pool_data && pool_idx < m_pool_count))){
      return m_pool_data[pool_idx].cache_count();
   }
   else{
      return 0u;
   }
}

#endif   //!defined(BOOST_CONTAINER_DOXYGEN_INVOKED)

}  //namespace pmr {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_POOL_RESOURCE_HPP
