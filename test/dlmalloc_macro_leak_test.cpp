//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//The header-only dlmalloc implementation defines several short macros
//(gm, fm, chunksize, PREACTION...). They must all be undefined before user
//code is reached.
#include <boost/container/detail/dlmalloc.hpp>

#if defined(gm) || defined(fm) || defined(mparams) || defined(chunksize) || \
    defined(mem2chunk) || defined(chunk2mem) || defined(PREACTION) ||       \
    defined(POSTACTION) || defined(CALL_MMAP) || defined(MFAIL) ||          \
    defined(ABORT) || defined(FOOTERS) || defined(USE_LOCKS) ||             \
    defined(MSPACES) || defined(HAVE_MORECORE) || defined(HAVE_MMAP) ||     \
    defined(DLMALLOC_EXPORT) || defined(FORCEINLINE) ||                     \
    defined(DL_ASSERT) || defined(DL_DEBUG) || defined(MLOCK_T) ||          \
    defined(is_small) || defined(is_global) || defined(page_align) ||       \
    defined(disable_lock) || defined(dev_zero_fd) ||                        \
    defined(s_allocated_memory) || defined(malloc_global_mutex) ||          \
    defined(malloc_corruption_error_count) || defined(DL_WIN32)
#error "dlmalloc implementation macros leaked into user code"
#endif

//<cassert>'s assert must be intact (dlmalloc historically hijacked it)
#include <cassert>

int main()
{
   assert(1 + 1 == 2);
   return 0;
}
