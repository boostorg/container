// Does routing search's verify step through segmented_mismatch cost anything?
//
// Only the NON-SEGMENTED haystack path changed: HEAD used a hand-rolled
// lock-step loop, now it calls segmented_mismatch_bounded_dispatch.  The
// segmented path already used mismatch at HEAD, so it is a control here.
//
// The group-20 bench patterns yield ~1 candidate (hit) and 0 candidates
// (miss), so they never stress verification.  This probe sweeps candidate
// density and needle length, which is what decides whether the extra
// mismatch prologue (two difference_type subtractions + min) pays off
// against its unrolled counted loop.

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

// ---------------------------------------------------------------------------
// HEAD's non-segmented dispatch: find_if + hand-rolled verify loop.
// ---------------------------------------------------------------------------
template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
FwdIt1 search_old(FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last)
{
   if (BOOST_UNLIKELY(s_first == s_last))
      return first;

   da::equal_to_deref<FwdIt2> eq(s_first);

   while (first != last) {
      first = boost::container::segmented_find_if(first, last, eq);
      if (first == last)
         return last;

      FwdIt1 it = first;
      FwdIt2 s_it = s_first;
      for(;;) {
         ++it;
         ++s_it;
         if(s_it == s_last)
            return first;
         if(it == last)
            return last;
         if(!(*it == *s_it))
            break;
      }
      ++first;
   }
   return last;
}

// ---------------------------------------------------------------------------
template <class F>
static double time_ns(F f, std::size_t iters, std::size_t n)
{
   cpu_timer t;
   t.resume();
   for (std::size_t i = 0; i != iters; ++i)
      f();
   t.stop();
   return double(t.elapsed()) / double(iters * n);
}

static const std::size_t N = 100000;

// period == 0 -> strictly increasing data (candidates are rare)
// period  > 0 -> values repeat mod `period` (candidates every `period` slots)
template <class C>
static void fill_data(C &c, std::size_t period)
{
   c.clear();
   for (std::size_t i = 0; i != N; ++i)
      c.push_back(period ? int(i % period) : int(i));
}

template <class C>
static void run(const char *what, const C &c, std::size_t period,
                std::size_t needle_len, std::size_t iters)
{
   typedef typename C::const_iterator cit_t;

   // Needle that never fully matches: take a real prefix then poison the end,
   // so every candidate start is verified and then rejected.
   std::vector<int> pat;
   for (std::size_t k = 0; k != needle_len; ++k)
      pat.push_back(period ? int(k % period) : int(N / 2 + k));
   pat[needle_len - 1] = -12345;      // force verify to fail late

   const int *pb = &pat[0];
   const int *pe = pb + needle_len;

   const double o = time_ns([&]{
      cit_t r = search_old(c.begin(), c.end(), pb, pe);
      sink = (r == c.end()) ? 0 : 1;
   }, iters, N);

   const double n = time_ns([&]{
      cit_t r = bc::segmented_search(c.begin(), c.end(), pb, pe);
      sink = (r == c.end()) ? 0 : 1;
   }, iters, N);

   const std::size_t cands = period ? (N / period) : 1;
   std::printf("  %-18s period=%-6zu needle=%-3zu cands~%-7zu old=%.4f new=%.4f  new/old=%.2f\n",
               what, period, needle_len, cands, o, n, n / o);
}

int main()
{
   typedef bc::vector<int> vec_t;
   typedef bc::deque<int>  dq_t;

   std::printf("N=%zu   (new/old > 1.00 means the mismatch refactor is slower)\n\n", N);

   // ---- non-segmented haystack: THIS is the path the refactor changed ----
   std::printf("non-segmented haystack (bc::vector) - CHANGED PATH\n");
   {
      vec_t v;
      fill_data(v, 0);
      run("sparse", v, 0, 3, 200);
      run("sparse", v, 0, 8, 200);
   }
   {
      vec_t v;
      fill_data(v, 64);   // a candidate every 64 elements
      run("dense/64", v, 64, 3, 200);
      run("dense/64", v, 64, 8, 200);
   }
   {
      vec_t v;
      fill_data(v, 4);    // a candidate every 4 elements
      run("dense/4", v, 4, 3, 200);
      run("dense/4", v, 4, 8, 200);
   }
   {
      vec_t v;
      fill_data(v, 1);    // every position is a candidate: worst case
      run("every-pos", v, 1, 3, 100);
      run("every-pos", v, 1, 8, 100);
      run("every-pos", v, 1, 32, 100);
   }

   // ---- segmented haystack: control, unchanged by the refactor ----
   std::printf("\nsegmented haystack (bc::deque) - CONTROL, unchanged\n");
   {
      dq_t d;
      fill_data(d, 0);
      run("sparse", d, 0, 3, 100);
   }
   {
      dq_t d;
      fill_data(d, 4);
      run("dense/4", d, 4, 3, 100);
   }
   {
      dq_t d;
      fill_data(d, 1);
      run("every-pos", d, 1, 3, 50);
   }
   return 0;
}
