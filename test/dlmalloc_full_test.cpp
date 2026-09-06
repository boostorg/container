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
// Load test for dlmalloc. Every bin class of the algorithm is driven in
// turn: the smallbins, the treebins, the designated victim, the top chunk,
// the direct mmap path and the segment list. Each block carries a pattern
// that says which block it is, so a coalescing mistake shows as wrong data
// and not only as a broken walk.
//
// The heap walk (check(), which is do_check_malloc_state()) runs between the
// phases. It only does anything when the header is compiled with assertions
// on, so run this test in a debug build to get the whole of it.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/detail/dlmalloc.hpp>
#include <boost/container/vector.hpp>
#include "lightweight_test.hpp"

#include <cstddef>
#include <cstring>

using boost::container::dlmalloc;
using boost::container::vector;

namespace {

typedef dlmalloc::size_type size_type;

//A repeatable sequence. rand() differs between platforms and would make a
//failure impossible to reproduce.
class rng
{
   public:
   explicit rng(unsigned seed)  : m_state(seed ? seed : 1u) {}

   unsigned next()
   {
      //Numerical Recipes linear congruential generator
      m_state = m_state*1664525u + 1013904223u;
      return m_state >> 8;
   }

   size_type below(size_type n)
   {  return n ? size_type(this->next() % n) : size_type(0);  }

   private:
   unsigned m_state;
};

//One handed-out block, with what was asked for and what was written into it
struct block
{
   block() : ptr(0), bytes(0), tag(0) {}
   block(void *p, size_type b, unsigned char t) : ptr(p), bytes(b), tag(t) {}
   void        *ptr;
   size_type    bytes;
   unsigned char tag;
};

void fill(const block &b)
{  std::memset(b.ptr, b.tag, b.bytes);  }

bool verify(const block &b)
{
   const unsigned char *const p = static_cast<const unsigned char *>(b.ptr);
   for(size_type i = 0; i != b.bytes; ++i)
      if(p[i] != b.tag)
         return false;
   return true;
}

//Hands out a block and records it, or reports the failure
bool take(dlmalloc &h, vector<block> &live, size_type bytes, unsigned char tag)
{
   void *const p = h.allocate(bytes);
   if(!p)
      return false;
   if(dlmalloc::usable_size(p) < bytes)
      return false;
   const block b(p, bytes, tag);
   fill(b);
   live.push_back(b);
   return true;
}

void drop_all(dlmalloc &h, vector<block> &live)
{
   for(std::size_t i = 0; i != live.size(); ++i){
      BOOST_TEST(verify(live[i]));
      h.deallocate(live[i].ptr);
   }
   live.clear();
}

//------------------------------------------------------------------------
// The smallbins: every request under MAX_SMALL_REQUEST has its own bin, and
// freeing neighbours must coalesce them into the next class up
//------------------------------------------------------------------------
void test_smallbin_classes()
{
   dlmalloc h;
   vector<block> live;
   //Cover every small size class, twice, so each bin holds a list and not
   //only a single entry
   for(unsigned pass = 0; pass != 2; ++pass)
      for(size_type bytes = 1; bytes <= 248; ++bytes)
         BOOST_TEST(take(h, live, bytes, (unsigned char)(bytes ^ pass)));
   BOOST_TEST(h.check());

   //Free every second block: the survivors keep their contents, and the gaps
   //cannot coalesce because a used block sits between them
   for(std::size_t i = 0; i < live.size(); i += 2){
      BOOST_TEST(verify(live[i]));
      h.deallocate(live[i].ptr);
      live[i].ptr = 0;
   }
   BOOST_TEST(h.check());
   for(std::size_t i = 1; i < live.size(); i += 2)
      BOOST_TEST(verify(live[i]));

   //Now free the rest, which lets everything coalesce back to one run
   for(std::size_t i = 1; i < live.size(); i += 2)
      h.deallocate(live[i].ptr);
   live.clear();
   BOOST_TEST(h.check());
}

//------------------------------------------------------------------------
// The treebins: requests above MAX_SMALL_REQUEST but below the mmap
// threshold go into the bitwise trie, which is where the unlink code is
// hardest
//------------------------------------------------------------------------
void test_treebin_classes()
{
   dlmalloc h;
   vector<block> live;
   rng r(20260901u);

   //Sizes spread across the whole trie, in an order that is not sorted, so
   //insertions land on every side of every node
   for(unsigned i = 0; i != 400; ++i){
      const size_type bytes = 256 + r.below(200*1024);
      BOOST_TEST(take(h, live, bytes, (unsigned char)i));
   }
   BOOST_TEST(h.check());

   //Remove them in a different order from the one they were made in
   while(!live.empty()){
      const std::size_t i = r.below(live.size());
      BOOST_TEST(verify(live[i]));
      h.deallocate(live[i].ptr);
      live[i] = live.back();
      live.pop_back();
   }
   BOOST_TEST(h.check());
}

//------------------------------------------------------------------------
// The designated victim: dlmalloc keeps the remainder of the last split and
// serves the next small request from it. Repeating one size then changing it
// is what exercises replace_dv().
//------------------------------------------------------------------------
void test_designated_victim()
{
   dlmalloc h;
   vector<block> live;
   for(unsigned round = 0; round != 50; ++round){
      const size_type bytes = 40 + (round % 7)*24;
      for(unsigned i = 0; i != 20; ++i)
         BOOST_TEST(take(h, live, bytes, (unsigned char)(round*20 + i)));
      //Give half of them back straight away, which is what keeps feeding
      //the victim
      for(unsigned i = 0; i != 10; ++i){
         BOOST_TEST(verify(live.back()));
         h.deallocate(live.back().ptr);
         live.pop_back();
      }
   }
   BOOST_TEST(h.check());
   drop_all(h, live);
   BOOST_TEST(h.check());
}

//------------------------------------------------------------------------
// Above the mmap threshold the block comes straight from the system and
// never enters a bin at all
//------------------------------------------------------------------------
void test_direct_mmap_blocks()
{
   dlmalloc h;
   vector<block> live;
   for(unsigned i = 0; i != 20; ++i){
      const size_type bytes = 512*1024 + i*64*1024;
      BOOST_TEST(take(h, live, bytes, (unsigned char)(i + 1)));
   }
   BOOST_TEST(h.check());

   //A directly mapped block can be grown by mremap where the system has it,
   //and copied where it has not. Either way the contents survive.
   for(std::size_t i = 0; i != live.size(); ++i){
      const size_type newsize = live[i].bytes*2;
      void *const p = h.reallocate(live[i].ptr, newsize);
      BOOST_TEST(p != 0);
      if(p){
         live[i].ptr = p;
         BOOST_TEST(verify(live[i]));      //the old prefix is intact
         live[i].bytes = newsize;
         fill(live[i]);
      }
   }
   BOOST_TEST(h.check());
   drop_all(h, live);
   BOOST_TEST(h.check());

   //Everything mapped directly is unmapped on free, so the heap comes back
   //to about what it started at
   h.trim(0);
   BOOST_TEST(h.footprint() < 1024*1024);
}

//------------------------------------------------------------------------
// Growing past the first segment adds more of them, and sys_trim() gives
// them back
//------------------------------------------------------------------------
void test_segment_growth_and_trim()
{
   dlmalloc h;
   vector<block> live;
   const size_type initial = h.footprint();

   //Ask for far more than the first segment holds, in pieces small enough to
   //stay under the mmap threshold, so the heap must add segments
   for(unsigned i = 0; i != 300; ++i)
      BOOST_TEST(take(h, live, 100*1024, (unsigned char)i));
   BOOST_TEST(h.footprint() > initial);
   BOOST_TEST(h.max_footprint() >= h.footprint());
   BOOST_TEST(h.check());

   drop_all(h, live);
   BOOST_TEST(h.check());

   const size_type peak = h.max_footprint();
   h.trim(0);
   BOOST_TEST(h.footprint() <= peak);
   BOOST_TEST(h.max_footprint() == peak);
   BOOST_TEST(h.check());
}

//------------------------------------------------------------------------
// A long random mix of every operation, with the contents checked all the
// way through
//------------------------------------------------------------------------
void test_random_mix()
{
   dlmalloc h;
   vector<block> live;
   rng r(0xC0FFEEu);
   unsigned char tag = 0;

   for(unsigned step = 0; step != 60000; ++step){
      const unsigned what = r.next() % 100;

      if(live.size() < 32 || what < 45){
         //Allocate, with a size distribution that reaches every bin class
         size_type bytes;
         const unsigned klass = r.next() % 100;
         if(klass < 60)        bytes = 1 + r.below(250);          //smallbins
         else if(klass < 90)   bytes = 256 + r.below(60*1024);    //treebins
         else if(klass < 98)   bytes = 64*1024 + r.below(200*1024);
         else                  bytes = 300*1024 + r.below(300*1024);   //mmap
         void *const p = (klass % 7) == 0
            ? h.allocate_aligned(size_type(16) << (klass % 6), bytes)
            : h.allocate(bytes);
         BOOST_TEST(p != 0);
         if(p){
            const block b(p, bytes, ++tag ? tag : ++tag);
            BOOST_TEST(dlmalloc::usable_size(p) >= bytes);
            fill(b);
            live.push_back(b);
         }
      }
      else if(what < 75){
         //Free one at random
         const std::size_t i = r.below(live.size());
         BOOST_TEST(verify(live[i]));
         h.deallocate(live[i].ptr);
         live[i] = live.back();
         live.pop_back();
      }
      else if(what < 90){
         //Reallocate one at random, up or down
         const std::size_t i = r.below(live.size());
         BOOST_TEST(verify(live[i]));
         const size_type newsize = 1 + r.below(4*live[i].bytes + 64);
         void *const p = h.reallocate(live[i].ptr, newsize);
         BOOST_TEST(p != 0);
         if(p){
            //Whatever still fits must have survived the move
            const size_type kept = newsize < live[i].bytes ? newsize
                                                           : live[i].bytes;
            live[i].ptr = p;
            const size_type asked = live[i].bytes;
            live[i].bytes = kept;
            BOOST_TEST(verify(live[i]));
            (void)asked;
            live[i].bytes = newsize;
            fill(live[i]);
         }
      }
      else if(what < 95){
         //Try to resize one where it stands, mostly down but sometimes up
         const std::size_t i = r.below(live.size());
         const size_type newsize = 1 + r.below(live[i].bytes + 1);
         const size_type oldsize = live[i].bytes;
         if(h.reallocate_in_place(live[i].ptr, newsize) != 0){
            //The block did not move, so the bytes both sizes have are the
            //old ones. Anything the resize added is not written yet.
            live[i].bytes = newsize < oldsize ? newsize : oldsize;
            BOOST_TEST(verify(live[i]));
            live[i].bytes = newsize;
            fill(live[i]);
         }
         else{
            BOOST_TEST(verify(live[i]));
         }
      }
      else if(what < 98){
         //Free a run of them under one lock
         const std::size_t n = live.size() < 16 ? live.size() : 16;
         void *array[16];
         for(std::size_t i = 0; i != n; ++i){
            BOOST_TEST(verify(live[live.size()-1-i]));
            array[i] = live[live.size()-1-i].ptr;
         }
         BOOST_TEST(h.bulk_free(array, n) == 0);
         for(std::size_t i = 0; i != n; ++i)
            live.pop_back();
      }
      else{
         h.trim(0);
      }

      //Walking the whole heap at every step would make the test far too
      //slow, so do it often enough to place a failure inside a short window
      if((step % 2000) == 0)
         BOOST_TEST(h.check());
   }

   BOOST_TEST(h.check());
   drop_all(h, live);
   BOOST_TEST(h.check());
}

//------------------------------------------------------------------------
// Several heaps used in turn must stay apart. A block belongs to the heap
// that made it, and giving it to the wrong one would corrupt both.
//------------------------------------------------------------------------
void test_many_heaps_side_by_side()
{
   const unsigned n = 8;
   dlmalloc heaps[n];
   vector<block> live[n];
   rng r(7u);

   for(unsigned step = 0; step != 4000; ++step){
      const unsigned k = step % n;
      if(live[k].size() < 20 || (r.next() & 1)){
         BOOST_TEST(take(heaps[k], live[k], 1 + r.below(9000),
                         (unsigned char)(k + 1)));
      }
      else{
         const std::size_t i = r.below(live[k].size());
         BOOST_TEST(verify(live[k][i]));
         heaps[k].deallocate(live[k][i].ptr);
         live[k][i] = live[k].back();
         live[k].pop_back();
      }
   }
   for(unsigned k = 0; k != n; ++k){
      BOOST_TEST(heaps[k].check());
      drop_all(heaps[k], live[k]);
      BOOST_TEST(heaps[k].check());
   }
}

//------------------------------------------------------------------------
// The heap must report failure rather than break when it can grow no more
//------------------------------------------------------------------------
void test_exhaustion_is_clean()
{
   dlmalloc h;
   h.set_footprint_limit(2*1024*1024);

   vector<block> live;
   unsigned char tag = 0;
   for(;;){
      void *const p = h.allocate(64*1024);
      if(!p)
         break;
      const block b(p, 64*1024, ++tag ? tag : ++tag);
      fill(b);
      live.push_back(b);
      if(live.size() > 1000){         //the cap must bite well before this
         BOOST_TEST(false);
         break;
      }
   }
   BOOST_TEST(!live.empty());
   BOOST_TEST(h.footprint() <= h.footprint_limit());
   BOOST_TEST(h.check());

   //After a refusal the heap is still usable for what it can serve
   drop_all(h, live);
   BOOST_TEST(h.check());
   void *const p = h.allocate(1024);
   BOOST_TEST(p != 0);
   h.deallocate(p);
   BOOST_TEST(h.check());
}

}  //namespace

int main()
{
   test_smallbin_classes();
   test_treebin_classes();
   test_designated_victim();
   test_direct_mmap_blocks();
   test_segment_growth_and_trim();
   test_random_mix();
   test_many_heaps_side_by_side();
   test_exhaustion_is_clean();
   return boost::report_errors();
}
