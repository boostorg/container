//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//
// Over-aligned BOOST_CONTAINER_ALLOCATE_NEW, i.e. the mspace_memalign_lockless
// path of boost_cont_allocation_command. The other alloc_* tests always pass
// alignof_object == 1 and so never take that branch at all.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/detail/dlmalloc.hpp>
#include <boost/container/vector.hpp>
#include "lightweight_test.hpp"

#include <cstddef>
#include <cstring>

#if !defined(BOOST_NO_CXX11_HDR_THREAD)
#include <thread>
#include <vector>
#endif

using namespace boost::container;

namespace {

//One allocation through the command interface at the requested alignment
void *aligned_new(std::size_t align, std::size_t bytes, std::size_t &received)
{
   received = 0;
   boost_cont_command_ret_t r = dlmalloc_allocation_command
      (BOOST_CONTAINER_ALLOCATE_NEW, 1u, align, bytes, bytes, &received, 0);
   BOOST_TEST(r.second == 0);    //a fresh block is never reported as reused
   return r.first;
}

void test_alignment_and_size()
{
   static const std::size_t aligns[] = { 16, 32, 64, 128, 256, 512, 1024, 4096 };
   for(std::size_t a = 0; a != sizeof(aligns)/sizeof(aligns[0]); ++a){
      const std::size_t align = aligns[a];
      for(std::size_t bytes = 1; bytes <= 8192; bytes *= 3){
         std::size_t received = 0;
         void *p = aligned_new(align, bytes, received);
         BOOST_TEST(p != 0);
         if(!p)   continue;
         BOOST_TEST(0 == (reinterpret_cast<std::size_t>(p) & (align - 1)));
         //The bookkeeping really ran: this is what the old code skipped when
         //it failed to reacquire the lock after internal_memalign
         BOOST_TEST(received >= bytes);
         BOOST_TEST(received == dlmalloc_size(p));
         std::memset(p, 0xA5, received);
         dlmalloc_free(p);
      }
   }
}

//Allocate a batch and free it: the heap must report exactly the figure it
//started from, which only holds if every block was accounted for
void test_accounting_round_trip()
{
   BOOST_TEST(dlmalloc_all_deallocated() != 0);
   const std::size_t before = dlmalloc_allocated_memory();

   vector<void *> blocks;
   for(std::size_t i = 0; i != 200; ++i){
      std::size_t received = 0;
      const std::size_t align = std::size_t(64) << (i % 5);   //64..1024
      void *p = aligned_new(align, 17*(i+1), received);
      BOOST_TEST(p != 0);
      BOOST_TEST(received != 0);
      if(p){
         BOOST_TEST(0 == (reinterpret_cast<std::size_t>(p) & (align - 1)));
         blocks.push_back(p);
      }
   }
   BOOST_TEST(dlmalloc_allocated_memory() > before);
   for(std::size_t i = 0; i != blocks.size(); ++i)
      dlmalloc_free(blocks[i]);
   BOOST_TEST(dlmalloc_allocated_memory() == before);
   BOOST_TEST(dlmalloc_malloc_check() != 0);
}

//Interleave the over-aligned path with the plain one, so that a lock left in
//the wrong state by either would show up as heap corruption
void test_interleaved_with_plain()
{
   vector<void *> blocks;
   for(std::size_t i = 0; i != 500; ++i){
      std::size_t received = 0;
      void *p;
      if(i & 1){
         p = aligned_new(256, 40+i, received);
         BOOST_TEST(p == 0 || 0 == (reinterpret_cast<std::size_t>(p) & 255u));
      }
      else{
         p = aligned_new(1, 40+i, received);
      }
      BOOST_TEST(p != 0);
      BOOST_TEST(received >= 40+i);
      if(p)  blocks.push_back(p);
   }
   BOOST_TEST(dlmalloc_malloc_check() != 0);
   for(std::size_t i = 0; i != blocks.size(); ++i)
      dlmalloc_free(blocks[i]);
   BOOST_TEST(dlmalloc_malloc_check() != 0);
}

#if !defined(BOOST_NO_CXX11_HDR_THREAD)

unsigned thread_errors = 0;

void hammer(unsigned seed)
{
   vector<void *> blocks;
   for(unsigned i = 0; i != 4000; ++i){
      std::size_t received = 0;
      const std::size_t align = std::size_t(64) << ((i + seed) % 4);
      boost_cont_command_ret_t r = dlmalloc_allocation_command
         ( BOOST_CONTAINER_ALLOCATE_NEW, 1u, align
         , 8 + ((i*37u + seed) % 900), 8 + ((i*37u + seed) % 900), &received, 0);
      if(r.first){
         //Not BOOST_TEST: it is not documented as thread-safe
         if(0 != (reinterpret_cast<std::size_t>(r.first) & (align - 1)))
            ++thread_errors;
         if(0 == received)
            ++thread_errors;
         std::memset(r.first, int(seed), received);
         blocks.push_back(r.first);
      }
      if(blocks.size() > 64){
         dlmalloc_free(blocks.back());
         blocks.pop_back();
      }
   }
   for(std::size_t i = 0; i != blocks.size(); ++i)
      dlmalloc_free(blocks[i]);
}

//The whole operation now runs inside a single critical section; getting the
//lock discipline wrong would trip the consistency check under this load
void test_threaded()
{
   const unsigned num_threads = 8;
   std::vector<std::thread> threads;
   for(unsigned i = 0; i != num_threads; ++i)
      threads.push_back(std::thread(hammer, i));
   for(unsigned i = 0; i != num_threads; ++i)
      threads[i].join();
   BOOST_TEST(thread_errors == 0);
   BOOST_TEST(dlmalloc_malloc_check() != 0);
}

#endif   //!defined(BOOST_NO_CXX11_HDR_THREAD)

}  //namespace

int main()
{
   test_alignment_and_size();
   test_accounting_round_trip();
   test_interleaved_with_plain();
   #if !defined(BOOST_NO_CXX11_HDR_THREAD)
   test_threaded();
   #endif
   return boost::report_errors();
}
