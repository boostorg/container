// Comparison-count probe for the set_* / merge family, which this change does
// NOT touch.  Measures whether the "comparison applied, destination found full,
// element not consumed" shape pushes those algorithms past the comparison
// bounds the standard gives them ([set.union] etc: at most 2*(N1+N2)-1;
// [alg.merge]: at most N1+N2-1).

#include <cstdio>
#include <cstddef>

#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <segmented_test_helper.hpp>

#include <boost/container/experimental/segmented_merge.hpp>
#include <boost/container/experimental/segmented_set_union.hpp>
#include <boost/container/experimental/segmented_set_difference.hpp>
#include <boost/container/experimental/segmented_set_intersection.hpp>
#include <boost/container/experimental/segmented_set_symmetric_difference.hpp>

namespace bc = boost::container;

long g_cnt  = 0;
int  g_over = 0;

struct cmp_int
{
   int v;
   cmp_int() : v(0) {}
   cmp_int(int x) : v(x) {}
};

struct cnt_less
{  bool operator()(const cmp_int &a, const cmp_int &b) const { ++g_cnt; return a.v < b.v; }  };

struct mk_flat
{
   typedef bc::vector<cmp_int> cont_t;
   static const char *name() { return "flat"; }
   void make(cont_t &c, std::size_t n) const { c.assign(n, cmp_int(-1)); }
};

template <std::size_t B>
struct mk_deque
{
   typedef typename bc::deque_options<bc::block_size<B> >::type opt_t;
   typedef bc::deque<cmp_int, void, opt_t> cont_t;
   static const char *name() { return B == 2 ? "deque2" : "deque8"; }
   void make(cont_t &c, std::size_t n) const { c.assign(n, cmp_int(-1)); }
};

template <std::size_t S>
struct mk_seg1
{
   typedef test_detail::seg_vector<cmp_int> cont_t;
   static const char *name() { return S == 1 ? "seg1(1)" : "seg1(8)"; }
   void make(cont_t &c, std::size_t n) const
   {
      std::size_t done = 0;
      while(done < n) {
         const std::size_t k = (n - done) < S ? (n - done) : S;
         c.add_segment(k, cmp_int(-1));
         done += k;
      }
   }
};

template <std::size_t S>
struct mk_seg2
{
   typedef test_detail::seg2_vector<cmp_int> cont_t;
   static const char *name() { return S == 1 ? "seg2(1x2)" : "seg2(8x4)"; }
   void make(cont_t &c, std::size_t n) const
   {
      std::size_t done = 0;
      while(done < n) {
         test_detail::seg_vector<cmp_int> inner;
         for(std::size_t j = 0; j != (S == 1 ? 2u : 4u) && done < n; ++j) {
            const std::size_t k = (n - done) < S ? (n - done) : S;
            inner.add_segment(k, cmp_int(-1));
            done += k;
         }
         c.add_segment(inner);
      }
   }
};

void report(const char *algo, const char *dst, std::size_t n1, std::size_t n2,
            long got, long bound)
{
   const bool over = got > bound;
   if(over) ++g_over;
   std::printf("%-22s dst=%-12s n1=%-4lu n2=%-4lu comparisons=%-6ld bound=%-6ld %s\n",
               algo, dst, (unsigned long)n1, (unsigned long)n2, got, bound,
               over ? "OVER BOUND" : "within");
}

template <class Maker>
void run(const Maker &mk, const int *a, std::size_t n1, const int *b, std::size_t n2)
{
   typedef typename Maker::cont_t cont_t;

   bc::vector<cmp_int> r1, r2;
   for(std::size_t i = 0; i != n1; ++i) r1.push_back(cmp_int(a[i]));
   for(std::size_t i = 0; i != n2; ++i) r2.push_back(cmp_int(b[i]));

   const long set_bound   = (long)(2u*(n1+n2)) - 1;
   const long merge_bound = (long)(n1+n2) - 1;

   {  cont_t d; mk.make(d, n1+n2); g_cnt = 0;
      bc::segmented_merge(r1.begin(), r1.end(), r2.begin(), r2.end(), d.begin(), cnt_less());
      report("merge", Maker::name(), n1, n2, g_cnt, merge_bound);  }
   {  cont_t d; mk.make(d, n1+n2); g_cnt = 0;
      bc::segmented_set_union(r1.begin(), r1.end(), r2.begin(), r2.end(), d.begin(), cnt_less());
      report("set_union", Maker::name(), n1, n2, g_cnt, set_bound);  }
   {  cont_t d; mk.make(d, n1+n2); g_cnt = 0;
      bc::segmented_set_difference(r1.begin(), r1.end(), r2.begin(), r2.end(), d.begin(), cnt_less());
      report("set_difference", Maker::name(), n1, n2, g_cnt, set_bound);  }
   {  cont_t d; mk.make(d, n1+n2); g_cnt = 0;
      bc::segmented_set_intersection(r1.begin(), r1.end(), r2.begin(), r2.end(), d.begin(), cnt_less());
      report("set_intersection", Maker::name(), n1, n2, g_cnt, set_bound);  }
   {  cont_t d; mk.make(d, n1+n2); g_cnt = 0;
      bc::segmented_set_symmetric_difference(r1.begin(), r1.end(), r2.begin(), r2.end(), d.begin(), cnt_less());
      report("set_sym_difference", Maker::name(), n1, n2, g_cnt, set_bound);  }
}

int main()
{
   const std::size_t N = 200;
   int a[N], b[N];
   // Interleaved with a shared subsequence, so every set_* algorithm both
   // writes and skips.
   for(std::size_t i = 0; i != N; ++i) {
      a[i] = (int)(2*i);
      b[i] = (int)(3*i);
   }

   run(mk_flat(),      a, N, b, N);
   run(mk_deque<2>(),  a, N, b, N);
   run(mk_deque<8>(),  a, N, b, N);
   run(mk_seg1<1>(),   a, N, b, N);
   run(mk_seg1<8>(),   a, N, b, N);
   run(mk_seg2<1>(),   a, N, b, N);
   run(mk_seg2<8>(),   a, N, b, N);

   std::printf("\nover_bound_cases=%d\n", g_over);
   return 0;
}
