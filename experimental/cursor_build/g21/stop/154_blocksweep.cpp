// The stop flag removes compares from the *segment transition*, which is
// amortised over a whole segment. This sweeps the deque block size to find the
// segment length at which that saving becomes visible at all.

#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/iterator.hpp>

#include <vector>
#include <chrono>
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

template <class SrcIter, class SegIter2, class BinaryPred>
segduo<SrcIter, SegIter2> walker_b
   (SrcIter first1, SrcIter last1, SegIter2 first2, BinaryPred pred)
{
   typedef segmented_iterator_traits<SegIter2>  t2;
   typedef typename t2::local_iterator          loc_t;
   typedef typename t2::segment_iterator        seg_t;

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
segduo<SrcIter, SegIter2> walker_b
   (SrcIter first1, SrcIter last1, SegIter2 first2, BinaryPred pred)
{
   typedef segmented_iterator_traits<SegIter2>  t2;
   typedef typename t2::local_iterator          loc_t;
   typedef typename t2::segment_iterator        seg_t;

   if(first1 == last1)
      return segduo<SrcIter, SegIter2>(first1, first2);

   seg_t seg2 = t2::segment(first2);
   loc_t loc2 = t2::local(first2);

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

static double sec()
{
   return std::chrono::duration<double>
      (std::chrono::steady_clock::now().time_since_epoch()).count();
}

template <class T, std::size_t BS>
static void sweep(std::size_t n, std::size_t iters)
{
   typedef typename bc::deque_options< bc::block_size<BS> >::type opt_t;
   typedef bc::deque<T, void, opt_t>                             dq_t;
   typedef typename std::vector<T>::const_iterator                VI;
   typedef typename dq_t::const_iterator                          DI;

   std::vector<T> v;
   dq_t           d;
   for(std::size_t i = 0; i != n; ++i) {
      v.push_back(T(int(i)));
      d.push_back(T(int(i)));
   }

   double bb = 1e30, vb = 1e30;
   std::size_t sink = 0;

   for(int rep = 0; rep != 9; ++rep) {
      const double t0 = sec();
      for(std::size_t it = 0; it != iters; ++it) {
         segduo<VI, DI> r = base::walker_b(v.begin(), v.end(), d.begin(), eqp());
         sink += std::size_t(r.first - v.begin());
      }
      const double t1 = sec();
      for(std::size_t it = 0; it != iters; ++it) {
         segduo<VI, DI> r = varn::walker_b(v.begin(), v.end(), d.begin(), eqp());
         sink += std::size_t(r.first - v.begin());
      }
      const double t2 = sec();

      const double per = double(n) * double(iters) * 1e-9;
      if((t1 - t0) / per < bb) bb = (t1 - t0) / per;
      if((t2 - t1) / per < vb) vb = (t2 - t1) / per;
   }

   std::printf("  block=%-5zu base %.4f  flag %.4f   %+6.1f%%\n",
               BS, bb, vb, 100.0 * (vb - bb) / bb);
   if(sink == 123456789u) std::printf(" ");
}

template <class T>
static void all(const char *tname, std::size_t n, std::size_t iters)
{
   std::printf("%s  (n=%zu, ns/element, best of 9)\n", tname, n);
   sweep<T, 4>   (n, iters);
   sweep<T, 8>   (n, iters);
   sweep<T, 16>  (n, iters);
   sweep<T, 32>  (n, iters);
   sweep<T, 64>  (n, iters);
   sweep<T, 128> (n, iters);
   sweep<T, 256> (n, iters);
   sweep<T, 512> (n, iters);
   std::printf("\n");
}

int main()
{
   all<int>         ("int",        100000, 300);
   all<MyFatInt<> > ("MyFatInt<>", 100000, 200);
   return 0;
}
