// 364: how does the copy_if 2S gap scale with the destination block size?
//
// Fixed flat bc::vector<MyInt> source of N elements, destination a
// bc::deque<MyInt, block_size<B> > for B in {8..512}.  For each B we time
//   seg : bc::segmented_copy_if with the real (segmented) destination iterator
//   nsg : the same call with both iterators wrapped, so the library takes its
//         non-segmented path and writes through deque_iterator::operator++
// and additionally a flat bc::vector destination as an absolute floor.
//
// Same measurement shape as bench_segmented_algos.cpp (batched, clobber/escape
// barriers, ns/element).

#include <cstddef>
#include <cstdio>
#include <vector>
#include <algorithm>

#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_copy_if.hpp>
#include <boost/container/experimental/wrapped_iterator.hpp>
#include "../../../../bench/bench_utils.hpp"

namespace bc = boost::container;

template<class T> struct is_odd
{  bool operator()(const T& x) const { return (int_value(x) & 1) != 0; } };
template<class T> struct is_negative
{  bool operator()(const T& x) const { return int_value(x) < 0; } };

typedef boost::move_detail::nanosecond_type ns_t;

template<class F>
BOOST_NOINLINE ns_t measure(std::size_t iters, F f)
{
   cpu_timer t;
   t.resume();
   for(std::size_t i = 0; i < iters; ++i)
      f();
   t.stop();
   return t.elapsed();
}

template<bool B> struct wrap_tag {};

template<class SrcC, class DstC, class Pred, bool Wrap>
struct op
{
   const SrcC &c; DstC &out; Pred pred;
   op(const SrcC &c_, DstC &o_, Pred p_) : c(c_), out(o_), pred(p_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {
      clobber();
      run(wrap_tag<Wrap>());
      escape(&*out.begin());
   }
   BOOST_CONTAINER_FORCEINLINE void run(wrap_tag<false>)
   {  bc::segmented_copy_if(c.begin(), c.end(), out.begin(), pred);  }
   BOOST_CONTAINER_FORCEINLINE void run(wrap_tag<true>)
   {
      typedef typename SrcC::const_iterator sit;
      typedef typename DstC::iterator       dit;
      bc::segmented_copy_if(bc::wrapped_iterator<sit>(c.begin()),
                            bc::wrapped_iterator<sit>(c.end()),
                            bc::wrapped_iterator<dit>(out.begin()), pred);
   }
};

static const std::size_t N     = 100000;
static const std::size_t ITERS = 2000;
static const int         REPS  = 5;

double med(std::vector<double> v)
{  std::sort(v.begin(), v.end()); return v[v.size()/2]; }

template<std::size_t B, class Pred>
void sweep_deque(const bc::vector<MyInt> &src, Pred pred, const char *pname)
{
   typedef typename bc::deque_options< bc::block_size<B> >::type opt_t;
   typedef bc::deque<MyInt, void, opt_t> dq_t;
   dq_t out(N);

   std::vector<double> seg, nsg;
   for(int r = 0; r < REPS; ++r) {
      seg.push_back(double(measure(ITERS, op<bc::vector<MyInt>, dq_t, Pred, false>(src, out, pred)))
                    / double(ITERS * N));
      nsg.push_back(double(measure(ITERS, op<bc::vector<MyInt>, dq_t, Pred, true >(src, out, pred)))
                    / double(ITERS * N));
   }
   const double s = med(seg), n = med(nsg);
   std::printf("%-10s block=%-4u  seg=%.4f  nsg=%.4f  nsg/seg=%.3f  seg-nsg=%+.4f ns/el"
               "   per-boundary=%s\n",
      pname, unsigned(B), s, n, n/s, s-n,
      "");
}

template<class Pred>
void flat_control(const bc::vector<MyInt> &src, Pred pred, const char *pname)
{
   bc::vector<MyInt> out(N);
   std::vector<double> seg, nsg;
   for(int r = 0; r < REPS; ++r) {
      seg.push_back(double(measure(ITERS, op<bc::vector<MyInt>, bc::vector<MyInt>, Pred, false>(src, out, pred)))
                    / double(ITERS * N));
      nsg.push_back(double(measure(ITERS, op<bc::vector<MyInt>, bc::vector<MyInt>, Pred, true >(src, out, pred)))
                    / double(ITERS * N));
   }
   std::printf("%-10s FLAT dst    seg=%.4f  nsg=%.4f  nsg/seg=%.3f\n",
      pname, med(seg), med(nsg), med(nsg)/med(seg));
}

template<class Pred>
void run_all(const bc::vector<MyInt> &src, Pred pred, const char *pname)
{
   std::printf("--- predicate: %s ---\n", pname);
   flat_control(src, pred, pname);
   sweep_deque<8>  (src, pred, pname);
   sweep_deque<16> (src, pred, pname);
   sweep_deque<32> (src, pred, pname);
   sweep_deque<64> (src, pred, pname);
   sweep_deque<128>(src, pred, pname);
   sweep_deque<256>(src, pred, pname);
   sweep_deque<512>(src, pred, pname);
   std::printf("\n");
}

int main()
{
   bc::vector<MyInt> src(N);
   for(std::size_t i = 0; i < N; ++i) src[i] = MyInt(int(i));

   run_all(src, is_odd<MyInt>(),      "hit50");
   run_all(src, is_negative<MyInt>(), "miss0");
   return 0;
}
