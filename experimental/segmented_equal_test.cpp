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
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

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

   return boost::report_errors();
}
