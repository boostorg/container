// Focused A/B probe for segmented_copy_n leaf shapes.
// Build twice with different include orders (HEAD header vs working tree).
#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_copy_n.hpp>
#include "../../bench/bench_utils.hpp"
#include <cstdio>
#include <cstddef>

namespace bc = boost::container;

//Per-rep samples through the shared median timer; the wall budget is tracked
//separately because elapsed() sorts the sample vector on every call.
template <class F>
double bench_ns_per_elem(F f, std::size_t nelem, double budget_ns)
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

template <class T>
struct shapes
{
   typedef typename bc::deque_options< bc::block_size<128> >::type opt_t;
   typedef bc::deque<T, void, opt_t> deq_t;
   typedef bc::vector<T>             vec_t;

   deq_t dq, dq_out;
   vec_t vc, vc_out;
   std::size_t n;

   explicit shapes(std::size_t n_) : n(n_)
   {
      dq.resize(n);
      dq_out.resize(n);
      vc.resize(n);
      vc_out.resize(n);
      for (std::size_t i = 0; i != n; ++i) {
         dq[i] = T(int(i));
         vc[i] = T(int(i));
      }
   }

   //deque source -> vector destination (flat dst: bypasses dst_bounded)
   void s1() { bc::segmented_copy_n(dq.begin(), std::ptrdiff_t(n), vc_out.begin()); }
   //vector source -> deque destination (segmented dst: hits the new leaf)
   void s2() { bc::segmented_copy_n(vc.begin(), std::ptrdiff_t(n), dq_out.begin()); }
   //deque -> deque
   void s3() { bc::segmented_copy_n(dq.begin(), std::ptrdiff_t(n), dq_out.begin()); }

   int checksum() const
   {
      int s = 0;
      for (std::size_t i = 0; i < n; i += 997)
         s += vc_out[i].int_value() + dq_out[i].int_value();
      return s;
   }
};

template <class T>
struct call_s1 { shapes<T> *p; void operator()() { p->s1(); } };
template <class T>
struct call_s2 { shapes<T> *p; void operator()() { p->s2(); } };
template <class T>
struct call_s3 { shapes<T> *p; void operator()() { p->s3(); } };

template <class T>
void run(const char* tname, double budget_ns)
{
   const std::size_t N = 100000;
   shapes<T> s(N);
   call_s1<T> c1 = { &s };
   call_s2<T> c2 = { &s };
   call_s3<T> c3 = { &s };
   const double a = bench_ns_per_elem(c1, N, budget_ns);
   const double b = bench_ns_per_elem(c2, N, budget_ns);
   const double c = bench_ns_per_elem(c3, N, budget_ns);
   std::printf("%-14s copy_n(1S)=%.4f  copy_n(2S)=%.4f  copy_n(1+2S)=%.4f  [chk=%d]\n",
               tname, a, b, c, s.checksum());
   std::fflush(stdout);
}

int main()
{
   //~1.5s per shape, 3 shapes, 3 types -> ~14s per build
   const double budget = 1.5e9;
#ifdef COPY_N_BASE
   std::printf("--- form=BASE ---\n");
#else
   std::printf("--- form=NEW ---\n");
#endif
   run<MyInt>("MyInt", budget);
   run<MyFatInt<4> >("MyFatInt<4>", budget);
   run<MyFatInt<8> >("MyFatInt<8>", budget);
   return 0;
}
