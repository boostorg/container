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
// dlmalloc is the mspace algorithm with the mstate as a data member. This
// test covers every entry point of the class once, plus the invariants that
// the original mspace_* interface guarantees.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/detail/dlmalloc.hpp>
#include "lightweight_test.hpp"

#include <cstddef>
#include <cstring>

using boost::container::dlmalloc;

namespace {

typedef dlmalloc::size_type size_type;

bool is_filled(const void *p, unsigned char value, std::size_t n)
{
   const unsigned char *const b = static_cast<const unsigned char *>(p);
   for(std::size_t i = 0; i != n; ++i)
      if(b[i] != value)
         return false;
   return true;
}

//A fresh heap must serve a request and take it back
void test_construct_and_destroy()
{
   {
      dlmalloc h;
      BOOST_TEST(h.footprint() != 0);
      BOOST_TEST(h.max_footprint() >= h.footprint());
      BOOST_TEST(h.check());
   }
   //A heap asked for a definite capacity must serve it in one piece
   {
      dlmalloc h(1024*1024);
      BOOST_TEST(h.footprint() >= 1024*1024);
      void *const p = h.allocate(1024*1024 - 1024);
      BOOST_TEST(p != 0);
      h.deallocate(p);
      BOOST_TEST(h.check());
   }
}

void test_allocate_and_usable_size()
{
   dlmalloc h;
   static const size_type sizes[] = { 1, 7, 8, 16, 17, 64, 100, 255, 256, 1000,
                                      4096, 65000, 300000 };
   const std::size_t count = sizeof(sizes)/sizeof(sizes[0]);
   void *blocks[sizeof(sizes)/sizeof(sizes[0])];

   for(std::size_t i = 0; i != count; ++i){
      blocks[i] = h.allocate(sizes[i]);
      BOOST_TEST(blocks[i] != 0);
      if(!blocks[i])  continue;
      //usable_size() never understates the request
      BOOST_TEST(dlmalloc::usable_size(blocks[i]) >= sizes[i]);
      //dlmalloc guarantees MALLOC_ALIGNMENT on every block
      BOOST_TEST(0 == (reinterpret_cast<std::size_t>(blocks[i])
                       & (2*sizeof(void*) - 1)));
      std::memset(blocks[i], int(0x30 + i), sizes[i]);
   }
   //Writing through one block must not disturb another
   for(std::size_t i = 0; i != count; ++i)
      if(blocks[i])
         BOOST_TEST(is_filled(blocks[i], (unsigned char)(0x30 + i), sizes[i]));

   for(std::size_t i = 0; i != count; ++i)
      h.deallocate(blocks[i]);
   BOOST_TEST(h.check());

   //Both of these are documented no-ops
   h.deallocate(0);
   BOOST_TEST(dlmalloc::usable_size(0) == 0);
   //A request the size machinery cannot represent fails instead of wrapping
   BOOST_TEST(h.allocate(size_type(-1)) == 0);
   BOOST_TEST(h.check());
}

void test_allocate_zeroed()
{
   dlmalloc h;
   for(size_type n = 1; n <= 512; n *= 4){
      void *const p = h.allocate_zeroed(n, 7);
      BOOST_TEST(p != 0);
      if(!p)   continue;
      BOOST_TEST(dlmalloc::usable_size(p) >= n*7);
      BOOST_TEST(is_filled(p, 0, n*7));
      h.deallocate(p);
   }
   //An overflowing product must fail, not wrap round to a small block. The
   //count comes through a volatile so that the compiler cannot follow it
   //into the memset() that clears the block and warn about the bound.
   volatile size_type huge = size_type(-1);
   BOOST_TEST(h.allocate_zeroed(huge, 2) == 0);
   BOOST_TEST(h.check());
}

void test_reallocate()
{
   dlmalloc h;
   //From null, reallocate() behaves as allocate()
   void *p = h.reallocate(0, 100);
   BOOST_TEST(p != 0);
   std::memset(p, 0x5A, 100);

   //Growing keeps the old bytes
   p = h.reallocate(p, 100000);
   BOOST_TEST(p != 0);
   BOOST_TEST(is_filled(p, 0x5A, 100));
   BOOST_TEST(dlmalloc::usable_size(p) >= 100000);

   //Shrinking keeps as much as still fits
   p = h.reallocate(p, 50);
   BOOST_TEST(p != 0);
   BOOST_TEST(is_filled(p, 0x5A, 50));

   //A failed reallocate() leaves the old block alone
   void *const q = h.reallocate(p, size_type(-1));
   BOOST_TEST(q == 0);
   BOOST_TEST(is_filled(p, 0x5A, 50));

   h.deallocate(p);
   BOOST_TEST(h.check());
}

void test_reallocate_in_place()
{
   dlmalloc h;
   void *const p = h.allocate(64);
   BOOST_TEST(p != 0);
   std::memset(p, 0x3C, 64);

   //Anything that fits the block it already has must succeed in place
   const size_type usable = dlmalloc::usable_size(p);
   BOOST_TEST(h.reallocate_in_place(p, usable) == p);
   BOOST_TEST(h.reallocate_in_place(p, 1) == p);
   BOOST_TEST(is_filled(p, 0x3C, 1));

   //An impossible size fails and never moves the block
   BOOST_TEST(h.reallocate_in_place(p, size_type(-1)) == 0);
   BOOST_TEST(is_filled(p, 0x3C, 1));

   h.deallocate(p);
   BOOST_TEST(h.check());
}

void test_allocate_aligned()
{
   dlmalloc h;
   for(size_type align = 1; align <= 8192; align *= 2){
      for(size_type bytes = 1; bytes <= 20000; bytes *= 7){
         void *const p = h.allocate_aligned(align, bytes);
         BOOST_TEST(p != 0);
         if(!p)   continue;
         BOOST_TEST(0 == (reinterpret_cast<std::size_t>(p) & (align - 1)));
         BOOST_TEST(dlmalloc::usable_size(p) >= bytes);
         std::memset(p, 0x11, bytes);
         h.deallocate(p);
      }
   }
   BOOST_TEST(h.check());
}

void test_independent_calloc()
{
   dlmalloc h;
   const size_type n = 32;
   void *chunks[32];
   void **const r = h.independent_calloc(n, 40, chunks);
   BOOST_TEST(r == chunks);
   if(r){
      for(size_type i = 0; i != n; ++i){
         BOOST_TEST(chunks[i] != 0);
         BOOST_TEST(dlmalloc::usable_size(chunks[i]) >= 40);
         BOOST_TEST(is_filled(chunks[i], 0, 40));
      }
      for(size_type i = 0; i != n; ++i)
         h.deallocate(chunks[i]);
   }
   BOOST_TEST(h.check());
}

void test_independent_comalloc()
{
   dlmalloc h;
   const size_type n = 6;
   size_type sizes[6];
   void *chunks[6];
   for(size_type i = 0; i != n; ++i)
      sizes[i] = 10*(i+1);

   void **const r = h.independent_comalloc(n, sizes, chunks);
   BOOST_TEST(r == chunks);
   if(r){
      for(size_type i = 0; i != n; ++i){
         BOOST_TEST(chunks[i] != 0);
         BOOST_TEST(dlmalloc::usable_size(chunks[i]) >= sizes[i]);
         std::memset(chunks[i], int(i+1), sizes[i]);
      }
      for(size_type i = 0; i != n; ++i)
         BOOST_TEST(is_filled(chunks[i], (unsigned char)(i+1), sizes[i]));
      //comalloc puts them all in one block, so freeing the first is enough,
      //which is what the original documents
      h.deallocate(chunks[0]);
   }
   BOOST_TEST(h.check());
}

void test_bulk_free()
{
   dlmalloc h;
   const size_type n = 100;
   void *array[100];
   for(size_type i = 0; i != n; ++i){
      array[i] = h.allocate(24 + i);
      BOOST_TEST(array[i] != 0);
   }
   BOOST_TEST(h.bulk_free(array, n) == 0);
   //Every entry it consumed is nulled
   for(size_type i = 0; i != n; ++i)
      BOOST_TEST(array[i] == 0);
   BOOST_TEST(h.check());
}

void test_footprint_and_trim()
{
   dlmalloc h;
   const size_type initial = h.footprint();
   BOOST_TEST(initial != 0);

   void *const big = h.allocate(4*1024*1024);
   BOOST_TEST(big != 0);
   BOOST_TEST(h.footprint() > initial);
   BOOST_TEST(h.max_footprint() >= h.footprint());
   const size_type peak = h.max_footprint();

   h.deallocate(big);
   h.trim(0);
   //trim() may or may not find something to give back, but it must never
   //grow the heap nor lose the high-water mark
   BOOST_TEST(h.footprint() <= peak);
   BOOST_TEST(h.max_footprint() == peak);
   BOOST_TEST(h.check());
}

//mallinfo().fordblks is a walk, not a counter, so it has to move by what
//the chunks move by - and it has to keep agreeing with footprint() and with
//allocated_memory(), which walk the same heap for different figures.
void test_free_memory()
{
   dlmalloc h;
   const size_type empty = h.mallinfo().fordblks;
   BOOST_TEST(empty != 0);              //top is free, and top is never nothing
   //Nothing is handed out yet and the state is a member, not a chunk, so the
   //whole of what the heap took from the system is free. That fixes the
   //figure absolutely, not just its direction: the padding at the end of the
   //segment counts as free, exactly as dlmalloc counts it in fordblks.
   BOOST_TEST(empty == h.footprint());

   //A block leaves free memory shorter by its chunk, which is the usable
   //size plus what the chunk costs. Never by less than the request.
   void *const p = h.allocate(10000);
   BOOST_TEST(p != 0);
   const size_type after = h.mallinfo().fordblks;
   BOOST_TEST(after < empty);
   BOOST_TEST(empty - after >= 10000u);
   BOOST_TEST(empty - after >= dlmalloc::usable_size(p));

   //Giving it back restores the figure exactly: one chunk out, one chunk in,
   //and the heap asked the system for nothing in between
   h.deallocate(p);
   BOOST_TEST(h.mallinfo().fordblks == empty);
   BOOST_TEST(h.check());

   //Many blocks at once, then all back. They fit in the segment the heap
   //already has, so nothing is asked of the system and the figure returns
   //to exactly what it was.
   void *v[32];
   for(size_type i = 0; i != 32; ++i){
      v[i] = h.allocate(1 + i * 17);
      BOOST_TEST(v[i] != 0);
   }
   BOOST_TEST(h.mallinfo().fordblks < empty);
   for(size_type i = 0; i != 32; ++i)
      h.deallocate(v[i]);
   BOOST_TEST(h.mallinfo().fordblks == empty);
   BOOST_TEST(h.all_deallocated());

   //Enough to make the heap ask for another segment. Once it all comes back
   //the heap holds more than it started with, and all of it is free - it
   //hands segments back on a schedule, not on every free.
   void *w[64];
   for(size_type i = 0; i != 64; ++i){
      w[i] = h.allocate(4000);
      BOOST_TEST(w[i] != 0);
   }
   for(size_type i = 0; i != 64; ++i)
      h.deallocate(w[i]);
   BOOST_TEST(h.mallinfo().fordblks >= empty);
   BOOST_TEST(h.allocated_memory() == 0);
   BOOST_TEST(h.check());

   //A block the heap maps on its own is in no segment, so it changes what
   //the heap took from the system without changing what is free in it
   const size_type before_big = h.mallinfo().fordblks;
   void *const big = h.allocate(4u * 1024u * 1024u);
   BOOST_TEST(big != 0);
   if(big){
      BOOST_TEST(h.mallinfo().fordblks == before_big);
      BOOST_TEST(h.footprint() > before_big);
      h.deallocate(big);
      BOOST_TEST(h.mallinfo().fordblks == before_big);
   }

   //Nothing is handed out any more. What the heap holds is not all free
   //though: a heap that grew carries a record chunk for each segment after
   //the first, and that chunk is in use - the heap made it for itself and
   //never gave it to anybody, which is why allocated_memory() does not count
   //it either. Whether the heap has such records at all depends on where the
   //system put the new segment, so only the inequality is dependable here.
   BOOST_TEST(h.allocated_memory() == 0);
   BOOST_TEST(h.mallinfo().fordblks <= h.footprint());
   BOOST_TEST(h.check());
}

void test_malloc_stats()
{
   dlmalloc h;

   //An empty heap: nothing handed out, and what it holds is what it holds
   {
      const dlmalloc::malloc_stats_t st = h.malloc_stats();
      const dlmalloc::mallinfo_t     mi = h.mallinfo();
      BOOST_TEST(st.system_bytes     == h.footprint());
      BOOST_TEST(st.max_system_bytes == h.max_footprint());
      BOOST_TEST(st.in_use_bytes     == mi.uordblks);
      BOOST_TEST(st.system_bytes     == mi.uordblks + mi.fordblks);
      BOOST_TEST(st.max_system_bytes == mi.usmblks);
   }

   //...and with blocks live, including one big enough to be mapped on its own
   void *const small = h.allocate(1000);
   void *const big   = h.allocate(400*1024);
   BOOST_TEST(small != 0 && big != 0);
   {
      const dlmalloc::malloc_stats_t st = h.malloc_stats();
      const dlmalloc::mallinfo_t     mi = h.mallinfo();
      BOOST_TEST(st.system_bytes     == h.footprint());
      BOOST_TEST(st.max_system_bytes == h.max_footprint());
      BOOST_TEST(st.in_use_bytes     == mi.uordblks);
      BOOST_TEST(st.system_bytes     == mi.uordblks + mi.fordblks);
      BOOST_TEST(st.max_system_bytes == mi.usmblks);
      //and what is handed out cannot exceed what the heap has
      BOOST_TEST(st.in_use_bytes <= st.system_bytes);
      BOOST_TEST(st.system_bytes <= st.max_system_bytes);
   }
   h.deallocate(small);
   h.deallocate(big);
   BOOST_TEST(h.all_deallocated());
   BOOST_TEST(h.check());
}

void test_mallinfo()
{
   dlmalloc h;
   dlmalloc::mallinfo_t mi = h.mallinfo();

   //The fields dlmalloc keeps only for the shape of the struct
   BOOST_TEST(mi.smblks  == 0);
   BOOST_TEST(mi.hblks   == 0);
   BOOST_TEST(mi.fsmblks == 0);

   //An empty heap: one segment, nothing handed out, one free chunk - the top
   BOOST_TEST(mi.arena    == h.footprint());
   BOOST_TEST(mi.hblkhd   == 0);
   BOOST_TEST(mi.ordblks  == 1);
   BOOST_TEST(mi.uordblks == 0);
   BOOST_TEST(mi.fordblks == h.footprint());
   BOOST_TEST(mi.keepcost != 0);
   BOOST_TEST(mi.usmblks  == h.max_footprint());

   //One block out: what is handed out and what is free move by the chunk,
   //and together they still come to everything the heap took
   void *const p = h.allocate(10000);
   BOOST_TEST(p != 0);
   mi = h.mallinfo();
   BOOST_TEST(mi.uordblks >= 10000u);
   BOOST_TEST(mi.uordblks + mi.fordblks == h.footprint());
   BOOST_TEST(mi.fordblks + mi.uordblks == h.footprint());
   BOOST_TEST(mi.hblkhd   == 0);        //small enough to live in the segment
   h.deallocate(p);

   //A block the heap maps on its own is counted apart, in hblkhd, and is no
   //part of the arena
   void *const big = h.allocate(4u * 1024u * 1024u);
   BOOST_TEST(big != 0);
   if(big){
      mi = h.mallinfo();
      BOOST_TEST(mi.hblkhd >= 4u * 1024u * 1024u);
      BOOST_TEST(mi.arena + mi.hblkhd == h.footprint());
      BOOST_TEST(mi.uordblks >= mi.hblkhd);
      h.deallocate(big);
      BOOST_TEST(h.mallinfo().hblkhd == 0);
   }

   //Freeing everything puts the heap back where it started
   mi = h.mallinfo();
   BOOST_TEST(mi.uordblks == 0);
   BOOST_TEST(mi.ordblks  == 1);
   BOOST_TEST(mi.fordblks == h.footprint());
   BOOST_TEST(h.check());

   //Two free chunks where one block is still out between them
   void *const a = h.allocate(64);
   void *const b = h.allocate(64);
   void *const c = h.allocate(64);
   BOOST_TEST(a != 0 && b != 0 && c != 0);
   h.deallocate(a);
   h.deallocate(c);
   mi = h.mallinfo();
   BOOST_TEST(mi.ordblks >= 2);         //the hole a left, and the top
   h.deallocate(b);
   BOOST_TEST(h.mallinfo().ordblks == 1);
   BOOST_TEST(h.check());
}

//What inspect_all() reports, gathered so the test can look at it.
struct inspection
{
   std::size_t blocks;      //every block it reported
   std::size_t live;        //those with bytes in use
   std::size_t live_bytes;  //how many bytes those came to
   bool        ordered;     //it walked in address order
   bool        sane;        //start < end for every one of them
   const char *last_end;
};

void collect(void *start, void *end, dlmalloc::size_type used, void *arg)
{
   inspection &i = *static_cast<inspection *>(arg);
   ++i.blocks;
   if(used){
      ++i.live;
      i.live_bytes += used;
   }
   if(start >= end)
      i.sane = false;
   if(static_cast<const char *>(start) < i.last_end)
      i.ordered = false;
   i.last_end = static_cast<const char *>(end);
}

inspection inspect(const dlmalloc &h)
{
   inspection i = { 0, 0, 0, true, true, 0 };
   h.inspect_all(&collect, &i);
   return i;
}

void test_inspect_all()
{
   dlmalloc h;

   //An empty heap is one block: the top chunk, free
   inspection i = inspect(h);
   BOOST_TEST(i.blocks == 1);
   BOOST_TEST(i.live == 0);
   BOOST_TEST(i.sane && i.ordered);

   //Every block handed out is reported, once, with at least what was asked
   const std::size_t n = 24;
   void *p[n];
   std::size_t asked = 0;
   for(std::size_t k = 0; k != n; ++k){
      p[k] = h.allocate(100 + k * 7);
      BOOST_TEST(p[k] != 0);
      asked += 100 + k * 7;
   }
   i = inspect(h);
   BOOST_TEST(i.live == n);
   BOOST_TEST(i.live_bytes >= asked);
   BOOST_TEST(i.sane && i.ordered);

   //What it says is in use is what mallinfo says, less the top padding and
   //whatever the heap keeps for itself
   BOOST_TEST(i.live_bytes <= h.mallinfo().uordblks);

   //Freeing every other one leaves half reported as live, and the holes
   //show up as free blocks
   for(std::size_t k = 0; k < n; k += 2)
      h.deallocate(p[k]);
   i = inspect(h);
   BOOST_TEST(i.live == n / 2);
   BOOST_TEST(i.blocks > i.live);
   BOOST_TEST(i.sane && i.ordered);

   for(std::size_t k = 1; k < n; k += 2)
      h.deallocate(p[k]);
   i = inspect(h);
   BOOST_TEST(i.live == 0);
   BOOST_TEST(h.check());
}

void test_track_large_chunks()
{
   dlmalloc h;
   const std::size_t big = 1024u * 1024u;   //well past the mmap threshold

   //Untracked: the block is mapped on its own, so it is outside every
   //segment - mallinfo puts it in hblkhd and inspect_all cannot see it
   void *const a = h.allocate(big);
   BOOST_TEST(a != 0);
   BOOST_TEST(h.mallinfo().hblkhd >= big);
   BOOST_TEST(inspect(h).live == 0);
   h.deallocate(a);

   //Turning tracking on says what the setting was, and it was off
   BOOST_TEST(h.track_large_chunks(true) == false);

   //Tracked: the same request comes out of a segment, where both can see it.
   //It need not be the only block the walker calls live - a segment the heap
   //had to add carries a record chunk, which is in use as well - so what has
   //to hold is that the block itself is now among them.
   void *const b = h.allocate(big);
   BOOST_TEST(b != 0);
   const inspection i = inspect(h);
   BOOST_TEST(i.live >= 1);
   BOOST_TEST(i.live_bytes >= big);
   h.deallocate(b);

   //And back: turning it off reports that it was on
   BOOST_TEST(h.track_large_chunks(false) == true);
   BOOST_TEST(h.check());

   //That says nothing about where the NEXT block comes from, though: this
   //heap now holds a segment big enough to serve the request out of memory
   //it already has, and only a request it cannot serve reaches the point
   //where the setting is read. A fresh heap is what shows the setting at
   //work in both positions.
   {
      dlmalloc off;
      void *const c = off.allocate(big);
      BOOST_TEST(c != 0);
      BOOST_TEST(off.mallinfo().hblkhd >= big);   //mapped on its own
      off.deallocate(c);

      dlmalloc on;
      BOOST_TEST(on.track_large_chunks(true) == false);
      void *const d = on.allocate(big);
      BOOST_TEST(d != 0);
      BOOST_TEST(on.mallinfo().hblkhd < big);     //came out of a segment
      BOOST_TEST(inspect(on).live_bytes >= big);  //and the walker sees it

      on.deallocate(d);
      BOOST_TEST(on.check());
   }
}

void test_heap_over_user_memory()
{
   //Aligned to a granularity boundary on purpose. A destructor that wrongly
   //treated this segment as its own would call the system to release it, and
   //on a buffer that is not page-aligned the call simply fails and the
   //mistake goes unseen. Aligned, the release would succeed and take the
   //memory away - so the writes at the end of this test are what catches it.
   static char storage[512u * 1024u];
   const dlmalloc::size_type align = 64u * 1024u;
   char *const buffer = storage +
      ((align - (reinterpret_cast<dlmalloc::size_type>(storage) & (align - 1u))) & (align - 1u));
   const dlmalloc::size_type buffer_size = 256u * 1024u;
   {
      dlmalloc h(buffer, buffer_size);

      //The heap took the buffer and asked the system for nothing
      BOOST_TEST(h.footprint() <= buffer_size);
      BOOST_TEST(h.footprint() >= buffer_size - dlmalloc::size_type(64));
      BOOST_TEST(h.mallinfo().fordblks == h.footprint());

      //And it hands out memory from inside it
      void *const p = h.allocate(1000);
      BOOST_TEST(p != 0);
      BOOST_TEST(static_cast<char *>(p) >= buffer);
      BOOST_TEST(static_cast<char *>(p) < buffer + buffer_size);
      std::memset(p, 0x5A, 1000);
      BOOST_TEST(h.check());
      h.deallocate(p);
      BOOST_TEST(h.all_deallocated());
   }
   //The destructor left the buffer alone - it is still ours to write to
   std::memset(buffer, 0x11, buffer_size);
   BOOST_TEST(buffer[0] == 0x11);
   BOOST_TEST(buffer[buffer_size - 1] == 0x11);

   //A buffer too small for a heap leaves the object empty but usable: every
   //request is simply served from memory it takes for itself
   {
      char tiny[8];
      dlmalloc h(tiny, sizeof(tiny));
      void *const p = h.allocate(64);
      BOOST_TEST(p != 0);
      BOOST_TEST(static_cast<char *>(p) < tiny ||
                 static_cast<char *>(p) >= tiny + sizeof(tiny));
      h.deallocate(p);
      BOOST_TEST(h.check());
   }
}

void test_footprint_limit()
{
   dlmalloc h;
   //No limit by default
   BOOST_TEST(h.footprint_limit() == size_type(-1));

   const size_type limit = h.footprint() + 64*1024;
   BOOST_TEST(h.set_footprint_limit(limit) >= limit);
   BOOST_TEST(h.footprint_limit() >= limit);

   //A request that would push past the cap fails, and the heap stays sane
   BOOST_TEST(h.allocate(64*1024*1024) == 0);
   BOOST_TEST(h.footprint() <= h.footprint_limit());
   BOOST_TEST(h.check());

   //Lifting the cap makes the same request possible again
   h.set_footprint_limit(size_type(-1));
   void *const p = h.allocate(1024*1024);
   BOOST_TEST(p != 0);
   h.deallocate(p);
   BOOST_TEST(h.check());
}

void test_mallopt()
{
   dlmalloc h;
   BOOST_TEST(h.mallopt(dlmalloc::option_trim_threshold, 128*1024));
   BOOST_TEST(h.mallopt(dlmalloc::option_mmap_threshold, 128*1024));
   //Granularity must stay a power of two and at least a page
   BOOST_TEST(!h.mallopt(dlmalloc::option_granularity, 3));
   //An unknown parameter is rejected. It takes a cast to write one at all
   //now, which is the point of the enumeration. Zero is used rather than
   //some large number because a value outside the enumeration range would
   //convert to an unspecified one; zero is inside it and names no option.
   BOOST_TEST(!h.mallopt((dlmalloc::option_t)0, 1));

   //The value is a size, not an int, so a threshold above INT_MAX has to be
   //settable - which is why size_type is its type. The top bit of size_type
   //is such a value on both a 32-bit and a 64-bit target, and it is a power
   //of two, which the granularity needs. Sizes that large cannot then serve
   //a request, so they go to a heap of their own.
   {
      const size_type huge = size_type(1) << (sizeof(size_type)*8u - 1u);
      BOOST_TEST(huge > size_type(0x7FFFFFFFu));   //above INT_MAX either way
      dlmalloc big;
      BOOST_TEST(big.mallopt(dlmalloc::option_mmap_threshold, huge));
      BOOST_TEST(big.mallopt(dlmalloc::option_granularity, huge));
      //-1 still asks for "no limit", the largest size_type
      BOOST_TEST(big.mallopt(dlmalloc::option_trim_threshold, size_type(-1)));
      BOOST_TEST(big.check());
   }

   void *const p = h.allocate(200*1024);
   BOOST_TEST(p != 0);
   h.deallocate(p);
   BOOST_TEST(h.check());
}

//Two heaps must not see each other in any figure
void test_heaps_are_independent()
{
   dlmalloc a;
   dlmalloc b;
   const size_type b_before = b.footprint();

   void *const pa = a.allocate(2*1024*1024);
   BOOST_TEST(pa != 0);
   BOOST_TEST(b.footprint() == b_before);

   void *const pb = b.allocate(64);
   BOOST_TEST(pb != 0);
   BOOST_TEST(pb != pa);

   a.deallocate(pa);
   b.deallocate(pb);
   BOOST_TEST(a.check());
   BOOST_TEST(b.check());
}

//The unlocked flavour takes the same paths without the atomics
void test_unlocked_heap()
{
   dlmalloc h(0, false);
   void *const p = h.allocate(1000);
   BOOST_TEST(p != 0);
   std::memset(p, 0x77, 1000);
   BOOST_TEST(is_filled(p, 0x77, 1000));
   h.deallocate(p);
   BOOST_TEST(h.check());
}

//Blocks still held when the heap dies go away with it
void test_destructor_releases_everything()
{
   size_type footprint = 0;
   {
      dlmalloc h;
      for(size_type i = 0; i != 500; ++i)
         BOOST_TEST(h.allocate(1000 + i) != 0);
      footprint = h.footprint();
      BOOST_TEST(footprint > 500*1000);
   }
   //Nothing can be asserted on the released memory itself. Reaching this
   //point without a fault, and with no complaint from a leak checker, is the
   //result.
   BOOST_TEST(footprint != 0);
}

}  //namespace

int main()
{
   test_construct_and_destroy();
   test_allocate_and_usable_size();
   test_allocate_zeroed();
   test_reallocate();
   test_reallocate_in_place();
   test_allocate_aligned();
   test_independent_calloc();
   test_independent_comalloc();
   test_bulk_free();
   test_footprint_and_trim();
   test_free_memory();
   test_mallinfo();
   test_malloc_stats();
   test_inspect_all();
   test_track_large_chunks();
   test_heap_over_user_memory();
   test_footprint_limit();
   test_mallopt();
   test_heaps_are_independent();
   test_unlocked_heap();
   test_destructor_releases_everything();
   return boost::report_errors();
}
