//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Shared library "A" of the pmr intermodule test: inlines its own copy of the
//header-only polymorphic resources, which must still share one set of
//process-wide singletons and one default-resource slot.
#include <boost/config.hpp>
#include <boost/container/pmr/global_resource.hpp>
#include <cstddef>

namespace bc = boost::container;

BOOST_SYMBOL_EXPORT bc::pmr::memory_resource *lib_a_new_delete_resource()
{  return bc::pmr::new_delete_resource();  }

BOOST_SYMBOL_EXPORT bc::pmr::memory_resource *lib_a_null_memory_resource()
{  return bc::pmr::null_memory_resource();  }

BOOST_SYMBOL_EXPORT bc::pmr::memory_resource *lib_a_get_default_resource()
{  return bc::pmr::get_default_resource();  }

BOOST_SYMBOL_EXPORT bc::pmr::memory_resource *lib_a_set_default_resource(bc::pmr::memory_resource *r)
{  return bc::pmr::set_default_resource(r);  }

//Allocates through whatever resource is currently installed as the default,
//so the executable can watch a library route through a resource the
//executable itself owns.
BOOST_SYMBOL_EXPORT void *lib_a_allocate_from_default(std::size_t bytes, std::size_t align)
{  return bc::pmr::get_default_resource()->allocate(bytes, align);  }

BOOST_SYMBOL_EXPORT void lib_a_deallocate_from_default(void *p, std::size_t bytes, std::size_t align)
{  bc::pmr::get_default_resource()->deallocate(p, bytes, align);  }
