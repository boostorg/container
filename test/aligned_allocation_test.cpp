//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#include <boost/container/detail/workaround.hpp>   //BOOST_CONTAINER_ASAN
#include <boost/container/detail/aligned_allocation.hpp>
#include "lightweight_test.hpp"
#include <cstddef>

//The test asks for blocks that can not be allocated, which must return null.
//AddressSanitizer reports them as errors unless allocator_may_return_null is set.
#if defined(BOOST_CONTAINER_ASAN)
extern "C" const char* __asan_default_options()
{  return "allocator_may_return_null=1";  }
#endif

using boost::container::dtl::aligned_allocate;
using boost::container::dtl::aligned_deallocate;

void test_aligned_allocation()
{
   //A size that does not fit with the alignment and the bookkeeping
   const std::size_t huge = std::size_t(-1) - 8u;
   BOOST_TEST(aligned_allocate(64u, huge) == 0);
   BOOST_TEST(aligned_allocate(4096u, std::size_t(-1)) == 0);

   //The returned blocks are aligned and usable
   for(std::size_t al = 1u; al <= 4096u; al *= 2u){
      unsigned char *const p = static_cast<unsigned char*>(aligned_allocate(al, 100u));
      BOOST_TEST(p != 0);
      if(p){
         BOOST_TEST(reinterpret_cast<std::size_t>(p) % al == 0u);
         for(std::size_t i = 0; i != 100u; ++i)
            p[i] = (unsigned char)i;
         aligned_deallocate(p);
      }
   }
   //A null pointer is accepted
   aligned_deallocate(0);
}

int main()
{
   test_aligned_allocation();
   return boost::report_errors();
}
