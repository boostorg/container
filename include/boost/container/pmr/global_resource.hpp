//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_PMR_GLOBAL_RESOURCE_HPP
#define BOOST_CONTAINER_PMR_GLOBAL_RESOURCE_HPP

#if defined (_MSC_VER)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/container_fwd.hpp>

#include <boost/container/pmr/memory_resource.hpp>
#include <boost/container/throw_exception.hpp>
#include <boost/container/detail/operator_new_helpers.hpp>
#include <boost/container/detail/intermodule_globals.hpp>
#include <boost/container/detail/type_traits.hpp>

#include <cstddef>
#include <new>

namespace boost {
namespace container {
namespace pmr {

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

class new_delete_resource_imp BOOST_FINAL
   : public memory_resource
{
   public:

   ~new_delete_resource_imp() BOOST_OVERRIDE
   {}

   void* do_allocate(std::size_t bytes, std::size_t alignment) BOOST_OVERRIDE
   {  return boost::container::dtl::operator_new_raw_allocate(bytes, alignment);  }

   void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) BOOST_OVERRIDE
   {  return boost::container::dtl::operator_delete_raw_deallocate(p, bytes, alignment);  }

   bool do_is_equal(const memory_resource& other) const BOOST_NOEXCEPT BOOST_OVERRIDE
   {  return &other == this;   }
};

struct null_memory_resource_imp BOOST_FINAL
   : public memory_resource
{
   public:

   ~null_memory_resource_imp() BOOST_OVERRIDE
   {}

   void* do_allocate(std::size_t bytes, std::size_t alignment) BOOST_OVERRIDE
   {
      (void)bytes; (void)alignment;
      #if defined(BOOST_CONTAINER_USER_DEFINED_THROW_CALLBACKS) || defined(BOOST_NO_EXCEPTIONS)
      throw_bad_alloc();
      return 0;
      #else
      throw std::bad_alloc();
      #endif
   }

   void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) BOOST_OVERRIDE
   {  (void)p;  (void)bytes; (void)alignment;  }

   bool do_is_equal(const memory_resource& other) const BOOST_NOEXCEPT BOOST_OVERRIDE
   {  return &other == this;   }
};

namespace pmr_globals_dtl {

//Process-wide PMR state, shared across modules (see intermodule_globals.hpp):
//the default-resource slot plus in-place storage for the two standard
//resource singletons, so new_delete_resource()/null_memory_resource() return
//the same address from every module of the process. POD, zero-initialized.
struct BOOST_SYMBOL_VISIBLE pmr_globals_t
{
   //Replaced with a lock-free atomic exchange, so no lock is needed here:
   //set_default_resource() is a single pointer-sized read-modify-write and
   //get_default_resource() a single load.
   void *default_resource;          //memory_resource*; 0 = new_delete_resource()
   void *new_delete_ptr;            //upcast memory_resource* of ndr_storage
   void *null_ptr;                  //upcast memory_resource* of null_storage
   dtl::aligned_storage
      < sizeof(new_delete_resource_imp)
      , dtl::alignment_of<new_delete_resource_imp>::value>::type ndr_storage;
   dtl::aligned_storage
      < sizeof(null_memory_resource_imp)
      , dtl::alignment_of<null_memory_resource_imp>::value>::type null_storage;

   //Constructed before main() and destroyed after it, like a global object.
   //The storage always arrives zero-initialized, so default_resource starts
   //as "never set" and get_default_resource() falls back to new_delete.
   pmr_globals_t()
   {
      new_delete_ptr = static_cast<memory_resource *>
         (::new((void *)&ndr_storage, boost_container_new_t()) new_delete_resource_imp);
      null_ptr = static_cast<memory_resource *>
         (::new((void *)&null_storage, boost_container_new_t()) null_memory_resource_imp);
   }

   ~pmr_globals_t()
   {
      static_cast<memory_resource *>(new_delete_ptr)->~memory_resource();
      static_cast<memory_resource *>(null_ptr)->~memory_resource();
   }
};

struct BOOST_SYMBOL_VISIBLE pmr_globals_options
{
   //The resources are polymorphic: their vtables and virtual functions live
   //in the image of whichever module constructs them, so that module must
   //stay loaded while any other one can still reach them.
   static const bool pin_constructing_module = true;

   //destroy_at_exit defaults to true
};

BOOST_CONTAINER_FORCEINLINE pmr_globals_t &pmr_globals()
{
   return dtl::intermodule_globals<pmr_globals_t, pmr_globals_options>();
}

}  //namespace pmr_globals_dtl {

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//! <b>Returns</b>: A pointer to a static-duration object of a type derived from
//!   memory_resource that can serve as a resource for allocating memory using
//!   global `operator new` and global `operator delete`. The same value is returned every time this function
//!   is called. For return value p and memory resource r, p->is_equal(r) returns &r == p.
BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE memory_resource* new_delete_resource() BOOST_NOEXCEPT
{
   return static_cast<memory_resource *>(pmr_globals_dtl::pmr_globals().new_delete_ptr);
}

//! <b>Returns</b>: A pointer to a static-duration object of a type derived from
//!   memory_resource for which allocate() always throws bad_alloc and for which
//!   deallocate() has no effect. The same value is returned every time this function
//!   is called. For return value p and memory resource r, p->is_equal(r) returns &r == p.
BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE memory_resource* null_memory_resource() BOOST_NOEXCEPT
{
   return static_cast<memory_resource *>(pmr_globals_dtl::pmr_globals().null_ptr);
}

//! <b>Effects</b>: If r is non-null, sets the value of the default memory resource
//!   pointer to r, otherwise sets the default memory resource pointer to new_delete_resource().
//!
//! <b>Postconditions</b>: get_default_resource() == r.
//!
//! <b>Returns</b>: The previous value of the default memory resource pointer.
//!
//! <b>Remarks</b>: Calling the set_default_resource and get_default_resource functions shall
//!   not incur a data race. A call to the set_default_resource function shall synchronize
//!   with subsequent calls to the set_default_resource and get_default_resource functions.
inline memory_resource* set_default_resource(memory_resource* r) BOOST_NOEXCEPT
{
   //Lock-free atomic exchange: concurrent setters each get a distinct,
   //coherent "previous" value, and the release side of the exchange makes
   //everything the caller did before it visible to whoever reads the new
   //pointer - the synchronizes-with this function is required to provide.
   pmr_globals_dtl::pmr_globals_t &g = pmr_globals_dtl::pmr_globals();
   memory_resource *const res = r ? r : new_delete_resource();
   memory_resource *previous = static_cast<memory_resource *>
      (dtl::atomic_ptr_exchange_acq_rel(&g.default_resource, static_cast<void *>(res)));
   if(!previous){   //slot still zero-initialized: the default was never changed
      previous = new_delete_resource();
   }
   return previous;
}

//! <b>Returns</b>: The current value of the default
//!   memory resource pointer.
BOOST_CONTAINER_NODISCARD inline memory_resource* get_default_resource() BOOST_NOEXCEPT
{
   //Acquire load, pairing with the release side of the exchange in
   //set_default_resource(). A consume/dependent load would order the
   //resource object itself but not whatever else the setter published
   //beforehand, and this function's contract covers that too.
   pmr_globals_dtl::pmr_globals_t &g = pmr_globals_dtl::pmr_globals();
   memory_resource *current = static_cast<memory_resource *>
      (dtl::atomic_ptr_read_acq(&g.default_resource));
   if(!current){   //slot still zero-initialized: the default was never changed
      current = new_delete_resource();
   }
   return current;
}

}  //namespace pmr {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_PMR_GLOBAL_RESOURCE_HPP
