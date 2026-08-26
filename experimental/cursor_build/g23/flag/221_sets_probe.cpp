// Instantiates one segmented_set_* algorithm against a destination segmented
// DEPTH levels deep, so the walkers can be disassembled per symbol.
//
// DEPTH=1: the destination's local iterator is int*, so *_until_exhausts calls
//          the force-inlined flat leaf directly (criterion condition 3 fails).
// DEPTH=2: *_until_exhausts calls the segmented *_dst_bounded overload, which
//          is a real, non-inlined call (condition 3 holds); that overload in
//          turn calls the force-inlined flat leaf.

#include <iterator>
#include <utility>
#include <cstddef>

#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include "../../../segmented_test_helper.hpp"

#include <boost/container/experimental/segmented_set_union.hpp>
#include <boost/container/experimental/segmented_set_difference.hpp>
#include <boost/container/experimental/segmented_set_intersection.hpp>
#include <boost/container/experimental/segmented_set_symmetric_difference.hpp>

namespace bc = boost::container;

#if !defined(DEPTH)
#define DEPTH 2
#endif

#if DEPTH == 2
typedef test_detail::seg2_vector<int>::iterator dst_t;
#else
typedef test_detail::seg_vector<int>::iterator  dst_t;
#endif

typedef const int *src_t;

struct less_t { bool operator()(int a, int b) const { return a < b; } };

#if   ALGO == 1
dst_t run_set(src_t f1, src_t l1, src_t f2, src_t l2, dst_t d)
{ return bc::segmented_set_union(f1, l1, f2, l2, d, less_t()); }
#elif ALGO == 2
dst_t run_set(src_t f1, src_t l1, src_t f2, src_t l2, dst_t d)
{ return bc::segmented_set_difference(f1, l1, f2, l2, d, less_t()); }
#elif ALGO == 3
dst_t run_set(src_t f1, src_t l1, src_t f2, src_t l2, dst_t d)
{ return bc::segmented_set_intersection(f1, l1, f2, l2, d, less_t()); }
#elif ALGO == 4
dst_t run_set(src_t f1, src_t l1, src_t f2, src_t l2, dst_t d)
{ return bc::segmented_set_symmetric_difference(f1, l1, f2, l2, d, less_t()); }
#else
#error "no ALGO selected"
#endif
