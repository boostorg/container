// Three verify strategies for segmented_search, same find_if front end:
//
//   old    - hand-rolled lock-step loop (HEAD's non-segmented overload)
//   new    - delegate straight to segmented_mismatch_bounded_dispatch
//   hybrid - reject the candidate inline on the 2nd element, delegate only
//            the tail (needle length >= 3 that survived two matches)
//
// Rationale for hybrid: nearly every candidate dies on its 2nd element, and
// paying a mismatch dispatch (two difference_type subtractions + min + a
// possibly out-of-line call) to compare one pair is the wrong trade.  Long
// needles still reach mismatch and keep its unrolled counted leaf.

#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_search.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
#include <bench_utils.hpp>

#include <cstdio>
#include <cstddef>
#include <vector>

namespace bc = boost::container;
namespace da = boost::container::detail_algo;

volatile int sink;

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
FwdIt1 search_old(FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last)
{
   if (BOOST_UNLIKELY(s_first == s_last)) return first;
   da::equal_to_deref<FwdIt2> eq(s_first);
   while (first != last) {
      first = boost::container::segmented_find_if(first, last, eq);
      if (first == last) return last;
      FwdIt1 it = first;
      FwdIt2 s_it = s_first;
      for(;;) {
         ++it; ++s_it;
         if(s_it == s_last) return first;
         if(it == last)     return last;
         if(!(*it == *s_it)) break;
      }
      ++first;
   }
   return last;
}

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2, class Tag>
FwdIt1 search_hybrid_disp(FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last, Tag tag)
{
   if (BOOST_UNLIKELY(s_first == s_last)) return first;
   typedef typename bc::iterator_traits<FwdIt1>::iterator_category cat_t;
   da::equal_to_deref<FwdIt2> eq(s_first);

   while (first != last) {
      first = boost::container::segmented_find_if(first, last, eq);
      if (first == last) return last;

      FwdIt1 it = first;  ++it;
      FwdIt2 s_it = s_first; ++s_it;
      if (s_it == s_last) return first;
      if (it == last)     return last;

      //Peel one comparison: the overwhelming majority of candidates die here,
      //and this avoids a dispatch just to compare a single pair.
      if (!(*it == *s_it)) { ++first; continue; }
      ++it; ++s_it;
      if (s_it == s_last) return first;
      if (it == last)     return last;

      const bc::segduo<FwdIt1, FwdIt2> r = (da::segmented_search_verify)
         (it, last, s_it, s_last, tag, cat_t());
      if (r.second == s_last) return first;
      if (r.first == last)    return last;
      ++first;
   }
   return last;
}

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
BOOST_CONTAINER_FORCEINLINE
FwdIt1 search_hybrid(FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last)
{
   typedef bc::segmented_iterator_traits<FwdIt1> traits;
   return search_hybrid_disp(first, last, s_first, s_last,
                             typename traits::is_segmented_iterator());
}

template <class F>
static double time_ns(F f, std::size_t iters, std::size_t n)
{
   cpu_timer t;
   t.resume();
   for (std::size_t i = 0; i != iters; ++i) f();
   t.stop();
   return double(t.elapsed()) / double(iters * n);
}

static const std::size_t N = 100000;

template <class C>
static void fill_data(C &c, std::size_t period)
{
   c.clear();
   for (std::size_t i = 0; i != N; ++i)
      c.push_back(period ? int(i % period) : int(i));
}

template <class C>
static void run(const char *what, const C &c, std::size_t period,
                std::size_t nl, std::size_t iters)
{
   typedef typename C::const_iterator cit_t;
   std::vector<int> pat;
   for (std::size_t k = 0; k != nl; ++k)
      pat.push_back(period ? int(k % period) : int(N/2 + k));
   pat[nl-1] = -12345;
   const int *pb = &pat[0];
   const int *pe = pb + nl;

   const double o = time_ns([&]{ cit_t r = search_old(c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);
   const double n = time_ns([&]{ cit_t r = bc::segmented_search(c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);
   const double h = time_ns([&]{ cit_t r = search_hybrid(c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);

   std::printf("  %-10s per=%-6zu ndl=%-3zu old=%.4f new=%.4f hyb=%.4f | new/old=%.2f hyb/old=%.2f hyb/new=%.2f\n",
               what, period, nl, o, n, h, n/o, h/o, h/n);
}

int main()
{
   typedef bc::vector<int> vec_t;
   typedef bc::deque<int>  dq_t;
   std::printf("N=%zu\n\nNON-SEGMENTED haystack (path changed by the refactor)\n", N);
   { vec_t v; fill_data(v,0);  run("sparse",v,0,3,200); run("sparse",v,0,8,200); }
   { vec_t v; fill_data(v,64); run("dense/64",v,64,3,200); }
   { vec_t v; fill_data(v,4);  run("dense/4",v,4,3,200);  run("dense/4",v,4,8,200); }
   { vec_t v; fill_data(v,1);  run("every-pos",v,1,3,100); run("every-pos",v,1,8,100);
                               run("every-pos",v,1,32,100); }

   std::printf("\nSEGMENTED haystack (already used mismatch at HEAD)\n");
   { dq_t d; fill_data(d,0); run("sparse",d,0,3,100); }
   { dq_t d; fill_data(d,4); run("dense/4",d,4,3,100); run("dense/4",d,4,8,100); }
   { dq_t d; fill_data(d,1); run("every-pos",d,1,3,50); run("every-pos",d,1,32,50); }
   return 0;
}
