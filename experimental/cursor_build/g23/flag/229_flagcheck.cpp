// Compiles only against the flagged (shadow) set_* headers: reads .fourth off
// every leaf's return value.  Used to prove that an MSVC run with the shadow
// include directory really picked the shadow up and not the real header.
#include <iterator>
#include <utility>
#include <cstddef>

#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_set_union.hpp>
#include <boost/container/experimental/segmented_set_difference.hpp>
#include <boost/container/experimental/segmented_set_intersection.hpp>
#include <boost/container/experimental/segmented_set_symmetric_difference.hpp>

namespace bc = boost::container;
namespace da = boost::container::detail_algo;

struct less_t { bool operator()(int a, int b) const { return a < b; } };

int main()
{
   const int a[3] = { 1, 2, 3 };
   const int b[3] = { 2, 3, 4 };
   int out[8];
   int n = 0;
   n += (da::set_union_dst_bounded)
      (a, a + 3, b, b + 3, out, out + 8, less_t(),
       bc::non_segmented_iterator_tag(), int()).fourth ? 1 : 0;
   n += (da::set_difference_dst_bounded)
      (a, a + 3, b, b + 3, out, out + 8, less_t(),
       bc::non_segmented_iterator_tag(), int()).fourth ? 2 : 0;
   n += (da::set_intersection_dst_bounded)
      (a, a + 3, b, b + 3, out, out + 8, less_t(),
       bc::non_segmented_iterator_tag(), int()).fourth ? 4 : 0;
   n += (da::set_symmetric_difference_dst_bounded)
      (a, a + 3, b, b + 3, out, out + 8, less_t(),
       bc::non_segmented_iterator_tag(), int()).fourth ? 8 : 0;
   return n == 15 ? 0 : 1;
}
