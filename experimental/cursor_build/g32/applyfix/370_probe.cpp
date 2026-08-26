// Conditional-write leaf conformance probe.
//
// Counts predicate / element-comparison applications performed by the
// conditional-write segmented algorithms against destinations of several
// segmentation depths, and dumps the resulting container contents plus the
// offset of the returned iterator so that a pre-fix and a post-fix run can be
// diffed.
//
//   COUNT lines: applied vs mandated (the standard's exact bound).
//   SHAPE lines: destination contents and returned iterator offsets; these
//                must be byte-identical before and after the change.

#include <cstdio>
#include <cstddef>
#include <string>

#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <segmented_test_helper.hpp>

#include <boost/container/experimental/segmented_copy_if.hpp>
#include <boost/container/experimental/segmented_remove_copy.hpp>
#include <boost/container/experimental/segmented_remove_copy_if.hpp>
#include <boost/container/experimental/segmented_remove.hpp>
#include <boost/container/experimental/segmented_remove_if.hpp>
#include <boost/container/experimental/segmented_partition_copy.hpp>

namespace bc = boost::container;

long g_cnt  = 0;
int  g_fail = 0;

struct cnt_int
{
   int v;
   cnt_int() : v(0) {}
   cnt_int(int x) : v(x) {}
};

inline bool operator==(const cnt_int &a, const cnt_int &b)
{  ++g_cnt;  return a.v == b.v;  }

inline bool operator!=(const cnt_int &a, const cnt_int &b)
{  return !(a.v == b.v);  }

struct is_odd
{  bool operator()(const cnt_int &x) const { ++g_cnt; return (x.v & 1) != 0; }  };

struct never
{  bool operator()(const cnt_int &x) const { ++g_cnt; return x.v < -1000000; }  };

struct always
{  bool operator()(const cnt_int &x) const { ++g_cnt; return x.v > -1000000; }  };

//////////////////////////////////////////////////////////////////////////////
// Destination makers
//////////////////////////////////////////////////////////////////////////////

struct mk_flat
{
   typedef bc::vector<cnt_int> cont_t;
   static const char *name() { return "flat"; }
   void make(cont_t &c, std::size_t n) const { c.assign(n, cnt_int(-1)); }
};

template <std::size_t B>
struct mk_deque
{
   typedef typename bc::deque_options<bc::block_size<B> >::type opt_t;
   typedef bc::deque<cnt_int, void, opt_t> cont_t;
   static const char *name() { return B == 8 ? "deque8" : "deque128"; }
   void make(cont_t &c, std::size_t n) const { c.assign(n, cnt_int(-1)); }
};

template <std::size_t S, bool Empties>
struct mk_seg1
{
   typedef test_detail::seg_vector<cnt_int> cont_t;
   static const char *name()
   {
      return Empties ? "seg1(8,empties)" : (S == 8 ? "seg1(8)" : "seg1(128)");
   }
   void make(cont_t &c, std::size_t n) const
   {
      std::size_t done = 0;
      while(done < n) {
         const std::size_t k = (n - done) < S ? (n - done) : S;
         if(Empties)
            c.add_segment(0u, cnt_int(-1));
         c.add_segment(k, cnt_int(-1));
         done += k;
      }
      if(Empties)
         c.add_segment(0u, cnt_int(-1));
   }
};

template <std::size_t S>
struct mk_seg2
{
   typedef test_detail::seg2_vector<cnt_int> cont_t;
   static const char *name() { return S == 8 ? "seg2(8x4)" : "seg2(128x4)"; }
   void make(cont_t &c, std::size_t n) const
   {
      std::size_t done = 0;
      while(done < n) {
         test_detail::seg_vector<cnt_int> inner;
         for(std::size_t j = 0; j != 4u && done < n; ++j) {
            const std::size_t k = (n - done) < S ? (n - done) : S;
            inner.add_segment(k, cnt_int(-1));
            done += k;
         }
         c.add_segment(inner);
      }
   }
};

//////////////////////////////////////////////////////////////////////////////
// Reporting
//////////////////////////////////////////////////////////////////////////////

void report_count(const char *algo, const char *src, const char *dst,
                  const char *cse, std::size_t n, long got, long want)
{
   const bool ok = (got == want);
   if(!ok) ++g_fail;
   std::printf("COUNT %-16s src=%-14s dst=%-18s %-6s n=%-5lu applied=%-7ld mandated=%-7ld %s\n",
               algo, src, dst, cse, (unsigned long)n, got, want, ok ? "ok" : "MISMATCH");
}

template <class Cont>
std::string contents(Cont &c)
{
   std::string s;
   char buf[32];
   for(typename Cont::iterator it = c.begin(), e = c.end(); it != e; ++it) {
      std::snprintf(buf, sizeof buf, "%d,", (*it).v);
      s += buf;
   }
   return s;
}

template <class Cont>
std::size_t ret_offset(Cont &c, typename Cont::iterator r)
{
   std::size_t k = 0;
   typename Cont::iterator it = c.begin();
   const typename Cont::iterator e = c.end();
   while(it != r) {
      if(it == e) return 999999u;
      ++it;
      ++k;
   }
   return k;
}

template <class Cont>
void report_shape(const char *algo, const char *src, const char *dst,
                  const char *cse, Cont &c, typename Cont::iterator r)
{
   std::printf("SHAPE %-16s src=%-14s dst=%-18s %-6s ret=%-6lu out=%s\n",
               algo, src, dst, cse, (unsigned long)ret_offset(c, r), contents(c).c_str());
}

//////////////////////////////////////////////////////////////////////////////
// Per-algorithm drivers.  Each is run three times per destination kind:
//   "exact" destination sized to the number of writes,
//   "spare" destination one element longer,
//   "none"  predicate/value selecting nothing, so nothing is written.
//////////////////////////////////////////////////////////////////////////////

template <class SrcIt, class Maker>
void run_copy_if(const char *srcname, SrcIt sf, SrcIt sl, std::size_t n,
                 std::size_t n_odd, const Maker &mk)
{
   typedef typename Maker::cont_t cont_t;
   {
      cont_t dst;
      mk.make(dst, n_odd);
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_copy_if(sf, sl, dst.begin(), is_odd());
      const long c = g_cnt;
      report_count("copy_if", srcname, Maker::name(), "exact", n, c, (long)n);
      report_shape("copy_if", srcname, Maker::name(), "exact", dst, r);
   }
   {
      cont_t dst;
      mk.make(dst, n_odd + 1u);
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_copy_if(sf, sl, dst.begin(), is_odd());
      const long c = g_cnt;
      report_count("copy_if", srcname, Maker::name(), "spare", n, c, (long)n);
      report_shape("copy_if", srcname, Maker::name(), "spare", dst, r);
   }
   {
      cont_t dst;
      mk.make(dst, 4u);
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_copy_if(sf, sl, dst.begin(), never());
      const long c = g_cnt;
      report_count("copy_if", srcname, Maker::name(), "none", n, c, (long)n);
      report_shape("copy_if", srcname, Maker::name(), "none", dst, r);
   }
}

template <class SrcIt, class Maker>
void run_remove_copy(const char *srcname, SrcIt sf, SrcIt sl, std::size_t n,
                     std::size_t n_keep, const Maker &mk)
{
   typedef typename Maker::cont_t cont_t;
   {
      cont_t dst;
      mk.make(dst, n_keep);
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_remove_copy(sf, sl, dst.begin(), cnt_int(7));
      const long c = g_cnt;
      report_count("remove_copy", srcname, Maker::name(), "exact", n, c, (long)n);
      report_shape("remove_copy", srcname, Maker::name(), "exact", dst, r);
   }
   {
      cont_t dst;
      mk.make(dst, n_keep + 1u);
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_remove_copy(sf, sl, dst.begin(), cnt_int(7));
      const long c = g_cnt;
      report_count("remove_copy", srcname, Maker::name(), "spare", n, c, (long)n);
      report_shape("remove_copy", srcname, Maker::name(), "spare", dst, r);
   }
}

template <class SrcIt, class Maker>
void run_remove_copy_if(const char *srcname, SrcIt sf, SrcIt sl, std::size_t n,
                        std::size_t n_even, const Maker &mk)
{
   typedef typename Maker::cont_t cont_t;
   {
      cont_t dst;
      mk.make(dst, n_even);
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_remove_copy_if(sf, sl, dst.begin(), is_odd());
      const long c = g_cnt;
      report_count("remove_copy_if", srcname, Maker::name(), "exact", n, c, (long)n);
      report_shape("remove_copy_if", srcname, Maker::name(), "exact", dst, r);
   }
   {
      cont_t dst;
      mk.make(dst, n_even + 1u);
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_remove_copy_if(sf, sl, dst.begin(), is_odd());
      const long c = g_cnt;
      report_count("remove_copy_if", srcname, Maker::name(), "spare", n, c, (long)n);
      report_shape("remove_copy_if", srcname, Maker::name(), "spare", dst, r);
   }
   {
      cont_t dst;
      mk.make(dst, 4u);
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_remove_copy_if(sf, sl, dst.begin(), always());
      const long c = g_cnt;
      report_count("remove_copy_if", srcname, Maker::name(), "none", n, c, (long)n);
      report_shape("remove_copy_if", srcname, Maker::name(), "none", dst, r);
   }
}

template <class SrcIt, class Maker>
void run_partition_copy(const char *srcname, SrcIt sf, SrcIt sl, std::size_t n,
                        std::size_t n_odd, const Maker &mk)
{
   typedef typename Maker::cont_t cont_t;
   typedef std::pair<typename cont_t::iterator, typename cont_t::iterator> pair_t;
   {
      cont_t t, f;
      mk.make(t, n_odd);
      mk.make(f, n - n_odd);
      g_cnt = 0;
      const pair_t r = bc::segmented_partition_copy(sf, sl, t.begin(), f.begin(), is_odd());
      const long c = g_cnt;
      report_count("partition_copy", srcname, Maker::name(), "exact", n, c, (long)n);
      report_shape("partition_copy/T", srcname, Maker::name(), "exact", t, r.first);
      report_shape("partition_copy/F", srcname, Maker::name(), "exact", f, r.second);
   }
   {
      cont_t t, f;
      mk.make(t, n_odd + 1u);
      mk.make(f, n - n_odd + 1u);
      g_cnt = 0;
      const pair_t r = bc::segmented_partition_copy(sf, sl, t.begin(), f.begin(), is_odd());
      const long c = g_cnt;
      report_count("partition_copy", srcname, Maker::name(), "spare", n, c, (long)n);
      report_shape("partition_copy/T", srcname, Maker::name(), "spare", t, r.first);
      report_shape("partition_copy/F", srcname, Maker::name(), "spare", f, r.second);
   }
   {
      cont_t t, f;
      mk.make(t, 4u);
      mk.make(f, n);
      g_cnt = 0;
      const pair_t r = bc::segmented_partition_copy(sf, sl, t.begin(), f.begin(), never());
      const long c = g_cnt;
      report_count("partition_copy", srcname, Maker::name(), "none", n, c, (long)n);
      report_shape("partition_copy/T", srcname, Maker::name(), "none", t, r.first);
      report_shape("partition_copy/F", srcname, Maker::name(), "none", f, r.second);
   }
}

// remove / remove_if work in place, so the "destination" is the source range.
template <class Maker>
void run_remove_inplace(const int *vals, std::size_t n, const Maker &mk)
{
   typedef typename Maker::cont_t cont_t;
   {
      cont_t c;
      mk.make(c, n);
      {
         typename cont_t::iterator it = c.begin();
         for(std::size_t i = 0; i != n; ++i, ++it)
            *it = cnt_int(vals[i]);
      }
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_remove(c.begin(), c.end(), cnt_int(7));
      const long cc = g_cnt;
      report_count("remove", "self", Maker::name(), "inpl", n, cc, (long)n);
      report_shape("remove", "self", Maker::name(), "inpl", c, r);
   }
   {
      cont_t c;
      mk.make(c, n);
      {
         typename cont_t::iterator it = c.begin();
         for(std::size_t i = 0; i != n; ++i, ++it)
            *it = cnt_int(vals[i]);
      }
      g_cnt = 0;
      const typename cont_t::iterator r = bc::segmented_remove_if(c.begin(), c.end(), is_odd());
      const long cc = g_cnt;
      report_count("remove_if", "self", Maker::name(), "inpl", n, cc, (long)n);
      report_shape("remove_if", "self", Maker::name(), "inpl", c, r);
   }
}

//////////////////////////////////////////////////////////////////////////////

template <class SrcIt>
void run_dsts(const char *srcname, SrcIt sf, SrcIt sl, std::size_t n,
              std::size_t n_odd, std::size_t n_keep7)
{
   const std::size_t n_even = n - n_odd;

   run_copy_if(srcname, sf, sl, n, n_odd, mk_flat());
   run_copy_if(srcname, sf, sl, n, n_odd, mk_deque<8>());
   run_copy_if(srcname, sf, sl, n, n_odd, mk_deque<128>());
   run_copy_if(srcname, sf, sl, n, n_odd, mk_seg1<8, false>());
   run_copy_if(srcname, sf, sl, n, n_odd, mk_seg1<128, false>());
   run_copy_if(srcname, sf, sl, n, n_odd, mk_seg1<8, true>());
   run_copy_if(srcname, sf, sl, n, n_odd, mk_seg2<8>());
   run_copy_if(srcname, sf, sl, n, n_odd, mk_seg2<128>());

   run_remove_copy(srcname, sf, sl, n, n_keep7, mk_flat());
   run_remove_copy(srcname, sf, sl, n, n_keep7, mk_deque<8>());
   run_remove_copy(srcname, sf, sl, n, n_keep7, mk_deque<128>());
   run_remove_copy(srcname, sf, sl, n, n_keep7, mk_seg1<8, false>());
   run_remove_copy(srcname, sf, sl, n, n_keep7, mk_seg1<128, false>());
   run_remove_copy(srcname, sf, sl, n, n_keep7, mk_seg1<8, true>());
   run_remove_copy(srcname, sf, sl, n, n_keep7, mk_seg2<8>());
   run_remove_copy(srcname, sf, sl, n, n_keep7, mk_seg2<128>());

   run_remove_copy_if(srcname, sf, sl, n, n_even, mk_flat());
   run_remove_copy_if(srcname, sf, sl, n, n_even, mk_deque<8>());
   run_remove_copy_if(srcname, sf, sl, n, n_even, mk_deque<128>());
   run_remove_copy_if(srcname, sf, sl, n, n_even, mk_seg1<8, false>());
   run_remove_copy_if(srcname, sf, sl, n, n_even, mk_seg1<128, false>());
   run_remove_copy_if(srcname, sf, sl, n, n_even, mk_seg1<8, true>());
   run_remove_copy_if(srcname, sf, sl, n, n_even, mk_seg2<8>());
   run_remove_copy_if(srcname, sf, sl, n, n_even, mk_seg2<128>());

   run_partition_copy(srcname, sf, sl, n, n_odd, mk_flat());
   run_partition_copy(srcname, sf, sl, n, n_odd, mk_deque<8>());
   run_partition_copy(srcname, sf, sl, n, n_odd, mk_deque<128>());
   run_partition_copy(srcname, sf, sl, n, n_odd, mk_seg1<8, false>());
   run_partition_copy(srcname, sf, sl, n, n_odd, mk_seg1<128, false>());
   run_partition_copy(srcname, sf, sl, n, n_odd, mk_seg1<8, true>());
   run_partition_copy(srcname, sf, sl, n, n_odd, mk_seg2<8>());
   run_partition_copy(srcname, sf, sl, n, n_odd, mk_seg2<128>());
}

int main()
{
   const std::size_t N = 300;
   int vals[N];
   std::size_t n_odd = 0, n_keep7 = 0;
   for(std::size_t i = 0; i != N; ++i) {
      vals[i] = (int)(i % 23);
      if(vals[i] & 1) ++n_odd;
      if(vals[i] != 7) ++n_keep7;
   }

   bc::vector<cnt_int> flat_src;
   flat_src.reserve(N);
   for(std::size_t i = 0; i != N; ++i)
      flat_src.push_back(cnt_int(vals[i]));

   typedef bc::deque_options<bc::block_size<16> >::type src_opt_t;
   typedef bc::deque<cnt_int, void, src_opt_t> seg_src_t;
   seg_src_t seg_src;
   for(std::size_t i = 0; i != N; ++i)
      seg_src.push_back(cnt_int(vals[i]));

   run_dsts("flat", flat_src.begin(), flat_src.end(), N, n_odd, n_keep7);
   run_dsts("deque(16)", seg_src.begin(), seg_src.end(), N, n_odd, n_keep7);

   run_remove_inplace(vals, N, mk_flat());
   run_remove_inplace(vals, N, mk_deque<8>());
   run_remove_inplace(vals, N, mk_deque<128>());
   run_remove_inplace(vals, N, mk_seg1<8, false>());
   run_remove_inplace(vals, N, mk_seg1<128, false>());
   run_remove_inplace(vals, N, mk_seg1<8, true>());
   run_remove_inplace(vals, N, mk_seg2<8>());
   run_remove_inplace(vals, N, mk_seg2<128>());

   std::printf("\nfailures=%d\n", g_fail);
   return g_fail != 0;
}
