// Does the walker's  if (r != fe)  survive, or does inlining the leaf already
// fold it away?  The leaf's loop condition IS  first != last  and the walker
// passes  fe  as that  last , so on the exhaustion path the compiler knows
// r == fe and on the break path it knows r != fe.

#include <iterator>

#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_find.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
#include <boost/container/experimental/segmented_partition_point.hpp>
#include <boost/container/experimental/segmented_is_sorted_until.hpp>

namespace bc = boost::container;

typedef bc::deque_options< bc::block_size<128> >::type opt_t;
typedef bc::deque<int, void, opt_t>                    dq_t;
typedef dq_t::iterator                                 it_t;

struct lt10 { bool operator()(int x) const { return x < 10; } };
struct less_t { bool operator()(int a, int b) const { return a < b; } };

it_t fnd(it_t f, it_t l, const int &v)
{ return bc::segmented_find(f, l, v); }

it_t fndif(it_t f, it_t l)
{ return bc::segmented_find_if(f, l, lt10()); }

it_t ppoint(it_t f, it_t l)
{ return bc::segmented_partition_point(f, l, lt10()); }

it_t sortuntil(it_t f, it_t l)
{ return bc::segmented_is_sorted_until(f, l, less_t()); }
