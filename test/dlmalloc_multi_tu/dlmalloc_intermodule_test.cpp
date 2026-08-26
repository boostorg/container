//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//The header-only dlmalloc heap across module boundaries: this executable and
//two shared libraries each inline their own copy of dlmalloc, yet there must
//be exactly one heap in the process. Memory obtained in one module has to be
//freeable from any other, and the global statistics have to agree everywhere.
//
//dlmalloc_multi_tu_test covers the same ground for two translation units
//inside a single binary; this adds the module boundary.
#include <boost/container/detail/dlmalloc.hpp>
#include "../lightweight_test.hpp"
#include <cstddef>

namespace bc = boost::container;

//Exported by dlmalloc_intermodule_lib_a.cpp / dlmalloc_intermodule_lib_b.cpp
void *lib_a_malloc(std::size_t n);
void lib_a_free(void *p);
std::size_t lib_a_in_use_memory();
int lib_a_all_deallocated();
std::size_t lib_a_size(void *p);

void *lib_b_malloc(std::size_t n);
void lib_b_free(void *p);
std::size_t lib_b_in_use_memory();
int lib_b_all_deallocated();
std::size_t lib_b_size(void *p);

//Every module reports the same in-use figure, because there is one heap.
void test_statistics_agree()
{
   BOOST_TEST(bc::dlmalloc_in_use_memory() == lib_a_in_use_memory());
   BOOST_TEST(bc::dlmalloc_in_use_memory() == lib_b_in_use_memory());
}

//Allocate in one module, free in another, in every direction. With
//per-module heaps any one of these corrupts the heap or crashes.
void test_cross_module_free()
{
   void *pa = lib_a_malloc(1000);
   BOOST_TEST(pa != 0);
   test_statistics_agree();
   lib_b_free(pa);

   void *pb = lib_b_malloc(2000);
   BOOST_TEST(pb != 0);
   bc::dlmalloc_free(pb);

   void *pe = bc::dlmalloc_malloc(3000);
   BOOST_TEST(pe != 0);
   lib_a_free(pe);
}

//A block's bookkeeping is readable from any module, not just the one that
//allocated it: the block header is in the shared heap, not in module state.
void test_block_size_visible_everywhere()
{
   const std::size_t requested = 1234u;
   void *p = lib_a_malloc(requested);
   BOOST_TEST(p != 0);

   const std::size_t from_exe = bc::dlmalloc_size(p);
   BOOST_TEST(from_exe >= requested);
   BOOST_TEST(lib_a_size(p) == from_exe);
   BOOST_TEST(lib_b_size(p) == from_exe);

   lib_b_free(p);
}

//After everything is returned, all three modules agree the heap is empty.
void test_all_deallocated_agrees()
{
   BOOST_TEST(bc::dlmalloc_all_deallocated() != 0);
   BOOST_TEST(lib_a_all_deallocated() != 0);
   BOOST_TEST(lib_b_all_deallocated() != 0);
}

int main()
{
   test_statistics_agree();
   test_cross_module_free();
   test_block_size_visible_everywhere();
   test_all_deallocated_agrees();
   return boost::report_errors();
}
