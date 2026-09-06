//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Shared library "B" of the dlmalloc intermodule test: inlines its own copy
//of dlmalloc, whose process-wide instance must still be the one heap.
#include <boost/config.hpp>
#include <boost/container/detail/dlmalloc.hpp>
#include <cstddef>

namespace bc = boost::container;

BOOST_SYMBOL_EXPORT void *lib_b_malloc(std::size_t n)
{  return bc::dlmalloc_heap().allocate(n);  }

BOOST_SYMBOL_EXPORT void lib_b_free(void *p)
{  bc::dlmalloc_heap().deallocate(p);  }

BOOST_SYMBOL_EXPORT std::size_t lib_b_allocated_memory()
{  return bc::dlmalloc_heap().allocated_memory();  }

BOOST_SYMBOL_EXPORT int lib_b_all_deallocated()
{  return bc::dlmalloc_heap().all_deallocated() ? 1 : 0;  }

BOOST_SYMBOL_EXPORT std::size_t lib_b_usable_size(void *p)
{  return bc::dlmalloc::usable_size(p);  }

BOOST_SYMBOL_EXPORT std::size_t lib_b_footprint()
{  return bc::dlmalloc_heap().footprint();  }

//The address of the one process-wide heap, as this module sees it
BOOST_SYMBOL_EXPORT const void *lib_b_heap_address()
{  return &bc::dlmalloc_heap();  }
