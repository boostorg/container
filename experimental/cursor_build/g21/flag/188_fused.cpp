// The candidates whose leaves FUSE several termination reasons into one exit
// edge (so the compiler cannot jump-thread the walker's re-test away):
//   copy_n   : if(dst_first == dst_last || first == last) goto out_path;
//   fill_n   : n = min(count, dst_n)            (control: walker tests !count)
//   generate_n: same as fill_n                 (control)
//   partition_copy: walker tests  first == last || r.fourth

#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_copy_n.hpp>
#include <boost/container/experimental/segmented_fill_n.hpp>
#include <boost/container/experimental/segmented_generate_n.hpp>
#include <boost/container/experimental/segmented_partition_copy.hpp>

namespace bc = boost::container;

typedef bc::deque_options< bc::block_size<128> >::type opt_t;
typedef bc::deque<int, void, opt_t>                    dq_t;
typedef dq_t::iterator                                 it_t;

struct lt10 { bool operator()(int x) const { return x < 10; } };
struct gen  { int operator()() { return 7; } };

it_t cpn(it_t f, std::size_t n, it_t d)
{ return bc::segmented_copy_n(f, n, d); }

it_t fln(it_t f, std::size_t n, const int &v)
{ return bc::segmented_fill_n(f, n, v); }

it_t gnn(it_t f, std::size_t n)
{ return bc::segmented_generate_n(f, n, gen()); }

std::pair<it_t, it_t> pcpy(it_t f, it_t l, it_t t, it_t g)
{ return bc::segmented_partition_copy(f, l, t, g, lt10()); }
