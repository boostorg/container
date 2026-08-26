// 362: count predicate applications performed by bc::segmented_copy_if for a
// flat destination versus a segmented destination, against the standard's
// mandate of "exactly last - first applications of pred" ([alg.copy]/copy_if).
//
// No timing here at all.  Destination shapes covered:
//   flat   bc::vector<int>
//   deque  bc::deque<int, void, block_size<B> >          (depth 1)
//   seg1   test_detail::seg_vector<int>                  (depth 1, custom)
//   seg2   test_detail::seg2_vector<int>                 (depth 2, custom)
// Source shapes: flat bc::vector<int>, and bc::deque (to cover 1+2S).

#include <cstddef>
#include <cstdio>
#include <iterator>

#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_copy_if.hpp>
#include <boost/container/experimental/wrapped_iterator.hpp>
#include "../../../segmented_test_helper.hpp"

namespace bc = boost::container;

struct counting_pred
{
   long *cnt;
   int   every;   // 0 => never true, 1 => always, 2 => every other, ...
   counting_pred(long *c, int e) : cnt(c), every(e) {}
   bool operator()(int x) const
   {
      ++*cnt;
      if(every == 0) return false;
      return (x % every) == 0;
   }
};

template<class SegIt>
std::size_t count_segments(SegIt first, SegIt last)
{
   typedef bc::segmented_iterator_traits<SegIt> tr;
   typename tr::segment_iterator s = tr::segment(first);
   typename tr::segment_iterator e = tr::segment(last);
   std::size_t n = 1;
   for(; s != e; ++s) ++n;
   return n;
}

//---------------------------------------------------------------- deque dest
template<std::size_t B, class SrcIt>
void run_deque(const char *tag, SrcIt sf, SrcIt sl, std::size_t n, int every)
{
   typedef typename bc::deque_options< bc::block_size<B> >::type opt_t;
   typedef bc::deque<int, void, opt_t> dq_t;
   dq_t out(n);
   long cnt = 0;
   bc::segmented_copy_if(sf, sl, out.begin(), counting_pred(&cnt, every));
   const long mandated = static_cast<long>(n);
   std::printf("%-28s block=%-5u n=%-7u every=%-2d  pred_applied=%-8ld mandated=%-8ld extra=%-6ld (%.3f%%)\n",
      tag, unsigned(B), unsigned(n), every, cnt, mandated, cnt - mandated,
      100.0 * double(cnt - mandated) / double(mandated));
}

//---------------------------------------------------------------- flat dest
template<class SrcIt>
void run_flat(const char *tag, SrcIt sf, SrcIt sl, std::size_t n, int every)
{
   bc::vector<int> out(n);
   long cnt = 0;
   bc::segmented_copy_if(sf, sl, out.begin(), counting_pred(&cnt, every));
   const long mandated = static_cast<long>(n);
   std::printf("%-28s block=%-5s n=%-7u every=%-2d  pred_applied=%-8ld mandated=%-8ld extra=%-6ld (%.3f%%)\n",
      tag, "-", unsigned(n), every, cnt, mandated, cnt - mandated,
      100.0 * double(cnt - mandated) / double(mandated));
}

//------------------------------------------------- nsg (wrapped) flat/deque
template<std::size_t B, class SrcIt>
void run_deque_nsg(const char *tag, SrcIt sf, SrcIt sl, std::size_t n, int every)
{
   typedef typename bc::deque_options< bc::block_size<B> >::type opt_t;
   typedef bc::deque<int, void, opt_t> dq_t;
   dq_t out(n);
   long cnt = 0;
   bc::segmented_copy_if(bc::wrapped_iterator<SrcIt>(sf), bc::wrapped_iterator<SrcIt>(sl),
                         bc::wrapped_iterator<typename dq_t::iterator>(out.begin()),
                         counting_pred(&cnt, every));
   const long mandated = static_cast<long>(n);
   std::printf("%-28s block=%-5u n=%-7u every=%-2d  pred_applied=%-8ld mandated=%-8ld extra=%-6ld (%.3f%%)\n",
      tag, unsigned(B), unsigned(n), every, cnt, mandated, cnt - mandated,
      100.0 * double(cnt - mandated) / double(mandated));
}

//---------------------------------------------------------- seg_vector dest
void run_seg1(const char *tag, const int *sf, const int *sl, std::size_t n,
              std::size_t seg, int every)
{
   test_detail::seg_vector<int> out;
   bc::vector<int> zeros(seg, 0);
   for(std::size_t i = 0; i < (n + seg - 1) / seg; ++i)
      out.add_segment_range(zeros.begin(), zeros.end());
   long cnt = 0;
   bc::segmented_copy_if(sf, sl, out.begin(), counting_pred(&cnt, every));
   const long mandated = static_cast<long>(n);
   std::printf("%-28s block=%-5u n=%-7u every=%-2d  pred_applied=%-8ld mandated=%-8ld extra=%-6ld (%.3f%%)\n",
      tag, unsigned(seg), unsigned(n), every, cnt, mandated, cnt - mandated,
      100.0 * double(cnt - mandated) / double(mandated));
}

//--------------------------------------------------------- seg2_vector dest
void run_seg2(const char *tag, const int *sf, const int *sl, std::size_t n,
              std::size_t inner, std::size_t inner_per_outer, int every)
{
   test_detail::seg2_vector<int> out;
   bc::vector<int> zeros(inner, 0);
   const std::size_t total_inner = (n + inner - 1) / inner + 1;
   std::size_t made = 0;
   while(made < total_inner) {
      test_detail::seg_vector<int> sv;
      for(std::size_t k = 0; k < inner_per_outer && made < total_inner; ++k, ++made)
         sv.add_segment_range(zeros.begin(), zeros.end());
      out.add_segment(boost::move(sv));
   }
   long cnt = 0;
   bc::segmented_copy_if(sf, sl, out.begin(), counting_pred(&cnt, every));
   const long mandated = static_cast<long>(n);
   std::printf("%-28s block=%-5u n=%-7u every=%-2d  pred_applied=%-8ld mandated=%-8ld extra=%-6ld (%.3f%%)\n",
      tag, unsigned(inner), unsigned(n), every, cnt, mandated, cnt - mandated,
      100.0 * double(cnt - mandated) / double(mandated));
}

int main()
{
   // -------- sanity: what does block_size<B> actually mean, in elements? ----
   {
      typedef bc::deque_options< bc::block_size<8> >::type opt_t;
      bc::deque<int, void, opt_t> d(64);
      typedef bc::segmented_iterator_traits<bc::deque<int, void, opt_t>::iterator> tr;
      tr::segment_iterator s = tr::segment(d.begin());
      std::printf("deque block_size<8>, 64 elems: first segment holds %d elements, %u segments spanned\n",
         int(tr::end(s) - tr::begin(s)), unsigned(count_segments(d.begin(), d.end())));
   }
   std::printf("\n");

   // -------- the reported 71-vs-64 case -----------------------------------
   {
      const std::size_t n = 64;
      bc::vector<int> src(n);
      for(std::size_t i = 0; i < n; ++i) src[i] = int(i);
      std::printf("--- reported case: 64 source elements, every element copied ---\n");
      run_flat        ("copy_if flat dst",        src.begin(), src.end(), n, 1);
      run_deque<8>    ("copy_if deque dst",       src.begin(), src.end(), n, 1);
      run_deque_nsg<8>("copy_if deque dst (nsg)", src.begin(), src.end(), n, 1);
      std::printf("\n");
   }

   // -------- block-size sweep, flat source, deque destination --------------
   {
      const std::size_t n = 100000;
      bc::vector<int> src(n);
      for(std::size_t i = 0; i < n; ++i) src[i] = int(i);

      for(int every = 1; every <= 2; ++every) {
         std::printf("--- 2S: flat vector source, deque destination, every=%d (%s) ---\n",
                     every, every == 1 ? "all pass" : "half pass");
         run_flat      ("  flat dst  (control)", src.begin(), src.end(), n, every);
         run_deque<8>  ("  deque dst",           src.begin(), src.end(), n, every);
         run_deque<16> ("  deque dst",           src.begin(), src.end(), n, every);
         run_deque<32> ("  deque dst",           src.begin(), src.end(), n, every);
         run_deque<64> ("  deque dst",           src.begin(), src.end(), n, every);
         run_deque<128>("  deque dst",           src.begin(), src.end(), n, every);
         run_deque<256>("  deque dst",           src.begin(), src.end(), n, every);
         run_deque<512>("  deque dst",           src.begin(), src.end(), n, every);
         std::printf("\n");
      }

      std::printf("--- 2S miss: flat vector source, deque destination, every=0 (none pass) ---\n");
      run_flat      ("  flat dst  (control)", src.begin(), src.end(), n, 0);
      run_deque<8>  ("  deque dst",           src.begin(), src.end(), n, 0);
      run_deque<128>("  deque dst",           src.begin(), src.end(), n, 0);
      std::printf("\n");
   }

   // -------- 1+2S: deque source, deque destination -------------------------
   {
      const std::size_t n = 100000;
      typedef bc::deque_options< bc::block_size<128> >::type src_opt_t;
      bc::deque<int, void, src_opt_t> src(n);
      {
         int v = 0;
         for(bc::deque<int, void, src_opt_t>::iterator it = src.begin(); it != src.end(); ++it, ++v)
            *it = v;
      }
      std::printf("--- 1+2S: deque source (block 128), deque destination ---\n");
      run_flat      ("  flat dst  (1S control)", src.begin(), src.end(), n, 1);
      run_deque<128>("  deque dst",              src.begin(), src.end(), n, 1);
      run_deque<128>("  deque dst",              src.begin(), src.end(), n, 2);
      std::printf("\n");
   }

   // -------- recursively segmented destinations ----------------------------
   {
      const std::size_t n = 4096;
      bc::vector<int> src(n);
      for(std::size_t i = 0; i < n; ++i) src[i] = int(i);
      std::printf("--- recursively segmented destinations, flat source, n=%u ---\n", unsigned(n));
      const int *p = &src[0];
      run_flat("  flat dst  (control)", src.begin(), src.end(), n, 1);
      run_seg1("  seg_vector dst  (d1)", p, p + n, n, 64, 1);
      run_seg2("  seg2_vector dst (d2)", p, p + n, n, 64, 4, 1);
      run_seg1("  seg_vector dst  (d1)", p, p + n, n, 64, 2);
      run_seg2("  seg2_vector dst (d2)", p, p + n, n, 64, 4, 2);
      std::printf("\n");
   }

   return 0;
}
