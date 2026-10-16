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
//must be compatible with <windows.h>: here <windows.h> is included after them.
#include <boost/container/detail/dlmalloc.hpp>
#include "lightweight_test.hpp"
#if defined(_WIN32) || defined(__CYGWIN__)
#  include <windows.h>
#endif

//The intermodule dlmalloc heap works
int main()
{
   void *const q = boost::container::dlmalloc_heap().allocate(8u);
   BOOST_TEST(q != 0);
   boost::container::dlmalloc_heap().deallocate(q);
   return boost::report_errors();
}
