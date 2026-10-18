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
// alloc_full_test.cpp for the dlmalloc class: the same cases against one
// heap instance instead of the process-wide one. The chain macros become
// member functions of dlmalloc::multiallocation_chain, and all_deallocated() now walks
// the heap rather than reading a counter.
//
//////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (disable:4702)
#pragma warning (disable:4530) // C++ exception handler used, but unwind semantics are not enabled
#endif

#include <iostream>
#include <cstring>
#include <algorithm>    //std::remove
#include <boost/container/detail/dlmalloc.hpp>
#include <boost/container/vector.hpp>
#include <boost/move/utility_core.hpp>

namespace boost { namespace container { namespace test {

//The one heap every case below works on. The original test reaches the
//process-wide heap through the dlmalloc_* functions; here the same calls are
//member functions of this instance.
static dlmalloc h;

static const std::size_t NumIt = 200;

//The check the original test wanted here and could not have: that a round
//which gives every block back leaves the heap holding what it held before.
//
//mallinfo().fordblks walks the heap, so it says what is free right now, and
//once everything is deallocated the only thing standing between it and
//footprint() is the record chunk each segment after the first carries - an
//in-use chunk the heap made for itself and never gave to anybody. Records
//come and go only with segments, so:
//
//  - the heap asked the system for nothing: the figure has to be back to
//    exactly what it was;
//  - the heap grew: it may hold more free memory now, but never more than it
//    grew by, since each new segment spends part of itself on its record;
//  - the heap gave a segment back: it may hold less, by the same reasoning.
//
//In every case fordblks cannot exceed footprint(), and what the heap has
//handed out has to be nothing at all.
//What holds at any moment, blocks live or not: everything the heap took from
//the system is either free, or handed out, or spent on a segment record. So
//the first two can never come to more than the whole. The round-trip check
//below cannot see this, because it compares two moments that are wrong in the
//same way whenever fordblks is wrong by a constant.
static bool heap_accounting_holds()
{
   return h.mallinfo().fordblks + h.allocated_memory() <= h.footprint();
}

static bool free_memory_returned(std::size_t free_before, std::size_t footprint_before)
{
   const std::size_t free_now      = h.mallinfo().fordblks;
   const std::size_t footprint_now = h.footprint();

   if(free_now > footprint_now)
      return false;
   if(h.allocated_memory() != 0)
      return false;
   if(footprint_now == footprint_before)
      return free_now == free_before;
   if(footprint_now > footprint_before)
      return free_now >= free_before &&
             (free_now - free_before) <= (footprint_now - footprint_before);
   return free_now <= free_before &&
          (free_before - free_now) <= (footprint_before - footprint_now);
}


enum deallocation_type { DirectDeallocation, InverseDeallocation, MixedDeallocation, EndDeallocationType };

//This test allocates until there is no more memory
//and after that deallocates all in the inverse order

bool test_allocation()
{
   if(!h.all_deallocated())
      return false;
   h.check();
   for( deallocation_type t = DirectDeallocation
      ; t != EndDeallocationType
      ; t = (deallocation_type)((int)t + 1)){
      boost::container::vector<void*> buffers;
      const std::size_t free_before      = h.mallinfo().fordblks;
      const std::size_t footprint_before = h.footprint();

      for(std::size_t i = 0; i != NumIt; ++i){
         void *ptr = h.allocate(i);
         if(!ptr)
            break;
         buffers.push_back(ptr);
      }
      if(!heap_accounting_holds())
         return false;

      switch(t){
         case DirectDeallocation:
         {
            for(std::size_t j = 0, max = buffers.size()
               ;j < max
               ;++j){
               h.deallocate(buffers[j]);
            }
         }
         break;
         case InverseDeallocation:
         {
            for(std::size_t j = buffers.size()
               ;j--
               ;){
               h.deallocate(buffers[j]);
            }
         }
         break;
         case MixedDeallocation:
         {
            for(std::size_t j = 0, max = buffers.size()
               ;j < max
               ;++j){
               std::size_t pos = (j%4)*(buffers.size())/4;
               h.deallocate(buffers[pos]);
               buffers.erase(buffers.begin()+(std::ptrdiff_t)pos);
            }
         }
         break;
         default:
         break;
      }
      if(!h.all_deallocated())
         return false;
      if(!h.all_deallocated())
         return false;
      if(!heap_accounting_holds())
         return false;
      if(!free_memory_returned(free_before, footprint_before))
         return false;
      h.check();
   }
   h.check();
   return 0 != h.all_deallocated();
}

//This test allocates until there is no more memory
//and after that tries to shrink all the buffers to the
//half of the original size

bool test_allocation_shrink()
{
   h.check();
   boost::container::vector<void*> buffers;

   //Allocate buffers with extra memory
   for(std::size_t i = 0; i != NumIt; ++i){
      void *ptr = h.allocate(i*2u);
      if(!ptr)
         break;
      buffers.push_back(ptr);
   }

   //Now shrink to half
   for(std::size_t i = 0, max = buffers.size()
      ;i < max
      ; ++i){
      std::size_t try_received_size = 0;
      void* try_result = h.allocation_command
               ( dlmalloc::try_shrink_in_place, 1, 1, i*2
               , i, &try_received_size, (char*)buffers[i]).first;

      std::size_t received_size = 0;
      void* result = h.allocation_command
         ( dlmalloc::shrink_in_place, 1, 1, i*2
         , i, &received_size, (char*)buffers[i]).first;

      if(result != try_result)
         return false;

      if(received_size != try_received_size)
         return false;

      if(result){
         if(received_size > std::size_t(i*2)){
            return false;
         }
         if(received_size < std::size_t(i)){
            return false;
         }
      }
   }

   //Deallocate it in non sequential order
   for(std::size_t j = 0, max = buffers.size()
      ;j < max
      ;++j){
      std::size_t pos = (j%4u)*(buffers.size())/4u;
      h.deallocate(buffers[pos]);
      buffers.erase(buffers.begin()+(std::ptrdiff_t)pos);
   }
   h.check();
   return 0 != h.all_deallocated();//a.all_memory_deallocated() && a.check_sanity();
}

//This test allocates until there is no more memory
//and after that tries to expand all the buffers to
//avoid the wasted internal fragmentation

bool test_allocation_expand()
{
   h.check();
   boost::container::vector<void*> buffers;

   //Allocate buffers with extra memory
   for(std::size_t i = 0; i != NumIt; ++i){
      void *ptr = h.allocate(i);
      if(!ptr)
         break;
      buffers.push_back(ptr);
   }

   //Now try to expand to the double of the size
   for(std::size_t i = 0, max = buffers.size()
      ;i < max
      ;++i){
      std::size_t received_size = 0;
      std::size_t min_size = i+1;
      std::size_t preferred_size = i*2;
      preferred_size = min_size > preferred_size ? min_size : preferred_size;
      while(h.allocation_command
         ( dlmalloc::expand_fwd, 1, 1, min_size
         , preferred_size, &received_size, (char*)buffers[i]).first){
         //Check received size is bigger than minimum
         if(received_size < min_size){
            return false;
         }
         //Now, try to expand further
         min_size       = received_size+1;
         preferred_size = min_size*2;
      }
   }

   //Deallocate it in non sequential order
   for(std::size_t j = 0, max = buffers.size()
      ;j < max
      ;++j){
      std::size_t pos = (j%4u)*(buffers.size())/4u;
      h.deallocate(buffers[pos]);
      buffers.erase(buffers.begin()+(std::ptrdiff_t)pos);
   }
   h.check();
   return 0 != h.all_deallocated();//a.all_memory_deallocated() && a.check_sanity();
}

//This test allocates until there is no more memory
//and after that tries to expand all the buffers to
//avoid the wasted internal fragmentation
bool test_allocation_shrink_and_expand()
{
   boost::container::vector<void*> buffers;
   boost::container::vector<std::size_t> received_sizes;
   boost::container::vector<bool>        size_reduced;

   //Allocate buffers wand store received sizes
   for(std::size_t i = 0; i != NumIt; ++i){
      std::size_t received_size = 0;
      void *ptr = h.allocation_command
         (dlmalloc::allocate_new, 1u, 1u, i, i*2u, &received_size, 0).first;
      if(!ptr){
         ptr = h.allocation_command
            ( dlmalloc::allocate_new, 1u, 1u, 1u, i*2, &received_size, 0).first;
         if(!ptr)
            break;
      }
      buffers.push_back(ptr);
      received_sizes.push_back(received_size);
   }

   //Now shrink to half
   for(std::size_t i = 0, max = buffers.size()
      ; i < max
      ; ++i){
      std::size_t received_size = 0;
      bool size_reduced_flag;
      if(true == (size_reduced_flag = !!
         h.allocation_command
         ( dlmalloc::shrink_in_place, 1, 1, received_sizes[i]
         , i, &received_size, (char*)buffers[i]).first)){
         if(received_size > std::size_t(received_sizes[i])){
            return false;
         }
         if(received_size < std::size_t(i)){
            return false;
         }
      }
      size_reduced.push_back(size_reduced_flag);
   }

   //Now try to expand to the original size
   for(std::size_t i = 0, max = buffers.size()
      ;i < max
      ;++i){
      if(!size_reduced[i])  continue;
      std::size_t received_size = 0;
      std::size_t request_size =  received_sizes[i];
      if(h.allocation_command
         ( dlmalloc::expand_fwd, 1, 1, request_size
         , request_size, &received_size, (char*)buffers[i]).first){
         if(received_size != request_size){
            return false;
         }
      }
      else{
         return false;
      }
   }

   //Deallocate it in non sequential order
   for(std::size_t j = 0, max = buffers.size()
      ;j < max
      ;++j){
      std::size_t pos = (j%4u)*(buffers.size())/4u;
      h.deallocate(buffers[pos]);
      buffers.erase(buffers.begin()+(std::ptrdiff_t)pos);
   }

   return 0 != h.all_deallocated();//a.all_memory_deallocated() && a.check_sanity();
}

//This test allocates until there is no more memory
//and after that deallocates the odd buffers to
//make room for expansions. The expansion will probably
//success since the deallocation left room for that.

bool test_allocation_deallocation_expand()
{
   h.check();
   boost::container::vector<void*> buffers;

   //Allocate buffers with extra memory
   for(std::size_t i = 0; i != NumIt; ++i){
      void *ptr = h.allocate(i);
      if(!ptr)
         break;
      buffers.push_back(ptr);
   }

   //Now deallocate the half of the blocks
   //so expand maybe can merge new free blocks
   for(std::size_t i = 0, max = buffers.size()
      ;i < max
      ;++i){
      if(i%2){
         h.deallocate(buffers[i]);
         buffers[i] = 0;
      }
   }

   //Now try to expand to the double of the size
   for(std::size_t i = 0, max = buffers.size()
      ;i < max
      ;++i){
      //
      if(buffers[i]){
         std::size_t received_size = 0;
         std::size_t min_size = i+1;
         std::size_t preferred_size = i*2;
         preferred_size = min_size > preferred_size ? min_size : preferred_size;

         while(h.allocation_command
            ( dlmalloc::expand_fwd, 1, 1, min_size
            , preferred_size, &received_size, (char*)buffers[i]).first){
            //Check received size is bigger than minimum
            if(received_size < min_size){
               return false;
            }
            //Now, try to expand further
            min_size       = received_size+1;
            preferred_size = min_size*2;
         }
      }
   }

   //Now erase null values from the vector
   buffers.erase(std::remove(buffers.begin(), buffers.end(), (void*)0)
                ,buffers.end());

   //Deallocate it in non sequential order
   for(std::size_t j = 0, max = buffers.size()
      ;j < max
      ;++j){
      std::size_t pos = (j%4u)*(buffers.size())/4u;
      h.deallocate(buffers[pos]);
      buffers.erase(buffers.begin()+(std::ptrdiff_t)pos);
   }
   h.check();
   return 0 != h.all_deallocated();//a.all_memory_deallocated() && a.check_sanity();
}

//This test allocates until there is no more memory
//and after that deallocates all except the last.
//If the allocation algorithm is a bottom-up algorithm
//the last buffer will be in the end of the segment.
//Then the test will start expanding backwards, until
//the buffer fills all the memory

bool test_allocation_with_reuse()
{
   h.check();
   //We will repeat this test for different sized elements
   for(std::size_t sizeof_object = 1; sizeof_object < 20; ++sizeof_object){
      boost::container::vector<void*> buffers;

      //Allocate buffers with extra memory
      for(std::size_t i = 0; i != NumIt; ++i){
         void *ptr = h.allocate(i*sizeof_object);
         if(!ptr)
            break;
         buffers.push_back(ptr);
      }
      if(!heap_accounting_holds())
         return false;

      //Now deallocate all except the latest
      //Now try to expand to the double of the size
      for(std::size_t i = 0, max = buffers.size() - 1
         ;i < max
         ;++i){
         h.deallocate(buffers[i]);
      }

      //Save the unique buffer and clear vector
      void *ptr = buffers.back();
      buffers.clear();

      //Now allocate with reuse
      std::size_t received_size = 0;
      for(std::size_t i = 0; i != NumIt; ++i){
         std::size_t min_size = (received_size/sizeof_object + 1u)*sizeof_object;
         std::size_t prf_size = (received_size/sizeof_object + (i+1u)*2u)*sizeof_object;
         dlmalloc::command_ret_t ret = h.allocation_command
            ( dlmalloc::expand_bwd, sizeof_object, 1u, min_size
            , prf_size, &received_size, (char*)ptr);
         //If we have memory, this must be a buffer reuse
         if(!ret.first)
            break;
         //If we have memory, this must be a buffer reuse
         if(!ret.second)
            return false;
         if(received_size < min_size)
            return false;
         ptr = ret.first;
      }
      //There should be only a single block so deallocate it
      h.deallocate(ptr);
      h.check();
      if(!h.all_deallocated())
         return false;
   }
   return true;
}


//Every contiguous_elements mode of the two multialloc entry points must work,
//including the ALL_CONTIGUOUS sentinel: it is (size_t)-1, so a range check that
//treats it as a literal element count rejects it outright and the call never
//allocates at all.
bool test_multialloc_contiguous_modes()
{
   h.check();
   const std::size_t counts[]  = { 1u, 2u, 37u, 500u };
   const std::size_t esizes[]  = { 1u, 8u, 24u };
   const std::size_t contig[]  = { dlmalloc::default_contiguous
                                 , dlmalloc::all_contiguous
                                 , 1u, 4u, 16u };

   for(std::size_t c = 0; c != sizeof(contig)/sizeof(contig[0]); ++c){
      for(std::size_t e = 0; e != sizeof(esizes)/sizeof(esizes[0]); ++e){
         for(std::size_t n = 0; n != sizeof(counts)/sizeof(counts[0]); ++n){
            const std::size_t num = counts[n];
            //An explicit count larger than num is a documented error, skip it
            if(contig[c] != dlmalloc::default_contiguous &&
               contig[c] != dlmalloc::all_contiguous &&
               contig[c] > num){
               continue;
            }

            if(!h.all_deallocated())
               return false;

            //---- nodes variant ----
            {
               dlmalloc::multiallocation_chain ch;
               if(!h.multialloc_nodes(num, esizes[e], contig[c], ch))
                  return false;
               if(ch.size() != num)
                  return false;
               //walk it: the link count must agree, and the ends must be right
               std::size_t walked = 0;
               void *last = 0;
               dlmalloc::multiallocation_chain::iterator it = ch.begin();
               while(it != ch.end()){
                  void *a = &*it;
                  if(!a)
                     return false;
                  last = a;
                  if(++walked > num)      //cycle guard
                     return false;
                  ++it;
               }
               if(walked != num)
                  return false;
               if(&*ch.last() != last)
                  return false;
               h.multidealloc(ch);
               if(!h.all_deallocated())
                  return false;
            }

            //---- arrays variant, same modes ----
            {
               boost::container::vector<std::size_t> sizes(num, esizes[e]);
               dlmalloc::multiallocation_chain ch;
               if(!h.multialloc_arrays(num, &sizes[0], 1u, contig[c], ch))
                  return false;
               if(ch.size() != num)
                  return false;
               std::size_t walked = 0;
               dlmalloc::multiallocation_chain::iterator it = ch.begin();
               while(it != ch.end()){
                  if(!it.pointed_node())
                     return false;
                  if(++walked > num)
                     return false;
                  ++it;
               }
               if(walked != num)
                  return false;
               h.multidealloc(ch);
               if(!h.all_deallocated())
                  return false;
            }
         }
      }
   }
   h.check();
   return true;
}


//Expansion that neither side can satisfy on its own but both can together.
//Free space is created before AND after a block, and the block is then asked
//for a size that needs part of each: a combined forward+backward expansion.
bool test_allocation_expand_both()
{
   h.check();
   bool exercised = false;
   //Several backwards_multiple values, powers of two and not, so the
   //lcm/alignment branches that size the backward part all get used
   for(std::size_t sizeof_object = 1; sizeof_object != 9; ++sizeof_object){
      if(!h.all_deallocated())
         return false;

      const std::size_t BlockSize = 256;
      void *a = h.allocate(BlockSize);
      void *b = h.allocate(BlockSize);
      void *c = h.allocate(BlockSize);
      //d pins c: without it, freeing c would merge it into top and the forward
      //side would become effectively unbounded, defeating the test
      void *d = h.allocate(BlockSize);
      if(!a || !b || !c || !d)
         return false;

      //Are the four blocks neighbours? All four were asked for the same
      //size, so their chunks are the same size too, and three equal strides
      //in increasing address order is what "one after another" means. The
      //stride is also too small to hold another block of that size, so
      //nothing can be sitting between them.
      const std::size_t stride_ab = (std::size_t)((char*)b - (char*)a);
      const std::size_t stride_bc = (std::size_t)((char*)c - (char*)b);
      const std::size_t stride_cd = (std::size_t)((char*)d - (char*)c);
      const bool adjacent = stride_ab == stride_bc && stride_bc == stride_cd
                         && stride_ab >= BlockSize && stride_ab < 2u*BlockSize;
      if(!adjacent){   //not the layout this test needs, try the next size
         h.deallocate(a);  h.deallocate(b);
         h.deallocate(c);  h.deallocate(d);
         continue;
      }

      //Being neighbours, the stride from one to the next IS the chunk of the
      //earlier one.
      const std::size_t chunk_a = stride_ab;
      const std::size_t chunk_c = stride_cd;

      std::memset(b, 'B', BlockSize);
      const std::size_t b_user = dlmalloc::usable_size(b);
      h.deallocate(a);          //free space before b
      h.deallocate(c);          //free space after  b

      //More than the forward side alone can give, less than the two together,
      //rounded up to a multiple of sizeof_object as the backward sizing needs
      std::size_t need = b_user + chunk_c + sizeof_object;
      need = ((need + sizeof_object - 1u)/sizeof_object)*sizeof_object;
      if(need <= (b_user + chunk_c) || need > (b_user + chunk_c + chunk_a)){
         h.deallocate(b);  h.deallocate(d);
         continue;
      }

      //Neither direction alone can do it, and a failed attempt has to leave
      //the block exactly as it was
      std::size_t received = 0;
      if(h.allocation_command
            ( dlmalloc::expand_fwd, sizeof_object, 1u, need
            , need, &received, b).first)
         return false;
      if(dlmalloc::usable_size(b) != b_user)
         return false;
      if(h.allocation_command
            ( dlmalloc::expand_bwd, sizeof_object, 1u, need
            , need, &received, b).first)
         return false;
      if(dlmalloc::usable_size(b) != b_user)
         return false;

      //Both together must succeed, and must do it by reusing the block
      received = 0;
      dlmalloc::command_ret_t ret = h.allocation_command
         ( dlmalloc::expand_both, sizeof_object, 1u, need
         , need, &received, b);
      if(!ret.first || !ret.second || received < need)
         return false;

      //Backward expansion moves the block start, so the old contents now sit
      //at an offset inside the returned block. They must be intact: moving
      //them is the caller's job.
      const char *const newmem = (const char *)ret.first;
      if(newmem > (const char *)b)
         return false;
      const std::size_t offset = std::size_t((const char *)b - newmem);
      if((offset + BlockSize) > received)
         return false;
      for(std::size_t i = 0; i != BlockSize; ++i){
         if(newmem[offset + i] != 'B')
            return false;
      }

      h.deallocate(ret.first);
      h.deallocate(d);
      h.check();
      if(!h.all_deallocated())
         return false;
      exercised = true;
   }
   //A heap layout this test could never set up would make it silently vacuous
   return exercised;
}

//Expand both sides. When the previous chunk can be neither split nor merged
//whole the call fails, and then the block must be left untouched and received_size == 0
bool test_allocation_expand_both_failure()
{
   h.check();
   const std::size_t BlockSize      = 256u;
   const std::size_t object_sizes[] = { 12u, 20u, 24u, 40u };
   bool failure_exercised = false;

   for(std::size_t o = 0; o != sizeof(object_sizes)/sizeof(object_sizes[0]); ++o){
      const std::size_t sizeof_object = object_sizes[o];
      for(std::size_t extra = sizeof_object; ; extra += sizeof_object){
         if(!h.all_deallocated())
            return false;
         void *a = h.allocate(BlockSize);
         void *b = h.allocate(BlockSize);
         void *c = h.allocate(BlockSize);
         void *d = h.allocate(BlockSize);   //d pins c, see test_allocation_expand_both
         if(!a || !b || !c || !d)
            return false;
         const std::size_t stride_ab = (std::size_t)((char*)b - (char*)a);
         const std::size_t stride_bc = (std::size_t)((char*)c - (char*)b);
         const std::size_t stride_cd = (std::size_t)((char*)d - (char*)c);
         const bool adjacent = stride_ab == stride_bc && stride_bc == stride_cd
                            && stride_ab >= BlockSize && stride_ab < 2u*BlockSize;
         const std::size_t chunk_a = stride_ab;
         const std::size_t chunk_c = stride_cd;
         const std::size_t b_user  = dlmalloc::usable_size(b);
         std::size_t need = b_user + chunk_c + extra;
         need = ((need + sizeof_object - 1u)/sizeof_object)*sizeof_object;
         if(!adjacent || need > (b_user + chunk_c + chunk_a)){
            h.deallocate(a);  h.deallocate(b);
            h.deallocate(c);  h.deallocate(d);
            break;
         }
         std::memset(b, 'B', BlockSize);
         h.deallocate(a);
         h.deallocate(c);

         std::size_t received = 1u;
         const dlmalloc::command_ret_t ret = h.allocation_command
            ( dlmalloc::expand_both, sizeof_object, 1u, need
            , need, &received, b);
         if(ret.first){
            if(!ret.second || received < need)
               return false;
            h.deallocate(ret.first);
         }
         else{
            failure_exercised = true;
            if(received != 0u || dlmalloc::usable_size(b) != b_user)
               return false;
            for(std::size_t i = 0; i != BlockSize; ++i){
               if(static_cast<char*>(b)[i] != 'B')
                  return false;
            }
            h.deallocate(b);
         }
         h.deallocate(d);
         h.check();
      }
   }
   h.check();
   //A layout that never provokes failure_exercised is a bad test
   return failure_exercised && 0 != h.all_deallocated();
}

//Repeated growth through EXPAND_BOTH over a fragmented heap: broad coverage of
//the combined path, where every reported size must be honoured.
bool test_allocation_expand_both_repeated()
{
   h.check();
   for(std::size_t sizeof_object = 1; sizeof_object < 20; ++sizeof_object){
      if(!h.all_deallocated())
         return false;
      boost::container::vector<void*> buffers;
      for(std::size_t i = 0; i != NumIt; ++i){
         void *ptr = h.allocate(i*sizeof_object);
         if(!ptr)
            break;
         buffers.push_back(ptr);
      }
      //Free every other buffer, so survivors have free space on both sides
      for(std::size_t i = 0; i < buffers.size(); i += 2u){
         h.deallocate(buffers[i]);
         buffers[i] = 0;
      }
      buffers.erase( std::remove(buffers.begin(), buffers.end(), (void*)0)
                   , buffers.end());

      for(std::size_t b = 0; b != buffers.size(); ++b){
         void *ptr = buffers[b];
         std::size_t received_size = dlmalloc::usable_size(ptr);
         for(std::size_t i = 0; i != 4u; ++i){
            const std::size_t min_size =
               ((received_size + sizeof_object)/sizeof_object)*sizeof_object;
            const std::size_t prf_size =
               ((received_size + (i+1u)*8u*sizeof_object)/sizeof_object)*sizeof_object;
            dlmalloc::command_ret_t ret = h.allocation_command
               ( dlmalloc::expand_both, sizeof_object, 1u, min_size
               , prf_size, &received_size, ptr);
            if(!ret.first)
               break;            //no room left on either side, that is fine
            if(!ret.second)       //an expansion must never report a fresh block
               return false;
            if(received_size < min_size)
               return false;
            if(dlmalloc::usable_size(ret.first) != received_size)
               return false;
            ptr = ret.first;
         }
         buffers[b] = ptr;
      }

      for(std::size_t i = 0; i != buffers.size(); ++i)
         h.deallocate(buffers[i]);
      h.check();
      if(!h.all_deallocated())
         return false;
   }
   return true;
}


//This test allocates memory with different alignments
//and checks returned memory is aligned.

bool test_aligned_allocation()
{
   h.check();
   //Allocate aligned buffers in a loop
   //and then deallocate it
   for(std::size_t i = 1u; i != (1u << (sizeof(int)/2u)); i <<= 1u){
      for(std::size_t j = 1u; j != 512u; j <<= 1){
         void *ptr = h.allocate_aligned(j, i-1);
         if(!ptr){
            return false;
         }

         if(((std::size_t)ptr & (j - 1)) != 0)
            return false;
         h.deallocate(ptr);
         //if(!a.all_memory_deallocated() || !a.check_sanity()){
         //   return false;
         //}
      }
   }
   h.check();
   return 0 != h.all_deallocated();//a.all_memory_deallocated() && a.check_sanity();
}

//This test allocates memory with different alignments
//and checks returned memory is aligned.

bool test_continuous_aligned_allocation()
{
   h.check();
   boost::container::vector<void*> buffers;
   //Allocate aligned buffers in a loop
   //and then deallocate it
   bool continue_loop = true;
   std::size_t MaxAlign = 4096;
   std::size_t MaxSize  = 4096;
   for(std::size_t i = 1; i < MaxSize; i <<= 1){
      for(std::size_t j = 1; j < MaxAlign; j <<= 1){
         for(std::size_t k = 0; k != NumIt; ++k){
            void *ptr = h.allocate_aligned(j, i-1);
            buffers.push_back(ptr);
            if(!ptr){
               continue_loop = false;
               break;
            }

            if(((std::size_t)ptr & (j - 1)) != 0)
               return false;
         }
         //Deallocate all
         for(std::size_t k = buffers.size(); k--;){
            h.deallocate(buffers[k]);
         }
         buffers.clear();
         //if(!a.all_memory_deallocated() && a.check_sanity())
         //   return false;
         if(!continue_loop)
            break;
      }
   }
   h.check();
   return 0 != h.all_deallocated();//a.all_memory_deallocated() && a.check_sanity();
}

//This test allocates multiple values until there is no more memory
//and after that deallocates all in the inverse order
bool test_many_equal_allocation()
{
   h.check();
   for( deallocation_type t = DirectDeallocation
      ; t != EndDeallocationType
      ; t = (deallocation_type)((int)t + 1)){
      const std::size_t free_before      = h.mallinfo().fordblks;
      const std::size_t footprint_before = h.footprint();

      boost::container::vector<void*> buffers2;

      //Allocate buffers with extra memory
      for(std::size_t i = 0; i != NumIt; ++i){
         void *ptr = h.allocate(i);
         if(!ptr)
            break;
         //if(!a.check_sanity())
            //return false;
         buffers2.push_back(ptr);
      }
      if(!heap_accounting_holds())
         return false;

      //Now deallocate the half of the blocks
      //so expand maybe can merge new free blocks
      for(std::size_t i = 0, max = buffers2.size()
         ;i < max
         ;++i){
         if(i%2){
            h.deallocate(buffers2[i]);
            buffers2[i] = 0;
         }
      }

      //if(!a.check_sanity())
         //return false;

      boost::container::vector<void*> buffers;
      for(std::size_t i = 0; i != NumIt/10; ++i){
         dlmalloc::multiallocation_chain chain;
         h.multialloc_nodes((i+1)*2, i+1, dlmalloc::default_contiguous, chain);
         dlmalloc::multiallocation_chain::iterator it = chain.begin();
         if(it == chain.end())
            break;

         std::size_t n = 0;
         for(; it != chain.end(); ++n){
            buffers.push_back(&*it);
            ++it;
         }
         if(n != std::size_t((i+1)*2))
            return false;
      }

      //if(!a.check_sanity())
         //return false;

      switch(t){
         case DirectDeallocation:
         {
            for(std::size_t j = 0, max = buffers.size()
               ;j < max
               ;++j){
               h.deallocate(buffers[j]);
            }
         }
         break;
         case InverseDeallocation:
         {
            for(std::size_t j = buffers.size()
               ;j--
               ;){
               h.deallocate(buffers[j]);
            }
         }
         break;
         case MixedDeallocation:
         {
            for(std::size_t j = 0, max = buffers.size()
               ;j < max
               ;++j){
               std::size_t pos = (j%4u)*(buffers.size())/4u;
               h.deallocate(buffers[pos]);
               buffers.erase(buffers.begin()+(std::ptrdiff_t)pos);
            }
         }
         break;
         default:
         break;
      }

      //Deallocate the rest of the blocks

      //Deallocate it in non sequential order
      for(std::size_t j = 0, max = buffers2.size()
         ;j < max
         ;++j){
         std::size_t pos = (j%4u)*(buffers2.size())/4u;
         h.deallocate(buffers2[pos]);
         buffers2.erase(buffers2.begin()+(std::ptrdiff_t)pos);
      }

      if(!h.all_deallocated())
         return false;
      if(!heap_accounting_holds())
         return false;
      if(!free_memory_returned(free_before, footprint_before))
         return false;
      h.check();
   }
   h.check();
   return 0 != h.all_deallocated();
}

//This test allocates multiple values until there is no more memory
//and after that deallocates all in the inverse order

bool test_many_different_allocation()
{
   h.check();
   const std::size_t ArraySize = 11;
   std::size_t requested_sizes[ArraySize];
   for(std::size_t i = 0; i < ArraySize; ++i){
      requested_sizes[i] = 4*i;
   }

   for( deallocation_type t = DirectDeallocation
      ; t != EndDeallocationType
      ; t = (deallocation_type)((int)t + 1)){
      const std::size_t free_before      = h.mallinfo().fordblks;
      const std::size_t footprint_before = h.footprint();

      boost::container::vector<void*> buffers2;

      //Allocate buffers with extra memory
      for(std::size_t i = 0; i != NumIt; ++i){
         void *ptr = h.allocate(i);
         if(!ptr)
            break;
         buffers2.push_back(ptr);
      }
      if(!heap_accounting_holds())
         return false;

      //Now deallocate the half of the blocks
      //so expand maybe can merge new free blocks
      for(std::size_t i = 0, max = buffers2.size()
         ;i < max
         ;++i){
         if(i%2){
            h.deallocate(buffers2[i]);
            buffers2[i] = 0;
         }
      }

      boost::container::vector<void*> buffers;
      for(std::size_t i = 0; i != NumIt; ++i){
         dlmalloc::multiallocation_chain chain;
         h.multialloc_arrays(ArraySize, requested_sizes, 1, dlmalloc::default_contiguous, chain);
         dlmalloc::multiallocation_chain::iterator it = chain.begin();
         if(it == chain.end())
            break;
         std::size_t n = 0;
         for(; it != chain.end(); ++n){
            buffers.push_back(&*it);
            ++it;
         }
         if(n != ArraySize)
            return false;
      }

      switch(t){
         case DirectDeallocation:
         {
            for(std::size_t j = 0, max = buffers.size()
               ;j < max
               ;++j){
               h.deallocate(buffers[j]);
            }
         }
         break;
         case InverseDeallocation:
         {
            for(std::size_t j = buffers.size()
               ;j--
               ;){
               h.deallocate(buffers[j]);
            }
         }
         break;
         case MixedDeallocation:
         {
            for(std::size_t j = 0, max = buffers.size()
               ;j < max
               ;++j){
               std::size_t pos = (j%4)*(buffers.size())/4;
               h.deallocate(buffers[pos]);
               buffers.erase(buffers.begin()+(std::ptrdiff_t)pos);
            }
         }
         break;
         default:
         break;
      }

      //Deallocate the rest of the blocks

      //Deallocate it in non sequential order
      for(std::size_t j = 0, max = buffers2.size()
         ;j < max
         ;++j){
         std::size_t pos = (j%4u)*(buffers2.size())/4u;
         h.deallocate(buffers2[pos]);
         buffers2.erase(buffers2.begin()+(std::ptrdiff_t)pos);
      }

      if(!h.all_deallocated())
         return false;
      if(!heap_accounting_holds())
         return false;
      if(!free_memory_returned(free_before, footprint_before))
         return false;
      h.check();
   }
   h.check();
   return 0 != h.all_deallocated();
}

bool test_many_deallocation()
{
   const std::size_t ArraySize = 11;
   //The chain is movable, not copyable
   boost::container::vector<dlmalloc::multiallocation_chain> buffers;
   std::size_t requested_sizes[ArraySize];
   for(std::size_t i = 0; i < ArraySize; ++i){
      requested_sizes[i] = 4*i;
   }

   for(std::size_t i = 0; i != NumIt; ++i){
      dlmalloc::multiallocation_chain chain;
      h.multialloc_arrays(ArraySize, requested_sizes, 1, dlmalloc::default_contiguous, chain);
      dlmalloc::multiallocation_chain::iterator it = chain.begin();
      if(it == chain.end())
         return false;
      buffers.push_back(boost::move(chain));
   }
   for(std::size_t i = 0; i != NumIt; ++i){
      h.multidealloc(buffers[i]);
      //Every block went back, so the chain is left empty
      if(!buffers[i].empty())
         return false;
   }
   buffers.clear();

   h.check();
   if(!h.all_deallocated())
      return false;

   for(std::size_t i = 0; i != NumIt; ++i){
      dlmalloc::multiallocation_chain chain;
      h.multialloc_nodes(ArraySize, i*4+1, dlmalloc::default_contiguous, chain);
      dlmalloc::multiallocation_chain::iterator it = chain.begin();
      if(it == chain.end())
         return false;
      buffers.push_back(boost::move(chain));
   }
   for(std::size_t i = 0; i != NumIt; ++i){
      h.multidealloc(buffers[i]);
      //Every block went back, so the chain is left empty
      if(!buffers[i].empty())
         return false;
   }
   buffers.clear();

   h.check();
   if(!h.all_deallocated())
      return false;

   return true;
}

//This function calls all tests

//True if the blocks [a, a + a_size) and [b, b + b_size) overlap. The blocks
//can belong to different arrays, so the addresses are compared as integers.
static bool blocks_overlap(const void *a, std::size_t a_size, const void *b, std::size_t b_size)
{
   const std::size_t ua = reinterpret_cast<std::size_t>(a);
   const std::size_t ub = reinterpret_cast<std::size_t>(b);
   return ua < ub + b_size && ub < ua + a_size;
}

//Requests of SIZE_MAX or near it in the expansion functions and in
//multialloc_nodes must fail, or grow the block correctly. Before the fix the
//requested size wrapped to a few bytes: the block was cut to 16 bytes and
//the call reported success.
bool test_huge_expansion_requests()
{
   h.check();
   const std::size_t max_bytes       = std::size_t(-1);
   const std::size_t default_request = 100u;
   const std::size_t small_request   = default_request/2u;
   const std::size_t near_max_bytes  = max_bytes - default_request;
   const std::size_t num_nodes       = 3u;
   const char        fill_value      = 1;

   char *const p = static_cast<char*>(h.allocate(default_request));
   if(!p)
      return false;
   std::memset(p, fill_value, default_request);
   const std::size_t old_usable = dlmalloc::usable_size(p);

   //A minimum that cannot be satisfied fails and leaves the block alone
   std::size_t received = 0;
   if(h.grow(p, max_bytes, max_bytes, &received) || h.grow(p, near_max_bytes, near_max_bytes, &received))
      return false;
   if(h.allocation_command(dlmalloc::expand_fwd, 1, 1, max_bytes, max_bytes, &received, p).first)
      return false;
   if(h.allocation_command(dlmalloc::expand_fwd | dlmalloc::expand_bwd, 1, 1, near_max_bytes, max_bytes, &received, p).first)
      return false;
   if(dlmalloc::usable_size(p) != old_usable)
      return false;

   //A huge maximum is only a preference: the block can grow, but never shrinks
   received = 0;
   if(h.grow(p, old_usable + 1u, max_bytes, &received) && received <= old_usable)
      return false;
   received = 0;
   if(h.allocation_command(dlmalloc::expand_fwd, 1, 1, old_usable + 1u, max_bytes, &received, p).first &&
      received <= old_usable)
      return false;
   if(dlmalloc::usable_size(p) < old_usable)
      return false;

   //The old bytes are kept, and a new block does not overlap the old one
   for(std::size_t i = 0; i != default_request; ++i){
      if(p[i] != fill_value)
         return false;
   }
   void *const q = h.allocate(small_request);
   if(q && blocks_overlap(p, dlmalloc::usable_size(p), q, small_request))
      return false;
   if(q)
      h.deallocate(q);
   h.deallocate(p);

   //multialloc_nodes with a huge element size fails
   {
      dlmalloc::multiallocation_chain ch;
      if(h.multialloc_nodes(num_nodes, near_max_bytes, dlmalloc::default_contiguous, ch))
         return false;
      if(h.multialloc_nodes(num_nodes, near_max_bytes, dlmalloc::all_contiguous, ch))
         return false;
      if(ch.size() != 0)
         return false;
   }
   h.check();
   return 0 != h.all_deallocated();
}

//A block above the mmap threshold, aligned or not, shrinks in place only when
//do_commit is true. The query and the shrink give the same size, the size is
//the usable size of the block after the shrink, and it is at least minbytes.
bool test_mmapped_shrink()
{
   h.check();
   const std::size_t block_size     = 1024u*1024u;
   const std::size_t min_size       = 100u*1024u;
   const std::size_t alignments[]   = { 0u, 4096u, 65536u };
   const char        fill_value     = 2;

   for(std::size_t a = 0; a != sizeof(alignments)/sizeof(alignments[0]); ++a){
      const std::size_t alignment = alignments[a];
      char *const p = static_cast<char*>
         (alignment ? h.allocate_aligned(alignment, block_size) : h.allocate(block_size));
      if(!p)
         return false;
      const std::size_t old_usable = dlmalloc::usable_size(p);
      std::memset(p, fill_value, old_usable);

      //A query does not change the block
      std::size_t try_received = 0;
      const bool try_ok = h.shrink(p, min_size, old_usable, &try_received, false);
      if(dlmalloc::usable_size(p) != old_usable)
         return false;

      std::size_t received = 0;
      const bool ok = h.shrink(p, min_size, old_usable, &received, true);
      if(ok != try_ok)
         return false;
      if(ok){
         if(received != try_received || received < min_size || received > old_usable)
            return false;
         if(dlmalloc::usable_size(p) != received)
            return false;
      }
      else if(dlmalloc::usable_size(p) != old_usable){
         return false;
      }

      //The block keeps its alignment and its first bytes, and all its usable
      //bytes can be written
      if(alignment && (reinterpret_cast<std::size_t>(p) % alignment) != 0u)
         return false;
      const std::size_t new_usable = dlmalloc::usable_size(p);
      for(std::size_t i = 0; i != new_usable; ++i){
         if(p[i] != fill_value)
            return false;
      }
      std::memset(p, 0, new_usable);
      h.check();
      h.deallocate(p);
   }
   h.check();
   return 0 != h.all_deallocated();
}

//expand_bwd fails for a block above the mmap threshold
bool test_mmapped_expand_bwd()
{
   h.check();
   const std::size_t block_size     = 1024u*1024u;
   const std::size_t alignments[]   = { 0u, 4096u, 65536u };
   const std::size_t sizeof_object  = 16u;
   const std::size_t extra          = 64u;
   const char        fill_value     = 3;

   for(std::size_t a = 0; a != sizeof(alignments)/sizeof(alignments[0]); ++a){
      const std::size_t alignment = alignments[a];
      char *const p = static_cast<char*>
         (alignment ? h.allocate_aligned(alignment, block_size) : h.allocate(block_size));
      if(!p)
         return false;
      const std::size_t old_usable = dlmalloc::usable_size(p);
      std::memset(p, fill_value, old_usable);

      std::size_t received = 0;
      const dlmalloc::command_ret_t ret = h.allocation_command
         ( dlmalloc::expand_bwd, sizeof_object, sizeof_object
         , old_usable + extra, old_usable + extra, &received, p);
      if(ret.first)
         return false;
      if(dlmalloc::usable_size(p) != old_usable)
         return false;
      for(std::size_t i = 0; i != old_usable; ++i){
         if(p[i] != fill_value)
            return false;
      }
      h.check();
      h.deallocate(p);
   }
   h.check();
   return 0 != h.all_deallocated();
}

//multialloc_nodes_aligned and multialloc_arrays_aligned return blocks aligned to
//the requested alignment, also for alignments bigger than malloc_alignment
bool test_multialloc_aligned()
{
   h.check();
   const std::size_t alignments[] = { 1u, dlmalloc::malloc_alignment, 32u, 64u, 256u, 4096u };
   const std::size_t contig[]     = { dlmalloc::default_contiguous, dlmalloc::all_contiguous, 4u };
   const std::size_t num          = 37u;
   const std::size_t elem_size    = 24u;
   std::size_t sizes[num];
   for(std::size_t i = 0; i != num; ++i)
      sizes[i] = 1u + (i % 5u);

   for(std::size_t a = 0; a != sizeof(alignments)/sizeof(alignments[0]); ++a){
      const std::size_t alignment = alignments[a];
      for(std::size_t c = 0; c != sizeof(contig)/sizeof(contig[0]); ++c){
         for(std::size_t variant = 0; variant != 2u; ++variant){
            dlmalloc::multiallocation_chain ch;
            //Blocks already in the chain are kept
            void *const first = h.allocate(8u);
            if(!first)
               return false;
            ch.push_back(first);
            const bool ok = variant == 0u
               ? h.multialloc_nodes_aligned(num, elem_size, alignment, contig[c], ch)
               : h.multialloc_arrays_aligned(num, sizes, elem_size, alignment, contig[c], ch);
            if(!ok || ch.size() != num + 1u)
               return false;
            dlmalloc::multiallocation_chain::iterator it = ch.begin();
            if(&*it != first)
               return false;
            ++it;
            for(std::size_t i = 0; i != num; ++i, ++it){
               void *const mem = &*it;
               if(reinterpret_cast<std::size_t>(mem) % alignment)
                  return false;
               const std::size_t bytes = variant == 0u ? elem_size : sizes[i]*elem_size;
               if(dlmalloc::usable_size(mem) < bytes)
                  return false;
            }
            if(it != ch.end())
               return false;
            //Write every block after the chain was read: the links are in the blocks
            void *ptrs[num + 1u];
            std::size_t n = 0;
            for(it = ch.begin(); it != ch.end(); ++it)
               ptrs[n++] = &*it;
            for(std::size_t i = 1; i != n; ++i){
               //Old GCC + glibc fortify sees a constant zero length in the
               //inlined usable_size null path: skip it explicitly
               const std::size_t usable = dlmalloc::usable_size(ptrs[i]);
               if(usable)
                  std::memset(ptrs[i], int(i), usable);
            }
            for(std::size_t i = 0; i != n; ++i)
               h.deallocate(ptrs[i]);
            h.check();
         }
      }
   }
   return 0 != h.all_deallocated();
}

bool test_all_allocation()
{
   std::cout << "Starting test_allocation"
             << std::endl;

   if(!test_allocation()){
      std::cout << "test_allocation_direct_deallocation failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_many_equal_allocation"
             << std::endl;

   if(!test_many_equal_allocation()){
      std::cout << "test_many_equal_allocation failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_many_different_allocation"
             << std::endl;

   if(!test_many_different_allocation()){
      std::cout << "test_many_different_allocation failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_allocation_shrink"
             << std::endl;

   if(!test_allocation_shrink()){
      std::cout << "test_allocation_shrink failed"
                << std::endl;
      return false;
   }

   if(!test_allocation_shrink_and_expand()){
      std::cout << "test_allocation_shrink_and_expand failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_allocation_expand"
             << std::endl;

   if(!test_allocation_expand()){
      std::cout << "test_allocation_expand failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_allocation_deallocation_expand"
             << std::endl;

   if(!test_allocation_deallocation_expand()){
      std::cout << "test_allocation_deallocation_expand failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_allocation_with_reuse"
             << std::endl;

   if(!test_allocation_with_reuse()){
      std::cout << "test_allocation_with_reuse failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_huge_expansion_requests"
             << std::endl;

   if(!test_huge_expansion_requests()){
      std::cout << "test_huge_expansion_requests failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_mmapped_shrink"
             << std::endl;

   if(!test_mmapped_shrink()){
      std::cout << "test_mmapped_shrink failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_mmapped_expand_bwd"
             << std::endl;

   if(!test_mmapped_expand_bwd()){
      std::cout << "test_mmapped_expand_bwd failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_multialloc_aligned"
             << std::endl;

   if(!test_multialloc_aligned()){
      std::cout << "test_multialloc_aligned failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_multialloc_contiguous_modes"
             << std::endl;

   if(!test_multialloc_contiguous_modes()){
      std::cout << "test_multialloc_contiguous_modes failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_allocation_expand_both"
             << std::endl;

   if(!test_allocation_expand_both()){
      std::cout << "test_allocation_expand_both failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_allocation_expand_both_failure"
             << std::endl;

   if(!test_allocation_expand_both_failure()){
      std::cout << "test_allocation_expand_both_failure failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_allocation_expand_both_repeated"
             << std::endl;

   if(!test_allocation_expand_both_repeated()){
      std::cout << "test_allocation_expand_both_repeated failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_aligned_allocation"
             << std::endl;

   if(!test_aligned_allocation()){
      std::cout << "test_aligned_allocation failed"
                << std::endl;
      return false;
   }

   std::cout << "Starting test_continuous_aligned_allocation"
             << std::endl;

   if(!test_continuous_aligned_allocation()){
      std::cout << "test_continuous_aligned_allocation failed"
                << std::endl;
      return false;
   }

   if(!test_many_deallocation()){
      std::cout << "test_many_deallocation failed"
                << std::endl;
      return false;
   }

   return 0 != h.all_deallocated();
}

}}}   //namespace boost { namespace container { namespace test {


int main()
{
   if(!boost::container::test::test_all_allocation())
      return 1;
   return 0;
}
