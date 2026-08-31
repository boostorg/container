//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//
// Process-wide globals for a header-only Boost.Container.
//
// intermodule_globals<T, Options>() returns a reference to a single,
// process-wide object of type T, shared by every module (EXE/DLL/shared
// object) that instantiates the same T/Options pair, even when each module
// inlines its own copy of this code.
//
// T is created/destroyed calling the default constructor and the destructor,
// on zero-initialized storage. A per-module registrar (a static
// object instantiated together with the template) attaches during static
// initialization, so the object is constructed before main(), and the
// destructor runs when the last attached module detaches during static
// destruction (after main(), or at DLL unload). Lazy attachment from get()
// additionally covers uses from other objects' static constructors (static
// initialization across translation units is unordered), and a late get()
// after destruction phoenix-reconstructs the object.
//
// T requirements: default-constructible with a constructor that does not
// throw, and destructible. On ELF, T must be declared BOOST_SYMBOL_VISIBLE,
// and so must Options unless it is void (see note below). Over-aligned types are supported.:
//
// Identity: the instance is keyed on the T/Options *pair*, so two Options
// types over the same T name two different objects, and the same pair names
// the same object in every module. The key is derived from the two type
// names, which means it agrees between modules built by the same compiler
// regardless of whether each of them was built with RTTI enabled. Types
// that must rendezvous across modules therefore have to be the very same
// types, i.e. come from a shared header, and never from an anonymous
// namespace.
//
// Options requirements. Options defaults to void, which is the empty option
// set: every member below then takes its default. Every member is optional
// and falls back to the value in intermodule_options_defaults:
//    static const bool destroy_at_exit;  // default true. When false the
//                                        // object is intentionally
//                                        // immortal and its destructor is
//                                        // never run (the OS reclaims it
//                                        // at process exit): required for
//                                        // state that atexit-time code may
//                                        // still use, such as the dlmalloc
//                                        // heap
//    static const bool pin_constructing_module;
//                                        // default false. True when T
//                                        // stores pointers into its own
//                                        // module's image (vtables,
//                                        // function pointers), so that
//                                        // module must never be unloaded
//                                        // while another one can still
//                                        // reach the object
//
//    struct my_options
//    {
//       static const bool destroy_at_exit = false;   //other options default
//    };
//
// The Windows backend requires the ability to create/open a pagefile-backed
// section; processes that deny this (e.g. some restrictive AppContainer
// sandboxes) are not supported and BOOST_ASSERT will fire.
//
// Locking: the hot get() path is lock-free; only attach/detach lock, twice
// per module lifetime.
//
// The availability of the object is signalled through the cached object pointer
// so the fast path needs no acquire fence, only "consume" semantics after a
// "release" fence.
//
// Backends:
// 
// ---------
//  Windows 
// ---------
//    One pagefile-backed section, named after the ABI tag and the
//    current pid, holds every singleton of the process together with the
//    linear-hashed index used to find them. The bootstrap lock lives in the
//    sectionheader, so no named mutex (nor any other shared object) is needed.
// 
//    Named kernel objects are refcounted, associated to the pid, and disappear with
//    the process, so never collide with a dead process.
// 
//    Each module may map the section at a different address, which is why
//    the first one publishes its view as "canonical" and the others adopt it.
// 
//    On Windows we have to be careful with data races:
//   * The EXE's static constructors run in the CRT startup, after loader
//     initialization has completed and released the lock. An EXE registrar
//     attaching there can call LoadLibrary on any pre-main thread (created
//     by an implicitly loaded DLL's DllMain, an injected DLL, a thread
//     pool): both can find no registry entry and each create one, one
//     silently overwriting the other - two heaps, and memory allocated
//     against one freed against the other.
//   * Static destruction is CRT atexit processing, not a loader callback.
//     An EXE detach() decrementing the refcount races a FreeLibrary-driven
//     detach() on another thread; the non-atomic decrement can lose an update
//     and the object is never destroyed.
//   * A lazy attach() from get() (the static-init-order safety net, and the
//     phoenix path after destruction) can run on any thread at any time.

// ------------
//  ELF/Mach-O
// ------------
//    The Minimal Visibility Rule is the mechanism used by C++ compilers
//    (like GCC and Clang): the final visibility of a template
//    instantiation (e.g., holder<T>) is determined by the most restrictive (most hidden)
//    visibility attribute among the template itself and all of its template arguments.
// 
//    A default-visibility (BOOST_SYMBOL_VISIBLE) class
//    template static data member is used and the dynamic loader unifies the
//    definition across shared objects (e.g. GCC emits STB_GNU_UNIQUE, which
//    also makes the defining shared object non-unloadable).
//    The unified registrar behaves as one process-wide global object.
// 
// 
//  - Fallback (BOOST_CONTAINER_NO_INTERMODULE_GLOBALS, WinCE/UWP, or no
//    usable atomic primitives on Windows): a per-module static - semantics
//    then match classic static linking.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#ifndef BOOST_CONTAINER_DETAIL_INTERMODULE_GLOBALS_HPP
#define BOOST_CONTAINER_DETAIL_INTERMODULE_GLOBALS_HPP

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/detail/spin_mutex.hpp>   //dtl::spin_mutex
#include <boost/container/detail/type_traits.hpp>   //alignment_of, aligned_storage
#include <boost/container/detail/placement_new.hpp>
#include <boost/container/detail/atomic_ptr.hpp>   //atomic_ptr_read_csm & friends
#include <boost/assert.hpp>

#include <cstddef>   //std::size_t
#include <cstring>   //std::memset (phoenix re-zeroing)

//Backend selection
#if defined(BOOST_CONTAINER_NO_INTERMODULE_GLOBALS)
#  define BOOST_CONTAINER_INTERMODULE_BACKEND_LOCAL
#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__)
      //Cygwin is a POSIX emulation layer, but it produces ordinary PE/COFF
      //modules: it has no ELF symbol unification, so the VISIBLE backend
      //silently degrades to per-module globals there. The Win32 rendezvous
      //(named kernel objects + process heap) is available and is the only
      //mechanism that actually works, so Cygwin takes the Windows path.
#  if defined(_WIN32_WCE) || defined(BOOST_PLAT_WINDOWS_UWP)
      //No classic named file mappings (or no atomic primitives): degrade to per-module
#     define BOOST_CONTAINER_INTERMODULE_BACKEND_LOCAL
#  else
#     define BOOST_CONTAINER_INTERMODULE_BACKEND_WINAPI
#  endif
#else
#  define BOOST_CONTAINER_INTERMODULE_BACKEND_VISIBLE
#endif

/* Detecting Minimal Visibility Rule.

   Since the visibility of the storage instantiation is the minimum over the
   template and both of its arguments, a T or an Options that is not
   default-visible gives every module a private copy: it compiles/links/runs
   without error, and no warning exists so the mistake cannot be diagnosed.

   GCC can at least be asked whether a type carries the attribute with limitations:

     * It must be applied once per concrete instantiation.
     * it reports the *explicit* attribute only, so it cannot observe
       -fvisibility=hidden acting on an unmarked type.
     * evaluating it emits -Wattributes even when the answer is yes, so that
       diagnostic is suppressed for the length of the check.

   Clang has no equivalent builtin. */
#if defined(BOOST_CONTAINER_INTERMODULE_BACKEND_VISIBLE) && \
    defined(BOOST_CONTAINER_GCC_PRAGMAS) && \
    BOOST_CONTAINER_HAS_BUILTIN(__builtin_has_attribute)
#  define BOOST_CONTAINER_INTERMODULE_ASSERT_VISIBLE(T, MSG)                   \
      _Pragma("GCC diagnostic push")                                           \
      _Pragma("GCC diagnostic ignored \"-Wattributes\"")                       \
      BOOST_CONTAINER_STATIC_ASSERT_MSG                                        \
         (__builtin_has_attribute(T, visibility("default")), MSG);             \
      _Pragma("GCC diagnostic pop")                                            \
      typedef int boost_container_intermodule_visibility_checked_t
#else
#  define BOOST_CONTAINER_INTERMODULE_ASSERT_VISIBLE(T, MSG)                   \
      typedef int boost_container_intermodule_visibility_checked_t
#endif

namespace boost {
namespace container {
namespace dtl {


//////////////////////////////////////////////////////////////////////////////
//
//    Options
//
//////////////////////////////////////////////////////////////////////////////

//Values used for every option the Options type does not define
struct intermodule_options_defaults
{
   static const bool destroy_at_exit = true;
   static const bool pin_constructing_module = false;
};

//Defines intermodule_opt_<NAME><Options>::value: Options::<NAME> when that
//member exists, intermodule_options_defaults::<NAME> otherwise. Detection
//uses the classic C++03 "inherit from both and see if the name becomes
//ambiguous" trick; the value is only ever read in a constant expression, so
//no out-of-line definition of the member is required.
#define BOOST_CONTAINER_INTERMODULE_BOOL_OPTION(NAME)                          \
template<class Options, bool Has>                                              \
struct intermodule_optval_##NAME                                               \
{  static const bool value = intermodule_options_defaults::NAME;  };           \
                                                                               \
template<class Options>                                                        \
struct intermodule_optval_##NAME<Options, true>                                \
{  static const bool value = Options::NAME;  };                                \
                                                                               \
template<class Options>                                                        \
struct intermodule_opt_##NAME                                                  \
{                                                                              \
   struct fallback                                                             \
   {  static const bool NAME = intermodule_options_defaults::NAME;  };         \
   struct probe : Options, fallback {};                                        \
                                                                               \
   template<bool> struct bool_arg {};                                          \
   typedef char one_type;                                                      \
   struct two_type { char dummy[2]; };                                         \
                                                                               \
   /*Viable only while the name is unambiguous, i.e. Options lacks it*/        \
   template<class U> static one_type test(bool_arg<U::NAME> *);                \
   template<class U> static two_type test(...);                                \
                                                                               \
   static const bool has = sizeof(test<probe>(0)) == sizeof(two_type);         \
   static const bool value = intermodule_optval_##NAME<Options, has>::value;   \
};                                                                             \
                                                                               \
/*void is the empty option set: it can't be inherited from, and it defines*/   \
/*nothing, so answer straight from the defaults*/                              \
template<>                                                                     \
struct intermodule_opt_##NAME<void>                                            \
{                                                                              \
   static const bool has = false;                                              \
   static const bool value = intermodule_options_defaults::NAME;               \
};                                                                             \
/**/

BOOST_CONTAINER_INTERMODULE_BOOL_OPTION(destroy_at_exit)
BOOST_CONTAINER_INTERMODULE_BOOL_OPTION(pin_constructing_module)

}  //namespace dtl {
}  //namespace container {
}  //namespace boost {

//////////////////////////////////////////////////////////////////////////////
//
//    Windows backend
//
//////////////////////////////////////////////////////////////////////////////

#if defined(BOOST_CONTAINER_INTERMODULE_BACKEND_WINAPI)

#ifdef BOOST_USE_WINDOWS_H
#  include <windows.h>
#else

//Forward declarations of Windows struct tags so that our hand-written
//prototypes are token-for-token compatible with <windows.h> if some other
//header includes it in the same translation unit.
struct _SECURITY_ATTRIBUTES;
struct HINSTANCE__;

namespace boost {
namespace container_intermodule_winapi {

//SIZE_T/ULONG_PTR. The exact SPELLING matters here, not just the width:
//these are extern "C" declarations of entry points Boost.WinAPI also
//declares (boost/winapi/basic_types.hpp), and one translation unit can see
//both, so a mismatch is a conflicting declaration of the same function - not
//merely a different-but-compatible type. On 32-bit Windows SIZE_T is
//"unsigned long" while std::size_t is "unsigned int": same width, different
//type. Follow the rule Boost.WinAPI uses, which is also the SDK's.
#if defined(_WIN64)
#  if defined(BOOST_HAS_MS_INT64)
typedef unsigned __int64   uintptr_type;
#  else
typedef unsigned long long uintptr_type;
#  endif
#else
   //ILP32 Windows and LP64 Cygwin alike: ULONG_PTR is "unsigned long" on
   //both, since Cygwin does not define _WIN64
typedef unsigned long      uintptr_type;
#endif

//DWORD is always 32 bits. The Windows headers spell it "unsigned long"
//because "long" is 32 bits on LLP64 (MSVC, MinGW), but Cygwin targets LP64,
//where "long" is 64 bits and the correct spelling is "unsigned int". This is
//the same distinction Cygwin's own w32api draws with its __LONG32 macro.
#if defined(__LP64__) && !defined(_WIN64)
typedef unsigned int   dword_type;
#else
typedef unsigned long  dword_type;
#endif
typedef void *         handle_type;
typedef int            bool_type;

}  //namespace container_intermodule_winapi {
}  //namespace boost {

extern "C" {

__declspec(dllimport) boost::container_intermodule_winapi::bool_type __stdcall
   CloseHandle(void *hObject);

__declspec(dllimport) void * __stdcall
   CreateFileMappingA
      ( void *hFile
      , ::_SECURITY_ATTRIBUTES *lpAttributes
      , boost::container_intermodule_winapi::dword_type flProtect
      , boost::container_intermodule_winapi::dword_type dwMaximumSizeHigh
      , boost::container_intermodule_winapi::dword_type dwMaximumSizeLow
      , const char *lpName);

__declspec(dllimport) void * __stdcall
   MapViewOfFile
      ( void *hFileMappingObject
      , boost::container_intermodule_winapi::dword_type dwDesiredAccess
      , boost::container_intermodule_winapi::dword_type dwFileOffsetHigh
      , boost::container_intermodule_winapi::dword_type dwFileOffsetLow
      , boost::container_intermodule_winapi::uintptr_type dwNumberOfBytesToMap);

__declspec(dllimport) boost::container_intermodule_winapi::bool_type __stdcall
   UnmapViewOfFile(const void *lpBaseAddress);

__declspec(dllimport) boost::container_intermodule_winapi::dword_type __stdcall
   GetCurrentProcessId(void);

__declspec(dllimport) boost::container_intermodule_winapi::bool_type __stdcall
   GetModuleHandleExA
      ( boost::container_intermodule_winapi::dword_type dwFlags
      , const char *lpModuleName
      , ::HINSTANCE__ **phModule);

}  //extern "C"

#endif   //BOOST_USE_WINDOWS_H

namespace boost {
namespace container_intermodule_winapi {

#ifdef BOOST_USE_WINDOWS_H
typedef ::HANDLE       handle_type;
typedef ::DWORD        dword_type;
typedef ::SIZE_T       uintptr_type;
typedef ::BOOL         bool_type;
typedef ::HMODULE      module_handle_type;
#else
typedef ::HINSTANCE__ *module_handle_type;
#endif

static const dword_type page_readwrite_c        = 0x04;
static const dword_type file_map_read_write_c   = 0x02 | 0x04;   //FILE_MAP_WRITE|READ
static const dword_type module_handle_pin_c     = 0x01;  //GET_MODULE_HANDLE_EX_FLAG_PIN
static const dword_type module_from_address_c   = 0x04;  //GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS

BOOST_CONTAINER_FORCEINLINE void *invalid_handle()
{  return (void *)(~uintptr_type(0));  }   //INVALID_HANDLE_VALUE

}  //namespace container_intermodule_winapi {
}  //namespace boost {

namespace boost {
namespace container {
namespace dtl {

//Prevents the current module from ever being unloaded. Must be called by a
//module that installs code pointers (e.g. vtables of objects it constructs)
//into intermodule-shared state, so those pointers stay valid while any other
//module might still use them.
inline void intermodule_pin_this_module()
{
   //Address of a per-module datum: static data is never COMDAT-folded
   //across DLLs, so this always names the calling module.
   static int this_module_anchor = 0;
   static volatile int pinned = 0;
   if(!pinned){   //benign race: pinning is idempotent
      container_intermodule_winapi::module_handle_type h = 0;
      (void)GetModuleHandleExA
         ( container_intermodule_winapi::module_handle_pin_c
         | container_intermodule_winapi::module_from_address_c
         , (const char *)(const void *)&this_module_anchor
         , &h);
      pinned = 1;
   }
}

//Decimal/hex formatting without any CRT dependency
inline char *intermodule_append_dec(char *dst, unsigned long value)
{
   char tmp[24];
   unsigned n = 0;
   do{
      tmp[n++] = (char)('0' + (value % 10u));
      value /= 10u;
   }  while(value);
   while(n){
      *dst++ = tmp[--n];
   }
   return dst;
}

inline char *intermodule_append_str(char *dst, const char *s)
{
   while(*s){
      *dst++ = *s++;
   }
   return dst;
}

//Bump when the rendezvous protocol or block layout changes
#define BOOST_CONTAINER_INTERMODULE_ABI_VERSION 3

//////////////////////////////////////////////////////////////////////////////
//
//    Mapped-registry storage
//
//////////////////////////////////////////////////////////////////////////////
//
// A single pagefile-backed section, named after the pid, holds every
// singleton of the process plus the index used to find them.
//
// Everything inside is addressed by 32-bit offsets from the start of the
// section, never by pointers, so the contents do not depend on where a
// module happens to map it (the canonical view below exists for the
// addresses handed out to callers, not for the registry itself):
//
//   0                                                     REGISTRY_BYTES
//   |                                                                  |
//   +----------+---------------------+---- free ----+------------------+
//   | header   | bucket[0 .. split)  |     gap      |     entries      |
//   +----------+---------------------+--------------+------------------+
//              |                     |              |
//        sizeof(header)         bucket_end       arena_top
//                    buckets grow -->        <-- entries grow
//
// Buckets are appended forward, one slot per split; entries are bump
// allocated backward from the end. The registry is full exactly when the
// two frontiers would cross, which is reported rather than overrun.
//
// Each bucket holds the offset of the first record of its chain, 0 meaning
// empty, and each record chains to the next through "next".
//
// One entry, exploded (lower offsets on the left). The object storage is
// allocated first, so it ends up above the record that describes it, and
// the key is stored inline right after the record:
//
//   arena_top
//   |
//   v
//   +--------------------------+------------+-----+-------------------+
//   | intermodule_registry_    | key bytes  | pad | object bytes      |
//   |   record:                |  + '\0'    |     |   sizeof(T)       |
//   |   next, hash,            |            |     |                   |
//   |   obj_offset, obj_size,  |            |     |                   |
//   |   key_len, refcount      |            |     |                   |
//   +--------------------------+------------+-----+-------------------+
//   ^                          ^                  ^
//   |                          |                  r->obj_offset:
//   record offset, as stored   key, never         aligned for T, and what
//   in a bucket or in "next"   truncated          callers get as
//                                                 canonical + obj_offset
//
// The pad is whatever the record's own alignment left over; the object is
// aligned for T, so an over-aligned T simply pushes the record further
// down. Nothing is ever freed: a destroyed entry keeps its record and its
// storage (refcount back to 0) so a module loaded again later reconstructs
// T in the very same bytes.
//
// Keeping the key inline and in full is what turns a hash match into a mere
// candidate: it is confirmed with a full comparison, so two keys colliding
// on 32 bits chain instead of aliasing one another's storage. obj_size is
// checked on a match too, which catches the likelier accident of two
// modules disagreeing about T's layout.
//
// Growth uses linear hashing (Litwin), the scheme Boost.Intrusive exposes
// as its incremental<> option: a normal table would have to reallocate and
// rehash everything, impossible here without relocating objects other
// modules already point at. Appending one bucket and rehashing only the
// chain at the split pointer keeps the average chain at ~1 while every
// record stays exactly where it was put.
//
// Because MapViewOfFile may return a different base in each module, the
// first module publishes its view in the header and later ones adopt that
// canonical base and drop their own bootstrap view: the objects therefore
// have exactly one address per process, which they must (dlmalloc's
// malloc_state stores interior pointers to itself).
//
// The section is process-lifetime: per-entry refcounts still run T's
// destructor when its last module detaches, but the canonical view is never
// unmapped. Tearing it down would invalidate the addresses recorded for any
// entry declared immortal (destroy_at_exit false), whose whole purpose is to
// outlive every module.

#ifndef BOOST_CONTAINER_INTERMODULE_REGISTRY_BYTES
#define BOOST_CONTAINER_INTERMODULE_REGISTRY_BYTES 65536u
#endif
//Bucket count the registry starts with, chosen by whichever module creates
//it. It survives only as the initial value of the header's split/bucket_cnt
//pair, which every other module reads from there, so modules built with
//different values still interoperate: the creator's value wins.
#ifndef BOOST_CONTAINER_INTERMODULE_REGISTRY_BUCKETS
#define BOOST_CONTAINER_INTERMODULE_REGISTRY_BUCKETS 8u   //must be a power of 2
#endif

typedef unsigned int intermodule_u32;
BOOST_CONTAINER_STATIC_ASSERT(sizeof(intermodule_u32) == 4u);

//////////////////////////////////////////////////////////////////////////////
//
//    Rendezvous key
//
//////////////////////////////////////////////////////////////////////////////
//
// The key is the signature of a function template instantiated on the
// T/Options pair, as the compiler itself spells it. It therefore names both
// types, so the identity of an instance is the T/Options pair - the very
// same identity the other backend gets for free, where the shared storage
// is a static member of intermodule_globals_impl<T, Options> and the linker
// unifies it under a name that mangles both arguments.
//
// The front end produces the signature while parsing: unlike
// typeid(T).name() it needs no runtime type information, so a module built
// with RTTI disabled and one built with RTTI enabled compute the very same
// key and do rendezvous together.
//
// Records carry their key inline and sized, so a long signature costs a few
// bytes of arena and nothing else - no truncation, and no hashing of the
// name into the identity (the 32-bit hash below only picks a bucket;
// identity is always the full string). Like typeid names, a signature is
// only guaranteed to agree between modules built by the same compiler,
// which is all this backend needs.

//The intrinsic has to name the template arguments, which __func__ and
//__FUNCTION__ do not: they name the function alone, and every instantiation
//would then share a single key. Take the two that do, directly rather than
//through BOOST_CURRENT_FUNCTION, whose remaining fallbacks are exactly
//those unusable spellings. Between them they cover every compiler that
//targets the platforms this backend is compiled for: __PRETTY_FUNCTION__ on
//the GCC-compatible front ends (GCC, Clang, clang-cl, MinGW, Intel and the
//EDG-based ones, which all define __GNUC__ or __clang__) and __FUNCSIG__ on
//MSVC and the front ends that emulate it.
#if defined(__GNUC__) || defined(__clang__)
#  define BOOST_CONTAINER_INTERMODULE_KEY_SIGNATURE __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
#  define BOOST_CONTAINER_INTERMODULE_KEY_SIGNATURE __FUNCSIG__
#endif

template<class T, class Options>
BOOST_CONTAINER_FORCEINLINE const char *intermodule_rendezvous_key()
{
   #if defined(BOOST_CONTAINER_INTERMODULE_KEY_SIGNATURE)
   return BOOST_CONTAINER_INTERMODULE_KEY_SIGNATURE;
   #else
   //Dependent, so this only fires if a key is actually needed
   BOOST_CONTAINER_STATIC_ASSERT_MSG(sizeof(T) == 0
      , "Boost.Container: no intrinsic naming the template arguments of the "
        "current function is known for this compiler, so intermodule globals "
        "cannot be keyed");
   return 0;
   #endif
}

struct intermodule_registry_header
{
   spin_mutex lock;                 //guards every field below
   intermodule_u32    magic;
   //Linear hashing state, in the two-variable form Boost.Intrusive uses:
   //bucket_cnt is the capacity of the round (always a power of two) and
   //split is both the split pointer and the number of buckets actually
   //materialized, since every index folds back into [0, split).
   intermodule_u32    bucket_cnt;
   intermodule_u32    split;
   intermodule_u32    entry_count;
   intermodule_u32    bucket_end;   //offset one past the last bucket
   intermodule_u32    arena_top;    //offset of the lowest record allocated
   void              *canonical;    //process-wide base every module adopts
};

struct intermodule_registry_record
{
   intermodule_u32 next;        //next record of the chain, 0 = end
   intermodule_u32 hash;
   intermodule_u32 obj_offset;
   intermodule_u32 obj_size;
   intermodule_u32 key_len;
   long            refcount;
   //char key[key_len+1] follows
};

BOOST_CONTAINER_CONSTANT_VAR intermodule_u32 intermodule_registry_magic = 0xB005C048u;

BOOST_CONTAINER_FORCEINLINE intermodule_u32 *intermodule_reg_buckets(intermodule_registry_header *h)
{  return (intermodule_u32 *)(void *)((char *)h + sizeof(intermodule_registry_header));  }

BOOST_CONTAINER_FORCEINLINE intermodule_registry_record *intermodule_reg_record
   (intermodule_registry_header *h, intermodule_u32 off)
{  return (intermodule_registry_record *)(void *)((char *)h + off);  }

BOOST_CONTAINER_FORCEINLINE char *intermodule_reg_key(intermodule_registry_record *r)
{  return (char *)(void *)(r + 1);  }

//FNV-1a over the key
inline intermodule_u32 intermodule_reg_hash(const char *s, std::size_t n)
{
   intermodule_u32 h = 2166136261u;
   for(std::size_t i = 0; i != n; ++i){
      h ^= (intermodule_u32)(unsigned char)s[i];
      h *= 16777619u;
   }
   return h;
}

//Linear hashing address function: buckets below the split pointer have
//already been rehashed for the next round, so they use the wider mask
inline intermodule_u32 intermodule_reg_bucket
   (const intermodule_registry_header *h, intermodule_u32 hash)
{
   intermodule_u32 idx = hash & (h->bucket_cnt - 1u);
   if(idx >= h->split){
      //Not materialized yet: it still lives in the bucket it will split from
      idx -= h->bucket_cnt >> 1;
   }
   return idx;
}

//Bump-allocates backward from the end. Returns 0 when it would cross the
//bucket array, i.e. when the registry is genuinely full.
inline intermodule_u32 intermodule_reg_alloc
   (intermodule_registry_header *h, intermodule_u32 size, intermodule_u32 align)
{
   intermodule_u32 top = h->arena_top;
   if(size > top){
      return 0;
   }
   top = (top - size) & ~(align - 1u);
   if(top < h->bucket_end || top > h->arena_top){
      return 0;
   }
   h->arena_top = top;
   return top;
}

//Appends one bucket and rehashes only the chain at the split pointer. If
//there is no room to append, chains simply get longer: never an error.
inline void intermodule_reg_split(intermodule_registry_header *h)
{
   if(h->bucket_end + (intermodule_u32)sizeof(intermodule_u32) > h->arena_top){
      return;
   }
   intermodule_u32 *const b = intermodule_reg_buckets(h);
   //The bucket appended is the one at the split pointer; the chain it takes
   //over comes from half a round below it
   const intermodule_u32 new_idx = h->split;
   const intermodule_u32 old_idx = new_idx - (h->bucket_cnt >> 1);
   const intermodule_u32 mask    = h->bucket_cnt - 1u;
   b[new_idx] = 0;
   h->bucket_end += (intermodule_u32)sizeof(intermodule_u32);

   //A record of that chain can only stay put or move to the bucket just
   //appended, which is what makes the rehash local
   intermodule_u32 off = b[old_idx];
   b[old_idx] = 0;
   while(off){
      intermodule_registry_record *const r = intermodule_reg_record(h, off);
      const intermodule_u32 next = r->next;
      const intermodule_u32 idx = r->hash & mask;
      r->next = b[idx];
      b[idx] = off;
      off = next;
   }
   if(++h->split == h->bucket_cnt){
      h->bucket_cnt <<= 1;   //round finished: the next one is twice as wide
   }
}

inline void intermodule_reg_init(intermodule_registry_header *h)
{
   const intermodule_u32 base = BOOST_CONTAINER_INTERMODULE_REGISTRY_BUCKETS;
   //The address function masks instead of dividing, so it must be a power of 2
   BOOST_ASSERT(base && 0 == (base & (base - 1u)));
   h->split       = base;        //buckets materialized == split pointer
   h->bucket_cnt  = base << 1;   //capacity of the round in progress
   h->entry_count = 0;
   h->bucket_end  = (intermodule_u32)sizeof(intermodule_registry_header)
                  + base*(intermodule_u32)sizeof(intermodule_u32);
   h->arena_top   = BOOST_CONTAINER_INTERMODULE_REGISTRY_BYTES;
   h->canonical   = h;
   h->magic       = intermodule_registry_magic;
}

inline intermodule_registry_record *intermodule_reg_find
   (intermodule_registry_header *h, intermodule_u32 hash, const char *key, std::size_t key_len)
{
   intermodule_u32 off = intermodule_reg_buckets(h)[intermodule_reg_bucket(h, hash)];
   while(off){
      intermodule_registry_record *const r = intermodule_reg_record(h, off);
      //A hash match is only a candidate: the full key decides, so two keys
      //colliding on 32 bits get separate entries instead of aliasing
      if( r->hash == hash && r->key_len == (intermodule_u32)key_len
       && 0 == std::memcmp(intermodule_reg_key(r), key, key_len)){
         return r;
      }
      off = r->next;
   }
   return 0;
}

inline intermodule_registry_record *intermodule_reg_insert
   ( intermodule_registry_header *h, intermodule_u32 hash
   , const char *key, std::size_t key_len
   , intermodule_u32 obj_size, intermodule_u32 obj_align)
{
   const intermodule_u32 obj_off = intermodule_reg_alloc(h, obj_size, obj_align);
   if(!obj_off){
      return 0;
   }
   const intermodule_u32 rec_size =
      (intermodule_u32)(sizeof(intermodule_registry_record) + key_len + 1u);
   const intermodule_u32 rec_off =
      intermodule_reg_alloc(h, rec_size, (intermodule_u32)sizeof(void *));
   if(!rec_off){
      return 0;
   }
   intermodule_registry_record *const r = intermodule_reg_record(h, rec_off);
   r->hash       = hash;
   r->obj_offset = obj_off;
   r->obj_size   = obj_size;
   r->key_len    = (intermodule_u32)key_len;
   r->refcount   = 0;   //not constructed yet
   std::memcpy(intermodule_reg_key(r), key, key_len);
   intermodule_reg_key(r)[key_len] = 0;

   intermodule_u32 *const b = intermodule_reg_buckets(h);
   const intermodule_u32 idx = intermodule_reg_bucket(h, hash);
   r->next = b[idx];
   b[idx]  = rec_off;
   //Records never move, only chain links change, so r stays valid.
   //split is the live bucket count, so this keeps the load factor at ~1
   if(++h->entry_count > h->split){
      intermodule_reg_split(h);
   }
   return r;
}

//"Local\boost_container_greg_<abi>_<pid>": one section for every singleton
inline void intermodule_build_registry_name(char (&dst)[96])
{
   char *p = intermodule_append_str(dst, "Local\\boost_container_greg_");
   p = intermodule_append_dec(p, (unsigned long)BOOST_CONTAINER_INTERMODULE_ABI_VERSION);
   *p++ = '_';
   p = intermodule_append_dec(p, (unsigned long)GetCurrentProcessId());
   *p = 0;
   BOOST_ASSERT(std::size_t(p - dst) < sizeof(dst));
}

template<class T, class Options>
struct intermodule_globals_impl
{
   BOOST_CONTAINER_FORCEINLINE static T &get()
   {
      (void)&ms_registrar;
      T *ptr = atomic_ptr_read_csm(&ms_cached);
      if(BOOST_UNLIKELY(!ptr)){
         ptr = attach();
      }
      return *ptr;
   }

   private:
   struct module_registrar
   {
      module_registrar()  {  (void)intermodule_globals_impl::attach();  }
      ~module_registrar() {  intermodule_globals_impl::detach();  }
   };
   friend struct module_registrar;

   static T *ms_cached;                                  //zero-init, per module
   static void *ms_section;                              //kept while attached
   static intermodule_registry_header *ms_header;        //canonical view
   static intermodule_registry_record *ms_record;
   static module_registrar ms_registrar;

   BOOST_NOINLINE static T *attach()
   {
      if(atomic_ptr_read_csm(&ms_cached)){
         return ms_cached;
      }
      char reg_name[96];
      intermodule_build_registry_name(reg_name);

      //Create-or-open is atomic, so the section itself is the rendezvous:
      //this backend needs no separate named mutex, the lock lives in the
      //header and is the same physical word in every module's view
      void *const section = CreateFileMappingA
         ( container_intermodule_winapi::invalid_handle(), 0
         , container_intermodule_winapi::page_readwrite_c
         , 0, (container_intermodule_winapi::dword_type)
              BOOST_CONTAINER_INTERMODULE_REGISTRY_BYTES
         , reg_name);
      BOOST_ASSERT_MSG(section != 0, "Boost.Container: CreateFileMappingA failed "
         "(processes that cannot create named sections are not supported)");

      intermodule_registry_header *const view = (intermodule_registry_header *)
         MapViewOfFile(section, container_intermodule_winapi::file_map_read_write_c, 0, 0, 0);
      BOOST_ASSERT_MSG(view != 0, "Boost.Container: MapViewOfFile failed");

      //A fresh section is zero-filled, so the lock starts unlocked and the
      //magic tells the first module to lay the registry out
      view->lock.lock();

      intermodule_registry_header *hdr;
      if(view->magic != intermodule_registry_magic){
         intermodule_reg_init(view);
         hdr = view;
      }
      else{
         //Adopt the base the first module published: object addresses are
         //absolute and must be identical in every module
         hdr = (intermodule_registry_header *)view->canonical;
      }

      const char *const key = intermodule_rendezvous_key<T, Options>();
      const std::size_t key_len = std::strlen(key);
      const intermodule_u32 hash = intermodule_reg_hash(key, key_len);

      intermodule_registry_record *r = intermodule_reg_find(hdr, hash, key, key_len);
      if(!r){
         r = intermodule_reg_insert(hdr, hash, key, key_len
            , (intermodule_u32)sizeof(T), (intermodule_u32)alignment_of<T>::value);
         BOOST_ASSERT_MSG(r != 0, "Boost.Container: intermodule registry exhausted "
            "(raise BOOST_CONTAINER_INTERMODULE_REGISTRY_BYTES)");
      }
      //Same key but a different size means two modules disagree about T:
      //an ODR violation that placement-new would turn into corruption
      BOOST_ASSERT_MSG(r->obj_size == (intermodule_u32)sizeof(T)
         , "Boost.Container: intermodule globals size mismatch for this key");

      T *const obj = (T *)(void *)((char *)hdr + r->obj_offset);
      bool constructed_here = false;
      if(0 == r->refcount){
         //Exactly like the constructor of a global variable of type T
         //(T's constructor must not throw)
         ::new((void *)obj, boost_container_new_t()) T();
         constructed_here = true;
      }
      ++r->refcount;

      //Unlock through the bootstrap view, which is still mapped; it is the
      //same physical word as hdr->lock
      view->lock.unlock();

      if(hdr != view){
         (void)UnmapViewOfFile(view);
      }
      ms_section = section;   //keeps the section alive while this module lives
      ms_header  = hdr;
      ms_record  = r;
      atomic_ptr_write_rel(&ms_cached, obj);

      //Outside the critical section: see the note in the other backend
      if(constructed_here && intermodule_opt_pin_constructing_module<Options>::value){
         intermodule_pin_this_module();
      }
      return obj;
   }

   BOOST_NOINLINE static void detach()
   {
      if(!intermodule_opt_destroy_at_exit<Options>::value){
         return;   //immortal: keep the handle, and with it the section
      }
      if(!atomic_ptr_read_csm(&ms_cached)){
         return;
      }
      intermodule_registry_header *const hdr = ms_header;
      hdr->lock.lock();
      intermodule_registry_record *const r = ms_record;
      if(r && 0 == --r->refcount){
         ((T *)(void *)((char *)hdr + r->obj_offset))->~T();
         //The record and its storage stay in place, now with refcount 0: a
         //module loaded again later reconstructs T in the very same slot
      }
      hdr->lock.unlock();

      //The canonical view is deliberately never unmapped, see the note above
      (void)CloseHandle(ms_section);
      ms_section = 0;
      ms_header  = 0;
      ms_record  = 0;
      //A get() running even later (static destruction is unordered across
      //translation units) phoenix-reconstructs through attach()
      atomic_ptr_write_rel(&ms_cached, (T *)0);
   }
};

template<class T, class Options>
T *intermodule_globals_impl<T, Options>::ms_cached = 0;

template<class T, class Options>
void *intermodule_globals_impl<T, Options>::ms_section = 0;

template<class T, class Options>
intermodule_registry_header *intermodule_globals_impl<T, Options>::ms_header = 0;

template<class T, class Options>
intermodule_registry_record *intermodule_globals_impl<T, Options>::ms_record = 0;


template<class T, class Options>
typename intermodule_globals_impl<T, Options>::module_registrar
   intermodule_globals_impl<T, Options>::ms_registrar;

}  //namespace dtl {
}  //namespace container {
}  //namespace boost {

#else //VISIBLE and LOCAL backends

namespace boost {
namespace container {
namespace dtl {

//No-op outside Windows: on ELF, GCC's STB_GNU_UNIQUE already makes the
//defining shared object non-unloadable; on Mach-O bundles the situation
//is equivalent to the documented caveat.
inline void intermodule_pin_this_module()
{}

//NOTE: keep BOOST_SYMBOL_VISIBLE here AND on every T/Options instantiated
//with intermodule_globals<>: on ELF the visibility of a template
//instantiation is the minimum of the visibilities of the template and all
//its arguments, so a hidden payload type would silently demote the shared
//storage to one copy per module under -fvisibility=hidden.
//
//The payload is held as raw storage, not as a T member: T is constructed
//and destroyed explicitly (like a global variable, but under our control),
//so a T member would run its constructor as an ordinary static initializer.
//Raw storage also keeps the whole thing zero-initialized and honours T's
//alignment, over-aligned types included.
template<class T>
struct BOOST_SYMBOL_VISIBLE intermodule_globals_storage
{
   typename aligned_storage<sizeof(T), alignment_of<T>::value>::type obj;
};

#if defined(BOOST_CONTAINER_INTERMODULE_BACKEND_VISIBLE)
#  define BOOST_CONTAINER_INTERMODULE_VISIBILITY BOOST_SYMBOL_VISIBLE
#else
#  define BOOST_CONTAINER_INTERMODULE_VISIBILITY
#endif

//Class template static data members have vague linkage (C++03-compatible,
//unlike C++17 inline variables). With default visibility the dynamic loader
//unifies the definitions across all shared objects of the process - the
//registrar included, which therefore behaves as one process-wide global
//C++ object: its constructor runs once (in the first module that
//initializes it) and its destructor is registered once, running after
//main() / when the defining shared object would unload (never before exit
//with GCC's STB_GNU_UNIQUE, which makes that object non-unloadable).
template<class T, class Options>
struct BOOST_CONTAINER_INTERMODULE_VISIBILITY intermodule_globals_impl
{
   static T &get()
   {
      (void)&ms_registrar;   //construct before main(), destroy after it
      //Readiness is carried by the pointer itself, not by a separate flag,
      //so the consume load below suffices. A separate "constructed" flag
      //would make this classic double-checked locking, where a reader could
      //see the flag set yet a half-constructed object on a weakly ordered
      //CPU (the flag and the storage are unrelated addresses).
      T *p = atomic_ptr_read_csm(&ms_cached);
      if(!p){
         p = attach();
      }
      return *p;
   }

   private:
   struct module_registrar
   {
      module_registrar()  {  (void)intermodule_globals_impl::attach();  }
      ~module_registrar() {  intermodule_globals_impl::detach();  }
   };
   friend struct module_registrar;

   static T *ms_cached;                                //zero-initialized
   static intermodule_globals_storage<T> ms_storage;   //zero-initialized
   static spin_mutex ms_lock;
   static module_registrar ms_registrar;

   BOOST_NOINLINE static T *attach()
   {
      ms_lock.lock();
      if(!ms_cached){
         //Exactly like the constructor of a global variable of type T
         //(T's constructor must not throw).
         //
         //What gets published is the pointer placement new returns, never
         //one re-derived from &ms_storage.obj: that address designates the
         //aligned_storage object whose lifetime has just ended, and reading
         //the new T through it is precisely the case std::launder exists
         //for - which C++03 cannot express.
         T *const p = ::new((void *)&ms_storage.obj, boost_container_new_t()) T();
         //Publish only once the object is fully constructed. The release is
         //NOT redundant with the unlock below: an unlock is a one-way
         //barrier, it only keeps earlier writes from sinking past it and
         //would still allow the pointer store to be hoisted above the
         //constructor's stores. get() reads this pointer without taking the
         //lock, so it would then reach a half-built object; the mutex only
         //orders threads that acquire it.
         atomic_ptr_write_rel(&ms_cached, p);
      }
      ms_lock.unlock();
      return ms_cached;
   }

   BOOST_NOINLINE static void detach()
   {
      if(!intermodule_opt_destroy_at_exit<Options>::value){
         return;
      }
      ms_lock.lock();
      T *const obj = ms_cached;
      if(obj){
         obj->~T();
         //Re-zero so a later get() (unordered static destruction in some
         //other translation unit) phoenix-reconstructs instead of using a
         //destroyed object
         std::memset((void *)&ms_storage, 0, sizeof(ms_storage));
         ms_cached = 0;
      }
      ms_lock.unlock();
   }
};

template<class T, class Options>
T *intermodule_globals_impl<T, Options>::ms_cached = 0;

template<class T, class Options>
intermodule_globals_storage<T> intermodule_globals_impl<T, Options>::ms_storage;

template<class T, class Options>
//No initializer: a spin_mutex is unlocked when its word is zero, so static
//zero initialization already leaves it usable, before any dynamic
//initialization of this translation unit could run
spin_mutex intermodule_globals_impl<T, Options>::ms_lock;

template<class T, class Options>
typename intermodule_globals_impl<T, Options>::module_registrar
   intermodule_globals_impl<T, Options>::ms_registrar;

}  //namespace dtl {
}  //namespace container {
}  //namespace boost {

#endif   //backend

namespace boost {
namespace container {
namespace dtl {

//////////////////////////////////////////////////////////////////////////////
//
//    Public (detail) interface
//
//////////////////////////////////////////////////////////////////////////////

//Returns the single process-wide instance of T for the given Options, with
//global-C++-object lifetime: T's constructor runs before main() (or on
//first use, whichever comes first) and, unless Options disables it, T's
//destructor runs during static destruction when the last attached module
//detaches. See the file header for the T/Options requirements.
//
//Note: the instance is keyed on the T/Options pair, so a configuration that
//changes the layout of the shared state has to change one of those two
//types (and modules yielding a different sizeof(T) for the same pair are an
//ODR violation the backend asserts on).
template<class T, class Options>
BOOST_CONTAINER_FORCEINLINE T &intermodule_globals()
{
   return intermodule_globals_impl<T, Options>::get();
}

//Options defaults to void, the empty option set, so intermodule_globals<T>()
//and intermodule_globals<T, void>() name one and the same object. Spelt as
//an overload and not as a default template argument, which C++03 allows on
//class templates only.
template<class T>
BOOST_CONTAINER_FORCEINLINE T &intermodule_globals()
{
   return intermodule_globals_impl<T, void>::get();
}

}  //namespace dtl {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_DETAIL_INTERMODULE_GLOBALS_HPP
