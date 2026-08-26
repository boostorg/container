// The rest of the algorithms, instantiated with two-level segmented ranges.
// The survey in 202_survey.cpp covered the ones that write through a separate
// destination; this one covers the single-range algorithms (whose output, when
// they have one, is the range being walked) and the remaining multi-range ones,
// so that "depth 2 is supported" can be claimed for the whole set rather than
// for the copy-style family alone.

#include <iterator>
#include <utility>
#include <cstddef>

#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include "../../../segmented_test_helper.hpp"

#include <boost/container/experimental/segmented_all_of.hpp>
#include <boost/container/experimental/segmented_any_of.hpp>
#include <boost/container/experimental/segmented_none_of.hpp>
#include <boost/container/experimental/segmented_count.hpp>
#include <boost/container/experimental/segmented_count_if.hpp>
#include <boost/container/experimental/segmented_equal.hpp>
#include <boost/container/experimental/segmented_fill.hpp>
#include <boost/container/experimental/segmented_fill_n.hpp>
#include <boost/container/experimental/segmented_find.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
#include <boost/container/experimental/segmented_find_if_not.hpp>
#include <boost/container/experimental/segmented_find_last.hpp>
#include <boost/container/experimental/segmented_find_last_if.hpp>
#include <boost/container/experimental/segmented_find_last_if_not.hpp>
#include <boost/container/experimental/segmented_for_each.hpp>
#include <boost/container/experimental/segmented_generate.hpp>
#include <boost/container/experimental/segmented_generate_n.hpp>
#include <boost/container/experimental/segmented_is_partitioned.hpp>
#include <boost/container/experimental/segmented_is_sorted.hpp>
#include <boost/container/experimental/segmented_is_sorted_until.hpp>
#include <boost/container/experimental/segmented_mismatch.hpp>
#include <boost/container/experimental/segmented_partition.hpp>
#include <boost/container/experimental/segmented_partition_point.hpp>
#include <boost/container/experimental/segmented_remove.hpp>
#include <boost/container/experimental/segmented_remove_if.hpp>
#include <boost/container/experimental/segmented_replace.hpp>
#include <boost/container/experimental/segmented_replace_if.hpp>
#include <boost/container/experimental/segmented_reverse.hpp>
#include <boost/container/experimental/segmented_search.hpp>
#include <boost/container/experimental/segmented_search_n.hpp>
#include <boost/container/experimental/segmented_stable_partition.hpp>

namespace bc = boost::container;

#if !defined(DEPTH)
#define DEPTH 2
#endif

#if DEPTH == 2
typedef test_detail::seg2_vector<int>::iterator it_t;
#else
typedef test_detail::seg_vector<int>::iterator  it_t;
#endif

struct lt10_t  { bool operator()(int a) const { return a < 10; } };
struct less_t  { bool operator()(int a, int b) const { return a < b; } };
struct eq_t    { bool operator()(int a, int b) const { return a == b; } };
struct noop_t  { void operator()(int&) const {} };
struct gen_t   { int  operator()() const { return 7; } };

#if   ALGO == 1
bool run(it_t f, it_t l) { return bc::segmented_all_of(f, l, lt10_t()); }
#elif ALGO == 2
bool run(it_t f, it_t l) { return bc::segmented_any_of(f, l, lt10_t()); }
#elif ALGO == 3
bool run(it_t f, it_t l) { return bc::segmented_none_of(f, l, lt10_t()); }
#elif ALGO == 4
std::ptrdiff_t run(it_t f, it_t l) { return bc::segmented_count(f, l, 3); }
#elif ALGO == 5
std::ptrdiff_t run(it_t f, it_t l) { return bc::segmented_count_if(f, l, lt10_t()); }
#elif ALGO == 6
bool run(it_t f, it_t l, it_t g) { return bc::segmented_equal(f, l, g); }
#elif ALGO == 7
void run(it_t f, it_t l) { bc::segmented_fill(f, l, 3); }
#elif ALGO == 8
it_t run(it_t f, std::size_t n) { return bc::segmented_fill_n(f, n, 3); }
#elif ALGO == 9
it_t run(it_t f, it_t l) { return bc::segmented_find(f, l, 3); }
#elif ALGO == 10
it_t run(it_t f, it_t l) { return bc::segmented_find_if(f, l, lt10_t()); }
#elif ALGO == 11
it_t run(it_t f, it_t l) { return bc::segmented_find_if_not(f, l, lt10_t()); }
#elif ALGO == 12
it_t run(it_t f, it_t l) { return bc::segmented_find_last(f, l, 3); }
#elif ALGO == 13
it_t run(it_t f, it_t l) { return bc::segmented_find_last_if(f, l, lt10_t()); }
#elif ALGO == 14
it_t run(it_t f, it_t l) { return bc::segmented_find_last_if_not(f, l, lt10_t()); }
#elif ALGO == 15
noop_t run(it_t f, it_t l) { return bc::segmented_for_each(f, l, noop_t()); }
#elif ALGO == 16
void run(it_t f, it_t l) { bc::segmented_generate(f, l, gen_t()); }
#elif ALGO == 17
it_t run(it_t f, std::size_t n) { return bc::segmented_generate_n(f, n, gen_t()); }
#elif ALGO == 18
bool run(it_t f, it_t l) { return bc::segmented_is_partitioned(f, l, lt10_t()); }
#elif ALGO == 19
bool run(it_t f, it_t l) { return bc::segmented_is_sorted(f, l); }
#elif ALGO == 20
it_t run(it_t f, it_t l) { return bc::segmented_is_sorted_until(f, l); }
#elif ALGO == 21
std::pair<it_t, it_t> run(it_t f, it_t l, it_t g) { return bc::segmented_mismatch(f, l, g); }
#elif ALGO == 22
it_t run(it_t f, it_t l) { return bc::segmented_partition(f, l, lt10_t()); }
#elif ALGO == 23
it_t run(it_t f, it_t l) { return bc::segmented_partition_point(f, l, lt10_t()); }
#elif ALGO == 24
it_t run(it_t f, it_t l) { return bc::segmented_remove(f, l, 3); }
#elif ALGO == 25
it_t run(it_t f, it_t l) { return bc::segmented_remove_if(f, l, lt10_t()); }
#elif ALGO == 26
void run(it_t f, it_t l) { bc::segmented_replace(f, l, 3, 4); }
#elif ALGO == 27
void run(it_t f, it_t l) { bc::segmented_replace_if(f, l, lt10_t(), 4); }
#elif ALGO == 28
void run(it_t f, it_t l) { bc::segmented_reverse(f, l); }
#elif ALGO == 29
it_t run(it_t f, it_t l, it_t g, it_t h) { return bc::segmented_search(f, l, g, h); }
#elif ALGO == 30
it_t run(it_t f, it_t l) { return bc::segmented_search_n(f, l, 2, 3); }
#elif ALGO == 31
it_t run(it_t f, it_t l) { return bc::segmented_stable_partition(f, l, lt10_t()); }
#else
#error "no ALGO selected"
#endif
