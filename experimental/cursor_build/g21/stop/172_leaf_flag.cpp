// Calls the random-access leaf directly and checks its stop flag against the
// documented contract: third must be true whenever the source was consumed,
// because that is a reason the caller cannot resume from.

#include <boost/container/experimental/segmented_mismatch.hpp>
#include <boost/container/experimental/segmented_equal.hpp>
#include <cstdio>
#include <cstddef>

namespace bc = boost::container;

static int failures = 0;

static void probe(int src_n, int iter2_n)
{
   int a[64], b[64];
   for(int i = 0; i != 64; ++i) {
      a[i] = i;
      b[i] = i;                 //all equal: no mismatch, so only lengths decide
   }

   const bc::segtrio<int*, int*, bool> rm = (bc::detail_algo::segmented_mismatch_iter2_bounded)
      (a, a + src_n, b, b + iter2_n, bc::detail_algo::mismatch_equal()
      , bc::non_segmented_iterator_tag(), std::random_access_iterator_tag());

   const bc::segtrio<int*, int*, bool> re = (bc::detail_algo::segmented_equal_iter2_bounded)
      (a, a + src_n, b, b + iter2_n, bc::detail_algo::equal_pred()
      , bc::non_segmented_iterator_tag(), std::random_access_iterator_tag());

   const bool src_consumed = (rm.first == a + src_n);
   const bool want         = src_consumed;      //no mismatch in this data

   const char *tag = src_n == iter2_n ? "  <-- tie" : "";
   if(rm.third != want || re.third != want) {
      std::printf("FAIL src_n=%-3d iter2_n=%-3d src_consumed=%d  mismatch.third=%d "
                  "equal.third=%d want=%d%s\n",
                  src_n, iter2_n, int(src_consumed), int(rm.third), int(re.third),
                  int(want), tag);
      ++failures;
   }
   else {
      std::printf("ok   src_n=%-3d iter2_n=%-3d third=%d%s\n",
                  src_n, iter2_n, int(rm.third), tag);
   }
}

int main()
{
   probe(8, 16);      //source shorter  -> consumed  -> true
   probe(16, 8);      //iter2 shorter   -> not       -> false
   probe(8, 8);       //tie             -> consumed  -> true
   probe(1, 1);       //tie, length 1
   probe(32, 32);     //tie, longer
   probe(0, 8);       //empty source    -> consumed  -> true
   probe(0, 0);       //both empty      -> consumed  -> true
   probe(8, 0);       //iter2 empty     -> not       -> false

   std::printf("\n%s (%d failures)\n", failures ? "CONTRACT VIOLATED" : "all OK", failures);
   return failures != 0;
}
