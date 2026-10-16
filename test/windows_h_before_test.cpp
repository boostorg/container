//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//The Win32 declarations of the intermodule globals (intermodule_globals.hpp)
//must be compatible with <windows.h>: here <windows.h> is included before them.
//Boost.Config goes first: with -Wundef, some MinGW <windows.h> versions make
//boost/config/platform/win32.hpp fail when <windows.h> is included before it.
#include <boost/config.hpp>
#if defined(_WIN32) || defined(__CYGWIN__)
#  include <windows.h>
#endif
#include <boost/container/pmr/unsynchronized_pool_resource.hpp>
#include <boost/container/detail/dlmalloc.hpp>
#include "lightweight_test.hpp"

//The intermodule globals (the pmr globals and the dlmalloc heap) work
int main()
{
   boost::container::pmr::unsynchronized_pool_resource r;
   void *const p = r.allocate(10u);
   BOOST_TEST(p != 0);
   r.deallocate(p, 10u);
   void *const q = boost::container::dlmalloc_heap().allocate(8u);
   BOOST_TEST(q != 0);
   boost::container::dlmalloc_heap().deallocate(q);
   #if defined(_WIN32) || defined(__CYGWIN__)
   BOOST_TEST(GetCurrentProcessId() != 0);
   #endif
   return boost::report_errors();
}
