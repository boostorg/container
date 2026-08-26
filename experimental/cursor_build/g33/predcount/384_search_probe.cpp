#include <boost/container/experimental/segmented_search.hpp>
#include <segmented_test_helper.hpp>
#include <cstdio>

using namespace boost::container;

struct probe
{
   template<class C1, class C2>
   void operator()(C1& hay, std::size_t n1, const char* s1,
                   C2& ndl, std::size_t n2, const char* s2) const
   {
      test_detail::counted_int_ops().reset();
      segmented_search(hay.begin(), test_detail::iter_at(hay, n1),
                       ndl.begin(), test_detail::iter_at(ndl, n2));
      const std::size_t applied = test_detail::counted_int_ops().cmp;
      if(applied > n1*n2) {
         std::printf("hay=%-3s(%u) ndl=%-3s(%u) applied=%2u bound=%2u  hay=",
                     s1, unsigned(n1), s2, unsigned(n2), unsigned(applied), unsigned(n1*n2));
         boost::container::vector<int> h = test_detail::flatten_n_ints(hay, n1);
         for(std::size_t i = 0; i != h.size(); ++i) std::printf("%d ", h[i]);
         std::printf(" ndl=");
         boost::container::vector<int> p = test_detail::flatten_n_ints(ndl, n2);
         for(std::size_t i = 0; i != p.size(); ++i) std::printf("%d ", p[i]);
         std::printf("\n");
      }
   }
};

int main()
{
   const std::size_t sizes[] = { 1u, 2u, 3u, 4u, 5u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n1 = sizes[s];
      int hay[16] = {0};
      int ndl[16] = {0};
      for(std::size_t i = 0; i != n1; ++i)
         hay[i] = int(i % 3u) + 1;
      for(std::size_t len = 0u; len <= n1; ++len) {
         for(std::size_t off = 0; off + len <= n1; ++off) {
            for(std::size_t j = 0; j != len; ++j) ndl[j] = hay[off + j];
            test_detail::for_each_shape2_all<test_detail::counted_int, test_detail::counted_int>
               (hay, n1, ndl, len, -999, probe());
            if(len) {
               ndl[len - 1u] = 500;
               test_detail::for_each_shape2_all<test_detail::counted_int, test_detail::counted_int>
                  (hay, n1, ndl, len, -999, probe());
            }
         }
      }
   }
   return 0;
}
