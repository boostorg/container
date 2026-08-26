//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Header-only polymorphic memory resources: two translation units must link
//without duplicate symbols and agree on the process-wide state, i.e. the
//resource singletons have one address and the default resource is one slot.
//
//pmr_intermodule_test covers the same ground across a module boundary.
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include "../lightweight_test.hpp"
#include <cstddef>
#include <cstdlib>

namespace bc = boost::container;
using bc::pmr::memory_resource;

//Defined in pmr_multi_tu_test_other.cpp
memory_resource *other_tu_new_delete_resource();
memory_resource *other_tu_null_memory_resource();
memory_resource *other_tu_get_default_resource();
memory_resource *other_tu_set_default_resource(memory_resource *r);

namespace {

//A user-provided resource living in this translation unit
class test_resource
   : public memory_resource
{
   public:
   virtual ~test_resource() BOOST_OVERRIDE
   {}

   virtual void *do_allocate(std::size_t bytes, std::size_t) BOOST_OVERRIDE
   {  return std::malloc(bytes ? bytes : 1u);  }

   virtual void do_deallocate(void *p, std::size_t, std::size_t) BOOST_OVERRIDE
   {  std::free(p);  }

   virtual bool do_is_equal(const memory_resource &other) const BOOST_NOEXCEPT BOOST_OVERRIDE
   {  return &other == this;  }
};

}  //namespace

//One instance of each singleton, whichever translation unit asks.
void test_singletons_are_shared()
{
   memory_resource *const ndr = bc::pmr::new_delete_resource();
   BOOST_TEST(ndr != 0);
   BOOST_TEST(other_tu_new_delete_resource() == ndr);

   memory_resource *const null_r = bc::pmr::null_memory_resource();
   BOOST_TEST(null_r != 0);
   BOOST_TEST(other_tu_null_memory_resource() == null_r);

   BOOST_TEST(ndr != null_r);
}

//One default-resource slot, so a change in either translation unit is seen by
//the other.
void test_default_resource_is_shared()
{
   memory_resource *const ndr = bc::pmr::new_delete_resource();
   BOOST_TEST(bc::pmr::get_default_resource() == ndr);
   BOOST_TEST(other_tu_get_default_resource() == ndr);

   static test_resource user_res;

   //Set here, observed there
   memory_resource *prev = bc::pmr::set_default_resource(&user_res);
   BOOST_TEST(prev == ndr);
   BOOST_TEST(other_tu_get_default_resource() == &user_res);

   //Set there, observed here; a null argument restores new_delete_resource
   prev = other_tu_set_default_resource(0);
   BOOST_TEST(prev == &user_res);
   BOOST_TEST(bc::pmr::get_default_resource() == ndr);
}

//The shared new_delete_resource really allocates, from either side.
void test_shared_resource_allocates()
{
   memory_resource *const ndr = bc::pmr::new_delete_resource();
   void *p = ndr->allocate(64u);
   BOOST_TEST(p != 0);
   //Freed through the other translation unit's handle on the same object
   other_tu_new_delete_resource()->deallocate(p, 64u);
}

int main()
{
   test_singletons_are_shared();
   test_default_resource_is_shared();
   test_shared_resource_allocates();
   return boost::report_errors();
}
