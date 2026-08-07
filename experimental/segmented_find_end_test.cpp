//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_find_end.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>
#include <algorithm>

using namespace boost::container;

//! Explicit predicate for the five-argument overload.  Spelling out the
//! comparison the four-argument one performs implicitly keeps the two
//! overloads answering the same question on every test range.
struct pred_equal
{
   template<class T, class U>
   bool operator()(const T& a, const U& b) const { return a == b; }
};

//! A predicate that is not equality, so that a result obtained with it could
//! not have come from the default comparison.
struct pred_same_units
{
   bool operator()(int a, int b) const { return (a % 10) == (b % 10); }
};

//////////////////////////////////////////////////////////////////////////////
// Hand-written cases
//////////////////////////////////////////////////////////////////////////////

void test_find_end_multiple_occurrences()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 9};
   int a2[] = {1, 2, 9};
   int a3[] = {1, 2, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 3);

   int pattern[] = {1, 2};

   //The whole point of find_end: the third occurrence wins, not the first.
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), pattern, pattern + 2)
              == test_detail::iter_at(sv, 6));
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), pattern, pattern + 2, pred_equal())
              == test_detail::iter_at(sv, 6));
}

void test_find_end_at_beginning_and_end()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {7, 8, 3};
   int a2[] = {4, 5, 7};
   int a3[] = {8, 9, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 3);

   int only_first[] = {7, 8, 3};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), only_first, only_first + 3) == sv.begin());

   int at_the_end[] = {9, 9};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), at_the_end, at_the_end + 2)
              == test_detail::iter_at(sv, 7));

   int absent[] = {3, 5};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), absent, absent + 2) == sv.end());
}

void test_find_end_empty_needle()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   //Unlike search, an empty needle occurs at the end of the range.
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), a, a) == sv.end());
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), a, a, pred_equal()) == sv.end());

   test_detail::seg_vector<int> empty_hay;
   BOOST_TEST(segmented_find_end(empty_hay.begin(), empty_hay.begin(), a, a)
              == empty_hay.begin());
}

void test_find_end_needle_longer_than_haystack()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2};
   int a2[] = {3};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 1);

   //Agrees with the haystack everywhere and still must not match.
   int longer[] = {1, 2, 3, 4};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), longer, longer + 4) == sv.end());
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), longer, longer + 4, pred_equal()) == sv.end());
}

void test_find_end_whole_haystack()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2};
   int a2[] = {3, 4};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   int whole[] = {1, 2, 3, 4};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), whole, whole + 4) == sv.begin());
}

void test_find_end_single_element_needle()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {5, 1, 5};
   int a2[] = {2, 5, 3};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   int five = 5;
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), &five, &five + 1)
              == test_detail::iter_at(sv, 4));

   int absent = 99;
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), &absent, &absent + 1) == sv.end());
}

void test_find_end_straddles_boundaries()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2};
   int a2[] = {3, 4};
   int a3[] = {5, 6};
   int a4[] = {2, 3};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 2);
   sv.add_segment_range(a4, a4 + 2);

   //Two occurrences of a needle that crosses one boundary; the later one
   //crosses a different boundary.
   int pair[] = {2, 3};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), pair, pair + 2)
              == test_detail::iter_at(sv, 6));

   //A needle spanning three segments, so the middle-segment call site of the
   //verify walk is the one that has to carry it.
   int wide[] = {2, 3, 4, 5, 6};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), wide, wide + 5)
              == test_detail::iter_at(sv, 1));

   //The same needle grown by one element past the end no longer matches.
   int wider[] = {2, 3, 4, 5, 6, 2, 3, 9};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), wider, wider + 8) == sv.end());
}

void test_find_end_single_segment()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 20, 30, 60};
   sv.add_segment_range(a, a + 6);

   int pattern[] = {20, 30};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), pattern, pattern + 2)
              == test_detail::iter_at(sv, 3));

   //A sub-range with data on both sides: an off-by-one on either bound turns
   //the answer into a different occurrence.
   const test_detail::seg_vector<int>::iterator first = test_detail::iter_at(sv, 1);
   const test_detail::seg_vector<int>::iterator last  = test_detail::iter_at(sv, 4);
   BOOST_TEST(segmented_find_end(first, last, pattern, pattern + 2) == first);

   int past_end[] = {30, 60};
   BOOST_TEST(segmented_find_end(first, last, past_end, past_end + 2) == last);
}

void test_find_end_single_segment_forward()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;
   typedef cont_t::iterator iter_t;

   cont_t sv;
   int a[] = {10, 20, 30, 20, 30, 60, 70};
   sv.add_segment_range(a, a + 7);

   int pattern[] = {20, 30};
   const iter_t whole_last = test_detail::iter_at(sv, 7);
   BOOST_TEST(segmented_find_end(sv.begin(), whole_last, pattern, pattern + 2)
              == test_detail::iter_at(sv, 3));

   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 4);
   BOOST_TEST(segmented_find_end(first, last, pattern, pattern + 2) == first);

   int past_end[] = {30, 60};
   BOOST_TEST(segmented_find_end(first, last, past_end, past_end + 2) == last);
   BOOST_TEST(segmented_find_end(first, last, pattern, pattern) == last);
}

void test_find_end_forward_multi_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;

   cont_t sv;
   int a1[] = {1, 2, 3};
   int a2[] = {1, 2};
   int a3[] = {3, 4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 3);

   //Two occurrences, the later one straddling a boundary.
   int pattern[] = {2, 3};
   BOOST_TEST(segmented_find_end(sv.begin(), test_detail::iter_at(sv, 8), pattern, pattern + 2)
              == test_detail::iter_at(sv, 4));

   //Only occurrence lives in the first segment: the walk over the later ones
   //must not discard it.
   int early[] = {1, 2, 3, 1};
   BOOST_TEST(segmented_find_end(sv.begin(), test_detail::iter_at(sv, 8), early, early + 4)
              == sv.begin());
}

void test_find_end_non_segmented()
{
   int src[] = {1, 2, 3, 2, 3, 6};
   boost::container::vector<int> v(src, src + 6);
   int pattern[] = {2, 3};
   BOOST_TEST(segmented_find_end(v.begin(), v.end(), pattern, pattern + 2) == v.begin() + 3);
   BOOST_TEST(segmented_find_end(v.begin(), v.end(), pattern, pattern) == v.end());

   int absent[] = {3, 3};
   BOOST_TEST(segmented_find_end(v.begin(), v.end(), absent, absent + 2) == v.end());
}

void test_find_end_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {4, 5, 6};
   int a2[] = {1, 2};
   int a3[] = {4, 5, 6, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int pattern[] = {4, 5, 6};
   BOOST_TEST(segmented_find_end(sv2.begin(), sv2.end(), pattern, pattern + 3)
              == test_detail::iter_at(sv2, 5));

   //Straddling the boundary between two outer segments.
   int straddle[] = {2, 4};
   BOOST_TEST(segmented_find_end(sv2.begin(), sv2.end(), straddle, straddle + 2)
              == test_detail::iter_at(sv2, 4));
}

//! Both ranges two-level segmented: the needle is walked flat, but it still
//! has to survive being handed a recursively segmented iterator.
void test_find_end_seg2_both_ranges()
{
   test_detail::seg2_vector<int> hay;
   int h1[] = {3, 4, 5};
   int h2[] = {1, 2};
   int h3[] = {3, 4, 5};
   hay.add_flat_segment_range(h1, h1 + 3);
   hay.add_flat_segment_range(h2, h2 + 2);
   hay.add_flat_segment_range(h3, h3 + 3);

   test_detail::seg2_vector<int> ndl;
   int p1[] = {3, 4};
   int p2[] = {5};
   ndl.add_flat_segment_range(p1, p1 + 2);
   ndl.add_flat_segment_range(p2, p2 + 1);

   BOOST_TEST(segmented_find_end(hay.begin(), hay.end(), ndl.begin(), ndl.end())
              == test_detail::iter_at(hay, 5));
   BOOST_TEST(segmented_find_end(hay.begin(), hay.end(), ndl.begin(), ndl.begin())
              == hay.end());
}

void test_find_end_single_segment_seg2()
{
   test_detail::seg_vector<int> inner;
   int a[] = {10, 20, 30, 20, 30, 60, 70};
   inner.add_segment_range(a, a + 7);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 5);

   int pattern[] = {20, 30};
   BOOST_TEST(segmented_find_end(sv2.begin(), sv2.end(), pattern, pattern + 2)
              == test_detail::iter_at(sv2, 3));
   BOOST_TEST(segmented_find_end(first, last, pattern, pattern + 2)
              == test_detail::iter_at(sv2, 3));

   int past_end[] = {30, 60};
   BOOST_TEST(segmented_find_end(first, last, past_end, past_end + 2) == last);
}

void test_find_end_segmented_needle()
{
   test_detail::seg_vector<int> hay;
   int h1[] = {10, 20, 30};
   int h2[] = {40, 20};
   int h3[] = {30, 40, 80};
   hay.add_segment_range(h1, h1 + 3);
   hay.add_segment_range(h2, h2 + 2);
   hay.add_segment_range(h3, h3 + 3);

   test_detail::seg_vector<int> ndl;
   int p1[] = {20, 30};
   int p2[] = {40};
   ndl.add_segment_range(p1, p1 + 2);
   ndl.add_segment_range(p2, p2 + 1);

   BOOST_TEST(segmented_find_end(hay.begin(), hay.end(), ndl.begin(), ndl.end())
              == test_detail::iter_at(hay, 4));

   //Flat haystack, segmented needle.
   int src[] = {1, 2, 3, 1, 2, 3};
   boost::container::vector<int> flat(src, src + 6);
   test_detail::seg_vector<int> ndl2;
   int q1[] = {1};
   int q2[] = {2, 3};
   ndl2.add_segment_range(q1, q1 + 1);
   ndl2.add_segment_range(q2, q2 + 2);
   BOOST_TEST(segmented_find_end(flat.begin(), flat.end(), ndl2.begin(), ndl2.end())
              == flat.begin() + 3);
}

void test_find_end_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {1, 2, 3};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   int pattern[] = {2, 3};
   BOOST_TEST(segmented_find_end(sv.begin(), test_detail::make_sentinel(sv.end()),
                                 pattern, test_detail::make_sentinel(pattern + 2))
              == test_detail::iter_at(sv, 4));

   //A sentinel-terminated range still has to name its own end when nothing
   //matches, and when the needle is empty.
   int absent[] = {3, 3};
   BOOST_TEST(segmented_find_end(sv.begin(), test_detail::make_sentinel(sv.end()),
                                 absent, test_detail::make_sentinel(absent + 2)) == sv.end());
   BOOST_TEST(segmented_find_end(sv.begin(), test_detail::make_sentinel(sv.end()),
                                 pattern, test_detail::make_sentinel(pattern)) == sv.end());

   int src[] = {5, 6, 5, 6};
   boost::container::vector<int> v(src, src + 4);
   int pv[] = {5, 6};
   BOOST_TEST(segmented_find_end(v.begin(), test_detail::make_sentinel(v.end()),
                                 pv, test_detail::make_sentinel(pv + 2)) == v.begin() + 2);
}

void test_find_end_explicit_predicate()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {11, 22, 33};
   int a2[] = {41, 52, 63};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   //Matches only under the units-digit comparison, and matches twice, so the
   //answer also proves the predicate reached the backwards scan.
   int pattern[] = {1, 2};
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), pattern, pattern + 2, pred_same_units())
              == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_find_end(sv.begin(), sv.end(), pattern, pattern + 2) == sv.end());
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.  Runs segmented_find_end over a pair of ranges whose
// segmentation shapes are dictated by two independent branch specs, and
// checks every answer against std::find_end over flattened copies of the two
// logical ranges.  Neither guard element is part of them, so reading past the
// end of either side turns a hit into a miss or the other way round.
//////////////////////////////////////////////////////////////////////////////

struct find_end_shape_check
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

      const boost::container::vector<int> h = test_detail::flatten_n_ints(hay, n1);
      const boost::container::vector<int> p = test_detail::flatten_n_ints(ndl, n2);

      const std::size_t expected = std::size_t
         (std::find_end(h.begin(), h.end(), p.begin(), p.end()) - h.begin());
      const hay_iter_t want = test_detail::iter_at(hay, expected);

      BOOST_TEST(segmented_find_end(hfirst, hlast, nfirst, nlast) == want);
      BOOST_TEST(segmented_find_end(hfirst, hlast, nfirst, nlast, pred_equal()) == want);

      //A sentinel end downgrades the range to forward-only, which is the only
      //way the forward scan is reached from a bidirectional container.
      BOOST_TEST(segmented_find_end(hfirst, test_detail::make_sentinel(hlast),
                                    nfirst, test_detail::make_sentinel(nlast)) == want);

      BOOST_TEST(s1 != 0 && s2 != 0);
   }
};

void run_find_end_shapes(const int* hay, std::size_t n1, const int* ndl, std::size_t n2)
{
   test_detail::for_each_shape2_all<int, int>(hay, n1, ndl, n2, -999, find_end_shape_check());
}

//! Needles drawn from one haystack layout: every length up to four (plus one
//! longer than the haystack itself) at every offset, then the same needles
//! made unmatchable, either wholly or only in their last element, which is
//! what forces the scan past a false start and on to the next candidate.
void run_find_end_needles(const int* hay, std::size_t n1)
{
   const std::size_t maxlen = (n1 + 1u < 4u) ? n1 + 1u : 4u;
   int ndl[16] = { 0 };
   std::size_t j = 0;

   run_find_end_shapes(hay, n1, ndl, 0u);

   for(std::size_t len = 1u; len <= maxlen; ++len) {
      for(std::size_t off = 0; off + len <= n1; ++off) {
         for(j = 0; j != len; ++j)
            ndl[j] = hay[off + j];
         run_find_end_shapes(hay, n1, ndl, len);

         ndl[len - 1u] = 500;
         run_find_end_shapes(hay, n1, ndl, len);
      }
      for(j = 0; j != len; ++j)
         ndl[j] = 500 + int(j);
      run_find_end_shapes(hay, n1, ndl, len);
   }

   //A needle one element longer than the whole haystack: it agrees with the
   //haystack everywhere and still must not match.
   for(j = 0; j != n1; ++j)
      ndl[j] = hay[j];
   ndl[n1] = 500;
   run_find_end_shapes(hay, n1, ndl, n1 + 1u);
}

void test_find_end_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 8u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n1 = sizes[s];
      int hay[16] = { 0 };
      std::size_t i = 0;

      //All distinct: at most one candidate start per needle.
      for(i = 0; i != n1; ++i)
         hay[i] = int(i) + 1;
      run_find_end_needles(hay, n1);

      //Periodic: many candidate starts per needle, so the latest match has to
      //win and every false start after it has to be rejected.
      for(i = 0; i != n1; ++i)
         hay[i] = int(i % 3u) + 1;
      run_find_end_needles(hay, n1);
   }
}

//! Forward-category haystacks over the same needle sweep.  The forward scan is
//! a different instantiation of every dispatch template, and it is the one
//! that has to remember a match instead of stopping at it.
void test_find_end_forward_matrix()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;

   const std::size_t n1 = 8u;
   int hay[16] = { 0 };
   std::size_t i = 0, off = 0, j = 0;
   for(i = 0; i != n1; ++i)
      hay[i] = int(i % 3u) + 1;

   const char* const specs[] = { "s", "m", "e" };
   for(std::size_t sp = 0; sp != sizeof(specs)/sizeof(specs[0]); ++sp) {
      for(std::size_t len = 0u; len <= 4u; ++len) {
         for(off = 0; off + len <= n1; ++off) {
            int ndl[8] = { 0 };
            for(j = 0; j != len; ++j)
               ndl[j] = hay[off + j];

            cont_t c;
            test_detail::make_range(c, specs[sp], hay, n1, -999);

            const boost::container::vector<int> h = test_detail::flatten_n_ints(c, n1);
            const std::size_t expected = std::size_t
               (std::find_end(h.begin(), h.end(), ndl, ndl + len) - h.begin());

            BOOST_TEST(segmented_find_end(c.begin(), test_detail::iter_at(c, n1), ndl, ndl + len)
                       == test_detail::iter_at(c, expected));
         }
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Comparison count.
//
// [alg.find.end] mandates "At most S * (N - S + 1) applications of the
// corresponding predicate", with S the needle length and N the length of the
// searched range.  A needle that does not fit costs nothing at all, since the
// scan never starts.
//////////////////////////////////////////////////////////////////////////////

inline std::size_t find_end_bound(std::size_t n1, std::size_t n2)
{  return (n2 == 0u || n2 > n1) ? 0u : n2 * (n1 - n2 + 1u);   }

struct find_end_comparison_check
{
   template<class C1, class C2>
   void operator()(C1& hay, std::size_t n1, const char* s1,
                   C2& ndl, std::size_t n2, const char* s2) const
   {
      const std::size_t bound = find_end_bound(n1, n2);

      test_detail::counted_int_ops().reset();
      segmented_find_end(hay.begin(), test_detail::iter_at(hay, n1),
                         ndl.begin(), test_detail::iter_at(ndl, n2));
      BOOST_TEST(test_detail::counted_int_ops().cmp <= bound);

      //The sentinel form routes through the forward scan, which visits every
      //candidate start and so is the harder of the two to keep in budget.
      test_detail::counted_int_ops().reset();
      segmented_find_end(hay.begin(), test_detail::make_sentinel(test_detail::iter_at(hay, n1)),
                         ndl.begin(), test_detail::make_sentinel(test_detail::iter_at(ndl, n2)));
      BOOST_TEST(test_detail::counted_int_ops().cmp <= bound);

      //The explicit-predicate overload is counted outside the value type, so
      //it also proves the predicate is applied exactly where equality was.
      test_detail::op_counter counter;
      segmented_find_end(hay.begin(), test_detail::iter_at(hay, n1),
                         ndl.begin(), test_detail::iter_at(ndl, n2),
                         test_detail::counting_pred(counter, pred_equal()));
      BOOST_TEST(counter.n <= bound);

      BOOST_TEST(s1 != 0 && s2 != 0);
   }
};

void run_find_end_count_shapes(const int* hay, std::size_t n1, const int* ndl, std::size_t n2)
{
   test_detail::for_each_shape2_all<test_detail::counted_int, test_detail::counted_int>
      (hay, n1, ndl, n2, -999, find_end_comparison_check());
}

void test_find_end_comparison_count()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 8u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n1 = sizes[s];
      int hay[16] = { 0 };
      int ndl[16] = { 0 };
      std::size_t i = 0, j = 0;

      //Periodic haystack: many candidate starts per needle, so every false
      //start has to be rejected and the re-comparisons pile up fastest.
      for(i = 0; i != n1; ++i)
         hay[i] = int(i % 3u) + 1;

      const std::size_t maxlen = (n1 + 1u < 4u) ? n1 + 1u : 4u;
      run_find_end_count_shapes(hay, n1, ndl, 0u);

      for(std::size_t len = 1u; len <= maxlen; ++len) {
         for(std::size_t off = 0; off + len <= n1; ++off) {
            for(j = 0; j != len; ++j)
               ndl[j] = hay[off + j];
            run_find_end_count_shapes(hay, n1, ndl, len);

            ndl[len - 1u] = 500;
            run_find_end_count_shapes(hay, n1, ndl, len);
         }
         for(j = 0; j != len; ++j)
            ndl[j] = 500 + int(j);
         run_find_end_count_shapes(hay, n1, ndl, len);
      }
   }
}

//! The worst case for the bound: a uniform haystack against a needle that
//! agrees with it everywhere but its last element, so every candidate start
//! is a false start rejected as late as possible.  A haystack this uniform is
//! also the case in which an unrestricted scan would test the positions past
//! the last one the needle fits at, which is exactly what the bound forbids.
void test_find_end_comparison_count_worst_case()
{
   typedef test_detail::counted_int ci_t;

   const std::size_t n1 = 12u;
   for(std::size_t n2 = 1u; n2 <= 5u; ++n2) {
      boost::container::vector<ci_t> hay(n1, ci_t(1));
      boost::container::vector<ci_t> ndl(n2, ci_t(1));
      ndl[n2 - 1u] = ci_t(2);

      const std::size_t bound = find_end_bound(n1, n2);

      test_detail::counted_int_ops().reset();
      segmented_find_end(hay.begin(), hay.end(), ndl.begin(), ndl.end());
      BOOST_TEST(test_detail::counted_int_ops().cmp <= bound);

      test_detail::counted_int_ops().reset();
      segmented_find_end(hay.begin(), test_detail::make_sentinel(hay.end()),
                         ndl.begin(), ndl.end());
      BOOST_TEST(test_detail::counted_int_ops().cmp <= bound);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Direct contract check for the verify helper's stop flag.
//
// third == (final source != last) || (final needle == s_last)
//
// The interesting case is the tie, a range ending exactly where the needle
// does: the walk left nothing to carry into the next segment, so the flag has
// to say stop even though the source was not cut short.  The matrix below
// walks every length pair, so every tie is in it, along with the empty
// needle, a one-element needle, needles longer than what is left, and starts
// that land exactly on a segment boundary.
//////////////////////////////////////////////////////////////////////////////

void test_verify_contract_flat()
{
   int hay[8];
   int ndl[8];
   std::size_t i = 0;
   for(i = 0; i != 8u; ++i)
      hay[i] = int(i);

   for(std::size_t n = 0; n <= 8u; ++n) {
      for(std::size_t s = 0; s <= 8u; ++s) {
         for(int diff = -1; diff < int(s); ++diff) {
            for(i = 0; i != s; ++i)
               ndl[i] = int(i);
            if(diff >= 0)
               ndl[diff] = 500;

            const segtrio<int*, int*, bool> r = detail_algo::find_end_verify
               (hay, hay + n, ndl, ndl + s, detail_algo::segmented_default_equal_to(),
                non_segmented_iterator_tag());

            std::size_t a = 0, b = 0;
            while(a != n && b != s && hay[a] == ndl[b]) { ++a; ++b; }

            BOOST_TEST(r.first  == hay + a);
            BOOST_TEST(r.second == ndl + b);
            BOOST_TEST(r.third  == (r.first != hay + n || r.second == ndl + s));
         }
      }
   }
}

void test_verify_contract_segmented()
{
   typedef test_detail::seg_vector<int> cont_t;
   typedef cont_t::iterator iter_t;

   int vals[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
   int ndl[8];

   //Three equal segments, so offsets 0, 3 and 6 start exactly on a boundary.
   cont_t sv;
   sv.add_segment_range(vals, vals + 3);
   sv.add_segment_range(vals + 3, vals + 6);
   sv.add_segment_range(vals + 6, vals + 9);

   for(std::size_t off = 0; off <= 9u; ++off) {
      const iter_t first = test_detail::iter_at(sv, off);
      for(std::size_t lim = off; lim <= 9u; ++lim) {
         const iter_t last = test_detail::iter_at(sv, lim);
         const std::size_t n = lim - off;
         for(std::size_t s = 0; s <= 7u; ++s) {
            for(int diff = -1; diff < int(s); ++diff) {
               std::size_t i = 0;
               for(i = 0; i != s; ++i)
                  ndl[i] = vals[(off + i) % 9u];
               if(diff >= 0)
                  ndl[diff] = 500;

               const segtrio<iter_t, int*, bool> r = detail_algo::find_end_verify
                  (first, last, ndl, ndl + s, detail_algo::segmented_default_equal_to(),
                   segmented_iterator_tag());

               std::size_t a = 0, b = 0;
               while(a != n && b != s && vals[off + a] == ndl[b]) { ++a; ++b; }

               BOOST_TEST(r.first  == test_detail::iter_at(sv, off + a));
               BOOST_TEST(r.second == ndl + b);
               BOOST_TEST(r.third  == (r.first != last || r.second == ndl + s));
            }
         }
      }
   }
}

//! Same contract one level down, where the walker's leaf is another walker
//! and the flag is propagated rather than produced.
void test_verify_contract_seg2()
{
   typedef test_detail::seg2_vector<int> cont_t;
   typedef cont_t::iterator iter_t;

   int vals[6] = {0, 1, 2, 3, 4, 5};
   int ndl[6];

   cont_t sv2;
   sv2.add_flat_segment_range(vals, vals + 2);
   sv2.add_flat_segment_range(vals + 2, vals + 4);
   sv2.add_flat_segment_range(vals + 4, vals + 6);

   for(std::size_t off = 0; off <= 6u; ++off) {
      const iter_t first = test_detail::iter_at(sv2, off);
      for(std::size_t lim = off; lim <= 6u; ++lim) {
         const iter_t last = test_detail::iter_at(sv2, lim);
         const std::size_t n = lim - off;
         for(std::size_t s = 0; s <= 5u; ++s) {
            std::size_t i = 0;
            for(i = 0; i != s; ++i)
               ndl[i] = vals[(off + i) % 6u];

            const segtrio<iter_t, int*, bool> r = detail_algo::find_end_verify
               (first, last, ndl, ndl + s, detail_algo::segmented_default_equal_to(),
                segmented_iterator_tag());

            std::size_t a = 0, b = 0;
            while(a != n && b != s && vals[off + a] == ndl[b]) { ++a; ++b; }

            BOOST_TEST(r.first  == test_detail::iter_at(sv2, off + a));
            BOOST_TEST(r.second == ndl + b);
            BOOST_TEST(r.third  == (r.first != last || r.second == ndl + s));
         }
      }
   }
}

int main()
{
   test_verify_contract_flat();
   test_verify_contract_segmented();
   test_verify_contract_seg2();
   test_find_end_multiple_occurrences();
   test_find_end_at_beginning_and_end();
   test_find_end_empty_needle();
   test_find_end_needle_longer_than_haystack();
   test_find_end_whole_haystack();
   test_find_end_single_element_needle();
   test_find_end_straddles_boundaries();
   test_find_end_single_segment();
   test_find_end_single_segment_forward();
   test_find_end_forward_multi_segment();
   test_find_end_non_segmented();
   test_find_end_seg2();
   test_find_end_seg2_both_ranges();
   test_find_end_single_segment_seg2();
   test_find_end_segmented_needle();
   test_find_end_sentinel();
   test_find_end_explicit_predicate();
   test_find_end_shape_matrix();
   test_find_end_forward_matrix();
   test_find_end_comparison_count();
   test_find_end_comparison_count_worst_case();
   return boost::report_errors();
}
