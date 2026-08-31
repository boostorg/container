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
//the default-resource slot plus the two standard resource singletons.
struct BOOST_SYMBOL_VISIBLE pmr_globals_t
{
   //Ordinary members: both stateless and publicly destructible
   //Their vtables still live in the image of whichever module constructs
   //them, which is what pin_constructing_module below is for.
   new_delete_resource_imp  ndr;
   null_memory_resource_imp null_r;

   //Declared after ndr and initialized with &hdr: it is never null
   //Accessed only through lock-free atomic_ptr_* , no lock is needed.
   memory_resource *default_resource;

   //Constructed before main() and destroyed after it, like a global object. The
   //default starts out as new_delete_resource().
   pmr_globals_t()
      : default_resource(&ndr)
   {}
};

struct BOOST_SYMBOL_VISIBLE pmr_globals_options
{
   //The resources are polymorphic: their vtables and virtual functions live
   //in the image of whichever module constructs them, so that module must
   //stay loaded while any other one can still reach them.
   static const bool pin_constructing_module = true;

   //destroy_at_exit defaults to true
};

//Both arguments must be default-visible or every module gets its own
//singletons; see BOOST_CONTAINER_INTERMODULE_ASSERT_VISIBLE in
//intermodule_globals.hpp.
BOOST_CONTAINER_INTERMODULE_ASSERT_VISIBLE(pmr_globals_t,
   "boost::container::pmr::pmr_globals_dtl::pmr_globals_t must be declared BOOST_SYMBOL_VISIBLE"
);
BOOST_CONTAINER_INTERMODULE_ASSERT_VISIBLE(pmr_globals_options,
   "boost::container::pmr::pmr_globals_dtl::pmr_globals_options must be "
   "declared BOOST_SYMBOL_VISIBLE.");

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
   return &pmr_globals_dtl::pmr_globals().ndr;
}

//! <b>Returns</b>: A pointer to a static-duration object of a type derived from
//!   memory_resource for which allocate() always throws bad_alloc and for which
//!   deallocate() has no effect. The same value is returned every time this function
//!   is called. For return value p and memory resource r, p->is_equal(r) returns &r == p.
BOOST_CONTAINER_NODISCARD BOOST_CONTAINER_FORCEINLINE memory_resource* null_memory_resource() BOOST_NOEXCEPT
{
   return &pmr_globals_dtl::pmr_globals().null_r;
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
   //A null argument means new_delete_resource(), which is g.ndr itself
   memory_resource *const res = r ? r : &g.ndr;
   //The displaced value is subject to the same invariant as the slot itself
   memory_resource *const previous =
      dtl::atomic_ptr_exchange_acq_rel(&g.default_resource, res);
   BOOST_CONTAINER_ASSUME(previous != 0);
   return previous;
}

//! <b>Returns</b>: The current value of the default
//!   memory resource pointer.
BOOST_CONTAINER_NODISCARD inline memory_resource* get_default_resource() BOOST_NOEXCEPT
{
   //Acquire load, pairing with the release side of the exchange in
   //set_default_resource()
   pmr_globals_dtl::pmr_globals_t &g = pmr_globals_dtl::pmr_globals();
   //Never null: seeded at construction, and set_default_resource() puts &ndr
   //in the slot for a null argument. Stated so callers need not re-check, and
   //so a broken invariant is caught in a debug build instead of miscompiled.
   memory_resource *const current = dtl::atomic_ptr_read_acq(&g.default_resource);
   BOOST_CONTAINER_ASSUME(current != 0);
   return current;
}

}  //namespace pmr {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_PMR_GLOBAL_RESOURCE_HPP
