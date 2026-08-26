// Two questions:
//  (a) copy family: the walker re-tests  first == last , which is also the
//      leaf's own loop condition -> should fold like find did.
//  (b) merge/set_* family: the leaf's loop condition is a THREE-way conjunction
//      (first1 != last1 && first2 != last2 && dst != dst_last) and the dst
//      walker re-tests  first1 == last1 || first2 == last2 .  On loop exit the
//      compiler knows the conjunction failed but not WHICH conjunct, so this
//      one cannot fold.  Count the compares at the segment transition.

#include <iterator>
#include <utility>

#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_copy.hpp>
#include <boost/container/experimental/segmented_copy_if.hpp>
#include <boost/container/experimental/segmented_transform.hpp>
#include <boost/container/experimental/segmented_merge.hpp>
#include <boost/container/experimental/segmented_set_union.hpp>
#include <boost/container/experimental/segmented_set_difference.hpp>
#include <boost/container/experimental/segmented_set_intersection.hpp>
#include <boost/container/experimental/segmented_set_symmetric_difference.hpp>
#include <boost/container/experimental/segmented_partition_copy.hpp>

namespace bc = boost::container;

typedef bc::deque_options< bc::block_size<128> >::type opt_t;
typedef bc::deque<int, void, opt_t>                    dq_t;
typedef dq_t::iterator                                 it_t;

struct lt10   { bool operator()(int x) const { return x < 10; } };
struct less_t { bool operator()(int a, int b) const { return a < b; } };
struct dbl    { int  operator()(int x) const { return x + x; } };

// (a) copy family: segmented source -> segmented destination
it_t cpy(it_t f, it_t l, it_t d)
{ return bc::segmented_copy(f, l, d); }

it_t cpyif(it_t f, it_t l, it_t d)
{ return bc::segmented_copy_if(f, l, d, lt10()); }

it_t xform(it_t f, it_t l, it_t d)
{ return bc::segmented_transform(f, l, d, dbl()); }

// (b) merge / set_* : two segmented inputs -> segmented destination
it_t mrg(it_t f1, it_t l1, it_t f2, it_t l2, it_t d)
{ return bc::segmented_merge(f1, l1, f2, l2, d, less_t()); }

it_t suni(it_t f1, it_t l1, it_t f2, it_t l2, it_t d)
{ return bc::segmented_set_union(f1, l1, f2, l2, d, less_t()); }

it_t sdif(it_t f1, it_t l1, it_t f2, it_t l2, it_t d)
{ return bc::segmented_set_difference(f1, l1, f2, l2, d, less_t()); }

it_t sint(it_t f1, it_t l1, it_t f2, it_t l2, it_t d)
{ return bc::segmented_set_intersection(f1, l1, f2, l2, d, less_t()); }

it_t ssym(it_t f1, it_t l1, it_t f2, it_t l2, it_t d)
{ return bc::segmented_set_symmetric_difference(f1, l1, f2, l2, d, less_t()); }

std::pair<it_t, it_t> pcpy(it_t f, it_t l, it_t t, it_t g)
{ return bc::segmented_partition_copy(f, l, t, g, lt10()); }
