//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Header-only dlmalloc: two translation units must link without duplicate
//symbols and share a single heap (memory allocated in one TU must be
//freeable from the other, and global statistics must agree).
#include <boost/container/detail/dlmalloc.hpp>
#include "../lightweight_test.hpp"

//Defined in dlmalloc_multi_tu_test_other.cpp
void *other_tu_malloc(std::size_t n);
void  other_tu_free(void *p);
std::size_t other_tu_in_use_memory();

int main()
{
   using namespace boost::container;

   //Allocate in the other TU, free here
   void *p = other_tu_malloc(1000);
   BOOST_TEST(p != 0);
   dlmalloc_free(p);

   //Allocate here, free in the other TU
   void *q = dlmalloc_malloc(500);
   BOOST_TEST(q != 0);
   //Both TUs must observe the same allocation counter
   BOOST_TEST(other_tu_in_use_memory() == dlmalloc_in_use_memory());
   other_tu_free(q);

   BOOST_TEST(dlmalloc_all_deallocated() != 0);
   return boost::report_errors();
}
