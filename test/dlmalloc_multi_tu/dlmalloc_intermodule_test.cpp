//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//The process-wide dlmalloc heap across module (exe/DLL) boundaries: this
//executable and two shared libraries each inline their own copy of dlmalloc,
//yet dlmalloc_heap() must name exactly one object in the process. Memory
//obtained in one module has to be releasable from any other, and all queries
//have to give the same answer everywhere.
//
//dlmalloc_multi_tu_test covers two translation units inside one binary;
//this adds the module boundary.
#include <boost/container/detail/dlmalloc.hpp>
#include "../lightweight_test.hpp"
#include <cstddef>
#include <cstring>

namespace bc = boost::container;

//Exported by dlmalloc_intermodule_lib_a.cpp / dlmalloc_intermodule_lib_b.cpp
void *lib_a_malloc(std::size_t n);
void lib_a_free(void *p);
std::size_t lib_a_allocated_memory();
int lib_a_all_deallocated();
std::size_t lib_a_usable_size(void *p);
std::size_t lib_a_footprint();
const void *lib_a_heap_address();

void *lib_b_malloc(std::size_t n);
void lib_b_free(void *p);
std::size_t lib_b_allocated_memory();
int lib_b_all_deallocated();
std::size_t lib_b_usable_size(void *p);
std::size_t lib_b_footprint();
const void *lib_b_heap_address();

//One object, one address. This is the whole claim; everything below is what
//follows from it.
void test_one_instance()
{
   BOOST_TEST(&bc::dlmalloc_heap() == lib_a_heap_address());
   BOOST_TEST(&bc::dlmalloc_heap() == lib_b_heap_address());
}

//Every module reports the same figures, because there is one heap.
void test_statistics_agree()
{
   BOOST_TEST(bc::dlmalloc_heap().allocated_memory() == lib_a_allocated_memory());
   BOOST_TEST(bc::dlmalloc_heap().allocated_memory() == lib_b_allocated_memory());
   BOOST_TEST(bc::dlmalloc_heap().footprint() == lib_a_footprint());
   BOOST_TEST(bc::dlmalloc_heap().footprint() == lib_b_footprint());
}

//Allocate in one module, release in another, in every direction. With
//per-module heaps any one of these corrupts a heap or crashes.
void test_cross_module_free()
{
   void *pa = lib_a_malloc(1000);
   BOOST_TEST(pa != 0);
   test_statistics_agree();
   lib_b_free(pa);

   void *pb = lib_b_malloc(2000);
   BOOST_TEST(pb != 0);
   bc::dlmalloc_heap().deallocate(pb);

   void *pe = bc::dlmalloc_heap().allocate(3000);
   BOOST_TEST(pe != 0);
   lib_a_free(pe);
}

//A block's bookkeeping is readable from any module, not just the one that
//obtained it: the block header is in the shared heap, not in module state.
void test_block_size_visible_everywhere()
{
   const std::size_t requested = 1234u;
   void *p = lib_a_malloc(requested);
   BOOST_TEST(p != 0);

   const std::size_t from_exe = bc::dlmalloc::usable_size(p);
   BOOST_TEST(from_exe >= requested);
   BOOST_TEST(lib_a_usable_size(p) == from_exe);
   BOOST_TEST(lib_b_usable_size(p) == from_exe);

   lib_b_free(p);
}

//Growing a block obtained elsewhere: reallocate() may move it, and the block
//it leaves behind belongs to the same heap whichever module asked.
void test_cross_module_reallocate()
{
   char *p = static_cast<char *>(lib_a_malloc(64));
   BOOST_TEST(p != 0);
   std::memset(p, 'x', 64);

   char *const grown = static_cast<char *>(bc::dlmalloc_heap().reallocate(p, 8000));
   BOOST_TEST(grown != 0);
   for(std::size_t i = 0; i != 64; ++i)
      BOOST_TEST(grown[i] == 'x');

   lib_b_free(grown);
}

//The walkers see the blocks of every module alike: what one module allocates
//shows up in the figures the others read.
void test_walkers_see_every_module()
{
   const std::size_t before = bc::dlmalloc_heap().allocated_memory();

   void *pa = lib_a_malloc(5000);
   void *pb = lib_b_malloc(5000);
   BOOST_TEST(pa != 0 && pb != 0);

   //Both blocks are counted, and counted by all three modules
   const std::size_t after = bc::dlmalloc_heap().allocated_memory();
   BOOST_TEST(after >= before + 10000u);
   BOOST_TEST(after == lib_a_allocated_memory());
   BOOST_TEST(after == lib_b_allocated_memory());

   //mallinfo() answers for the whole process too
   const bc::dlmalloc::mallinfo_t mi = bc::dlmalloc_heap().mallinfo();
   BOOST_TEST(mi.uordblks >= 10000u);
   BOOST_TEST(mi.fordblks + mi.uordblks <= bc::dlmalloc_heap().footprint());

   lib_b_free(pa);
   lib_a_free(pb);
   BOOST_TEST(bc::dlmalloc_heap().allocated_memory() == before);
}

//After everything is returned, all three modules agree the heap is empty.
void test_all_deallocated_agrees()
{
   BOOST_TEST(bc::dlmalloc_heap().all_deallocated());
   BOOST_TEST(lib_a_all_deallocated() != 0);
   BOOST_TEST(lib_b_all_deallocated() != 0);
}

int main()
{
   test_one_instance();
   test_statistics_agree();
   test_cross_module_free();
   test_block_size_visible_everywhere();
   test_cross_module_reallocate();
   test_walkers_see_every_module();
   test_all_deallocated_agrees();
   return boost::report_errors();
}
