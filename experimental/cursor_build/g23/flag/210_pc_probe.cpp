// segmented_partition_copy with both outputs segmented two levels deep.
//
// This is the configuration where the walker condition
//    if(first == last || r.fourth)
// has two live tests: out_true is segmented so partition_copy_true_bounded
// hands the false stage a REAL t_last (the true-output-full exit is not folded
// away), and out_false is segmented two levels so the callee of the outer
// partition_copy_false_bounded / _false_dispatch walkers is a non-inlined
// recursive instance of partition_copy_false_bounded rather than the
// force-inlined flat leaf.
//
// DEPTH selects the segmentation depth of both outputs so the depth-1 case
// (force-inlined leaf, condition 3 of the criterion fails) can be compared.

#include <iterator>
#include <utility>
#include <cstddef>

#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include "../../../segmented_test_helper.hpp"

#include <boost/container/experimental/segmented_partition_copy.hpp>

namespace bc = boost::container;

#if !defined(DEPTH)
#define DEPTH 2
#endif

#if DEPTH == 2
typedef test_detail::seg2_vector<int>::iterator out_t;
#else
typedef test_detail::seg_vector<int>::iterator  out_t;
#endif

typedef const int *src_t;

struct lt10_t { bool operator()(int a) const { return a < 10; } };

std::pair<out_t, out_t> run_pc(src_t f, src_t l, out_t t, out_t g)
{ return bc::segmented_partition_copy(f, l, t, g, lt10_t()); }
