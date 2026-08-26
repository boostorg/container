// A/B of candidate segmented_search verify shapes, all inside ONE binary.
//
//   flat  - HEAD's hand-rolled lock-step loop (the baseline to beat)
//   mm    - delegate to segmented_mismatch_bounded_dispatch (working tree)
//   own   - bespoke verify: flat lock-step leaf, bool result, needle walked
//           recursively by segments (collapses to the leaf for a flat needle)
//   ownc  - own + random-access clamp of the candidate scan to
//           last - (n2 - 1), so the verify runs unbounded on the haystack
//   owns  - own, but the haystack is also walked segment by segment
//
// Ratios are reported against `flat`; >1 means slower than flat.

#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
#include <boost/container/experimental/segmented_mismatch.hpp>
#include <boost/container/experimental/segmented_search.hpp>
#include <bench_utils.hpp>

#include <cstdio>
#include <cstddef>
#include <vector>

namespace bc = boost::container;
namespace da = boost::container::detail_algo;

volatile int sink;

//---------------------------------------------------------------------------
// arm: flat
//---------------------------------------------------------------------------
namespace arm_flat {

template <class I1, class S1, class I2, class S2>
I1 search(I1 first, S1 last, I2 s_first, S2 s_last)
{
   if(BOOST_UNLIKELY(s_first == s_last)) return first;
   da::equal_to_deref<I2> eq(s_first);
   while(first != last) {
      first = bc::segmented_find_if(first, last, eq);
      if(first == last) return last;
      I1 it = first;
      I2 s_it = s_first;
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

} // namespace arm_flat

//---------------------------------------------------------------------------
// arm: mm
//---------------------------------------------------------------------------
namespace arm_mm {

template <class I1, class S1, class I2, class S2, class Tag>
I1 disp(I1 first, S1 last, I2 s_first, S2 s_last, Tag tag)
{
   if(BOOST_UNLIKELY(s_first == s_last)) return first;
   typedef typename bc::iterator_traits<I1>::iterator_category cat_t;
   da::equal_to_deref<I2> eq(s_first);
   while(first != last) {
      first = bc::segmented_find_if(first, last, eq);
      if(first == last) return last;
      I1 it = first; ++it;
      I2 s_it = s_first; ++s_it;
      if(s_it == s_last) return first;
      if(it == last)     return last;
      const bc::segduo<I1, I2> r = (da::segmented_mismatch_bounded_dispatch)
         (it, last, s_it, s_last, da::mismatch_equal(), tag, cat_t());
      if(r.second == s_last) return first;
      if(r.first == last)    return last;
      ++first;
   }
   return last;
}

template <class I1, class S1, class I2, class S2>
BOOST_CONTAINER_FORCEINLINE I1 search(I1 first, S1 last, I2 s_first, S2 s_last)
{
   typedef bc::segmented_iterator_traits<I1> traits;
   return disp(first, last, s_first, s_last, typename traits::is_segmented_iterator());
}

} // namespace arm_mm

//---------------------------------------------------------------------------
// arm: own
//---------------------------------------------------------------------------
namespace arm_own {

template <class I1, class S1, class I2, class S2, class NdlTag>
BOOST_CONTAINER_FORCEINLINE
typename da::algo_enable_if_c
   <!NdlTag::value || bc::is_sentinel<S2, I2>::value, bc::segduo<I1, bool> >::type
verify(I1 it, S1 last, I2 s_it, S2 s_last, NdlTag)
{
   for(;;) {
      if(!(*it == *s_it))
         return bc::segduo<I1, bool>(it, false);
      ++it;
      ++s_it;
      if(s_it == s_last)
         return bc::segduo<I1, bool>(it, true);
      if(it == last)
         return bc::segduo<I1, bool>(it, false);
   }
}

template <class I1, class S1, class SegI2>
bc::segduo<I1, bool>
verify(I1 it, S1 last, SegI2 s_it, SegI2 s_last, bc::segmented_iterator_tag)
{
   typedef bc::segmented_iterator_traits<SegI2>     ndl_traits;
   typedef typename ndl_traits::local_iterator      local_iterator;
   typedef typename ndl_traits::segment_iterator    segment_iterator;
   typedef typename bc::segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef bc::segduo<I1, bool>                     result_t;

   segment_iterator       sfirst = ndl_traits::segment(s_it);
   const segment_iterator slast  = ndl_traits::segment(s_last);
   local_iterator         lb     = ndl_traits::local(s_it);

   for(;;) {
      const bool last_seg = sfirst == slast;
      const local_iterator le = last_seg ? ndl_traits::local(s_last) : ndl_traits::end(sfirst);
      if(lb != le) {
         if(BOOST_UNLIKELY(it == last))
            return result_t(it, false);
         const result_t r = (verify)(it, last, lb, le, is_local_seg_t());
         if(!r.second)
            return r;
         it = r.first;
      }
      if(BOOST_UNLIKELY(last_seg))
         return result_t(it, true);

      for(++sfirst; sfirst != slast; ++sfirst) {
         const local_iterator mb = ndl_traits::begin(sfirst);
         const local_iterator me = ndl_traits::end(sfirst);
         if(mb == me)
            continue;
         if(BOOST_UNLIKELY(it == last))
            return result_t(it, false);
         const result_t r = (verify)(it, last, mb, me, is_local_seg_t());
         if(!r.second)
            return r;
         it = r.first;
      }
      lb = ndl_traits::begin(sfirst);
   }
}

template <class I1, class S1, class I2, class S2>
I1 search(I1 first, S1 last, I2 s_first, S2 s_last)
{
   if(BOOST_UNLIKELY(s_first == s_last)) return first;
   typedef typename bc::segmented_iterator_traits<I2>::is_segmented_iterator ndl_tag;
   da::equal_to_deref<I2> eq(s_first);
   while(first != last) {
      first = bc::segmented_find_if(first, last, eq);
      if(first == last) return last;
      I1 it = first; ++it;
      I2 s_it = s_first; ++s_it;
      if(s_it == s_last) return first;
      if(it == last)     return last;
      const bc::segduo<I1, bool> r = (verify)(it, last, s_it, s_last, ndl_tag());
      if(r.second)        return first;
      if(r.first == last) return last;
      ++first;
   }
   return last;
}

} // namespace arm_own

//---------------------------------------------------------------------------
// arm: ownc  (random-access clamp)
//---------------------------------------------------------------------------
namespace arm_ownc {

template <class I1, class I2>
I1 search(I1 first, I1 last, I2 s_first, I2 s_last)
{
   typedef typename bc::iterator_traits<I1>::difference_type diff_t;
   typedef typename bc::segmented_iterator_traits<I2>::is_segmented_iterator ndl_tag;

   const diff_t n2 = diff_t(s_last - s_first);
   if(BOOST_UNLIKELY(!n2))     return first;
   if((last - first) < n2)     return last;

   const I1 scan_last = last - (n2 - 1);
   da::equal_to_deref<I2> eq(s_first);

   while(first != scan_last) {
      first = bc::segmented_find_if(first, scan_last, eq);
      if(first == scan_last) return last;
      I1 it = first; ++it;
      I2 s_it = s_first; ++s_it;
      if(s_it == s_last) return first;
      const bc::segduo<I1, bool> r = (arm_own::verify)
         (it, bc::unreachable_sentinel_t(), s_it, s_last, ndl_tag());
      if(r.second) return first;
      ++first;
   }
   return last;
}

} // namespace arm_ownc

//---------------------------------------------------------------------------
// arm: ownr  (like own, but the haystack cursor travels by reference so the
//             verify returns a bare bool instead of a fat segduo)
//---------------------------------------------------------------------------
namespace arm_ownr {

template <class I1, class S1, class I2, class S2, class NdlTag>
BOOST_CONTAINER_FORCEINLINE
typename da::algo_enable_if_c
   <!NdlTag::value || bc::is_sentinel<S2, I2>::value, bool>::type
verify(I1 &it, S1 last, I2 s_it, S2 s_last, NdlTag)
{
   for(;;) {
      if(!(*it == *s_it))
         return false;
      ++it;
      ++s_it;
      if(s_it == s_last)
         return true;
      if(it == last)
         return false;
   }
}

template <class I1, class S1, class SegI2>
bool verify(I1 &it, S1 last, SegI2 s_it, SegI2 s_last, bc::segmented_iterator_tag)
{
   typedef bc::segmented_iterator_traits<SegI2>     ndl_traits;
   typedef typename ndl_traits::local_iterator      local_iterator;
   typedef typename ndl_traits::segment_iterator    segment_iterator;
   typedef typename bc::segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;

   segment_iterator       sfirst = ndl_traits::segment(s_it);
   const segment_iterator slast  = ndl_traits::segment(s_last);
   local_iterator         lb     = ndl_traits::local(s_it);

   for(;;) {
      const bool last_seg = sfirst == slast;
      const local_iterator le = last_seg ? ndl_traits::local(s_last) : ndl_traits::end(sfirst);
      if(lb != le) {
         if(BOOST_UNLIKELY(it == last))
            return false;
         if(!(verify)(it, last, lb, le, is_local_seg_t()))
            return false;
      }
      if(BOOST_UNLIKELY(last_seg))
         return true;

      for(++sfirst; sfirst != slast; ++sfirst) {
         const local_iterator mb = ndl_traits::begin(sfirst);
         const local_iterator me = ndl_traits::end(sfirst);
         if(mb == me)
            continue;
         if(BOOST_UNLIKELY(it == last))
            return false;
         if(!(verify)(it, last, mb, me, is_local_seg_t()))
            return false;
      }
      lb = ndl_traits::begin(sfirst);
   }
}

template <class I1, class S1, class I2, class S2>
I1 search(I1 first, S1 last, I2 s_first, S2 s_last)
{
   if(BOOST_UNLIKELY(s_first == s_last)) return first;
   typedef typename bc::segmented_iterator_traits<I2>::is_segmented_iterator ndl_tag;
   da::equal_to_deref<I2> eq(s_first);
   while(first != last) {
      first = bc::segmented_find_if(first, last, eq);
      if(first == last) return last;
      I1 it = first; ++it;
      I2 s_it = s_first; ++s_it;
      if(s_it == s_last) return first;
      if(it == last)     return last;
      if((verify)(it, last, s_it, s_last, ndl_tag())) return first;
      if(it == last) return last;
      ++first;
   }
   return last;
}

} // namespace arm_ownr

//---------------------------------------------------------------------------
// arm: ownrc  (ownr + random-access clamp; the verify then runs with an
//              unreachable haystack end, so its bound test folds away)
//---------------------------------------------------------------------------
namespace arm_ownrc {

template <class I1, class I2>
I1 search(I1 first, I1 last, I2 s_first, I2 s_last)
{
   typedef typename bc::iterator_traits<I1>::difference_type diff_t;
   typedef typename bc::segmented_iterator_traits<I2>::is_segmented_iterator ndl_tag;

   const diff_t n2 = diff_t(s_last - s_first);
   if(BOOST_UNLIKELY(!n2))     return first;
   if((last - first) < n2)     return last;

   const I1 scan_last = last - (n2 - 1);
   da::equal_to_deref<I2> eq(s_first);

   while(first != scan_last) {
      first = bc::segmented_find_if(first, scan_last, eq);
      if(first == scan_last) return last;
      I1 it = first; ++it;
      I2 s_it = s_first; ++s_it;
      if(s_it == s_last) return first;
      if((arm_ownr::verify)(it, bc::unreachable_sentinel_t(), s_it, s_last, ndl_tag()))
         return first;
      ++first;
   }
   return last;
}

} // namespace arm_ownrc

//---------------------------------------------------------------------------
// arm: owns  (haystack walked segment by segment as well)
//---------------------------------------------------------------------------
namespace arm_owns {

template <class I1, class S1, class I2, class S2, class HayTag>
BOOST_CONTAINER_FORCEINLINE
typename da::algo_enable_if_c
   <!HayTag::value || bc::is_sentinel<S1, I1>::value, bc::segtrio<I1, I2, bool> >::type
verify(I1 it, S1 last, I2 s_it, S2 s_last, HayTag)
{
   typedef bc::segtrio<I1, I2, bool> result_t;
   for(;;) {
      if(!(*it == *s_it))
         return result_t(it, s_it, false);
      ++it;
      ++s_it;
      if(s_it == s_last)
         return result_t(it, s_it, true);
      if(it == last)
         return result_t(it, s_it, false);
   }
}

template <class SegI1, class I2, class S2>
bc::segtrio<SegI1, I2, bool>
verify(SegI1 it, SegI1 last, I2 s_it, S2 s_last, bc::segmented_iterator_tag)
{
   typedef bc::segmented_iterator_traits<SegI1>     traits;
   typedef typename traits::local_iterator          local_iterator;
   typedef typename traits::segment_iterator        segment_iterator;
   typedef typename bc::segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef bc::segtrio<SegI1, I2, bool>             result_t;
   typedef bc::segtrio<local_iterator, I2, bool>    local_result_t;

   segment_iterator       sfirst = traits::segment(it);
   const segment_iterator slast  = traits::segment(last);
   local_iterator         lb     = traits::local(it);

   for(;;) {
      const bool last_seg = sfirst == slast;
      const local_iterator le = last_seg ? traits::local(last) : traits::end(sfirst);
      if(lb != le) {
         const local_result_t r = (verify)(lb, le, s_it, s_last, is_local_seg_t());
         s_it = r.second;
         if(r.third || r.first != le)
            return result_t(traits::compose(sfirst, r.first), s_it, r.third);
      }
      if(BOOST_UNLIKELY(last_seg))
         return result_t(last, s_it, false);

      for(++sfirst; sfirst != slast; ++sfirst) {
         const local_iterator mb = traits::begin(sfirst);
         const local_iterator me = traits::end(sfirst);
         if(mb == me)
            continue;
         const local_result_t r = (verify)(mb, me, s_it, s_last, is_local_seg_t());
         s_it = r.second;
         if(r.third || r.first != me)
            return result_t(traits::compose(sfirst, r.first), s_it, r.third);
      }
      lb = traits::begin(sfirst);
   }
}

template <class I1, class S1, class I2, class S2>
I1 search(I1 first, S1 last, I2 s_first, S2 s_last)
{
   if(BOOST_UNLIKELY(s_first == s_last)) return first;
   typedef typename bc::segmented_iterator_traits<I1>::is_segmented_iterator hay_tag;
   da::equal_to_deref<I2> eq(s_first);
   while(first != last) {
      first = bc::segmented_find_if(first, last, eq);
      if(first == last) return last;
      I1 it = first; ++it;
      I2 s_it = s_first; ++s_it;
      if(s_it == s_last) return first;
      if(it == last)     return last;
      const bc::segtrio<I1, I2, bool> r = (verify)(it, last, s_it, s_last, hay_tag());
      if(r.third)         return first;
      if(r.first == last) return last;
      ++first;
   }
   return last;
}

} // namespace arm_owns

//---------------------------------------------------------------------------

template <class F>
static double time_ns(F f, std::size_t iters, std::size_t n)
{
   cpu_timer t;
   t.resume();
   for(std::size_t i = 0; i != iters; ++i) f();
   t.stop();
   return double(t.elapsed()) / double(iters * n);
}

static const std::size_t N = 100000;

template <class C>
static void fill_data(C &c, std::size_t period)
{
   c.clear();
   for(std::size_t i = 0; i != N; ++i)
      c.push_back(period ? int(i % period) : int(i));
}

template <class C>
static void run(const char *kind, const char *what, const C &c, std::size_t period,
                std::size_t nl, std::size_t iters)
{
   typedef typename C::const_iterator cit_t;
   std::vector<int> pat;
   for(std::size_t k = 0; k != nl; ++k)
      pat.push_back(period ? int(k % period) : int(N/2 + k));
   pat[nl-1] = -12345;
   const int *pb = &pat[0];
   const int *pe = pb + nl;

   //Correctness cross-check: every arm must agree with flat.
   {
      const cit_t rf = arm_flat::search (c.begin(), c.end(), pb, pe);
      const bool ok = (arm_mm::search  (c.begin(), c.end(), pb, pe) == rf)
                   && (arm_own::search (c.begin(), c.end(), pb, pe) == rf)
                   && (arm_ownc::search(c.begin(), c.end(), pb, pe) == rf)
                   && (arm_ownr::search(c.begin(), c.end(), pb, pe) == rf)
                   && (arm_ownrc::search(c.begin(), c.end(), pb, pe) == rf)
                   && (arm_owns::search(c.begin(), c.end(), pb, pe) == rf)
                   && (bc::segmented_search(c.begin(), c.end(), pb, pe) == rf);
      if(!ok) std::printf("  *** ARM DISAGREEMENT %s %s per=%zu ndl=%zu\n", kind, what, period, nl);
   }

   const double f = time_ns([&]{ cit_t r = arm_flat::search (c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);
   const double m = time_ns([&]{ cit_t r = arm_mm::search   (c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);
   const double o = time_ns([&]{ cit_t r = arm_own::search  (c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);
   const double q = time_ns([&]{ cit_t r = arm_ownc::search (c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);
   const double s = time_ns([&]{ cit_t r = arm_owns::search (c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);
   const double b = time_ns([&]{ cit_t r = arm_ownr::search (c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);
   const double d = time_ns([&]{ cit_t r = arm_ownrc::search(c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);
   const double l = time_ns([&]{ cit_t r = bc::segmented_search(c.begin(), c.end(), pb, pe);
                                 sink = (r==c.end())?0:1; }, iters, N);

   std::printf("ROW %s|%s|per=%zu|ndl=%zu flat=%.4f | mm/flat=%.3f own/flat=%.3f ownc/flat=%.3f owns/flat=%.3f ownr/flat=%.3f ownrc/flat=%.3f lib/flat=%.3f\n",
               kind, what, period, nl, f, m/f, o/f, q/f, s/f, b/f, d/f, l/f);
}

int main()
{
   typedef bc::vector<int> vec_t;
   typedef bc::deque<int>  dq_t;

   { vec_t v; fill_data(v,0);  run("nsg","sparse",v,0,3,200); run("nsg","sparse",v,0,8,200);
                               run("nsg","sparse",v,0,32,200); }
   { vec_t v; fill_data(v,64); run("nsg","dense64",v,64,3,200); }
   { vec_t v; fill_data(v,4);  run("nsg","dense4",v,4,3,200);  run("nsg","dense4",v,4,8,200); }
   { vec_t v; fill_data(v,1);  run("nsg","everypos",v,1,3,100); run("nsg","everypos",v,1,8,100);
                               run("nsg","everypos",v,1,32,100); }

   { dq_t d; fill_data(d,0);  run("seg","sparse",d,0,3,100); run("seg","sparse",d,0,8,100); }
   { dq_t d; fill_data(d,64); run("seg","dense64",d,64,3,100); }
   { dq_t d; fill_data(d,4);  run("seg","dense4",d,4,3,100); run("seg","dense4",d,4,8,100); }
   { dq_t d; fill_data(d,1);  run("seg","everypos",d,1,3,50); run("seg","everypos",d,1,8,50);
                              run("seg","everypos",d,1,32,50); }
   return 0;
}
