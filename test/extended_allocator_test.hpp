//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_TEST_EXTENDED_ALLOCATOR_TEST_HPP
#define BOOST_CONTAINER_TEST_EXTENDED_ALLOCATOR_TEST_HPP

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/detail/type_traits.hpp>   //alignment_of
#include <boost/move/detail/iterator_to_raw_pointer.hpp>
#include <boost/move/detail/to_raw_pointer.hpp>
#include <boost/container/throw_exception.hpp>      //bad_alloc_t
#include <boost/container/detail/multiallocation_chain.hpp>

#include <cstddef>
#include <cstdio>
#include <iterator>

//Exercises the "extended" (Version == 2) allocator interface: the node and
//chain operations containers use for bulk allocation. Containers reach these
//only indirectly and only for some sizes, so they are tested here directly.

namespace boost {
namespace container {
namespace test {

//Largest node count any case below asks for. The checks keep the collected
//pointers in a fixed array so the harness itself never allocates, which keeps
//it usable from tests that replace global operator new.
static const std::size_t extended_alloc_max_nodes = 40u;

//Structural invariants that every bulk allocation must satisfy.
template<class Allocator, class Chain>
int check_chain(Chain &chain, std::size_t expected, const char *alloc, const char *what)
{
   typedef typename Allocator::value_type value_type;
   typedef typename Chain::iterator       chain_iterator;


   //The chain must actually link as many nodes as expected.
   if(chain.size() != expected){
      std::printf("  %s / %s: chain.size()=%u, expected %u\n", alloc, what,
                  static_cast<unsigned>(chain.size()), static_cast<unsigned>(expected));
      return 1;
   }

   //Collect the bulk allocations in the array
   void *ptrs[extended_alloc_max_nodes];
   std::size_t walked = 0;
   for(chain_iterator it = chain.begin(), e = chain.end();
       it != e && walked < extended_alloc_max_nodes; ++it){
      ptrs[walked++] = static_cast<void *>(boost::movelib::iterator_to_raw_pointer(it));
   }

   //Check that the chain actually links as many nodes as expected
   if(walked != expected){
      std::printf("  %s / %s: chain reports %u nodes but links %u\n", alloc, what,
                  static_cast<unsigned>(expected), static_cast<unsigned>(walked));
      return 1;
   }

   //Check that the nodes are non-null, aligned, and distinct
   const std::size_t align = boost::container::dtl::alignment_of<value_type>::value;
   for(std::size_t i = 0; i != walked; ++i){
      if(!ptrs[i]){
         std::printf("  %s / %s: node %u is null\n", alloc, what, static_cast<unsigned>(i));
         return 1;
      }
      if(reinterpret_cast<std::size_t>(ptrs[i]) % align){
         std::printf("  %s / %s: node %u misaligned (needs %u)\n", alloc, what,
                     static_cast<unsigned>(i), static_cast<unsigned>(align));
         return 1;
      }
      for(std::size_t j = 0; j != i; ++j){
         if(ptrs[i] == ptrs[j]){
            std::printf("  %s / %s: nodes %u and %u are the same address\n", alloc, what,
                        static_cast<unsigned>(j), static_cast<unsigned>(i));
            return 1;
         }
      }
   }
   return 0;
}

//Writes a distinct byte pattern over every block and read it all back, so
//blocks that overlap are detected.
template<class Allocator>
int check_blocks_are_usable(Allocator &a, std::size_t n, const char *alloc)
{
   typedef typename Allocator::value_type            value_type;
   typedef typename Allocator::pointer               pointer;
   typedef typename Allocator::multiallocation_chain chain_t;

   chain_t chain;
   //allocate_individual's blocks are individually deallocatable with deallocate_one
   a.allocate_individual(n, chain);
   if(check_chain<Allocator>(chain, n, alloc, "allocate_individual (usable)"))
      return 1;
   //Collect the bulk allocations in the array
   pointer nodes[extended_alloc_max_nodes];
   for(std::size_t i = 0; i != n; ++i){
      nodes[i] = chain.pop_front();
   }
   if(!chain.empty()){
      std::printf("  %s: chain not empty after popping every node\n", alloc);
      return 1;
   }
   //Write a distinct byte pattern over every block
   for(std::size_t i = 0; i != n; ++i){
      unsigned char *const raw =
         reinterpret_cast<unsigned char *>(boost::movelib::iterator_to_raw_pointer(nodes[i]));
      for(std::size_t b = 0; b != sizeof(value_type); ++b){
         raw[b] = static_cast<unsigned char>(i + 1u);
      }
   }
   //Read it all back, so blocks that overlap are detected
   int err = 0;
   for(std::size_t i = 0; i != n; ++i){
      const unsigned char *const raw =
         reinterpret_cast<const unsigned char *>(boost::movelib::iterator_to_raw_pointer(nodes[i]));
      for(std::size_t b = 0; b != sizeof(value_type); ++b){
         if(raw[b] != static_cast<unsigned char>(i + 1u)){
            std::printf("  %s: block %u byte %u overwritten -> blocks overlap\n", alloc,
                        static_cast<unsigned>(i), static_cast<unsigned>(b));
            err = 1;
            break;
         }
      }
      if(err) break;
   }

   //allocate_individual's blocks are individually deallocatable with deallocate_one
   for(std::size_t i = 0; i != n; ++i){
      a.deallocate_one(nodes[i]);
   }
   return err;
}

//The iterator of the type-erased chain: its values are the addresses of the
//blocks, returned by value. That makes it a C++17 input iterator and a
//C++20 forward iterator. The typed chain of an allocator returns T&, so it
//is a forward iterator in both.
template<class Allocator>
void check_chain_iterator_types()
{
   typedef typename Allocator::value_type                      value_type;
   typedef typename Allocator::multiallocation_chain::iterator typed_it;
   typedef boost::container::dtl::basic_multiallocation_chain<void*> void_chain_t;
   typedef typename void_chain_t::iterator                     void_it;
   using boost::container::dtl::is_same;

   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename void_it::value_type, void*>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename void_it::reference,  void*>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename void_it::pointer,    void*>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename void_it::iterator_category, std::input_iterator_tag>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename void_it::iterator_concept, std::forward_iterator_tag>::value));

   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename typed_it::value_type, value_type>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename typed_it::reference,  value_type&>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename typed_it::iterator_category, std::forward_iterator_tag>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename typed_it::iterator_concept, std::forward_iterator_tag>::value));

   #if defined(__cpp_lib_ranges)
   static_assert(std::forward_iterator<void_it>, "void chain iterator is a C++20 forward iterator");
   static_assert(!std::sized_sentinel_for<void_it, void_it>, "distance is not constant time");
   static_assert(std::forward_iterator<typed_it>, "typed chain iterator is a C++20 forward iterator");
   static_assert(!std::sized_sentinel_for<typed_it, typed_it>, "distance is not constant time");
   #endif
}

//!Test allocate_one/deallocate_one, allocate_individual/deallocate_individual,
//!both allocate_many overloads with deallocate_many, and size().
template<class Allocator>
int extended_allocator_test(const char *alloc_name)
{
   typedef typename Allocator::value_type            value_type;
   typedef typename Allocator::pointer               pointer;
   typedef typename Allocator::size_type             size_type;
   typedef typename Allocator::multiallocation_chain chain_t;

   check_chain_iterator_types<Allocator>();

   Allocator a;

   //Counts deliberately include 1: with a single node the first and last of a
   //chain coincide, so a splice that confuses the two still looks correct.
   const std::size_t counts[] = { 1u, 2u, 3u, 8u, 33u };
   const std::size_t ncounts  = sizeof(counts)/sizeof(counts[0]);

   //allocate_one / deallocate_one, repeated so freed nodes get recycled
   for(std::size_t r = 0; r != 4u; ++r){
      pointer p = a.allocate_one();
      if(!p){
         std::printf("  %s: allocate_one returned null\n", alloc_name);
         return 1;
      }
      *p = value_type();
      a.deallocate_one(p);
   }

   //allocate_individual -> deallocate_individual
   for(std::size_t i = 0; i != ncounts; ++i){
      chain_t chain;
      a.allocate_individual(counts[i], chain);
      if(check_chain<Allocator>(chain, counts[i], alloc_name, "allocate_individual"))
         return 1;
      a.deallocate_individual(chain);
   }

   //allocate_individual -> deallocate_one, with the blocks written and verified
   if(check_blocks_are_usable(a, 16u, alloc_name))
      return 1;

   //allocate_many(elem_size, n_elements) -> deallocate_many.
   //Try more than one element per block.
   const size_type elem_sizes[] = { size_type(1), size_type(3) };
   for(std::size_t e = 0; e != sizeof(elem_sizes)/sizeof(elem_sizes[0]); ++e){
      for(std::size_t i = 0; i != ncounts; ++i){
         chain_t chain;
         a.allocate_many(elem_sizes[e], counts[i], chain);
         if(check_chain<Allocator>(chain, counts[i], alloc_name, "allocate_many(elem_size, n)"))
            return 1;
         a.deallocate_many(chain);
      }
   }

   //allocate_many(elem_sizes[], n_elements) -> deallocate_many
   for(std::size_t i = 0; i != ncounts; ++i){
      size_type sizes[extended_alloc_max_nodes];
      for(std::size_t s = 0; s != counts[i]; ++s){
         sizes[s] = size_type(1u + (s % 4u));
      }
      chain_t chain;
      a.allocate_many(sizes, size_type(counts[i]), chain);
      if(check_chain<Allocator>(chain, counts[i], alloc_name, "allocate_many(sizes[], n)"))
         return 1;
      a.deallocate_many(chain);
   }

   //deallocate_individual and deallocate_many leave the chain empty, so it can be
   //used again: a stale chain would free its old nodes a second time
   {
      chain_t chain;
      for(std::size_t r = 0; r != 2u; ++r){
         a.allocate_individual(4u, chain);
         a.deallocate_individual(chain);
         if(!chain.empty()){
            std::printf("  %s: deallocate_individual did not leave the chain empty\n", alloc_name);
            return 1;
         }
         a.allocate_many(size_type(1u), 4u, chain);
         a.deallocate_many(chain);
         if(!chain.empty()){
            std::printf("  %s: deallocate_many did not leave the chain empty\n", alloc_name);
            return 1;
         }
      }
   }

   //A request for 0 elements does not throw and leaves the chain empty
   {
      chain_t chain;
      const size_type size_one = 1;
      a.allocate_individual(0u, chain);
      a.allocate_many(size_one, 0u, chain);
      a.allocate_many(&size_one, size_type(0u), chain);
      if(!chain.empty()){
         std::printf("  %s: a request for 0 elements did not leave the chain empty\n", alloc_name);
         return 1;
      }
   }

   //size(): the number of objects that a block obtained from allocate() can hold
   {
      const size_type k = 4;
      pointer p = a.allocate(k);
      if(!p){
         std::printf("  %s: allocate(4) returned null\n", alloc_name);
         return 1;
      }
      if(a.size(p) < k){
         std::printf("  %s: size() reports %u objects, less than the %u requested\n", alloc_name,
                     static_cast<unsigned>(a.size(p)), static_cast<unsigned>(k));
         a.deallocate(p, k);
         return 1;
      }
      a.deallocate(p, k);
   }

   //allocate_many(elem_size, n_elements) with elem_size*sizeof(T) bigger than
   //size_type: throws bad_alloc, instead of allocating small blocks
   #ifndef BOOST_NO_EXCEPTIONS
   if(sizeof(value_type) > 1u){
      chain_t chain;
      bool thrown = false;
      try{
         a.allocate_many(size_type(size_type(-1)/sizeof(value_type) + 1u), 2u, chain);
      }
      catch(const boost::container::bad_alloc_t &){
         thrown = true;
      }
      if(!thrown || !chain.empty()){
         std::printf("  %s: allocate_many with an overflowing elem_size did not throw bad_alloc\n", alloc_name);
         if(!chain.empty())
            a.deallocate_many(chain);
         return 1;
      }
   }
   #endif   //BOOST_NO_EXCEPTIONS

   return 0;
}

//!deallocate_free_blocks() is only available in pooling allocators
template<class Allocator>
int extended_pool_allocator_test(const char *alloc_name)
{
   typedef typename Allocator::pointer               pointer;
   typedef typename Allocator::multiallocation_chain chain_t;

   if(extended_allocator_test<Allocator>(alloc_name))
      return 1;

   //Return the now-empty blocks to the upstream allocator.
   //Allocating again should work
   Allocator a;
   {
      chain_t chain;
      a.allocate_individual(32u, chain);
      a.deallocate_individual(chain);
   }
   Allocator::deallocate_free_blocks();

   pointer p = a.allocate_one();
   if(!p){
      std::printf("  %s: allocate_one failed after deallocate_free_blocks\n", alloc_name);
      return 1;
   }
   a.deallocate_one(p);
   Allocator::deallocate_free_blocks();
   return 0;
}

}  //namespace test {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_TEST_EXTENDED_ALLOCATOR_TEST_HPP
