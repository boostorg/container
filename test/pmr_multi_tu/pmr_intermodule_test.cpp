//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Polymorphic memory resources across module boundaries: this executable and
//two shared libraries each inline their own copy of the header-only pmr code,
//yet the process must behave as if there were one:
//  - new_delete_resource() and null_memory_resource() have one address in
//    every module.
//  - set_default_resource() in any module is observed by all the others, and
//    a resource owned by one module is usable through another's default.
//
//pmr_multi_tu_test covers the same ground for two translation units inside a
//single binary; this adds the module boundary.
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include "../lightweight_test.hpp"
#include <cstddef>
#include <cstdlib>

namespace bc = boost::container;
using bc::pmr::memory_resource;

//Exported by pmr_intermodule_lib_a.cpp / pmr_intermodule_lib_b.cpp
memory_resource *lib_a_new_delete_resource();
memory_resource *lib_a_null_memory_resource();
memory_resource *lib_a_get_default_resource();
memory_resource *lib_a_set_default_resource(memory_resource *r);
void *lib_a_allocate_from_default(std::size_t bytes, std::size_t align);
void lib_a_deallocate_from_default(void *p, std::size_t bytes, std::size_t align);

memory_resource *lib_b_new_delete_resource();
memory_resource *lib_b_null_memory_resource();
memory_resource *lib_b_get_default_resource();
memory_resource *lib_b_set_default_resource(memory_resource *r);
void *lib_b_allocate_from_default(std::size_t bytes, std::size_t align);
void lib_b_deallocate_from_default(void *p, std::size_t bytes, std::size_t align);

namespace {

//A resource that lives in the executable and counts what goes through it, so
//a library allocating "from the default" can be seen reaching back into this
//module's object.
class exe_resource
   : public memory_resource
{
   public:
   exe_resource() : allocations(0), deallocations(0) {}

   virtual ~exe_resource() BOOST_OVERRIDE
   {}

   virtual void *do_allocate(std::size_t bytes, std::size_t) BOOST_OVERRIDE
   {  ++allocations;  return std::malloc(bytes ? bytes : 1u);  }

   virtual void do_deallocate(void *p, std::size_t, std::size_t) BOOST_OVERRIDE
   {  ++deallocations;  std::free(p);  }

   virtual bool do_is_equal(const memory_resource &other) const BOOST_NOEXCEPT BOOST_OVERRIDE
   {  return &other == this;  }

   unsigned allocations;
   unsigned deallocations;
};

}  //namespace

//The resource singletons have one process-wide address.
void test_singletons_are_shared()
{
   memory_resource *const ndr = bc::pmr::new_delete_resource();
   BOOST_TEST(ndr != 0);
   BOOST_TEST(lib_a_new_delete_resource() == ndr);
   BOOST_TEST(lib_b_new_delete_resource() == ndr);

   memory_resource *const null_r = bc::pmr::null_memory_resource();
   BOOST_TEST(null_r != 0);
   BOOST_TEST(lib_a_null_memory_resource() == null_r);
   BOOST_TEST(lib_b_null_memory_resource() == null_r);

   BOOST_TEST(ndr != null_r);
}

//One default-resource slot for the whole process, written and read from any
//module.
void test_default_resource_is_shared()
{
   memory_resource *const ndr = bc::pmr::new_delete_resource();
   BOOST_TEST(bc::pmr::get_default_resource() == ndr);
   BOOST_TEST(lib_a_get_default_resource() == ndr);
   BOOST_TEST(lib_b_get_default_resource() == ndr);

   static exe_resource user_res;

   //Installed by library A, observed by the executable and by library B
   memory_resource *prev = lib_a_set_default_resource(&user_res);
   BOOST_TEST(prev == ndr);
   BOOST_TEST(bc::pmr::get_default_resource() == &user_res);
   BOOST_TEST(lib_a_get_default_resource() == &user_res);
   BOOST_TEST(lib_b_get_default_resource() == &user_res);

   //Installed by the executable, observed by both libraries; null restores
   //new_delete_resource
   prev = bc::pmr::set_default_resource(0);
   BOOST_TEST(prev == &user_res);
   BOOST_TEST(lib_a_get_default_resource() == ndr);
   BOOST_TEST(lib_b_get_default_resource() == ndr);
}

//A library allocating "from the default" must reach the executable's own
//resource object and call its virtuals - the sharpest check that this is one
//slot holding one pointer, not three copies.
void test_libraries_allocate_through_exe_resource()
{
   static exe_resource user_res;
   memory_resource *const prev = bc::pmr::set_default_resource(&user_res);

   const unsigned allocations_before = user_res.allocations;

   void *pa = lib_a_allocate_from_default(128u, sizeof(void *));
   BOOST_TEST(pa != 0);
   BOOST_TEST_EQ(user_res.allocations, allocations_before + 1u);

   void *pb = lib_b_allocate_from_default(256u, sizeof(void *));
   BOOST_TEST(pb != 0);
   BOOST_TEST_EQ(user_res.allocations, allocations_before + 2u);

   //Allocated in A, released through B, both landing on this module's object
   lib_b_deallocate_from_default(pa, 128u, sizeof(void *));
   lib_a_deallocate_from_default(pb, 256u, sizeof(void *));
   BOOST_TEST_EQ(user_res.deallocations, 2u);

   bc::pmr::set_default_resource(prev);
   BOOST_TEST(bc::pmr::get_default_resource() == prev);
}

//The shared new_delete_resource is one usable object: allocate through one
//module's handle, release through another's.
void test_shared_resource_allocates()
{
   void *p = bc::pmr::new_delete_resource()->allocate(64u);
   BOOST_TEST(p != 0);
   lib_a_new_delete_resource()->deallocate(p, 64u);

   void *q = lib_b_new_delete_resource()->allocate(32u);
   BOOST_TEST(q != 0);
   bc::pmr::new_delete_resource()->deallocate(q, 32u);
}

//is_equal is identity for these singletons, and identity holds across modules.
void test_is_equal_across_modules()
{
   BOOST_TEST(bc::pmr::new_delete_resource()->is_equal(*lib_a_new_delete_resource()));
   BOOST_TEST(bc::pmr::new_delete_resource()->is_equal(*lib_b_new_delete_resource()));
   BOOST_TEST(!bc::pmr::new_delete_resource()->is_equal(*lib_a_null_memory_resource()));
}

int main()
{
   test_singletons_are_shared();
   test_default_resource_is_shared();
   test_libraries_allocate_through_exe_resource();
   test_shared_resource_allocates();
   test_is_equal_across_modules();
   return boost::report_errors();
}
