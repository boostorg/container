//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#include <boost/container/pmr/global_resource.hpp>
#include <boost/container/pmr/memory_resource.hpp>
#include "lightweight_test.hpp"

#include "derived_from_memory_resource.hpp"

#include <cstdlib>
#include <new>

using namespace boost::container;
using namespace boost::container::pmr;

#ifdef BOOST_MSVC
#pragma warning (push)
#pragma warning (disable : 4290)
#endif

#if BOOST_CXX_VERSION >= 201103L
#define BOOST_CONTAINER_NEW_EXCEPTION_SPECIFIER
#define BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER noexcept
#else
#define BOOST_CONTAINER_NEW_EXCEPTION_SPECIFIER    throw(std::bad_alloc)
#define BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER throw()
#endif

#if defined(BOOST_GCC) && (BOOST_GCC >= 50000)
#pragma GCC diagnostic ignored "-Wsized-deallocation"
#endif

//Replacing global operator new/delete is not possible under every sanitizer:
//ASAN does not support overloading it, and TSAN's runtime defines its own
//(a multiple-definition link error on clang)
#if defined(BOOST_CONTAINER_ASAN) || defined(BOOST_CONTAINER_TSAN)
#define BOOST_CONTAINER_TEST_NO_NEW_REPLACEMENT
#endif

#ifndef BOOST_CONTAINER_TEST_NO_NEW_REPLACEMENT

std::size_t allocation_count = 0;

void* operator new[](std::size_t count) BOOST_CONTAINER_NEW_EXCEPTION_SPECIFIER
{
   ++allocation_count;
   return std::malloc(count ? count : 1u);
}

void* operator new(std::size_t count) BOOST_CONTAINER_NEW_EXCEPTION_SPECIFIER
{
   ++allocation_count;
   return std::malloc(count ? count : 1u);
}

void operator delete[](void *p) BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER
{
   --allocation_count;
   return std::free(p);
}

void operator delete(void *p) BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER
{
   --allocation_count;
   return std::free(p);
}

#if defined __cpp_aligned_new

void* operator new[](std::size_t count, std::align_val_t) BOOST_CONTAINER_NEW_EXCEPTION_SPECIFIER
{
   ++allocation_count;
   return std::malloc(count ? count : 1u);
}

void* operator new(std::size_t count, std::align_val_t) BOOST_CONTAINER_NEW_EXCEPTION_SPECIFIER
{
   ++allocation_count;
   return std::malloc(count ? count : 1u);
}

void operator delete[](void *p, std::align_val_t) BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER
{
   --allocation_count;
   return std::free(p);
}

void operator delete(void *p, std::align_val_t) BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER
{
   --allocation_count;
   return std::free(p);
}

#endif

//The library deallocates through the SIZED operator delete whenever the
//compiler provides it, so the counter only balances if that form is replaced
//too. GCC enables sized deallocation by default in C++14 on some targets
//(MinGW does, current Linux GCC does not), which is why leaving it out made
//this test pass on one toolchain and fail on another.
#if defined(__cpp_sized_deallocation)

void operator delete[](void *p, std::size_t) BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER
{
   --allocation_count;
   return std::free(p);
}

void operator delete(void *p, std::size_t) BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER
{
   --allocation_count;
   return std::free(p);
}

#if defined __cpp_aligned_new

void operator delete[](void *p, std::size_t, std::align_val_t) BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER
{
   --allocation_count;
   return std::free(p);
}

void operator delete(void *p, std::size_t, std::align_val_t) BOOST_CONTAINER_DELETE_EXCEPTION_SPECIFIER
{
   --allocation_count;
   return std::free(p);
}

#endif
#endif   //__cpp_sized_deallocation
#endif   //BOOST_CONTAINER_TEST_NO_NEW_REPLACEMENT

#ifdef BOOST_MSVC
#pragma warning (pop)
#endif

#ifndef BOOST_CONTAINER_TEST_NO_NEW_REPLACEMENT

void test_new_delete_resource()
{
   //Make sure new_delete_resource calls new[]/delete[]
   std::size_t memcount = allocation_count;
   memory_resource *mr = new_delete_resource();
   //each time should return the same pointer
   BOOST_TEST(mr == new_delete_resource());
   //The library is header-only, so new_delete_resource runs the caller's own
   //inlined code and a replaced operator new is always observed
   BOOST_TEST(memcount == allocation_count);
   void *const addr = mr->allocate(16, 1);
   BOOST_TEST((allocation_count - memcount) == 1);
   mr->deallocate(addr, 16, 1);
   BOOST_TEST(memcount == allocation_count);

   //A zero-sized allocation must still work
   void *const zero_addr = mr->allocate(0, 1);
   mr->deallocate(zero_addr, 0, 1);
   BOOST_TEST(memcount == allocation_count);
}

#endif   //BOOST_CONTAINER_TEST_NO_NEW_REPLACEMENT

void test_null_memory_resource()
{
   //Make sure it throw or returns null
   memory_resource *mr = null_memory_resource();
   BOOST_TEST(mr != 0);

   #if !defined(BOOST_NO_EXCEPTIONS)
   bool bad_allocexception_thrown = false;

   void *p = 0;
   BOOST_CONTAINER_TRY{
      p = mr->allocate(1, 1);
   }
   BOOST_CONTAINER_CATCH(std::bad_alloc&) {
      bad_allocexception_thrown = true;
   }
   BOOST_CONTAINER_CATCH(...) {
   }
   BOOST_CONTAINER_CATCH_END

   BOOST_TEST(bad_allocexception_thrown == true);
   if(p)
      mr->deallocate(p, 1, 1);

   //A zero-sized allocation must also fail (the standard permits a zero
   //request to fail, and null_memory_resource always throws).
   bool zero_bad_alloc_thrown = false;
   void *pz = 0;
   BOOST_CONTAINER_TRY{
      pz = mr->allocate(0, 1);
   }
   BOOST_CONTAINER_CATCH(std::bad_alloc&) {
      zero_bad_alloc_thrown = true;
   }
   BOOST_CONTAINER_CATCH(...) {
   }
   BOOST_CONTAINER_CATCH_END

   BOOST_TEST(zero_bad_alloc_thrown == true);
   if(pz)
      mr->deallocate(pz, 0, 1);
   #endif   //BOOST_NO_EXCEPTIONS
}

void test_default_resource()
{
   //Default resource must be new/delete before set_default_resource
   BOOST_TEST(get_default_resource() == new_delete_resource());
   //Set default resource and obtain previous
   derived_from_memory_resource d;
   memory_resource *prev_default = set_default_resource(&d);
   BOOST_TEST(get_default_resource() == &d);
   //Set default resource with null, which should be new/delete
   prev_default = set_default_resource(0);
   BOOST_TEST(prev_default == &d);
   BOOST_TEST(get_default_resource() == new_delete_resource());
}

#if !defined(BOOST_NO_CXX11_HDR_THREAD)

#include <thread>
#include <vector>

//set_default_resource() is a lock-free atomic exchange, so concurrent setters
//must still each observe a *distinct* previous value: the returned pointers,
//plus the one left installed at the end, have to be exactly the resources put
//in, with none lost and none seen twice. A torn or non-atomic exchange shows
//up as a duplicate or a missing entry.
void test_concurrent_set_default_resource()
{
   const unsigned threads_c = 8u;
   const unsigned rounds_c  = 2000u;

   std::vector<derived_from_memory_resource> resources(threads_c);
   memory_resource *const initial = set_default_resource(&resources[0]);

   //Every thread repeatedly installs its own resource and records what it
   //displaced; across all threads each installed pointer must come back out
   //exactly as many times as it went in.
   //Sized up front: this file replaces global operator new with a
   //non-atomic counter, so the workers must not allocate.
   std::vector< std::vector<memory_resource *> > seen(threads_c);
   for(unsigned t = 0; t != threads_c; ++t){
      seen[t].resize(rounds_c, 0);
   }
   std::vector<std::thread> workers;
   workers.reserve(threads_c);
   for(unsigned t = 0; t != threads_c; ++t){
      workers.push_back(std::thread([&, t]{
         for(unsigned i = 0; i != rounds_c; ++i){
            seen[t][i] = set_default_resource(&resources[t]);
         }
      }));
   }
   for(unsigned t = 0; t != threads_c; ++t){
      workers[t].join();
   }

   //Every exchange returns what the previous one installed, so the values
   //handed back, plus the one left in the slot, are exactly the values put
   //in. Thread t installed &resources[t] rounds_c times, and resources[0]
   //went in once more before the threads started.
   std::vector<unsigned> returned(threads_c, 0u);
   for(unsigned t = 0; t != threads_c; ++t){
      BOOST_TEST(seen[t].size() == rounds_c);
      for(unsigned i = 0; i != rounds_c; ++i){
         memory_resource *const p = seen[t][i];
         unsigned idx = threads_c;
         for(unsigned r = 0; r != threads_c; ++r){
            if(p == &resources[r]){ idx = r; break; }
         }
         //Nothing else can come out: the slot only ever held one of these
         //once the pre-thread set_default_resource(&resources[0]) landed.
         BOOST_TEST(idx != threads_c);
         if(idx != threads_c){
            ++returned[idx];
         }
      }
   }
   //The last install is still in the slot rather than in anyone's list
   memory_resource *const last = get_default_resource();
   for(unsigned r = 0; r != threads_c; ++r){
      if(last == &resources[r]){
         ++returned[r];
      }
   }

   BOOST_TEST(returned[0] == rounds_c + 1u);
   for(unsigned r = 1; r != threads_c; ++r){
      BOOST_TEST(returned[r] == rounds_c);
   }
   //The value displaced by the very first install was the standing default
   BOOST_TEST(initial == new_delete_resource());

   set_default_resource(0);
   BOOST_TEST(get_default_resource() == new_delete_resource());
}

#endif   //!defined(BOOST_NO_CXX11_HDR_THREAD)

int main()
{
   #ifndef BOOST_CONTAINER_TEST_NO_NEW_REPLACEMENT
   test_new_delete_resource();
   #endif
   test_null_memory_resource();
   test_default_resource();
   #if !defined(BOOST_NO_CXX11_HDR_THREAD)
   test_concurrent_set_default_resource();
   #endif
   return ::boost::report_errors();
}
