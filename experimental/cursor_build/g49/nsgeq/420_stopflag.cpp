#include <boost/container/experimental/detail/segmented_common_algo.hpp>
#include <cstddef>

namespace bc = boost::container;

struct eq {
   bool operator()(int a, int b) const { return a == b; }
};

// Current working-tree shape
__attribute__((noinline))
bc::segtrio<const int*, const int*, bool>
leaf_cur(const int* f1, const int* l1, const int* f2, const int* l2)
{
   typedef bc::segtrio<const int*, const int*, bool> result_t;
   typedef std::ptrdiff_t difference_type;
   const difference_type src_n  = l1 - f1;
   const difference_type iter2_n = difference_type(l2 - f2);
   difference_type n = src_n < iter2_n ? src_n : iter2_n;

   bool stop;
   while(n) {
      --n;
      if(!(*f1 == *f2)) {
         stop = true;
         goto out_path;
      }
      ++f1; ++f2;
   }
   stop = f1 == l1;
out_path:
   return result_t(f1, f2, stop);
}

// HEAD shape
__attribute__((noinline))
bc::segtrio<const int*, const int*, bool>
leaf_head(const int* f1, const int* l1, const int* f2, const int* l2)
{
   typedef bc::segtrio<const int*, const int*, bool> result_t;
   typedef std::ptrdiff_t difference_type;
   const difference_type src_n  = l1 - f1;
   const difference_type iter2_n = difference_type(l2 - f2);
   difference_type n = src_n < iter2_n ? src_n : iter2_n;

   bool stop = true;
   while(n) {
      --n;
      if(!(*f1 == *f2))
         goto out_path;
      ++f1; ++f2;
   }
   stop = f1 == l1;
out_path:
   return result_t(f1, f2, stop);
}

int main() { return 0; }
