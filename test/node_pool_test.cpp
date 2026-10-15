//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/detail/node_pool.hpp>
#include "lightweight_test.hpp"
#include <cstddef>
#include <cstring>

using namespace boost::container;

static const std::size_t test_node_size       = 16u;
static const std::size_t test_nodes_per_block = 8u;

typedef dtl::private_node_pool<test_node_size, test_nodes_per_block, sizeof(void*)>  private_pool_t;
typedef dtl::shared_node_pool <test_node_size, test_nodes_per_block, sizeof(void*)>  shared_pool_t;
typedef dtl::private_node_pool<test_node_size, test_nodes_per_block, 64u>            overaligned_pool_t;

//True if [a, a + size) and [b, b + size) overlap. The nodes can belong to
//different blocks, so the addresses are compared as integers.
static bool nodes_overlap(const void *a, const void *b, std::size_t size)
{
   const std::size_t ua = reinterpret_cast<std::size_t>(a);
   const std::size_t ub = reinterpret_cast<std::size_t>(b);
   return ua < ub + size && ub < ua + size;
}

//Each node is usable, aligned and does not overlap the other nodes
template<class Pool>
static void check_nodes(Pool &p, void * const *v, std::size_t n, std::size_t alignment)
{
   const std::size_t real_size = p.get_real_node_size();
   for(std::size_t i = 0; i != n; ++i){
      BOOST_TEST(v[i] != 0);
      BOOST_TEST(reinterpret_cast<std::size_t>(v[i]) % alignment == 0u);
      std::memset(v[i], int(i & 0xFFu), real_size);
   }
   for(std::size_t i = 0; i != n; ++i){
      for(std::size_t j = i + 1u; j != n; ++j)
         BOOST_TEST(!nodes_overlap(v[i], v[j], real_size));
      BOOST_TEST(static_cast<unsigned char*>(v[i])[real_size - 1u] == (i & 0xFFu));
   }
}

//get_real_node_size, get_real_num_node, get_segment_manager_base and num_free_nodes
template<class Pool>
void test_properties(std::size_t alignment)
{
   Pool p;
   BOOST_TEST(p.get_real_num_node() == test_nodes_per_block);
   BOOST_TEST(p.get_real_node_size() >= test_node_size);
   BOOST_TEST(p.get_real_node_size() % alignment == 0u);
   //The pool allocates from the internal heap, not from a segment manager object
   BOOST_TEST(p.get_segment_manager_base() == 0);
   BOOST_TEST(p.num_free_nodes() == 0u);
}

//allocate_node and deallocate_node
template<class Pool>
void test_allocate_deallocate_node(std::size_t alignment)
{
   Pool p;
   void *v[3u*test_nodes_per_block];
   const std::size_t n = sizeof(v)/sizeof(v[0]);
   for(std::size_t i = 0; i != n; ++i){
      v[i] = p.allocate_node();
      //Each new block gives test_nodes_per_block nodes
      BOOST_TEST(p.num_free_nodes() == (test_nodes_per_block - 1u - i % test_nodes_per_block));
   }
   check_nodes(p, v, n, alignment);
   for(std::size_t i = 0; i != n; ++i)
      p.deallocate_node(v[i]);
   BOOST_TEST(p.num_free_nodes() == n);
   //Free nodes are used before new blocks are allocated
   void *const w = p.allocate_node();
   BOOST_TEST(p.num_free_nodes() == n - 1u);
   p.deallocate_node(w);
}

//allocate_nodes and deallocate_nodes
template<class Pool>
void test_allocate_deallocate_nodes(std::size_t alignment)
{
   typedef typename Pool::multiallocation_chain chain_t;
   Pool p;
   const std::size_t counts[] = { 1u, test_nodes_per_block - 1u, test_nodes_per_block, 2u*test_nodes_per_block + 3u };
   void *v[2u*test_nodes_per_block + 3u];
   for(std::size_t c = 0; c != sizeof(counts)/sizeof(counts[0]); ++c){
      const std::size_t n = counts[c];
      chain_t chain;
      p.allocate_nodes(n, chain);
      BOOST_TEST(chain.size() == n);
      std::size_t i = 0;
      for(typename chain_t::iterator it = chain.begin(); it != chain.end(); ++it)
         v[i++] = &*it;
      BOOST_TEST(i == n);
      const std::size_t free_before = p.num_free_nodes();
      p.deallocate_nodes(chain);
      chain.clear();
      BOOST_TEST(p.num_free_nodes() == free_before + n);
      //The chain links are stored in the nodes, so the nodes are written
      //only after they are allocated again
      for(std::size_t k = 0; k != n; ++k)
         v[k] = p.allocate_node();
      check_nodes(p, v, n, alignment);
      for(std::size_t k = 0; k != n; ++k)
         p.deallocate_node(v[k]);
   }
   //allocate_nodes adds the nodes to a chain that is not empty
   {
      chain_t chain;
      p.allocate_nodes(2u, chain);
      p.allocate_nodes(3u, chain);
      BOOST_TEST(chain.size() == 5u);
      p.deallocate_nodes(chain);
      chain.clear();
   }
}

//deallocate_free_blocks keeps the free nodes of the blocks that still have
//nodes in use, and frees the blocks whose nodes are all free.
//Nodes are taken from a new block in order, so v[k*test_nodes_per_block...] share block k
template<class Pool>
void test_deallocate_free_blocks()
{
   //No blocks
   {
      Pool p;
      p.deallocate_free_blocks();
      BOOST_TEST(p.num_free_nodes() == 0u);
   }
   //One block with one node in use
   {
      Pool p;
      void *const v = p.allocate_node();
      p.deallocate_free_blocks();
      BOOST_TEST(p.num_free_nodes() == test_nodes_per_block - 1u);
      p.deallocate_node(v);
      p.deallocate_free_blocks();
      BOOST_TEST(p.num_free_nodes() == 0u);
   }
   //Two blocks: the first visited block has no free nodes
   {
      Pool p;
      void *v[2u*test_nodes_per_block];
      for(std::size_t i = 0; i != 2u*test_nodes_per_block; ++i)
         v[i] = p.allocate_node();
      p.deallocate_node(v[0]);
      p.deallocate_free_blocks();
      BOOST_TEST(p.num_free_nodes() == 1u);
      //The free node can be used again
      v[0] = p.allocate_node();
      BOOST_TEST(p.num_free_nodes() == 0u);
      for(std::size_t i = 0; i != 2u*test_nodes_per_block; ++i)
         p.deallocate_node(v[i]);
      p.deallocate_free_blocks();
      BOOST_TEST(p.num_free_nodes() == 0u);
   }
   //Three blocks: one full, one with one node in use and one with all nodes free
   {
      Pool p;
      void *v[3u*test_nodes_per_block];
      for(std::size_t i = 0; i != 3u*test_nodes_per_block; ++i)
         v[i] = p.allocate_node();
      for(std::size_t i = test_nodes_per_block + 1u; i != 3u*test_nodes_per_block; ++i)
         p.deallocate_node(v[i]);
      p.deallocate_free_blocks();
      //Only the free nodes of the partly used block are kept
      BOOST_TEST(p.num_free_nodes() == test_nodes_per_block - 1u);
      for(std::size_t i = test_nodes_per_block + 1u; i != 2u*test_nodes_per_block; ++i)
         v[i] = p.allocate_node();
      BOOST_TEST(p.num_free_nodes() == 0u);
      for(std::size_t i = 0; i != 2u*test_nodes_per_block; ++i)
         p.deallocate_node(v[i]);
      p.deallocate_free_blocks();
      BOOST_TEST(p.num_free_nodes() == 0u);
   }
}

//purge_blocks frees all the blocks when no node is in use
template<class Pool>
void test_purge_blocks()
{
   Pool p;
   void *v[2u*test_nodes_per_block];
   for(std::size_t i = 0; i != 2u*test_nodes_per_block; ++i)
      v[i] = p.allocate_node();
   for(std::size_t i = 0; i != 2u*test_nodes_per_block; ++i)
      p.deallocate_node(v[i]);
   BOOST_TEST(p.num_free_nodes() == 2u*test_nodes_per_block);
   p.purge_blocks();
   BOOST_TEST(p.num_free_nodes() == 0u);
   //The pool can be used again
   void *const w = p.allocate_node();
   BOOST_TEST(p.num_free_nodes() == test_nodes_per_block - 1u);
   p.deallocate_node(w);
}

//swap exchanges the blocks, the free nodes and the count of nodes in use
void test_swap()
{
   private_pool_t a, b;
   void *const va = a.allocate_node();
   void *vb[test_nodes_per_block + 1u];
   for(std::size_t i = 0; i != test_nodes_per_block + 1u; ++i)
      vb[i] = b.allocate_node();
   BOOST_TEST(a.num_free_nodes() == test_nodes_per_block - 1u);
   BOOST_TEST(b.num_free_nodes() == test_nodes_per_block - 1u);
   b.deallocate_node(vb[0]);
   a.swap(b);
   BOOST_TEST(a.num_free_nodes() == test_nodes_per_block);
   BOOST_TEST(b.num_free_nodes() == test_nodes_per_block - 1u);
   //Each node is returned to the pool that now owns its block
   b.deallocate_node(va);
   for(std::size_t i = 1; i != test_nodes_per_block + 1u; ++i)
      a.deallocate_node(vb[i]);
   BOOST_TEST(a.num_free_nodes() == 2u*test_nodes_per_block);
   BOOST_TEST(b.num_free_nodes() == test_nodes_per_block);
}

template<class Pool>
void test_pool(std::size_t alignment)
{
   test_properties<Pool>(alignment);
   test_allocate_deallocate_node<Pool>(alignment);
   test_allocate_deallocate_nodes<Pool>(alignment);
   test_deallocate_free_blocks<Pool>();
   test_purge_blocks<Pool>();
}

int main()
{
   test_pool<private_pool_t>(sizeof(void*));
   test_pool<shared_pool_t>(sizeof(void*));
   test_pool<overaligned_pool_t>(64u);
   test_swap();
   return ::boost::report_errors();
}
