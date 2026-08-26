//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Shared library "B" of the intermodule test: instantiates its own inline
//copy of the header-only Boost.Container globals machinery.
#include <boost/config.hpp>
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/detail/dlmalloc.hpp>
#include <cstddef>

namespace bc = boost::container;

BOOST_SYMBOL_EXPORT bc::pmr::memory_resource *lib_b_new_delete_resource()
{  return bc::pmr::new_delete_resource();  }

BOOST_SYMBOL_EXPORT bc::pmr::memory_resource *lib_b_null_memory_resource()
{  return bc::pmr::null_memory_resource();  }

BOOST_SYMBOL_EXPORT bc::pmr::memory_resource *lib_b_get_default_resource()
{  return bc::pmr::get_default_resource();  }

BOOST_SYMBOL_EXPORT bc::pmr::memory_resource *lib_b_set_default_resource(bc::pmr::memory_resource *r)
{  return bc::pmr::set_default_resource(r);  }

BOOST_SYMBOL_EXPORT void *lib_b_malloc(std::size_t n)
{  return bc::dlmalloc_malloc(n);  }

BOOST_SYMBOL_EXPORT void lib_b_free(void *p)
{  bc::dlmalloc_free(p);  }

BOOST_SYMBOL_EXPORT std::size_t lib_b_in_use_memory()
{  return bc::dlmalloc_in_use_memory();  }

BOOST_SYMBOL_EXPORT int lib_b_all_deallocated()
{  return bc::dlmalloc_all_deallocated();  }
