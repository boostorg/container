//Reports the predicate applications segmented_find_end performs against the
//S * (N - S + 1) bound, for the shapes the test asserts on.
#include <boost/container/experimental/segmented_find_end.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>
#include <cstdio>

using namespace boost::container;

typedef test_detail::counted_int ci_t;

static std::size_t bound_of(std::size_t n1, std::size_t n2)
{  return (n2 == 0u || n2 > n1) ? 0u : n2 * (n1 - n2 + 1u);   }

int main()
{
   std::printf("%4s %4s %8s %8s %8s %8s\n", "N", "S", "bound", "bwd", "fwd", "fwdseg");
   for(std::size_t n1 = 1u; n1 <= 16u; n1 += 3u) {
      for(std::size_t n2 = 1u; n2 <= 5u; ++n2) {
         boost::container::vector<ci_t> hay(n1, ci_t(1));
         boost::container::vector<ci_t> ndl(n2, ci_t(1));
         ndl[n2 - 1u] = ci_t(2);

         test_detail::counted_int_ops().reset();
         segmented_find_end(hay.begin(), hay.end(), ndl.begin(), ndl.end());
         const std::size_t bwd = test_detail::counted_int_ops().cmp;

         test_detail::counted_int_ops().reset();
         segmented_find_end(hay.begin(), test_detail::make_sentinel(hay.end()),
                            ndl.begin(), ndl.end());
         const std::size_t fwd = test_detail::counted_int_ops().cmp;

         boost::container::vector<int> ints(n1, 1);
         test_detail::seg_vector<ci_t, std::forward_iterator_tag> seg;
         test_detail::make_range(seg, "m", &ints[0], n1, -999);
         boost::container::vector<ci_t> ndl2(ndl);

         test_detail::counted_int_ops().reset();
         segmented_find_end(seg.begin(), test_detail::iter_at(seg, n1),
                            ndl2.begin(), ndl2.end());
         const std::size_t fseg = test_detail::counted_int_ops().cmp;

         const std::size_t b = bound_of(n1, n2);
         std::printf("%4u %4u %8u %8u %8u %8u%s\n",
            unsigned(n1), unsigned(n2), unsigned(b),
            unsigned(bwd), unsigned(fwd), unsigned(fseg),
            (bwd > b || fwd > b || fseg > b) ? "   OVER" : "");
      }
   }
   return 0;
}
