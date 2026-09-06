//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
// One configuration of the pristine dlmalloc_2_8_6.c, compiled as C++ inside
// a namespace of its own, next to a dlmalloc configured the same way.
//
// Include this once per configuration, with every knob defined first:
//
//   DLC_NS                     the namespace this configuration lives in
//   DLC_FOOTERS                0/1  FOOTERS                / footers
//   DLC_INSECURE               0/1  INSECURE               / insecure
//   DLC_MALLOC_INSPECT_ALL     0/1  MALLOC_INSPECT_ALL     / malloc_inspect_all
//   DLC_NO_SEGMENT_TRAVERSAL   0/1  NO_SEGMENT_TRAVERSAL   / no_segment_traversal
//   DLC_USE_LOCKS              0/1  USE_LOCKS              / use_locks
//   DLC_DEBUG                  0/1  DEBUG                  / debug
//   DLC_MALLOC_ALIGNMENT            MALLOC_ALIGNMENT       / malloc_alignment
//   DLC_DEFAULT_GRANULARITY         DEFAULT_GRANULARITY    / default_granularity
//   DLC_DEFAULT_TRIM_THRESHOLD      DEFAULT_TRIM_THRESHOLD / default_trim_threshold
//   DLC_DEFAULT_MMAP_THRESHOLD      DEFAULT_MMAP_THRESHOLD / default_mmap_threshold
//   DLC_MAX_RELEASE_CHECK_RATE      MAX_RELEASE_CHECK_RATE / max_release_check_rate
//
// Each knob is spelled once and feeds both sides: the macro the .c reads, and
// the member of the dlmalloc Config built below. The two cannot drift apart,
// which is the point of a compatibility test.
//
// Fixed for every configuration, because dlmalloc has no other setting:
// mspaces only, no sbrk, mmap on, no malloc_stats (it prints), mallinfo on
// (it walks and checks the heap, which is what the test wants from it). A
// lock, when there is one, is the very spin_mutex_t dlmalloc uses, plugged
// in through USE_LOCKS == 2 the way dlmalloc.hpp does. wide_smallbins and
// rebased_smallbins are off: the pristine .c has no such option.
//
// The dlmalloc built here runs on the C structs themselves: its Config names
// them as its layout, so both sides read and write the heap through one set
// of types and no access is ever made through a type the object does not
// have. Nothing in this test depends on how a compiler treats aliasing.
//
// What it leaves behind is DLC_NS::api and nothing else. Every macro the .c
// defines is pushed before and popped after; the functions with C linkage
// get a name per namespace, since two definitions of one C name cannot
// link; the knobs are #undefined at the end.
//////////////////////////////////////////////////////////////////////////////
#if !defined(DLC_NS) || !defined(DLC_FOOTERS) || !defined(DLC_INSECURE)
#  error "define every DLC_ knob before including dlmalloc_2_8_6_config_inc.h"
#endif
#if !defined(DLC_MALLOC_INSPECT_ALL) || !defined(DLC_NO_SEGMENT_TRAVERSAL)
#  error "define every DLC_ knob before including dlmalloc_2_8_6_config_inc.h"
#endif
#if !defined(DLC_USE_LOCKS) || !defined(DLC_DEBUG) || !defined(DLC_MALLOC_ALIGNMENT)
#  error "define every DLC_ knob before including dlmalloc_2_8_6_config_inc.h"
#endif
#if !defined(DLC_DEFAULT_GRANULARITY) || !defined(DLC_DEFAULT_TRIM_THRESHOLD)
#  error "define every DLC_ knob before including dlmalloc_2_8_6_config_inc.h"
#endif
#if !defined(DLC_DEFAULT_MMAP_THRESHOLD) || !defined(DLC_MAX_RELEASE_CHECK_RATE)
#  error "define every DLC_ knob before including dlmalloc_2_8_6_config_inc.h"
#endif

#include "dlmalloc_2_8_6_push_macros_inc.h"

//---------------------------------------------------------------------------
// The C-linkage names, one set per namespace
//---------------------------------------------------------------------------
// The .c declares its entry points extern "C". Inside a namespace that still
// names one C symbol, so a second inclusion would define it twice. Each
// inclusion therefore renames them with the namespace as a prefix; api below
// calls them while the renames are in force, and the driver never spells
// them.
#define DLC_CAT2(a, b) a##b
#define DLC_CAT(a, b)  DLC_CAT2(a, b)
#define DLC_SYM(name)  DLC_CAT(DLC_CAT(DLC_NS, _), name)
#define DLC_STR2(x)    #x
#define DLC_STR(x)     DLC_STR2(x)

#pragma push_macro("create_mspace")
#pragma push_macro("create_mspace_with_base")
#pragma push_macro("destroy_mspace")
#pragma push_macro("mspace_track_large_chunks")
#pragma push_macro("mspace_malloc")
#pragma push_macro("mspace_free")
#pragma push_macro("mspace_calloc")
#pragma push_macro("mspace_realloc")
#pragma push_macro("mspace_realloc_in_place")
#pragma push_macro("mspace_memalign")
#pragma push_macro("mspace_independent_calloc")
#pragma push_macro("mspace_independent_comalloc")
#pragma push_macro("mspace_bulk_free")
#pragma push_macro("mspace_trim")
#pragma push_macro("mspace_footprint")
#pragma push_macro("mspace_max_footprint")
#pragma push_macro("mspace_footprint_limit")
#pragma push_macro("mspace_set_footprint_limit")
#pragma push_macro("mspace_mallinfo")
#pragma push_macro("mspace_usable_size")
#pragma push_macro("mspace_mallopt")
#pragma push_macro("mspace_inspect_all")
#pragma push_macro("mspace_malloc_stats")

#define create_mspace                DLC_SYM(create_mspace)
#define create_mspace_with_base      DLC_SYM(create_mspace_with_base)
#define destroy_mspace               DLC_SYM(destroy_mspace)
#define mspace_track_large_chunks    DLC_SYM(mspace_track_large_chunks)
#define mspace_malloc                DLC_SYM(mspace_malloc)
#define mspace_free                  DLC_SYM(mspace_free)
#define mspace_calloc                DLC_SYM(mspace_calloc)
#define mspace_realloc               DLC_SYM(mspace_realloc)
#define mspace_realloc_in_place      DLC_SYM(mspace_realloc_in_place)
#define mspace_memalign              DLC_SYM(mspace_memalign)
#define mspace_independent_calloc    DLC_SYM(mspace_independent_calloc)
#define mspace_independent_comalloc  DLC_SYM(mspace_independent_comalloc)
#define mspace_bulk_free             DLC_SYM(mspace_bulk_free)
#define mspace_trim                  DLC_SYM(mspace_trim)
#define mspace_footprint             DLC_SYM(mspace_footprint)
#define mspace_max_footprint         DLC_SYM(mspace_max_footprint)
#define mspace_footprint_limit       DLC_SYM(mspace_footprint_limit)
#define mspace_set_footprint_limit   DLC_SYM(mspace_set_footprint_limit)
#define mspace_mallinfo              DLC_SYM(mspace_mallinfo)
#define mspace_usable_size           DLC_SYM(mspace_usable_size)
#define mspace_mallopt               DLC_SYM(mspace_mallopt)
#define mspace_inspect_all           DLC_SYM(mspace_inspect_all)
#define mspace_malloc_stats          DLC_SYM(mspace_malloc_stats)

//---------------------------------------------------------------------------
// The configuration: fixed part
//---------------------------------------------------------------------------
#define MSPACES              1
#define ONLY_MSPACES         1
#define HAVE_MORECORE        0
#define HAVE_MMAP            1
#define NO_MALLOC_STATS      1
#define NO_MALLINFO          0
//mremap stays off on the C side, for two reasons. The .c turns it on with
//"#ifdef linux", a macro a strict -std=c++NN build does not have, so what
//it would pick depends on the compiler mode. And with it on, its
//mmap_resize() sizes the new mapping without the chunk offset, so a
//realloc of a memalign()ed, directly mapped block comes back smaller than
//asked - dlmalloc 2.8.6 has this defect, dlmalloc corrects it (see
//mmap_resize there), and a test that wrote into such a block would only
//be testing the defect. dlmalloc still uses mremap where it has it, so
//that path is exercised: the C side frees and audits what it resized.
#define HAVE_MREMAP          0
//The global lock guards one-time start-up and create_mspace. The driver
//runs start-up itself, once, on one thread.
#define ACQUIRE_MALLOC_GLOBAL_LOCK()
#define RELEASE_MALLOC_GLOBAL_LOCK()

//---------------------------------------------------------------------------
// The configuration: the knobs
//---------------------------------------------------------------------------
#define FOOTERS                 DLC_FOOTERS
#define INSECURE                DLC_INSECURE
#define MALLOC_INSPECT_ALL      DLC_MALLOC_INSPECT_ALL
#define NO_SEGMENT_TRAVERSAL    DLC_NO_SEGMENT_TRAVERSAL
#define MALLOC_ALIGNMENT        DLC_MALLOC_ALIGNMENT
#define DEFAULT_GRANULARITY     DLC_DEFAULT_GRANULARITY
#define DEFAULT_TRIM_THRESHOLD  DLC_DEFAULT_TRIM_THRESHOLD
#define DEFAULT_MMAP_THRESHOLD  DLC_DEFAULT_MMAP_THRESHOLD
#define MAX_RELEASE_CHECK_RATE  DLC_MAX_RELEASE_CHECK_RATE

//The .c asks "#ifdef DEBUG", so off has to mean undefined, not zero.
#if DLC_DEBUG
#  define DEBUG 1
#endif

#if DLC_USE_LOCKS
#  define USE_LOCKS 2
#  define MLOCK_T          ::boost::container::dtl::spin_mutex_t
#  define INITIAL_LOCK(lk) (::boost::container::dtl::spin_mutex_init(lk), 0)
#  define DESTROY_LOCK(lk) (0)
#  define ACQUIRE_LOCK(lk) (::boost::container::dtl::spin_mutex_lock(lk), 0)
#  define RELEASE_LOCK(lk) ::boost::container::dtl::spin_mutex_unlock(lk)
#  define TRY_LOCK(lk)     (::boost::container::dtl::spin_mutex_try_lock(lk) ? 1 : 0)
#else
#  define USE_LOCKS 0
#endif

namespace DLC_NS {

//Every warning the pristine .c draws is turned off by the wrapper below -
//see dlmalloc_2_8_6_system_inc.h, which explains how per compiler.
//
//A sanitizer is not a warning and none of that reaches it: TOP_FOOT_SIZE
//measures its offset by arithmetic on a null pointer, which UBSan reports
//at run time ("applying non-zero offset to null pointer"), so that one
//check comes off the functions themselves.
#if defined(__clang__) && defined(__has_feature)
#  if __has_feature(undefined_behavior_sanitizer)
#    pragma clang attribute push (__attribute__((no_sanitize("pointer-overflow"))), apply_to = function)
#    define DLC_POP_UBSAN_ATTRIBUTE
#  endif
#endif
#include "dlmalloc_2_8_6_system_inc.h"
#if defined(DLC_POP_UBSAN_ATTRIBUTE)
#  pragma clang attribute pop
#  undef DLC_POP_UBSAN_ATTRIBUTE
#endif

//---------------------------------------------------------------------------
// The C structs, as dlmalloc's layout
//---------------------------------------------------------------------------
//Handed to dlmalloc below. Its malloc_state IS this malloc_state, its chunks
//are these chunks, and dlmalloc finds exactly its own types in the heap.
struct c_layout
{
   typedef struct malloc_chunk      malloc_chunk;
   typedef struct malloc_tree_chunk malloc_tree_chunk;
   typedef struct malloc_segment    malloc_segment;
   typedef struct malloc_state      malloc_state;
   typedef struct malloc_params     malloc_params;
};

//---------------------------------------------------------------------------
// The dlmalloc side of the same configuration, from the same knobs
//---------------------------------------------------------------------------
struct config
{
   typedef ::std::size_t size_type;
   typedef c_layout      layout;
   static const bool footers                 = (DLC_FOOTERS) != 0;
   static const bool insecure                = (DLC_INSECURE) != 0;
   static const bool proceed_on_error        = false;
   static const bool malloc_inspect_all      = (DLC_MALLOC_INSPECT_ALL) != 0;
   static const bool no_segment_traversal    = (DLC_NO_SEGMENT_TRAVERSAL) != 0;
   static const bool use_locks               = (DLC_USE_LOCKS) != 0;
   static const bool wide_smallbins          = false;
   static const bool rebased_smallbins       = false;
   static const bool debug                   = (DLC_DEBUG) != 0;
   static const bool abort_on_assert_failure = true;
   typedef ::boost::container::dlmalloc_errno_action malloc_failure_action;
   typedef ::boost::container::dlmalloc_abort        abort_action;
   static const size_type malloc_alignment       = DLC_MALLOC_ALIGNMENT;
   static const size_type default_granularity    = DLC_DEFAULT_GRANULARITY;
   static const size_type default_trim_threshold = DLC_DEFAULT_TRIM_THRESHOLD;
   static const size_type default_mmap_threshold = DLC_DEFAULT_MMAP_THRESHOLD;
   static const size_type max_release_check_rate = DLC_MAX_RELEASE_CHECK_RATE;
};

typedef ::boost::container::basic_dlmalloc<config> heap_type;

//---------------------------------------------------------------------------
// Everything the driver needs from this configuration, behind one type
//---------------------------------------------------------------------------
// The driver is a template over this, so one body runs every configuration.
// The C entry points are called from here, while their names for this
// namespace are still in force.
struct api
{
   typedef DLC_NS::config      config_type;
   typedef DLC_NS::heap_type   heap_type;
   typedef mspace               handle_type;
   typedef struct malloc_chunk      chunk_type;      //the C structs themselves
   typedef struct malloc_tree_chunk tree_chunk_type;
   typedef struct malloc_segment    segment_type;
   typedef struct malloc_state      state_type;
   typedef struct malloc_params     params_type;
   typedef struct mallinfo          mallinfo_type;

   static const bool has_inspect_all = (DLC_MALLOC_INSPECT_ALL) != 0;
   static const char *name()  {  return DLC_STR(DLC_NS);  }

   //A dlmalloc on c_layout is a malloc_state followed by a malloc_params -
   //these very structs - and the handle dlmalloc wants is the address of the
   //state. A pointer to a standard-layout object converts to a pointer to its
   //first member, so state() is a conversion, not a reinterpretation of
   //bytes; the params follow the state, and the driver checks that the object
   //is exactly the two before relying on it.
   static state_type *state(heap_type &h)
   {  return reinterpret_cast<state_type *>(&h);  }
   static params_type *params_of(heap_type &h)
   {  //through void*, because the address is inside one object and so is
      //aligned for the member that lives there
      void *const after_state = reinterpret_cast<char *>(&h) + sizeof(state_type);
      return static_cast<params_type *>(after_state);
   }
   static handle_type handle(heap_type &h)
   {  return static_cast<handle_type>(state(h));  }

   //dlmalloc's process-wide parameters: what its own start-up computes, and
   //what the driver then overwrites so both sides describe one heap.
   static params_type &params()
   {  return mparams;  }
   static void run_dlmalloc_init()
   {  destroy_mspace(create_mspace(0, 0));  }

   static void *malloc_(handle_type m, ::std::size_t n)
   {  return mspace_malloc(m, n);  }
   static void free_(handle_type m, void *p)
   {  mspace_free(m, p);  }
   static void *realloc_(handle_type m, void *p, ::std::size_t n)
   {  return mspace_realloc(m, p, n);  }
   static void *calloc_(handle_type m, ::std::size_t n, ::std::size_t e)
   {  return mspace_calloc(m, n, e);  }
   static void *memalign_(handle_type m, ::std::size_t a, ::std::size_t n)
   {  return mspace_memalign(m, a, n);  }
   static void **independent_calloc_(handle_type m, ::std::size_t n, ::std::size_t e, void *c[])
   {  return mspace_independent_calloc(m, n, e, c);  }
   static void **independent_comalloc_(handle_type m, ::std::size_t n, ::std::size_t s[], void *c[])
   {  return mspace_independent_comalloc(m, n, s, c);  }
   static ::std::size_t bulk_free_(handle_type m, void *a[], ::std::size_t n)
   {  return mspace_bulk_free(m, a, n);  }
   static int trim_(handle_type m)
   {  return mspace_trim(m, 0);  }
   static ::std::size_t footprint_(handle_type m)
   {  return mspace_footprint(m);  }
   static ::std::size_t max_footprint_(handle_type m)
   {  return mspace_max_footprint(m);  }
   static ::std::size_t usable_size_(const void *p)
   {  return mspace_usable_size(p);  }
   //Walks every segment, bin and chunk. With DEBUG on it runs
   //check_malloc_state first, which is the whole of dlmalloc's own audit.
   static mallinfo_type mallinfo_(handle_type m)
   {  return mspace_mallinfo(m);  }

   #if DLC_MALLOC_INSPECT_ALL
   static void count_inuse(void *, void *, ::std::size_t used, void *arg)
   {  if(used)  ++*static_cast< ::std::size_t *>(arg);  }
   //How many chunks the walker reports in use. It walks segments only, so
   //a directly mapped block is not among them.
   static long inspect_inuse_count(handle_type m)
   {
      ::std::size_t n = 0;
      mspace_inspect_all(m, &count_inuse, &n);
      return (long)n;
   }
   //Every block the C walker reports, in the order it reports them, so the
   //driver can hold dlmalloc's walk against it block for block.
   static void inspect_c(handle_type m,
                         void (*handler)(void *, void *, ::std::size_t, void *),
                         void *arg)
   {  mspace_inspect_all(m, handler, arg);  }
   #else
   static long inspect_inuse_count(handle_type)
   {  return -1;  }
   static void inspect_c(handle_type,
                         void (*)(void *, void *, ::std::size_t, void *), void *)
   {}
   #endif
};

}  //namespace DLC_NS

#pragma pop_macro("create_mspace")
#pragma pop_macro("create_mspace_with_base")
#pragma pop_macro("destroy_mspace")
#pragma pop_macro("mspace_track_large_chunks")
#pragma pop_macro("mspace_malloc")
#pragma pop_macro("mspace_free")
#pragma pop_macro("mspace_calloc")
#pragma pop_macro("mspace_realloc")
#pragma pop_macro("mspace_realloc_in_place")
#pragma pop_macro("mspace_memalign")
#pragma pop_macro("mspace_independent_calloc")
#pragma pop_macro("mspace_independent_comalloc")
#pragma pop_macro("mspace_bulk_free")
#pragma pop_macro("mspace_trim")
#pragma pop_macro("mspace_footprint")
#pragma pop_macro("mspace_max_footprint")
#pragma pop_macro("mspace_footprint_limit")
#pragma pop_macro("mspace_set_footprint_limit")
#pragma pop_macro("mspace_mallinfo")
#pragma pop_macro("mspace_usable_size")
#pragma pop_macro("mspace_mallopt")
#pragma pop_macro("mspace_inspect_all")
#pragma pop_macro("mspace_malloc_stats")

#include "dlmalloc_2_8_6_pop_macros_inc.h"

#undef DLC_CAT2
#undef DLC_CAT
#undef DLC_SYM
#undef DLC_STR2
#undef DLC_STR

#undef DLC_NS
#undef DLC_FOOTERS
#undef DLC_INSECURE
#undef DLC_MALLOC_INSPECT_ALL
#undef DLC_NO_SEGMENT_TRAVERSAL
#undef DLC_USE_LOCKS
#undef DLC_DEBUG
#undef DLC_MALLOC_ALIGNMENT
#undef DLC_DEFAULT_GRANULARITY
#undef DLC_DEFAULT_TRIM_THRESHOLD
#undef DLC_DEFAULT_MMAP_THRESHOLD
#undef DLC_MAX_RELEASE_CHECK_RATE
