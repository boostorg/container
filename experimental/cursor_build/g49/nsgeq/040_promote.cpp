// Does the *m_node register promotion depend on the loop shape of the leaf?
//
// In the group-20 binary, clang promotes the deque block base to a register
// for std::equal (lea block-end) but reloads it every element for the
// segmented_equal leaf (load+add), costing ~0.1 ns/elem on mixed rows.
// Standalone, both compile identically, so the trigger is contextual.
// This probe reproduces the measure_batch-like context and tries leaf shapes.

#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
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

//clobber()/escape() come from bench_utils.hpp

//shape A: exactly std::equal's loop, no iter2 bound at all
template<class I1, class S1, class I2>
__attribute__((noinline)) bool eq_shape_std(I1 f1, S1 l1, I2 f2)
{
   for(; f1 != l1; ++f1, ++f2)
      if(!(*f1 == *f2))
         return false;
   return true;
}

//shape B: the current leaf's shape with the iter2 test removed by hand
template<class I1, class S1, class I2>
__attribute__((noinline)) bool eq_shape_leaf(I1 f1, S1 l1, I2 f2)
{
   for(; f1 != l1; ++f1) {
      if(!(*f1 == *f2))
         break;
      ++f2;
   }
   return f1 == l1;
}

struct run_std {
   const dq_t &d; const vec_t &v; int &r;
   run_std(const dq_t &d_, const vec_t &v_, int &r_) : d(d_), v(v_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   { clobber(); r = std::equal(d.begin(), d.end(), v.begin()) ? 1 : 0; escape(&r); }
};
struct run_nsg {
   const dq_t &d; const vec_t &v; int &r;
   run_nsg(const dq_t &d_, const vec_t &v_, int &r_) : d(d_), v(v_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   { clobber(); r = bc::segmented_equal(wd_t(d.begin()), wd_t(d.end()), wv_t(v.begin())) ? 1 : 0; escape(&r); }
};
struct run_shape_std {
   const dq_t &d; const vec_t &v; int &r;
   run_shape_std(const dq_t &d_, const vec_t &v_, int &r_) : d(d_), v(v_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   { clobber(); r = eq_shape_std(wd_t(d.begin()), wd_t(d.end()), wv_t(v.begin())) ? 1 : 0; escape(&r); }
};
struct run_shape_leaf {
   const dq_t &d; const vec_t &v; int &r;
   run_shape_leaf(const dq_t &d_, const vec_t &v_, int &r_) : d(d_), v(v_), r(r_) {}
   BOOST_CONTAINER_FORCEINLINE void operator()()
   { clobber(); r = eq_shape_leaf(wd_t(d.begin()), wd_t(d.end()), wv_t(v.begin())) ? 1 : 0; escape(&r); }
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

int main()
{
   const std::size_t N = 100000, IT = 3000;
   dq_t  d;
   vec_t v;
   for(std::size_t i = 0; i != N; ++i) { d.push_back(MyInt(int(i))); v.push_back(MyInt(int(i))); }
   int r1 = 0, r2 = 0, r3 = 0, r4 = 0;

   const double t_std  = time_one(run_std(d, v, r1),        IT, N);
   const double t_nsg  = time_one(run_nsg(d, v, r2),        IT, N);
   const double t_sh_s = time_one(run_shape_std(d, v, r3),  IT, N);
   const double t_sh_l = time_one(run_shape_leaf(d, v, r4), IT, N);

   if(r1 != 1 || r2 != 1 || r3 != 1 || r4 != 1) { std::printf("BAD RESULTS\n"); return 1; }
   std::printf("std::equal          %.4f ns/elem\n", t_std);
   std::printf("segmented_equal nsg %.4f ns/elem   nsg/std=%.2f\n", t_nsg, t_nsg / t_std);
   std::printf("shape-std leaf      %.4f ns/elem   /std=%.2f\n", t_sh_s, t_sh_s / t_std);
   std::printf("shape-leaf leaf     %.4f ns/elem   /std=%.2f\n", t_sh_l, t_sh_l / t_std);
   return 0;
}
