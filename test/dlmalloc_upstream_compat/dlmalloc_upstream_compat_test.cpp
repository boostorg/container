//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
// dlmalloc against the pristine dlmalloc_2_8_6.c, both driving ONE heap.
//
// dlmalloc is a port of dlmalloc's logic, and a port can drift. This test
// makes the two implementations share a single heap: what dlmalloc
// allocates, dlmalloc frees, reallocates and audits, and the other way
// round. Every free chunk one side files away, the other side has to find in
// the same bin; every size one side writes, the other has to read back. If
// the layouts or the algorithms differ in any detail, dlmalloc's own DEBUG
// checks fire on structures dlmalloc built, or dlmalloc's fire on
// dlmalloc's, or a block simply goes missing. When dlmalloc_2_8_6.c is
// updated, this is what says whether dlmalloc still matches it.
//
// How the two meet on one heap:
//
// - A dlmalloc object is a malloc_state followed by a malloc_params. The
//   mspace handle dlmalloc takes is the address of a malloc_state, so the
//   address of the dlmalloc object IS an mspace. The test verifies that
//   shape against the C structs compiled from the .c itself, per
//   configuration, before relying on it.
//
// - dlmalloc keeps its parameters (page size, granularity, thresholds, and
//   the magic that checks a state) process-wide; dlmalloc keeps them per
//   instance. The test first lets dlmalloc compute its own and checks they
//   equal dlmalloc's - that is the "did the defaults change" detector - then
//   copies dlmalloc's over them, so the magic matches too and footers, when
//   on, verify across the two.
//
// - dlmalloc's first-time initialisation assumes the malloc_state is embedded
//   at the start of the first segment, and offsets top past it (sys_alloc,
//   "Offset top by embedded malloc_state"). A dlmalloc heap keeps its state
//   in the object, so that one step must be dlmalloc's. Each heap is warmed
//   up with one allocation before the C side touches it; top never returns
//   to null afterwards, since neither side ever unmaps the first segment.
//
// - The .c is included several times, each inside a namespace of its own,
//   with a different configuration - see dlmalloc_2_8_6_config_inc.h. Each
//   configuration is spelled once and feeds both sides.
//
// - The dlmalloc of each configuration is built ON the C structs: its Config
//   names them as its layout (see default_dlmalloc_layout), so both sides read and
//   write the heap through one set of types. No access is made through a
//   type the object does not have, and nothing here depends on how a
//   compiler treats aliasing. What that leaves unproven - that dlmalloc's
//   OWN layout is dlmalloc's - is checked first, field for field, between
//   the two compiled type sets.
//////////////////////////////////////////////////////////////////////////////

//Boost.Config first, and before <windows.h>. It tests
//WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP, and a Windows SDK that defines
//WINAPI_FAMILY without that one - MinGW's does - then leaves the comparison
//reading an undefined identifier, which -Wundef reports. With Boost.Config
//read first, WINAPI_FAMILY is not defined yet, the defined() in front of
//the comparison short-circuits, and nothing is read. Every other test gets
//this for free by not including <windows.h> at all.
#include <boost/config.hpp>

#if defined(_WIN32)
#  if !defined(WIN32_LEAN_AND_MEAN)
#     define WIN32_LEAN_AND_MEAN
#  endif
#  if !defined(NOMINMAX)
#     define NOMINMAX
#  endif
#  include <windows.h>
#endif

#include <boost/container/detail/dlmalloc.hpp>
#include <boost/container/detail/spin_mutex.hpp>
#include "../lightweight_test.hpp"

#include <cstddef>
#include <cstring>
#include <cstdio>

//System headers dlmalloc_2_8_6.c includes mid-file. Seen here first, at
//global scope, so the copies inside each namespace expand to nothing.
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#if !defined(_WIN32)
#  include <unistd.h>
#  include <sys/param.h>
#  include <sys/mman.h>
#  include <fcntl.h>
#  include <sched.h>
#endif

//////////////////////////////////////////////////////////////////////////////
//                            The configurations
//////////////////////////////////////////////////////////////////////////////
//Each block is one inclusion of the .c and one dlmalloc Config, from the
//same knobs. Only options the pristine .c has are varied; wide_smallbins and
//rebased_smallbins are off throughout, since the .c has no such thing.

//1. dlmalloc out of the box, with its DEBUG audit on
#define DLC_NS                     cfg_stock
#define DLC_FOOTERS                0
#define DLC_INSECURE               0
#define DLC_MALLOC_INSPECT_ALL     0
#define DLC_NO_SEGMENT_TRAVERSAL   0
#define DLC_USE_LOCKS              1
#define DLC_DEBUG                  1
#define DLC_MALLOC_ALIGNMENT       ((size_t)(2 * sizeof(void *)))
#define DLC_DEFAULT_GRANULARITY    ((size_t)64U * (size_t)1024U)
#define DLC_DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#define DLC_DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)
#define DLC_MAX_RELEASE_CHECK_RATE 4095
#include "dlmalloc_2_8_6_config_inc.h"

//2. footers: every block records its heap, and both sides must agree on
//   how, since the magic goes into the record
#define DLC_NS                     cfg_footers
#define DLC_FOOTERS                1
#define DLC_INSECURE               0
#define DLC_MALLOC_INSPECT_ALL     0
#define DLC_NO_SEGMENT_TRAVERSAL   0
#define DLC_USE_LOCKS              1
#define DLC_DEBUG                  1
#define DLC_MALLOC_ALIGNMENT       ((size_t)(2 * sizeof(void *)))
#define DLC_DEFAULT_GRANULARITY    ((size_t)64U * (size_t)1024U)
#define DLC_DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#define DLC_DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)
#define DLC_MAX_RELEASE_CHECK_RATE 4095
#include "dlmalloc_2_8_6_config_inc.h"

//3. no lock: the state has one member fewer and every field after it moves
#define DLC_NS                     cfg_unlocked
#define DLC_FOOTERS                0
#define DLC_INSECURE               0
#define DLC_MALLOC_INSPECT_ALL     0
#define DLC_NO_SEGMENT_TRAVERSAL   0
#define DLC_USE_LOCKS              0
#define DLC_DEBUG                  1
#define DLC_MALLOC_ALIGNMENT       ((size_t)(2 * sizeof(void *)))
#define DLC_DEFAULT_GRANULARITY    ((size_t)64U * (size_t)1024U)
#define DLC_DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#define DLC_DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)
#define DLC_MAX_RELEASE_CHECK_RATE 4095
#include "dlmalloc_2_8_6_config_inc.h"

//4. the lean production shape: no checks of any kind on either side, no
//   segment traversal, and the inspect_all walker compiled in
#define DLC_NS                     cfg_lean
#define DLC_FOOTERS                0
#define DLC_INSECURE               1
#define DLC_MALLOC_INSPECT_ALL     1
#define DLC_NO_SEGMENT_TRAVERSAL   1
#define DLC_USE_LOCKS              1
#define DLC_DEBUG                  0
#define DLC_MALLOC_ALIGNMENT       ((size_t)(2 * sizeof(void *)))
#define DLC_DEFAULT_GRANULARITY    ((size_t)64U * (size_t)1024U)
#define DLC_DEFAULT_TRIM_THRESHOLD ((size_t)2U * (size_t)1024U * (size_t)1024U)
#define DLC_DEFAULT_MMAP_THRESHOLD ((size_t)256U * (size_t)1024U)
#define DLC_MAX_RELEASE_CHECK_RATE 4095
#include "dlmalloc_2_8_6_config_inc.h"

//5. every numeric knob moved, plus footers: a wider alignment changes the
//   chunk geometry, the small thresholds make trimming and direct mapping
//   frequent, and the low release rate makes segment release frequent
#define DLC_NS                     cfg_tuned
#define DLC_FOOTERS                1
#define DLC_INSECURE               0
#define DLC_MALLOC_INSPECT_ALL     1
#define DLC_NO_SEGMENT_TRAVERSAL   0
#define DLC_USE_LOCKS              1
#define DLC_DEBUG                  1
#define DLC_MALLOC_ALIGNMENT       ((size_t)(4 * sizeof(void *)))
#define DLC_DEFAULT_GRANULARITY    ((size_t)128U * (size_t)1024U)
#define DLC_DEFAULT_TRIM_THRESHOLD ((size_t)64U * (size_t)1024U)
#define DLC_DEFAULT_MMAP_THRESHOLD ((size_t)64U * (size_t)1024U)
#define DLC_MAX_RELEASE_CHECK_RATE 15
#include "dlmalloc_2_8_6_config_inc.h"

//////////////////////////////////////////////////////////////////////////////
//                               The driver
//////////////////////////////////////////////////////////////////////////////
namespace {

//Which implementation performs an operation
enum side { by_class, by_upstream };

side other(side s)
{  return s == by_class ? by_upstream : by_class;  }

//Deterministic, so a failure replays. xorshift32.
struct rng
{
   unsigned s;
   explicit rng(unsigned seed) : s(seed ? seed : 1u) {}
   unsigned next()
   {  s ^= s << 13;  s ^= s >> 17;  s ^= s << 5;  return s;  }
   ::std::size_t below(::std::size_t n)
   {  return (::std::size_t)(next() % (unsigned)n);  }
   side pick_side()
   {  return (next() & 1u) ? by_class : by_upstream;  }
};

//A live block: where it is, how much was asked for, and the byte it is
//filled with, so its contents can be checked before it is freed or after it
//is moved.
struct block
{
   void         *p;
   ::std::size_t size;
   unsigned char tag;
};

void fill(void *p, ::std::size_t n, unsigned char tag)
{  if(n)  ::std::memset(p, tag, n);  }

bool holds(const void *p, ::std::size_t n, unsigned char tag)
{
   const unsigned char *c = static_cast<const unsigned char *>(p);
   for(::std::size_t i = 0; i != n; ++i)
      if(c[i] != tag)  return false;
   return true;
}

bool all_zero(const void *p, ::std::size_t n)
{  return holds(p, n, 0u);  }

bool is_aligned_to(const void *p, ::std::size_t a)
{  return ((::std::size_t)p & (a - 1u)) == 0u;  }

//Requests that land in every part of the heap: the small bins, tree bins of
//several magnitudes, and blocks big enough to be mapped on their own. The
//mmap threshold is read from the heap's parameters, so the spread follows
//the configuration.
::std::size_t pick_size(rng &r, ::std::size_t mmap_threshold)
{
   switch(r.below(8)){
      case 0: case 1: case 2: return r.below(256u);
      case 3: case 4:         return 256u + r.below(4096u);
      case 5:                 return 4096u + r.below(60000u);
      case 6:                 return mmap_threshold / 2u + r.below(mmap_threshold / 2u);
      default:                return mmap_threshold + r.below(3u * 4096u);
   }
}

//---------------------------------------------------------------------------
// One operation, by either side
//---------------------------------------------------------------------------
template<class Api>
void *alloc_by(side s, typename Api::heap_type &h, ::std::size_t n)
{  return s == by_class ? h.allocate(n) : Api::malloc_(Api::handle(h), n);  }

template<class Api>
void free_by(side s, typename Api::heap_type &h, void *p)
{  if(s == by_class)  h.deallocate(p);  else  Api::free_(Api::handle(h), p);  }

template<class Api>
void *realloc_by(side s, typename Api::heap_type &h, void *p, ::std::size_t n)
{  return s == by_class ? h.reallocate(p, n) : Api::realloc_(Api::handle(h), p, n);  }

template<class Api>
void *calloc_by(side s, typename Api::heap_type &h, ::std::size_t n, ::std::size_t e)
{  return s == by_class ? h.allocate_zeroed(n, e) : Api::calloc_(Api::handle(h), n, e);  }

template<class Api>
void *memalign_by(side s, typename Api::heap_type &h, ::std::size_t a, ::std::size_t n)
{  return s == by_class ? h.allocate_aligned(a, n) : Api::memalign_(Api::handle(h), a, n);  }

//---------------------------------------------------------------------------
// Audits
//---------------------------------------------------------------------------
//Both sides walk the whole heap - dlmalloc inside mallinfo, running its
//DEBUG check_malloc_state first when it has one; dlmalloc in check() - and
//each is auditing structures the other one built. Then the figures the two
//report for the same state have to agree.
template<class Api>
void walks_agree(typename Api::heap_type &h)
{
   typename Api::handle_type m = Api::handle(h);
   typename Api::mallinfo_type mi = Api::mallinfo_(m);
   BOOST_TEST(h.check());

   BOOST_TEST(mi.usmblks == h.max_footprint());
   BOOST_TEST(mi.uordblks + mi.fordblks == h.footprint());
   BOOST_TEST(mi.keepcost == Api::state(h)->topsize);

   //dlmalloc's own mallinfo() walks the same heap the C internal_mallinfo
   //just walked, so every field has to come out the same - on every
   //configuration and after every operation.
   const typename Api::heap_type::mallinfo_t bi = h.mallinfo();
   BOOST_TEST(bi.arena    == (::std::size_t)mi.arena);
   BOOST_TEST(bi.ordblks  == (::std::size_t)mi.ordblks);
   BOOST_TEST(bi.smblks   == (::std::size_t)mi.smblks);
   BOOST_TEST(bi.hblks    == (::std::size_t)mi.hblks);
   BOOST_TEST(bi.hblkhd   == (::std::size_t)mi.hblkhd);
   BOOST_TEST(bi.usmblks  == (::std::size_t)mi.usmblks);
   BOOST_TEST(bi.fsmblks  == (::std::size_t)mi.fsmblks);
   BOOST_TEST(bi.uordblks == (::std::size_t)mi.uordblks);
   BOOST_TEST(bi.fordblks == (::std::size_t)mi.fordblks);
   BOOST_TEST(bi.keepcost == (::std::size_t)mi.keepcost);

   BOOST_TEST(Api::footprint_(m)     == h.footprint());
   BOOST_TEST(Api::max_footprint_(m) == h.max_footprint());
}

//---------------------------------------------------------------------------
// 0. dlmalloc's own layout is dlmalloc's, field for field
//---------------------------------------------------------------------------
//The heaps below run on the C structs, so they cannot prove this; it is what
//lets their result carry over to a dlmalloc built with no layout given.
//default_dlmalloc_layout<use_locks> and the C structs are both real compiled types
//here, and every field of every struct must sit at the same offset with the
//same size. Nothing is restated by hand.
template<class A, class FA, class B, class FB>
void same_field(const A &a, const FA &fa, const B &b, const FB &fb)
{
   BOOST_TEST(sizeof(fa) == sizeof(fb));
   BOOST_TEST(((const char *)&fa - (const char *)&a) == ((const char *)&fb - (const char *)&b));
}

template<class N, class C>
void same_lock_field(const N &, const C &, ::boost::container::dtl::false_)
{}
template<class N, class C>
void same_lock_field(const N &n, const C &c, ::boost::container::dtl::true_)
{  same_field(n, n.mutex, c, c.mutex);  }

template<class Api>
void native_layout_matches_c()
{
   typedef ::boost::container::default_dlmalloc_layout<Api::config_type::use_locks> native;
   {
      typename native::malloc_chunk n;  typename Api::chunk_type c;
      BOOST_TEST(sizeof(n) == sizeof(c));
      same_field(n, n.prev_foot, c, c.prev_foot);
      same_field(n, n.head,      c, c.head);
      same_field(n, n.fd,        c, c.fd);
      same_field(n, n.bk,        c, c.bk);
   }
   {
      typename native::malloc_tree_chunk n;  typename Api::tree_chunk_type c;
      BOOST_TEST(sizeof(n) == sizeof(c));
      same_field(n, n.prev_foot, c, c.prev_foot);
      same_field(n, n.head,      c, c.head);
      same_field(n, n.fd,        c, c.fd);
      same_field(n, n.bk,        c, c.bk);
      same_field(n, n.child,     c, c.child);
      same_field(n, n.parent,    c, c.parent);
      same_field(n, n.index,     c, c.index);
   }
   {
      typename native::malloc_segment n;  typename Api::segment_type c;
      BOOST_TEST(sizeof(n) == sizeof(c));
      same_field(n, n.base,   c, c.base);
      same_field(n, n.size,   c, c.size);
      same_field(n, n.next,   c, c.next);
      same_field(n, n.sflags, c, c.sflags);
   }
   {
      typename native::malloc_state n;  typename Api::state_type c;
      BOOST_TEST(sizeof(n) == sizeof(c));
      same_field(n, n.smallmap,        c, c.smallmap);
      same_field(n, n.treemap,         c, c.treemap);
      same_field(n, n.dvsize,          c, c.dvsize);
      same_field(n, n.topsize,         c, c.topsize);
      same_field(n, n.least_addr,      c, c.least_addr);
      same_field(n, n.dv,              c, c.dv);
      same_field(n, n.top,             c, c.top);
      same_field(n, n.trim_check,      c, c.trim_check);
      same_field(n, n.release_checks,  c, c.release_checks);
      same_field(n, n.magic,           c, c.magic);
      same_field(n, n.smallbins,       c, c.smallbins);
      same_field(n, n.treebins,        c, c.treebins);
      same_field(n, n.footprint,       c, c.footprint);
      same_field(n, n.max_footprint,   c, c.max_footprint);
      same_field(n, n.footprint_limit, c, c.footprint_limit);
      same_field(n, n.mflags,          c, c.mflags);
      same_lock_field(n, c, ::boost::container::dtl::bool_<Api::config_type::use_locks>());
      same_field(n, n.seg,             c, c.seg);
      same_field(n, n.extp,            c, c.extp);
      same_field(n, n.exts,            c, c.exts);
   }
   {
      typename native::malloc_params n;  typename Api::params_type c;
      BOOST_TEST(sizeof(n) == sizeof(c));
      same_field(n, n.magic,          c, c.magic);
      same_field(n, n.page_size,      c, c.page_size);
      same_field(n, n.granularity,    c, c.granularity);
      same_field(n, n.mmap_threshold, c, c.mmap_threshold);
      same_field(n, n.trim_threshold, c, c.trim_threshold);
      same_field(n, n.default_mflags, c, c.default_mflags);
   }
}

//---------------------------------------------------------------------------
// 1. Shape and parameters
//---------------------------------------------------------------------------
template<class Api>
void shape_and_parameters(typename Api::heap_type &h)
{
   typedef typename Api::heap_type   heap_type;
   typedef typename Api::state_type  state_type;
   typedef typename Api::params_type params_type;

   //The object is dlmalloc's two structs, state first, and nothing else -
   //which is what makes params_of() point at the params.
   BOOST_TEST(sizeof(heap_type) == sizeof(state_type) + sizeof(params_type));

   const state_type  &st = *Api::state(h);
   const params_type &hp = *Api::params_of(h);
   BOOST_TEST(hp.magic != 0u);
   BOOST_TEST(st.magic == hp.magic);

   //dlmalloc's own start-up has to arrive at the same parameters dlmalloc
   //did from the same knobs. This is where a change of default in a new
   //dlmalloc shows up.
   Api::run_dlmalloc_init();
   params_type &cp = Api::params();
   BOOST_TEST(cp.magic != 0u);
   BOOST_TEST(cp.page_size      == hp.page_size);
   BOOST_TEST(cp.granularity    == hp.granularity);
   BOOST_TEST(cp.mmap_threshold == hp.mmap_threshold);
   BOOST_TEST(cp.trim_threshold == hp.trim_threshold);
   BOOST_TEST(cp.default_mflags == hp.default_mflags);

   //Now the C side adopts dlmalloc's parameters wholesale. The magic comes
   //with them, so the state passes dlmalloc's check and a footer written by
   //either side reads back on the other.
   cp = hp;
}

//---------------------------------------------------------------------------
// 2. Warm-up: the first segment has to be dlmalloc's doing
//---------------------------------------------------------------------------
template<class Api>
void warm_up(typename Api::heap_type &h)
{
   void *p = h.allocate(1u);
   BOOST_TEST(p != 0);
   h.deallocate(p);
   BOOST_TEST(Api::state(h)->top != 0);   //initialized: dlmalloc will not try to be first
}

//---------------------------------------------------------------------------
// 3. The two block walkers report the same blocks, in the same order
//---------------------------------------------------------------------------
//inspect_all is the one entry point that hands the whole heap out block by
//block, so comparing the two walks compares the two views of every chunk in
//it - free ones included, which no other check reaches.
struct walk_log
{
   static const ::std::size_t cap = 4096;
   ::std::size_t n;
   void         *start[cap];
   void         *end[cap];
   ::std::size_t used[cap];
   bool          overflowed;
};

void log_block(void *s, void *e, ::std::size_t u, void *arg)
{
   walk_log &w = *static_cast<walk_log *>(arg);
   if(w.n == walk_log::cap){  w.overflowed = true;  return;  }
   w.start[w.n] = s;  w.end[w.n] = e;  w.used[w.n] = u;  ++w.n;
}

template<class Api>
void walkers_agree(typename Api::heap_type &h)
{
   if(!Api::has_inspect_all)
      return;
   walk_log c; c.n = 0; c.overflowed = false;
   walk_log b; b.n = 0; b.overflowed = false;
   Api::inspect_c(Api::handle(h), &log_block, &c);
   h.inspect_all(&log_block, &b);
   BOOST_TEST(!c.overflowed && !b.overflowed);
   BOOST_TEST(c.n == b.n);
   const ::std::size_t n = c.n < b.n ? c.n : b.n;
   for(::std::size_t i = 0; i != n; ++i){
      BOOST_TEST(c.start[i] == b.start[i]);
      BOOST_TEST(c.end[i]   == b.end[i]);
      BOOST_TEST(c.used[i]  == b.used[i]);
   }
}

//---------------------------------------------------------------------------
// 3b. The inspect_all walker, when compiled in, sees exactly the live blocks
//---------------------------------------------------------------------------
//Runs on the fresh heap, and keeps every block small enough to stay inside
//the first segment: a further segment would add its own record chunk to the
//count, and a directly mapped block would be missing from it.
template<class Api>
void inspect_sees_live_blocks(typename Api::heap_type &h, rng &r)
{
   if(!Api::has_inspect_all)
      return;
   typename Api::handle_type m = Api::handle(h);
   const ::std::size_t n = 48u;
   void *p[n];

   const long before = Api::inspect_inuse_count(m);
   for(::std::size_t i = 0; i != n; ++i){
      p[i] = alloc_by<Api>(r.pick_side(), h, 1u + r.below(512u));
      BOOST_TEST(p[i] != 0);
   }
   BOOST_TEST(Api::inspect_inuse_count(m) - before == (long)n);

   for(::std::size_t i = 0; i != n; i += 2u)
      free_by<Api>(r.pick_side(), h, p[i]);
   BOOST_TEST(Api::inspect_inuse_count(m) - before == (long)(n / 2u));

   for(::std::size_t i = 1u; i < n; i += 2u)
      free_by<Api>(r.pick_side(), h, p[i]);
   BOOST_TEST(Api::inspect_inuse_count(m) == before);
   walkers_agree<Api>(h);
   walks_agree<Api>(h);
}

//---------------------------------------------------------------------------
// 4. Allocated by one side, freed by the other
//---------------------------------------------------------------------------
template<class Api>
void allocated_here_freed_there(typename Api::heap_type &h, side allocator, rng &r)
{
   const ::std::size_t thr = Api::params().mmap_threshold;
   const ::std::size_t n = 128u;
   block b[n];

   for(::std::size_t i = 0; i != n; ++i){
      b[i].size = pick_size(r, thr);
      b[i].tag  = (unsigned char)(i + 1u);
      b[i].p    = alloc_by<Api>(allocator, h, b[i].size);
      BOOST_TEST(b[i].p != 0);
      fill(b[i].p, b[i].size, b[i].tag);
   }

   //Both sides read the same size off every block
   for(::std::size_t i = 0; i != n; ++i){
      const ::std::size_t u = Api::usable_size_(b[i].p);
      BOOST_TEST(u == Api::heap_type::usable_size(b[i].p));
      BOOST_TEST(u >= b[i].size);
   }
   walks_agree<Api>(h);

   //Freed by the other side, in a shuffled order, so coalescing happens in
   //every direction and every bin gets fed by the "wrong" implementation
   ::std::size_t order[n];
   for(::std::size_t i = 0; i != n; ++i)
      order[i] = i;
   for(::std::size_t i = n; i > 1u; --i){
      const ::std::size_t j = r.below(i);
      const ::std::size_t t = order[i - 1u];  order[i - 1u] = order[j];  order[j] = t;
   }
   for(::std::size_t k = 0; k != n; ++k){
      block &x = b[order[k]];
      BOOST_TEST(holds(x.p, x.size, x.tag));
      free_by<Api>(other(allocator), h, x.p);
   }
   walks_agree<Api>(h);
   BOOST_TEST(h.all_deallocated());
}

//---------------------------------------------------------------------------
// 5. Both sides at once, at random, including moves
//---------------------------------------------------------------------------
template<class Api>
void interleaved(typename Api::heap_type &h, rng &r)
{
   const ::std::size_t thr = Api::params().mmap_threshold;
   const ::std::size_t slots = 96u;
   block b[slots] = {};
   unsigned char next_tag = 1u;

   for(unsigned op = 0; op != 4000u; ++op){
      block &x = b[r.below(slots)];
      const side s = r.pick_side();

      if(x.p == 0){
         x.tag = next_tag++;
         if(!next_tag)  next_tag = 1u;
         switch(r.below(3)){
            case 0:
               x.size = pick_size(r, thr);
               x.p = alloc_by<Api>(s, h, x.size);
            break;
            case 1: {
               const ::std::size_t cnt  = 1u + r.below(16u);
               const ::std::size_t each = 1u + r.below(thr / 8u);
               x.size = cnt * each;
               x.p = calloc_by<Api>(s, h, cnt, each);
               BOOST_TEST(x.p != 0 && all_zero(x.p, x.size));
            }
            break;
            default: {
               const ::std::size_t a = (::std::size_t)8u << r.below(10u);
               x.size = pick_size(r, thr);
               x.p = memalign_by<Api>(s, h, a, x.size);
               BOOST_TEST(x.p != 0 && is_aligned_to(x.p, a));
            }
            break;
         }
         BOOST_TEST(x.p != 0);
         fill(x.p, x.size, x.tag);
      }
      else{
         BOOST_TEST(holds(x.p, x.size, x.tag));
         if(r.below(3) == 0){
            free_by<Api>(s, h, x.p);
            x.p = 0;
         }
         else{
            const ::std::size_t nsize = 1u + pick_size(r, thr);
            void *np = realloc_by<Api>(s, h, x.p, nsize);
            BOOST_TEST(np != 0);
            if(np){
               //whichever side moved it, the prefix survived the move
               const ::std::size_t kept = nsize < x.size ? nsize : x.size;
               BOOST_TEST(holds(np, kept, x.tag));
               x.p = np;
               x.size = nsize;
               fill(x.p, x.size, x.tag);
            }
         }
      }

      if((op & 255u) == 255u){
         walks_agree<Api>(h);
         walkers_agree<Api>(h);
      }
   }

   //Drain, alternating sides
   side s = by_class;
   for(::std::size_t i = 0; i != slots; ++i){
      if(b[i].p){
         BOOST_TEST(holds(b[i].p, b[i].size, b[i].tag));
         free_by<Api>(s, h, b[i].p);
         s = other(s);
      }
   }
   walks_agree<Api>(h);
   BOOST_TEST(h.all_deallocated());
}

//---------------------------------------------------------------------------
// 6. The group operations, across the boundary
//---------------------------------------------------------------------------
template<class Api>
void groups(typename Api::heap_type &h, side allocator, rng &r)
{
   typename Api::handle_type m = Api::handle(h);
   const side freer = other(allocator);
   const ::std::size_t thr = Api::params().mmap_threshold;

   //independent_calloc: made as a group by one side, returned one by one by
   //the other - the array itself included, it is a block of the heap too
   {
      const ::std::size_t n = 32u, each = 48u;
      void **arr = allocator == by_class
         ? h.independent_calloc(n, each, 0)
         : Api::independent_calloc_(m, n, each, 0);
      BOOST_TEST(arr != 0);
      if(arr){
         for(::std::size_t i = 0; i != n; ++i){
            BOOST_TEST(arr[i] != 0 && all_zero(arr[i], each));
            free_by<Api>(freer, h, arr[i]);
         }
         free_by<Api>(freer, h, arr);
      }
   }

   //independent_comalloc, with sizes of every kind
   {
      const ::std::size_t n = 16u;
      ::std::size_t sizes[n];
      for(::std::size_t i = 0; i != n; ++i)
         sizes[i] = 1u + r.below(300u);
      void **arr = allocator == by_class
         ? h.independent_comalloc(n, sizes, 0)
         : Api::independent_comalloc_(m, n, sizes, 0);
      BOOST_TEST(arr != 0);
      if(arr){
         for(::std::size_t i = 0; i != n; ++i){
            BOOST_TEST(arr[i] != 0);
            fill(arr[i], sizes[i], (unsigned char)(0xA0u + i));
         }
         for(::std::size_t i = 0; i != n; ++i){
            BOOST_TEST(holds(arr[i], sizes[i], (unsigned char)(0xA0u + i)));
            free_by<Api>(freer, h, arr[i]);
         }
         free_by<Api>(freer, h, arr);
      }
   }

   //bulk_free: allocated one by one by one side, handed back as one array
   //by the other, which coalesces neighbours as it goes
   {
      const ::std::size_t n = 40u;
      void *blocks[n];
      for(::std::size_t i = 0; i != n; ++i){
         blocks[i] = alloc_by<Api>(allocator, h, pick_size(r, thr));
         BOOST_TEST(blocks[i] != 0);
      }
      const ::std::size_t unfreed = freer == by_class
         ? h.bulk_free(blocks, n)
         : Api::bulk_free_(m, blocks, n);
      BOOST_TEST(unfreed == 0u);
   }

   walks_agree<Api>(h);
   BOOST_TEST(h.all_deallocated());
}

//---------------------------------------------------------------------------
// 7. Giving memory back, from either side
//---------------------------------------------------------------------------
template<class Api>
void trims(typename Api::heap_type &h)
{
   (void)Api::trim_(Api::handle(h));
   walks_agree<Api>(h);
   (void)h.trim(0u);
   walks_agree<Api>(h);
   BOOST_TEST(h.all_deallocated());
}

//---------------------------------------------------------------------------
// One configuration, start to finish, on one heap
//---------------------------------------------------------------------------
template<class Api>
void run(unsigned seed)
{
   ::std::printf("dlmalloc <-> dlmalloc_2_8_6.c: %s\n", Api::name());
   rng r(seed);

   native_layout_matches_c<Api>();
   typename Api::heap_type h(0u, Api::config_type::use_locks);
   shape_and_parameters<Api>(h);
   warm_up<Api>(h);
   inspect_sees_live_blocks<Api>(h, r);

   allocated_here_freed_there<Api>(h, by_class,    r);
   allocated_here_freed_there<Api>(h, by_upstream, r);
   interleaved<Api>(h, r);
   groups<Api>(h, by_class,    r);
   groups<Api>(h, by_upstream, r);
   trims<Api>(h);

   //dlmalloc never re-ran its start-up behind the test's back
   BOOST_TEST(Api::params().magic == Api::params_of(h)->magic);
}

}  //namespace

int main()
{
   run<cfg_stock::api>   (0x9E3779B9u);
   run<cfg_footers::api> (0x7F4A7C15u);
   run<cfg_unlocked::api>(0x85EBCA6Bu);
   run<cfg_lean::api>    (0xC2B2AE35u);
   run<cfg_tuned::api>   (0x27D4EB2Fu);
   return ::boost::report_errors();
}
