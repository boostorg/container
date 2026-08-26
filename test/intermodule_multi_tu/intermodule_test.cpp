//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//The point of the header-only conversion: this executable and two shared
//libraries each inline their own copy of Boost.Container, yet all globals
//must behave as a single process-wide instance:
//  - new_delete_resource()/null_memory_resource() return the SAME address
//    from every module.
//  - set_default_resource() in one module is visible in all the others.
//  - dlmalloc memory allocated in one module can be freed in another, and
//    the allocation statistics are process-global.
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/detail/dlmalloc.hpp>
#include "../lightweight_test.hpp"
#include <cstddef>
#include <cstdlib>

namespace bc = boost::container;
using bc::pmr::memory_resource;

//Exported by intermodule_lib_a.cpp / intermodule_lib_b.cpp
memory_resource *lib_a_new_delete_resource();
memory_resource *lib_a_null_memory_resource();
memory_resource *lib_a_get_default_resource();
memory_resource *lib_a_set_default_resource(memory_resource *r);
void *lib_a_malloc(std::size_t n);
void lib_a_free(void *p);
std::size_t lib_a_in_use_memory();
int lib_a_all_deallocated();

memory_resource *lib_b_new_delete_resource();
memory_resource *lib_b_null_memory_resource();
memory_resource *lib_b_get_default_resource();
memory_resource *lib_b_set_default_resource(memory_resource *r);
void *lib_b_malloc(std::size_t n);
void lib_b_free(void *p);
std::size_t lib_b_in_use_memory();
int lib_b_all_deallocated();

namespace {

//A user-provided resource living in the executable
class exe_resource
   : public memory_resource
{
   public:
   virtual ~exe_resource() BOOST_OVERRIDE
   {}

   virtual void* do_allocate(std::size_t bytes, std::size_t) BOOST_OVERRIDE
   {  return std::malloc(bytes);  }

   virtual void do_deallocate(void* p, std::size_t, std::size_t) BOOST_OVERRIDE
   {  std::free(p);  }

   virtual bool do_is_equal(const memory_resource& other) const BOOST_NOEXCEPT BOOST_OVERRIDE
   {  return &other == this;   }
};

}  //namespace

void test_global_resources_identity()
{
   //The singletons must have one process-wide address
   memory_resource *const exe_ndr = bc::pmr::new_delete_resource();
   BOOST_TEST(exe_ndr != 0);
   BOOST_TEST(lib_a_new_delete_resource() == exe_ndr);
   BOOST_TEST(lib_b_new_delete_resource() == exe_ndr);

   memory_resource *const exe_null = bc::pmr::null_memory_resource();
   BOOST_TEST(exe_null != 0);
   BOOST_TEST(lib_a_null_memory_resource() == exe_null);
   BOOST_TEST(lib_b_null_memory_resource() == exe_null);
}

void test_default_resource_visibility()
{
   //Initial default is the (process-wide) new_delete_resource in all modules
   memory_resource *const exe_ndr = bc::pmr::new_delete_resource();
   BOOST_TEST(bc::pmr::get_default_resource() == exe_ndr);
   BOOST_TEST(lib_a_get_default_resource() == exe_ndr);
   BOOST_TEST(lib_b_get_default_resource() == exe_ndr);

   //A change made by library A must be observed by the exe and library B
   static exe_resource user_res;
   memory_resource *prev = lib_a_set_default_resource(&user_res);
   BOOST_TEST(prev == exe_ndr);
   BOOST_TEST(bc::pmr::get_default_resource() == &user_res);
   BOOST_TEST(lib_a_get_default_resource() == &user_res);
   BOOST_TEST(lib_b_get_default_resource() == &user_res);

   //A change made by the exe must be observed by both libraries;
   //null restores the default
   prev = bc::pmr::set_default_resource(0);
   BOOST_TEST(prev == &user_res);
   BOOST_TEST(lib_a_get_default_resource() == exe_ndr);
   BOOST_TEST(lib_b_get_default_resource() == exe_ndr);
}

void test_shared_dlmalloc_heap()
{
   //Allocate in A, free in B; allocate in B, free in the exe; allocate in
   //the exe, free in A. Any of these corrupts or crashes with per-module
   //heaps.
   void *pa = lib_a_malloc(1000);
   BOOST_TEST(pa != 0);
   //All modules see the same in-use counter
   BOOST_TEST(lib_a_in_use_memory() == lib_b_in_use_memory());
   BOOST_TEST(lib_a_in_use_memory() == bc::dlmalloc_in_use_memory());
   lib_b_free(pa);

   void *pb = lib_b_malloc(2000);
   BOOST_TEST(pb != 0);
   bc::dlmalloc_free(pb);

   void *pe = bc::dlmalloc_malloc(3000);
   BOOST_TEST(pe != 0);
   lib_a_free(pe);

   //Global statistics agree everywhere: everything was deallocated
   BOOST_TEST(bc::dlmalloc_all_deallocated() != 0);
   BOOST_TEST(lib_a_all_deallocated() != 0);
   BOOST_TEST(lib_b_all_deallocated() != 0);
}

int main()
{
   test_global_resources_identity();
   test_default_resource_visibility();
   test_shared_dlmalloc_heap();
   return boost::report_errors();
}
