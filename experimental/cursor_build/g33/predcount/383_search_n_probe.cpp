#include <boost/container/experimental/segmented_search_n.hpp>
#include <segmented_test_helper.hpp>
#include <cstdio>

using namespace boost::container;

struct probe
{
   int count;
   int value;
   probe(int c, int v) : count(c), value(v) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      test_detail::counted_int_ops().reset();
      segmented_search_n(c.begin(), test_detail::iter_at(c, n), count,
                         test_detail::counted_int(value));
      const std::size_t applied = test_detail::counted_int_ops().cmp;
      if(applied > n) {
         std::printf("spec=%-3s n=%2u count=%d applied=%2u vals=",
                     spec, unsigned(n), count, unsigned(applied));
         boost::container::vector<int> f = test_detail::flatten_n_ints(c, n);
         for(std::size_t i = 0; i != f.size(); ++i) std::printf("%d ", f[i]);
         std::printf("\n");
      }
   }
};

int main()
{
   const std::size_t sizes[] = { 1u, 2u, 3u, 4u, 5u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16] = {0};
      for(int count = 0; count <= int(n) + 1; ++count) {
         for(std::size_t p = 0; p != n; ++p) {
            for(std::size_t len = 1u; p + len <= n; ++len) {
               for(std::size_t i = 0; i != n; ++i)
                  vals[i] = (i >= p && i < p + len) ? 7 : 100 + int(i);
               test_detail::for_each_shape_all<test_detail::counted_int>
                  (vals, n, -999, probe(count, 7));
            }
         }
         for(std::size_t g = 0; g != n; ++g) {
            for(std::size_t i = 0; i != n; ++i)
               vals[i] = (i == g) ? 100 : 7;
            test_detail::for_each_shape_all<test_detail::counted_int>
               (vals, n, -999, probe(count, 7));
         }
      }
   }
   return 0;
}
