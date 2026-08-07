//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_equal.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>

using namespace boost::container;

void test_equal_matching()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   int ref[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   BOOST_TEST(segmented_equal(sv.begin(), sv.end(), ref));
}

void test_equal_mismatch()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   int ref[] = {1, 2, 3, 4, 99};
   BOOST_TEST(!segmented_equal(sv.begin(), sv.end(), ref));
}

void test_equal_mismatch_first_segment()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   int ref[] = {1, 99, 3, 4, 5};
   BOOST_TEST(!segmented_equal(sv.begin(), sv.end(), ref));
}

void test_equal_empty()
{
   test_detail::seg_vector<int> sv;
   int dummy = 0;
   BOOST_TEST(segmented_equal(sv.begin(), sv.end(), &dummy));
}

void test_equal_single_segment()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30};
   sv.add_segment_range(a, a + 3);

   int ref[] = {10, 20, 30};
   BOOST_TEST(segmented_equal(sv.begin(), sv.end(), ref));
}

void test_equal_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);

   int ref_match[] = {1, 2, 3};
   BOOST_TEST(segmented_equal(v.begin(), v.end(), ref_match));

   int ref_fail[] = {1, 2, 99};
   BOOST_TEST(!segmented_equal(v.begin(), v.end(), ref_fail));
}

void test_equal_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   int ref[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   BOOST_TEST(segmented_equal(sv.begin(), test_detail::make_sentinel(sv.end()), ref));
}

void test_equal_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);

   int ref_match[] = {1, 2, 3};
   BOOST_TEST(segmented_equal(v.begin(), test_detail::make_sentinel(v.end()), ref_match));

   int ref_fail[] = {1, 2, 99};
   BOOST_TEST(!segmented_equal(v.begin(), test_detail::make_sentinel(v.end()), ref_fail));
}

void test_equal_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int ref[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   BOOST_TEST(segmented_equal(sv2.begin(), sv2.end(), ref));

   int ref_bad[] = {1, 2, 3, 4, 5, 6, 7, 8, 0};
   BOOST_TEST(!segmented_equal(sv2.begin(), sv2.end(), ref_bad));
}

void test_equal_seg_to_seg()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv1.add_segment_range(a1, a1 + 3);
   sv1.add_segment_range(a2, a2 + 2);
   sv1.add_segment_range(a3, a3 + 4);

   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2};
   int b2[] = {3, 4, 5, 6};
   int b3[] = {7, 8, 9};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 4);
   sv2.add_segment_range(b3, b3 + 3);

   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), sv2.begin()));
}

void test_equal_seg_to_seg_mismatch()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv1.add_segment_range(a1, a1 + 3);
   sv1.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2};
   int b2[] = {3, 4, 99};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 3);

   BOOST_TEST(!segmented_equal(sv1.begin(), sv1.end(), sv2.begin()));
}

void test_equal_seg2_to_seg2()
{
   test_detail::seg2_vector<int> sv1;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv1.add_flat_segment_range(a1, a1 + 3);
   sv1.add_flat_segment_range(a2, a2 + 2);
   sv1.add_flat_segment_range(a3, a3 + 4);

   test_detail::seg2_vector<int> sv2;
   int b1[] = {1, 2};
   int b2[] = {3, 4, 5, 6};
   int b3[] = {7, 8, 9};
   sv2.add_flat_segment_range(b1, b1 + 2);
   sv2.add_flat_segment_range(b2, b2 + 4);
   sv2.add_flat_segment_range(b3, b3 + 3);

   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), sv2.begin()));

   test_detail::seg2_vector<int> sv3;
   int c1[] = {1, 2, 3, 4, 5, 6, 7, 8, 0};
   sv3.add_flat_segment_range(c1, c1 + 9);

   BOOST_TEST(!segmented_equal(sv1.begin(), sv1.end(), sv3.begin()));
}

void test_equal_seg_to_seg_misaligned()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {10};
   int a2[] = {20, 30};
   int a3[] = {40, 50, 60};
   sv1.add_segment_range(a1, a1 + 1);
   sv1.add_segment_range(a2, a2 + 2);
   sv1.add_segment_range(a3, a3 + 3);

   test_detail::seg_vector<int> sv2;
   int b1[] = {10, 20, 30, 40};
   int b2[] = {50, 60};
   sv2.add_segment_range(b1, b1 + 4);
   sv2.add_segment_range(b2, b2 + 2);

   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), sv2.begin()));
}

//////////////////////////////////////////////////////////////////////////////
// Single-segment coverage.
//
// The segmented walkers take their single-segment branch only when
// segment(first) == segment(last).  A range spanning a whole seg_vector can
// never do that, because the end iterator lives in the trailing sentinel
// segment; these tests therefore build one oversized segment and compare a
// proper sub-range of it.
//////////////////////////////////////////////////////////////////////////////

struct test_equal_double_eq
{
   bool operator()(int a, int b) const { return a * 2 == b; }
};

// S1: one segment, range starting at the segment edge.
void test_equal_single_segment_full_range()
{
   int vals[] = {10, 20, 30, 40, 50, 60};
   test_detail::seg_vector<int> sv;
   test_detail::make_range(sv, "s", vals, 6, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t last = test_detail::iter_at(sv, 6);

   int ref[] = {10, 20, 30, 40, 50, 60};
   BOOST_TEST(segmented_equal(sv.begin(), last, ref));

   // The element past last1 takes no part in the comparison.
   int ref_tail_differs[] = {10, 20, 30, 40, 50, 60, 999};
   BOOST_TEST(segmented_equal(sv.begin(), last, ref_tail_differs));

   int ref_bad[] = {10, 20, 30, 40, 50, 99};
   BOOST_TEST(!segmented_equal(sv.begin(), last, ref_bad));
}

// S2: one segment, both endpoints strictly interior.
void test_equal_single_segment_interior_bounds()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   int ref[] = {10, 20, 30, 40, 50, 777};
   BOOST_TEST(segmented_equal(first, last, ref));

   int ref_first_differs[] = {99, 20, 30, 40, 50, 777};
   BOOST_TEST(!segmented_equal(first, last, ref_first_differs));

   int ref_last_differs[] = {10, 20, 30, 40, 99, 777};
   BOOST_TEST(!segmented_equal(first, last, ref_last_differs));
}

// S3: one segment, empty range positioned mid-segment.
void test_equal_single_segment_empty_range()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, 60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t mid = test_detail::iter_at(sv, 3);

   int ref[] = {99, 99, 99};
   BOOST_TEST(segmented_equal(mid, mid, ref));
   BOOST_TEST(segmented_equal(mid, mid, ref, test_equal_double_eq()));
}

// S4: S2 through the sentinel overload.
void test_equal_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   int ref[] = {10, 20, 30, 40, 50, 777};
   BOOST_TEST(segmented_equal(first, test_detail::make_sentinel(last), ref));

   int ref_bad[] = {10, 20, 30, 40, 99, 777};
   BOOST_TEST(!segmented_equal(first, test_detail::make_sentinel(last), ref_bad));
}

// S1 and S2 through the predicate-taking overload.
void test_equal_single_segment_pred()
{
   int vals[] = {1, 2, 3, 4, 5, 6};
   test_detail::seg_vector<int> sv;
   test_detail::make_range(sv, "s", vals, 6, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t last = test_detail::iter_at(sv, 6);

   int ref[] = {2, 4, 6, 8, 10, 12};
   BOOST_TEST(segmented_equal(sv.begin(), last, ref, test_equal_double_eq()));

   int ref_bad[] = {2, 4, 6, 8, 10, 99};
   BOOST_TEST(!segmented_equal(sv.begin(), last, ref_bad, test_equal_double_eq()));

   const iter_t inner_first = test_detail::iter_at(sv, 2);
   const iter_t inner_last  = test_detail::iter_at(sv, 5);
   int ref_inner[] = {6, 8, 10, 999};
   BOOST_TEST(segmented_equal(inner_first, inner_last, ref_inner, test_equal_double_eq()));
}

// S5: one outer segment holding several inner segments.
void test_equal_single_segment_seg2_outer()
{
   int vals[] = {10, 20, 30, 40, 50};
   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "sm", vals, 5, -1);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t last = test_detail::iter_at(sv2, 5);

   int ref[] = {10, 20, 30, 40, 50, 777};
   BOOST_TEST(segmented_equal(sv2.begin(), last, ref));

   int ref_bad[] = {10, 20, 30, 40, 99, 777};
   BOOST_TEST(!segmented_equal(sv2.begin(), last, ref_bad));
}

// S6: single segment at both levels of recursion.
void test_equal_single_segment_seg2_both_levels()
{
   int vals[] = {10, 20, 30, 40, 50};
   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "ss", vals, 5, -1);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 4);

   int ref[] = {20, 30, 40, 777};
   BOOST_TEST(segmented_equal(first, last, ref));
   BOOST_TEST(segmented_equal(sv2.begin(), test_detail::iter_at(sv2, 5), vals));

   int ref_bad[] = {20, 30, 99, 777};
   BOOST_TEST(!segmented_equal(first, last, ref_bad));
}

// M4: multi-segment first range against a single-segment second range,
// with the inequality at every position in turn and nowhere.
void test_equal_single_segment_second_range()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   sv1.add_segment_range(a1, a1 + 3);
   sv1.add_segment_range(a2, a2 + 2);

   const int N = 5;
   int vals[] = {10, 20, 30, 40, 50, 60, 70};

   test_detail::seg_vector<int> sv2;
   sv2.add_segment_range(vals, vals + 7);
   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), sv2.begin()));

   for(int pos = 0; pos < N; ++pos) {
      int ref[7];
      for(int j = 0; j < 7; ++j) ref[j] = vals[j];
      ref[pos] = -1;

      test_detail::seg_vector<int> sv_bad;
      sv_bad.add_segment_range(ref, ref + 7);
      BOOST_TEST(!segmented_equal(sv1.begin(), sv1.end(), sv_bad.begin()));
   }

   // A difference past the end of the first range is never seen.
   int tail[] = {10, 20, 30, 40, 50, -1, -1};
   test_detail::seg_vector<int> sv_tail;
   sv_tail.add_segment_range(tail, tail + 7);
   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), sv_tail.begin()));
}

// M4 reversed: single-segment first range against a multi-segment second one.
void test_equal_single_segment_first_range()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv1.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv1, 1);
   const iter_t last  = test_detail::iter_at(sv1, 6);

   test_detail::seg_vector<int> sv2;
   int b1[] = {10, 20};
   int b2[] = {30};
   int b3[] = {40, 50, 60};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 1);
   sv2.add_segment_range(b3, b3 + 3);

   BOOST_TEST(segmented_equal(first, last, sv2.begin()));

   test_detail::seg_vector<int> sv3;
   int c1[] = {10, 20};
   int c2[] = {30};
   int c3[] = {40, 99, 60};
   sv3.add_segment_range(c1, c1 + 2);
   sv3.add_segment_range(c2, c2 + 1);
   sv3.add_segment_range(c3, c3 + 3);

   BOOST_TEST(!segmented_equal(first, last, sv3.begin()));
}

// M3: both ranges single-segment.
void test_equal_single_segment_both_ranges()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {10, 20, 30, 40, 50, 60, 70};
   sv2.add_segment_range(b, b + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv1, 1);
   const iter_t last  = test_detail::iter_at(sv1, 6);

   BOOST_TEST(segmented_equal(first, last, sv2.begin()));
   BOOST_TEST(segmented_equal(first, first, sv2.begin()));

   test_detail::seg_vector<int> sv3;
   int c[] = {10, 20, 30, 40, 99, 60, 70};
   sv3.add_segment_range(c, c + 7);
   BOOST_TEST(!segmented_equal(first, last, sv3.begin()));
}

// M4 with a recursively segmented second range whose outer level holds a
// single segment.
void test_equal_single_segment_second_range_seg2()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   sv1.add_segment_range(a1, a1 + 3);
   sv1.add_segment_range(a2, a2 + 2);

   int b[] = {10, 20, 30, 40, 50, 60, 70};
   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "sm", b, 7, -1);
   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), sv2.begin()));

   int c[] = {10, 20, 30, 40, 50, 60, 70};
   test_detail::seg2_vector<int> sv3;
   test_detail::make_range(sv3, "ss", c, 7, -1);
   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), sv3.begin()));

   int d[] = {10, 20, 99, 40, 50, 60, 70};
   test_detail::seg2_vector<int> sv4;
   test_detail::make_range(sv4, "sm", d, 7, -1);
   BOOST_TEST(!segmented_equal(sv1.begin(), sv1.end(), sv4.begin()));
}

// Non-segmented first range against a single-segment segmented second range.
void test_equal_single_segment_flat_first_range()
{
   boost::container::vector<int> v;
   v.push_back(10); v.push_back(20); v.push_back(30);

   test_detail::seg_vector<int> sv2;
   int b[] = {10, 20, 30, 40, 50};
   sv2.add_segment_range(b, b + 5);
   BOOST_TEST(segmented_equal(v.begin(), v.end(), sv2.begin()));

   test_detail::seg_vector<int> sv3;
   int c[] = {10, 99, 30, 40, 50};
   sv3.add_segment_range(c, c + 5);
   BOOST_TEST(!segmented_equal(v.begin(), v.end(), sv3.begin()));
}

// Inequality at the first element, at the last one and nowhere, with both
// endpoints of the single segment strictly interior.
void test_equal_single_segment_every_position()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   const int N = 5;
   int vals[] = {10, 20, 30, 40, 50};
   BOOST_TEST(segmented_equal(first, last, vals));

   for(int pos = 0; pos < N; ++pos) {
      int ref[5];
      for(int j = 0; j < N; ++j) ref[j] = vals[j];
      ref[pos] = -1;
      BOOST_TEST(!segmented_equal(first, last, ref));
   }
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// segmented_equal walks two independently segmented ranges at once, so the
// interesting cross product is of the two ranges' shapes, not of one range's
// shape with itself.  for_each_shape2_all supplies that, including the 'e'
// shapes whose empty segments the two walkers have to skip in step with each
// other.
//
// The second range is deliberately one element longer than the first and its
// extra element is a value that appears nowhere else, so that a walker which
// runs one element past last1 compares range 1's guard against it and fails
// visibly.
//////////////////////////////////////////////////////////////////////////////

const int equal_shape_tail = 12345;

struct equal_shape_check
{
   // Index of the element of range 2 that was corrupted, or n1 for none.
   std::size_t bad_pos;

   explicit equal_shape_check(std::size_t p) : bad_pos(p) {}

   void report(const char* s1, std::size_t n1, const char* s2) const
   {
      BOOST_LIGHTWEIGHT_TEST_OSTREAM
         << "   shapes \"" << s1 << "\" / \"" << s2 << "\", n = " << n1
         << ", differing at " << bad_pos << std::endl;
   }

   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      typedef typename C1::iterator iter1_t;

      const boost::container::vector<int> f1 = test_detail::flatten_n_ints(c1, n1);
      const boost::container::vector<int> f2 = test_detail::flatten_n_ints(c2, n2);

      bool expected = true;
      for(std::size_t i = 0; i != f1.size(); ++i) {
         if(f1[i] != f2[i]) { expected = false; break; }
      }

      const iter1_t first1 = c1.begin();
      const iter1_t last1  = test_detail::iter_at(c1, n1);

      if(!BOOST_TEST_EQ(segmented_equal(first1, last1, c2.begin()), expected))
         this->report(s1, n1, s2);

      // Same question through the sentinel overload, which reaches a
      // different set of dispatch templates.
      if(!BOOST_TEST_EQ(segmented_equal(first1, test_detail::make_sentinel(last1), c2.begin()),
                        expected))
         this->report(s1, n1, s2);

      // Neither range is an output, so both guards must still be intact.
      if(!BOOST_TEST(test_detail::filler_intact(c1, n1, -999)))
         this->report(s1, n1, s2);
      if(!BOOST_TEST(test_detail::filler_intact(c2, n2, -999)))
         this->report(s1, n1, s2);
   }
};

void test_equal_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 9u };

   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n1 = sizes[s];
      const std::size_t n2 = n1 + 1u;

      int v1[10] = {};
      for(std::size_t i = 0; i != n1; ++i)
         v1[i] = int(i) + 1;

      // bad == n1 means "the two ranges agree"; otherwise range 2 differs at
      // exactly that position.
      for(std::size_t bad = 0; bad <= n1; ++bad) {
         int v2[11] = {};
         for(std::size_t i = 0; i != n1; ++i)
            v2[i] = v1[i];
         v2[n1] = equal_shape_tail;
         if(bad != n1)
            v2[bad] = -7;

         test_detail::for_each_shape2_all<int, int>
            (v1, n1, v2, n2, -999, equal_shape_check(bad));
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Predicate application count.
//
// [alg.equal] mandates "At most last1 - first1 applications of the
// corresponding predicate".  The lower bound below is what stops the check
// from passing vacuously: the answer cannot be known before the first
// differing position has been looked at.
//////////////////////////////////////////////////////////////////////////////

struct eq_int
{
   bool operator()(int a, int b) const { return a == b; }
};

struct equal_count_check
{
   // Index of the element of range 2 that was corrupted, or n1 for none.
   std::size_t bad_pos;

   explicit equal_count_check(std::size_t p) : bad_pos(p) {}

   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      const std::size_t needed = bad_pos < n1 ? bad_pos + 1u : n1;

      {
         test_detail::op_counter calls;
         segmented_equal(c1.begin(), test_detail::iter_at(c1, n1), c2.begin(),
                         test_detail::counting_pred(calls, eq_int()));
         BOOST_TEST(calls.n <= n1);
         BOOST_TEST(calls.n >= needed);
      }
      {
         test_detail::op_counter calls;
         segmented_equal(c1.begin(), test_detail::make_sentinel(test_detail::iter_at(c1, n1)),
                         c2.begin(), test_detail::counting_pred(calls, eq_int()));
         BOOST_TEST(calls.n <= n1);
         BOOST_TEST(calls.n >= needed);
      }

      BOOST_TEST(s1 != 0 && s2 != 0 && n2 != 0);
   }
};

void test_equal_predicate_count()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 9u };

   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n1 = sizes[s];
      const std::size_t n2 = n1 + 1u;

      int v1[10] = {};
      for(std::size_t i = 0; i != n1; ++i)
         v1[i] = int(i) + 1;

      for(std::size_t bad = 0; bad <= n1; ++bad) {
         int v2[11] = {};
         for(std::size_t i = 0; i != n1; ++i)
            v2[i] = v1[i];
         v2[n1] = equal_shape_tail;
         if(bad != n1)
            v2[bad] = -7;

         test_detail::for_each_shape2_all<int, int>
            (v1, n1, v2, n2, -999, equal_count_check(bad));
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Four-argument overloads.
//
// [alg.equal] gives the two-range form different semantics from the
// one-and-a-half-range form: ranges of different lengths are unequal, where
// the three-argument form only ever looks at last1 - first1 elements.
//////////////////////////////////////////////////////////////////////////////

// Element-wise reference answer, independent of the algorithm under test.
// std::equal's own four-iterator form is C++14, so it is not usable at C++03.
bool ref_equal(const boost::container::vector<int>& a,
               const boost::container::vector<int>& b)
{
   if(a.size() != b.size())
      return false;
   for(std::size_t i = 0; i != a.size(); ++i) {
      if(a[i] != b[i])
         return false;
   }
   return true;
}

void test_equal4_flat()
{
   int a[] = {1, 2, 3, 4};
   int b[] = {1, 2, 3, 4, 5};

   BOOST_TEST(segmented_equal(a, a + 4, b, b + 4));
   BOOST_TEST(segmented_equal(a, a + 4, b, b + 4, eq_int()));

   // Different lengths, either way round.
   BOOST_TEST(!segmented_equal(a, a + 4, b, b + 5));
   BOOST_TEST(!segmented_equal(b, b + 5, a, a + 4));
   BOOST_TEST(!segmented_equal(a, a + 4, b, b + 5, eq_int()));
   BOOST_TEST(!segmented_equal(b, b + 5, a, a + 4, eq_int()));

   // Same length, differing content, at each position in turn.
   for(int pos = 0; pos != 4; ++pos) {
      int c[4];
      for(int j = 0; j != 4; ++j) c[j] = a[j];
      c[pos] = -1;
      BOOST_TEST(!segmented_equal(a, a + 4, c, c + 4));
   }

   // Empty on one side, on the other, and on both.
   BOOST_TEST(segmented_equal(a, a, b, b));
   BOOST_TEST(!segmented_equal(a, a, b, b + 1));
   BOOST_TEST(!segmented_equal(a, a + 1, b, b));
   BOOST_TEST(segmented_equal(a, a, b, b, test_equal_double_eq()));

   // The predicate is not required to be symmetric.
   int d[] = {2, 4, 6, 8};
   BOOST_TEST(segmented_equal(a, a + 4, d, d + 4, test_equal_double_eq()));
   BOOST_TEST(!segmented_equal(a, a + 4, d, d + 3, test_equal_double_eq()));
}

// bc::vector: random access and flat, so the sized fast path is taken.
void test_equal4_vector()
{
   boost::container::vector<int> v1, v2;
   for(int i = 0; i != 6; ++i) { v1.push_back(i); v2.push_back(i); }

   BOOST_TEST(segmented_equal(v1.begin(), v1.end(), v2.begin(), v2.end()));
   v2.push_back(6);
   BOOST_TEST(!segmented_equal(v1.begin(), v1.end(), v2.begin(), v2.end()));
   BOOST_TEST(!segmented_equal(v2.begin(), v2.end(), v1.begin(), v1.end()));
}

// bc::deque: random access and segmented, the case the sized fast path has to
// decide about, since last - first there is block arithmetic rather than a
// pointer subtraction.
void test_equal4_deque()
{
   boost::container::deque<int> d1, d2;
   for(int i = 0; i != 300; ++i) { d1.push_back(i); d2.push_back(i); }

   BOOST_TEST(segmented_equal(d1.begin(), d1.end(), d2.begin(), d2.end()));
   BOOST_TEST(segmented_equal(d1.begin(), d1.end(), d2.begin(), d2.end(), eq_int()));

   d2.push_back(300);
   BOOST_TEST(!segmented_equal(d1.begin(), d1.end(), d2.begin(), d2.end()));
   BOOST_TEST(!segmented_equal(d2.begin(), d2.end(), d1.begin(), d1.end()));
   d2.pop_back();

   // Difference in the middle of a block and on a block boundary.
   d2[150] = -1;
   BOOST_TEST(!segmented_equal(d1.begin(), d1.end(), d2.begin(), d2.end()));
   d2[150] = 150;
   d2[0] = -1;
   BOOST_TEST(!segmented_equal(d1.begin(), d1.end(), d2.begin(), d2.end()));
   d2[0] = 0;

   // A sub-range that starts and ends inside a block.
   boost::container::deque<int>::iterator f1 = d1.begin(), l1 = d1.begin();
   boost::container::deque<int>::iterator f2 = d2.begin(), l2 = d2.begin();
   for(int i = 0; i != 37; ++i)  { ++f1; ++f2; }
   for(int i = 0; i != 211; ++i) { ++l1; ++l2; }
   BOOST_TEST(segmented_equal(f1, l1, f2, l2));
   BOOST_TEST(!segmented_equal(f1, l1, f2, d2.end()));

   // Deque against a flat range of the same contents.
   boost::container::vector<int> v;
   for(int i = 0; i != 300; ++i) v.push_back(i);
   BOOST_TEST(segmented_equal(d1.begin(), d1.end(), v.begin(), v.end()));
   BOOST_TEST(!segmented_equal(d1.begin(), d1.end(), v.begin(), v.end() - 1));
}

// seg_vector's iterator is bidirectional, so these all take the walking path.
void test_equal4_segmented()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv1.add_segment_range(a1, a1 + 3);
   sv1.add_segment_range(a2, a2 + 2);
   sv1.add_segment_range(a3, a3 + 4);

   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2};
   int b2[] = {3, 4, 5, 6};
   int b3[] = {7, 8, 9};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 4);
   sv2.add_segment_range(b3, b3 + 3);

   // Same nine elements, segmented differently on the two sides.
   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), sv2.begin(), sv2.end()));
   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), eq_int()));

   // A shorter second range, cut mid-segment and on a segment boundary.
   BOOST_TEST(!segmented_equal(sv1.begin(), sv1.end(), sv2.begin(), test_detail::iter_at(sv2, 8)));
   BOOST_TEST(!segmented_equal(sv1.begin(), sv1.end(), sv2.begin(), test_detail::iter_at(sv2, 2)));
   BOOST_TEST(!segmented_equal(sv1.begin(), test_detail::iter_at(sv1, 3), sv2.begin(), sv2.end()));

   // Empty ranges.
   BOOST_TEST(segmented_equal(sv1.begin(), sv1.begin(), sv2.begin(), sv2.begin()));
   BOOST_TEST(!segmented_equal(sv1.begin(), sv1.begin(), sv2.begin(), test_detail::iter_at(sv2, 1)));
   BOOST_TEST(!segmented_equal(sv1.begin(), test_detail::iter_at(sv1, 1), sv2.begin(), sv2.begin()));

   // Flat against segmented and back.
   int flat[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
   BOOST_TEST(segmented_equal(flat, flat + 9, sv1.begin(), sv1.end()));
   BOOST_TEST(!segmented_equal(flat, flat + 8, sv1.begin(), sv1.end()));
   BOOST_TEST(segmented_equal(sv1.begin(), sv1.end(), flat, flat + 9));
   BOOST_TEST(!segmented_equal(sv1.begin(), sv1.end(), flat, flat + 10));

   test_detail::seg_vector<int> sv_bad;
   int c1[] = {1, 2, 3, 4};
   int c2[] = {5, 6, 99, 8, 9};
   sv_bad.add_segment_range(c1, c1 + 4);
   sv_bad.add_segment_range(c2, c2 + 5);
   BOOST_TEST(!segmented_equal(sv1.begin(), sv1.end(), sv_bad.begin(), sv_bad.end()));
}

void test_equal4_seg2()
{
   int vals[] = {1, 2, 3, 4, 5, 6, 7};

   test_detail::seg2_vector<int> sv1;
   test_detail::make_range(sv1, "sm", vals, 7, -1);
   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "ss", vals, 7, -1);

   const test_detail::seg2_vector<int>::iterator l1 = test_detail::iter_at(sv1, 7);
   const test_detail::seg2_vector<int>::iterator l2 = test_detail::iter_at(sv2, 7);

   BOOST_TEST(segmented_equal(sv1.begin(), l1, sv2.begin(), l2));
   BOOST_TEST(!segmented_equal(sv1.begin(), l1, sv2.begin(), test_detail::iter_at(sv2, 6)));
   BOOST_TEST(!segmented_equal(sv1.begin(), test_detail::iter_at(sv1, 6), sv2.begin(), l2));
   BOOST_TEST(segmented_equal(sv1.begin(), l1, vals, vals + 7));
   BOOST_TEST(!segmented_equal(sv1.begin(), l1, vals, vals + 6));
}

void test_equal4_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5};
   sv.add_segment_range(a, a + 5);

   const test_detail::seg_vector<int>::iterator last = test_detail::iter_at(sv, 5);

   int ref[] = {1, 2, 3, 4, 5, 6};
   BOOST_TEST(segmented_equal(sv.begin(), test_detail::make_sentinel(last), ref, ref + 5, eq_int()));
   BOOST_TEST(!segmented_equal(sv.begin(), test_detail::make_sentinel(last), ref, ref + 6, eq_int()));
   BOOST_TEST(!segmented_equal(sv.begin(), test_detail::make_sentinel(last), ref, ref + 4, eq_int()));

   // Sentinel on the second range too.
   BOOST_TEST(segmented_equal(ref, ref + 5, sv.begin(),
                              test_detail::make_sentinel(last), eq_int()));
   BOOST_TEST(!segmented_equal(ref, ref + 6, sv.begin(),
                               test_detail::make_sentinel(last), eq_int()));

   boost::container::vector<int> v;
   for(int i = 1; i != 6; ++i) v.push_back(i);
   BOOST_TEST(segmented_equal(v.begin(), test_detail::make_sentinel(v.end()), ref, ref + 5, eq_int()));
   BOOST_TEST(!segmented_equal(v.begin(), test_detail::make_sentinel(v.end()), ref, ref + 6, eq_int()));
}

//////////////////////////////////////////////////////////////////////////////
// Four-argument shape matrix, cross-checked against a flattened reference.
//////////////////////////////////////////////////////////////////////////////

struct equal4_shape_check
{
   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      typedef typename C1::iterator iter1_t;
      typedef typename C2::iterator iter2_t;

      const boost::container::vector<int> f1 = test_detail::flatten_n_ints(c1, n1);
      const boost::container::vector<int> f2 = test_detail::flatten_n_ints(c2, n2);
      const bool expected = ref_equal(f1, f2);

      const iter1_t first1 = c1.begin();
      const iter1_t last1  = test_detail::iter_at(c1, n1);
      const iter2_t first2 = c2.begin();
      const iter2_t last2  = test_detail::iter_at(c2, n2);

      if(!BOOST_TEST_EQ(segmented_equal(first1, last1, first2, last2), expected))
         BOOST_LIGHTWEIGHT_TEST_OSTREAM
            << "   shapes \"" << s1 << "\"/" << n1 << " \"" << s2 << "\"/" << n2 << std::endl;

      if(!BOOST_TEST_EQ(segmented_equal(first1, last1, first2, last2, eq_int()), expected))
         BOOST_LIGHTWEIGHT_TEST_OSTREAM
            << "   shapes \"" << s1 << "\"/" << n1 << " \"" << s2 << "\"/" << n2 << std::endl;

      // The sentinel overload reaches a different set of dispatch templates.
      if(!BOOST_TEST_EQ(segmented_equal(first1, test_detail::make_sentinel(last1),
                                        first2, last2, eq_int()), expected))
         BOOST_LIGHTWEIGHT_TEST_OSTREAM
            << "   shapes \"" << s1 << "\"/" << n1 << " \"" << s2 << "\"/" << n2 << std::endl;

      // Neither range is an output, so both guards must still be intact.
      BOOST_TEST(test_detail::filler_intact(c1, n1, -999));
      BOOST_TEST(test_detail::filler_intact(c2, n2, -999));
   }
};

void test_equal4_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u };

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
   for(std::size_t j = 0; j != sizeof(sizes)/sizeof(sizes[0]); ++j) {
      const std::size_t n1 = sizes[i];
      const std::size_t n2 = sizes[j];

      int v1[6] = {};
      int v2[6] = {};
      for(std::size_t k = 0; k != n1; ++k) v1[k] = int(k) + 1;
      for(std::size_t k = 0; k != n2; ++k) v2[k] = int(k) + 1;

      // Once with the common prefix agreeing, once with it differing.
      test_detail::for_each_shape2_all<int, int>
         (v1, n1, v2, n2, -999, equal4_shape_check());

      if(n2 != 0u) {
         v2[n2 - 1u] = -7;
         test_detail::for_each_shape2_all<int, int>
            (v1, n1, v2, n2, -999, equal4_shape_check());
      }
   }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Four-argument predicate application count.
//
// [alg.equal] allows at most min(last1 - first1, last2 - first2)
// applications, and none at all when the two sized ranges differ in length:
// the answer is then already known from the lengths.
//////////////////////////////////////////////////////////////////////////////

void test_equal4_predicate_count_sized()
{
   boost::container::vector<int> v1, v2;
   for(int i = 0; i != 8; ++i) { v1.push_back(i); v2.push_back(i); }

   // Different lengths, both ranges sized: the predicate is never applied.
   {
      test_detail::op_counter calls;
      BOOST_TEST(!segmented_equal(v1.begin(), v1.end(), v2.begin(), v2.end() - 1,
                                  test_detail::counting_pred(calls, eq_int())));
      BOOST_TEST_EQ(calls.n, 0u);
   }
   {
      test_detail::op_counter calls;
      BOOST_TEST(!segmented_equal(v1.begin(), v1.end() - 3, v2.begin(), v2.end(),
                                  test_detail::counting_pred(calls, eq_int())));
      BOOST_TEST_EQ(calls.n, 0u);
   }
   // Raw pointers take the same path.
   {
      int a[4] = {1, 2, 3, 4};
      int b[3] = {1, 2, 3};
      test_detail::op_counter calls;
      BOOST_TEST(!segmented_equal(a, a + 4, b, b + 3,
                                  test_detail::counting_pred(calls, eq_int())));
      BOOST_TEST_EQ(calls.n, 0u);
   }
   // Segmented and random access: the same O(1) decision on a deque.
   {
      boost::container::deque<int> d1, d2;
      for(int i = 0; i != 200; ++i) { d1.push_back(i); d2.push_back(i); }
      d2.push_back(200);
      test_detail::op_counter calls;
      BOOST_TEST(!segmented_equal(d1.begin(), d1.end(), d2.begin(), d2.end(),
                                  test_detail::counting_pred(calls, eq_int())));
      BOOST_TEST_EQ(calls.n, 0u);
   }

   // Equal lengths: at most n, and at least enough to reach the difference.
   {
      test_detail::op_counter calls;
      BOOST_TEST(segmented_equal(v1.begin(), v1.end(), v2.begin(), v2.end(),
                                 test_detail::counting_pred(calls, eq_int())));
      BOOST_TEST_EQ(calls.n, 8u);
   }
   for(std::size_t bad = 0; bad != 8u; ++bad) {
      boost::container::vector<int> v3(v2);
      v3[bad] = -1;
      test_detail::op_counter calls;
      BOOST_TEST(!segmented_equal(v1.begin(), v1.end(), v3.begin(), v3.end(),
                                  test_detail::counting_pred(calls, eq_int())));
      BOOST_TEST(calls.n <= 8u);
      BOOST_TEST(calls.n >= bad + 1u);
   }
}

struct equal4_count_check
{
   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      const std::size_t shorter = n1 < n2 ? n1 : n2;

      test_detail::op_counter calls;
      segmented_equal(c1.begin(), test_detail::iter_at(c1, n1),
                      c2.begin(), test_detail::iter_at(c2, n2),
                      test_detail::counting_pred(calls, eq_int()));
      if(!BOOST_TEST(calls.n <= shorter))
         BOOST_LIGHTWEIGHT_TEST_OSTREAM
            << "   shapes \"" << s1 << "\"/" << n1 << " \"" << s2 << "\"/" << n2
            << ", calls = " << calls.n << std::endl;
   }
};

void test_equal4_predicate_count_walked()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u };

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
   for(std::size_t j = 0; j != sizeof(sizes)/sizeof(sizes[0]); ++j) {
      const std::size_t n1 = sizes[i];
      const std::size_t n2 = sizes[j];

      int v1[6] = {};
      int v2[6] = {};
      for(std::size_t k = 0; k != n1; ++k) v1[k] = int(k) + 1;
      for(std::size_t k = 0; k != n2; ++k) v2[k] = int(k) + 1;

      test_detail::for_each_shape2_all<int, int>
         (v1, n1, v2, n2, -999, equal4_count_check());
   }
   }
}

int main()
{
   test_equal_shape_matrix();
   test_equal_matching();
   test_equal_mismatch();
   test_equal_mismatch_first_segment();
   test_equal_empty();
   test_equal_single_segment();
   test_equal_non_segmented();
   test_equal_sentinel_segmented();
   test_equal_sentinel_non_segmented();
   test_equal_seg2();
   test_equal_seg_to_seg();
   test_equal_seg_to_seg_mismatch();
   test_equal_seg2_to_seg2();
   test_equal_seg_to_seg_misaligned();

   // Single-segment coverage:
   test_equal_single_segment_full_range();
   test_equal_single_segment_interior_bounds();
   test_equal_single_segment_empty_range();
   test_equal_single_segment_sentinel();
   test_equal_single_segment_pred();
   test_equal_single_segment_seg2_outer();
   test_equal_single_segment_seg2_both_levels();
   test_equal_single_segment_second_range();
   test_equal_single_segment_first_range();
   test_equal_single_segment_both_ranges();
   test_equal_single_segment_second_range_seg2();
   test_equal_single_segment_flat_first_range();
   test_equal_single_segment_every_position();

   test_equal_predicate_count();

   // Four-argument overloads:
   test_equal4_flat();
   test_equal4_vector();
   test_equal4_deque();
   test_equal4_segmented();
   test_equal4_seg2();
   test_equal4_sentinel();
   test_equal4_shape_matrix();
   test_equal4_predicate_count_sized();
   test_equal4_predicate_count_walked();

   return boost::report_errors();
}
