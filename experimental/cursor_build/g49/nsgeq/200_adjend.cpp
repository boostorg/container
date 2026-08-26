// A/B: current counted RA leaf (while(n){--n;...}) vs adjusted-end leaf
// (bound range1 by first1 + min_len, compare iterators) for the group-20
// *_2r nsg rows.  Reference: std 4-arg forms.

#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_mismatch.hpp>
#include <boost/container/experimental/segmented_equal.hpp>
#include <boost/container/experimental/wrapped_iterator.hpp>
#include <bench_utils.hpp>
#include <algorithm>
#include <cstdio>

namespace bc = boost::container;
typedef bc::deque<MyInt>            dq_t;
typedef bc::vector<MyInt>           vec_t;
typedef bc::wrapped_iterator<dq_t::const_iterator>  wd_t;
typedef bc::wrapped_iterator<vec_t::const_iterator> wv_t;

//variant B leaf: min length as adjusted end iterator, no counter
template<class I1, class I2, class Pred>
BOOST_CONTAINER_FORCEINLINE
bc::segtrio<I1, I2, bool> leaf_adjend(I1 first1, I1 last1, I2 first2, I2 last2, Pred pred)
{
   typedef typename bc::iterator_traits<I1>::difference_type difference_type;
   const difference_type src_n   = last1 - first1;
   const difference_type iter2_n = difference_type(last2 - first2);
   const I1 bounded_last1 = iter2_n < src_n ? first1 + iter2_n : last1;

   bool stop;
   while(first1 != bounded_last1) {
      if(!pred(*first1, *first2)) {
         stop = true;
         goto out_path;
      }
      ++first1;
      ++first2;
   }
   stop = first1 == last1;

   out_path:
   return bc::segtrio<I1, I2, bool>(first1, first2, stop);
}

//variant C leaf: countdown in the loop-latch (do-while), single test per lap
template<class I1, class I2, class Pred>
BOOST_CONTAINER_FORCEINLINE
bc::segtrio<I1, I2, bool> leaf_dowhile(I1 first1, I1 last1, I2 first2, I2 last2, Pred pred)
{
   typedef typename bc::iterator_traits<I1>::difference_type difference_type;
   const difference_type src_n   = last1 - first1;
   const difference_type iter2_n = difference_type(last2 - first2);
   difference_type n = src_n < iter2_n ? src_n : iter2_n;

   bool stop;
   if(n) {
      do {
         if(!pred(*first1, *first2)) {
            stop = true;
            goto out_path;
         }
         ++first1;
         ++first2;
      } while(--n);
   }
   stop = first1 == last1;

   out_path:
   return bc::segtrio<I1, I2, bool>(first1, first2, stop);
}

//--- mismatch_2r 1+2S (deque + deque) ---------------------------------------
struct mm_dd_cur {
   const dq_t &a, &b; int &r;
   mm_dd_cur(const dq_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      r = (bc::segmented_mismatch(wd_t(a.begin()), wd_t(a.end()), wd_t(b.begin()), wd_t(b.end())).first == wd_t(a.end())) ? 1 : 0;
      escape(&r);  }
};
struct mm_dd_adj {
   const dq_t &a, &b; int &r;
   mm_dd_adj(const dq_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      r = (leaf_adjend(wd_t(a.begin()), wd_t(a.end()), wd_t(b.begin()), wd_t(b.end()),
                       bc::detail_algo::segmented_default_equal_to()).first == wd_t(a.end())) ? 1 : 0;
      escape(&r);  }
};
struct mm_dd_dw {
   const dq_t &a, &b; int &r;
   mm_dd_dw(const dq_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      r = (leaf_dowhile(wd_t(a.begin()), wd_t(a.end()), wd_t(b.begin()), wd_t(b.end()),
                        bc::detail_algo::segmented_default_equal_to()).first == wd_t(a.end())) ? 1 : 0;
      escape(&r);  }
};
struct mm_vd_cur {
   const vec_t &a; const dq_t &b; int &r;
   mm_vd_cur(const vec_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      r = (bc::segmented_mismatch(wv_t(a.begin()), wv_t(a.end()), wd_t(b.begin()), wd_t(b.end())).first == wv_t(a.end())) ? 1 : 0;
      escape(&r);  }
};
struct mm_vd_dw {
   const vec_t &a; const dq_t &b; int &r;
   mm_vd_dw(const vec_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      r = (leaf_dowhile(wv_t(a.begin()), wv_t(a.end()), wd_t(b.begin()), wd_t(b.end()),
                        bc::detail_algo::segmented_default_equal_to()).first == wv_t(a.end())) ? 1 : 0;
      escape(&r);  }
};
struct mm_vd_std {
   const vec_t &a; const dq_t &b; int &r;
   mm_vd_std(const vec_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      r = (std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first == a.end()) ? 1 : 0;
      escape(&r);  }
};
struct mm_dd_std {
   const dq_t &a, &b; int &r;
   mm_dd_std(const dq_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      r = (std::mismatch(a.begin(), a.end(), b.begin(), b.end()).first == a.end()) ? 1 : 0;
      escape(&r);  }
};

//--- equal_2r 2S (vector + deque) --------------------------------------------
struct eq_vd_cur {
   const vec_t &a; const dq_t &b; int &r;
   eq_vd_cur(const vec_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      r = bc::segmented_equal(wv_t(a.begin()), wv_t(a.end()), wd_t(b.begin()), wd_t(b.end())) ? 1 : 0;
      escape(&r);  }
};
struct eq_vd_adj {
   const vec_t &a; const dq_t &b; int &r;
   eq_vd_adj(const vec_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      if(a.size() != b.size()) { r = 0; }
      else {
         bc::segtrio<wv_t, wd_t, bool> t =
            leaf_adjend(wv_t(a.begin()), wv_t(a.end()), wd_t(b.begin()), wd_t(b.end()),
                        bc::detail_algo::segmented_default_equal_to());
         r = (t.first == wv_t(a.end())) ? 1 : 0;
      }
      escape(&r);  }
};
struct eq_vd_std {
   const vec_t &a; const dq_t &b; int &r;
   eq_vd_std(const vec_t &a_, const dq_t &b_, int &r_) : a(a_), b(b_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   {  clobber();
      r = std::equal(a.begin(), a.end(), b.begin(), b.end()) ? 1 : 0;
      escape(&r);  }
};

template<class F>
static double time_one(F f, std::size_t iters, std::size_t n)
{
   cpu_timer t;
   t.resume();
   for(std::size_t i = 0; i != iters; ++i)
      f();
   t.stop();
   return double(t.elapsed()) / double(iters * n);
}

template<class F>
static double med3(F f, std::size_t iters, std::size_t n)
{
   double a = time_one(f, iters, n), b = time_one(f, iters, n), c = time_one(f, iters, n);
   if(a > b) { double t = a; a = b; b = t; }
   if(b > c) { double t = b; b = c; c = t; }
   if(a > b) { double t = a; a = b; b = t; }
   return b;
}

int main()
{
   const std::size_t N = 100000, IT = 3000;
   dq_t  d1, d2;
   vec_t v;
   for(std::size_t i = 0; i != N; ++i) {
      d1.push_back(MyInt(int(i))); d2.push_back(MyInt(int(i))); v.push_back(MyInt(int(i)));
   }
   int r[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

   const double mm_std  = med3(mm_dd_std(d1, d2, r[0]), IT, N);
   const double mm_cur  = med3(mm_dd_cur(d1, d2, r[1]), IT, N);
   const double mm_adj  = med3(mm_dd_adj(d1, d2, r[2]), IT, N);
   const double mm_dw   = med3(mm_dd_dw(d1, d2, r[6]),  IT, N);
   const double mv_std  = med3(mm_vd_std(v, d2, r[7]),  IT, N);
   const double mv_cur  = med3(mm_vd_cur(v, d2, r[8]),  IT, N);
   const double mv_dw   = med3(mm_vd_dw(v, d2, r[9]),   IT, N);
   const double eq_std  = med3(eq_vd_std(v, d2, r[3]),  IT, N);
   const double eq_cur  = med3(eq_vd_cur(v, d2, r[4]),  IT, N);
   const double eq_adj  = med3(eq_vd_adj(v, d2, r[5]),  IT, N);

   for(int i = 0; i != 10; ++i)
      if(r[i] != 1) { std::printf("BAD RESULT %d\n", i); return 1; }

   std::printf("mismatch_2r D+D  std %.4f  cur %.4f (std/cur %.2f)  adj %.4f (std/adj %.2f)  dw %.4f (std/dw %.2f)\n",
               mm_std, mm_cur, mm_std / mm_cur, mm_adj, mm_std / mm_adj, mm_dw, mm_std / mm_dw);
   std::printf("mismatch_2r V+D  std %.4f  cur %.4f (std/cur %.2f)  dw %.4f (std/dw %.2f)\n",
               mv_std, mv_cur, mv_std / mv_cur, mv_dw, mv_std / mv_dw);
   std::printf("equal_2r    V+D  std %.4f  cur %.4f (std/cur %.2f)  adj %.4f (std/adj %.2f)\n",
               eq_std, eq_cur, eq_std / eq_cur, eq_adj, eq_std / eq_adj);
   return 0;
}
