//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_DETAIL_ATOMIC_PTR_HPP
#define BOOST_CONTAINER_DETAIL_ATOMIC_PTR_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <cstddef>   //std::size_t

//////////////////////////////////////////////////////////////////////////////
//
//    Ordered operations on a single pointer
//
//////////////////////////////////////////////////////////////////////////////
//
// Publishing a pointer to an object built beforehand, and reading it back,
// needs ordering but not a full atomic type. These four operations provide
// exactly that, built from compiler intrinsics rather than <atomic>, which is
// unavailable in C++03 - the minimum standard this library supports. Each is
// named after the WEAKEST order it guarantees:
//
//    atomic_ptr_write_rel: publish a pointer once everything it leads to
//       is fully built. Every preceding write must be visible to whoever
//       observes the new value.
//    atomic_ptr_read_csm:  read such a pointer with consume ordering. The
//       value is only ever used to reach the object it points to, so the
//       address dependency of that dereference is what orders the access at
//       runtime and no barrier instruction is required; only the compiler has
//       to be kept in place. This is the shape of the Linux kernel's
//       rcu_dereference: a load that must not be cached, duplicated or torn
//       (hence the volatile access) followed by a compiler-only fence.
//    atomic_ptr_read_acq:  read it with real acquire ordering, which orders
//       every following access and not merely those that reach through the
//       pointer. Needed whenever a synchronizes-with relation is promised to
//       callers, because the writer may have published unrelated state before
//       the pointer store.
//    atomic_ptr_exchange_acq_rel: replace the pointer and return the previous
//       value, so a slot can be swapped without a lock and concurrent
//       replacers still each observe a distinct predecessor.
//
// Asking the compiler for consume instead would NOT do this: GCC and Clang
// promote __ATOMIC_CONSUME to __ATOMIC_ACQUIRE, because faithfully tracking
// the "carries-a-dependency" relation is not implementable without
// pessimizing the optimizer. That costs a real load-acquire (ldar on
// AArch64, lwsync-guarded loads on POWER) for ordering the dependency
// already provides.
//
// The price is that dependency ordering is not something the standard
// promises, so a caller of atomic_ptr_read_csm must not do anything that lets
// the compiler reconstruct the address instead of following the pointer:
// comparing it against other known pointers is the classic way to lose the
// dependency. Comparing against null and adding a constant offset are both
// fine. Anything that cannot honour that restriction wants
// atomic_ptr_read_acq instead.
//
// The tiers below follow boost/interprocess/detail/atomic.hpp. Every one of
// them reads with a plain load plus a compiler-only fence; only the spelling
// of that fence changes (__atomic_signal_fence, _ReadWriteBarrier, or an
// empty asm with a memory clobber). The write side is where they differ,
// since publishing does need the hardware: a release store on GCC/Clang,
// stlr on MSVC ARM64, a dmb before the store on MSVC ARMv7, and nothing but
// the compiler fence on x86/x64, which never reorders the relevant pairs.
//
// The only tier not verified here is MSVC ARMv7 (long dead); MSVC ARM64 was
// checked with the cross compiler and emits ldr for the read and stlr for
// the write.

//ThreadSanitizer cannot model dependency ordering (no happens-before edge is
//recorded for a consume-style read), so the accesses reached through the
//pointer would all be reported as races. Under TSan the consume read is
//promoted to a genuine acquire: same correctness, analyzable by the tool,
//and only instrumented builds pay for it.
#if defined(__SANITIZE_THREAD__)
#  define BOOST_CONTAINER_ATOMIC_PTR_TSAN
#elif defined(__has_feature)
#  if __has_feature(thread_sanitizer)
#     define BOOST_CONTAINER_ATOMIC_PTR_TSAN
#  endif
#endif

#if defined(__ATOMIC_SEQ_CST)
   //GCC >= 4.7 and Clang: the compiler emits the optimal load/store for the
   //target and the requested memory order
#  define BOOST_CONTAINER_ATOMIC_PTR_GNU
#elif defined(_MSC_VER)
   //_ReadWriteBarrier is MSVC's compiler-only fence, the counterpart of
   //__atomic_signal_fence. It is declared here rather than pulled from
   //<intrin.h> so that header is not dragged in on x86/x64; it is marked
   //deprecated there, so every use is wrapped to keep C4996 quiet in case
   //some other header already declared it.
extern "C" void _ReadWriteBarrier(void);
#  pragma intrinsic(_ReadWriteBarrier)
#  define BOOST_CONTAINER_ATOMIC_PTR_DISABLE_DEPRECATED \
             __pragma(warning(push)) __pragma(warning(disable:4996))
#  define BOOST_CONTAINER_ATOMIC_PTR_RESTORE_WARNING __pragma(warning(pop))
#  if defined(_M_ARM64) || defined(_M_ARM64EC)
      //ARMv8 honours address dependencies, so the read needs only a plain
      //load (__iso_volatile_load64, which unlike a volatile access is never
      //given acquire semantics by /volatile:ms) plus the compiler fence.
      //The store does need the hardware: stlr.
#     include <intrin.h>
#     define BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM64
#  elif defined(_M_ARM)
      //ARMv7 is weakly ordered and has no store-release instruction, so a
      //real hardware barrier is required on the store
#     include <intrin.h>
#     define BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM
#  else
      //x86/x64 never reorders a store with the stores that precede it, nor a
      //load with the loads that follow it, so only the compiler must be kept
      //in place
#     define BOOST_CONTAINER_ATOMIC_PTR_MSVC_X86
#  endif
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
   //GCC 4.1 to 4.6: only the legacy __sync builtins are available
#  define BOOST_CONTAINER_ATOMIC_PTR_GNU_SYNC
#else
   //Unknown compiler: no way to order anything explicitly. Correct for the
   //single-threaded configurations that reach this point, best effort
   //otherwise.
#  define BOOST_CONTAINER_ATOMIC_PTR_PLAIN
#endif

#if !defined(BOOST_CONTAINER_ATOMIC_PTR_DISABLE_DEPRECATED)
#  define BOOST_CONTAINER_ATOMIC_PTR_DISABLE_DEPRECATED
#  define BOOST_CONTAINER_ATOMIC_PTR_RESTORE_WARNING
#endif

#if defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_X86)
   //The ARM tiers above already pulled <intrin.h> in; x86/x64 avoids it, so
   //the one interlocked intrinsic needed there is declared by hand, exactly
   //as _ReadWriteBarrier is. Both are full barriers on this architecture,
   //which is all atomic_ptr_exchange_acq_rel promises.
extern "C" {
#  if defined(_WIN64)
__int64 _InterlockedExchange64(__int64 volatile *, __int64);
#     pragma intrinsic(_InterlockedExchange64)
#  else
long _InterlockedExchange(long volatile *, long);
#     pragma intrinsic(_InterlockedExchange)
#  endif
}  //extern "C"
#endif

namespace boost {
namespace container {
namespace dtl {

//Reads a pointer published with atomic_ptr_write_rel, with consume ordering
//("csm"): only accesses that reach through the pointer are ordered
template<class T>
BOOST_CONTAINER_FORCEINLINE T *atomic_ptr_read_csm(T *const *pp)
{
   #if defined(BOOST_CONTAINER_ATOMIC_PTR_TSAN)
   return __atomic_load_n(const_cast<T *const *>(pp), __ATOMIC_ACQUIRE);
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_GNU)
   //A RELAXED atomic load, not a volatile one: it compiles to the very same
   //plain load on every target, but the paired release store then makes the
   //program free of data races in the C++ sense (volatile would not).
   //__atomic_signal_fence orders the compiler only: it emits no instruction
   T *const v = __atomic_load_n(const_cast<T *const *>(pp), __ATOMIC_RELAXED);
   __atomic_signal_fence(__ATOMIC_ACQUIRE);
   return v;
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM64)
   //Plain load: the address dependency of the dereference orders it, so the
   //load-acquire (ldar) __ldar64 would emit is not needed
   T *const v = (T *)(std::size_t)__iso_volatile_load64
      ((const volatile __int64 *)(const void *)pp);
   BOOST_CONTAINER_ATOMIC_PTR_DISABLE_DEPRECATED
   _ReadWriteBarrier();
   BOOST_CONTAINER_ATOMIC_PTR_RESTORE_WARNING
   return v;
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM)
   T *const v = *(T *const volatile *)pp;
   __dmb(_ARM_BARRIER_ISH);
   return v;
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_X86)
   //_ReadWriteBarrier is MSVC's compiler-only fence, the exact counterpart
   //of __atomic_signal_fence above; x86/x64 needs nothing more
   T *const v = *(T *const volatile *)pp;
   BOOST_CONTAINER_ATOMIC_PTR_DISABLE_DEPRECATED
   _ReadWriteBarrier();
   BOOST_CONTAINER_ATOMIC_PTR_RESTORE_WARNING
   return v;
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_GNU_SYNC)
   //GCC 4.1-4.6: an empty asm with a memory clobber is the compiler fence
   T *const v = *(T *const volatile *)pp;
   __asm__ __volatile__("" ::: "memory");
   return v;
   #else
   return *(T *const volatile *)pp;
   #endif
}

//Publishes a pointer, ordering every preceding write before it
template<class T>
BOOST_CONTAINER_FORCEINLINE void atomic_ptr_write_rel(T **pp, T *value)
{
   #if defined(BOOST_CONTAINER_ATOMIC_PTR_GNU)
   __atomic_store_n(pp, value, __ATOMIC_RELEASE);
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM64)
   __stlr64( (unsigned __int64 volatile *)(void *)pp
           , (unsigned __int64)(std::size_t)value);
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM)
   __dmb(_ARM_BARRIER_ISH);
   *(T *volatile *)pp = value;
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_X86)
   BOOST_CONTAINER_ATOMIC_PTR_DISABLE_DEPRECATED
   _ReadWriteBarrier();
   BOOST_CONTAINER_ATOMIC_PTR_RESTORE_WARNING
   *(T *volatile *)pp = value;
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_GNU_SYNC)
   __sync_synchronize();
   *(T *volatile *)pp = value;
   #else
   *(T *volatile *)pp = value;
   #endif
}

//Reads a pointer published with atomic_ptr_write_rel, ordering every
//access that follows, whether or not it goes through the pointer.
//
//This is the primitive to reach for when a *documented* synchronizes-with
//guarantee is being offered to users, as pmr::get_default_resource() does:
//the writer may have published unrelated state before the pointer store, and
//a reader that observes the new pointer must see that state too.
//atomic_ptr_read_csm only orders what the pointer itself leads to, which
//is enough for this header's own bootstrap but not for that contract.
//
//The two coincide on x86/x64 (a plain load already has acquire semantics
//there, so only the compiler must be pinned); on ARM64 this costs a real
//ldar where the consume form gets away with a plain ldr.
template<class T>
BOOST_CONTAINER_FORCEINLINE T *atomic_ptr_read_acq(T *const *pp)
{
   #if defined(BOOST_CONTAINER_ATOMIC_PTR_GNU)
   return __atomic_load_n(pp, __ATOMIC_ACQUIRE);
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM64)
   return (T *)(std::size_t)__ldar64
      ((unsigned __int64 volatile *)(void *)const_cast<T **>(pp));
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM)
   T *const v = *(T *const volatile *)pp;
   __dmb(_ARM_BARRIER_ISH);
   return v;
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_X86)
   T *const v = *(T *const volatile *)pp;
   BOOST_CONTAINER_ATOMIC_PTR_DISABLE_DEPRECATED
   _ReadWriteBarrier();
   BOOST_CONTAINER_ATOMIC_PTR_RESTORE_WARNING
   return v;
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_GNU_SYNC)
   T *const v = *(T *const volatile *)pp;
   __sync_synchronize();
   return v;
   #else
   return *(T *const volatile *)pp;
   #endif
}

//Stores "value" and returns the previous pointer, atomically, ordering both
//sides (acquire on what the old pointer leads to, release on everything
//written before the store). Lets a slot be replaced without a lock while
//concurrent replacers still each get a distinct, coherent previous value.
template<class T>
BOOST_CONTAINER_FORCEINLINE T *atomic_ptr_exchange_acq_rel(T **pp, T *value)
{
   #if defined(BOOST_CONTAINER_ATOMIC_PTR_GNU)
   return __atomic_exchange_n(pp, value, __ATOMIC_ACQ_REL);
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM64) || \
         defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_ARM) || \
         defined(BOOST_CONTAINER_ATOMIC_PTR_MSVC_X86)
      //Interlocked operations are full barriers on every MSVC target:
      //stronger than the required acquire-release
   #  if defined(_WIN64)
   return (T *)(std::size_t)_InterlockedExchange64
      ( (__int64 volatile *)(void *)pp, (__int64)(std::size_t)value);
   #  else
   return (T *)(std::size_t)(unsigned long)_InterlockedExchange
      ( (long volatile *)(void *)pp, (long)(std::size_t)value);
   #  endif
   #elif defined(BOOST_CONTAINER_ATOMIC_PTR_GNU_SYNC)
      //GCC 4.1-4.6: __sync_lock_test_and_set is only an acquire barrier and
      //may be a plain store on some targets, so the exchange is spelled as a
      //full-barrier compare-and-swap loop
   T *old;
   do{
      old = *(T *volatile *)pp;
   }while(__sync_val_compare_and_swap(pp, old, value) != old);
   return old;
   #else
      //Single-threaded fallback
   T *const old = *(T *volatile *)pp;
   *(T *volatile *)pp = value;
   return old;
   #endif
}

}  //namespace dtl {
}  //namespace container {
}  //namespace boost {

//These only ever guarded the intrinsic calls above
#undef BOOST_CONTAINER_ATOMIC_PTR_DISABLE_DEPRECATED
#undef BOOST_CONTAINER_ATOMIC_PTR_RESTORE_WARNING

#endif   //BOOST_CONTAINER_DETAIL_ATOMIC_PTR_HPP
