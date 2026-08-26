// Pins the find_end verify leaf, the segmented verify walkers at one and two
// levels, and the two public scans into named non-inlinable symbols so that
// nm/objdump can be diffed across candidate return shapes.
//
// FE_HAS_FLAG selects how the caller of the leaf obtains the stop condition:
// from the returned flag, or by recomputing the compound the baseline walker
// has to evaluate itself.  Without that the flag would be dead code in the
// variant builds and every measurement would be meaningless.

#include <boost/container/experimental/segmented_find_end.hpp>
#include <boost/container/deque.hpp>
#include "segmented_test_helper.hpp"

#include <iterator>

namespace bc = boost::container;
namespace da = bc::detail_algo;

#define NOINL __attribute__((noinline))

#ifdef FE_HAS_FLAG
#  define STOP_OF(r, last1, last2) (r.third)
#else
#  define STOP_OF(r, last1, last2) (r.first != (last1) || r.second == (last2))
#endif

typedef test_detail::seg_vector<int, std::random_access_iterator_tag>  segv_t;
typedef segv_t::iterator                                               segit_t;
typedef test_detail::seg2_vector<int, std::random_access_iterator_tag> segv2_t;
typedef segv2_t::iterator                                              seg2it_t;

#ifndef FE_ENTRYPOINTS_ONLY

//------------------------------------------------------------- flat leaf ---

extern "C" NOINL void verify_leaf
   (const int *f1, const int *l1, const int *f2, const int *l2,
    const int **o1, const int **o2, bool *o3)
{
   const
#ifdef FE_HAS_FLAG
      bc::segtrio<const int*, const int*, bool>
#else
      bc::segduo<const int*, const int*>
#endif
      r = (da::find_end_verify)
         (f1, l1, f2, l2, da::find_end_equal(), bc::non_segmented_iterator_tag());
   *o1 = r.first; *o2 = r.second; *o3 = STOP_OF(r, l1, l2);
}

//-------------------------------------------------------- verify walkers ---

extern "C" NOINL void verify_walk1
   (segit_t f1, segit_t l1, const int *f2, const int *l2, const int **o2, bool *o3)
{
   const
#ifdef FE_HAS_FLAG
      bc::segtrio<segit_t, const int*, bool>
#else
      bc::segduo<segit_t, const int*>
#endif
      r = (da::find_end_verify)
         (f1, l1, f2, l2, da::find_end_equal(), bc::segmented_iterator_tag());
   *o2 = r.second; *o3 = STOP_OF(r, l1, l2);
}

extern "C" NOINL void verify_walk2
   (seg2it_t f1, seg2it_t l1, const int *f2, const int *l2, const int **o2, bool *o3)
{
   const
#ifdef FE_HAS_FLAG
      bc::segtrio<seg2it_t, const int*, bool>
#else
      bc::segduo<seg2it_t, const int*>
#endif
      r = (da::find_end_verify)
         (f1, l1, f2, l2, da::find_end_equal(), bc::segmented_iterator_tag());
   *o2 = r.second; *o3 = STOP_OF(r, l1, l2);
}

#endif // FE_ENTRYPOINTS_ONLY

//--------------------------------------------------------- public scans ----
// These carry no probe-side recomputation of the stop condition, so a TU built
// with FE_ENTRYPOINTS_ONLY compares the shapes without charging the baseline
// for work its real callers never do.

extern "C" NOINL bool fe_bwd_seg(segit_t f1, segit_t l1, const int *f2, const int *l2)
{  return bc::segmented_find_end(f1, l1, f2, l2) == l1;   }

extern "C" NOINL bool fe_fwd_seg(segit_t f1, segit_t l1, const int *f2, const int *l2)
{
   return bc::segmented_find_end
      (f1, test_detail::make_sentinel(l1), f2, test_detail::make_sentinel(l2)) == l1;
}

extern "C" NOINL bool fe_bwd_seg2(seg2it_t f1, seg2it_t l1, const int *f2, const int *l2)
{  return bc::segmented_find_end(f1, l1, f2, l2) == l1;   }

typedef bc::deque<int>::const_iterator dqit_t;

extern "C" NOINL bool fe_deque(dqit_t f1, dqit_t l1, const int *f2, const int *l2)
{  return bc::segmented_find_end(f1, l1, f2, l2) == l1;   }
