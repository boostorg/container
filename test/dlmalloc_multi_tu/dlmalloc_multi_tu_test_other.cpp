//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Second translation unit of dlmalloc_multi_tu_test (no main here).
#include <boost/container/detail/dlmalloc.hpp>

void *other_tu_malloc(std::size_t n)
{  return boost::container::dlmalloc_malloc(n);  }

void other_tu_free(void *p)
{  boost::container::dlmalloc_free(p);  }

std::size_t other_tu_in_use_memory()
{  return boost::container::dlmalloc_in_use_memory();  }
