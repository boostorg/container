// g28 pattern study probe: instantiates the three walker shapes (full-scan,
// early-exit, dual-range) at segmentation depth 1 (bc::deque, seg_vector),
// depth 2 (seg2_vector) and with segmented destinations, so that per-symbol
// codegen of each variant can be measured from one TU.
#include <boost/container/experimental/segmented_fill.hpp>
#include <boost/container/experimental/segmented_count.hpp>
#include <boost/container/experimental/segmented_find.hpp>
#include <boost/container/experimental/segmented_copy.hpp>
#include <boost/container/deque.hpp>
#include "segmented_test_helper.hpp"
#include <cstddef>

#if defined(BOOST_CONTAINER_G28_EXPECT_SHADOW)
#  if !defined(BOOST_CONTAINER_G28_SHADOW_FILL) || !defined(BOOST_CONTAINER_G28_SHADOW_COUNT) \
   || !defined(BOOST_CONTAINER_G28_SHADOW_FIND) || !defined(BOOST_CONTAINER_G28_SHADOW_COPY)
#     error "g28 shadow headers were not picked up"
#  endif
#endif

namespace bc = boost::container;

typedef bc::deque<int>::iterator                 deq_it;
typedef test_detail::seg_vector<int>::iterator   d1_it;
typedef test_detail::seg2_vector<int>::iterator  d2_it;

// ---- full-scan shape --------------------------------------------------
void probe_fill_deq(deq_it f, deq_it l, const int& v) { bc::segmented_fill(f, l, v); }
void probe_fill_d1 (d1_it f, d1_it l, const int& v)   { bc::segmented_fill(f, l, v); }
void probe_fill_d2 (d2_it f, d2_it l, const int& v)   { bc::segmented_fill(f, l, v); }

std::ptrdiff_t probe_count_deq(deq_it f, deq_it l, const int& v) { return bc::segmented_count(f, l, v); }
std::ptrdiff_t probe_count_d1 (d1_it f, d1_it l, const int& v)   { return bc::segmented_count(f, l, v); }
std::ptrdiff_t probe_count_d2 (d2_it f, d2_it l, const int& v)   { return bc::segmented_count(f, l, v); }

// ---- early-exit shape -------------------------------------------------
deq_it probe_find_deq(deq_it f, deq_it l, const int& v) { return bc::segmented_find(f, l, v); }
d1_it  probe_find_d1 (d1_it f, d1_it l, const int& v)   { return bc::segmented_find(f, l, v); }
d2_it  probe_find_d2 (d2_it f, d2_it l, const int& v)   { return bc::segmented_find(f, l, v); }

// ---- dual-range shape (source walker + destination walker) -------------
deq_it probe_copy_deq_deq(deq_it f, deq_it l, deq_it d)        { return bc::segmented_copy(f, l, d); }
int*   probe_copy_d2_flat(d2_it f, d2_it l, int* d)            { return bc::segmented_copy(f, l, d); }
d2_it  probe_copy_flat_d2(const int* f, const int* l, d2_it d) { return bc::segmented_copy(f, l, d); }
d1_it  probe_copy_d1_d1  (d1_it f, d1_it l, d1_it d)           { return bc::segmented_copy(f, l, d); }
