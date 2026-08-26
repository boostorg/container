//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Shared library "B" of the dlmalloc intermodule test: inlines its own copy of
//the header-only dlmalloc, which must still resolve to the one process heap.
#include <boost/config.hpp>
#include <boost/container/detail/dlmalloc.hpp>
#include <cstddef>

namespace bc = boost::container;

BOOST_SYMBOL_EXPORT void *lib_b_malloc(std::size_t n)
{  return bc::dlmalloc_malloc(n);  }

BOOST_SYMBOL_EXPORT void lib_b_free(void *p)
{  bc::dlmalloc_free(p);  }

BOOST_SYMBOL_EXPORT std::size_t lib_b_in_use_memory()
{  return bc::dlmalloc_in_use_memory();  }

BOOST_SYMBOL_EXPORT int lib_b_all_deallocated()
{  return bc::dlmalloc_all_deallocated();  }

BOOST_SYMBOL_EXPORT std::size_t lib_b_size(void *p)
{  return bc::dlmalloc_size(p);  }
