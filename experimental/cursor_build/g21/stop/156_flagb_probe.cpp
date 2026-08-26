// Three shapes of the leaf/walker contract, compared by generated code:
//
//   base  : leaf returns segduo; walker re-checks first1 == last1 || loc2 != end2
//   flagA : leaf returns (pos, pos, stop) with stop = mism || src_shorter,
//           where src_shorter is the side of the min() that was taken
//   flagB : same, but stop = mism || (first1 == last1), which keeps the min
//           branchless because it never needs the comparison as a value
//
// flagA makes GCC materialise the min comparison, which it does by cloning the
// leaf prologue; flagB asks the leaf for one compare it can fold into the exit
// it already has.

#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/iterator.hpp>

#include <vector>
#include <utility>
#include <cstdio>
#include <cstddef>

#include "../../../../bench/bench_utils.hpp"

namespace bc = boost::container;
using bc::segduo;
using bc::segmented_iterator_traits;

struct eqp
{
   template <class T, class U>
   BOOST_CONTAINER_FORCEINLINE bool operator()(const T &a, const U &b) const { return a == b; }
};

template <class A, class B>
struct trio
{
   A first;
   B second;
   bool stop;
   BOOST_CONTAINER_FORCEINLINE trio(A a, B b, bool s) : first(a), second(b), stop(s) {}
};

//////////////////////////////////////////////////////////////////////////////
// base
//////////////////////////////////////////////////////////////////////////////

namespace base {

template <class I1, class I2, class P>
BOOST_CONTAINER_FORCEINLINE segduo<I1, I2> leaf(I1 f1, I1 l1, I2 f2, I2 l2, P pred)
{
   typedef typename bc::iterator_traits<I1>::difference_type diff_t;
   const diff_t src_n = l1 - f1, i2_n = diff_t(l2 - f2);
   diff_t n = src_n < i2_n ? src_n : i2_n;
   while(n) {
      --n;
      if(!pred(*f1, *f2))
         break;
      ++f1;
      ++f2;
   }
   return segduo<I1, I2>(f1, f2);
}

template <class I1, class S2, class P>
segduo<I1, S2> walker_a(I1 f1, I1 l1, S2 i2_first, S2 l2, P pred)
{
   typedef segmented_iterator_traits<S2>  t2;
   typedef typename t2::local_iterator    loc_t;
   typedef typename t2::segment_iterator  seg_t;

   seg_t       sfirst = t2::segment(i2_first);
   const seg_t slast  = t2::segment(l2);
   loc_t       lb2    = t2::local(i2_first);

   if(sfirst != slast) {
      {
         loc_t end2 = t2::end(sfirst);
         const segduo<I1, loc_t> r = base::leaf(f1, l1, lb2, end2, pred);
         f1 = r.first;
         const loc_t loc2 = r.second;
         if(f1 == l1 || loc2 != end2)
            return segduo<I1, S2>(f1, t2::compose(sfirst, loc2));
      }
      for(++sfirst; sfirst != slast; ++sfirst) {
         loc_t end2 = t2::end(sfirst);
         const segduo<I1, loc_t> r = base::leaf(f1, l1, t2::begin(sfirst), end2, pred);
         f1 = r.first;
         const loc_t loc2 = r.second;
         if(f1 == l1 || loc2 != end2)
            return segduo<I1, S2>(f1, t2::compose(sfirst, loc2));
      }
      lb2 = t2::begin(slast);
   }
   const segduo<I1, loc_t> r = base::leaf(f1, l1, lb2, t2::local(l2), pred);
   return segduo<I1, S2>(r.first, t2::compose(sfirst, r.second));
}

template <class I1, class S2, class P>
segduo<I1, S2> walker_b(I1 f1, I1 l1, S2 f2, P pred)
{
   typedef segmented_iterator_traits<S2>  t2;
   typedef typename t2::local_iterator    loc_t;
   typedef typename t2::segment_iterator  seg_t;

   if(f1 == l1)
      return segduo<I1, S2>(f1, f2);

   seg_t seg2 = t2::segment(f2);
   loc_t loc2 = t2::local(f2);

   while(f1 != l1) {
      loc_t end2 = t2::end(seg2);
      segduo<I1, loc_t> r = base::leaf(f1, l1, loc2, end2, pred);
      f1 = r.first;
      loc2 = r.second;
      if(f1 != l1 && loc2 != end2)
         return segduo<I1, S2>(f1, t2::compose(seg2, loc2));
      if(f1 != l1) {
         ++seg2;
         loc2 = t2::begin(seg2);
      }
   }
   return segduo<I1, S2>(f1, t2::compose(seg2, loc2));
}

}  //namespace base

//////////////////////////////////////////////////////////////////////////////
// flagA: stop = mism || src_shorter
//////////////////////////////////////////////////////////////////////////////

namespace flaga {

template <class I1, class I2, class P>
BOOST_CONTAINER_FORCEINLINE trio<I1, I2> leaf(I1 f1, I1 l1, I2 f2, I2 l2, P pred)
{
   typedef typename bc::iterator_traits<I1>::difference_type diff_t;
   const diff_t src_n = l1 - f1, i2_n = diff_t(l2 - f2);
   const bool src_shorter = src_n <= i2_n;
   diff_t n = src_shorter ? src_n : i2_n;
   bool mism = false;
   while(n) {
      --n;
      if(!pred(*f1, *f2)) {
         mism = true;
         break;
      }
      ++f1;
      ++f2;
   }
   return trio<I1, I2>(f1, f2, mism || src_shorter);
}

#define WALKERS(NS)                                                          \
template <class I1, class S2, class P>                                       \
segduo<I1, S2> walker_a(I1 f1, I1 l1, S2 i2_first, S2 l2, P pred)            \
{                                                                            \
   typedef segmented_iterator_traits<S2>  t2;                                \
   typedef typename t2::local_iterator    loc_t;                             \
   typedef typename t2::segment_iterator  seg_t;                             \
   seg_t       sfirst = t2::segment(i2_first);                               \
   const seg_t slast  = t2::segment(l2);                                     \
   loc_t       lb2    = t2::local(i2_first);                                 \
   if(sfirst != slast) {                                                     \
      {                                                                      \
         const trio<I1, loc_t> r = NS::leaf(f1, l1, lb2, t2::end(sfirst), pred); \
         f1 = r.first;                                                       \
         if(r.stop)                                                          \
            return segduo<I1, S2>(f1, t2::compose(sfirst, r.second));        \
      }                                                                      \
      for(++sfirst; sfirst != slast; ++sfirst) {                             \
         const trio<I1, loc_t> r =                                           \
            NS::leaf(f1, l1, t2::begin(sfirst), t2::end(sfirst), pred);      \
         f1 = r.first;                                                       \
         if(r.stop)                                                          \
            return segduo<I1, S2>(f1, t2::compose(sfirst, r.second));        \
      }                                                                      \
      lb2 = t2::begin(slast);                                                \
   }                                                                         \
   const trio<I1, loc_t> r = NS::leaf(f1, l1, lb2, t2::local(l2), pred);     \
   return segduo<I1, S2>(r.first, t2::compose(sfirst, r.second));            \
}                                                                            \
                                                                             \
template <class I1, class S2, class P>                                       \
segduo<I1, S2> walker_b(I1 f1, I1 l1, S2 f2, P pred)                         \
{                                                                            \
   typedef segmented_iterator_traits<S2>  t2;                                \
   typedef typename t2::local_iterator    loc_t;                             \
   typedef typename t2::segment_iterator  seg_t;                             \
   if(f1 == l1)                                                              \
      return segduo<I1, S2>(f1, f2);                                         \
   seg_t seg2 = t2::segment(f2);                                             \
   loc_t loc2 = t2::local(f2);                                               \
   for(;;) {                                                                 \
      const trio<I1, loc_t> r = NS::leaf(f1, l1, loc2, t2::end(seg2), pred); \
      f1 = r.first;                                                          \
      loc2 = r.second;                                                       \
      if(r.stop)                                                             \
         break;                                                              \
      ++seg2;                                                                \
      loc2 = t2::begin(seg2);                                                \
   }                                                                         \
   return segduo<I1, S2>(f1, t2::compose(seg2, loc2));                       \
}

WALKERS(flaga)

}  //namespace flaga

//////////////////////////////////////////////////////////////////////////////
// flagB: stop = mism || (f1 == l1), min stays branchless
//////////////////////////////////////////////////////////////////////////////

namespace flagb {

template <class I1, class I2, class P>
BOOST_CONTAINER_FORCEINLINE trio<I1, I2> leaf(I1 f1, I1 l1, I2 f2, I2 l2, P pred)
{
   typedef typename bc::iterator_traits<I1>::difference_type diff_t;
   const diff_t src_n = l1 - f1, i2_n = diff_t(l2 - f2);
   diff_t n = src_n < i2_n ? src_n : i2_n;
   while(n) {
      --n;
      if(!pred(*f1, *f2))
         return trio<I1, I2>(f1, f2, true);
      ++f1;
      ++f2;
   }
   return trio<I1, I2>(f1, f2, f1 == l1);
}

WALKERS(flagb)

}  //namespace flagb

//////////////////////////////////////////////////////////////////////////////
// Symbols
//////////////////////////////////////////////////////////////////////////////

typedef bc::deque_options< bc::block_size<128> >::type opt_t;

typedef std::vector<int>::const_iterator                        vit;
typedef bc::deque<int, void, opt_t>::const_iterator              dit;
typedef std::vector<MyFatInt<> >::const_iterator                 vfit;
typedef bc::deque<MyFatInt<>, void, opt_t>::const_iterator       dfit;

#define MKA(name, ns, VI, DI)                                        \
   BOOST_NOINLINE std::pair<VI, DI> name(VI f1, VI l1, DI f2, DI l2)  \
   {  segduo<VI, DI> r = ns::walker_a(f1, l1, f2, l2, eqp());          \
      return std::pair<VI, DI>(r.first, r.second);  }

#define MKB(name, ns, VI, DI)                                        \
   BOOST_NOINLINE std::pair<VI, DI> name(VI f1, VI l1, DI f2)         \
   {  segduo<VI, DI> r = ns::walker_b(f1, l1, f2, eqp());             \
      return std::pair<VI, DI>(r.first, r.second);  }

MKA(a_base_int,  base,  vit, dit)
MKA(a_flaga_int, flaga, vit, dit)
MKA(a_flagb_int, flagb, vit, dit)
MKB(b_base_int,  base,  vit, dit)
MKB(b_flaga_int, flaga, vit, dit)
MKB(b_flagb_int, flagb, vit, dit)

MKA(a_base_fat,  base,  vfit, dfit)
MKA(a_flaga_fat, flaga, vfit, dfit)
MKA(a_flagb_fat, flagb, vfit, dfit)
MKB(b_base_fat,  base,  vfit, dfit)
MKB(b_flaga_fat, flaga, vfit, dfit)
MKB(b_flagb_fat, flagb, vfit, dfit)

//////////////////////////////////////////////////////////////////////////////
// Correctness: all three must agree with a plain lock-step walk.
//////////////////////////////////////////////////////////////////////////////

static int failures = 0;

static void check(std::size_t n, long mp)
{
   std::vector<int>             v;
   bc::deque<int, void, opt_t>  d;
   for(std::size_t i = 0; i != n; ++i) {
      v.push_back(int(i));
      d.push_back(int(i));
   }
   if(mp >= 0 && std::size_t(mp) < n)
      d[std::size_t(mp)] = -1;

   std::size_t k = 0;
   while(k != n && v[k] == d[k])
      ++k;

   const std::pair<vit, dit> r[6] = {
      a_base_int (v.begin(), v.end(), d.begin(), d.end()),
      a_flaga_int(v.begin(), v.end(), d.begin(), d.end()),
      a_flagb_int(v.begin(), v.end(), d.begin(), d.end()),
      b_base_int (v.begin(), v.end(), d.begin()),
      b_flaga_int(v.begin(), v.end(), d.begin()),
      b_flagb_int(v.begin(), v.end(), d.begin())
   };
   for(int i = 0; i != 6; ++i) {
      if(std::size_t(r[i].first - v.begin()) != k) {
         std::printf("FAIL n=%zu mp=%ld variant=%d want=%zu got=%zu\n",
                     n, mp, i, k, std::size_t(r[i].first - v.begin()));
         ++failures;
      }
   }
   //iter2 must agree across variants of the same walker
   if(!(r[0].second == r[1].second) || !(r[0].second == r[2].second) ||
      !(r[3].second == r[4].second) || !(r[3].second == r[5].second)) {
      std::printf("FAIL n=%zu mp=%ld : iter2 disagrees\n", n, mp);
      ++failures;
   }
}

int main()
{
   static const std::size_t ns[] =
      { 0, 1, 2, 127, 128, 129, 255, 256, 257, 383, 384, 385, 1000 };
   for(std::size_t i = 0; i != sizeof(ns)/sizeof(ns[0]); ++i) {
      const std::size_t n = ns[i];
      check(n, -1);
      if(n) {
         check(n, 0);
         check(n, long(n/2));
         check(n, long(n-1));
         if(n > 128) { check(n, 127); check(n, 128); check(n, 129); }
      }
   }
   std::printf("correctness: %s (%d failures)\n", failures ? "FAIL" : "OK", failures);
   return failures != 0;
}
