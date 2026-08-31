//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_DETAIL_DLMALLOC_HPP
#define BOOST_CONTAINER_DETAIL_DLMALLOC_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>


/* ===================== C interface types and macros ===================== */


#include <stddef.h>

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4127)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*!An forward iterator to traverse the elements of a memory chain container.*/
typedef struct multialloc_node_impl
{
   struct multialloc_node_impl *next_node_ptr;
} boost_cont_memchain_node;


/*!An forward iterator to traverse the elements of a memory chain container.*/
typedef struct multialloc_it_impl
{
   boost_cont_memchain_node *node_ptr;
} boost_cont_memchain_it;

/*!Memory chain: A container holding memory portions allocated by boost_cont_multialloc_nodes
   and boost_cont_multialloc_arrays functions.*/
typedef struct boost_cont_memchain_impl
{
   size_t                   num_mem;
   boost_cont_memchain_node  root_node;
   boost_cont_memchain_node *last_node_ptr;
} boost_cont_memchain;

/*!Advances the iterator one position so that it points to the next element in the memory chain*/
#define BOOST_CONTAINER_MEMIT_NEXT(IT)         (IT.node_ptr = IT.node_ptr->next_node_ptr)

/*!Returns the address of the memory chain currently pointed by the iterator*/
#define BOOST_CONTAINER_MEMIT_ADDR(IT)      ((void*)IT.node_ptr)

/*!Initializer for an iterator pointing to the position before the first element*/
#define BOOST_CONTAINER_MEMCHAIN_BEFORE_BEGIN_IT(PMEMCHAIN)   { &((PMEMCHAIN)->root_node) }

/*!Initializer for an iterator pointing to the first element*/
#define BOOST_CONTAINER_MEMCHAIN_BEGIN_IT(PMEMCHAIN)   {(PMEMCHAIN)->root_node.next_node_ptr }

/*!Initializer for an iterator pointing to the last element*/
#define BOOST_CONTAINER_MEMCHAIN_LAST_IT(PMEMCHAIN)    {(PMEMCHAIN)->last_node_ptr }

/*!Initializer for an iterator pointing to one past the last element (end iterator)*/
#define BOOST_CONTAINER_MEMCHAIN_END_IT(PMEMCHAIN)     {(boost_cont_memchain_node *)0 }

/*!True if IT is the end iterator, false otherwise*/
#define BOOST_CONTAINER_MEMCHAIN_IS_END_IT(PMEMCHAIN, IT) (!(IT).node_ptr)

/*!The address of the first memory portion hold by the memory chain*/
#define BOOST_CONTAINER_MEMCHAIN_FIRSTMEM(PMEMCHAIN)((void*)((PMEMCHAIN)->root_node.next_node_ptr))

/*!The address of the last memory portion hold by the memory chain*/
#define BOOST_CONTAINER_MEMCHAIN_LASTMEM(PMEMCHAIN) ((void*)((PMEMCHAIN)->last_node_ptr))

/*!The number of memory portions hold by the memory chain*/
#define BOOST_CONTAINER_MEMCHAIN_SIZE(PMEMCHAIN) ((PMEMCHAIN)->num_mem)

/*!Initializes the memory chain from the first memory portion, the last memory
   portion and number of portions obtained from another memory chain*/
#define BOOST_CONTAINER_MEMCHAIN_INIT_FROM(PMEMCHAIN, FIRST, LAST, NUM)\
   (PMEMCHAIN)->last_node_ptr = (boost_cont_memchain_node *)(LAST), \
   (PMEMCHAIN)->root_node.next_node_ptr  = (boost_cont_memchain_node *)(FIRST), \
   (PMEMCHAIN)->num_mem  = (NUM);\
/**/

/*!Default initializes a memory chain. Postconditions: begin iterator is end iterator,
   the number of portions is zero.*/
#define BOOST_CONTAINER_MEMCHAIN_INIT(PMEMCHAIN)\
   ((PMEMCHAIN)->root_node.next_node_ptr = 0, (PMEMCHAIN)->last_node_ptr = &((PMEMCHAIN)->root_node), (PMEMCHAIN)->num_mem = 0)\
/**/

/*!True if the memory chain is empty (holds no memory portions*/
#define BOOST_CONTAINER_MEMCHAIN_EMPTY(PMEMCHAIN)\
   ((PMEMCHAIN)->num_mem == 0)\
/**/

/*!Inserts a new memory portions in the front of the chain*/
#define BOOST_CONTAINER_MEMCHAIN_PUSH_BACK(PMEMCHAIN, MEM)\
   do{\
      boost_cont_memchain *____chain____ = (PMEMCHAIN);\
      boost_cont_memchain_node *____tmp_mem____ = (boost_cont_memchain_node *)(MEM);\
      ____chain____->last_node_ptr->next_node_ptr = ____tmp_mem____;\
      ____tmp_mem____->next_node_ptr = 0;\
      ____chain____->last_node_ptr = ____tmp_mem____;\
      ++____chain____->num_mem;\
   }while(0)\
/**/

/*!Inserts a new memory portions in the back of the chain*/
#define BOOST_CONTAINER_MEMCHAIN_PUSH_FRONT(PMEMCHAIN, MEM)\
   do{\
      boost_cont_memchain *____chain____ = (PMEMCHAIN);\
      boost_cont_memchain_node *____tmp_mem____   = (boost_cont_memchain_node *)(MEM);\
      boost_cont_memchain_node *____root____  = &((PMEMCHAIN)->root_node);\
      if(!____chain____->root_node.next_node_ptr){\
         ____chain____->last_node_ptr = ____tmp_mem____;\
      }\
      boost_cont_memchain_node *____old_first____ = ____root____->next_node_ptr;\
      ____tmp_mem____->next_node_ptr = ____old_first____;\
      ____root____->next_node_ptr = ____tmp_mem____;\
      ++____chain____->num_mem;\
   }while(0)\
/**/

/*!Erases the memory portion after the portion pointed by BEFORE_IT from the memory chain*/
/*!Precondition: BEFORE_IT must be a valid iterator of the memory chain and it can't be the end iterator*/
#define BOOST_CONTAINER_MEMCHAIN_ERASE_AFTER(PMEMCHAIN, BEFORE_IT)\
   do{\
      boost_cont_memchain *____chain____ = (PMEMCHAIN);\
      boost_cont_memchain_node *____prev_node____  = (BEFORE_IT).node_ptr;\
      boost_cont_memchain_node *____erase_node____ = ____prev_node____->next_node_ptr;\
      if(____chain____->last_node_ptr == ____erase_node____){\
         ____chain____->last_node_ptr = &____chain____->root_node;\
      }\
      ____prev_node____->next_node_ptr = ____erase_node____->next_node_ptr;\
      --____chain____->num_mem;\
   }while(0)\
/**/

/*!Erases the first portion from the memory chain.
   Precondition: the memory chain must not be empty*/
#define BOOST_CONTAINER_MEMCHAIN_POP_FRONT(PMEMCHAIN)\
   do{\
      boost_cont_memchain *____chain____ = (PMEMCHAIN);\
      boost_cont_memchain_node *____prev_node____  = &____chain____->root_node;\
      boost_cont_memchain_node *____erase_node____ = ____prev_node____->next_node_ptr;\
      if(____chain____->last_node_ptr == ____erase_node____){\
         ____chain____->last_node_ptr = &____chain____->root_node;\
      }\
      ____prev_node____->next_node_ptr = ____erase_node____->next_node_ptr;\
      --____chain____->num_mem;\
   }while(0)\
/**/

/*!Joins two memory chains inserting the portions of the second chain at the back of the first chain*/
/*
#define BOOST_CONTAINER_MEMCHAIN_SPLICE_BACK(PMEMCHAIN, PMEMCHAIN2)\
   do{\
      boost_cont_memchain *____chain____  = (PMEMCHAIN);\
      boost_cont_memchain *____chain2____ = (PMEMCHAIN2);\
      if(!____chain2____->root_node.next_node_ptr){\
         break;\
      }\
      else if(!____chain____->first_mem){\
         ____chain____->first_mem  = ____chain2____->first_mem;\
         ____chain____->last_node_ptr = ____chain2____->last_node_ptr;\
         ____chain____->num_mem  = ____chain2____->num_mem;\
         BOOST_CONTAINER_MEMCHAIN_INIT(*____chain2____);\
      }\
      else{\
         ____chain____->last_node_ptr->next_node_ptr = ____chain2____->first_mem;\
         ____chain____->last_node_ptr = ____chain2____->last_node_ptr;\
         ____chain____->num_mem += ____chain2____->num_mem;\
      }\
   }while(0)\*/
/**/

/*!Joins two memory chains inserting the portions of the second chain at the back of the first chain*/
#define BOOST_CONTAINER_MEMCHAIN_INCORPORATE_AFTER(PMEMCHAIN, BEFORE_IT, FIRST, BEFORELAST, NUM)\
   do{\
      boost_cont_memchain *____chain____  = (PMEMCHAIN);\
      boost_cont_memchain_node *____pnode____  = (BEFORE_IT).node_ptr;\
      boost_cont_memchain_node *____next____   = ____pnode____->next_node_ptr;\
      boost_cont_memchain_node *____first____  = (boost_cont_memchain_node *)(FIRST);\
      boost_cont_memchain_node *____blast____  = (boost_cont_memchain_node *)(BEFORELAST);\
      size_t ____num____ = (NUM);\
      if(!____num____){\
         break;\
      }\
      if(____pnode____ == ____chain____->last_node_ptr){\
         ____chain____->last_node_ptr = ____blast____;\
      }\
      ____pnode____->next_node_ptr  = ____first____;\
      ____blast____->next_node_ptr  = ____next____;\
      ____chain____->num_mem  += ____num____;\
   }while(0)\
/**/

/*!Indicates the all elements allocated by boost_cont_multialloc_nodes or boost_cont_multialloc_arrays
   must be contiguous.*/
#define BOOST_CONTAINER_DL_MULTIALLOC_ALL_CONTIGUOUS        ((size_t)(-1))

/*!Indicates the number of contiguous elements allocated by boost_cont_multialloc_nodes or boost_cont_multialloc_arrays
   should be selected by those functions.*/
#define BOOST_CONTAINER_DL_MULTIALLOC_DEFAULT_CONTIGUOUS    ((size_t)(0))

typedef struct boost_cont_malloc_stats_impl
{
   size_t max_system_bytes;
   size_t system_bytes;
   size_t in_use_bytes;
} boost_cont_malloc_stats_t;

typedef unsigned int allocation_type;

enum
{
   // constants for allocation commands
   BOOST_CONTAINER_ALLOCATE_NEW          = 0X01,
   BOOST_CONTAINER_EXPAND_FWD            = 0X02,
   BOOST_CONTAINER_EXPAND_BWD            = 0X04,
   BOOST_CONTAINER_SHRINK_IN_PLACE       = 0X08,
   BOOST_CONTAINER_NOTHROW_ALLOCATION    = 0X10,
//   BOOST_CONTAINER_ZERO_MEMORY           = 0X20,
   BOOST_CONTAINER_TRY_SHRINK_IN_PLACE   = 0X40,
   BOOST_CONTAINER_EXPAND_BOTH           = BOOST_CONTAINER_EXPAND_FWD | BOOST_CONTAINER_EXPAND_BWD,
   BOOST_CONTAINER_EXPAND_OR_NEW         = BOOST_CONTAINER_ALLOCATE_NEW | BOOST_CONTAINER_EXPAND_BOTH
};

//#define BOOST_CONTAINER_DLMALLOC_FOOTERS
#ifndef BOOST_CONTAINER_DLMALLOC_FOOTERS
enum {   BOOST_CONTAINER_ALLOCATION_PAYLOAD = sizeof(size_t)   };
#else
enum {   BOOST_CONTAINER_ALLOCATION_PAYLOAD = sizeof(size_t)*2   };
#endif

typedef struct boost_cont_command_ret_impl
{
   void *first;
   int   second;
}boost_cont_command_ret_t;


#ifdef __cplusplus
}  //extern "C" {
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif


/* ============ dlmalloc implementation and extensions ============ */
#include <boost/container/detail/intermodule_globals.hpp>
#include <boost/container/detail/spin_mutex.hpp>

//dlmalloc bug: EINVAL is used in posix_memalign without checking LACKS_ERRNO_H
#include <errno.h>
#include <limits.h>  //CHAR_BIT
#include <stddef.h>
#include <stdlib.h>  //abort

/* Single-threaded lock elision. glibc 2.32+ maintains __libc_single_threaded,
   a plain char that is nonzero only while the process provably has a single
   thread: pthread_create clears it before the new thread runs a single
   instruction, and the creating thread observes the cleared value before
   pthread_create returns. That gives the elision its safety argument: the
   flag can only change inside pthread_create (or fork), and the one live
   thread cannot be inside this allocator and inside pthread_create at the
   same time, so the flag is stable across any single critical section and
   the ACQUIRE/RELEASE decisions always pair up. This is the same reasoning
   libstdc++ uses to elide shared_ptr's atomic reference counts.
   Uncontended, the elision replaces a locked exchange plus a release store -
   the single largest cost of a malloc/free pair - with one byte load.
   Included here, at global scope, so the declaration cannot land inside the
   dl_detail namespace (the core includes system headers mid-namespace). */
#pragma push_macro("BOOST_CONTAINER_DL_SINGLE_THREADED")
#if !defined(BOOST_CONTAINER_DLMALLOC_NO_SINGLE_THREAD_OPT) && \
    defined(__GLIBC__) && defined(__GLIBC_MINOR__) && \
    (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 32))
#  include <sys/single_threaded.h>
#  define BOOST_CONTAINER_DL_SINGLE_THREADED (::__libc_single_threaded != 0)
#else
#  define BOOST_CONTAINER_DL_SINGLE_THREADED 0
#endif
#include <string.h>  //memset/memcpy

//System headers original dlmalloc included mid-file: included here, outside the
//namespace, so the copies inside the namespace expand to nothing thanks to
//their include guards.
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <fcntl.h>

#if !defined(_WIN32)
#  if defined(linux) && !defined(__USE_GNU)
      //mremap decl in mman.h needs __USE_GNU
#     define __USE_GNU 1
#     include <sys/mman.h>
#     undef __USE_GNU
#  else
#     include <sys/mman.h>
#  endif
#  include <unistd.h>
#  include <sys/param.h>
#  if defined (__SVR4) && defined (__sun)
#     include <thread.h>
#  elif !defined(LACKS_SCHED_H)
#     include <sched.h>
#  endif
#  if !(defined(__GNUC__) && \
        ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)) || \
         defined(__i386__) || defined(__x86_64__)))
#     include <pthread.h>   //no spin-lock support: dlmalloc uses pthread mutexes
#  endif
#else //_WIN32
   //No <windows.h> (it would leak min/max & itger macros into every consumer)
   //Declare the handful of APIs dlmalloc needs, exactly matching
   //the <windows.h> prototypes so both can coexist in one translation unit.
#  if defined(_WIN64) || defined(__LP64__)
typedef unsigned __int64 dl_win_size_t;   //matches SIZE_T
#  else
typedef unsigned long    dl_win_size_t;
#  endif
#  ifdef BOOST_USE_WINDOWS_H
#     include <windows.h>
#  else
struct _MEMORY_BASIC_INFORMATION;
struct _SYSTEM_INFO;

extern "C" {
__declspec(dllimport) void * __stdcall VirtualAlloc
   (void *lpAddress, dl_win_size_t dwSize, unsigned long flAllocationType, unsigned long flProtect);
__declspec(dllimport) int __stdcall VirtualFree
   (void *lpAddress, dl_win_size_t dwSize, unsigned long dwFreeType);
__declspec(dllimport) dl_win_size_t __stdcall VirtualQuery
   (const void *lpAddress, ::_MEMORY_BASIC_INFORMATION *lpBuffer, dl_win_size_t dwLength);
__declspec(dllimport) void __stdcall GetSystemInfo(::_SYSTEM_INFO *lpSystemInfo);
__declspec(dllimport) unsigned long __stdcall GetTickCount(void);
}  //extern "C"
#  endif //BOOST_USE_WINDOWS_H

//Mirror structs (layout-compatible with MEMORY_BASIC_INFORMATION/SYSTEM_INFO)
//so we never need the real definitions from <windows.h>.
struct dl_win_memory_basic_information
{
   void          *BaseAddress;
   void          *AllocationBase;
   unsigned long  AllocationProtect;
#  if defined(_WIN64)
   unsigned short PartitionId;
#  endif
   dl_win_size_t  RegionSize;
   unsigned long  State;
   unsigned long  Protect;
   unsigned long  Type;
};

struct dl_win_system_info
{
   unsigned long  dwOemId;
   unsigned long  dwPageSize;
   void          *lpMinimumApplicationAddress;
   void          *lpMaximumApplicationAddress;
   dl_win_size_t  dwActiveProcessorMask;
   unsigned long  dwNumberOfProcessors;
   unsigned long  dwProcessorType;
   unsigned long  dwAllocationGranularity;
   unsigned short wProcessorLevel;
   unsigned short wProcessorRevision;
};

#endif //!_WIN32

/* --------------------------- macro hygiene, save ---------------------------
   The implementation below defines 284 short macros (gm, fm, chunksize,
   PREACTION...). Every one of them is saved here and restored at the bottom
   of this header, so none leaks into the translation unit that included us.

   Saving does NOT undefine: dlmalloc's own #ifndef guards must still see the
   macros already in effect (MAP_ANONYMOUS, FORCEINLINE...). The restore block
   reinstates exactly this state, dropping only dlmalloc's own additions.

   GENERATED by convert_dlmalloc.py, together with the restore block.
   -------------------------------------------------------------------------- */
#pragma push_macro("mymalloc")
#pragma push_macro("DLMALLOC_EXPORT")
#pragma push_macro("DLMALLOC_VERSION")
#pragma push_macro("DL_WIN32")
#pragma push_macro("LACKS_FCNTL_H")
#pragma push_macro("HAVE_MMAP")
#pragma push_macro("HAVE_MORECORE")
#pragma push_macro("LACKS_UNISTD_H")
#pragma push_macro("LACKS_SYS_PARAM_H")
#pragma push_macro("LACKS_SYS_MMAN_H")
#pragma push_macro("LACKS_STRING_H")
#pragma push_macro("LACKS_STRINGS_H")
#pragma push_macro("LACKS_SYS_TYPES_H")
#pragma push_macro("LACKS_ERRNO_H")
#pragma push_macro("LACKS_SCHED_H")
#pragma push_macro("MALLOC_FAILURE_ACTION")
#pragma push_macro("MMAP_CLEARS")
#pragma push_macro("MALLOC_ALIGNMENT")
#pragma push_macro("MAX_SIZE_T")
#pragma push_macro("USE_LOCKS")
#pragma push_macro("USE_SPIN_LOCKS")
#pragma push_macro("ONLY_MSPACES")
#pragma push_macro("MSPACES")
#pragma push_macro("FOOTERS")
#pragma push_macro("ABORT")
#pragma push_macro("ABORT_ON_ASSERT_FAILURE")
#pragma push_macro("PROCEED_ON_ERROR")
#pragma push_macro("INSECURE")
#pragma push_macro("MALLOC_INSPECT_ALL")
#pragma push_macro("HAVE_MREMAP")
#pragma push_macro("MORECORE_CONTIGUOUS")
#pragma push_macro("MORECORE_DEFAULT")
#pragma push_macro("DEFAULT_GRANULARITY")
#pragma push_macro("DEFAULT_TRIM_THRESHOLD")
#pragma push_macro("DEFAULT_MMAP_THRESHOLD")
#pragma push_macro("MAX_RELEASE_CHECK_RATE")
#pragma push_macro("USE_BUILTIN_FFS")
#pragma push_macro("USE_DEV_RANDOM")
#pragma push_macro("NO_MALLINFO")
#pragma push_macro("MALLINFO_FIELD_TYPE")
#pragma push_macro("NO_MALLOC_STATS")
#pragma push_macro("NO_SEGMENT_TRAVERSAL")
#pragma push_macro("M_TRIM_THRESHOLD")
#pragma push_macro("M_GRANULARITY")
#pragma push_macro("M_MMAP_THRESHOLD")
#pragma push_macro("_STRUCT_MALLINFO")
#pragma push_macro("STRUCT_MALLINFO_DECLARED")
#pragma push_macro("FORCEINLINE")
#pragma push_macro("NOINLINE")
#pragma push_macro("dlcalloc")
#pragma push_macro("dlfree")
#pragma push_macro("dlmalloc")
#pragma push_macro("dlmemalign")
#pragma push_macro("dlposix_memalign")
#pragma push_macro("dlrealloc")
#pragma push_macro("dlrealloc_in_place")
#pragma push_macro("dlvalloc")
#pragma push_macro("dlpvalloc")
#pragma push_macro("dlmallinfo")
#pragma push_macro("dlmallopt")
#pragma push_macro("dlmalloc_trim")
#pragma push_macro("dlmalloc_stats")
#pragma push_macro("dlmalloc_usable_size")
#pragma push_macro("dlmalloc_footprint")
#pragma push_macro("dlmalloc_max_footprint")
#pragma push_macro("dlmalloc_footprint_limit")
#pragma push_macro("dlmalloc_set_footprint_limit")
#pragma push_macro("dlmalloc_inspect_all")
#pragma push_macro("dlindependent_calloc")
#pragma push_macro("dlindependent_comalloc")
#pragma push_macro("dlbulk_free")
#pragma push_macro("DL_ASSERT")
#pragma push_macro("DL_DEBUG")
#pragma push_macro("__USE_GNU")
#pragma push_macro("interlockedcompareexchange")
#pragma push_macro("interlockedexchange")
#pragma push_macro("LOCK_AT_FORK")
#pragma push_macro("BitScanForward")
#pragma push_macro("BitScanReverse")
#pragma push_macro("_SC_PAGE_SIZE")
#pragma push_macro("malloc_getpagesize")
#pragma push_macro("SIZE_T_SIZE")
#pragma push_macro("SIZE_T_BITSIZE")
#pragma push_macro("SIZE_T_ZERO")
#pragma push_macro("SIZE_T_ONE")
#pragma push_macro("SIZE_T_TWO")
#pragma push_macro("SIZE_T_FOUR")
#pragma push_macro("TWO_SIZE_T_SIZES")
#pragma push_macro("FOUR_SIZE_T_SIZES")
#pragma push_macro("SIX_SIZE_T_SIZES")
#pragma push_macro("HALF_MAX_SIZE_T")
#pragma push_macro("CHUNK_ALIGN_MASK")
#pragma push_macro("is_aligned")
#pragma push_macro("align_offset")
#pragma push_macro("MFAIL")
#pragma push_macro("CMFAIL")
#pragma push_macro("MUNMAP_DEFAULT")
#pragma push_macro("MMAP_PROT")
#pragma push_macro("MAP_ANONYMOUS")
#pragma push_macro("MMAP_FLAGS")
#pragma push_macro("MMAP_DEFAULT")
#pragma push_macro("dev_zero_fd")
#pragma push_macro("DIRECT_MMAP_DEFAULT")
#pragma push_macro("MREMAP_DEFAULT")
#pragma push_macro("CALL_MORECORE")
#pragma push_macro("USE_MMAP_BIT")
#pragma push_macro("CALL_MMAP")
#pragma push_macro("CALL_MUNMAP")
#pragma push_macro("CALL_DIRECT_MMAP")
#pragma push_macro("MMAP")
#pragma push_macro("MUNMAP")
#pragma push_macro("DIRECT_MMAP")
#pragma push_macro("CALL_MREMAP")
#pragma push_macro("USE_NONCONTIGUOUS_BIT")
#pragma push_macro("EXTERN_BIT")
#pragma push_macro("USE_LOCK_BIT")
#pragma push_macro("INITIAL_LOCK")
#pragma push_macro("DESTROY_LOCK")
#pragma push_macro("ACQUIRE_MALLOC_GLOBAL_LOCK")
#pragma push_macro("RELEASE_MALLOC_GLOBAL_LOCK")
#pragma push_macro("CAS_LOCK")
#pragma push_macro("CLEAR_LOCK")
#pragma push_macro("SPINS_PER_YIELD")
#pragma push_macro("SLEEP_EX_DURATION")
#pragma push_macro("SPIN_LOCK_YIELD")
#pragma push_macro("MLOCK_T")
#pragma push_macro("TRY_LOCK")
#pragma push_macro("RELEASE_LOCK")
#pragma push_macro("ACQUIRE_LOCK")
#pragma push_macro("THREAD_ID_T")
#pragma push_macro("CURRENT_THREAD")
#pragma push_macro("EQ_OWNER")
#pragma push_macro("NEED_GLOBAL_LOCK_INIT")
#pragma push_macro("PTHREAD_MUTEX_RECURSIVE")
#pragma push_macro("pthread_mutexattr_settype")
#pragma push_macro("MCHUNK_SIZE")
#pragma push_macro("CHUNK_OVERHEAD")
#pragma push_macro("MMAP_CHUNK_OVERHEAD")
#pragma push_macro("MMAP_FOOT_PAD")
#pragma push_macro("MIN_CHUNK_SIZE")
#pragma push_macro("chunk2mem")
#pragma push_macro("mem2chunk")
#pragma push_macro("align_as_chunk")
#pragma push_macro("MAX_REQUEST")
#pragma push_macro("MIN_REQUEST")
#pragma push_macro("pad_request")
#pragma push_macro("request2size")
#pragma push_macro("PINUSE_BIT")
#pragma push_macro("CINUSE_BIT")
#pragma push_macro("FLAG4_BIT")
#pragma push_macro("INUSE_BITS")
#pragma push_macro("FLAG_BITS")
#pragma push_macro("FENCEPOST_HEAD")
#pragma push_macro("cinuse")
#pragma push_macro("pinuse")
#pragma push_macro("flag4inuse")
#pragma push_macro("is_inuse")
#pragma push_macro("is_mmapped")
#pragma push_macro("chunksize")
#pragma push_macro("clear_pinuse")
#pragma push_macro("set_flag4")
#pragma push_macro("clear_flag4")
#pragma push_macro("chunk_plus_offset")
#pragma push_macro("chunk_minus_offset")
#pragma push_macro("next_chunk")
#pragma push_macro("prev_chunk")
#pragma push_macro("next_pinuse")
#pragma push_macro("get_foot")
#pragma push_macro("set_foot")
#pragma push_macro("set_size_and_pinuse_of_free_chunk")
#pragma push_macro("set_free_with_pinuse")
#pragma push_macro("overhead_for")
#pragma push_macro("calloc_must_clear")
#pragma push_macro("leftmost_child")
#pragma push_macro("is_mmapped_segment")
#pragma push_macro("is_extern_segment")
#pragma push_macro("NSMALLBINS")
#pragma push_macro("NTREEBINS")
#pragma push_macro("SMALLBIN_SHIFT")
#pragma push_macro("SMALLBIN_WIDTH")
#pragma push_macro("TREEBIN_SHIFT")
#pragma push_macro("MIN_LARGE_SIZE")
#pragma push_macro("MAX_SMALL_SIZE")
#pragma push_macro("MAX_SMALL_REQUEST")
#pragma push_macro("malloc_global_mutex")
#pragma push_macro("malloc_corruption_error_count")
#pragma push_macro("mparams")
#pragma push_macro("ensure_initialization")
#pragma push_macro("gm")
#pragma push_macro("is_global")
#pragma push_macro("is_initialized")
#pragma push_macro("use_lock")
#pragma push_macro("enable_lock")
#pragma push_macro("disable_lock")
#pragma push_macro("use_mmap")
#pragma push_macro("enable_mmap")
#pragma push_macro("disable_mmap")
#pragma push_macro("use_noncontiguous")
#pragma push_macro("disable_contiguous")
#pragma push_macro("set_lock")
#pragma push_macro("page_align")
#pragma push_macro("granularity_align")
#pragma push_macro("mmap_align")
#pragma push_macro("SYS_ALLOC_PADDING")
#pragma push_macro("is_page_aligned")
#pragma push_macro("is_granularity_aligned")
#pragma push_macro("segment_holds")
#pragma push_macro("should_trim")
#pragma push_macro("TOP_FOOT_SIZE")
#pragma push_macro("PREACTION")
#pragma push_macro("POSTACTION")
#pragma push_macro("CORRUPTION_ERROR_ACTION")
#pragma push_macro("USAGE_ERROR_ACTION")
#pragma push_macro("check_free_chunk")
#pragma push_macro("check_inuse_chunk")
#pragma push_macro("check_malloced_chunk")
#pragma push_macro("check_mmapped_chunk")
#pragma push_macro("check_malloc_state")
#pragma push_macro("check_top_chunk")
#pragma push_macro("is_small")
#pragma push_macro("small_index")
#pragma push_macro("small_index2size")
#pragma push_macro("MIN_SMALL_INDEX")
#pragma push_macro("smallbin_at")
#pragma push_macro("treebin_at")
#pragma push_macro("compute_tree_index")
#pragma push_macro("bit_for_tree_index")
#pragma push_macro("leftshift_for_tree_index")
#pragma push_macro("minsize_for_tree_index")
#pragma push_macro("idx2bit")
#pragma push_macro("mark_smallmap")
#pragma push_macro("clear_smallmap")
#pragma push_macro("smallmap_is_marked")
#pragma push_macro("mark_treemap")
#pragma push_macro("clear_treemap")
#pragma push_macro("treemap_is_marked")
#pragma push_macro("least_bit")
#pragma push_macro("left_bits")
#pragma push_macro("same_or_left_bits")
#pragma push_macro("compute_bit2idx")
#pragma push_macro("ok_address")
#pragma push_macro("ok_next")
#pragma push_macro("ok_inuse")
#pragma push_macro("ok_pinuse")
#pragma push_macro("ok_magic")
#pragma push_macro("RTCHECK")
#pragma push_macro("mark_inuse_foot")
#pragma push_macro("set_inuse")
#pragma push_macro("set_inuse_and_pinuse")
#pragma push_macro("set_size_and_pinuse_of_inuse_chunk")
#pragma push_macro("get_mstate_for")
#pragma push_macro("insert_small_chunk")
#pragma push_macro("unlink_small_chunk")
#pragma push_macro("unlink_first_small_chunk")
#pragma push_macro("replace_dv")
#pragma push_macro("insert_large_chunk")
#pragma push_macro("unlink_large_chunk")
#pragma push_macro("insert_chunk")
#pragma push_macro("unlink_chunk")
#pragma push_macro("internal_malloc")
#pragma push_macro("internal_free")
#pragma push_macro("fm")
#pragma push_macro("MORECORE")
#pragma push_macro("MAX_POOL_ENTRIES")
#pragma push_macro("MINIMUM_MORECORE_SIZE")
#pragma push_macro("DL_MEM_COMMIT")
#pragma push_macro("DL_MEM_RESERVE")
#pragma push_macro("DL_MEM_RELEASE")
#pragma push_macro("DL_MEM_TOP_DOWN")
#pragma push_macro("DL_PAGE_READWRITE")
#pragma push_macro("DL_DEBUG_DEFINED")
#pragma push_macro("USE_DL_PREFIX")
#pragma push_macro("DL_SIZE_IMPL")
#pragma push_macro("s_allocated_memory")
#pragma push_macro("BOOST_CONTAINER_DL_WIDE_SMALLBINS")
#pragma push_macro("BOOST_CONTAINER_DL_REBASED_SMALLBINS")
#pragma push_macro("GET_TRUNCATED_SIZE")
#pragma push_macro("GET_ROUNDED_SIZE")
#pragma push_macro("GET_TRUNCATED_PO2_SIZE")
#pragma push_macro("GET_ROUNDED_PO2_SIZE")
#pragma push_macro("CALCULATE_GCD")
#pragma push_macro("CALCULATE_LCM")
#pragma push_macro("INTERNAL_MULTIALLOC_DEFAULT_CONTIGUOUS_MEM")
#pragma push_macro("SQRT_MAX_SIZE_T")
#pragma push_macro("BOOST_ALLOC_PLUS_MEMCHAIN_MEM_JUMP_NEXT")

#ifdef _WIN32
/* VirtualAlloc flags, values from <winnt.h> (which is never included here) */
#define DL_MEM_COMMIT        0x1000
#define DL_MEM_RESERVE        0x2000
#define DL_MEM_RELEASE        0x8000
#define DL_MEM_TOP_DOWN     0x100000
#define DL_PAGE_READWRITE       0x04
#endif //_WIN32

/* ------------------------- dlmalloc configuration ------------------------ */
#ifdef BOOST_CONTAINER_DLMALLOC_FOOTERS
#define FOOTERS      1
#endif

/* ----------------------- wide small bins (64-bit) ------------------------
   dlmalloc spaces its 32 small bins SMALLBIN_WIDTH = 8 bytes apart, which
   matches the chunk-size granularity of 32-bit targets (MALLOC_ALIGNMENT =
   2 * sizeof(void*) = 8). On 64-bit targets chunk sizes step by 16, so
   every bin at an odd multiple of 8 can never hold a chunk: half the bins
   are dead, exact-fit service ends at 240-byte chunks, and everything from
   256 bytes up pays the best-fit tree walk on both malloc and free.

   So on 64-bit the bins are re-spaced to the 64-bit granularity
   (SMALLBIN_SHIFT 3 -> 4), moving the small/large boundary in lockstep
   (TREEBIN_SHIFT 8 -> 9, so that 1 << TREEBIN_SHIFT ==
   NSMALLBINS << SMALLBIN_SHIFT still holds): all 32 bins become usable and
   chunks up to 496 bytes get exact O(1) bins instead of the tree.

   64-bit only: with 8-byte granularity a 16-byte spacing would make bins
   hold two different sizes, breaking the exact-fit invariant, so 32-bit
   targets keep dlmalloc's original spacing. */
#if defined(_WIN64) || defined(__LP64__) || defined(_LP64) || \
    (defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8)
#define SMALLBIN_SHIFT (4U)
#define TREEBIN_SHIFT  (9U)
#define BOOST_CONTAINER_DL_WIDE_SMALLBINS 1
#else
#define BOOST_CONTAINER_DL_WIDE_SMALLBINS 0
#endif

/* --------------------- re-based small bin index --------------------------
   dlmalloc indexes the small bins as size >> SMALLBIN_SHIFT, so bin i
   stands for size i * SMALLBIN_WIDTH and the bins below MIN_CHUNK_SIZE
   stand for sizes no chunk can have: bins 0-1 are dead on 32-bit
   (MIN_CHUNK_SIZE 16, width 8), and 0-3 on 64-bit at the default width
   (0-1 with WIDE_SMALLBINS). They are initialized, they occupy their two
   pointers each in malloc_state, and the bitmap logic shifts across them,
   but nothing can ever be linked there.

   The index is therefore re-based at MIN_CHUNK_SIZE, so bin 0 means
   MIN_CHUNK_SIZE and bin i means MIN_CHUNK_SIZE + i * SMALLBIN_WIDTH. Every
   bin becomes reachable and the exact-fit range grows by exactly the bins
   that were dead - two size classes on 32-bit and with the wide spacing
   above, four at dlmalloc's original 64-bit width (of which two are the
   usable even ones).

   The small/large boundary moves with it, to
   MIN_CHUNK_SIZE + (NSMALLBINS << SMALLBIN_SHIFT). */
#define BOOST_CONTAINER_DL_REBASED_SMALLBINS 1
/* MIN_CHUNK_SIZE, SMALLBIN_SHIFT and NSMALLBINS are defined by the core;
   these macros are expanded at their use sites, which all follow it */
#define is_small(s)         ((((s) - MIN_CHUNK_SIZE) >> SMALLBIN_SHIFT) < NSMALLBINS)
#define small_index(s)      (bindex_t)(((s) - MIN_CHUNK_SIZE) >> SMALLBIN_SHIFT)
#define small_index2size(i) (((size_t)(i) << SMALLBIN_SHIFT) + MIN_CHUNK_SIZE)
#define MIN_LARGE_SIZE      (MIN_CHUNK_SIZE + ((size_t)NSMALLBINS << SMALLBIN_SHIFT))
//Locking. USE_LOCKS == 2 is dlmalloc's documented hook for a user-supplied
//lock: it bypasses every lock routine dlmalloc defines and takes MLOCK_T plus
//INITIAL_LOCK/DESTROY_LOCK/ACQUIRE_LOCK/RELEASE_LOCK/TRY_LOCK from here, so
//not a line of dlmalloc has to change.
#define USE_LOCKS 2
#define MLOCK_T          ::boost::container::dtl::spin_mutex_t
#define INITIAL_LOCK(lk) (::boost::container::dtl::spin_mutex_init(lk), 0)
#define DESTROY_LOCK(lk) (0)
//ACQUIRE_LOCK must evaluate to 0 on success: PREACTION tests it.
//Every routine is elided while the process is single-threaded (see
//BOOST_CONTAINER_DL_SINGLE_THREADED above): the flag cannot change while an
//elided critical section runs, so acquire and release always agree.
#define ACQUIRE_LOCK(lk) (BOOST_CONTAINER_DL_SINGLE_THREADED ? 0 : (::boost::container::dtl::spin_mutex_lock(lk), 0))
#define RELEASE_LOCK(lk) (BOOST_CONTAINER_DL_SINGLE_THREADED ? (void)0 : ::boost::container::dtl::spin_mutex_unlock(lk))
//TRY_LOCK is non-zero when the lock was taken
#define TRY_LOCK(lk)     (BOOST_CONTAINER_DL_SINGLE_THREADED ? 1 : (::boost::container::dtl::spin_mutex_try_lock(lk) ? 1 : 0))

#define MSPACES      1
#define NO_MALLINFO  1
#define NO_MALLOC_STATS 1
//disable sbrk as it's deprecated in some systems and weakens ASLR
#define HAVE_MORECORE 0
#define DLMALLOC_VERSION 286

#if !defined(NDEBUG)
   #if !defined(DL_DEBUG)
      #define DL_DEBUG 1
      #define DL_DEBUG_DEFINED
   #endif
#endif

#define USE_DL_PREFIX

//Header-only: every dlmalloc entry point becomes inline (prototypes carry it)
#define DLMALLOC_EXPORT inline

#ifdef __GNUC__
#define FORCEINLINE inline
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable : 4127)
#pragma warning (disable : 4267)
#pragma warning (disable : 4702)
#pragma warning (disable : 4390) /*empty controlled statement found; is this the intent?*/
#pragma warning (disable : 4251 4231 4660) /*dll warnings*/
#pragma warning (disable : 4057) /*differs in indirection to slightly different base types from*/
#pragma warning (disable : 4018) /*signed/unsigned mismatch in the bin arithmetic*/
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
//dlmalloc's internal helpers are static: unused ones are expected in TUs
//that call only part of the API
#pragma GCC diagnostic ignored "-Wunused-function"
# if defined(__GNUC__) && !defined(__clang__) && \
     ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) >= 40800)
   //Disable false positives triggered by -Waggressive-loop-optimizations
#  pragma GCC diagnostic ignored "-Waggressive-loop-optimizations"
# endif
# if !defined(__clang__)
   //Disable false positives triggered by -Warray-bounds
#  pragma GCC diagnostic ignored "-Warray-bounds"
# endif
//dlmalloc is imported third-party C, re-imported wholesale whenever upstream
//moves, so its style is not ours to change. Three things it does everywhere
//are intrinsic to the algorithm rather than defects:
//  - reinterpreting raw storage as chunk headers and back (-Wcast-align,
//    -Wcast-qual); the allocator is what establishes that alignment in the
//    first place, so it knows the casts are good
//  - mixing size_t, bindex_t, binmap_t and int in the bin arithmetic
//    (-Wconversion, -Wsign-conversion, -Warith-conversion, -Wsign-compare)
//Silenced here rather than at the build level so a strict -Werror build of
//user code including this header still works, and so the rest of
//Boost.Container stays under the full warning set.
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif

namespace boost {
namespace container {
namespace dl_detail {

/* ------------- boost_cont_* API (inline: definitions follow) ------------- */
inline size_t boost_cont_size(const void *p);
inline void*  boost_cont_malloc(size_t bytes);
inline void   boost_cont_free(void* mem);
inline void*  boost_cont_memalign(size_t bytes, size_t alignment);
inline int    boost_cont_multialloc_nodes
   (size_t n_elements, size_t elem_size, size_t contiguous_elements, boost_cont_memchain *pchain);
inline int    boost_cont_multialloc_arrays
   (size_t n_elements, const size_t *sizes, size_t sizeof_element, size_t contiguous_elements, boost_cont_memchain *pchain);
inline void   boost_cont_multidealloc(boost_cont_memchain *pchain);
inline size_t boost_cont_allocated_memory(void);
inline size_t boost_cont_chunksize(const void *p);
inline int    boost_cont_all_deallocated(void);
inline boost_cont_malloc_stats_t boost_cont_malloc_stats(void);
inline size_t boost_cont_in_use_memory(void);
inline int    boost_cont_trim(size_t pad);
inline int    boost_cont_mallopt(int parameter_number, int parameter_value);
inline int    boost_cont_grow(void* oldmem, size_t minbytes, size_t maxbytes, size_t *received);
inline int    boost_cont_shrink(void* oldmem, size_t minbytes, size_t maxbytes, size_t *received, int do_commit);
inline void*  boost_cont_alloc(size_t minbytes, size_t preferred_bytes, size_t *received_bytes);
inline int    boost_cont_malloc_check(void);
inline boost_cont_command_ret_t boost_cont_allocation_command
   ( allocation_type command, size_t sizeof_object, size_t alignof_object
   , size_t limit_size, size_t preferred_size, size_t *received_size, void *reuse_ptr);

#include <boost/container/detail/dlmalloc_2_8_6.hpp>

#if BOOST_CONTAINER_DL_WIDE_SMALLBINS
/* The exact-fit invariant: a bin holds one chunk size only, so the bin
   spacing must equal the chunk-size granularity (guards against a
   user-overridden MALLOC_ALIGNMENT, which the preprocessor cannot see) */
typedef int boost_container_dl_wide_smallbins_need_16_byte_granularity
   [SMALLBIN_WIDTH == MALLOC_ALIGNMENT ? 1 : -1];
#endif

#if BOOST_CONTAINER_DL_REBASED_SMALLBINS
/* The re-based small range runs from MIN_CHUNK_SIZE and must hand over to
   the tree exactly where MIN_LARGE_SIZE says (no gap, no overlap) */
typedef int boost_container_dl_rebased_ranges_must_meet
   [MIN_LARGE_SIZE == (MIN_CHUNK_SIZE + ((size_t)NSMALLBINS << SMALLBIN_SHIFT)) ? 1 : -1];
/* ...and that boundary must fall inside the FIRST tree bin, i.e. in
   [1 << TREEBIN_SHIFT, minsize_for_tree_index(1)). Below it, large chunks
   would compute a tree index of 0 while being smaller than the bin's
   nominal minimum; at or above the second bin's minimum, tree bin 0 could
   never be filled and compute_tree_index's X == 0 shortcut would be
   reachable for real chunk sizes. */
typedef int boost_container_dl_rebased_boundary_in_first_treebin
   [(MIN_LARGE_SIZE >= (SIZE_T_ONE << TREEBIN_SHIFT) &&
     MIN_LARGE_SIZE <  minsize_for_tree_index(1)) ? 1 : -1];
#endif

#define DL_SIZE_IMPL(p) (chunksize(mem2chunk(p)) - overhead_for(mem2chunk(p)))


#define s_allocated_memory (dl_globals()->allocated_memory)

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//
//         SLIGHTLY MODIFIED DLMALLOC FUNCTIONS
//
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

//This function is equal to mspace_free
//replacing PREACTION with 0 and POSTACTION with nothing
static void mspace_free_lockless(mspace msp, void* mem)
{
  if (mem != 0) {
    mchunkptr p  = mem2chunk(mem);
#if FOOTERS
    mstate fm = get_mstate_for(p);
    (void)msp; /* placate people compiling -Wunused */
#else /* FOOTERS */
    mstate fm = (mstate)msp;
#endif /* FOOTERS */
    if (!ok_magic(fm)) {
      USAGE_ERROR_ACTION(fm, p);
      return;
    }
    if (!0){//PREACTION(fm)) {
      check_inuse_chunk(fm, p);
      if (RTCHECK(ok_address(fm, p) && ok_inuse(p))) {
        size_t psize = chunksize(p);
        mchunkptr next = chunk_plus_offset(p, psize);
        if (!pinuse(p)) {
          size_t prevsize = p->prev_foot;
          if (is_mmapped(p)) {
            psize += prevsize + MMAP_FOOT_PAD;
            if (CALL_MUNMAP((char*)p - prevsize, psize) == 0)
              fm->footprint -= psize;
            goto postaction;
          }
          else {
            mchunkptr prev = chunk_minus_offset(p, prevsize);
            psize += prevsize;
            p = prev;
            if (RTCHECK(ok_address(fm, prev))) { /* consolidate backward */
              if (p != fm->dv) {
                unlink_chunk(fm, p, prevsize);
              }
              else if ((next->head & INUSE_BITS) == INUSE_BITS) {
                fm->dvsize = psize;
                set_free_with_pinuse(p, psize, next);
                goto postaction;
              }
            }
            else
              goto erroraction;
          }
        }

        if (RTCHECK(ok_next(p, next) && ok_pinuse(next))) {
          if (!cinuse(next)) {  /* consolidate forward */
            if (next == fm->top) {
              size_t tsize = fm->topsize += psize;
              fm->top = p;
              p->head = tsize | PINUSE_BIT;
              if (p == fm->dv) {
                fm->dv = 0;
                fm->dvsize = 0;
              }
              if (should_trim(fm, tsize))
                sys_trim(fm, 0);
              goto postaction;
            }
            else if (next == fm->dv) {
              size_t dsize = fm->dvsize += psize;
              fm->dv = p;
              set_size_and_pinuse_of_free_chunk(p, dsize);
              goto postaction;
            }
            else {
              size_t nsize = chunksize(next);
              psize += nsize;
              unlink_chunk(fm, next, nsize);
              set_size_and_pinuse_of_free_chunk(p, psize);
              if (p == fm->dv) {
                fm->dvsize = psize;
                goto postaction;
              }
            }
          }
          else
            set_free_with_pinuse(p, psize, next);

          if (is_small(psize)) {
            insert_small_chunk(fm, p, psize);
            check_free_chunk(fm, p);
          }
          else {
            tchunkptr tp = (tchunkptr)p;
            insert_large_chunk(fm, tp, psize);
            check_free_chunk(fm, p);
            if (--fm->release_checks == 0)
              release_unused_segments(fm);
          }
          goto postaction;
        }
      }
    erroraction:
      USAGE_ERROR_ACTION(fm, p);
    postaction:
      ;//POSTACTION(fm);
    }
  }
}

//This function is equal to mspace_malloc
//replacing PREACTION with 0 and POSTACTION with nothing
static void* mspace_malloc_lockless(mspace msp, size_t bytes)
{
  mstate ms = (mstate)msp;
  if (!ok_magic(ms)) {
    USAGE_ERROR_ACTION(ms,ms);
    return 0;
  }
    if (!0){//PREACTION(ms)) {
    void* mem;
    size_t nb;
    if (bytes <= MAX_SMALL_REQUEST) {
      bindex_t idx;
      binmap_t smallbits;
      nb = (bytes < MIN_REQUEST)? MIN_CHUNK_SIZE : pad_request(bytes);
      idx = small_index(nb);
      smallbits = ms->smallmap >> idx;

      if ((smallbits & 0x3U) != 0) { /* Remainderless fit to a smallbin. */
        mchunkptr b, p;
        idx += ~smallbits & 1;       /* Uses next bin if idx empty */
        b = smallbin_at(ms, idx);
        p = b->fd;
        DL_ASSERT(chunksize(p) == small_index2size(idx));
        unlink_first_small_chunk(ms, b, p, idx);
        set_inuse_and_pinuse(ms, p, small_index2size(idx));
        mem = chunk2mem(p);
        check_malloced_chunk(ms, mem, nb);
        goto postaction;
      }

      else if (nb > ms->dvsize) {
        if (smallbits != 0) { /* Use chunk in next nonempty smallbin */
          mchunkptr b, p, r;
          size_t rsize;
          bindex_t i;
          binmap_t leftbits = (smallbits << idx) & left_bits(idx2bit(idx));
          binmap_t leastbit = least_bit(leftbits);
          compute_bit2idx(leastbit, i);
          b = smallbin_at(ms, i);
          p = b->fd;
          DL_ASSERT(chunksize(p) == small_index2size(i));
          unlink_first_small_chunk(ms, b, p, i);
          rsize = small_index2size(i) - nb;
          /* Fit here cannot be remainderless if 4byte sizes */
          if (SIZE_T_SIZE != 4 && rsize < MIN_CHUNK_SIZE)
            set_inuse_and_pinuse(ms, p, small_index2size(i));
          else {
            set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
            r = chunk_plus_offset(p, nb);
            set_size_and_pinuse_of_free_chunk(r, rsize);
            replace_dv(ms, r, rsize);
          }
          mem = chunk2mem(p);
          check_malloced_chunk(ms, mem, nb);
          goto postaction;
        }

        else if (ms->treemap != 0 && (mem = tmalloc_small(ms, nb)) != 0) {
          check_malloced_chunk(ms, mem, nb);
          goto postaction;
        }
      }
    }
    else if (bytes >= MAX_REQUEST)
      nb = MAX_SIZE_T; /* Too big to allocate. Force failure (in sys alloc) */
    else {
      nb = pad_request(bytes);
      if (ms->treemap != 0 && (mem = tmalloc_large(ms, nb)) != 0) {
        check_malloced_chunk(ms, mem, nb);
        goto postaction;
      }
    }

    if (nb <= ms->dvsize) {
      size_t rsize = ms->dvsize - nb;
      mchunkptr p = ms->dv;
      if (rsize >= MIN_CHUNK_SIZE) { /* split dv */
        mchunkptr r = ms->dv = chunk_plus_offset(p, nb);
        ms->dvsize = rsize;
        set_size_and_pinuse_of_free_chunk(r, rsize);
        set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
      }
      else { /* exhaust dv */
        size_t dvs = ms->dvsize;
        ms->dvsize = 0;
        ms->dv = 0;
        set_inuse_and_pinuse(ms, p, dvs);
      }
      mem = chunk2mem(p);
      check_malloced_chunk(ms, mem, nb);
      goto postaction;
    }

    else if (nb < ms->topsize) { /* Split top */
      size_t rsize = ms->topsize -= nb;
      mchunkptr p = ms->top;
      mchunkptr r = ms->top = chunk_plus_offset(p, nb);
      r->head = rsize | PINUSE_BIT;
      set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
      mem = chunk2mem(p);
      check_top_chunk(ms, ms->top);
      check_malloced_chunk(ms, mem, nb);
      goto postaction;
    }

    mem = sys_alloc(ms, nb);

  postaction:
      ;//POSTACTION(ms);
    return mem;
  }

  return 0;
}

//This function is equal to internal_memalign, but the caller must already
//hold the mspace lock: the oversized allocation goes through
//mspace_malloc_lockless and the PREACTION/POSTACTION pair that internal_memalign
//wraps around the realign/trim is dropped. Keeping the whole operation inside
//the caller's critical section saves three lock round-trips per over-aligned
//request, and removes the window where a failed relock had to report the
//allocation without ever running the chunk bookkeeping.
//
//The leak-repair path internal_memalign needs (freeing the block when its own
//PREACTION fails) is unnecessary here for the same reason: there is no relock
//that can fail.
static void* mspace_memalign_lockless(mstate m, size_t alignment, size_t bytes)
{
  void* mem = 0;
  if (alignment <  MIN_CHUNK_SIZE) /* must be at least a minimum chunk size */
    alignment = MIN_CHUNK_SIZE;
  if ((alignment & (alignment-SIZE_T_ONE)) != 0) {/* Ensure a power of 2 */
    size_t a = MALLOC_ALIGNMENT << 1;
    while (a < alignment) a <<= 1;
    alignment = a;
  }
  if (bytes >= MAX_REQUEST - alignment) {
    if (m != 0)  { /* Test isn't needed but avoids compiler warning */
      MALLOC_FAILURE_ACTION;
    }
  }
  else {
    size_t nb = request2size(bytes);
    size_t req = nb + alignment + MIN_CHUNK_SIZE - CHUNK_OVERHEAD;
    mem = mspace_malloc_lockless(m, req);
    if (mem != 0) {
      mchunkptr p = mem2chunk(mem);
      if ((((size_t)(mem)) & (alignment - 1)) != 0) { /* misaligned */
        /*
          Find an aligned spot inside chunk.  Since we need to give
          back leading space in a chunk of at least MIN_CHUNK_SIZE, if
          the first calculation places us at a spot with less than
          MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
          We've allocated enough total room so that this is always
          possible.
        */
        char* br = (char*)mem2chunk((size_t)(((size_t)((char*)mem + alignment -
                                                       SIZE_T_ONE)) &
                                             (0 - alignment)));
        char* pos = ((size_t)(br - (char*)(p)) >= MIN_CHUNK_SIZE)?
          br : br+alignment;
        mchunkptr newp = (mchunkptr)pos;
        size_t leadsize = pos - (char*)(p);
        size_t newsize = chunksize(p) - leadsize;

        if (is_mmapped(p)) { /* For mmapped chunks, just adjust offset */
          newp->prev_foot = p->prev_foot + leadsize;
          newp->head = newsize;
        }
        else { /* Otherwise, give back leader, use the rest */
          set_inuse(m, newp, newsize);
          set_inuse(m, p, leadsize);
          dispose_chunk(m, p, leadsize);
        }
        p = newp;
      }

      /* Give back spare room at the end */
      if (!is_mmapped(p)) {
        size_t size = chunksize(p);
        if (size > nb + MIN_CHUNK_SIZE) {
          size_t remainder_size = size - nb;
          mchunkptr remainder = chunk_plus_offset(p, nb);
          set_inuse(m, p, nb);
          set_inuse(m, remainder, remainder_size);
          dispose_chunk(m, remainder, remainder_size);
        }
      }

      mem = chunk2mem(p);
      DL_ASSERT (chunksize(p) >= nb);
      DL_ASSERT(((size_t)mem & (alignment - 1)) == 0);
      check_inuse_chunk(m, p);
      /*No POSTACTION: the caller keeps the lock it already held.*/
    }
  }
  return mem;
}

//This function is equal to try_realloc_chunk but handling
//minimum and desired bytes
static mchunkptr try_realloc_chunk_with_min(mstate m, mchunkptr p, size_t min_nb, size_t des_nb, int can_move)
{
  mchunkptr newp = 0;
  size_t oldsize = chunksize(p);
  mchunkptr next = chunk_plus_offset(p, oldsize);
  if (RTCHECK(ok_address(m, p) && ok_inuse(p) &&
              ok_next(p, next) && ok_pinuse(next))) {
    if (is_mmapped(p)) {
      newp = mmap_resize(m, p, des_nb, can_move);
      if(!newp)   //mmap does not return how many bytes we could reallocate, so go the minimum
         newp = mmap_resize(m, p, min_nb, can_move);
    }
    else if (oldsize >= min_nb) {             /* already big enough */
      size_t nb = oldsize >= des_nb ? des_nb : oldsize;
      size_t rsize = oldsize - nb;
      if (rsize >= MIN_CHUNK_SIZE) {      /* split off remainder */
        mchunkptr r = chunk_plus_offset(p, nb);
        set_inuse(m, p, nb);
        set_inuse(m, r, rsize);
        dispose_chunk(m, r, rsize);
      }
      newp = p;
    }
    else if (next == m->top) {  /* extend into top */
      if (oldsize + m->topsize > min_nb) {
        size_t nb = (oldsize + m->topsize) > des_nb ? des_nb : (oldsize + m->topsize - MALLOC_ALIGNMENT);
        size_t newsize = oldsize + m->topsize;
        size_t newtopsize = newsize - nb;
        mchunkptr newtop = chunk_plus_offset(p, nb);
        set_inuse(m, p, nb);
        newtop->head = newtopsize |PINUSE_BIT;
        m->top = newtop;
        m->topsize = newtopsize;
        newp = p;
      }
    }
    else if (next == m->dv) { /* extend into dv */
      size_t dvs = m->dvsize;
      if (oldsize + dvs >= min_nb) {
        size_t nb = (oldsize + dvs) >= des_nb ? des_nb : (oldsize + dvs);
        size_t dsize = oldsize + dvs - nb;
        if (dsize >= MIN_CHUNK_SIZE) {
          mchunkptr r = chunk_plus_offset(p, nb);
          mchunkptr n = chunk_plus_offset(r, dsize);
          set_inuse(m, p, nb);
          set_size_and_pinuse_of_free_chunk(r, dsize);
          clear_pinuse(n);
          m->dvsize = dsize;
          m->dv = r;
        }
        else { /* exhaust dv */
          size_t newsize = oldsize + dvs;
          set_inuse(m, p, newsize);
          m->dvsize = 0;
          m->dv = 0;
        }
        newp = p;
      }
    }
    else if (!cinuse(next)) { /* extend into next free chunk */
      size_t nextsize = chunksize(next);
      if (oldsize + nextsize >= min_nb) {
        size_t nb = (oldsize + nextsize) >= des_nb ? des_nb : (oldsize + nextsize);
        size_t rsize = oldsize + nextsize - nb;
        unlink_chunk(m, next, nextsize);
        if (rsize < MIN_CHUNK_SIZE) {
          size_t newsize = oldsize + nextsize;
          set_inuse(m, p, newsize);
        }
        else {
          mchunkptr r = chunk_plus_offset(p, nb);
          set_inuse(m, p, nb);
          set_inuse(m, r, rsize);
          dispose_chunk(m, r, rsize);
        }
        newp = p;
      }
    }
  }
  else {
    USAGE_ERROR_ACTION(m, chunk2mem(p));
  }
  return newp;
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//
//         NEW FUNCTIONS BASED ON DLMALLOC INTERNALS
//
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

#define GET_TRUNCATED_SIZE(ORIG_SIZE, ROUNDTO)     ((ORIG_SIZE)/(ROUNDTO)*(ROUNDTO))
#define GET_ROUNDED_SIZE(ORIG_SIZE, ROUNDTO)       ((((ORIG_SIZE)-1)/(ROUNDTO)+1)*(ROUNDTO))
#define GET_TRUNCATED_PO2_SIZE(ORIG_SIZE, ROUNDTO) ((ORIG_SIZE) & (~(ROUNDTO-1)))
#define GET_ROUNDED_PO2_SIZE(ORIG_SIZE, ROUNDTO)   (((ORIG_SIZE - 1) & (~(ROUNDTO-1))) + ROUNDTO)

/* Greatest common divisor and least common multiple
   gcd is an algorithm that calculates the greatest common divisor of two
   integers, using Euclid's algorithm.

   Pre: A > 0 && B > 0
   Recommended: A > B*/
#define CALCULATE_GCD(A, B, OUT)\
{\
   size_t a = A;\
   size_t b = B;\
   do\
   {\
      size_t tmp = b;\
      b = a % b;\
      a = tmp;\
   } while (b != 0);\
\
   OUT = a;\
}

/* lcm is an algorithm that calculates the least common multiple of two
   integers.

   Pre: A > 0 && B > 0
   Recommended: A > B*/
#define CALCULATE_LCM(A, B, OUT)\
{\
   CALCULATE_GCD(A, B, OUT);\
   OUT = (A / OUT)*B;\
}

static int calculate_lcm_and_needs_backwards_lcmed
   (size_t backwards_multiple, size_t received_size, size_t size_to_achieve,
    size_t *plcm, size_t *pneeds_backwards_lcmed)
{
   /* Now calculate lcm */
   size_t max = backwards_multiple;
   size_t min = MALLOC_ALIGNMENT;
   size_t needs_backwards;
   size_t needs_backwards_lcmed;
   size_t lcm;
   size_t current_forward;
   /*Swap if necessary*/
   if(max < min){
      size_t tmp = min;
      min = max;
      max = tmp;
   }
   /*Check if it's power of two*/
   if((backwards_multiple & (backwards_multiple-1)) == 0){
      if(0 != (size_to_achieve & ((backwards_multiple-1)))){
         return 0;
      }

      lcm = max;
      /*If we want to use minbytes data to get a buffer between maxbytes
      and minbytes if maxbytes can't be achieved, calculate the
      biggest of all possibilities*/
      current_forward = GET_TRUNCATED_PO2_SIZE(received_size, backwards_multiple);
      needs_backwards = size_to_achieve - current_forward;
      DL_ASSERT((needs_backwards % backwards_multiple) == 0);
      needs_backwards_lcmed = GET_ROUNDED_PO2_SIZE(needs_backwards, lcm);
      *plcm = lcm;
      *pneeds_backwards_lcmed = needs_backwards_lcmed;
      return 1;
   }
   /*Check if it's multiple of alignment*/
   else if((backwards_multiple & (MALLOC_ALIGNMENT - 1u)) == 0){
      lcm = backwards_multiple;
      current_forward = GET_TRUNCATED_SIZE(received_size, backwards_multiple);
      //No need to round needs_backwards because backwards_multiple == lcm
      needs_backwards_lcmed = needs_backwards = size_to_achieve - current_forward;
      DL_ASSERT((needs_backwards_lcmed & (MALLOC_ALIGNMENT - 1u)) == 0);
      *plcm = lcm;
      *pneeds_backwards_lcmed = needs_backwards_lcmed;
      return 1;
   }
   /*Check if it's multiple of the half of the alignmment*/
   else if((backwards_multiple & ((MALLOC_ALIGNMENT/2u) - 1u)) == 0){
      lcm = backwards_multiple*2u;
      current_forward = GET_TRUNCATED_SIZE(received_size, backwards_multiple);
      needs_backwards_lcmed = needs_backwards = size_to_achieve - current_forward;
      if(0 != (needs_backwards_lcmed & (MALLOC_ALIGNMENT-1)))
      //while(0 != (needs_backwards_lcmed & (MALLOC_ALIGNMENT-1)))
         needs_backwards_lcmed += backwards_multiple;
      DL_ASSERT((needs_backwards_lcmed % lcm) == 0);
      *plcm = lcm;
      *pneeds_backwards_lcmed = needs_backwards_lcmed;
      return 1;
   }
   /*Check if it's multiple of the quarter of the alignmment*/
   else if((backwards_multiple & ((MALLOC_ALIGNMENT/4u) - 1u)) == 0){
      size_t remainder;
      lcm = backwards_multiple*4u;
      current_forward = GET_TRUNCATED_SIZE(received_size, backwards_multiple);
      needs_backwards_lcmed = needs_backwards = size_to_achieve - current_forward;
      //while(0 != (needs_backwards_lcmed & (MALLOC_ALIGNMENT-1)))
         //needs_backwards_lcmed += backwards_multiple;
      if(0 != (remainder = ((needs_backwards_lcmed & (MALLOC_ALIGNMENT-1))>>(MALLOC_ALIGNMENT/8u)))){
         if(backwards_multiple & MALLOC_ALIGNMENT/2u){
            needs_backwards_lcmed += (remainder)*backwards_multiple;
         }
         else{
            needs_backwards_lcmed += (4-remainder)*backwards_multiple;
         }
      }
      DL_ASSERT((needs_backwards_lcmed % lcm) == 0);
      *plcm = lcm;
      *pneeds_backwards_lcmed = needs_backwards_lcmed;
      return 1;
   }
   else{
      CALCULATE_LCM(max, min, lcm);
      /*If we want to use minbytes data to get a buffer between maxbytes
      and minbytes if maxbytes can't be achieved, calculate the
      biggest of all possibilities*/
      current_forward = GET_TRUNCATED_SIZE(received_size, backwards_multiple);
      needs_backwards = size_to_achieve - current_forward;
      DL_ASSERT((needs_backwards % backwards_multiple) == 0);
      needs_backwards_lcmed = GET_ROUNDED_SIZE(needs_backwards, lcm);
      *plcm = lcm;
      *pneeds_backwards_lcmed = needs_backwards_lcmed;
      return 1;
   }
}

/* Largest chunk size p could reach by absorbing only the free space that
   follows it, without moving. It mirrors the cases
   try_realloc_chunk_with_min() can actually satisfy, so asking that function
   for exactly this size always succeeds. Returns chunksize(p) when there is
   nothing to take. */
static size_t internal_max_fwd_chunk_size(mstate m, mchunkptr p)
{
   const size_t oldsize = chunksize(p);
   mchunkptr next;
   if(is_mmapped(p))
      return oldsize;   /* those grow through mmap_resize instead */
   next = chunk_plus_offset(p, oldsize);
   if(!RTCHECK(ok_next(p, next) && ok_pinuse(next)))
      return oldsize;
   if(next == m->top)
      /* top keeps MALLOC_ALIGNMENT, exactly as try_realloc_chunk_with_min
         reserves when it cannot honour the desired size */
      return (m->topsize > MALLOC_ALIGNMENT) ? (oldsize + m->topsize - MALLOC_ALIGNMENT) : oldsize;
   else if(next == m->dv)
      return oldsize + m->dvsize;
   else if(!cinuse(next))
      return oldsize + chunksize(next);
   return oldsize;
}

static void *internal_grow_both_sides
                         (mstate m
                         ,allocation_type command
                         ,void *oldmem
                         ,size_t minbytes
                         ,size_t maxbytes
                         ,size_t *received_size
                         ,size_t backwards_multiple
                         ,int only_preferred_backwards)
{
   mchunkptr oldp = mem2chunk(oldmem);
   size_t oldsize = chunksize(oldp);
   *received_size = oldsize - overhead_for(oldp);
   if(minbytes <= *received_size)
      return oldmem;

   if (RTCHECK(ok_address(m, oldp) && ok_inuse(oldp))) {
      if(command & BOOST_CONTAINER_EXPAND_FWD){
         if(try_realloc_chunk_with_min(m, oldp, request2size(minbytes), request2size(maxbytes), 0)){
            check_inuse_chunk(m, oldp);
            *received_size = DL_SIZE_IMPL(oldmem);
            s_allocated_memory += chunksize(oldp) - oldsize;
            return oldmem;
         }
      }
      else{
         *received_size = DL_SIZE_IMPL(oldmem);
         if(*received_size >= maxbytes)
            return oldmem;
      }
/*
      Should we check this?
      if(backwards_multiple &&
         (0 != (minbytes % backwards_multiple) &&
          0 != (maxbytes % backwards_multiple)) ){
         USAGE_ERROR_ACTION(m, oldp);
         return 0;
      }
*/
      /* We reach here only if forward expansion fails */
      if(!(command & BOOST_CONTAINER_EXPAND_BWD) || pinuse(oldp)){
         return 0;
      }
      {
         size_t prevsize = oldp->prev_foot;
         if ((prevsize & USE_MMAP_BIT) != 0){
            /*Return failure the previous chunk was mmapped.
              mremap does not allow expanding to a fixed address (MREMAP_MAYMOVE) without
              copying (MREMAP_MAYMOVE must be also set).*/
            return 0;
         }
         else {
            mchunkptr prev = chunk_minus_offset(oldp, prevsize);
            size_t dsize = oldsize + prevsize;
            size_t needs_backwards_lcmed;
            size_t lcm;
            /* Forward expansion could not reach minbytes on its own, but the
               space that *is* free in front still counts towards the total, so
               the previous chunk only has to cover the remainder. Measure that
               space without taking it: should the two sides together still fall
               short, the block has to be left exactly as it was. */
            size_t fwd_chunk = oldsize;
            size_t fwd_size  = *received_size;
            if(command & BOOST_CONTAINER_EXPAND_FWD){
               const size_t max_fwd = internal_max_fwd_chunk_size(m, oldp);
               if(max_fwd > oldsize){
                  fwd_chunk = max_fwd;
                  fwd_size  = max_fwd - overhead_for(oldp);
               }
            }

            /* Let's calculate the number of extra bytes of data before the current
            block's begin. The value is a multiple of backwards_multiple
            and the alignment*/
            if(!calculate_lcm_and_needs_backwards_lcmed
               ( backwards_multiple, fwd_size
               , only_preferred_backwards ? maxbytes : minbytes
               , &lcm, &needs_backwards_lcmed)
               || !RTCHECK(ok_address(m, prev))){
               USAGE_ERROR_ACTION(m, oldp);
               return 0;
            }
            /* Check if previous block has enough size */
            else if(prevsize < needs_backwards_lcmed){
               /* preferred size? */
               return 0;
            }
            /* Both halves are available, so now take the forward space. Sized
               from internal_max_fwd_chunk_size(), so this cannot fail. */
            if(fwd_chunk > oldsize){
               if(!try_realloc_chunk_with_min(m, oldp, fwd_chunk, fwd_chunk, 0)){
                  DL_ASSERT(0);
               }
               check_inuse_chunk(m, oldp);
               *received_size = DL_SIZE_IMPL(oldmem);
               s_allocated_memory += chunksize(oldp) - oldsize;
               oldsize = chunksize(oldp);
               dsize = oldsize + prevsize;
            }
            /* We need a minimum size to split the previous one */
            if(prevsize >= (needs_backwards_lcmed + MIN_CHUNK_SIZE)){
               mchunkptr r  = chunk_minus_offset(oldp, needs_backwards_lcmed);
               size_t rsize = oldsize + needs_backwards_lcmed;
               size_t newprevsize = dsize - rsize;
               int prev_was_dv = prev == m->dv;

               DL_ASSERT(newprevsize >= MIN_CHUNK_SIZE);

               if (prev_was_dv) {
                  m->dvsize = newprevsize;
               }
               else{/* if ((next->head & INUSE_BITS) == INUSE_BITS) { */
                  unlink_chunk(m, prev, prevsize);
                  insert_chunk(m, prev, newprevsize);
               }

               set_size_and_pinuse_of_free_chunk(prev, newprevsize);
               clear_pinuse(r);
               set_inuse(m, r, rsize);
               check_malloced_chunk(m, chunk2mem(r), rsize);
               *received_size = chunksize(r) - overhead_for(r);
               s_allocated_memory += chunksize(r) - oldsize;
               return chunk2mem(r);
            }
            /* Check if there is no place to create a new block and
               the whole new block is multiple of the backwards expansion multiple */
            else if(prevsize >= needs_backwards_lcmed && !(prevsize % lcm)) {
               /* Just merge the whole previous block */
               /* prevsize is multiple of lcm (and backwards_multiple)*/
               *received_size  += prevsize;

               if (prev != m->dv) {
                  unlink_chunk(m, prev, prevsize);
               }
               else{
                  m->dvsize = 0;
                  m->dv     = 0;
               }
               set_inuse(m, prev, dsize);
               check_malloced_chunk(m, chunk2mem(prev), dsize);
               s_allocated_memory += chunksize(prev) - oldsize;
               return chunk2mem(prev);
            }
            else{
               /* Previous block was big enough but there is no room
                  to create an empty block and taking the whole block does
                  not fulfill alignment requirements */
               return 0;
            }
         }
      }
   }
   else{
      USAGE_ERROR_ACTION(m, oldmem);
      return 0;
   }
}

/* This is similar to mmap_resize but:
   * Only to shrink
   * It takes min and max sizes
   * Takes additional 'do_commit' argument to obtain the final
     size before doing the real shrink operation.
*/
static int internal_mmap_shrink_in_place(mstate m, mchunkptr oldp, size_t nbmin, size_t nbmax, size_t *received_size, int do_commit)
{
  size_t oldsize = chunksize(oldp);
  *received_size = oldsize;
  #if HAVE_MREMAP
  if (is_small(nbmax)) /* Can't shrink mmap regions below small size */
    return 0;
  {
   size_t effective_min = nbmin > MIN_LARGE_SIZE ? nbmin : MIN_LARGE_SIZE;
   /* Keep old chunk if big enough but not too big */
   if (oldsize >= effective_min + SIZE_T_SIZE &&
         (oldsize - effective_min) <= (mparams.granularity << 1))
      return 0;
   /* Now calculate new sizes */
   {
      size_t offset = oldp->prev_foot;
      size_t oldmmsize = oldsize + offset + MMAP_FOOT_PAD;
      size_t newmmsize = mmap_align(effective_min + SIX_SIZE_T_SIZES + CHUNK_ALIGN_MASK);
      *received_size = newmmsize;
      if(!do_commit){
         const int flags = 0; /* placate people compiling -Wunused */
         char* cp = (char*)CALL_MREMAP((char*)oldp - offset,
                                       oldmmsize, newmmsize, flags);
         /*This must always succeed */
         if(!cp){
            USAGE_ERROR_ACTION(m, m);
            return 0;
         }
         {
         mchunkptr newp = (mchunkptr)(cp + offset);
         size_t psize = newmmsize - offset - MMAP_FOOT_PAD;
         newp->head = psize;
         mark_inuse_foot(m, newp, psize);
         chunk_plus_offset(newp, psize)->head = FENCEPOST_HEAD;
         chunk_plus_offset(newp, psize+SIZE_T_SIZE)->head = 0;

         if (cp < m->least_addr)
            m->least_addr = cp;
         if ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
            m->max_footprint = m->footprint;
         check_mmapped_chunk(m, newp);
         }
      }
    }
    return 1;
  }
  #else  //#if HAVE_MREMAP
  (void)m;
  (void)oldp;
  (void)nbmin;
  (void)nbmax;
  (void)received_size;
  (void)do_commit;
  return 0;
  #endif //#if HAVE_MREMAP
}

static int internal_shrink(mstate m, void* oldmem, size_t minbytes, size_t maxbytes, size_t *received_size, int do_commit)
{
   *received_size = chunksize(mem2chunk(oldmem)) - overhead_for(mem2chunk(oldmem));
   if (minbytes >= MAX_REQUEST || maxbytes >= MAX_REQUEST) {
      MALLOC_FAILURE_ACTION;
      return 0;
   }
   else if(minbytes < MIN_REQUEST){
      minbytes = MIN_REQUEST;
   }
   if (minbytes > maxbytes) {
      return 0;
   }

   {
      mchunkptr oldp = mem2chunk(oldmem);
      size_t oldsize = chunksize(oldp);
      mchunkptr next = chunk_plus_offset(oldp, oldsize);
      void* extra = 0;

      /* Try to either shrink or extend into top. Else malloc-copy-free*/
      if (RTCHECK(ok_address(m, oldp) && ok_inuse(oldp) &&
                  ok_next(oldp, next) && ok_pinuse(next))) {
         size_t nbmin = request2size(minbytes);
         size_t nbmax = request2size(maxbytes);

         if (nbmin > oldsize){
            /* Return error if old size is too small */
         }
         else if (is_mmapped(oldp)){
            return internal_mmap_shrink_in_place(m, oldp, nbmin, nbmax, received_size, do_commit);
         }
         else{ // nbmin <= oldsize /* already big enough*/
            size_t nb = nbmin;
            size_t rsize = oldsize - nb;
            if (rsize >= MIN_CHUNK_SIZE) {
               if(do_commit){
                  mchunkptr remainder = chunk_plus_offset(oldp, nb);
                  set_inuse(m, oldp, nb);
                  set_inuse(m, remainder, rsize);
                  s_allocated_memory -= rsize;
                  extra = chunk2mem(remainder);
                  mspace_free_lockless(m, extra);
                  check_inuse_chunk(m, oldp);
               }
               *received_size = nb - overhead_for(oldp);
               return 1;
            }
         }
      }
      else {
         USAGE_ERROR_ACTION(m, oldmem);
      }
      return 0;
   }
}

#define INTERNAL_MULTIALLOC_DEFAULT_CONTIGUOUS_MEM 4096
#define SQRT_MAX_SIZE_T           (((size_t)-1)>>(sizeof(size_t)*CHAR_BIT/2))

/* Failure unwind shared by the two multialloc functions below.

   Two things have to be true afterwards: only the blocks *this call* linked
   in are returned to the heap (the chain may already carry blocks belonging
   to the caller), and the chain is left exactly as it was on entry - it must
   not keep pointing at blocks that are back in the heap, or the caller's
   next deallocate_many() double-frees them.

   Each memchain node lives inside the block it describes, so a node's 'next'
   is read before that block is freed. Freeing block k cannot disturb block
   k+1's node either: k+1 is still in use, and dlmalloc only coalesces with
   free neighbours. */
static void internal_multialloc_rollback
   (mstate m, boost_cont_memchain *pchain,
    boost_cont_memchain_it entry_last_it, size_t entry_num_mem)
{
   boost_cont_memchain_it it = entry_last_it;
   BOOST_CONTAINER_MEMIT_NEXT(it);
   while(!BOOST_CONTAINER_MEMCHAIN_IS_END_IT(pchain, it)){
      void *addr = BOOST_CONTAINER_MEMIT_ADDR(it);
      BOOST_CONTAINER_MEMIT_NEXT(it);
      s_allocated_memory -= chunksize(mem2chunk(addr));
      mspace_free_lockless(m, addr);
   }
   /* Re-terminate at the entry tail. When the chain arrived empty this is the
      root node, which restores the pristine MEMCHAIN_INIT state. */
   entry_last_it.node_ptr->next_node_ptr = 0;
   pchain->last_node_ptr = entry_last_it.node_ptr;
   pchain->num_mem       = entry_num_mem;
}

static int internal_node_multialloc
(mstate m, size_t n_elements, size_t element_size, size_t contiguous_elements, boost_cont_memchain *pchain) {
	void*     mem;            /* malloced aggregate space */
	mchunkptr p;              /* corresponding chunk */
	size_t    remainder_size; /* remaining bytes while splitting */
	flag_t    was_enabled;    /* to disable mmap */
	size_t    elements_per_segment = 0;
	size_t    element_req_size = request2size(element_size);
	boost_cont_memchain_it prev_last_it = BOOST_CONTAINER_MEMCHAIN_LAST_IT(pchain);
	size_t prev_num_mem = BOOST_CONTAINER_MEMCHAIN_SIZE(pchain);
	/* The count that actually gets multiplied by element_req_size is the
	   per-segment one, and that is what the overflow test below has to use.
	   For ALL_CONTIGUOUS one segment holds everything, so it is n_elements;
	   DEFAULT_CONTIGUOUS derives its count from a fixed byte budget and can
	   never overflow, so nothing needs checking. Testing
	   contiguous_elements itself would reject ALL_CONTIGUOUS outright,
	   the sentinel being (size_t)-1. */
	size_t max_seg_elements =
		(contiguous_elements == BOOST_CONTAINER_DL_MULTIALLOC_ALL_CONTIGUOUS)     ? n_elements :
		(contiguous_elements == BOOST_CONTAINER_DL_MULTIALLOC_DEFAULT_CONTIGUOUS) ? 0u        :
														  contiguous_elements;

	/*Error if wrong element_size parameter */
	if (!element_size ||
		/*OR Error if n_elements less than contiguous_elements */
		((contiguous_elements + 1) > (BOOST_CONTAINER_DL_MULTIALLOC_DEFAULT_CONTIGUOUS + 1) && n_elements < contiguous_elements) ||
		/* OR Error if integer overflow */
		(max_seg_elements &&
		 SQRT_MAX_SIZE_T < (element_req_size | max_seg_elements) &&
		 (MAX_SIZE_T / element_req_size) < max_seg_elements)) {
		return 0;
	}
	switch (contiguous_elements) {
	case BOOST_CONTAINER_DL_MULTIALLOC_DEFAULT_CONTIGUOUS:
	{
		/* Default contiguous, just check that we can store at least one element */
		elements_per_segment = INTERNAL_MULTIALLOC_DEFAULT_CONTIGUOUS_MEM / element_req_size;
		elements_per_segment += (size_t)(!elements_per_segment);
	}
	break;
	case BOOST_CONTAINER_DL_MULTIALLOC_ALL_CONTIGUOUS:
		/* All elements should be allocated in a single call */
		elements_per_segment = n_elements;
		break;
	default:
		/* Allocate in chunks of "contiguous_elements" */
		elements_per_segment = contiguous_elements;
	}

	{
		size_t    i;
		size_t next_i;
		/*
		   Allocate the aggregate chunk.  First disable direct-mmapping so
		   malloc won't use it, since we would not be able to later
		   free/realloc space internal to a segregated mmap region.
		*/
		was_enabled = use_mmap(m);
		disable_mmap(m);
		for (i = 0; i != n_elements; i = next_i)
		{
			size_t accum_size;
			size_t n_elements_left = n_elements - i;
			next_i = i + ((n_elements_left < elements_per_segment) ? n_elements_left : elements_per_segment);
			accum_size = element_req_size * (next_i - i);

			mem = mspace_malloc_lockless(m, accum_size - CHUNK_OVERHEAD);
			if (mem == 0) {
				internal_multialloc_rollback(m, pchain, prev_last_it, prev_num_mem);
				if (was_enabled)
					enable_mmap(m);
				return 0;
			}
			p = mem2chunk(mem);
			remainder_size = chunksize(p);
			s_allocated_memory += remainder_size;

			DL_ASSERT(!is_mmapped(p));
			{  /* split out elements */
				/* Each element's first word IS its memchain node, so the run is
				   linked in place with one store per element and spliced onto the
				   chain with a single INCORPORATE_AFTER, instead of a PUSH_BACK
				   per element (which also rewrites last_node_ptr and num_mem
				   every time). The arrays variant below does the same. */
				void *mem_orig = mem;
				boost_cont_memchain_it last_it = BOOST_CONTAINER_MEMCHAIN_LAST_IT(pchain);
				size_t num_elements = next_i - i;

				size_t num_loops = num_elements - 1;
				remainder_size -= element_req_size * num_loops;
				while (num_loops) {
					void **mem_prev = ((void**)mem);
					--num_loops;
					set_size_and_pinuse_of_inuse_chunk(m, p, element_req_size);
					p = chunk_plus_offset(p, element_req_size);
					mem = chunk2mem(p);
					*mem_prev = mem;
				}
				set_size_and_pinuse_of_inuse_chunk(m, p, remainder_size);
				/* mem is the last element; INCORPORATE_AFTER terminates it */
				BOOST_CONTAINER_MEMCHAIN_INCORPORATE_AFTER(pchain, last_it, mem_orig, mem, num_elements);
			}
		}
		if (was_enabled)
			enable_mmap(m);
	}
	return 1;
}

#define BOOST_ALLOC_PLUS_MEMCHAIN_MEM_JUMP_NEXT(THISMEM, NEXTMEM) \
   *((void**)(THISMEM)) = *((void**)((NEXTMEM)))

//This function is based on internal_bulk_free
//replacing iteration over array[] with boost_cont_memchain.
//Instead of returning the unallocated nodes, returns a chain of non-deallocated nodes.
//After forward merging, backwards merging is also tried
static void internal_multialloc_free(mstate m, boost_cont_memchain *pchain)
{
#if FOOTERS
	boost_cont_memchain ret_chain;
	BOOST_CONTAINER_MEMCHAIN_INIT(&ret_chain);
#endif
	if (!PREACTION(m)) {
		boost_cont_memchain_it a_it = BOOST_CONTAINER_MEMCHAIN_BEGIN_IT(pchain);
		while (!BOOST_CONTAINER_MEMCHAIN_IS_END_IT(pchain, a_it)) { /* Iterate though all memory holded by the chain */
			void* a_mem = BOOST_CONTAINER_MEMIT_ADDR(a_it);
			mchunkptr a_p = mem2chunk(a_mem);
			size_t psize = chunksize(a_p);
#if FOOTERS
			if (get_mstate_for(a_p) != m) {
				BOOST_CONTAINER_MEMIT_NEXT(a_it);
				BOOST_CONTAINER_MEMCHAIN_PUSH_BACK(&ret_chain, a_mem);
				continue;
			}
#endif
			check_inuse_chunk(m, a_p);
			if (RTCHECK(ok_address(m, a_p) && ok_inuse(a_p))) {
				while (1) { /* Internal loop to speed up forward and backward merging (avoids some redundant checks) */
					boost_cont_memchain_it b_it = a_it;
					BOOST_CONTAINER_MEMIT_NEXT(b_it);
					if (!BOOST_CONTAINER_MEMCHAIN_IS_END_IT(pchain, b_it)) {
						void *b_mem = BOOST_CONTAINER_MEMIT_ADDR(b_it);
						mchunkptr b_p = mem2chunk(b_mem);
						if (b_p == next_chunk(a_p)) { /* b chunk is contiguous and next so b's size can be added to a */
							psize += chunksize(b_p);
							set_inuse(m, a_p, psize);
							BOOST_ALLOC_PLUS_MEMCHAIN_MEM_JUMP_NEXT(a_mem, b_mem);
							continue;
						}
						if (RTCHECK(ok_address(m, b_p) && ok_inuse(b_p))) {
							/* b chunk is contiguous and previous so a's size can be added to b */
							if (a_p == next_chunk(b_p)) {
								psize += chunksize(b_p);
								set_inuse(m, b_p, psize);
								a_it = b_it;
								a_p = b_p;
								a_mem = b_mem;
								continue;
							}
						}
					}
					/* Normal deallocation starts again in the outer loop */
					a_it = b_it;
					s_allocated_memory -= psize;
					dispose_chunk(m, a_p, psize);
					break;
				}
			}
			else {
				CORRUPTION_ERROR_ACTION(m);
				break;
			}
		}
		if (should_trim(m, m->topsize))
			sys_trim(m, 0);
		POSTACTION(m);
	}
#if FOOTERS
	{
		boost_cont_memchain_it last_pchain = BOOST_CONTAINER_MEMCHAIN_LAST_IT(pchain);
		BOOST_CONTAINER_MEMCHAIN_INIT(pchain);
		BOOST_CONTAINER_MEMCHAIN_INCORPORATE_AFTER
		(pchain
			, last_pchain
			, BOOST_CONTAINER_MEMCHAIN_FIRSTMEM(&ret_chain)
			, BOOST_CONTAINER_MEMCHAIN_LASTMEM(&ret_chain)
			, BOOST_CONTAINER_MEMCHAIN_SIZE(&ret_chain)
		);
	}
#endif
}

static int internal_multialloc_arrays
   (mstate m, size_t n_elements, const size_t* sizes, size_t element_size, size_t contiguous_elements, boost_cont_memchain *pchain) {
   void*     mem;            /* malloced aggregate space */
   mchunkptr p;              /* corresponding chunk */
   size_t    remainder_size; /* remaining bytes while splitting */
   flag_t    was_enabled;    /* to disable mmap */
   size_t    size;
   size_t boost_cont_multialloc_segmented_malloc_size;
   size_t max_size;

   /* Check overflow */
   if(!element_size){
      return 0;
   }
   max_size = MAX_REQUEST/element_size;
   /* Different sizes*/
   switch(contiguous_elements){
      case BOOST_CONTAINER_DL_MULTIALLOC_DEFAULT_CONTIGUOUS:
         /* Use default contiguous mem */
         boost_cont_multialloc_segmented_malloc_size = INTERNAL_MULTIALLOC_DEFAULT_CONTIGUOUS_MEM;
      break;
      case BOOST_CONTAINER_DL_MULTIALLOC_ALL_CONTIGUOUS:
         boost_cont_multialloc_segmented_malloc_size = MAX_REQUEST + CHUNK_OVERHEAD;
      break;
      default:
         if(max_size < contiguous_elements){
            return 0;
         }
         else{
            /* The suggested buffer is just the the element count by the size */
            boost_cont_multialloc_segmented_malloc_size = element_size*contiguous_elements;
         }
   }

   {
      size_t    i;
      size_t next_i;
      /* Where this call's own blocks start, so the unwind below cannot
         touch blocks the chain already carried. */
      boost_cont_memchain_it entry_last_it = BOOST_CONTAINER_MEMCHAIN_LAST_IT(pchain);
      size_t entry_num_mem = BOOST_CONTAINER_MEMCHAIN_SIZE(pchain);
      /*
         Allocate the aggregate chunk.  First disable direct-mmapping so
         malloc won't use it, since we would not be able to later
         free/realloc space internal to a segregated mmap region.
      */
      was_enabled = use_mmap(m);
      disable_mmap(m);
      for(i = 0, next_i = 0; i != n_elements; i = next_i)
      {
         int error = 0;
         size_t accum_size;
         for(accum_size = 0; next_i != n_elements; ++next_i){
            size_t cur_array_size   = sizes[next_i];
            if(max_size < cur_array_size){
               error = 1;
               break;
            }
            else{
               size_t reqsize = request2size(cur_array_size*element_size);
               if(((boost_cont_multialloc_segmented_malloc_size - CHUNK_OVERHEAD) - accum_size) < reqsize){
                  if(!accum_size){
                     accum_size += reqsize;
                     ++next_i;
                  }
                  break;
               }
               accum_size += reqsize;
            }
         }

         mem = error ? 0 : mspace_malloc_lockless(m, accum_size - CHUNK_OVERHEAD);
         if (mem == 0){
            internal_multialloc_rollback(m, pchain, entry_last_it, entry_num_mem);
            if (was_enabled)
               enable_mmap(m);
            return 0;
         }
         p = mem2chunk(mem);
         remainder_size = chunksize(p);
         s_allocated_memory += remainder_size;

         DL_ASSERT(!is_mmapped(p));

         {  /* split out elements */
            void *mem_orig = mem;
            boost_cont_memchain_it last_it = BOOST_CONTAINER_MEMCHAIN_LAST_IT(pchain);
            size_t num_elements = next_i-i;

            for(++i; i != next_i; ++i) {
               void **mem_prev = ((void**)mem);
               size = request2size(sizes[i]*element_size);
               remainder_size -= size;
               set_size_and_pinuse_of_inuse_chunk(m, p, size);
               p = chunk_plus_offset(p, size);
               mem = chunk2mem(p);
               *mem_prev = mem;
            }
            set_size_and_pinuse_of_inuse_chunk(m, p, remainder_size);
            BOOST_CONTAINER_MEMCHAIN_INCORPORATE_AFTER(pchain, last_it, mem_orig, mem, num_elements);
         }
      }
      if (was_enabled)
         enable_mmap(m);
   }
   return 1;
}

int boost_cont_multialloc_arrays
   (size_t n_elements, const size_t *sizes, size_t element_size, size_t contiguous_elements, boost_cont_memchain *pchain)
{
   int ret = 0;
   mstate ms = (mstate)gm;
   ensure_initialization();
   if (!ok_magic(ms)) {
      USAGE_ERROR_ACTION(ms,ms);
   }
   else if (!PREACTION(ms)) {
      ret = internal_multialloc_arrays(ms, n_elements, sizes, element_size, contiguous_elements, pchain);
      POSTACTION(ms);
   }
   return ret;
}


/*Doug Lea malloc extensions*/
static boost_cont_malloc_stats_t get_malloc_stats(mstate m)
{
   boost_cont_malloc_stats_t ret = { 0, 0, 0 };
   ensure_initialization();
   if (!PREACTION(m)) {
      size_t maxfp = 0;
      size_t fp = 0;
      size_t used = 0;
      check_malloc_state(m);
      if (is_initialized(m)) {
         msegmentptr s = &m->seg;
         maxfp = m->max_footprint;
         fp = m->footprint;
         used = fp - (m->topsize + TOP_FOOT_SIZE);

         while (s != 0) {
            mchunkptr q = align_as_chunk(s->base);
            while (segment_holds(s, q) &&
                  q != m->top && q->head != FENCEPOST_HEAD) {
               if (!cinuse(q))
               used -= chunksize(q);
               q = next_chunk(q);
            }
            s = s->next;
         }
      }

      ret.max_system_bytes   = maxfp;
      ret.system_bytes       = fp;
      ret.in_use_bytes       = used;
      POSTACTION(m);
   }
   return ret;
}

size_t boost_cont_size(const void *p)
{  return DL_SIZE_IMPL(p);  }

void* boost_cont_malloc(size_t bytes)
{
   void* mem = 0;
   dlmalloc_globals_t *const dlg = dl_globals();
   mstate const ms = &dlg->gm_state;
   /* ensure_initialization(), through the hoisted globals pointer. Needed
      before PREACTION: init_mparams is what turns gm's lock on. */
   (void)(dlg->params.magic != 0 || init_mparams());
   if (!PREACTION(ms)) {
      mem = mspace_malloc_lockless(ms, bytes);
      if(mem)
         dlg->allocated_memory += chunksize(mem2chunk(mem));
      POSTACTION(ms);
   }
   return mem;
}

void boost_cont_free(void* mem)
{
   dlmalloc_globals_t *const dlg = dl_globals();
   mstate const ms = &dlg->gm_state;
   if (!ok_magic(ms)) {
      USAGE_ERROR_ACTION(ms,ms);
   }
   else if (!PREACTION(ms)) {
      if(mem){
         mchunkptr const p = mem2chunk(mem);
         dlg->allocated_memory -= chunksize(p);
         mspace_free_lockless(ms, mem);
      }
      POSTACTION(ms);
   }
}

void* boost_cont_memalign(size_t bytes, size_t alignment)
{
   void *addr;
   ensure_initialization();
   /* Small alignments take the non-overaligned path,
      larger ones reach internal_memalign directly 
      to avoid internal magic check that can fail in the first allocation. */
   if(alignment <= MALLOC_ALIGNMENT)
      return boost_cont_malloc(bytes);   /* accounts for itself */
   addr = internal_memalign(gm, alignment, bytes);
   if(addr){
      /* internal_memalign released the lock; retake it for the accounting,
         which races other allocations otherwise */
      mstate ms = (mstate)gm;
      if (!PREACTION(ms)) {
         s_allocated_memory += chunksize(mem2chunk(addr));
         POSTACTION(ms);
      }
   }
   return addr;
}

int boost_cont_multialloc_nodes
   (size_t n_elements, size_t elem_size, size_t contiguous_elements, boost_cont_memchain *pchain)
{
   int ret = 0;
   mstate ms = (mstate)gm;
   ensure_initialization();
   if (!ok_magic(ms)) {
      USAGE_ERROR_ACTION(ms,ms);
   }
   else if (!PREACTION(ms)) {
      ret = internal_node_multialloc(ms, n_elements, elem_size, contiguous_elements, pchain);
      POSTACTION(ms);
   }
   return ret;
}

size_t boost_cont_allocated_memory(void)
{
   size_t alloc_mem = 0;
   mstate m = (mstate)gm;
   ensure_initialization();
   if (!ok_magic(m)) {
      USAGE_ERROR_ACTION(m,m);
   }

   if (!PREACTION(m)) {
      check_malloc_state(m);
      if (is_initialized(m)) {
      size_t nfree = SIZE_T_ONE; /* top always free */
      size_t mfree = m->topsize + TOP_FOOT_SIZE;
      msegmentptr s = &m->seg;
      while (s != 0) {
         mchunkptr q = align_as_chunk(s->base);
         while (segment_holds(s, q) &&
               q != m->top && q->head != FENCEPOST_HEAD) {
            size_t sz = chunksize(q);
            if (!is_inuse(q)) {
            mfree += sz;
            ++nfree;
            }
            q = next_chunk(q);
         }
         s = s->next;
      }
      {
         size_t uordblks = m->footprint - mfree;
         if(nfree)
            alloc_mem = (size_t)(uordblks - (nfree-1)*TOP_FOOT_SIZE);
         else
            alloc_mem = uordblks;
         }
      }

      POSTACTION(m);
   }
   return alloc_mem;
}

size_t boost_cont_chunksize(const void *p)
{  return chunksize(mem2chunk(p));   }

int boost_cont_all_deallocated(void)
{  return !s_allocated_memory;  }

boost_cont_malloc_stats_t boost_cont_malloc_stats(void)
{
  mstate ms = (mstate)gm;
  if (ok_magic(ms)) {
    return get_malloc_stats(ms);
  }
  else {
    boost_cont_malloc_stats_t r = { 0, 0, 0 };
    USAGE_ERROR_ACTION(ms,ms);
    return r;
  }
}

size_t boost_cont_in_use_memory(void)
{  return s_allocated_memory;   }

int boost_cont_trim(size_t pad)
{
   ensure_initialization();
   return dlmalloc_trim(pad);
}

int boost_cont_grow
   (void* oldmem, size_t minbytes, size_t maxbytes, size_t *received)
{
   mstate ms = (mstate)gm;
   if (!ok_magic(ms)) {
      USAGE_ERROR_ACTION(ms,ms);
      return 0;
   }

   if (!PREACTION(ms)) {
      mchunkptr p = mem2chunk(oldmem);
      size_t oldsize = chunksize(p);
      p = try_realloc_chunk_with_min(ms, p, request2size(minbytes), request2size(maxbytes), 0);
      /* Accounting and the debug check must happen before the lock is
         released or they race other threads' allocations */
      if(p){
         check_inuse_chunk(ms, p);
         *received = DL_SIZE_IMPL(oldmem);
         s_allocated_memory += chunksize(p) - oldsize;
      }
      POSTACTION(ms);
      return 0 != p;
   }
   return 0;
}

int boost_cont_shrink
   (void* oldmem, size_t minbytes, size_t maxbytes, size_t *received, int do_commit)
{
   mstate ms = (mstate)gm;
   if (!ok_magic(ms)) {
      USAGE_ERROR_ACTION(ms,ms);
      return 0;
   }

   if (!PREACTION(ms)) {
      int ret = internal_shrink(ms, oldmem, minbytes, maxbytes, received, do_commit);
      POSTACTION(ms);
      return 0 != ret;
   }
   return 0;
}


void* boost_cont_alloc
   (size_t minbytes, size_t preferred_bytes, size_t *received_bytes)
{
   //ensure_initialization provided by boost_cont_allocation_command
   return boost_cont_allocation_command
      (BOOST_CONTAINER_ALLOCATE_NEW, 1, 1,  minbytes, preferred_bytes, received_bytes, 0).first;
}

void boost_cont_multidealloc(boost_cont_memchain *pchain)
{
   mstate ms = (mstate)gm;
   if (!ok_magic(ms)) {
      (void)ms;
      USAGE_ERROR_ACTION(ms,ms);
   }
   internal_multialloc_free(ms, pchain);
}

int boost_cont_malloc_check(void)
{
#ifdef DL_DEBUG
   mstate ms = (mstate)gm;
   ensure_initialization();
   if (!ok_magic(ms)) {
      (void)ms;
      USAGE_ERROR_ACTION(ms,ms);
      return 0;
   }
   /* Unlike the sanity check above, this walks every chunk of the heap, so
      it has to run with the lock held */
   if (!PREACTION(ms)) {
      check_malloc_state(ms);
      POSTACTION(ms);
   }
   return 1;
#else
   return 1;
#endif
}


boost_cont_command_ret_t boost_cont_allocation_command
   (allocation_type command, size_t sizeof_object, size_t alignof_object, size_t limit_size
   , size_t preferred_size, size_t *received_size, void *reuse_ptr)
{
   boost_cont_command_ret_t ret = { 0, 0 };
   /* One globals lookup for the whole command (see boost_cont_malloc) */
   dlmalloc_globals_t *const dlg = dl_globals();
   (void)(dlg->params.magic != 0 || init_mparams());
   if(command & (BOOST_CONTAINER_SHRINK_IN_PLACE | BOOST_CONTAINER_TRY_SHRINK_IN_PLACE)){
      int success = boost_cont_shrink( reuse_ptr, preferred_size, limit_size
                             , received_size, (command & BOOST_CONTAINER_SHRINK_IN_PLACE));
      ret.first = success ? reuse_ptr : 0;
      return ret;
   }

   *received_size = 0;

   if(limit_size > preferred_size)
      return ret;

   {
      mstate ms = &dlg->gm_state;

      /*Expand in place*/
      if (!PREACTION(ms)) {
         #if FOOTERS
         if(reuse_ptr){
            mstate m = get_mstate_for(mem2chunk(reuse_ptr));
            if (!ok_magic(m)) {
               USAGE_ERROR_ACTION(m, reuse_ptr);
               goto postaction;   /* Boost.Container: don't leak the lock */
            }
         }
         #endif
         if(reuse_ptr && (command & (BOOST_CONTAINER_EXPAND_FWD | BOOST_CONTAINER_EXPAND_BWD))){
            void *r = internal_grow_both_sides
               ( ms, command, reuse_ptr, limit_size
               , preferred_size, received_size, sizeof_object, 1);
            if(r){
               ret.first  = r;
               ret.second = 1;
               goto postaction;
            }
         }

         if(command & BOOST_CONTAINER_ALLOCATE_NEW){
            /* The lock is held here, so the nested allocation must not take
               it again: both variants below are lockless and run inside this
               one critical section, bookkeeping included. Never
               disable_lock(ms): that clears the use-lock flag of the whole
               mspace, so every OTHER thread's malloc/free skips locking for
               the duration - a data race that corrupts the heap under any
               real concurrency. */
            void* addr;
            if(alignof_object <= MALLOC_ALIGNMENT){
               addr = mspace_malloc_lockless(ms, preferred_size);
               /* Only worth a second try with the smaller size: when the two
                  are equal (boost_cont_malloc passes bytes for both) nothing
                  has changed under the still-held lock, so the retry repeats
                  the same bin walk and the same failing sys_alloc. */
               if(!addr && limit_size != preferred_size)
                  addr = mspace_malloc_lockless(ms, limit_size);
            }
            else{
               addr = mspace_memalign_lockless(ms, alignof_object, preferred_size);
               if(!addr && limit_size != preferred_size)
                  addr = mspace_memalign_lockless(ms, alignof_object, limit_size);
            }
            if(addr){
               dlg->allocated_memory += chunksize(mem2chunk(addr));
               *received_size = DL_SIZE_IMPL(addr);
            }

            ret.first  = addr;
            ret.second = 0;
            if(addr){
               goto postaction;
            }
         }

         //Now try to expand both sides with min size
         if(reuse_ptr && (command & (BOOST_CONTAINER_EXPAND_FWD | BOOST_CONTAINER_EXPAND_BWD))){
            void *r = internal_grow_both_sides
               ( ms, command, reuse_ptr, limit_size
               , preferred_size, received_size, sizeof_object, 0);
            if(r){
               ret.first  = r;
               ret.second = 1;
               goto postaction;
            }
         }
         postaction:
         POSTACTION(ms);
      }
   }
   return ret;
}

int boost_cont_mallopt(int param_number, int value)
{
  return change_mparam(param_number, value);
}

}  //namespace dl_detail {
}  //namespace container {
}  //namespace boost {

#ifdef _MSC_VER
#pragma warning (pop)
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

/* -------------------------- macro hygiene, restore -------------------------
   Undoes the save block at the top of this header, one pop per push.
   -------------------------------------------------------------------------- */
#pragma pop_macro("mymalloc")
#pragma pop_macro("DLMALLOC_EXPORT")
#pragma pop_macro("DLMALLOC_VERSION")
#pragma pop_macro("DL_WIN32")
#pragma pop_macro("LACKS_FCNTL_H")
#pragma pop_macro("HAVE_MMAP")
#pragma pop_macro("HAVE_MORECORE")
#pragma pop_macro("LACKS_UNISTD_H")
#pragma pop_macro("LACKS_SYS_PARAM_H")
#pragma pop_macro("LACKS_SYS_MMAN_H")
#pragma pop_macro("LACKS_STRING_H")
#pragma pop_macro("LACKS_STRINGS_H")
#pragma pop_macro("LACKS_SYS_TYPES_H")
#pragma pop_macro("LACKS_ERRNO_H")
#pragma pop_macro("LACKS_SCHED_H")
#pragma pop_macro("MALLOC_FAILURE_ACTION")
#pragma pop_macro("MMAP_CLEARS")
#pragma pop_macro("MALLOC_ALIGNMENT")
#pragma pop_macro("MAX_SIZE_T")
#pragma pop_macro("USE_LOCKS")
#pragma pop_macro("USE_SPIN_LOCKS")
#pragma pop_macro("ONLY_MSPACES")
#pragma pop_macro("MSPACES")
#pragma pop_macro("FOOTERS")
#pragma pop_macro("ABORT")
#pragma pop_macro("ABORT_ON_ASSERT_FAILURE")
#pragma pop_macro("PROCEED_ON_ERROR")
#pragma pop_macro("INSECURE")
#pragma pop_macro("MALLOC_INSPECT_ALL")
#pragma pop_macro("HAVE_MREMAP")
#pragma pop_macro("MORECORE_CONTIGUOUS")
#pragma pop_macro("MORECORE_DEFAULT")
#pragma pop_macro("DEFAULT_GRANULARITY")
#pragma pop_macro("DEFAULT_TRIM_THRESHOLD")
#pragma pop_macro("DEFAULT_MMAP_THRESHOLD")
#pragma pop_macro("MAX_RELEASE_CHECK_RATE")
#pragma pop_macro("USE_BUILTIN_FFS")
#pragma pop_macro("USE_DEV_RANDOM")
#pragma pop_macro("NO_MALLINFO")
#pragma pop_macro("MALLINFO_FIELD_TYPE")
#pragma pop_macro("NO_MALLOC_STATS")
#pragma pop_macro("NO_SEGMENT_TRAVERSAL")
#pragma pop_macro("M_TRIM_THRESHOLD")
#pragma pop_macro("M_GRANULARITY")
#pragma pop_macro("M_MMAP_THRESHOLD")
#pragma pop_macro("_STRUCT_MALLINFO")
#pragma pop_macro("STRUCT_MALLINFO_DECLARED")
#pragma pop_macro("FORCEINLINE")
#pragma pop_macro("NOINLINE")
#pragma pop_macro("dlcalloc")
#pragma pop_macro("dlfree")
#pragma pop_macro("dlmalloc")
#pragma pop_macro("dlmemalign")
#pragma pop_macro("dlposix_memalign")
#pragma pop_macro("dlrealloc")
#pragma pop_macro("dlrealloc_in_place")
#pragma pop_macro("dlvalloc")
#pragma pop_macro("dlpvalloc")
#pragma pop_macro("dlmallinfo")
#pragma pop_macro("dlmallopt")
#pragma pop_macro("dlmalloc_trim")
#pragma pop_macro("dlmalloc_stats")
#pragma pop_macro("dlmalloc_usable_size")
#pragma pop_macro("dlmalloc_footprint")
#pragma pop_macro("dlmalloc_max_footprint")
#pragma pop_macro("dlmalloc_footprint_limit")
#pragma pop_macro("dlmalloc_set_footprint_limit")
#pragma pop_macro("dlmalloc_inspect_all")
#pragma pop_macro("dlindependent_calloc")
#pragma pop_macro("dlindependent_comalloc")
#pragma pop_macro("dlbulk_free")
#pragma pop_macro("DL_ASSERT")
#pragma pop_macro("DL_DEBUG")
#pragma pop_macro("__USE_GNU")
#pragma pop_macro("interlockedcompareexchange")
#pragma pop_macro("interlockedexchange")
#pragma pop_macro("LOCK_AT_FORK")
#pragma pop_macro("BitScanForward")
#pragma pop_macro("BitScanReverse")
#pragma pop_macro("_SC_PAGE_SIZE")
#pragma pop_macro("malloc_getpagesize")
#pragma pop_macro("SIZE_T_SIZE")
#pragma pop_macro("SIZE_T_BITSIZE")
#pragma pop_macro("SIZE_T_ZERO")
#pragma pop_macro("SIZE_T_ONE")
#pragma pop_macro("SIZE_T_TWO")
#pragma pop_macro("SIZE_T_FOUR")
#pragma pop_macro("TWO_SIZE_T_SIZES")
#pragma pop_macro("FOUR_SIZE_T_SIZES")
#pragma pop_macro("SIX_SIZE_T_SIZES")
#pragma pop_macro("HALF_MAX_SIZE_T")
#pragma pop_macro("CHUNK_ALIGN_MASK")
#pragma pop_macro("is_aligned")
#pragma pop_macro("align_offset")
#pragma pop_macro("MFAIL")
#pragma pop_macro("CMFAIL")
#pragma pop_macro("MUNMAP_DEFAULT")
#pragma pop_macro("MMAP_PROT")
#pragma pop_macro("MAP_ANONYMOUS")
#pragma pop_macro("MMAP_FLAGS")
#pragma pop_macro("MMAP_DEFAULT")
#pragma pop_macro("dev_zero_fd")
#pragma pop_macro("DIRECT_MMAP_DEFAULT")
#pragma pop_macro("MREMAP_DEFAULT")
#pragma pop_macro("CALL_MORECORE")
#pragma pop_macro("USE_MMAP_BIT")
#pragma pop_macro("CALL_MMAP")
#pragma pop_macro("CALL_MUNMAP")
#pragma pop_macro("CALL_DIRECT_MMAP")
#pragma pop_macro("MMAP")
#pragma pop_macro("MUNMAP")
#pragma pop_macro("DIRECT_MMAP")
#pragma pop_macro("CALL_MREMAP")
#pragma pop_macro("USE_NONCONTIGUOUS_BIT")
#pragma pop_macro("EXTERN_BIT")
#pragma pop_macro("USE_LOCK_BIT")
#pragma pop_macro("INITIAL_LOCK")
#pragma pop_macro("DESTROY_LOCK")
#pragma pop_macro("ACQUIRE_MALLOC_GLOBAL_LOCK")
#pragma pop_macro("RELEASE_MALLOC_GLOBAL_LOCK")
#pragma pop_macro("CAS_LOCK")
#pragma pop_macro("CLEAR_LOCK")
#pragma pop_macro("SPINS_PER_YIELD")
#pragma pop_macro("SLEEP_EX_DURATION")
#pragma pop_macro("SPIN_LOCK_YIELD")
#pragma pop_macro("MLOCK_T")
#pragma pop_macro("TRY_LOCK")
#pragma pop_macro("RELEASE_LOCK")
#pragma pop_macro("ACQUIRE_LOCK")
#pragma pop_macro("THREAD_ID_T")
#pragma pop_macro("CURRENT_THREAD")
#pragma pop_macro("EQ_OWNER")
#pragma pop_macro("NEED_GLOBAL_LOCK_INIT")
#pragma pop_macro("PTHREAD_MUTEX_RECURSIVE")
#pragma pop_macro("pthread_mutexattr_settype")
#pragma pop_macro("MCHUNK_SIZE")
#pragma pop_macro("CHUNK_OVERHEAD")
#pragma pop_macro("MMAP_CHUNK_OVERHEAD")
#pragma pop_macro("MMAP_FOOT_PAD")
#pragma pop_macro("MIN_CHUNK_SIZE")
#pragma pop_macro("chunk2mem")
#pragma pop_macro("mem2chunk")
#pragma pop_macro("align_as_chunk")
#pragma pop_macro("MAX_REQUEST")
#pragma pop_macro("MIN_REQUEST")
#pragma pop_macro("pad_request")
#pragma pop_macro("request2size")
#pragma pop_macro("PINUSE_BIT")
#pragma pop_macro("CINUSE_BIT")
#pragma pop_macro("FLAG4_BIT")
#pragma pop_macro("INUSE_BITS")
#pragma pop_macro("FLAG_BITS")
#pragma pop_macro("FENCEPOST_HEAD")
#pragma pop_macro("cinuse")
#pragma pop_macro("pinuse")
#pragma pop_macro("flag4inuse")
#pragma pop_macro("is_inuse")
#pragma pop_macro("is_mmapped")
#pragma pop_macro("chunksize")
#pragma pop_macro("clear_pinuse")
#pragma pop_macro("set_flag4")
#pragma pop_macro("clear_flag4")
#pragma pop_macro("chunk_plus_offset")
#pragma pop_macro("chunk_minus_offset")
#pragma pop_macro("next_chunk")
#pragma pop_macro("prev_chunk")
#pragma pop_macro("next_pinuse")
#pragma pop_macro("get_foot")
#pragma pop_macro("set_foot")
#pragma pop_macro("set_size_and_pinuse_of_free_chunk")
#pragma pop_macro("set_free_with_pinuse")
#pragma pop_macro("overhead_for")
#pragma pop_macro("calloc_must_clear")
#pragma pop_macro("leftmost_child")
#pragma pop_macro("is_mmapped_segment")
#pragma pop_macro("is_extern_segment")
#pragma pop_macro("NSMALLBINS")
#pragma pop_macro("NTREEBINS")
#pragma pop_macro("SMALLBIN_SHIFT")
#pragma pop_macro("SMALLBIN_WIDTH")
#pragma pop_macro("TREEBIN_SHIFT")
#pragma pop_macro("MIN_LARGE_SIZE")
#pragma pop_macro("MAX_SMALL_SIZE")
#pragma pop_macro("MAX_SMALL_REQUEST")
#pragma pop_macro("malloc_global_mutex")
#pragma pop_macro("malloc_corruption_error_count")
#pragma pop_macro("mparams")
#pragma pop_macro("ensure_initialization")
#pragma pop_macro("gm")
#pragma pop_macro("is_global")
#pragma pop_macro("is_initialized")
#pragma pop_macro("use_lock")
#pragma pop_macro("enable_lock")
#pragma pop_macro("disable_lock")
#pragma pop_macro("use_mmap")
#pragma pop_macro("enable_mmap")
#pragma pop_macro("disable_mmap")
#pragma pop_macro("use_noncontiguous")
#pragma pop_macro("disable_contiguous")
#pragma pop_macro("set_lock")
#pragma pop_macro("page_align")
#pragma pop_macro("granularity_align")
#pragma pop_macro("mmap_align")
#pragma pop_macro("SYS_ALLOC_PADDING")
#pragma pop_macro("is_page_aligned")
#pragma pop_macro("is_granularity_aligned")
#pragma pop_macro("segment_holds")
#pragma pop_macro("should_trim")
#pragma pop_macro("TOP_FOOT_SIZE")
#pragma pop_macro("PREACTION")
#pragma pop_macro("POSTACTION")
#pragma pop_macro("CORRUPTION_ERROR_ACTION")
#pragma pop_macro("USAGE_ERROR_ACTION")
#pragma pop_macro("check_free_chunk")
#pragma pop_macro("check_inuse_chunk")
#pragma pop_macro("check_malloced_chunk")
#pragma pop_macro("check_mmapped_chunk")
#pragma pop_macro("check_malloc_state")
#pragma pop_macro("check_top_chunk")
#pragma pop_macro("is_small")
#pragma pop_macro("small_index")
#pragma pop_macro("small_index2size")
#pragma pop_macro("MIN_SMALL_INDEX")
#pragma pop_macro("smallbin_at")
#pragma pop_macro("treebin_at")
#pragma pop_macro("compute_tree_index")
#pragma pop_macro("bit_for_tree_index")
#pragma pop_macro("leftshift_for_tree_index")
#pragma pop_macro("minsize_for_tree_index")
#pragma pop_macro("idx2bit")
#pragma pop_macro("mark_smallmap")
#pragma pop_macro("clear_smallmap")
#pragma pop_macro("smallmap_is_marked")
#pragma pop_macro("mark_treemap")
#pragma pop_macro("clear_treemap")
#pragma pop_macro("treemap_is_marked")
#pragma pop_macro("least_bit")
#pragma pop_macro("left_bits")
#pragma pop_macro("same_or_left_bits")
#pragma pop_macro("compute_bit2idx")
#pragma pop_macro("ok_address")
#pragma pop_macro("ok_next")
#pragma pop_macro("ok_inuse")
#pragma pop_macro("ok_pinuse")
#pragma pop_macro("ok_magic")
#pragma pop_macro("RTCHECK")
#pragma pop_macro("mark_inuse_foot")
#pragma pop_macro("set_inuse")
#pragma pop_macro("set_inuse_and_pinuse")
#pragma pop_macro("set_size_and_pinuse_of_inuse_chunk")
#pragma pop_macro("get_mstate_for")
#pragma pop_macro("insert_small_chunk")
#pragma pop_macro("unlink_small_chunk")
#pragma pop_macro("unlink_first_small_chunk")
#pragma pop_macro("replace_dv")
#pragma pop_macro("insert_large_chunk")
#pragma pop_macro("unlink_large_chunk")
#pragma pop_macro("insert_chunk")
#pragma pop_macro("unlink_chunk")
#pragma pop_macro("internal_malloc")
#pragma pop_macro("internal_free")
#pragma pop_macro("fm")
#pragma pop_macro("MORECORE")
#pragma pop_macro("MAX_POOL_ENTRIES")
#pragma pop_macro("MINIMUM_MORECORE_SIZE")
#pragma pop_macro("DL_MEM_COMMIT")
#pragma pop_macro("DL_MEM_RESERVE")
#pragma pop_macro("DL_MEM_RELEASE")
#pragma pop_macro("DL_MEM_TOP_DOWN")
#pragma pop_macro("DL_PAGE_READWRITE")
#pragma pop_macro("DL_DEBUG_DEFINED")
#pragma pop_macro("USE_DL_PREFIX")
#pragma pop_macro("DL_SIZE_IMPL")
#pragma pop_macro("BOOST_CONTAINER_DL_WIDE_SMALLBINS")
#pragma pop_macro("BOOST_CONTAINER_DL_REBASED_SMALLBINS")
#pragma pop_macro("BOOST_CONTAINER_DL_SINGLE_THREADED")
#pragma pop_macro("s_allocated_memory")
#pragma pop_macro("GET_TRUNCATED_SIZE")
#pragma pop_macro("GET_ROUNDED_SIZE")
#pragma pop_macro("GET_TRUNCATED_PO2_SIZE")
#pragma pop_macro("GET_ROUNDED_PO2_SIZE")
#pragma pop_macro("CALCULATE_GCD")
#pragma pop_macro("CALCULATE_LCM")
#pragma pop_macro("INTERNAL_MULTIALLOC_DEFAULT_CONTIGUOUS_MEM")
#pragma pop_macro("SQRT_MAX_SIZE_T")
#pragma pop_macro("BOOST_ALLOC_PLUS_MEMCHAIN_MEM_JUMP_NEXT")

namespace boost{
namespace container{

typedef boost_cont_command_ret_t dlmalloc_command_ret_t;
typedef boost_cont_memchain dlmalloc_memchain;
typedef boost_cont_memchain_it dlmalloc_memchain_it;
typedef boost_cont_malloc_stats_t dlmalloc_malloc_stats_t;

inline size_t dlmalloc_size(const void *p)
{  return dl_detail::boost_cont_size(p);  }

inline void* dlmalloc_malloc(size_t bytes)
{  return dl_detail::boost_cont_malloc(bytes);  }

inline void  dlmalloc_free(void* mem)
{  return dl_detail::boost_cont_free(mem);  }

inline void* dlmalloc_memalign(size_t bytes, size_t alignment)
{  return dl_detail::boost_cont_memalign(bytes, alignment);  }

inline int dlmalloc_multialloc_nodes
   (size_t n_elements, size_t elem_size, size_t contiguous_elements, boost_cont_memchain *pchain)
{  return dl_detail::boost_cont_multialloc_nodes(n_elements, elem_size, contiguous_elements, pchain);  }

inline int dlmalloc_multialloc_arrays
   (size_t n_elements, const size_t *sizes, size_t sizeof_element, size_t contiguous_elements, boost_cont_memchain *pchain)
{  return dl_detail::boost_cont_multialloc_arrays(n_elements, sizes, sizeof_element, contiguous_elements, pchain); }

inline void dlmalloc_multidealloc(boost_cont_memchain *pchain)
{  return dl_detail::boost_cont_multidealloc(pchain); }

inline size_t dlmalloc_allocated_memory()
{  return dl_detail::boost_cont_allocated_memory(); }

inline size_t dlmalloc_chunksize(const void *p)
{  return dl_detail::boost_cont_chunksize(p); }

inline int dlmalloc_all_deallocated()
{  return dl_detail::boost_cont_all_deallocated(); }

inline boost_cont_malloc_stats_t dlmalloc_malloc_stats()
{  return dl_detail::boost_cont_malloc_stats(); }

inline size_t dlmalloc_in_use_memory()
{  return dl_detail::boost_cont_in_use_memory(); }

inline int dlmalloc_trim(size_t pad)
{  return dl_detail::boost_cont_trim(pad); }

inline int dlmalloc_mallopt(int parameter_number, int parameter_value)
{  return dl_detail::boost_cont_mallopt(parameter_number, parameter_value); }

inline int dlmalloc_grow
   (void* oldmem, size_t minbytes, size_t maxbytes, size_t *received)
{  return dl_detail::boost_cont_grow(oldmem, minbytes, maxbytes, received); }

inline int dlmalloc_shrink
   (void* oldmem, size_t minbytes, size_t maxbytes, size_t *received, int do_commit)
{  return dl_detail::boost_cont_shrink(oldmem, minbytes, maxbytes, received, do_commit); }

inline void* dlmalloc_alloc
   (size_t minbytes, size_t preferred_bytes, size_t *received_bytes)
{  return dl_detail::boost_cont_alloc(minbytes, preferred_bytes, received_bytes); }

inline int dlmalloc_malloc_check()
{  return dl_detail::boost_cont_malloc_check(); }

inline boost_cont_command_ret_t dlmalloc_allocation_command
   ( allocation_type command
   , size_t sizeof_object
   , size_t alignof_object
   , size_t limit_bytes
   , size_t preferred_bytes
   , size_t *received_bytes
   , void *reuse_ptr
   )
{  return dl_detail::boost_cont_allocation_command(command, sizeof_object, alignof_object, limit_bytes, preferred_bytes, received_bytes, reuse_ptr); }

}  //namespace container{
}  //namespace boost{

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_DETAIL_DLMALLOC_HPP
