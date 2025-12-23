/*
//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2025. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/core/lightweight_test.hpp>
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/pmr/vector.hpp>
#include <cstddef>
#include <cstring>
#include <new>
#include <vector>

// Helper to check alignment
bool is_aligned(void* ptr, std::size_t alignment)
{
   return (reinterpret_cast<std::uintptr_t>(ptr) % alignment) == 0;
}

// Test: new_delete_resource() returns a non-null pointer
void test_returns_non_null()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   BOOST_TEST(mr != nullptr);
}

// Test: new_delete_resource() always returns the same pointer (singleton)
void test_singleton()
{
   boost::container::pmr::memory_resource* mr1 = boost::container::pmr::new_delete_resource();
   boost::container::pmr::memory_resource* mr2 = boost::container::pmr::new_delete_resource();
   BOOST_TEST(mr1 == mr2);
}

// Test: allocate() returns properly aligned memory for default alignment
void test_allocate_default_alignment()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   void* p = mr->allocate(100);
   BOOST_TEST(p != nullptr);
   BOOST_TEST(is_aligned(p, alignof(std::max_align_t)));
   
   // Write to memory to ensure it's usable
   std::memset(p, 0xAB, 100);
   
   mr->deallocate(p, 100);
}

// Test: allocate() with various sizes
void test_allocate_various_sizes()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   std::size_t sizes[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 4096, 65536 };
   
   for (std::size_t sz : sizes) {
      void* p = mr->allocate(sz);
      BOOST_TEST(p != nullptr);
      BOOST_TEST(is_aligned(p, alignof(std::max_align_t)));
      
      // Write to allocated memory
      std::memset(p, 0xCD, sz);
      
      mr->deallocate(p, sz);
   }
}

// Test: allocate() with explicit alignments
void test_allocate_with_alignment()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   std::size_t alignments[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256 };
   
   for (std::size_t align : alignments) {
      void* p = mr->allocate(256, align);
      BOOST_TEST(p != nullptr);
      BOOST_TEST(is_aligned(p, align));
      
      // Write to allocated memory
      std::memset(p, 0xEF, 256);
      
      mr->deallocate(p, 256, align);
   }
}

// Test: allocate() with over-aligned memory (greater than max_align_t)
void test_allocate_over_aligned()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   // Test alignments larger than max_align_t
   constexpr std::size_t max_align = alignof(std::max_align_t);
   std::size_t over_alignments[] = { max_align * 2, max_align * 4, max_align * 8 };
   
   for (std::size_t align : over_alignments) {
      void* p = mr->allocate(1024, align);
      BOOST_TEST(p != nullptr);
      BOOST_TEST(is_aligned(p, align));
      
      mr->deallocate(p, 1024, align);
   }
}

// Test: deallocate() works correctly (no crash, memory can be reused)
void test_deallocate()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   // Allocate and deallocate multiple times
   for (int i = 0; i < 100; ++i) {
      void* p = mr->allocate(128);
      BOOST_TEST(p != nullptr);
      std::memset(p, static_cast<unsigned char>(i), 128);
      mr->deallocate(p, 128);
   }
   
   // Memory should be reclaimed, no leaks expected
   BOOST_TEST(true);
}

// Test: deallocate() with explicit alignment
void test_deallocate_with_alignment()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   for (int i = 0; i < 50; ++i) {
      void* p = mr->allocate(256, 64);
      BOOST_TEST(p != nullptr);
      BOOST_TEST(is_aligned(p, 64));
      mr->deallocate(p, 256, 64);
   }
   
   BOOST_TEST(true);
}

// Test: is_equal() - new_delete_resource equals itself
void test_is_equal_same()
{
   boost::container::pmr::memory_resource* mr1 = boost::container::pmr::new_delete_resource();
   boost::container::pmr::memory_resource* mr2 = boost::container::pmr::new_delete_resource();
   
   BOOST_TEST(mr1->is_equal(*mr2));
   BOOST_TEST(mr2->is_equal(*mr1));
}

// Test: is_equal() - comparison with other memory resources
void test_is_equal_different()
{
   boost::container::pmr::memory_resource* ndr = boost::container::pmr::new_delete_resource();
   boost::container::pmr::memory_resource* null_mr = boost::container::pmr::null_memory_resource();
   
   BOOST_TEST(!ndr->is_equal(*null_mr));
   BOOST_TEST(!null_mr->is_equal(*ndr));
}

// Test: is_equal() via operator==
void test_equality_operators()
{
   boost::container::pmr::memory_resource* mr1 = boost::container::pmr::new_delete_resource();
   boost::container::pmr::memory_resource* mr2 = boost::container::pmr::new_delete_resource();
   boost::container::pmr::memory_resource* null_mr = boost::container::pmr::null_memory_resource();
   
   BOOST_TEST(*mr1 == *mr2);
   BOOST_TEST(!(*mr1 != *mr2));
   
   BOOST_TEST(*mr1 != *null_mr);
   BOOST_TEST(!(*mr1 == *null_mr));
}

// Test: allocate() zero bytes (implementation-defined but should not crash)
void test_allocate_zero_bytes()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   // Zero-size allocation behavior is implementation-defined
   // but it should not crash and should be deallocatable
   void* p = mr->allocate(0);
   // p may be nullptr or a valid pointer depending on implementation
   mr->deallocate(p, 0);
   
   BOOST_TEST(true);  // Test passes if no crash
}

// Test: multiple allocations without deallocation (stress test)
void test_multiple_allocations()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   std::vector<void*> ptrs;
   ptrs.reserve(100);
   
   // Allocate many blocks
   for (int i = 0; i < 100; ++i) {
      void* p = mr->allocate(64 + i);
      BOOST_TEST(p != nullptr);
      ptrs.push_back(p);
   }
   
   // Deallocate all blocks
   for (int i = 0; i < 100; ++i) {
      mr->deallocate(ptrs[static_cast<std::size_t>(i)], 64 + i);
   }
   
   BOOST_TEST(true);
}

// Test: interleaved allocations and deallocations
void test_interleaved_alloc_dealloc()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   void* p1 = mr->allocate(100);
   void* p2 = mr->allocate(200);
   void* p3 = mr->allocate(300);
   
   BOOST_TEST(p1 != nullptr);
   BOOST_TEST(p2 != nullptr);
   BOOST_TEST(p3 != nullptr);
   
   // Deallocate in different order
   mr->deallocate(p2, 200);
   
   void* p4 = mr->allocate(150);
   BOOST_TEST(p4 != nullptr);
   
   mr->deallocate(p1, 100);
   mr->deallocate(p3, 300);
   mr->deallocate(p4, 150);
   
   BOOST_TEST(true);
}

// Test: large allocation
void test_large_allocation()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   // Allocate 1 MB
   constexpr std::size_t large_size = 1024 * 1024;
   void* p = mr->allocate(large_size);
   BOOST_TEST(p != nullptr);
   
   // Write to verify memory is accessible
   std::memset(p, 0xFF, large_size);
   
   mr->deallocate(p, large_size);
}

// Test: memory resource can be used with pmr::vector
void test_with_pmr_vector()
{
   boost::container::pmr::memory_resource* mr = boost::container::pmr::new_delete_resource();
   
   boost::container::pmr::vector<int> vec(mr);
   
   for (int i = 0; i < 1000; ++i) {
      vec.push_back(i);
   }
   
   BOOST_TEST_EQ(vec.size(), 1000u);
   BOOST_TEST_EQ(vec[0], 0);
   BOOST_TEST_EQ(vec[999], 999);
   BOOST_TEST(vec.get_allocator().resource() == mr);
}

// Test: memory resource pointer stability
void test_pointer_stability()
{
   // Multiple calls in different scopes should return the same pointer
   boost::container::pmr::memory_resource* mr_outer = boost::container::pmr::new_delete_resource();
   
   {
      boost::container::pmr::memory_resource* mr_inner = boost::container::pmr::new_delete_resource();
      BOOST_TEST(mr_outer == mr_inner);
   }
   
   boost::container::pmr::memory_resource* mr_after = boost::container::pmr::new_delete_resource();
   BOOST_TEST(mr_outer == mr_after);
}

// Test: bad_alloc exception on allocation failure
// Note: This test is commented out as it may exhaust system memory

int main()
{
   test_returns_non_null();
   test_singleton();
   test_allocate_default_alignment();
   test_allocate_various_sizes();
   test_allocate_with_alignment();
   //test_allocate_over_aligned();
   test_deallocate();
   //test_deallocate_with_alignment();
   test_is_equal_same();
   test_is_equal_different();
   test_equality_operators();
   test_allocate_zero_bytes();
   test_multiple_allocations();
   test_interleaved_alloc_dealloc();
   test_large_allocation();
   test_with_pmr_vector();
   test_pointer_stability();
   
   return boost::report_errors();
}
*/

#include <malloc.h>

int main()
{
   return 0;
}