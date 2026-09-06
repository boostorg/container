//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2007-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//
// alloc_basic_test.cpp for the dlmalloc class: alloc(), grow(), shrink()
// and the accounting queries against one heap instance.
//
// The container cases of the original are not here. They exercise
// boost::container::allocator, which is bound to the process-wide heap and
// reaches no instance.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/detail/dlmalloc.hpp>
#include "lightweight_test.hpp"

#include <cstddef>
#include <cstring>

using boost::container::dlmalloc;

namespace {

typedef dlmalloc::size_type size_type;

//What one block costs the heap: the bytes the caller may use, plus the fixed
//overhead every block carries. Both are published by the class, so no walk of
//the heap is needed to ask. Right for a block that lives in a segment, which
//is every block here; one big enough for the heap to map on its own carries a
//different overhead.
inline dlmalloc::size_type block_cost(const void *p)
{  return dlmalloc::usable_size(p) + dlmalloc::allocation_payload;  }

//The case of alloc_basic_test.cpp, one heap at a time. Every step checks the
//figure the heap reports against the chunk the block really occupies, which
//is what catches a grow or a shrink that forgets to account for what it did.
bool basic_test()
{
   dlmalloc h;
   size_type received = 0;

   if(!h.all_deallocated())
      return false;

   void *ptr = h.alloc(50, 98, &received);
   if(!ptr)
      return false;
   if(dlmalloc::usable_size(ptr) != received)
      return false;
   if(h.allocated_memory() != block_cost(ptr))
      return false;

   if(h.all_deallocated())
      return false;

   //Grow forwards: anything between received+20 and received+30
   h.grow(ptr, received + 20, received + 30, &received);

   if(h.allocated_memory() != block_cost(ptr))
      return false;
   if(dlmalloc::usable_size(ptr) != received)
      return false;

   //Shrink to somewhere between 100 and 140, and keep the result
   if(!h.shrink(ptr, 100, 140, &received, true))
      return false;
   if(h.allocated_memory() != block_cost(ptr))
      return false;

   //A minimum of zero means "as small as you can"
   if(!h.shrink(ptr, 0, 140, &received, true))
      return false;
   if(h.allocated_memory() != block_cost(ptr))
      return false;

   //Asking for less than the block already is must fail, and change nothing
   if(h.shrink(ptr, 0, received/2, &received, true))
      return false;
   if(h.allocated_memory() != block_cost(ptr))
      return false;
   if(dlmalloc::usable_size(ptr) != received)
      return false;

   h.deallocate(ptr);

   h.check();
   if(!h.all_deallocated())
      return false;
   return true;
}

//A shrink that is only asked about must report what it would give and leave
//the block alone
bool try_shrink_test()
{
   dlmalloc h;
   void *const ptr = h.allocate(1000);
   BOOST_TEST(ptr != 0);
   if(!ptr)
      return false;

   const size_type before      = block_cost(ptr);
   const size_type before_size = dlmalloc::usable_size(ptr);
   std::memset(ptr, 0x5A, before_size);

   //For a shrink the two sizes swap roles: limit_size is the largest the
   //block may stay, preferred_size the smallest it may become
   size_type received = 0;
   dlmalloc::command_ret_t r = h.allocation_command
      (dlmalloc::try_shrink_in_place, 1, 1, 200, 100, &received, ptr);
   BOOST_TEST(r.first == ptr);
   BOOST_TEST(received != 0);
   //Only asked, so nothing moved and nothing was given back
   BOOST_TEST(block_cost(ptr) == before);
   BOOST_TEST(h.allocated_memory() == before);

   //Now do it, and the heap must report the smaller chunk
   r = h.allocation_command
      (dlmalloc::shrink_in_place, 1, 1, 200, 100, &received, ptr);
   BOOST_TEST(r.first == ptr);
   BOOST_TEST(block_cost(ptr) < before);
   BOOST_TEST(h.allocated_memory() == block_cost(ptr));
   //The bytes that are left are the bytes that were there
   const unsigned char *const b = static_cast<const unsigned char *>(ptr);
   for(size_type i = 0; i != received; ++i)
      BOOST_TEST(b[i] == 0x5A);

   h.deallocate(ptr);
   BOOST_TEST(h.all_deallocated());
   return true;
}

//The figure allocated_memory() walks out of the heap must be the sum of the
//chunks the heap handed out, whatever route they came by
void test_accounting_matches_the_blocks()
{
   dlmalloc h;
   BOOST_TEST(h.allocated_memory() == 0);

   void *blocks[64];
   size_type expected = 0;
   for(size_type i = 0; i != 64; ++i){
      size_type received = 0;
      switch(i % 4){
         case 0:  blocks[i] = h.allocate(37*(i+1));                   break;
         case 1:  blocks[i] = h.alloc(20*(i+1), 40*(i+1), &received); break;
         case 2:  blocks[i] = h.allocate_aligned(256, 30*(i+1));      break;
         default: blocks[i] = h.allocate_zeroed(i+1, 16);             break;
      }
      BOOST_TEST(blocks[i] != 0);
      expected += block_cost(blocks[i]);
   }
   BOOST_TEST(h.allocated_memory() == expected);
   BOOST_TEST(h.allocated_memory() == expected);
   BOOST_TEST(!h.all_deallocated());

   //Give them back one at a time; the figure must fall by exactly the chunk
   for(size_type i = 0; i != 64; ++i){
      expected -= block_cost(blocks[i]);
      h.deallocate(blocks[i]);
      BOOST_TEST(h.allocated_memory() == expected);
   }
   BOOST_TEST(h.all_deallocated());
   BOOST_TEST(h.check());
}

//The same must hold once the heap has had to take a second segment from the
//system, which is where the fencepost correction of the walk earns its keep
void test_accounting_over_several_segments()
{
   dlmalloc h;
   void *blocks[200];
   size_type expected = 0;
   const size_type initial_footprint = h.footprint();

   for(size_type i = 0; i != 200; ++i){
      blocks[i] = h.allocate(100*1024);
      BOOST_TEST(blocks[i] != 0);
      expected += block_cost(blocks[i]);
   }
   BOOST_TEST(h.footprint() > initial_footprint);
   BOOST_TEST(h.allocated_memory() == expected);

   for(size_type i = 0; i != 200; ++i)
      h.deallocate(blocks[i]);
   BOOST_TEST(h.allocated_memory() == 0);
   BOOST_TEST(h.all_deallocated());
   BOOST_TEST(h.check());
}

//mallinfo() reports the same heap from the other side
void test_mallinfo_report()
{
   dlmalloc h;
   dlmalloc::mallinfo_t s = h.mallinfo();
   BOOST_TEST(s.uordblks + s.fordblks == h.footprint());
   BOOST_TEST(s.usmblks == h.max_footprint());

   void *const p = h.allocate(500*1024);
   BOOST_TEST(p != 0);
   s = h.mallinfo();
   BOOST_TEST(s.uordblks + s.fordblks == h.footprint());
   BOOST_TEST(s.usmblks >= h.footprint());
   BOOST_TEST(s.uordblks >= 500*1024);

   h.deallocate(p);
   BOOST_TEST(h.check());
}

//Two heaps keep their own figures
void test_accounting_is_per_heap()
{
   dlmalloc a;
   dlmalloc b;
   void *const pa = a.allocate(4096);
   BOOST_TEST(pa != 0);
   BOOST_TEST(a.allocated_memory() == block_cost(pa));
   BOOST_TEST(b.allocated_memory() == 0);
   BOOST_TEST(b.all_deallocated());
   BOOST_TEST(!a.all_deallocated());
   a.deallocate(pa);
   BOOST_TEST(a.all_deallocated());
}

}  //namespace

int main()
{
   BOOST_TEST(basic_test());
   BOOST_TEST(try_shrink_test());
   test_accounting_matches_the_blocks();
   test_accounting_over_several_segments();
   test_mallinfo_report();
   test_accounting_is_per_heap();
   return boost::report_errors();
}
