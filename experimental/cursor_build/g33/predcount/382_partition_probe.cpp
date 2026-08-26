// Probe: which segmentation shapes make segmented_partition exceed the exactly
// N predicate applications [alg.partitions] mandates.
#include <boost/container/experimental/segmented_partition.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>
#include <cstdio>

using namespace boost::container;

struct is_even { bool operator()(int x) const { return x % 2 == 0; } };

struct probe
{
   const char* label;
   explicit probe(const char* l) : label(l) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      const boost::container::vector<int> before = test_detail::flatten_n_ints(c, n);
      test_detail::op_counter calls;
      segmented_partition(c.begin(), test_detail::iter_at(c, n),
                          test_detail::counting_pred(calls, is_even()));
      if(calls.n != n) {
         std::printf("%-10s spec=%-3s n=%2u applied=%2u mandated=%2u vals=",
                     label, spec, unsigned(n), unsigned(calls.n), unsigned(n));
         for(std::size_t i = 0; i != before.size(); ++i)
            std::printf("%d ", before[i]);
         std::printf("\n");
      }
   }
};

// Prints the sequence of values pred is applied to, so the re-applied element
// is identifiable rather than only countable.
struct tracer
{
   bool operator()(int x) const { std::printf("   pred(%d)\n", x); return x % 2 == 0; }
};

template<class Cont>
void trace_one(const char* spec, const int* vals, std::size_t n)
{
   Cont c;
   test_detail::make_range(c, spec, vals, n, -999);
   std::printf("trace spec=%s n=%u\n", spec, unsigned(n));
   segmented_partition(c.begin(), test_detail::iter_at(c, n), tracer());
}

int main()
{
   {
      static const int one[] = {1};
      static const int two[] = {1, 3};
      trace_one<test_detail::seg_vector<int> >("m", one, 1);
      trace_one<test_detail::seg_vector<int> >("m", two, 2);
      trace_one<test_detail::seg_vector<int> >("e", one, 1);
   }

   for(std::size_t n = 0; n <= 6; ++n) {
      int vals[16];
      for(int i = 0; i != 16; ++i) vals[i] = i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -999, probe("alt"));
      test_detail::for_each_shape_all_fwd<int>(vals, n, -999, probe("alt-fwd"));

      for(int i = 0; i != 16; ++i) vals[i] = 2*i + 2;
      test_detail::for_each_shape_all<int>(vals, n, -999, probe("all-even"));
      test_detail::for_each_shape_all_fwd<int>(vals, n, -999, probe("alleven-f"));

      for(int i = 0; i != 16; ++i) vals[i] = 2*i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -999, probe("all-odd"));
      test_detail::for_each_shape_all_fwd<int>(vals, n, -999, probe("allodd-f"));

      for(int i = 0; i != 16; ++i) vals[i] = (i < 3) ? 2*i + 1 : 2*i + 2;
      test_detail::for_each_shape_all<int>(vals, n, -999, probe("rev"));
   }
   return 0;
}
