// The case the bc::deque probe never reached.
//
// With bc::deque the destination is segmented ONE level, so the destination's
// local iterator is a plain int* and merge_dst_bounded's segmented overload is
// never instantiated: every call lands on the force-inlined leaf, which is why
// the compiler could thread the walker's re-test away.
//
// seg2_vector is segmented TWO levels, so the recursive call at
// segmented_merge.hpp:158/168 resolves to the segmented overload itself.  That
// callee is not force-inlined, so the caller cannot know why it stopped and the
//    first1 == last1 || first2 == last2
// re-test at 163/173 has to be evaluated for real, at every nesting level.

#include <iterator>

#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include "../../../segmented_test_helper.hpp"

#include <boost/container/experimental/segmented_merge.hpp>
#include <boost/container/experimental/segmented_set_union.hpp>
#include <boost/container/experimental/segmented_copy.hpp>

namespace bc = boost::container;

typedef test_detail::seg_vector<int>::iterator  d1_t;   // one level of segmentation
typedef test_detail::seg2_vector<int>::iterator d2_t;   // two levels

struct less_t { bool operator()(int a, int b) const { return a < b; } };

// ---- destination segmented ONE level: recursion bottoms out at the leaf ----
d1_t mrg1(const int *f1, const int *l1, const int *f2, const int *l2, d1_t d)
{ return bc::segmented_merge(f1, l1, f2, l2, d, less_t()); }

d1_t uni1(const int *f1, const int *l1, const int *f2, const int *l2, d1_t d)
{ return bc::segmented_set_union(f1, l1, f2, l2, d, less_t()); }

d1_t cpy1(const int *f, const int *l, d1_t d)
{ return bc::segmented_copy(f, l, d); }

// ---- destination segmented TWO levels: instantiates the recursive walker ----
d2_t mrg2(const int *f1, const int *l1, const int *f2, const int *l2, d2_t d)
{ return bc::segmented_merge(f1, l1, f2, l2, d, less_t()); }

// set_union has no segmented-destination dst_bounded overload, so a depth-2
// destination does not compile for it: that is the known "seg2 destination
// tests for set_* once supported" gap, not something this probe can measure.

d2_t cpy2(const int *f, const int *l, d2_t d)
{ return bc::segmented_copy(f, l, d); }
