//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Peter Dimov 2008.
// (C) Copyright Ion Gaztanaga 2005-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//
// A spin mutex with an adaptive back-off, ported from Boost.Interprocess
// (sync/spin/mutex.hpp and sync/spin/wait.hpp, the latter deriving in turn
// from boost/smart_ptr/detail/yield_k.hpp - many thanks to Peter Dimov).
//
// It exists because the spin lock dlmalloc ships with, and the one
// detail/mutex.hpp copied from it, back off by counting raw iterations and
// calling sched_yield()/SleepEx(50) every 64 of them. That has two problems:
// it never emits the processor's spin-loop hint, so on a simultaneous
// multithreading core the spinning thread keeps the execution resources the
// lock holder needs to make progress; and it jumps straight from a tight loop
// to a syscall, which is far too abrupt for the very short critical sections
// an allocator has.
//
// The strategy here is the Interprocess one, in four phases:
//    1. a few plain re-reads, for a lock that is about to be released,
//    2. re-reads separated by the architecture's pause/yield hint,
//    3. thread_yield(), giving the core to a runnable thread,
//    4. sleeping for a scheduler tick, for a lock held a long time.
//
// Deviations from the Interprocess original, both to keep this header free of
// the os_thread_functions.hpp machinery:
//    - the switch from yielding to sleeping is a fixed number of yields
//      rather than a measured system tick. Interprocess yields for one tick's
//      worth of time and sleeps afterwards; here the budget is expressed in
//      iterations, which needs no clock.
//    - the core count is only used for the "single core, do not spin at all"
//      decision, and platforms where it cannot be obtained cheaply assume more
//      than one core (i.e. they spin), which is the common case.
//
// Two types are provided:
//    - spin_mutex_t, a POD holding one 32-bit word, plus free functions. Zero
//      is the unlocked state, so zero-initialized memory is a valid unlocked
//      mutex. This is the type dlmalloc uses through MLOCK_T, where it has to
//      live inside C structs.
//    - spin_mutex, the ordinary C++ class wrapping it.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_SPIN_MUTEX_HPP
#define BOOST_CONTAINER_DETAIL_SPIN_MUTEX_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/cstdint.hpp>

//////////////////////////////////////////////////////////////////////////////
//
//                   BOOST_CONTAINER_SMT_PAUSE
//
// Emits the processor hint that marks a spin loop. It saves power and, on
// simultaneous multithreading processors, hands the shared execution resources
// over to the sibling hardware threads, which is what lets the thread holding
// the lock make progress.
//
// Always defined: on processors with no such hint it expands to nothing, so
// spin_wait needs no conditional code.
//
//////////////////////////////////////////////////////////////////////////////

//Detect the portable x86 pause builtin (Clang, GCC 10 and later). Excluded on
//MSVC ARM targets, where the x86 spellings of the architecture macros are also
//defined but the builtin is not available.
#if defined(__has_builtin) && !defined(_M_ARM64EC) && !defined(_M_ARM64) && !defined(_M_ARM)
#  if __has_builtin(__builtin_ia32_pause) && !defined(__INTEL_COMPILER)
#     define BOOST_CONTAINER_HAS_BUILTIN_IA32_PAUSE
#  endif
#endif

//Forward declaration of MSVC intrinsics
//Note: ARM64EC also defines _M_AMD64/_M_X64, so it must be tested first
#if defined(_MSC_VER)
#if defined(_M_ARM64EC) || defined(_M_ARM64) || defined(_M_ARM)
extern "C" void __yield(void);
#if defined(BOOST_MSVC)
#pragma intrinsic(__yield)
#endif
#elif defined(_M_AMD64) || defined(_M_IX86) || defined(_M_X64)
extern "C" void _mm_pause(void);
#if defined(BOOST_MSVC)
#pragma intrinsic(_mm_pause)
#endif
#endif
#endif

#if defined(BOOST_CONTAINER_HAS_BUILTIN_IA32_PAUSE)

//x86/x86-64 PAUSE, without inline assembly
#define BOOST_CONTAINER_SMT_PAUSE   __builtin_ia32_pause();

#elif defined(_MSC_VER) && ( defined(_M_ARM64EC) || defined(_M_ARM64) || defined(_M_ARM) )

#define BOOST_CONTAINER_SMT_PAUSE __yield();

#elif defined(_MSC_VER) && ( defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64) )

#define BOOST_CONTAINER_SMT_PAUSE _mm_pause();

#elif defined(__GNUC__) && ( defined(__i386__) || defined(__x86_64__) ) && !defined(_CRAYC)

#define BOOST_CONTAINER_SMT_PAUSE   __asm__ __volatile__("rep; nop" : : : "memory");

#elif defined(__GNUC__) &&\
      (  defined(__aarch64__) || defined(__ARM_ARCH_8A__)\
      || (defined(__ARM_ARCH) && __ARM_ARCH >= 7)\
      || defined(__ARM_ARCH_7__)   || defined(__ARM_ARCH_7A__)  || defined(__ARM_ARCH_7R__)\
      || defined(__ARM_ARCH_7M__)  || defined(__ARM_ARCH_7EM__) || defined(__ARM_ARCH_7S__)\
      || defined(__ARM_ARCH_6K__)  || defined(__ARM_ARCH_6KZ__) || defined(__ARM_ARCH_6ZK__) )

//YIELD, available on AArch64 and on 32 bit ARM since ARMv6K/ARMv7. Older ARM
//processors have no such hint, and the instruction does not even assemble.
#define BOOST_CONTAINER_SMT_PAUSE   __asm__ __volatile__("yield" : : : "memory");

#elif defined(__GNUC__) &&\
      ( defined(__powerpc__) || defined(__powerpc64__) || defined(__ppc__)\
     || defined(__ppc64__)   || defined(__PPC__)       || defined(__PPC64__) || defined(_ARCH_PPC) )

//Drop this thread's program priority to low while spinning and restore it to
//medium right afterwards, which is how a POWER processor is told to give its
//shared resources to the sibling threads. Both are "or rX,rX,rX" forms, which
//are plain no-ops on processors that don't implement the priority hints, so
//they are safe everywhere.
#define BOOST_CONTAINER_SMT_PAUSE   __asm__ __volatile__("or 1,1,1\n\tor 2,2,2" : : : "memory");

#elif defined(__GNUC__) && defined(__riscv)

//PAUSE (Zihintpause extension). It's encoded in the FENCE space and defined as
//a no-op on processors that don't implement it, so it's always safe to emit.
//It's written as an encoding rather than as the "pause" mnemonic because
//assemblers without Zihintpause support reject the mnemonic, notably on 32 bit
//RISC-V.
#define BOOST_CONTAINER_SMT_PAUSE   __asm__ __volatile__(".insn i 0x0F, 0, x0, x0, 0x010" : : : "memory");

#endif

#if !defined(BOOST_CONTAINER_SMT_PAUSE)

//No spin loop hint is known for this processor.
#define BOOST_CONTAINER_SMT_PAUSE

#endif

//////////////////////////////////////////////////////////////////////////////
//
//    Backend selection for the atomics and for yield/sleep
//
//////////////////////////////////////////////////////////////////////////////

//This mutex is always a real lock. There is no single-threaded degradation
//and no way to ask for one: it guards, among other things, the dlmalloc heap,
//which is process-wide and shared between the modules of a process, so a
//variant that locks nothing would be a silent data race rather than an
//optimization. A compiler with no atomic operation at all cannot build
//Boost.Container.
#if defined(__GNUC__) && defined(__ATOMIC_ACQUIRE)
#  define BOOST_CONTAINER_SPIN_MUTEX_GNU        //__atomic builtins (GCC 4.7+, Clang)
#elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1))
#  define BOOST_CONTAINER_SPIN_MUTEX_GNU_SYNC   //legacy __sync builtins
#elif defined(_MSC_VER)
#  define BOOST_CONTAINER_SPIN_MUTEX_MSVC
#else
#  error "Boost.Container: no atomic backend is known for this compiler"
#endif

#if defined(BOOST_CONTAINER_SPIN_MUTEX_MSVC)
extern "C" long __cdecl _InterlockedCompareExchange(long volatile *, long, long);
extern "C" long __cdecl _InterlockedExchange(long volatile *, long);
#  if defined(BOOST_MSVC)
#     pragma intrinsic(_InterlockedCompareExchange)
#     pragma intrinsic(_InterlockedExchange)
#  endif
#endif

//Yield/sleep primitives, declared rather than pulled from <windows.h>, in the
//same style detail/mutex.hpp and detail/intermodule_globals.hpp already use.
#if !defined(__CYGWIN__) && (defined(_WIN32) || defined(__WIN32__) || defined(WIN32))
#  define BOOST_CONTAINER_SPIN_MUTEX_WINDOWS
#  if defined(BOOST_USE_WINDOWS_H)
#     include <windows.h>
#  else
#     if defined(_M_IX86) || defined(__i386__)
#        define BOOST_CONTAINER_SPIN_STDCALL __stdcall
#     else
#        define BOOST_CONTAINER_SPIN_STDCALL
#     endif
extern "C" {
__declspec(dllimport) int BOOST_CONTAINER_SPIN_STDCALL SwitchToThread(void);
__declspec(dllimport) void BOOST_CONTAINER_SPIN_STDCALL Sleep(unsigned long dwMilliseconds);
}  //extern "C"
#  endif
#else
#  define BOOST_CONTAINER_SPIN_MUTEX_POSIX
#  include <sched.h>     //sched_yield
#  include <time.h>      //nanosleep
#  if defined(BOOST_HAS_UNISTD_H)
#     include <unistd.h> //sysconf
#  endif
#endif

namespace boost {
namespace container {
namespace dtl {

//////////////////////////////////////////////////////////////////////////////
//
//    Atomic operations on a 32 bit word
//
//////////////////////////////////////////////////////////////////////////////

//Compare-and-swap with acquire ordering on success: writes "with" when the
//word holds "cmp" and returns the value the word had beforehand, so a return
//equal to "cmp" means this thread performed the swap.
inline boost::uint32_t atomic_cas32_acquire
   (volatile boost::uint32_t *mem, boost::uint32_t with, boost::uint32_t cmp)
{
   #if defined(BOOST_CONTAINER_SPIN_MUTEX_GNU)
   boost::uint32_t expected = cmp;
   __atomic_compare_exchange_n( const_cast<boost::uint32_t*>(mem), &expected, with
                              , /*weak*/false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE);
   return expected;
   #elif defined(BOOST_CONTAINER_SPIN_MUTEX_GNU_SYNC)
   //__sync_val_compare_and_swap is a full barrier, which subsumes acquire
   return __sync_val_compare_and_swap(const_cast<boost::uint32_t*>(mem), cmp, with);
   #else //BOOST_CONTAINER_SPIN_MUTEX_MSVC
   return boost::uint32_t(_InterlockedCompareExchange
      (reinterpret_cast<long volatile*>(mem), long(with), long(cmp)));
   #endif
}

//Publishes zero (or any value) with release ordering, so that everything done
//before it is visible to whoever observes the new value.
BOOST_CONTAINER_FORCEINLINE void atomic_write32_release(volatile boost::uint32_t *mem, boost::uint32_t val)
{
   #if defined(BOOST_CONTAINER_SPIN_MUTEX_GNU)
   __atomic_store_n(const_cast<boost::uint32_t*>(mem), val, __ATOMIC_RELEASE);
   #elif defined(BOOST_CONTAINER_SPIN_MUTEX_GNU_SYNC)
   __sync_synchronize();
   *mem = val;
   #else //BOOST_CONTAINER_SPIN_MUTEX_MSVC
   //A locked exchange is a full barrier, so it is a release store too
   (void)_InterlockedExchange(reinterpret_cast<long volatile*>(mem), long(val));
   #endif
}

//Unconditional exchange with acquire ordering: writes "with" and returns the
//previous value. For a word that is only ever 0 or 1 this is all a mutex
//needs, and on x86 it is a plain LOCK XCHG where a compare-and-swap would be
//a LOCK CMPXCHG, which measurably costs more on the uncontended path every
//program takes.
BOOST_CONTAINER_FORCEINLINE boost::uint32_t atomic_xchg32_acquire
   (volatile boost::uint32_t *mem, boost::uint32_t with)
{
   #if defined(BOOST_CONTAINER_SPIN_MUTEX_GNU)
   return __atomic_exchange_n(const_cast<boost::uint32_t*>(mem), with, __ATOMIC_ACQUIRE);
   #elif defined(BOOST_CONTAINER_SPIN_MUTEX_GNU_SYNC)
   //__sync_lock_test_and_set is defined as an acquire barrier
   return __sync_lock_test_and_set(const_cast<boost::uint32_t*>(mem), with);
   #else //BOOST_CONTAINER_SPIN_MUTEX_MSVC
   return boost::uint32_t(_InterlockedExchange
      (reinterpret_cast<long volatile*>(mem), long(with)));
   #endif
}

//Plain read, used only to re-check a taken lock while spinning: the value is
//confirmed by the exchange that follows, so no ordering is needed. On the
//__atomic tier it is spelled as a relaxed atomic load: the generated code is
//the same plain MOV/LDR, but a volatile read would formally be a data race
//with the locked exchange, and ThreadSanitizer reports every contended spin
//(seen in CI on dlmalloc_memalign_test's threaded hammer).
BOOST_CONTAINER_FORCEINLINE boost::uint32_t atomic_read32_relaxed(const volatile boost::uint32_t *mem)
{
   #if defined(BOOST_CONTAINER_SPIN_MUTEX_GNU)
   return __atomic_load_n(mem, __ATOMIC_RELAXED);
   #else
   return *mem;
   #endif
}

//////////////////////////////////////////////////////////////////////////////
//
//    Yield / sleep / core count
//
//////////////////////////////////////////////////////////////////////////////

inline void spin_thread_yield()
{
   #if defined(BOOST_CONTAINER_SPIN_MUTEX_WINDOWS)
   (void)SwitchToThread();
   #elif defined(BOOST_CONTAINER_SPIN_MUTEX_POSIX)
   (void)::sched_yield();
   #endif
}

//Sleeps for about one scheduler tick, which is the granularity the OS can
//actually honour: asking for less just returns later anyway.
inline void spin_thread_sleep_tick()
{
   #if defined(BOOST_CONTAINER_SPIN_MUTEX_WINDOWS)
   Sleep(1);
   #elif defined(BOOST_CONTAINER_SPIN_MUTEX_POSIX)
   struct timespec ts;
   ts.tv_sec  = 0;
   ts.tv_nsec = 1000000;   //1 ms
   (void)::nanosleep(&ts, 0);
   #endif
}

//Only the "is there more than one core" answer matters: on a single core
//spinning can never succeed, because the thread holding the lock is not
//running. Platforms where the count is not cheaply available answer "many",
//which is the common case and only costs a few wasted spins if wrong.
inline bool spin_multicore()
{
   #if defined(BOOST_CONTAINER_SPIN_MUTEX_POSIX) && defined(_SC_NPROCESSORS_ONLN)
   static const long cores = ::sysconf(_SC_NPROCESSORS_ONLN);
   return cores != 1;
   #else
   return true;
   #endif
}

//////////////////////////////////////////////////////////////////////////////
//
//    spin_wait: the back-off strategy
//
//////////////////////////////////////////////////////////////////////////////

class spin_wait
{
   public:
   //Phase boundaries, in iterations of yield().
   static const unsigned int nop_limit         = 8u;
   static const unsigned int nop_pause_limit   = 32u;
   //How many times to hand the core over before starting to sleep. Interprocess
   //measures one system tick instead; see the note at the top of this file.
   static const unsigned int yield_limit       = nop_pause_limit + 16u;

   spin_wait()
      : m_k(0u)
   {}

   unsigned int count() const
   {  return m_k;  }

   void yield()
   {
      //Lazy initialization: on a single core, skip straight to yielding, since
      //no amount of spinning can free the lock
      if(!m_k && !spin_multicore()){
         m_k = nop_pause_limit;
      }

      if(m_k < nop_limit){
         //Plain re-reads: the lock is very likely about to be released
      }
      else if(m_k < nop_pause_limit){
         BOOST_CONTAINER_SMT_PAUSE
      }
      else if(m_k < yield_limit){
         spin_thread_yield();
      }
      else{
         spin_thread_sleep_tick();
      }
      ++m_k;
   }

   void reset()
   {  m_k = 0u;  }

   private:
   unsigned int m_k;
};

//////////////////////////////////////////////////////////////////////////////
//
//    spin_mutex_t: a POD mutex, for use inside C structures
//
//////////////////////////////////////////////////////////////////////////////

//Zero is the unlocked state, so zero-initialized storage is an unlocked mutex.
struct spin_mutex_t
{
   volatile boost::uint32_t m_s;
};

BOOST_CONTAINER_FORCEINLINE void spin_mutex_init(spin_mutex_t *m)
{  m->m_s = 0u;  }

//Returns true when this thread took the mutex.
BOOST_CONTAINER_FORCEINLINE bool spin_mutex_try_lock(spin_mutex_t *m)
{
   //Taking the mutex must be an acquire, so that everything the previous owner
   //did before releasing it is visible here. Nothing has to be ordered when the
   //mutex is already taken and the swap fails: writing 1 over a 1 changes
   //nothing, which is why an unconditional exchange is enough here and the
   //compare-and-swap Interprocess uses is not needed.
   const boost::uint32_t prev = atomic_xchg32_acquire(&m->m_s, 1u);
   //A zero previous value means this thread performed the swap and owns the
   //mutex. Re-reading m_s would add nothing: no other thread can change it
   //while it is owned here.
   return prev == 0u;
}

//The back-off loop is kept out of line so that taking a free lock is just the
//exchange and a branch, which is the shape of dlmalloc's own
//"CAS_LOCK(sl)? spin_acquire_lock(sl) : 0" macro. Inlining the spin loop into
//every call site measurably costs the uncontended path, which every program
//pays whether or not it has threads.
//A function template rather than a plain function: templates already have
//vague linkage, so this needs no "inline" keyword - and EDG-based front ends
//(nvc++) warn when a routine is marked both "inline" and "noinline".
template<int Dummy>
BOOST_NOINLINE void spin_mutex_lock_slow(spin_mutex_t *m)
{
   spin_wait swait;
   do{
      //Re-read before trying again: an exchange on a word another core owns
      //costs an exclusive cache line each time, while a read only needs a
      //shared one. Only attempt the swap once the lock looks free.
      while(atomic_read32_relaxed(&m->m_s) != 0u){
         swait.yield();
      }
   } while(!spin_mutex_try_lock(m));
}

BOOST_CONTAINER_FORCEINLINE void spin_mutex_lock(spin_mutex_t *m)
{
   if(!spin_mutex_try_lock(m))
      spin_mutex_lock_slow<0>(m);
}

BOOST_CONTAINER_FORCEINLINE void spin_mutex_unlock(spin_mutex_t *m)
{
   //Only the owner unlocks, and no other thread can change m_s while it is
   //owned, so nothing has to be compared: a store is enough. It must be a
   //release, so that everything done inside the critical section is visible to
   //the next thread that takes the mutex.
   atomic_write32_release(&m->m_s, 0u);
}

//////////////////////////////////////////////////////////////////////////////
//
//    spin_mutex: the C++ wrapper
//
//////////////////////////////////////////////////////////////////////////////

class spin_mutex
{
   spin_mutex(const spin_mutex &);
   spin_mutex &operator=(const spin_mutex &);

   public:
   spin_mutex()
   {  spin_mutex_init(&m_lock);  }

   void lock()
   {  spin_mutex_lock(&m_lock);  }

   bool try_lock()
   {  return spin_mutex_try_lock(&m_lock);  }

   void unlock()
   {  spin_mutex_unlock(&m_lock);  }

   private:
   spin_mutex_t m_lock;
};

}  //namespace dtl {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_DETAIL_SPIN_MUTEX_HPP
