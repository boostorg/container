//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//dlmalloc.hpp brings the dlmalloc macro set into the class body and then
//puts every macro back with #pragma pop_macro. Nothing must survive here.
#include <boost/container/detail/dlmalloc.hpp>

#if defined(gm) || defined(fm) || defined(mparams) || defined(m_params) ||    \
    defined(chunksize) || defined(mem2chunk) || defined(chunk2mem) ||         \
    defined(PREACTION) || defined(POSTACTION) || defined(CALL_MMAP) ||        \
    defined(CALL_MUNMAP) || defined(CALL_DIRECT_MMAP) || defined(CALL_MREMAP)
#error "dlmalloc implementation macros leaked into user code (chunk layer)"
#endif

#if defined(MFAIL) || defined(CMFAIL) || defined(ABORT) || defined(MSPACES) || \
    defined(ONLY_MSPACES) || defined(MORECORE_CONTIGUOUS) ||                   \
    defined(HAVE_MORECORE) || defined(HAVE_MMAP) || defined(MLOCK_T) ||        \
    defined(DL_ASSERT) || defined(DL_DEBUG) ||                              \
    defined(BOOST_CONTAINER_DL_ASSERT)
#error "dlmalloc implementation macros leaked into user code (config layer)"
#endif

//The six yes/no configuration switches are static const bool members of the
//class, so they are not macros here even while dlmalloc.hpp is being read.
//Nothing may define them at any point.
#if defined(FOOTERS) || defined(INSECURE) || defined(PROCEED_ON_ERROR) ||      \
    defined(MALLOC_INSPECT_ALL) || defined(NO_SEGMENT_TRAVERSAL) ||            \
    defined(USE_LOCKS)
#error "dlmalloc turned its boolean configuration switches back into macros"
#endif

#if defined(is_small) || defined(is_global) || defined(page_align) ||          \
    defined(disable_lock) || defined(use_lock) || defined(set_lock) ||         \
    defined(ok_magic) || defined(ok_address) || defined(RTCHECK) ||            \
    defined(init_bins) || defined(init_top) || defined(segment_holds) ||       \
    defined(TOP_FOOT_SIZE) || defined(MIN_CHUNK_SIZE) || defined(MAX_SIZE_T)
#error "dlmalloc implementation macros leaked into user code (helper layer)"
#endif

#if defined(ensure_initialization) || defined(internal_malloc) ||              \
    defined(internal_free) || defined(DL_SINGLE_THREADED) ||                  \
    defined(DL_MEM_COMMIT) || defined(DL_MEM_RESERVE) ||                     \
    defined(DL_MEM_RELEASE) || defined(DL_PAGE_READWRITE) ||                 \
    defined(M_TRIM_THRESHOLD) || defined(M_GRANULARITY) ||                     \
    defined(M_MMAP_THRESHOLD)
#error "dlmalloc implementation macros leaked into user code (platform layer)"
#endif

//The Boost.Container extension layer brings a second set of them
#if defined(DL_SIZE_IMPL) || defined(s_allocated_memory) ||                    \
    defined(GET_TRUNCATED_SIZE) || defined(GET_ROUNDED_SIZE) ||                \
    defined(GET_TRUNCATED_PO2_SIZE) || defined(GET_ROUNDED_PO2_SIZE) ||        \
    defined(CALCULATE_GCD) || defined(CALCULATE_LCM) ||                        \
    defined(SQRT_MAX_SIZE_T) ||                                                \
    defined(INTERNAL_MULTIALLOC_DEFAULT_CONTIGUOUS_MEM) ||                     \
    defined(BOOST_ALLOC_PLUS_MEMCHAIN_MEM_JUMP_NEXT)
#error "dlmalloc implementation macros leaked into user code (extension layer)"
#endif

#if defined(DL_MEMIT_NEXT) || defined(DL_MEMIT_ADDR) ||                      \
    defined(DL_MEMCHAIN_BEFORE_BEGIN_IT) || defined(DL_MEMCHAIN_BEGIN_IT) || \
    defined(DL_MEMCHAIN_LAST_IT) || defined(DL_MEMCHAIN_END_IT) ||           \
    defined(DL_MEMCHAIN_IS_END_IT) || defined(DL_MEMCHAIN_FIRSTMEM) ||       \
    defined(DL_MEMCHAIN_LASTMEM) || defined(DL_MEMCHAIN_SIZE) ||             \
    defined(DL_MEMCHAIN_INIT_FROM) || defined(DL_MEMCHAIN_INIT) ||           \
    defined(DL_MEMCHAIN_EMPTY) || defined(DL_MEMCHAIN_PUSH_BACK) ||          \
    defined(DL_MEMCHAIN_PUSH_FRONT) || defined(DL_MEMCHAIN_ERASE_AFTER) ||   \
    defined(DL_MEMCHAIN_POP_FRONT) || defined(DL_MEMCHAIN_INCORPORATE_AFTER)
#error "dlmalloc memory-chain macros leaked into user code"
#endif


//Everything that used to be a function-like macro is a member now, so none
//of these names may be a macro after this header
#if defined(chunksize) || defined(mem2chunk) || defined(chunk2mem) ||          \
    defined(is_small) || defined(is_inuse) || defined(is_mmapped) ||          \
    defined(pinuse) || defined(cinuse) || defined(next_chunk) ||              \
    defined(prev_chunk) || defined(chunk_plus_offset) || defined(set_foot) || \
    defined(smallbin_at) || defined(treebin_at) || defined(idx2bit) ||        \
    defined(compute_tree_index) || defined(compute_bit2idx) ||                \
    defined(insert_chunk) || defined(unlink_chunk) ||                         \
    defined(insert_small_chunk) || defined(unlink_large_chunk) ||             \
    defined(ok_address) || defined(ok_magic) || defined(RTCHECK) ||           \
    defined(set_inuse) || defined(mark_inuse_foot) || defined(segment_holds)
#error "dlmalloc chunk-layer macros leaked into user code"
#endif

#if defined(INITIAL_LOCK) || defined(DESTROY_LOCK) || defined(ACQUIRE_LOCK) || \
    defined(RELEASE_LOCK) || defined(TRY_LOCK) || defined(CALL_MMAP) ||        \
    defined(CALL_MUNMAP) || defined(CALL_DIRECT_MMAP) ||                       \
    defined(CALL_MREMAP) || defined(CALL_MORECORE) ||                          \
    defined(MMAP_DEFAULT) || defined(MUNMAP_DEFAULT) ||                        \
    defined(DIRECT_MMAP_DEFAULT) || defined(MREMAP_DEFAULT) ||                 \
    defined(is_initialized) || defined(page_align) ||                          \
    defined(granularity_align) || defined(mmap_align)
#error "dlmalloc platform macros leaked into user code"
#endif

//HAVE_MMAP-style names are common enough (autoconf gives dozens of unrelated
//libraries the exact same spelling) that dlmalloc.hpp never uses them for
//its own config switches; it spells them BOOST_CONTAINER_DL_MMAP and the
//like instead, and #undefs those - unconditionally, not push/pop_macro'd,
//since nothing else has a reason to define a name this specific - at the
//very end. None of them may survive either.
//
//BOOST_CONTAINER_DLMALLOC_GLIBC_IS_SINGLE_THREAD is header-owned in the
//same way. It only says whether glibc can answer whether the process has
//one thread; single_threaded() reads it, and the header #undefs it too.
#if defined(BOOST_CONTAINER_DL_MORECORE) || defined(BOOST_CONTAINER_DL_MMAP) || \
    defined(BOOST_CONTAINER_DL_MREMAP) || \
    defined(BOOST_CONTAINER_DL_MMAP_CLEARS) || \
    defined(BOOST_CONTAINER_DL_MAP_ANONYMOUS) || \
    defined(BOOST_CONTAINER_DLMALLOC_GLIBC_IS_SINGLE_THREAD)
#error "dlmalloc's renamed HAVE_xxx macros leaked into user code"
#endif

//<cassert>'s assert must be intact (dlmalloc historically hijacked it)
#include <cassert>

int main()
{
   assert(1 + 1 == 2);
   return 0;
}
