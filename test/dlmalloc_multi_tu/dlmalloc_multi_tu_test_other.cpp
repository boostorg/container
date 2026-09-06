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

void *other_tu_allocate(boost::container::dlmalloc &h, std::size_t n)
{  return h.allocate(n);  }

void other_tu_deallocate(boost::container::dlmalloc &h, void *p)
{  h.deallocate(p);  }

std::size_t other_tu_footprint(const boost::container::dlmalloc &h)
{  return h.footprint();  }

std::size_t other_tu_usable_size(const void *p)
{  return boost::container::dlmalloc::usable_size(p);  }
