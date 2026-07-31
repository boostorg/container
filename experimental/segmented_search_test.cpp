//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_search.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_search_found()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   int pattern[] = {3, 4, 5};
   test_detail::seg_vector<int>::iterator it =
      segmented_search(sv.begin(), sv.end(), pattern, pattern + 3);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_search_empty_pattern()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   int* empty = a;
   test_detail::seg_vector<int>::iterator it =
      segmented_search(sv.begin(), sv.end(), empty, empty);
   BOOST_TEST(it == sv.begin());
}

void test_search_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> v(src, src + 5);
   int pattern[] = {2, 3, 4};
   boost::container::vector<int>::iterator it = segmented_search(v.begin(), v.end(), pattern, pattern + 3);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   int pattern[] = {3, 4, 5};
   test_detail::seg_vector<int>::iterator it =
      segmented_search(sv.begin(), test_detail::make_sentinel(sv.end()),
                       pattern, test_detail::make_sentinel(pattern + 3));
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_search_sentinel_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> v(src, src + 5);
   int pattern[] = {2, 3, 4};
   boost::container::vector<int>::iterator it =
      segmented_search(v.begin(), test_detail::make_sentinel(v.end()),
                       pattern, test_detail::make_sentinel(pattern + 3));
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int pattern[] = {4, 5, 6};
   test_detail::seg2_vector<int>::iterator it =
      segmented_search(sv2.begin(), sv2.end(), pattern, pattern + 3);
   BOOST_TEST(it != sv2.end());
   BOOST_TEST_EQ(*it, 4);
}

void test_search_every_position()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   int a3[] = {60, 70, 80, 90};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   int vals[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
   const int N = 9;
   typedef test_detail::seg_vector<int>::iterator iter_t;

   iter_t expected = sv.begin();
   for(int i = 0; i < N; ++i, ++expected) {
      iter_t it = segmented_search(sv.begin(), sv.end(), vals + i, vals + i + 1);
      BOOST_TEST(it != sv.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }

   int notfound = 999;
   BOOST_TEST(segmented_search(sv.begin(), sv.end(), &notfound, &notfound + 1) == sv.end());
}

void test_search_every_position_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   int a3[] = {60, 70, 80, 90};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int vals[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
   const int N = 9;
   typedef test_detail::seg2_vector<int>::iterator iter_t;

   iter_t expected = sv2.begin();
   for(int i = 0; i < N; ++i, ++expected) {
      iter_t it = segmented_search(sv2.begin(), sv2.end(), vals + i, vals + i + 1);
      BOOST_TEST(it != sv2.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }

   int notfound = 999;
   BOOST_TEST(segmented_search(sv2.begin(), sv2.end(), &notfound, &notfound + 1) == sv2.end());
}

//----------------------------------------------------------------------------
// Tests where the needle is itself a segmented iterator. These exercise the
// dual-segmentation path in segmented_search_bounded_mismatch.
//----------------------------------------------------------------------------

// Search every single-element segmented needle position against a segmented
// haystack, mirroring test_search_every_position but with a segmented needle.
void test_search_every_position_segmented_needle()
{
   test_detail::seg_vector<int> hay;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   int a3[] = {60, 70, 80, 90};
   hay.add_segment_range(a1, a1 + 3);
   hay.add_segment_range(a2, a2 + 2);
   hay.add_segment_range(a3, a3 + 4);

   int vals[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
   const int N = 9;
   typedef test_detail::seg_vector<int>::iterator iter_t;

   iter_t expected = hay.begin();
   for(int i = 0; i < N; ++i, ++expected) {
      test_detail::seg_vector<int> ndl;
      ndl.add_segment_range(vals + i, vals + i + 1);
      iter_t it = segmented_search(hay.begin(), hay.end(), ndl.begin(), ndl.end());
      BOOST_TEST(it != hay.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }

   test_detail::seg_vector<int> notfound;
   int nf = 999;
   notfound.add_segment_range(&nf, &nf + 1);
   BOOST_TEST(segmented_search(hay.begin(), hay.end(),
                               notfound.begin(), notfound.end()) == hay.end());
}

// Non-segmented haystack with a segmented needle.
void test_search_flat_haystack_segmented_needle()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> hay(src, src + 5);

   test_detail::seg_vector<int> ndl;
   int p1[] = {2, 3};
   int p2[] = {4};
   ndl.add_segment_range(p1, p1 + 2);
   ndl.add_segment_range(p2, p2 + 1);

   boost::container::vector<int>::iterator it =
      segmented_search(hay.begin(), hay.end(), ndl.begin(), ndl.end());
   BOOST_TEST(it != hay.end());
   BOOST_TEST_EQ(*it, 2);
}

// Sentinel on both segmented ranges.
void test_search_segmented_both_sentinel()
{
   test_detail::seg_vector<int> hay;
   int h1[] = {1, 2, 3};
   int h2[] = {4, 5, 6};
   hay.add_segment_range(h1, h1 + 3);
   hay.add_segment_range(h2, h2 + 3);

   test_detail::seg_vector<int> ndl;
   int p1[] = {3, 4};
   int p2[] = {5};
   ndl.add_segment_range(p1, p1 + 2);
   ndl.add_segment_range(p2, p2 + 1);

   test_detail::seg_vector<int>::iterator it =
      segmented_search(hay.begin(), test_detail::make_sentinel(hay.end()),
                       ndl.begin(), test_detail::make_sentinel(ndl.end()));
   BOOST_TEST(it != hay.end());
   BOOST_TEST_EQ(*it, 3);
}

// Runs segmented_search over a pair of ranges whose segmentation shapes are
// dictated by two independent branch specs, so that the haystack walk and the
// needle walk are each exercised on their single-segment, multi-segment and
// empty-segment paths, in every combination of the two.
struct search_shape_check
{
   template<class C1, class C2>
   void operator()(C1& hay, std::size_t n1, const char* s1,
                   C2& ndl, std::size_t n2, const char* s2) const
   {
      typedef typename C1::iterator hay_iter_t;
      const hay_iter_t hfirst = hay.begin();
      const hay_iter_t hlast  = test_detail::iter_at(hay, n1);
      const typename C2::iterator nfirst = ndl.begin();
      const typename C2::iterator nlast  = test_detail::iter_at(ndl, n2);

      // Reference: naive search over flattened copies of the two logical
      // ranges. Neither guard is part of them, so reading past the end of
      // either side turns a hit into a miss or the other way round.
      const boost::container::vector<int> h = test_detail::flatten_n_ints(hay, n1);
      const boost::container::vector<int> p = test_detail::flatten_n_ints(ndl, n2);

      std::size_t expected = h.size();
      if(p.empty()) {
         expected = 0;     // an empty needle matches at the start, per std::search
      }
      else if(p.size() <= h.size()) {
         for(std::size_t i = 0; i + p.size() <= h.size(); ++i) {
            std::size_t j = 0;
            for(; j != p.size(); ++j) {
               if(h[i + j] != p[j]) break;
            }
            if(j == p.size()) { expected = i; break; }
         }
      }

      const hay_iter_t r = segmented_search(hfirst, hlast, nfirst, nlast);
      BOOST_TEST(r == test_detail::iter_at(hay, expected));
      BOOST_TEST(s1 != 0 && s2 != 0);
   }
};

void run_search_shapes(const int* hay, std::size_t n1, const int* ndl, std::size_t n2)
{
   test_detail::for_each_shape2_all<int, int>(hay, n1, ndl, n2, -999, search_shape_check());
}

// Needles drawn from one haystack layout: every length up to four (plus one
// longer than the haystack itself) at every offset, then the same needles made
// unmatchable, either wholly or only in their last element, which is what
// forces the search past a false start and on to the next candidate.
void run_search_needles(const int* hay, std::size_t n1)
{
   const std::size_t maxlen = (n1 + 1u < 4u) ? n1 + 1u : 4u;
   int ndl[16] = { 0 };
   std::size_t j = 0;

   run_search_shapes(hay, n1, ndl, 0u);

   for(std::size_t len = 1u; len <= maxlen; ++len) {
      // Present at every offset it fits at, including offset 0 and the offset
      // that makes the needle end exactly at the end of the haystack.
      for(std::size_t off = 0; off + len <= n1; ++off) {
         for(j = 0; j != len; ++j)
            ndl[j] = hay[off + j];
         run_search_shapes(hay, n1, ndl, len);

         // The same needle with its last element replaced by one the haystack
         // does not contain: every candidate start is a false start.
         ndl[len - 1u] = 500;
         run_search_shapes(hay, n1, ndl, len);
      }
      // Nothing of the needle occurs in the haystack at all.
      for(j = 0; j != len; ++j)
         ndl[j] = 500 + int(j);
      run_search_shapes(hay, n1, ndl, len);
   }

   // A needle one element longer than the whole haystack: it agrees with the
   // haystack everywhere and still must not match.
   for(j = 0; j != n1; ++j)
      ndl[j] = hay[j];
   ndl[n1] = 500;
   run_search_shapes(hay, n1, ndl, n1 + 1u);
}

void test_search_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 8u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n1 = sizes[s];
      int hay[16] = { 0 };
      std::size_t i = 0;

      // All distinct: at most one candidate start per needle.
      for(i = 0; i != n1; ++i)
         hay[i] = int(i) + 1;
      run_search_needles(hay, n1);

      // Periodic: many candidate starts per needle, so the earliest match has
      // to win and every false start before it has to be rejected.
      for(i = 0; i != n1; ++i)
         hay[i] = int(i % 3u) + 1;
      run_search_needles(hay, n1);
   }
}

//----------------------------------------------------------------------------
// Single-segment cases with data before the start of the range. The shape
// matrix always starts both of its ranges at the container's first element, so
// the lower bounds are only exercised here. Haystack and needle are walked by
// separate dispatch functions, so each side gets its own coverage; every
// haystack keeps elements just outside both bounds that would complete a
// match, so an off-by-one on either side turns a miss into a hit.
//----------------------------------------------------------------------------

void test_search_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, 60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   int at_start[]    = {20, 30};
   int inside[]      = {30, 40};
   int at_end[]      = {40, 50};
   int absent[]      = {30, 50};
   int before_start[] = {10, 20};
   int past_end[]    = {50, 60};

   BOOST_TEST(segmented_search(first, last, at_start, at_start + 2) == first);
   BOOST_TEST(segmented_search(first, last, inside, inside + 2) == test_detail::iter_at(sv, 2));
   BOOST_TEST(segmented_search(first, last, at_end, at_end + 2) == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_search(first, last, absent, absent + 2) == last);
   BOOST_TEST(segmented_search(first, last, before_start, before_start + 2) == last);
   BOOST_TEST(segmented_search(first, last, past_end, past_end + 2) == last);
   BOOST_TEST(segmented_search(first, last, at_start, at_start) == first);
}

void test_search_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, 60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   int inside[]   = {30, 40};
   int past_end[] = {50, 60};

   BOOST_TEST(segmented_search(first, test_detail::make_sentinel(last),
                               inside, test_detail::make_sentinel(inside + 2))
              == test_detail::iter_at(sv, 2));
   BOOST_TEST(segmented_search(first, test_detail::make_sentinel(last),
                               past_end, test_detail::make_sentinel(past_end + 2)) == last);
}

void test_search_single_segment_seg2_inner_multi()
{
   test_detail::seg_vector<int> inner;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50, 60, 70};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 4);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 6);

   int at_start[] = {20, 30};
   int straddle[] = {30, 40};
   int at_end[]   = {50, 60};
   int past_end[] = {60, 70};

   BOOST_TEST(segmented_search(first, last, at_start, at_start + 2) == first);
   BOOST_TEST(segmented_search(first, last, straddle, straddle + 2) == test_detail::iter_at(sv2, 2));
   BOOST_TEST(segmented_search(first, last, at_end, at_end + 2) == test_detail::iter_at(sv2, 4));
   BOOST_TEST(segmented_search(first, last, past_end, past_end + 2) == last);
}

void test_search_single_segment_seg2_single_inner()
{
   test_detail::seg_vector<int> inner;
   int a[] = {10, 20, 30, 40, 50, 60, 70};
   inner.add_segment_range(a, a + 7);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 5);

   int at_start[] = {20, 30};
   int inside[]   = {30, 40};
   int at_end[]   = {40, 50};
   int absent[]   = {30, 50};
   int past_end[] = {50, 60};

   BOOST_TEST(segmented_search(first, last, at_start, at_start + 2) == first);
   BOOST_TEST(segmented_search(first, last, inside, inside + 2) == test_detail::iter_at(sv2, 2));
   BOOST_TEST(segmented_search(first, last, at_end, at_end + 2) == test_detail::iter_at(sv2, 3));
   BOOST_TEST(segmented_search(first, last, absent, absent + 2) == last);
   BOOST_TEST(segmented_search(first, last, past_end, past_end + 2) == last);
}

// Single-segment segmented needle against a multi-segment haystack: the needle
// is walked by its own dispatch, so its segmentation matters independently.
void test_search_single_segment_needle()
{
   test_detail::seg_vector<int> hay;
   int h1[] = {10, 20, 30};
   int h2[] = {40, 50};
   int h3[] = {60, 70, 80};
   hay.add_segment_range(h1, h1 + 3);
   hay.add_segment_range(h2, h2 + 2);
   hay.add_segment_range(h3, h3 + 3);

   //Needle sub-range strictly inside a single needle segment.
   test_detail::seg_vector<int> ndl;
   int p[] = {99, 30, 40, 50, 98};
   ndl.add_segment_range(p, p + 5);

   BOOST_TEST(segmented_search(hay.begin(), hay.end(),
                               test_detail::iter_at(ndl, 1), test_detail::iter_at(ndl, 4))
              == test_detail::iter_at(hay, 2));

   //The same needle grown by one element on each side no longer matches.
   BOOST_TEST(segmented_search(hay.begin(), hay.end(),
                               ndl.begin(), test_detail::iter_at(ndl, 4)) == hay.end());
   BOOST_TEST(segmented_search(hay.begin(), hay.end(),
                               test_detail::iter_at(ndl, 1), ndl.end()) == hay.end());
}

// Single-segment haystack against a multi-segment segmented needle, and the
// both-single-segment combination.
void test_search_single_segment_haystack_and_needle()
{
   test_detail::seg_vector<int> hay;
   int h[] = {10, 20, 30, 40, 50, 60, 70};
   hay.add_segment_range(h, h + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(hay, 1);
   const iter_t last  = test_detail::iter_at(hay, 6);

   //Multi-segment needle, single-segment haystack.
   test_detail::seg_vector<int> multi;
   int m1[] = {99, 30};
   int m2[] = {40, 50, 98};
   multi.add_segment_range(m1, m1 + 2);
   multi.add_segment_range(m2, m2 + 3);
   BOOST_TEST(segmented_search(first, last,
                               test_detail::iter_at(multi, 1), test_detail::iter_at(multi, 4))
              == test_detail::iter_at(hay, 2));

   //Single-segment needle, single-segment haystack.
   test_detail::seg_vector<int> single;
   int s[] = {99, 40, 50, 98};
   single.add_segment_range(s, s + 4);
   BOOST_TEST(segmented_search(first, last,
                               test_detail::iter_at(single, 1), test_detail::iter_at(single, 3))
              == test_detail::iter_at(hay, 3));

   //A needle reaching past the haystack's end must not match.
   test_detail::seg_vector<int> past;
   int q[] = {60, 70};
   past.add_segment_range(q, q + 2);
   BOOST_TEST(segmented_search(first, last, past.begin(), test_detail::iter_at(past, 2)) == last);
}

// The whole suite instantiates the bidirectional category only; the forward
// one is a different instantiation of every dispatch template.
void test_search_single_segment_forward()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;
   typedef cont_t::iterator iter_t;

   cont_t sv;
   int a[] = {10, 20, 30, 40, 50, 60, 70};
   sv.add_segment_range(a, a + 7);

   int at_start[] = {10, 20};
   int at_end[]   = {50, 60};
   int past_end[] = {60, 70};

   const iter_t whole_last = test_detail::iter_at(sv, 6);
   BOOST_TEST(segmented_search(sv.begin(), whole_last, at_start, at_start + 2) == sv.begin());
   BOOST_TEST(segmented_search(sv.begin(), whole_last, at_end, at_end + 2) == test_detail::iter_at(sv, 4));
   BOOST_TEST(segmented_search(sv.begin(), whole_last, past_end, past_end + 2) == whole_last);

   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);
   int inside[] = {30, 40};
   BOOST_TEST(segmented_search(first, last, inside, inside + 2) == test_detail::iter_at(sv, 2));
   BOOST_TEST(segmented_search(first, last, at_start, at_start + 2) == last);
   BOOST_TEST(segmented_search(first, last, at_end, at_end + 2) == last);
}

// Forward category, multi-segment, with the only match in an earlier segment
// and none in the last one: the last-segment call must not discard it.
void test_search_forward_match_before_last_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;

   cont_t sv;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   int a3[] = {60, 70, 80};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 3);

   int straddle[] = {30, 40};
   int absent[]   = {70, 90};

   BOOST_TEST(segmented_search(sv.begin(), test_detail::iter_at(sv, 7), straddle, straddle + 2)
              == test_detail::iter_at(sv, 2));
   BOOST_TEST(segmented_search(sv.begin(), test_detail::iter_at(sv, 7), absent, absent + 2)
              == test_detail::iter_at(sv, 7));
}

//////////////////////////////////////////////////////////////////////////////
// Comparison count.
//
// [alg.search] mandates "At most (last1 - first1) * (last2 - first2)
// applications of the corresponding predicate".  There is no predicate
// overload, so the count comes from the value type.
//////////////////////////////////////////////////////////////////////////////

struct search_comparison_check
{
   template<class C1, class C2>
   void operator()(C1& hay, std::size_t n1, const char* s1,
                   C2& ndl, std::size_t n2, const char* s2) const
   {
      test_detail::counted_int_ops().reset();
      segmented_search(hay.begin(), test_detail::iter_at(hay, n1),
                       ndl.begin(), test_detail::iter_at(ndl, n2));
      const std::size_t applied = test_detail::counted_int_ops().cmp;

      BOOST_TEST(applied <= n1 * n2);
      BOOST_TEST(s1 != 0 && s2 != 0);
   }
};

void run_search_count_shapes(const int* hay, std::size_t n1, const int* ndl, std::size_t n2)
{
   test_detail::for_each_shape2_all<test_detail::counted_int, test_detail::counted_int>
      (hay, n1, ndl, n2, -999, search_comparison_check());
}

void test_search_comparison_count()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 8u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n1 = sizes[s];
      int hay[16] = { 0 };
      int ndl[16] = { 0 };
      std::size_t i = 0, j = 0;

      // Periodic haystack: many candidate starts per needle, so every false
      // start has to be rejected and the re-comparisons pile up fastest.
      for(i = 0; i != n1; ++i)
         hay[i] = int(i % 3u) + 1;

      const std::size_t maxlen = (n1 + 1u < 4u) ? n1 + 1u : 4u;
      run_search_count_shapes(hay, n1, ndl, 0u);

      for(std::size_t len = 1u; len <= maxlen; ++len) {
         for(std::size_t off = 0; off + len <= n1; ++off) {
            for(j = 0; j != len; ++j)
               ndl[j] = hay[off + j];
            run_search_count_shapes(hay, n1, ndl, len);

            ndl[len - 1u] = 500;
            run_search_count_shapes(hay, n1, ndl, len);
         }
         for(j = 0; j != len; ++j)
            ndl[j] = 500 + int(j);
         run_search_count_shapes(hay, n1, ndl, len);
      }
   }
}

int main()
{
   test_search_shape_matrix();
   test_search_found();
   test_search_empty_pattern();
   test_search_non_segmented();
   test_search_sentinel_segmented();
   test_search_sentinel_non_segmented();
   test_search_seg2();
   test_search_every_position();
   test_search_every_position_seg2();
   //Tests exercising the dual-segmentation path (segmented needle).
   test_search_every_position_segmented_needle();
   test_search_flat_haystack_segmented_needle();
   test_search_segmented_both_sentinel();
   test_search_single_segment_interior();
   test_search_single_segment_sentinel();
   test_search_single_segment_seg2_inner_multi();
   test_search_single_segment_seg2_single_inner();
   test_search_single_segment_needle();
   test_search_single_segment_haystack_and_needle();
   test_search_single_segment_forward();
   test_search_forward_match_before_last_segment();
   test_search_comparison_count();
   return boost::report_errors();
}
