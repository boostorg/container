// Does returning an explicit "stop" flag from the non-segmented leaf remove the
// per-segment re-check in the segmented walkers of segmented_mismatch?
//
// Two walkers are reproduced verbatim from segmented_mismatch.hpp and then
// rewritten to consume a flag:
//
//   A = segmented_mismatch_iter2_bounded, segmented iter2 (both ends bounded)
//       re-check: first1 == last1 || loc2 != end2
//   B = segmented_mismatch_iter2_dispatch, segmented iter2 (iter2 unbounded)
//       re-check: (first1 != last1 && loc2 != end2), then first1 != last1,
//                 plus the while(first1 != last1) loop test
//
// The leaf knows which of the three exits it took, so it can hand back one bool
// that means "stop the segment walk". For both walkers that flag is
// mismatch || source-exhausted.

#include <boost/container/deque.hpp>
#include <boost/container/experimental/segmented_mismatch.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/iterator.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <cstdio>
#include <cstddef>
#include <ctime>

#include "../../../../bench/bench_utils.hpp"

namespace bc = boost::container;

using bc::segduo;
using bc::segmented_iterator_traits;

struct eqp
{
   template <class T, class U>
   BOOST_CONTAINER_FORCEINLINE bool operator()(const T &a, const U &b) const { return a == b; }
};

//////////////////////////////////////////////////////////////////////////////
// Baseline: exactly what the header does today.
//////////////////////////////////////////////////////////////////////////////

namespace base {

template <class RASrcIter, class RAIter2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE
segduo<RASrcIter, RAIter2> leaf
   (RASrcIter first1, RASrcIter last1, RAIter2 first2, RAIter2 last2, BinaryPred pred)
{
   typedef typename bc::iterator_traits<RASrcIter>::difference_type difference_type;
   const difference_type src_n   = last1 - first1;
   const difference_type iter2_n = difference_type(last2 - first2);
   difference_type n = src_n < iter2_n ? src_n : iter2_n;

   while(n) {
      --n;
      if(!pred(*first1, *first2))
         break;
      ++first1;
      ++first2;
   }
   return segduo<RASrcIter, RAIter2>(first1, first2);
}

//Walker A: both ends of iter2 bounded.
template <class SrcIter, class SegIter2, class BinaryPred>
segduo<SrcIter, SegIter2> walker_a
   (SrcIter first1, SrcIter last1, SegIter2 iter2_first, SegIter2 last2, BinaryPred pred)
{
   typedef segmented_iterator_traits<SegIter2>       t2;
   typedef typename t2::local_iterator               loc_t;
   typedef typename t2::segment_iterator             seg_t;

   seg_t       sfirst = t2::segment(iter2_first);
   const seg_t slast  = t2::segment(last2);

   loc_t lb2 = t2::local(iter2_first);

   if(sfirst != slast) {
      {
         loc_t end2 = t2::end(sfirst);
         const segduo<SrcIter, loc_t> r = base::leaf(first1, last1, lb2, end2, pred);
         first1 = r.first;
         const loc_t loc2 = r.second;
         if(first1 == last1 || loc2 != end2)
            return segduo<SrcIter, SegIter2>(first1, t2::compose(sfirst, loc2));
      }
      for(++sfirst; sfirst != slast; ++sfirst) {
         loc_t end2 = t2::end(sfirst);
         const segduo<SrcIter, loc_t> r = base::leaf(first1, last1, t2::begin(sfirst), end2, pred);
         first1 = r.first;
         const loc_t loc2 = r.second;
         if(first1 == last1 || loc2 != end2)
            return segduo<SrcIter, SegIter2>(first1, t2::compose(sfirst, loc2));
      }
      lb2 = t2::begin(slast);
   }
   const segduo<SrcIter, loc_t> r = base::leaf(first1, last1, lb2, t2::local(last2), pred);
   return segduo<SrcIter, SegIter2>(r.first, t2::compose(sfirst, r.second));
}

//Walker B: iter2 unbounded, segment walk driven by the source.
template <class SrcIter, class SegIter2, class BinaryPred>
segduo<SrcIter, SegIter2> walker_b
   (SrcIter first1, SrcIter last1, SegIter2 first2, BinaryPred pred)
{
   typedef segmented_iterator_traits<SegIter2>       t2;
   typedef typename t2::local_iterator               loc_t;
   typedef typename t2::segment_iterator             seg_t;

   if(first1 == last1)
      return segduo<SrcIter, SegIter2>(first1, first2);

   seg_t seg2 = t2::segment(first2);
   loc_t loc2 = t2::local(first2);

   while(first1 != last1) {
      loc_t end2 = t2::end(seg2);
      segduo<SrcIter, loc_t> r = base::leaf(first1, last1, loc2, end2, pred);
      first1 = r.first;
      loc2   = r.second;
      if(first1 != last1 && loc2 != end2)
         return segduo<SrcIter, SegIter2>(first1, t2::compose(seg2, loc2));
      if(first1 != last1) {
         ++seg2;
         loc2 = t2::begin(seg2);
      }
   }
   return segduo<SrcIter, SegIter2>(first1, t2::compose(seg2, loc2));
}

}  //namespace base

//////////////////////////////////////////////////////////////////////////////
// Variant: leaf hands back the reason.
//////////////////////////////////////////////////////////////////////////////

namespace varn {

template <class A, class B>
struct trio
{
   A first;
   B second;
   bool stop;
   BOOST_CONTAINER_FORCEINLINE trio(A a, B b, bool s) : first(a), second(b), stop(s) {}
};

template <class RASrcIter, class RAIter2, class BinaryPred>
BOOST_CONTAINER_FORCEINLINE
trio<RASrcIter, RAIter2> leaf
   (RASrcIter first1, RASrcIter last1, RAIter2 first2, RAIter2 last2, BinaryPred pred)
{
   typedef typename bc::iterator_traits<RASrcIter>::difference_type difference_type;
   const difference_type src_n   = last1 - first1;
   const difference_type iter2_n = difference_type(last2 - first2);
   //src_shorter also covers the tie: when both run out the source is exhausted,
   //which is a stop for every caller.
   const bool src_shorter = src_n <= iter2_n;
   difference_type n = src_shorter ? src_n : iter2_n;
   bool mism = false;

   while(n) {
      --n;
      if(!pred(*first1, *first2)) {
         mism = true;
         break;
      }
      ++first1;
      ++first2;
   }
   return trio<RASrcIter, RAIter2>(first1, first2, mism || src_shorter);
}

template <class SrcIter, class SegIter2, class BinaryPred>
segduo<SrcIter, SegIter2> walker_a
   (SrcIter first1, SrcIter last1, SegIter2 iter2_first, SegIter2 last2, BinaryPred pred)
{
   typedef segmented_iterator_traits<SegIter2>       t2;
   typedef typename t2::local_iterator               loc_t;
   typedef typename t2::segment_iterator             seg_t;

   seg_t       sfirst = t2::segment(iter2_first);
   const seg_t slast  = t2::segment(last2);

   loc_t lb2 = t2::local(iter2_first);

   if(sfirst != slast) {
      {
         const trio<SrcIter, loc_t> r = varn::leaf(first1, last1, lb2, t2::end(sfirst), pred);
         first1 = r.first;
         if(r.stop)
            return segduo<SrcIter, SegIter2>(first1, t2::compose(sfirst, r.second));
      }
      for(++sfirst; sfirst != slast; ++sfirst) {
         const trio<SrcIter, loc_t> r =
            varn::leaf(first1, last1, t2::begin(sfirst), t2::end(sfirst), pred);
         first1 = r.first;
         if(r.stop)
            return segduo<SrcIter, SegIter2>(first1, t2::compose(sfirst, r.second));
      }
      lb2 = t2::begin(slast);
   }
   const trio<SrcIter, loc_t> r = varn::leaf(first1, last1, lb2, t2::local(last2), pred);
   return segduo<SrcIter, SegIter2>(r.first, t2::compose(sfirst, r.second));
}

template <class SrcIter, class SegIter2, class BinaryPred>
segduo<SrcIter, SegIter2> walker_b
   (SrcIter first1, SrcIter last1, SegIter2 first2, BinaryPred pred)
{
   typedef segmented_iterator_traits<SegIter2>       t2;
   typedef typename t2::local_iterator               loc_t;
   typedef typename t2::segment_iterator             seg_t;

   if(first1 == last1)
      return segduo<SrcIter, SegIter2>(first1, first2);

   seg_t seg2 = t2::segment(first2);
   loc_t loc2 = t2::local(first2);

   //The flag folds the two re-checks and the loop test into one test.
   for(;;) {
      const trio<SrcIter, loc_t> r = varn::leaf(first1, last1, loc2, t2::end(seg2), pred);
      first1 = r.first;
      loc2   = r.second;
      if(r.stop)
         break;
      ++seg2;
      loc2 = t2::begin(seg2);
   }
   return segduo<SrcIter, SegIter2>(first1, t2::compose(seg2, loc2));
}

}  //namespace varn

//////////////////////////////////////////////////////////////////////////////
// Out-of-line instantiations, so each walker gets its own measurable symbol.
//////////////////////////////////////////////////////////////////////////////

typedef std::vector<int>::const_iterator          vit;
typedef bc::deque<int>::const_iterator            dit;
typedef std::vector<MyFatInt<> >::const_iterator  vfit;
typedef bc::deque<MyFatInt<> >::const_iterator    dfit;

#define MK(name, ns, walker, VI, DI)                                    \
   BOOST_NOINLINE std::pair<VI, DI> name(VI f1, VI l1, DI f2, DI l2)     \
   {                                                                    \
      segduo<VI, DI> r = ns::walker(f1, l1, f2, l2, eqp());              \
      return std::pair<VI, DI>(r.first, r.second);                       \
   }

#define MKB(name, ns, VI, DI)                                           \
   BOOST_NOINLINE std::pair<VI, DI> name(VI f1, VI l1, DI f2)            \
   {                                                                    \
      segduo<VI, DI> r = ns::walker_b(f1, l1, f2, eqp());                \
      return std::pair<VI, DI>(r.first, r.second);                       \
   }

MK(base_a_int, base, walker_a, vit, dit)
MK(var_a_int,  varn, walker_a, vit, dit)
MKB(base_b_int, base, vit, dit)
MKB(var_b_int,  varn, vit, dit)

MK(base_a_fat, base, walker_a, vfit, dfit)
MK(var_a_fat,  varn, walker_a, vfit, dfit)
MKB(base_b_fat, base, vfit, dfit)
MKB(var_b_fat,  varn, vfit, dfit)

//////////////////////////////////////////////////////////////////////////////
// Correctness
//////////////////////////////////////////////////////////////////////////////

static int failures = 0;

static void check_shape(std::size_t n, long mism_pos)
{
   std::vector<int>  v;
   bc::deque<int>    d;
   for(std::size_t i = 0; i != n; ++i) {
      v.push_back(int(i));
      d.push_back(int(i));
   }
   if(mism_pos >= 0 && std::size_t(mism_pos) < n)
      d[std::size_t(mism_pos)] = -1;

   //Reference: plain lock-step walk over the common prefix.
   std::size_t k = 0;
   while(k != n && v[k] == d[k])
      ++k;

   const std::pair<vit, dit> ra = base_a_int(v.begin(), v.end(), d.begin(), d.end());
   const std::pair<vit, dit> va = var_a_int (v.begin(), v.end(), d.begin(), d.end());
   const std::pair<vit, dit> rb = base_b_int(v.begin(), v.end(), d.begin());
   const std::pair<vit, dit> vb = var_b_int (v.begin(), v.end(), d.begin());

   const std::size_t ka = std::size_t(ra.first - v.begin());
   const std::size_t kva = std::size_t(va.first - v.begin());
   const std::size_t kb = std::size_t(rb.first - v.begin());
   const std::size_t kvb = std::size_t(vb.first - v.begin());

   if(ka != k || kva != k || kb != k || kvb != k) {
      std::printf("FAIL n=%zu mism=%ld : want %zu got a=%zu/%zu b=%zu/%zu\n",
                  n, mism_pos, k, ka, kva, kb, kvb);
      ++failures;
      return;
   }
   //Second iterator must agree between baseline and variant.
   if(!(ra.second == va.second) || !(rb.second == vb.second)) {
      std::printf("FAIL n=%zu mism=%ld : iter2 mismatch between base and var\n", n, mism_pos);
      ++failures;
   }
}

static void correctness()
{
   static const std::size_t ns[] =
      { 0, 1, 2, 3, 127, 128, 129, 255, 256, 257, 383, 384, 385, 1000 };
   for(std::size_t i = 0; i != sizeof(ns)/sizeof(ns[0]); ++i) {
      const std::size_t n = ns[i];
      check_shape(n, -1);
      if(n) {
         check_shape(n, 0);
         check_shape(n, long(n/2));
         check_shape(n, long(n-1));
         if(n > 128) {
            check_shape(n, 127);
            check_shape(n, 128);
            check_shape(n, 129);
         }
      }
   }
   std::printf("correctness: %s (%d failures)\n", failures ? "FAIL" : "OK", failures);
}

//////////////////////////////////////////////////////////////////////////////
// Timing
//////////////////////////////////////////////////////////////////////////////

static double now_ns()
{
   return double(std::clock()) / double(CLOCKS_PER_SEC) * 1e9;
}

template <class T>
static void time_one(const char *tname, std::size_t n, std::size_t iters)
{
   std::vector<T> v;
   bc::deque<T>   d;
   for(std::size_t i = 0; i != n; ++i) {
      v.push_back(T(int(i)));
      d.push_back(T(int(i)));
   }

   typedef typename std::vector<T>::const_iterator VI;
   typedef typename bc::deque<T>::const_iterator   DI;

   double best_ba = 1e30, best_va = 1e30, best_bb = 1e30, best_vb = 1e30;
   std::size_t sink = 0;

   for(int rep = 0; rep != 5; ++rep) {
      double t0 = now_ns();
      for(std::size_t it = 0; it != iters; ++it) {
         segduo<VI, DI> r = base::walker_a(v.begin(), v.end(), d.begin(), d.end(), eqp());
         sink += std::size_t(r.first - v.begin());
      }
      double t1 = now_ns();
      for(std::size_t it = 0; it != iters; ++it) {
         segduo<VI, DI> r = varn::walker_a(v.begin(), v.end(), d.begin(), d.end(), eqp());
         sink += std::size_t(r.first - v.begin());
      }
      double t2 = now_ns();
      for(std::size_t it = 0; it != iters; ++it) {
         segduo<VI, DI> r = base::walker_b(v.begin(), v.end(), d.begin(), eqp());
         sink += std::size_t(r.first - v.begin());
      }
      double t3 = now_ns();
      for(std::size_t it = 0; it != iters; ++it) {
         segduo<VI, DI> r = varn::walker_b(v.begin(), v.end(), d.begin(), eqp());
         sink += std::size_t(r.first - v.begin());
      }
      double t4 = now_ns();

      const double per = double(n) * double(iters);
      if((t1-t0)/per < best_ba) best_ba = (t1-t0)/per;
      if((t2-t1)/per < best_va) best_va = (t2-t1)/per;
      if((t3-t2)/per < best_bb) best_bb = (t3-t2)/per;
      if((t4-t3)/per < best_vb) best_vb = (t4-t3)/per;
   }

   std::printf("%-12s n=%-7zu  A base %.4f  A var %.4f (%+5.1f%%)   B base %.4f  B var %.4f (%+5.1f%%)\n",
               tname, n, best_ba, best_va, 100.0*(best_va-best_ba)/best_ba,
               best_bb, best_vb, 100.0*(best_vb-best_bb)/best_bb);
   if(sink == 12345678u) std::printf(" ");
}

int main()
{
   correctness();
   std::printf("\nns per element, full scan (no mismatch), best of 5\n");
   time_one<int>          ("int",       100000, 200);
   time_one<int>          ("int",         1000, 20000);
   time_one<MyFatInt<> >  ("MyFatInt<>", 100000, 200);
   time_one<MyFatInt<2> > ("MyFatInt<2>",100000, 200);
   return failures != 0;
}
