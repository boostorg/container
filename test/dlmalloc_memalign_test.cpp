//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
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

using boost::container::dlmalloc;
using boost::container::vector;

namespace {

typedef dlmalloc::size_type size_type;

bool is_aligned_to(const void *p, size_type align)
{  return 0 == (reinterpret_cast<std::size_t>(p) & (align - 1));  }

void test_alignment_and_size()
{
   dlmalloc h;
   static const size_type aligns[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512,
                                       1024, 4096, 16384, 65536 };
   for(std::size_t a = 0; a != sizeof(aligns)/sizeof(aligns[0]); ++a){
      const size_type align = aligns[a];
      for(size_type bytes = 1; bytes <= 8192; bytes *= 3){
         void *const p = h.allocate_aligned(align, bytes);
         BOOST_TEST(p != 0);
         if(!p)   continue;
         BOOST_TEST(is_aligned_to(p, align));
         //The block really is as large as it says: writing all of it must not
         //touch the chunk that follows
         const size_type usable = dlmalloc::usable_size(p);
         BOOST_TEST(usable >= bytes);
         std::memset(p, 0xA5, usable);
         h.deallocate(p);
         BOOST_TEST(h.check());
      }
   }
}

//An alignment below MALLOC_ALIGNMENT is raised to it, and one that is not a
//power of two is rounded up to the next one
void test_degenerate_alignments()
{
   dlmalloc h;
   const size_type natural = 2*sizeof(void*);

   void *p = h.allocate_aligned(1, 100);
   BOOST_TEST(p != 0);
   BOOST_TEST(is_aligned_to(p, natural));
   h.deallocate(p);

   p = h.allocate_aligned(3, 100);
   BOOST_TEST(p != 0);
   BOOST_TEST(is_aligned_to(p, 4));
   h.deallocate(p);

   p = h.allocate_aligned(100, 100);
   BOOST_TEST(p != 0);
   BOOST_TEST(is_aligned_to(p, 128));
   h.deallocate(p);

   //The largest alignment a size_type can hold. What matters is that the
   //heap never answers with a pointer that is not aligned the way it was
   //asked: it may fail, and it may succeed, but it may not do neither.
   //
   //Whether it can succeed is a question about the address space, not about
   //the allocator. Serving this needs a block about that big, so a 64-bit
   //process cannot, and neither can a 32-bit one on Windows with its 2GB of
   //user address space - but a 32-bit process on Linux has room for a 2GB
   //mapping and does serve it, returning 0x80000000.
   p = h.allocate_aligned(size_type(1) << (sizeof(size_type)*8 - 1), 100);
   if(p){
      BOOST_TEST(is_aligned_to(p, size_type(1) << (sizeof(size_type)*8 - 1)));
      h.deallocate(p);
   }
   BOOST_TEST(h.check());
}

//Allocate a batch and free it: the heap must come back to the figure it
//started from, which only holds if every remainder was returned
void test_accounting_round_trip()
{
   dlmalloc h;
   const size_type before = h.footprint();

   vector<void *> blocks;
   for(size_type i = 0; i != 200; ++i){
      const size_type align = size_type(64) << (i % 5);   //64..1024
      void *const p = h.allocate_aligned(align, 17*(i+1));
      BOOST_TEST(p != 0);
      if(p){
         BOOST_TEST(is_aligned_to(p, align));
         blocks.push_back(p);
      }
   }
   BOOST_TEST(h.footprint() > before);
   BOOST_TEST(h.check());

   for(std::size_t i = 0; i != blocks.size(); ++i)
      h.deallocate(blocks[i]);
   BOOST_TEST(h.check());

   //Everything is free again, so trimming must bring the heap back down
   h.trim(0);
   BOOST_TEST(h.footprint() <= before);
}

//Interleave the over-aligned path with the plain one, so that a free list
//left inconsistent by either shows up in the next walk
void test_interleaved_with_plain()
{
   dlmalloc h;
   vector<void *> blocks;
   for(size_type i = 0; i != 500; ++i){
      void *p;
      if(i & 1){
         p = h.allocate_aligned(256, 40+i);
         BOOST_TEST(p == 0 || is_aligned_to(p, 256));
      }
      else{
         p = h.allocate(40+i);
      }
      BOOST_TEST(p != 0);
      if(p){
         BOOST_TEST(dlmalloc::usable_size(p) >= 40+i);
         blocks.push_back(p);
      }
   }
   BOOST_TEST(h.check());
   for(std::size_t i = 0; i != blocks.size(); ++i)
      h.deallocate(blocks[i]);
   BOOST_TEST(h.check());
}

#if !defined(BOOST_NO_CXX11_HDR_THREAD)

//One shared heap. dlmalloc takes its own lock in every entry point, so the
//whole of internal_memalign, including the two splits, runs in one critical
//section. Each thread counts into its own slot, so the count itself needs no
//synchronization.
void hammer(dlmalloc *h, unsigned seed, unsigned *out_errors)
{
   vector<void *> blocks;
   unsigned errors = 0;
   for(unsigned i = 0; i != 4000; ++i){
      const size_type align = size_type(64) << ((i + seed) % 4);
      const size_type bytes = 8 + ((i*37u + seed) % 900);
      void *const p = h->allocate_aligned(align, bytes);
      if(p){
         if(!is_aligned_to(p, align))
            ++errors;
         //usable_size() is NOT asked for here. It reads the block's header,
         //and that word also holds the in-use bit of the block before it,
         //so another thread allocating or freeing a neighbour writes it -
         //a real data race, and one ThreadSanitizer reports. The
         //single-threaded cases above check the size; this one checks that
         //concurrent aligned allocation stays correct and consistent.
         std::memset(p, int(seed), bytes);
         blocks.push_back(p);
      }
      if(blocks.size() > 64){
         h->deallocate(blocks.back());
         blocks.pop_back();
      }
   }
   for(std::size_t i = 0; i != blocks.size(); ++i)
      h->deallocate(blocks[i]);
   *out_errors = errors;
}

const unsigned num_threads = 8;

void test_threaded()
{
   dlmalloc h;
   unsigned errors[num_threads] = {};
   std::vector<std::thread> threads;
   for(unsigned i = 0; i != num_threads; ++i)
      threads.push_back(std::thread(hammer, &h, i, &errors[i]));
   for(unsigned i = 0; i != num_threads; ++i)
      threads[i].join();
   for(unsigned i = 0; i != num_threads; ++i)
      BOOST_TEST(errors[i] == 0);
   BOOST_TEST(h.check());
}

//One heap per thread, built without the lock: nothing is shared, so the
//unlocked flavour must survive the same load
void test_threaded_unlocked_private_heaps()
{
   unsigned errors[num_threads] = {};
   std::vector<std::thread> threads;
   for(unsigned i = 0; i != num_threads; ++i){
      unsigned *const slot = &errors[i];
      threads.push_back(std::thread([i, slot]{
         dlmalloc local(0, false);
         hammer(&local, i, slot);
         if(!local.check())
            ++*slot;
      }));
   }
   for(unsigned i = 0; i != num_threads; ++i)
      threads[i].join();
   for(unsigned i = 0; i != num_threads; ++i)
      BOOST_TEST(errors[i] == 0);
}

#endif   //!defined(BOOST_NO_CXX11_HDR_THREAD)

//A directly mapped block that went through memalign keeps its alignment
//slack in front of the chunk, and prev_foot records how much. dlmalloc 2.8.6
//sized the mapping a realloc grows or shrinks that block into without that
//slack, so on a target with mremap the block came back smaller than asked:
//realloc(memalign(4096, 300000), 4201) gave 4064 usable bytes, and writing
//the 4201 ran off the end of the mapping. Without mremap the resize falls
//back to allocate, copy and free, which is why it never showed on Windows.
//dlmalloc corrects the formula; this holds it there, both ways, and on
//every target.
void test_realloc_of_aligned_mapped_block()
{
   dlmalloc h;
   const size_type big = 300000u;   //past the default mmap threshold
   void *p = h.allocate_aligned(4096u, big);
   BOOST_TEST(p != 0);
   if(!p)  return;
   ::std::memset(p, 0x5A, big);

   //shrink, well below the old size but still a large request
   const size_type smaller = 4201u;
   void *q = h.reallocate(p, smaller);
   BOOST_TEST(q != 0);
   if(!q)  return;
   BOOST_TEST(dlmalloc::usable_size(q) >= smaller);
   bool kept = true;
   for(size_type i = 0; i != smaller; ++i)
      kept = kept && static_cast<unsigned char *>(q)[i] == 0x5A;
   BOOST_TEST(kept);
   ::std::memset(q, 0xA5, smaller);   //every byte the size promises

   //grow again, back past the threshold
   const size_type larger = 400000u;
   void *r = h.reallocate(q, larger);
   BOOST_TEST(r != 0);
   if(!r)  return;
   BOOST_TEST(dlmalloc::usable_size(r) >= larger);
   kept = true;
   for(size_type i = 0; i != smaller; ++i)
      kept = kept && static_cast<unsigned char *>(r)[i] == 0xA5;
   BOOST_TEST(kept);
   ::std::memset(r, 0x3C, larger);
   h.deallocate(r);
   BOOST_TEST(h.check());
   BOOST_TEST(h.all_deallocated());
}

}  //namespace

int main()
{
   test_alignment_and_size();
   test_degenerate_alignments();
   test_accounting_round_trip();
   test_interleaved_with_plain();
   test_realloc_of_aligned_mapped_block();
   #if !defined(BOOST_NO_CXX11_HDR_THREAD)
   test_threaded();
   test_threaded_unlocked_private_heaps();
   #endif
   return boost::report_errors();
}
