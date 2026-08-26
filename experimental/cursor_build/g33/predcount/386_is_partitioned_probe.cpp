#include <boost/container/experimental/segmented_is_partitioned.hpp>
#include <segmented_test_helper.hpp>
#include <cstdio>

using namespace boost::container;

struct is_even { bool operator()(int a) const { return a % 2 == 0; } };

struct probe
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      test_detail::op_counter calls;
      segmented_is_partitioned(c.begin(), test_detail::iter_at(c, n),
                               test_detail::counting_pred(calls, is_even()));
      if(calls.n > n) {
         std::printf("spec=%-3s n=%2u applied=%2u mandated=%2u vals=",
                     spec, unsigned(n), unsigned(calls.n), unsigned(n));
         boost::container::vector<int> f = test_detail::flatten_n_ints(c, n);
         for(std::size_t i = 0; i != f.size(); ++i) std::printf("%d ", f[i]);
         std::printf("\n");
      }
   }
};

int main()
{
   const std::size_t sizes[] = { 1u, 2u, 3u, 4u, 5u, 6u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];
      for(std::size_t k = 0; k <= n; ++k) {   // k leading evens, rest odd
         for(std::size_t i = 0; i != n; ++i)
            vals[i] = (i < k) ? 2*int(i) + 2 : 2*int(i) + 1;
         test_detail::for_each_shape_all<int>(vals, n, -999, probe());
      }
   }
   return 0;
}
