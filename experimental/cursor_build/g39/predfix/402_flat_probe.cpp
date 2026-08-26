// Probe: the [alg.search] comparison bounds on *flat* (non-segmented) ranges,
// which the shape combinators in segmented_test_helper.hpp never build.
#include <boost/container/experimental/segmented_search_n.hpp>
#include <boost/container/experimental/segmented_search.hpp>
#include <boost/container/experimental/segmented_partition.hpp>
#include <boost/container/experimental/segmented_is_partitioned.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>
#include <cstdio>

using namespace boost::container;

struct is_even { bool operator()(int x) const { return x % 2 == 0; } };

static unsigned bad = 0;

static void probe_search_n(const int* vals, std::size_t n, int count)
{
   vector<test_detail::counted_int> v;
   for(std::size_t i = 0; i != n; ++i) v.push_back(test_detail::counted_int(vals[i]));
   test_detail::counted_int_ops().reset();
   segmented_search_n(v.begin(), v.end(), count, test_detail::counted_int(7));
   const std::size_t applied = test_detail::counted_int_ops().cmp;
   if(applied > n) {
      ++bad;
      std::printf("search_n  n=%2u count=%d applied=%2u bound=%2u vals=",
                  unsigned(n), count, unsigned(applied), unsigned(n));
      for(std::size_t i = 0; i != n; ++i) std::printf("%d ", vals[i]);
      std::printf("\n");
   }
}

static void probe_search(const int* h, std::size_t n1, const int* d, std::size_t n2)
{
   vector<test_detail::counted_int> a, b;
   for(std::size_t i = 0; i != n1; ++i) a.push_back(test_detail::counted_int(h[i]));
   for(std::size_t i = 0; i != n2; ++i) b.push_back(test_detail::counted_int(d[i]));
   test_detail::counted_int_ops().reset();
   segmented_search(a.begin(), a.end(), b.begin(), b.end());
   const std::size_t applied = test_detail::counted_int_ops().cmp;
   if(applied > n1 * n2) {
      ++bad;
      std::printf("search    n1=%2u n2=%2u applied=%2u bound=%2u\n",
                  unsigned(n1), unsigned(n2), unsigned(applied), unsigned(n1*n2));
   }
}

static void probe_partition(const int* vals, std::size_t n)
{
   vector<int> v(vals, vals + n);
   test_detail::op_counter calls;
   segmented_partition(v.begin(), v.end(), test_detail::counting_pred(calls, is_even()));
   if(calls.n != n) {
      ++bad;
      std::printf("partition n=%2u applied=%2u mandated=%2u vals=",
                  unsigned(n), unsigned(calls.n), unsigned(n));
      for(std::size_t i = 0; i != n; ++i) std::printf("%d ", vals[i]);
      std::printf("\n");
   }
}

static void probe_is_partitioned(const int* vals, std::size_t n)
{
   vector<int> v(vals, vals + n);
   test_detail::op_counter calls;
   segmented_is_partitioned(v.begin(), v.end(), test_detail::counting_pred(calls, is_even()));
   if(calls.n > n) {
      ++bad;
      std::printf("is_part   n=%2u applied=%2u bound=%2u\n",
                  unsigned(n), unsigned(calls.n), unsigned(n));
   }
}

int main()
{
   // Exhaustive over 7-or-fewer element ranges made of {7, other}.
   for(std::size_t n = 0; n <= 7; ++n) {
      const unsigned lim = 1u << n;
      for(unsigned mask = 0; mask != lim; ++mask) {
         int vals[8];
         for(std::size_t i = 0; i != n; ++i)
            vals[i] = (mask >> i) & 1u ? 7 : 100 + int(i);
         for(int count = 0; count <= int(n) + 1; ++count)
            probe_search_n(vals, n, count);
         probe_is_partitioned(vals, n);

         int pv[8];
         for(std::size_t i = 0; i != n; ++i)
            pv[i] = ((mask >> i) & 1u) ? 2*int(i) + 2 : 2*int(i) + 1;
         probe_partition(pv, n);
      }
   }

   // search: every {0,1} haystack x needle pair up to 5 x 4.
   for(std::size_t n1 = 0; n1 <= 5; ++n1) {
      for(unsigned m1 = 0; m1 != (1u << n1); ++m1) {
         int h[8];
         for(std::size_t i = 0; i != n1; ++i) h[i] = int((m1 >> i) & 1u);
         for(std::size_t n2 = 0; n2 <= 4; ++n2) {
            for(unsigned m2 = 0; m2 != (1u << n2); ++m2) {
               int d[8];
               for(std::size_t i = 0; i != n2; ++i) d[i] = int((m2 >> i) & 1u);
               probe_search(h, n1, d, n2);
            }
         }
      }
   }

   std::printf("flat probe: %u breaches\n", bad);
   return bad != 0;
}
