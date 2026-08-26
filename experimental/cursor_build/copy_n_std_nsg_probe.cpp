// Focused std vs nsg copy_n(1S) assembly / timing probe.
#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_copy_n.hpp>
#include <boost/container/experimental/wrapped_iterator.hpp>
#include <algorithm>
#include <cstdio>
#include <cstddef>
#include "../../bench/bench_utils.hpp"

namespace bc = boost::container;

#if defined(_MSC_VER)
#  define NOINLINE __declspec(noinline)
#else
#  define NOINLINE __attribute__((noinline))
#endif

typedef typename bc::deque_options< bc::block_size<128> >::type opt_t;
typedef bc::deque<MyInt, void, opt_t> deq_t;
typedef bc::vector<MyInt>             vec_t;

NOINLINE void do_std(const deq_t& src, vec_t& dst, std::ptrdiff_t n)
{
   std::copy_n(src.begin(), n, dst.begin());
}

NOINLINE void do_nsg(const deq_t& src, vec_t& dst, std::ptrdiff_t n)
{
   typedef bc::wrapped_iterator<deq_t::const_iterator> wit_in;
   typedef bc::wrapped_iterator<vec_t::iterator>        wit_out;
   bc::segmented_copy_n(wit_in(src.begin()), n, wit_out(dst.begin()));
}

NOINLINE void do_seg(const deq_t& src, vec_t& dst, std::ptrdiff_t n)
{
   bc::segmented_copy_n(src.begin(), n, dst.begin());
}

template <class F>
double bench(F f, std::size_t nelem, double budget_ns)
{
   f();
   cpu_timer t(4096);
   const boost::move_detail::nanosecond_type t0 = boost::move_detail::nsec_clock();
   std::size_t reps = 0;
   do {
      t.resume();
      f();
      t.stop();
      ++reps;
   } while (double(boost::move_detail::nsec_clock() - t0) < budget_ns);
   return double(t.elapsed()) / (double(reps) * double(nelem));
}

struct call_std { const deq_t* s; vec_t* d; std::ptrdiff_t n; void operator()() const { do_std(*s, *d, n); } };
struct call_nsg { const deq_t* s; vec_t* d; std::ptrdiff_t n; void operator()() const { do_nsg(*s, *d, n); } };
struct call_seg { const deq_t* s; vec_t* d; std::ptrdiff_t n; void operator()() const { do_seg(*s, *d, n); } };

int main()
{
   const std::size_t N = 100000;
   deq_t src; vec_t dst(N);
   src.resize(N);
   for (std::size_t i = 0; i != N; ++i)
      src[i] = MyInt(int(i));

   const double budget = 2.0e9;
   call_std cs = { &src, &dst, std::ptrdiff_t(N) };
   call_nsg cn = { &src, &dst, std::ptrdiff_t(N) };
   call_seg cg = { &src, &dst, std::ptrdiff_t(N) };

   std::printf("std=%.4f  nsg=%.4f  seg=%.4f  ns/elem\n",
               bench(cs, N, budget), bench(cn, N, budget), bench(cg, N, budget));
   return 0;
}
