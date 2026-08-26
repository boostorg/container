//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Second translation unit of pmr_multi_tu_test (no main here).
#include <boost/container/pmr/global_resource.hpp>

namespace bc = boost::container;

bc::pmr::memory_resource *other_tu_new_delete_resource()
{  return bc::pmr::new_delete_resource();  }

bc::pmr::memory_resource *other_tu_null_memory_resource()
{  return bc::pmr::null_memory_resource();  }

bc::pmr::memory_resource *other_tu_get_default_resource()
{  return bc::pmr::get_default_resource();  }

bc::pmr::memory_resource *other_tu_set_default_resource(bc::pmr::memory_resource *r)
{  return bc::pmr::set_default_resource(r);  }
