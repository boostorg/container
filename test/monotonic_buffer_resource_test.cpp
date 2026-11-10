//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/pmr/monotonic_buffer_resource.hpp>
#include <boost/container/pmr/global_resource.hpp>
#include "lightweight_test.hpp"
#include "derived_from_memory_resource.hpp"
#include "memory_resource_logger.hpp"
#include <climits>
#include <cstring>

using namespace boost::container::pmr;

//The alignment passed to the upstream is a power of two not less than the requested alignment
bool is_valid_upstream_alignment(std::size_t upstream_alignment, std::size_t alignment)
{  return upstream_alignment >= alignment && (upstream_alignment & (upstream_alignment - 1u)) == 0u;  }

//Number of bytes needed to align the current position of the resource
std::size_t padding_to_align(const monotonic_buffer_resource &m, std::size_t alignment)
{  return (alignment - std::size_t(m.current_buffer()) % alignment) % alignment;  }

void test_resource_constructor()
{
   //First constructor, null resource
   {
      memory_resource_logger mrl;
      BOOST_TEST(mrl.m_info.size() == 0u);
      set_default_resource(&mrl);
      monotonic_buffer_resource m;
      //test postconditions
      BOOST_TEST(m.upstream_resource() == get_default_resource());
      //test it does not allocate any memory
      BOOST_TEST(mrl.m_info.size() == 0u);
      set_default_resource(0);
   }
   //First constructor, non-null resource
   {
      derived_from_memory_resource dmr;
      dmr.reset();
      monotonic_buffer_resource m(&dmr);
      //test postconditions
      BOOST_TEST(m.upstream_resource() == &dmr);
      BOOST_TEST(m.next_buffer_size() == monotonic_buffer_resource::initial_next_buffer_size);
      BOOST_TEST(m.current_buffer() == 0);
      //test it does not allocate any memory
      BOOST_TEST(dmr.do_allocate_called == false);
   }
}

void test_initial_size_constructor()
{
   //Second constructor, null resource
   const std::size_t initial_size = monotonic_buffer_resource::initial_next_buffer_size*2;
   {
      memory_resource_logger mrl;
      BOOST_TEST(mrl.m_info.size() == 0u);
      set_default_resource(&mrl);
      monotonic_buffer_resource m(initial_size);
      //test postconditions
      BOOST_TEST(m.upstream_resource() == get_default_resource());
      BOOST_TEST(m.next_buffer_size() >= initial_size);
      BOOST_TEST(m.current_buffer() == 0);
      //test it does not allocate any memory
      BOOST_TEST(mrl.m_info.size() == 0u);
      set_default_resource(0);
   }
   //Second constructor, non-null resource
   {
      derived_from_memory_resource dmr;
      dmr.reset();
      monotonic_buffer_resource m(initial_size, &dmr);
      //test postconditions
      BOOST_TEST(m.upstream_resource() == &dmr);
      BOOST_TEST(m.next_buffer_size() >= initial_size);
      BOOST_TEST(m.current_buffer() == 0);
      //test it does not allocate any memory
      BOOST_TEST(dmr.do_allocate_called == false);
   }
}

void test_buffer_constructor()
{
   const std::size_t BufSz = monotonic_buffer_resource::initial_next_buffer_size*2;
   unsigned char buf[BufSz];
   //Third constructor, null resource
   {
      memory_resource_logger mrl;
      BOOST_TEST(mrl.m_info.size() == 0u);
      set_default_resource(&mrl);
      monotonic_buffer_resource m(buf, BufSz);
      //test postconditions
      BOOST_TEST(m.upstream_resource() == get_default_resource());
      BOOST_TEST(m.next_buffer_size() > BufSz);
      BOOST_TEST(m.current_buffer() == buf);
      //test it does not allocate any memory
      BOOST_TEST(mrl.m_info.size() == 0u);
      set_default_resource(0);
   }
   //Third constructor, non-null resource
   {
      derived_from_memory_resource dmr;
      dmr.reset();
      monotonic_buffer_resource m(buf, sizeof(buf), &dmr);
      //test postconditions
      BOOST_TEST(m.upstream_resource() == &dmr);
      BOOST_TEST(m.next_buffer_size() > sizeof(buf));
      BOOST_TEST(m.current_buffer() == buf);
      //test it does not allocate any memory
      BOOST_TEST(dmr.do_allocate_called == false);
   }
   //Check for empty buffers
   {
      monotonic_buffer_resource m(buf, 0);
      BOOST_TEST(m.upstream_resource() == get_default_resource());
      BOOST_TEST(m.next_buffer_size() > 1);
      BOOST_TEST(m.current_buffer() == buf);
   }
}

struct derived_from_monotonic_buffer_resource
   : public monotonic_buffer_resource
{
   explicit derived_from_monotonic_buffer_resource(memory_resource *p)
      : monotonic_buffer_resource(p)
   {}

   explicit derived_from_monotonic_buffer_resource(std::size_t initial_size, memory_resource* upstream)
      : monotonic_buffer_resource(initial_size, upstream)
   {}

   explicit derived_from_monotonic_buffer_resource(void* buffer, std::size_t buffer_size, memory_resource* upstream)
      : monotonic_buffer_resource(buffer, buffer_size, upstream)
   {}

   using monotonic_buffer_resource::do_allocate;
   using monotonic_buffer_resource::do_deallocate;
   using monotonic_buffer_resource::do_is_equal;
};

void test_upstream_resource()
{
   //Allocate buffer first to avoid stack-use-after-scope in monotonic_buffer_resource's destructor
   const std::size_t BufSz = monotonic_buffer_resource::initial_next_buffer_size;
   boost::move_detail::aligned_storage<BufSz*4u>::type buf;
   //Test stores the resource and uses it to allocate memory
   derived_from_memory_resource dmr;
   dmr.reset();
   derived_from_monotonic_buffer_resource dmbr(&dmr);
   //Resource must be stored and initial values given (no current buffer)
   BOOST_TEST(dmbr.upstream_resource() == &dmr);
   BOOST_TEST(dmbr.next_buffer_size() == monotonic_buffer_resource::initial_next_buffer_size);
   BOOST_TEST(dmbr.current_buffer() == 0);
   //Test it does not allocate any memory
   BOOST_TEST(dmr.do_allocate_called == false);
   //Now stub buffer storage it as the return buffer
   //for "derived_from_memory_resource":
   dmr.do_allocate_return = &buf;
   //Test that allocation uses the upstream_resource()
   void *addr = dmbr.do_allocate(1u, 1u);
   //Test returns stubbed memory and requests at least next_buffer_size() bytes
   BOOST_TEST((char*)addr >= (char*)&buf);
   BOOST_TEST((char*)addr + 1u <= (char*)(&buf+1));
   BOOST_TEST(dmr.do_allocate_called == true);
   BOOST_TEST(dmr.do_allocate_bytes >= BufSz);
   BOOST_TEST(dmr.do_allocate_bytes <= sizeof(buf));
   BOOST_TEST(is_valid_upstream_alignment(dmr.do_allocate_alignment, 1u));
}

void test_do_allocate()
{
   memory_resource_logger mrl;
   {
      std::size_t remaining_storage = 0u;
      derived_from_monotonic_buffer_resource dmbr(&mrl);
      //First test, no buffer
      {
         char *const p = (char*)dmbr.do_allocate(1, 1);
         //It should allocate initial size
         BOOST_TEST(mrl.m_info.size() == 1u);
         //... which requests at least the initial size to the allocator
         BOOST_TEST(mrl.m_info[0].bytes >= monotonic_buffer_resource::initial_next_buffer_size);
         std::size_t remaining = dmbr.remaining_storage(1u);
         //The allocated byte and the remaining storage are inside the upstream buffer
         BOOST_TEST(remaining > 0u);
         BOOST_TEST(p >= mrl.m_info[0].address);
         BOOST_TEST(p + 1u + remaining <= mrl.m_info[0].address + mrl.m_info[0].bytes);
         remaining_storage = remaining;
      }
      //Now ask for more internal storage with misaligned current buffer
      {
         //Test wasted space
         std::size_t wasted_due_to_alignment;
         dmbr.remaining_storage(4u, wasted_due_to_alignment);
         BOOST_TEST(wasted_due_to_alignment == padding_to_align(dmbr, 4u));
         dmbr.do_allocate(4, 4);
         //It should not have allocated
         BOOST_TEST(mrl.m_info.size() == 1u);
         std::size_t remaining = dmbr.remaining_storage(1u);
         //We wasted some bytes due to alignment plus 4 bytes of real storage
         BOOST_TEST(remaining == remaining_storage - 4 - wasted_due_to_alignment);
         remaining_storage = remaining;
      }
      //Now request the same alignment to test no storage is wasted
      {
         std::size_t wasted_due_to_alignment;
         std::size_t remaining = dmbr.remaining_storage(1u, wasted_due_to_alignment);
         BOOST_TEST(mrl.m_info.size() == 1u);
         dmbr.do_allocate(4, 4);
         //It should not have allocated
         BOOST_TEST(mrl.m_info.size() == 1u);
         remaining = dmbr.remaining_storage(1u);
         //We wasted no bytes due to alignment plus 4 bytes of real storage
         BOOST_TEST(remaining == remaining_storage - 4u);
         remaining_storage = remaining;
      }
      //Now exhaust the remaining storage with 2 byte alignment (the last allocation
      //was 4 bytes with 4 byte alignment) so it should be already 2-byte aligned.
      {
         dmbr.do_allocate(remaining_storage, 2);
         std::size_t wasted_due_to_alignment;
         std::size_t remaining = dmbr.remaining_storage(1u, wasted_due_to_alignment);
         BOOST_TEST(wasted_due_to_alignment == 0u);
         BOOST_TEST(remaining == 0u);
         //It should not have allocated
         BOOST_TEST(mrl.m_info.size() == 1u);
         remaining_storage = 0u;
      }
      //The next allocation should trigger the upstream resource, even with a 1 byte
      //allocation.
      {
         char *const p = (char*)dmbr.do_allocate(1u, 1u);
         BOOST_TEST(mrl.m_info.size() == 2u);
         //The next allocation should be geometrically bigger.
         BOOST_TEST(mrl.m_info[1].bytes > mrl.m_info[0].bytes);
         std::size_t wasted_due_to_alignment;
         //For a 2 byte alignment the padding depends on the position of the previous
         //1 byte allocation
         std::size_t remaining = dmbr.remaining_storage(2u, wasted_due_to_alignment);
         BOOST_TEST(wasted_due_to_alignment == padding_to_align(dmbr, 2u));
         BOOST_TEST(p >= mrl.m_info[1].address);
         BOOST_TEST(p + 1u + wasted_due_to_alignment + remaining <= mrl.m_info[1].address + mrl.m_info[1].bytes);
         //It should not have allocated
         remaining_storage = dmbr.remaining_storage(1u);
      }
      //Now try a bigger than next allocation: the upstream buffer holds the request and
      //next_buffer_size grows.
      {
         std::size_t next_alloc = 5*monotonic_buffer_resource::initial_next_buffer_size;
         const std::size_t next = dmbr.next_buffer_size();
         BOOST_TEST(next_alloc > next);
         char *const p = (char*)dmbr.do_allocate(next_alloc, 1u);
         BOOST_TEST(mrl.m_info.size() == 3u);
         BOOST_TEST(mrl.m_info[2].bytes >= next_alloc);
         BOOST_TEST(p >= mrl.m_info[2].address);
         BOOST_TEST(p + next_alloc + dmbr.remaining_storage(1u) <= mrl.m_info[2].address + mrl.m_info[2].bytes);
         BOOST_TEST(dmbr.next_buffer_size() > next);
         remaining_storage = dmbr.remaining_storage(1u);
      }
   }
   //derived_from_monotonic_buffer_resource dmbr(&mrl) is destroyed
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);

   //Now use a local buffer
   {
      boost::move_detail::aligned_storage
         <monotonic_buffer_resource::initial_next_buffer_size>::type buf;
      //Supply an external buffer
      derived_from_monotonic_buffer_resource dmbr(&buf, sizeof(buf), &mrl);
      BOOST_TEST(dmbr.remaining_storage(1u) == sizeof(buf));
      //Allocate all remaining storage
      dmbr.do_allocate(dmbr.remaining_storage(1u), 1u);
      //No new allocation should have occurred
      BOOST_TEST(mrl.m_info.size() == 0u);
      BOOST_TEST(dmbr.remaining_storage(1u) == 0u);
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);
}

void test_do_allocate_zero()
{
   memory_resource_logger mrl;

   //(1) Fresh resource without a buffer: a zero-sized allocation returns valid address
   {
      monotonic_buffer_resource m(&mrl);
      BOOST_TEST(m.current_buffer() == 0);
      void *const p = m.allocate(0u, 1u);
      BOOST_TEST(p != 0);
      //There was no buffer, so one upstream allocation happened
      BOOST_TEST(mrl.m_info.size() == 1u);
      //remaining_storage() must not underflow
      BOOST_TEST(m.remaining_storage(1u) < m.next_buffer_size());

      //A new zero sized allocation would return the same address, but it should not trigger a new allocation
      void *const p2 = m.allocate(0u, 1u);
      BOOST_TEST(p == p2);
      BOOST_TEST(mrl.m_info.size() == 1u);
      //remaining_storage() must not underflow
      BOOST_TEST(m.remaining_storage(1u) < m.next_buffer_size());
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);

   //(2) External buffer scenarios
   {
      boost::move_detail::aligned_storage<8u, 8u>::type storage;
      const std::size_t buffer_size = 7u;
      monotonic_buffer_resource m(&storage, buffer_size, &mrl);
      char *const base = (char*)&storage;
      BOOST_TEST(m.remaining_storage(1u) == buffer_size);

      //A zero-sized allocation on a fresh, aligned buffer returns the current
      //position without consuming storage; repeats return the same address.
      void *const z0 = m.allocate(0u, 1u);
      BOOST_TEST(z0 == base);
      BOOST_TEST(m.remaining_storage(1u) == buffer_size);   //nothing consumed
      void *const z0b = m.allocate(0u, 1u);
      BOOST_TEST(z0b == z0);                                 //same address

      //Exhaust the buffer
      void *const full = m.allocate(buffer_size, 1u);
      BOOST_TEST(full == base);
      BOOST_TEST(m.remaining_storage(1u) == 0u);

      //Zero-sized allocation on the exhausted (still 1-aligned) buffer returns
      //the one-past-the-end position (non-null) without going upstream.
      void *const zend = m.allocate(0u, 1u);
      BOOST_TEST(zend == base + buffer_size);
      BOOST_TEST(m.remaining_storage(1u) < m.next_buffer_size());
      BOOST_TEST(mrl.m_info.size() == 0u);                   //no upstream alloc

      //Zero-sized allocation that needs alignment padding which does not fit in
      //the exhausted buffer uses a new upstream buffer (no underflow).
      void *const z4 = m.allocate(0u, 4u);
      BOOST_TEST(z4 != 0);
      BOOST_TEST(m.remaining_storage(1u) < m.next_buffer_size());
      BOOST_TEST(mrl.m_info.size() == 1u);
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);

   //(3) Over-aligned zero-sized allocation from within a buffer returns an
   //    aligned position without consuming bytes, and repeats are stable.
   {
      boost::move_detail::aligned_storage<32u, 8u>::type storage;
      monotonic_buffer_resource m(&storage, 16u, &mrl);
      char *const base = (char*)&storage;
      //Misalign the current position by consuming 1 byte
      void *const one = m.allocate(1u, 1u);
      BOOST_TEST(one == base);
      //Now current is base+1; a zero-sized 4-aligned request returns base+4
      void *const z4 = m.allocate(0u, 4u);
      BOOST_TEST(z4 == base + 4u);
      BOOST_TEST((std::size_t(z4) % 4u) == 0u);
      //Repeats return the same aligned address (no storage consumed)
      void *const z4b = m.allocate(0u, 4u);
      BOOST_TEST(z4b == z4);
      //No upstream allocation for any of these
      BOOST_TEST(mrl.m_info.size() == 0u);
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);

   //(4) A zero-sized allocation whose aligned position lands EXACTLY on the
   //    one-past-the-end position of the current buffer must return that
   //    (correctly aligned) past-the-end address, without allocating a new
   //    buffer from upstream and without underflowing remaining_storage().
   {
      //8-aligned buffer of 4 bytes: end == base+4 is 4-aligned.
      boost::move_detail::aligned_storage<8u, 8u>::type storage;
      const std::size_t buffer_size = 4u;
      monotonic_buffer_resource m(&storage, buffer_size, &mrl);
      char *const base = (char*)&storage;
      char *const buffer_end = base + buffer_size;

      //Consume 2 bytes so the current position (base+2) is not 4-aligned and
      //the first 4-aligned address at/after it is exactly the past-the-end
      //position base+4.
      void *const two = m.allocate(2u, 1u);
      BOOST_TEST(two == base);
      BOOST_TEST(m.remaining_storage(1u) == 2u);

      //The 4-aligned zero-sized allocation must return the past-the-end address
      void *const zend = m.allocate(0u, 4u);
      BOOST_TEST(zend == buffer_end);
      BOOST_TEST((std::size_t(zend) % 4u) == 0u);
      //It consumed only the 2 bytes of padding; nothing remains, no underflow
      BOOST_TEST(m.remaining_storage(1u) == 0u);
      //No upstream buffer was requested
      BOOST_TEST(mrl.m_info.size() == 0u);

      void *const zend2 = m.allocate(0u, 4u);
      BOOST_TEST(zend2 == buffer_end);
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);

   //(5) A zero-sized allocation whose alignment requirement would push the
   //    aligned position STRICTLY beyond the one-past-the-end position of the
   //    current buffer (even though the buffer still has unused bytes) must
   //    allocate a new buffer
   {
      //8-aligned buffer of 6 bytes: end == base+6 is not 8-aligned.
      boost::move_detail::aligned_storage<8u, 8u>::type storage;
      const std::size_t buffer_size = 6u;
      monotonic_buffer_resource m(&storage, buffer_size, &mrl);
      char *const base = (char*)&storage;

      //Consume 3 bytes so the current position is base+3 with 3 bytes left.
      //The first 8-aligned address at/after base+3 is base+8, which is 2 bytes
      //beyond the one-past-the-end position base+6.
      void *const three = m.allocate(3u, 1u);
      BOOST_TEST(three == base);
      BOOST_TEST(m.remaining_storage(1u) == 3u);

      //The 8-aligned zero-sized allocation cannot be placed, so it should trigger an allocation.
      void *const zbeyond = m.allocate(0u, 8u);
      BOOST_TEST(zbeyond != 0);
      BOOST_TEST((std::size_t(zbeyond) % 8u) == 0u);
      //One upstream buffer was requested.
      BOOST_TEST(mrl.m_info.size() == 1u);
   }

   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);
}

void test_do_allocate_overaligned()
{
   //Over-aligned requests (alignment > memory_resource::max_align) must be
   //honored. These tests verify the returned pointer honors the requested
   //alignment in every code path.
   const std::size_t max_align = memory_resource::max_align;
   memory_resource_logger mrl;

   //(1) Over-aligned allocation from a fresh resource must trigger an upstream
   //    buffer and return a correctly over-aligned, fully usable block.
   {
      const std::size_t over  = max_align*4u;
      const std::size_t bytes = over*3u + 1u;
      monotonic_buffer_resource m(&mrl);
      void *const p = m.allocate(bytes, over);
      BOOST_TEST(p != 0);
      BOOST_TEST((std::size_t(p) % over) == 0u);
      //A single upstream buffer was requested for it
      BOOST_TEST(mrl.m_info.size() == 1u);
      //The whole block must be usable: writing every byte would trip ASAN or
      //corrupt the heap if the buffer were too small after realignment.
      char *const cp = (char*)p;
      for(std::size_t i = 0; i != bytes; ++i){
         cp[i] = (char)0xABu;
      }
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);

   //(2) A sequence of increasing over-alignments must each be honored.
   {
      monotonic_buffer_resource m(&mrl);
      for(std::size_t a = max_align*2u; a <= max_align*32u; a *= 2u){
         void *const p = m.allocate(a/2u, a);
         BOOST_TEST(p != 0);
         BOOST_TEST((std::size_t(p) % a) == 0u);
      }
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);

   //(3) Over-aligned allocation served from a user-supplied buffer (realigned in
   //    place, no upstream) and then from a fresh buffer after exhaustion.
   {
      //256-byte, 64-aligned external buffer
      boost::move_detail::aligned_storage<256u, 64u>::type storage;
      const std::size_t over = 64u;
      monotonic_buffer_resource m(&storage, sizeof(storage), &mrl);
      char *const base = (char*)&storage;
      //Misalign the current position by 1 byte
      void *const one = m.allocate(1u, 1u);
      BOOST_TEST(one == base);
      //Over-aligned request must be realigned within the same buffer (no upstream)
      void *const p = m.allocate(over, over);
      BOOST_TEST((std::size_t(p) % over) == 0u);
      BOOST_TEST(p == base + over);
      BOOST_TEST(mrl.m_info.size() == 0u);
      //Exhaust the remaining storage
      void *const ex = m.allocate(m.remaining_storage(1u), 1u);
      (void)ex;
      BOOST_TEST(m.remaining_storage(1u) == 0u);
      //Over-aligned request now must go upstream and still be correctly aligned
      void *const q = m.allocate(over, over);
      BOOST_TEST(q != 0);
      BOOST_TEST((std::size_t(q) % over) == 0u);
      BOOST_TEST(mrl.m_info.size() == 1u);
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);

   //(4) A zero-sized over-aligned request is also honored (returns an aligned
   //    address) following the same rules as any other zero-sized allocation.
   {
      boost::move_detail::aligned_storage<128u, 64u>::type storage;
      const std::size_t over = 64u;
      monotonic_buffer_resource m(&storage, sizeof(storage), &mrl);
      char *const base = (char*)&storage;
      //Misalign, then a zero-sized over-aligned request returns the next aligned
      //position inside the buffer without consuming real storage nor upstream.
      void *const one = m.allocate(1u, 1u);
      BOOST_TEST(one == base);
      void *const z = m.allocate(0u, over);
      BOOST_TEST(z == base + over);
      BOOST_TEST((std::size_t(z) % over) == 0u);
      BOOST_TEST(mrl.m_info.size() == 0u);
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);
}

//A request that needs a new buffer passes at least its alignment to the upstream
//resource, and the returned block is inside the upstream allocation.
void test_do_allocate_overaligned_upstream_alignment()
{
   memory_resource_logger mrl;
   {
      monotonic_buffer_resource m(&mrl);
      std::size_t n = 0u;
      for(std::size_t a = 1u; a <= 4096u; a *= 2u){
         //Exhaust the current buffer, so that the request goes upstream
         if(m.remaining_storage(1u))
            (void)m.allocate(m.remaining_storage(1u), 1u);
         char *const p = static_cast<char*>(m.allocate(1024u, a));
         ++n;
         BOOST_TEST(mrl.m_info.size() == n);
         const memory_resource_logger::allocation_info &info = mrl.m_info[n-1u];
         BOOST_TEST(is_valid_upstream_alignment(info.alignment, a));
         BOOST_TEST(p >= info.address);
         BOOST_TEST(p + 1024u <= info.address + info.bytes);
         BOOST_TEST((std::size_t(p) % a) == 0u);
      }
   }
   //All buffers are returned with the same size and alignment
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);
}

//next_buffer_size() is the minimum size of the next upstream buffer, and each new buffer
//is bigger than the previous one
void test_buffer_size_and_growth()
{
   const std::size_t initial = monotonic_buffer_resource::initial_next_buffer_size;
   //initial_size is the minimum size of the first upstream buffer
   {
      //initial_size shall be greater than zero
      const std::size_t sizes[] = { 1u, 7u, initial, initial*3u + 1u };
      for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i){
         memory_resource_logger mrl;
         {
            monotonic_buffer_resource m(sizes[i], &mrl);
            const std::size_t next = m.next_buffer_size();
            BOOST_TEST(next >= sizes[i]);
            BOOST_TEST(next > 0u);
            (void)m.allocate(1u, 1u);
            BOOST_TEST(mrl.m_info.size() == 1u);
            BOOST_TEST(mrl.m_info[0].bytes >= next);
         }
         BOOST_TEST(mrl.m_mismatches == 0u);
         BOOST_TEST(mrl.m_info.size() == 0u);
      }
   }
   //A user buffer makes next_buffer_size bigger than the buffer size, and release()
   //restores that value
   {
      const std::size_t bufsize = initial*3u + 1u;
      boost::move_detail::aligned_storage<monotonic_buffer_resource::initial_next_buffer_size*4u>::type buf;
      monotonic_buffer_resource m(&buf, bufsize);
      const std::size_t next = m.next_buffer_size();
      BOOST_TEST(next > bufsize);
      (void)m.allocate(bufsize, 1u);
      (void)m.allocate(1u, 1u);
      m.release();
      BOOST_TEST(m.next_buffer_size() == next);
   }
   //Each upstream buffer is not smaller than next_buffer_size() and is bigger than
   //the previous one
   {
      memory_resource_logger mrl;
      {
         monotonic_buffer_resource m(&mrl);
         for(std::size_t i = 0; i != 4u; ++i){
            const std::size_t next = m.next_buffer_size();
            //Exhaust the current buffer, so that the next allocation goes upstream
            if(m.remaining_storage(1u))
               (void)m.allocate(m.remaining_storage(1u), 1u);
            (void)m.allocate(1u, 1u);
            BOOST_TEST(mrl.m_info.size() == i + 1u);
            BOOST_TEST(mrl.m_info[i].bytes >= next);
            BOOST_TEST(i == 0u || mrl.m_info[i].bytes > mrl.m_info[i-1u].bytes);
         }
      }
      BOOST_TEST(mrl.m_mismatches == 0u);
      BOOST_TEST(mrl.m_info.size() == 0u);
   }
}

//Upstream resource that throws bad_alloc for requests bigger than max_bytes
class limited_memory_resource
   : public memory_resource_logger
{
   public:
   explicit limited_memory_resource(std::size_t max)
      : max_bytes(max)
   {}

   virtual void* do_allocate(std::size_t bytes, std::size_t alignment) BOOST_OVERRIDE
   {
      if(bytes > max_bytes)
         boost::container::throw_bad_alloc();
      return this->memory_resource_logger::do_allocate(bytes, alignment);
   }

   std::size_t max_bytes;
};

void test_do_allocate_overflow_and_upstream_failure()
{
   #ifndef BOOST_NO_EXCEPTIONS
   limited_memory_resource lmr(1024u*1024u);
   {
      derived_from_monotonic_buffer_resource dmbr(&lmr);
      const std::size_t next_size = dmbr.next_buffer_size();
      //The size plus the realignment space overflows
      BOOST_TEST_THROWS(dmbr.do_allocate(std::size_t(-1) - 10u, memory_resource::max_align*4u), boost::container::bad_alloc_t);
      BOOST_TEST_THROWS(dmbr.do_allocate(std::size_t(-1), memory_resource::max_align*2u), boost::container::bad_alloc_t);
      BOOST_TEST(lmr.m_info.empty());
      BOOST_TEST(dmbr.next_buffer_size() == next_size);
      //The upstream resource throws: no effects
      BOOST_TEST_THROWS(dmbr.do_allocate(std::size_t(1) << 30u, 1u), boost::container::bad_alloc_t);
      BOOST_TEST(lmr.m_info.empty());
      BOOST_TEST(dmbr.next_buffer_size() == next_size);
      BOOST_TEST(dmbr.current_buffer() == 0);
      //Later small allocations use the normal buffer size
      void *const p = dmbr.do_allocate(1u, 1u);
      BOOST_TEST(p != 0);
      BOOST_TEST(lmr.m_info.size() == 1u);
      BOOST_TEST(lmr.m_info[0].bytes >= next_size);
   }
   BOOST_TEST(lmr.m_mismatches == 0u);
   BOOST_TEST(lmr.m_info.empty());
   #endif
}

void test_do_deallocate()
{
   memory_resource_logger mrl;
   const std::size_t initial_size = 1u;
   {
      derived_from_monotonic_buffer_resource dmbr(initial_size, &mrl);
      //First test, no buffer
      const unsigned iterations = 8;
      char *bufs[iterations];
      std::size_t sizes[iterations];
      //Test each iteration allocates memory
      for(unsigned i = 0; i != iterations; ++i)
      {
         sizes[i] = dmbr.remaining_storage()+1;
         bufs[i]  = (char*)dmbr.do_allocate(sizes[i], 1);
         BOOST_TEST(mrl.m_info.size() == (i+1));
      }
      std::size_t remaining = dmbr.remaining_storage();
      //Test do_deallocate does not release any storage
      for(unsigned i = 0; i != iterations; ++i)
      {
         dmbr.do_deallocate(bufs[i], sizes[i], 1u);
         BOOST_TEST(mrl.m_info.size() == iterations);
         BOOST_TEST(remaining == dmbr.remaining_storage());
         BOOST_TEST(mrl.m_mismatches == 0u);
      }
   }
}

void test_do_is_equal()
{
   //! <b>Returns</b>:
   //!   `this == dynamic_cast<const monotonic_buffer_resource*>(&other)`.
   memory_resource_logger mrl;
   derived_from_monotonic_buffer_resource dmbr(&mrl);
   derived_from_monotonic_buffer_resource dmbr2(&mrl);
   BOOST_TEST(true == dmbr.do_is_equal(dmbr));
   BOOST_TEST(false == dmbr.do_is_equal(dmbr2));
   //A different type should be always different
   derived_from_memory_resource dmr;
   BOOST_TEST(false == dmbr.do_is_equal(dmr));
}

void test_release()
{
   {
      memory_resource_logger mrl;
      const std::size_t initial_size = 1u;
      derived_from_monotonic_buffer_resource dmbr(initial_size, &mrl);
      //First test, no buffer
      const unsigned iterations = 8;
      //Test each iteration allocates memory
      for(unsigned i = 0; i != iterations; ++i)
      {
         dmbr.do_allocate(dmbr.remaining_storage()+1, 1);
         BOOST_TEST(mrl.m_info.size() == (i+1));
      }
      //Release and check memory was released
      dmbr.release();
      BOOST_TEST(mrl.m_mismatches == 0u);
      BOOST_TEST(mrl.m_info.size() == 0u);
   }
   //Now use a local buffer
   {
      boost::move_detail::aligned_storage
         <monotonic_buffer_resource::initial_next_buffer_size>::type buf;
      //Supply an external buffer
      monotonic_buffer_resource monr(&buf, sizeof(buf));
      memory_resource &mr = monr;
      BOOST_TEST(monr.remaining_storage(1u) == sizeof(buf));
      //Allocate all remaining storage
      const std::size_t sz = monr.remaining_storage(1u);
      void *const p = mr.allocate(sz, 1u);
      BOOST_TEST(monr.current_buffer() == ((char*)&buf + sizeof(buf)));
      mr.deallocate(p, sz, 1u);
      //No new allocation should have occurred
      BOOST_TEST(monr.remaining_storage(1u) == 0u);
      //Release and check memory was released and the original buffer is back
      monr.release();
      BOOST_TEST(monr.remaining_storage(1u) == sizeof(buf));
      BOOST_TEST(monr.current_buffer() == &buf);
   }
}

void test_destructor()
{
   memory_resource_logger mrl;
   const std::size_t initial_size = 1u;
   {
      derived_from_monotonic_buffer_resource dmbr(initial_size, &mrl);
      //First test, no buffer
      const unsigned iterations = 8;
      //Test each iteration allocates memory
      for(unsigned i = 0; i != iterations; ++i)
      {
         dmbr.do_allocate(dmbr.remaining_storage()+1, 1);
         BOOST_TEST(mrl.m_info.size() == (i+1));
      }
   }  //dmbr is destroyed, memory should be released
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);
}

//release() restores the next buffer size set by the constructor
void test_release_restores_next_buffer_size()
{
   memory_resource_logger mrl;
   {
      monotonic_buffer_resource m(&mrl);
      const std::size_t initial = m.next_buffer_size();
      (void)m.allocate(1u);
      (void)m.allocate(m.remaining_storage() + 1u);
      BOOST_TEST(m.next_buffer_size() != initial);
      m.release();
      BOOST_TEST(m.next_buffer_size() == initial);
   }
   {
      monotonic_buffer_resource m(4096u, &mrl);
      const std::size_t initial = m.next_buffer_size();
      BOOST_TEST(initial >= 4096u);
      (void)m.allocate(1u);
      (void)m.allocate(m.remaining_storage() + 1u);
      m.release();
      BOOST_TEST(m.next_buffer_size() == initial);
   }
   {
      static const std::size_t buf_size = 1000u;
      boost::move_detail::aligned_storage<buf_size>::type buf;
      monotonic_buffer_resource m(&buf, buf_size, &mrl);
      const std::size_t initial = m.next_buffer_size();
      (void)m.allocate(buf_size);
      (void)m.allocate(1u);
      m.release();
      BOOST_TEST(m.next_buffer_size() == initial);
      BOOST_TEST(m.current_buffer() == &buf);
   }
   BOOST_TEST(mrl.m_info.size() == 0u);
}

//Upstream resource that returns the same static buffer for any alignment, so that any
//alignment can be requested (the returned memory is not used). It checks that each
//deallocation uses the address, size and alignment of the allocation.
class static_buffer_resource
   : public memory_resource
{
   public:
   static_buffer_resource()
      : m_allocated(false), m_bytes(), m_alignment(), m_mismatches()
   {}

   virtual void* do_allocate(std::size_t bytes, std::size_t alignment) BOOST_OVERRIDE
   {
      BOOST_TEST(!m_allocated);
      BOOST_TEST(bytes <= sizeof(m_buf));
      m_allocated = true;
      m_bytes = bytes;
      m_alignment = alignment;
      return &m_buf;
   }

   virtual void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) BOOST_OVERRIDE
   {
      m_mismatches += !m_allocated || p != &m_buf || bytes != m_bytes || alignment != m_alignment;
      m_allocated = false;
   }

   virtual bool do_is_equal(const memory_resource& other) const BOOST_NOEXCEPT BOOST_OVERRIDE
   {  return this == &other;  }

   boost::move_detail::aligned_storage<monotonic_buffer_resource::initial_next_buffer_size*4u>::type m_buf;
   bool m_allocated;
   std::size_t m_bytes;
   std::size_t m_alignment;
   unsigned m_mismatches;
};

//Every alignment (up to the highest power of two of std::size_t) is passed to the
//upstream resource, and the buffer is deallocated with the same size and alignment
void test_do_allocate_any_alignment()
{
   static_buffer_resource sbr;
   const std::size_t bits = sizeof(std::size_t)*CHAR_BIT;
   const std::size_t sizes[] = { 0u, 1u, monotonic_buffer_resource::initial_next_buffer_size/2u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s){
      for(std::size_t l = 0; l != bits; ++l){
         const std::size_t alignment = std::size_t(1u) << l;
         monotonic_buffer_resource m(&sbr);
         const std::size_t next = m.next_buffer_size();
         void *const p = m.allocate(sizes[s], alignment);
         BOOST_TEST(p != 0);
         BOOST_TEST(sbr.m_allocated);
         BOOST_TEST(sbr.m_bytes >= next);
         BOOST_TEST(is_valid_upstream_alignment(sbr.m_alignment, alignment));
         m.release();
         BOOST_TEST(!sbr.m_allocated);
      }
   }
   BOOST_TEST(sbr.m_mismatches == 0u);
}

//Upstream resource that records the last request and always throws bad_alloc
class throwing_resource
   : public memory_resource
{
   public:
   throwing_resource()
      : m_calls(), m_bytes(), m_alignment(), m_deallocations()
   {}

   virtual void* do_allocate(std::size_t bytes, std::size_t alignment) BOOST_OVERRIDE
   {
      ++m_calls;
      m_bytes = bytes;
      m_alignment = alignment;
      boost::container::throw_bad_alloc();
      BOOST_UNREACHABLE_RETURN(0)
   }

   virtual void do_deallocate(void*, std::size_t, std::size_t) BOOST_OVERRIDE
   {  ++m_deallocations;  }

   virtual bool do_is_equal(const memory_resource& other) const BOOST_NOEXCEPT BOOST_OVERRIDE
   {  return this == &other;  }

   std::size_t m_calls;
   std::size_t m_bytes;
   std::size_t m_alignment;
   std::size_t m_deallocations;
};

//Checks the upstream request (if any) made by an allocation of "bytes" that threw:
//its size is not less than max(bytes, next) and not bigger than PTRDIFF_MAX, and
//requests bigger than PTRDIFF_MAX throw bad_alloc without calling upstream.
bool check_throwing_request
   (const throwing_resource &tr, std::size_t calls_before, std::size_t bytes, std::size_t next, std::size_t alignment)
{
   const std::size_t ptrdiff_max = std::size_t(-1)/2u;
   if(tr.m_calls == calls_before)
      return true;
   return tr.m_calls == calls_before + 1u
      && bytes <= ptrdiff_max && next <= ptrdiff_max
      && tr.m_bytes >= bytes && tr.m_bytes >= next && tr.m_bytes <= ptrdiff_max
      && is_valid_upstream_alignment(tr.m_alignment, alignment);
}

//Huge requests and huge buffer sizes throw bad_alloc without effects. The upstream
//resource is never asked for more than PTRDIFF_MAX bytes.
void test_do_allocate_size_limits()
{
   #ifndef BOOST_NO_EXCEPTIONS
   const std::size_t ptrdiff_max = std::size_t(-1)/2u;
   throwing_resource tr;
   {
      monotonic_buffer_resource m(&tr);
      const std::size_t next = m.next_buffer_size();
      const std::size_t sizes[] =
         { ptrdiff_max/2u, ptrdiff_max - 4096u, ptrdiff_max - 1u, ptrdiff_max, ptrdiff_max + 1u, std::size_t(-1) };
      for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i){
         const std::size_t alignments[] = { 1u, memory_resource::max_align*2u };
         for(std::size_t j = 0; j != sizeof(alignments)/sizeof(alignments[0]); ++j){
            const std::size_t calls = tr.m_calls;
            BOOST_TEST_THROWS((void)m.allocate(sizes[i], alignments[j]), boost::container::bad_alloc_t);
            BOOST_TEST(check_throwing_request(tr, calls, sizes[i], next, alignments[j]));
            //No effects
            BOOST_TEST(m.next_buffer_size() == next);
            BOOST_TEST(m.current_buffer() == 0);
         }
      }
   }
   //Huge initial sizes
   {
      const std::size_t sizes[] = { ptrdiff_max, ptrdiff_max + 1u, std::size_t(-1) };
      for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i){
         monotonic_buffer_resource m(sizes[i], &tr);
         const std::size_t next = m.next_buffer_size();
         BOOST_TEST(next >= sizes[i]);
         const std::size_t calls = tr.m_calls;
         BOOST_TEST_THROWS((void)m.allocate(1u, 1u), boost::container::bad_alloc_t);
         BOOST_TEST(check_throwing_request(tr, calls, 1u, next, 1u));
         BOOST_TEST(m.next_buffer_size() == next);
         BOOST_TEST(m.current_buffer() == 0);
         //release() restores the initial size
         m.release();
         BOOST_TEST(m.next_buffer_size() == next);
      }
   }
   BOOST_TEST(tr.m_deallocations == 0u);
   #endif
}

//Every byte of every returned block can be used: the blocks are inside the upstream
//buffers and do not overlap each other or the data that the resource keeps in those
//buffers (if they did, release() would deallocate with another size or alignment).
struct filled_block_info
{
   unsigned char *p;
   std::size_t bytes;
   unsigned char value;
};

void test_do_allocate_fill_blocks()
{
   typedef filled_block_info block_info;
   memory_resource_logger mrl;
   {
      monotonic_buffer_resource m(&mrl);
      boost::container::vector<block_info> blocks;
      for(std::size_t i = 0; i != 300u; ++i){
         //Sizes include zero, small sizes and sizes bigger than the next buffer
         const std::size_t bytes = (i*37u) % 1000u;
         //Alignments from 1 to 4096
         const std::size_t alignment = std::size_t(1u) << (i % 13u);
         unsigned char *const p = static_cast<unsigned char*>(m.allocate(bytes, alignment));
         BOOST_TEST(p != 0);
         BOOST_TEST((std::size_t(p) % alignment) == 0u);
         if(bytes){
            //The block is inside one of the upstream buffers
            bool inside = false;
            for(std::size_t j = 0; j != mrl.m_info.size(); ++j){
               const memory_resource_logger::allocation_info &info = mrl.m_info[j];
               inside = inside ||
                  (p >= (unsigned char*)info.address && p + bytes <= (unsigned char*)info.address + info.bytes);
            }
            BOOST_TEST(inside);
         }
         const block_info b = { p, bytes, (unsigned char)(i + 1u) };
         std::memset(p, b.value, bytes);
         blocks.push_back(b);
      }
      //Later allocations did not overwrite the blocks
      for(std::size_t i = 0; i != blocks.size(); ++i){
         for(std::size_t j = 0; j != blocks[i].bytes; ++j){
            if(blocks[i].p[j] != blocks[i].value){
               BOOST_TEST(blocks[i].p[j] == blocks[i].value);
               break;
            }
         }
      }
   }
   BOOST_TEST(mrl.m_mismatches == 0u);
   BOOST_TEST(mrl.m_info.size() == 0u);
}

int main()
{
   test_resource_constructor();
   test_initial_size_constructor();
   test_buffer_constructor();

   test_upstream_resource();
   test_do_allocate();
   test_do_allocate_zero();
   test_do_allocate_overaligned();
   test_do_allocate_overflow_and_upstream_failure();
   test_do_deallocate();
   test_do_is_equal();
   test_release();
   test_destructor();
   test_release_restores_next_buffer_size();
   test_do_allocate_overaligned_upstream_alignment();
   test_buffer_size_and_growth();
   test_do_allocate_any_alignment();
   test_do_allocate_size_limits();
   test_do_allocate_fill_blocks();
   return ::boost::report_errors();
}
