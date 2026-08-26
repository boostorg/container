// Which output-producing segmented algorithms accept a destination that is
// itself segmented more than one level?
//
// seg_vector  is segmented one level : its local iterator is int*.
// seg2_vector is segmented two levels: its local iterator is a seg_vector
// iterator, which is itself segmented.  Algorithms that only provide
// destination leaves constrained on !DstTag::value have no overload to call
// once the destination's local iterator is still segmented, so they fail to
// compile.  Compile this file once per ALGO to keep failures independent.

#include <iterator>
#include <utility>
#include <cstddef>

#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include "../../../segmented_test_helper.hpp"

#include <boost/container/experimental/segmented_copy.hpp>
#include <boost/container/experimental/segmented_copy_if.hpp>
#include <boost/container/experimental/segmented_copy_n.hpp>
#include <boost/container/experimental/segmented_transform.hpp>
#include <boost/container/experimental/segmented_remove_copy.hpp>
#include <boost/container/experimental/segmented_remove_copy_if.hpp>
#include <boost/container/experimental/segmented_reverse_copy.hpp>
#include <boost/container/experimental/segmented_swap_ranges.hpp>
#include <boost/container/experimental/segmented_merge.hpp>
#include <boost/container/experimental/segmented_set_union.hpp>
#include <boost/container/experimental/segmented_set_difference.hpp>
#include <boost/container/experimental/segmented_set_intersection.hpp>
#include <boost/container/experimental/segmented_set_symmetric_difference.hpp>
#include <boost/container/experimental/segmented_partition_copy.hpp>

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

struct less_t   { bool operator()(int a, int b) const { return a < b; } };
struct lt10_t   { bool operator()(int a) const { return a < 10; } };
struct twice_t  { int  operator()(int a) const { return a * 2; } };

#if   ALGO == 1
dst_t run(src_t f, src_t l, dst_t d) { return bc::segmented_copy(f, l, d); }
#elif ALGO == 2
dst_t run(src_t f, src_t l, dst_t d) { return bc::segmented_copy_if(f, l, d, lt10_t()); }
#elif ALGO == 3
dst_t run(src_t f, std::size_t n, dst_t d) { return bc::segmented_copy_n(f, n, d); }
#elif ALGO == 4
dst_t run(src_t f, src_t l, dst_t d) { return bc::segmented_transform(f, l, d, twice_t()); }
#elif ALGO == 5
dst_t run(src_t f, src_t l, dst_t d) { return bc::segmented_remove_copy(f, l, d, 3); }
#elif ALGO == 6
dst_t run(src_t f, src_t l, dst_t d) { return bc::segmented_remove_copy_if(f, l, d, lt10_t()); }
#elif ALGO == 7
dst_t run(const int *f, const int *l, dst_t d) { return bc::segmented_reverse_copy(f, l, d); }
#elif ALGO == 8
//swap_ranges writes through both ranges, so its first range cannot be const.
dst_t run(int *f, int *l, dst_t d) { return bc::segmented_swap_ranges(f, l, d); }
#elif ALGO == 9
dst_t run(src_t f1, src_t l1, src_t f2, src_t l2, dst_t d)
{ return bc::segmented_merge(f1, l1, f2, l2, d, less_t()); }
#elif ALGO == 10
dst_t run(src_t f1, src_t l1, src_t f2, src_t l2, dst_t d)
{ return bc::segmented_set_union(f1, l1, f2, l2, d, less_t()); }
#elif ALGO == 11
dst_t run(src_t f1, src_t l1, src_t f2, src_t l2, dst_t d)
{ return bc::segmented_set_difference(f1, l1, f2, l2, d, less_t()); }
#elif ALGO == 12
dst_t run(src_t f1, src_t l1, src_t f2, src_t l2, dst_t d)
{ return bc::segmented_set_intersection(f1, l1, f2, l2, d, less_t()); }
#elif ALGO == 13
dst_t run(src_t f1, src_t l1, src_t f2, src_t l2, dst_t d)
{ return bc::segmented_set_symmetric_difference(f1, l1, f2, l2, d, less_t()); }
#elif ALGO == 14
std::pair<dst_t, dst_t> run(src_t f, src_t l, dst_t t, dst_t g)
{ return bc::segmented_partition_copy(f, l, t, g, lt10_t()); }
#else
#error "no ALGO selected"
#endif
