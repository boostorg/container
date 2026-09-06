//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//dlmalloc is header-only, so every one of its members is inline. Two
//translation units must link without duplicate symbols, and one heap must
//work the same from both: memory taken in one unit is given back in the
//other, and both units read the same figures from it.
#include <boost/container/detail/dlmalloc.hpp>
#include "../lightweight_test.hpp"

//Defined in dlmalloc_multi_tu_test_other.cpp
void       *other_tu_allocate(boost::container::dlmalloc &h, std::size_t n);
void        other_tu_deallocate(boost::container::dlmalloc &h, void *p);
std::size_t other_tu_footprint(const boost::container::dlmalloc &h);
std::size_t other_tu_usable_size(const void *p);

int main()
{
   using boost::container::dlmalloc;
   dlmalloc h;

   //Take it there, give it back here
   void *const p = other_tu_allocate(h, 1000);
   BOOST_TEST(p != 0);
   BOOST_TEST(other_tu_usable_size(p) == dlmalloc::usable_size(p));
   h.deallocate(p);

   //Take it here, give it back there
   void *const q = h.allocate(500);
   BOOST_TEST(q != 0);
   //Both units must read the same figure from the same heap
   BOOST_TEST(other_tu_footprint(h) == h.footprint());
   other_tu_deallocate(h, q);

   BOOST_TEST(h.check());
   return boost::report_errors();
}
