//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_mismatch.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>
#include <utility>

using namespace boost::container;

void test_mismatch_matching()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   int ref[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref);
   BOOST_TEST(r.first == sv.end());
   BOOST_TEST(r.second == ref + 9);
}

void test_mismatch_found()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   int ref[] = {1, 2, 3, 4, 99};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref);
   BOOST_TEST(r.first != sv.end());
   BOOST_TEST_EQ(*r.first, 5);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_first_segment()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   int ref[] = {1, 99, 3, 4, 5};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref);
   BOOST_TEST(r.first != sv.end());
   BOOST_TEST_EQ(*r.first, 2);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_empty()
{
   test_detail::seg_vector<int> sv;
   int dummy = 0;
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), &dummy);
   BOOST_TEST(r.first == sv.end());
   BOOST_TEST(r.second == &dummy);
}

void test_mismatch_single_segment()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30};
   sv.add_segment_range(a, a + 3);

   int ref_match[] = {10, 20, 30};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref_match);
   BOOST_TEST(r.first == sv.end());

   int ref_fail[] = {10, 20, 99};
   r = segmented_mismatch(sv.begin(), sv.end(), ref_fail);
   BOOST_TEST(r.first != sv.end());
   BOOST_TEST_EQ(*r.first, 30);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);

   int ref_match[] = {1, 2, 3};
   typedef boost::container::vector<int>::iterator vec_it;
   std::pair<vec_it, int*> r = segmented_mismatch(v.begin(), v.end(), ref_match);
   BOOST_TEST(r.first == v.end());

   int ref_fail[] = {1, 2, 99};
   r = segmented_mismatch(v.begin(), v.end(), ref_fail);
   BOOST_TEST(r.first != v.end());
   BOOST_TEST_EQ(*r.first, 3);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   int ref[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r =
      segmented_mismatch(sv.begin(), test_detail::make_sentinel(sv.end()), ref);
   BOOST_TEST(r.first == sv.end());
   BOOST_TEST(r.second == ref + 9);
}

void test_mismatch_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);

   int ref_match[] = {1, 2, 3};
   typedef boost::container::vector<int>::iterator vec_it;
   std::pair<vec_it, int*> r =
      segmented_mismatch(v.begin(), test_detail::make_sentinel(v.end()), ref_match);
   BOOST_TEST(r.first == v.end());

   int ref_fail[] = {1, 2, 99};
   r = segmented_mismatch(v.begin(), test_detail::make_sentinel(v.end()), ref_fail);
   BOOST_TEST(r.first != v.end());
   BOOST_TEST_EQ(*r.first, 3);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int ref[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   typedef test_detail::seg2_vector<int>::iterator seg2_it;
   std::pair<seg2_it, int*> r = segmented_mismatch(sv2.begin(), sv2.end(), ref);
   BOOST_TEST(r.first == sv2.end());

   int ref_bad[] = {1, 2, 3, 4, 5, 6, 7, 8, 0};
   r = segmented_mismatch(sv2.begin(), sv2.end(), ref_bad);
   BOOST_TEST(r.first != sv2.end());
   BOOST_TEST_EQ(*r.first, 9);
   BOOST_TEST_EQ(*r.second, 0);
}

void test_mismatch_every_position()
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

   for(int pos = 0; pos < N; ++pos) {
      int ref[9];
      for(int j = 0; j < N; ++j) ref[j] = vals[j];
      ref[pos] = -1;

      iter_t expected = sv.begin();
      for(int j = 0; j < pos; ++j) ++expected;

      std::pair<iter_t, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref);
      BOOST_TEST(r.first == expected);
      BOOST_TEST_EQ(*r.first, vals[pos]);
      BOOST_TEST_EQ(*r.second, -1);
   }

   std::pair<iter_t, int*> r = segmented_mismatch(sv.begin(), sv.end(), vals);
   BOOST_TEST(r.first == sv.end());
}

void test_mismatch_every_position_seg2()
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

   for(int pos = 0; pos < N; ++pos) {
      int ref[9];
      for(int j = 0; j < N; ++j) ref[j] = vals[j];
      ref[pos] = -1;

      iter_t expected = sv2.begin();
      for(int j = 0; j < pos; ++j) ++expected;

      std::pair<iter_t, int*> r = segmented_mismatch(sv2.begin(), sv2.end(), ref);
      BOOST_TEST(r.first == expected);
      BOOST_TEST_EQ(*r.first, vals[pos]);
      BOOST_TEST_EQ(*r.second, -1);
   }

   std::pair<iter_t, int*> r = segmented_mismatch(sv2.begin(), sv2.end(), vals);
   BOOST_TEST(r.first == sv2.end());
}

void test_mismatch_seg_to_seg()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2, 3, 4};
   int b2[] = {5, 6, 7, 8};
   int b3[] = {9};
   sv2.add_segment_range(b1, b1 + 4);
   sv2.add_segment_range(b2, b2 + 4);
   sv2.add_segment_range(b3, b3 + 1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin());
   BOOST_TEST(r.first == sv.end());
}

void test_mismatch_seg_to_seg_mismatch()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2, 3, 4};
   int b2[] = {5, 6, 99, 8};
   int b3[] = {9};
   sv2.add_segment_range(b1, b1 + 4);
   sv2.add_segment_range(b2, b2 + 4);
   sv2.add_segment_range(b3, b3 + 1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin());
   BOOST_TEST(r.first != sv.end());
   BOOST_TEST_EQ(*r.first, 7);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_seg2_to_seg2()
{
   test_detail::seg2_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_flat_segment_range(a1, a1 + 3);
   sv.add_flat_segment_range(a2, a2 + 2);
   sv.add_flat_segment_range(a3, a3 + 4);

   test_detail::seg2_vector<int> sv2;
   int b1[] = {1, 2, 3, 4, 5};
   int b2[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(b1, b1 + 5);
   sv2.add_flat_segment_range(b2, b2 + 4);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin());
   BOOST_TEST(r.first == sv.end());
}

void test_mismatch_seg_to_seg_misaligned()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3, 4, 5};
   int a2[] = {6, 7, 8};
   sv.add_segment_range(a1, a1 + 5);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2};
   int b2[] = {3, 4, 5};
   int b3[] = {6, 7, 8};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 3);
   sv2.add_segment_range(b3, b3 + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin());
   BOOST_TEST(r.first == sv.end());
}

void test_mismatch_seg_to_seg_every_position()
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

   for(int pos = 0; pos < N; ++pos) {
      test_detail::seg_vector<int> sv2;
      int ref[9];
      for(int j = 0; j < N; ++j) ref[j] = vals[j];
      ref[pos] = -1;
      int r1[] = {ref[0], ref[1], ref[2], ref[3]};
      int r2[] = {ref[4], ref[5], ref[6], ref[7]};
      int r3[] = {ref[8]};
      sv2.add_segment_range(r1, r1 + 4);
      sv2.add_segment_range(r2, r2 + 4);
      sv2.add_segment_range(r3, r3 + 1);

      iter_t expected = sv.begin();
      for(int j = 0; j < pos; ++j) ++expected;

      std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin());
      BOOST_TEST(r.first == expected);
      BOOST_TEST_EQ(*r.first, vals[pos]);
      BOOST_TEST_EQ(*r.second, -1);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Tests for the two-range overloads (first1, last1, first2, last2[, pred])
//////////////////////////////////////////////////////////////////////////////

void test_mismatch_2r_equal_same_length()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   int ref[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref, ref + 9);
   BOOST_TEST(r.first  == sv.end());
   BOOST_TEST(r.second == ref + 9);
}

void test_mismatch_2r_second_shorter()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   int ref[] = {1, 2, 3, 4};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref, ref + 4);
   BOOST_TEST(r.second == ref + 4);
   BOOST_TEST(r.first  != sv.end());
   BOOST_TEST_EQ(*r.first, 5);
}

void test_mismatch_2r_first_shorter()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2};
   int a2[] = {3};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 1);

   int ref[] = {1, 2, 3, 4, 5};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref, ref + 5);
   BOOST_TEST(r.first  == sv.end());
   BOOST_TEST_EQ(*r.second, 4);
}

void test_mismatch_2r_mismatch_within_common()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   int ref[] = {1, 2, 99, 4, 5, 6};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref, ref + 6);
   BOOST_TEST_EQ(*r.first,  3);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_2r_both_empty()
{
   test_detail::seg_vector<int> sv;
   int dummy = 0;
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), &dummy, &dummy);
   BOOST_TEST(r.first  == sv.end());
   BOOST_TEST(r.second == &dummy);
}

void test_mismatch_2r_first_empty()
{
   test_detail::seg_vector<int> sv;
   int ref[] = {1, 2, 3};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref, ref + 3);
   BOOST_TEST(r.first  == sv.end());
   BOOST_TEST(r.second == ref);
}

void test_mismatch_2r_second_empty()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   int dummy = 0;
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch(sv.begin(), sv.end(), &dummy, &dummy);
   BOOST_TEST(r.first  == sv.begin());
   BOOST_TEST(r.second == &dummy);
}

struct test_mismatch_double_eq {
   bool operator()(int a, int b) const { return a * 2 == b; }
};

void test_mismatch_2r_with_pred()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   int ref[] = {2, 4, 6, 8, 10};
   typedef test_mismatch_double_eq double_eq;
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r =
      segmented_mismatch(sv.begin(), sv.end(), ref + 0, ref + 5, double_eq());
   BOOST_TEST(r.first  == sv.end());
   BOOST_TEST(r.second == ref + 5);

   int ref_bad[] = {2, 4, 99, 8, 10};
   r = segmented_mismatch(sv.begin(), sv.end(), ref_bad + 0, ref_bad + 5, double_eq());
   BOOST_TEST_EQ(*r.first,  3);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_2r_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);

   int ref_match[] = {1, 2, 3, 4};
   typedef boost::container::vector<int>::iterator vec_it;
   std::pair<vec_it, int*> r = segmented_mismatch(v.begin(), v.end(), ref_match, ref_match + 4);
   BOOST_TEST(r.first  == v.end());
   BOOST_TEST_EQ(*r.second, 4);

   int ref_fail[] = {1, 99};
   r = segmented_mismatch(v.begin(), v.end(), ref_fail, ref_fail + 2);
   BOOST_TEST_EQ(*r.first,  2);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_2r_sentinel_first_range()
{
   // Sentinel for last1, regular iterator for last2 via the 5-arg pred form.
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   int ref[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   typedef test_detail::seg_vector<int>::iterator seg_it;
   std::pair<seg_it, int*> r = segmented_mismatch
      (sv.begin(), test_detail::make_sentinel(sv.end()),
       ref, ref + 9,
       boost::container::detail_algo::segmented_default_equal_to());
   BOOST_TEST(r.first  == sv.end());
   BOOST_TEST(r.second == ref + 9);
}

void test_mismatch_2r_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int ref[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
   typedef test_detail::seg2_vector<int>::iterator seg2_it;
   std::pair<seg2_it, int*> r = segmented_mismatch(sv2.begin(), sv2.end(), ref, ref + 9);
   BOOST_TEST(r.first  == sv2.end());
   BOOST_TEST(r.second == ref + 9);

   int ref_bad[] = {1, 2, 3, 4, 5, 6, 7, 0};
   r = segmented_mismatch(sv2.begin(), sv2.end(), ref_bad, ref_bad + 8);
   BOOST_TEST_EQ(*r.first,  8);
   BOOST_TEST_EQ(*r.second, 0);
}

void test_mismatch_2r_seg_to_seg_equal()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2, 3, 4};
   int b2[] = {5, 6, 7, 8};
   int b3[] = {9};
   sv2.add_segment_range(b1, b1 + 4);
   sv2.add_segment_range(b2, b2 + 4);
   sv2.add_segment_range(b3, b3 + 1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin(), sv2.end());
   BOOST_TEST(r.first  == sv.end());
   BOOST_TEST(r.second == sv2.end());
}

void test_mismatch_2r_seg_to_seg_second_shorter()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2};
   int b2[] = {3, 4};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 2);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin(), sv2.end());
   BOOST_TEST(r.second == sv2.end());
   BOOST_TEST(r.first  != sv.end());
   BOOST_TEST_EQ(*r.first, 5);
}

void test_mismatch_2r_seg_to_seg_mismatch_straddle()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2, 3, 4};
   int b2[] = {5, 6, 99, 8};
   int b3[] = {9};
   sv2.add_segment_range(b1, b1 + 4);
   sv2.add_segment_range(b2, b2 + 4);
   sv2.add_segment_range(b3, b3 + 1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin(), sv2.end());
   BOOST_TEST(r.first  != sv.end());
   BOOST_TEST_EQ(*r.first,  7);
   BOOST_TEST_EQ(*r.second, 99);
}

void test_mismatch_2r_seg2_to_seg2()
{
   test_detail::seg2_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_flat_segment_range(a1, a1 + 3);
   sv.add_flat_segment_range(a2, a2 + 2);
   sv.add_flat_segment_range(a3, a3 + 4);

   test_detail::seg2_vector<int> sv2;
   int b1[] = {1, 2, 3, 4, 5};
   int b2[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(b1, b1 + 5);
   sv2.add_flat_segment_range(b2, b2 + 4);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin(), sv2.end());
   BOOST_TEST(r.first  == sv.end());
   BOOST_TEST(r.second == sv2.end());
}

void test_mismatch_2r_every_position()
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

   for(int pos = 0; pos < N; ++pos) {
      int ref[9];
      for(int j = 0; j < N; ++j) ref[j] = vals[j];
      ref[pos] = -1;

      iter_t expected = sv.begin();
      for(int j = 0; j < pos; ++j) ++expected;

      std::pair<iter_t, int*> r = segmented_mismatch(sv.begin(), sv.end(), ref, ref + N);
      BOOST_TEST(r.first == expected);
      BOOST_TEST_EQ(*r.first,  vals[pos]);
      BOOST_TEST_EQ(*r.second, -1);
   }
}

void test_mismatch_2r_seg_to_seg_every_position()
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

   for(int pos = 0; pos < N; ++pos) {
      test_detail::seg_vector<int> sv2;
      int ref[9];
      for(int j = 0; j < N; ++j) ref[j] = vals[j];
      ref[pos] = -1;
      int r1[] = {ref[0], ref[1], ref[2], ref[3]};
      int r2[] = {ref[4], ref[5], ref[6], ref[7]};
      int r3[] = {ref[8]};
      sv2.add_segment_range(r1, r1 + 4);
      sv2.add_segment_range(r2, r2 + 4);
      sv2.add_segment_range(r3, r3 + 1);

      iter_t expected = sv.begin();
      for(int j = 0; j < pos; ++j) ++expected;

      std::pair<iter_t, iter_t> r = segmented_mismatch(sv.begin(), sv.end(), sv2.begin(), sv2.end());
      BOOST_TEST(r.first == expected);
      BOOST_TEST_EQ(*r.first,  vals[pos]);
      BOOST_TEST_EQ(*r.second, -1);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Single-segment coverage.
//
// The segmented walkers take their single-segment branch only when
// segment(first) == segment(last).  A range spanning a whole seg_vector can
// never do that, because the end iterator lives in the trailing sentinel
// segment; these tests therefore build one oversized segment and use a
// proper sub-range of it.  Both returned iterators are checked by their
// offset from the start of their own range.
//////////////////////////////////////////////////////////////////////////////

// S1: one segment, range starting at the segment edge.
void test_mismatch_single_segment_full_range()
{
   int vals[] = {10, 20, 30, 40, 50, 60};
   test_detail::seg_vector<int> sv;
   test_detail::make_range(sv, "s", vals, 6, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t last = test_detail::iter_at(sv, 6);

   int ref[] = {10, 20, 30, 40, 50, 60, 999};
   std::pair<iter_t, int*> r = segmented_mismatch(sv.begin(), last, ref);
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == ref + 6);

   int ref_bad[] = {10, 20, 30, 99, 50, 60, 999};
   r = segmented_mismatch(sv.begin(), last, ref_bad);
   BOOST_TEST(r.first  == test_detail::iter_at(sv, 3));
   BOOST_TEST(r.second == ref_bad + 3);
}

// S2: one segment, both endpoints strictly interior; the mismatch lands at
// the first element, at the last one and nowhere.
void test_mismatch_single_segment_interior_bounds()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   int ref[] = {10, 20, 30, 40, 50, 777};
   std::pair<iter_t, int*> r = segmented_mismatch(first, last, ref);
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == ref + 5);

   int ref_at_first[] = {99, 20, 30, 40, 50, 777};
   r = segmented_mismatch(first, last, ref_at_first);
   BOOST_TEST(r.first  == first);
   BOOST_TEST(r.second == ref_at_first);

   int ref_at_last[] = {10, 20, 30, 40, 99, 777};
   r = segmented_mismatch(first, last, ref_at_last);
   BOOST_TEST(r.first  == test_detail::iter_at(sv, 5));
   BOOST_TEST(r.second == ref_at_last + 4);
}

// S3: one segment, empty range positioned mid-segment.
void test_mismatch_single_segment_empty_range()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, 60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t mid = test_detail::iter_at(sv, 3);

   int ref[] = {99, 99, 99};
   std::pair<iter_t, int*> r = segmented_mismatch(mid, mid, ref);
   BOOST_TEST(r.first  == mid);
   BOOST_TEST(r.second == ref);
}

// S4: S2 through the sentinel overload.
void test_mismatch_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   int ref[] = {10, 20, 30, 40, 50, 777};
   std::pair<iter_t, int*> r =
      segmented_mismatch(first, test_detail::make_sentinel(last), ref);
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == ref + 5);

   int ref_bad[] = {10, 20, 99, 40, 50, 777};
   r = segmented_mismatch(first, test_detail::make_sentinel(last), ref_bad);
   BOOST_TEST(r.first  == test_detail::iter_at(sv, 3));
   BOOST_TEST(r.second == ref_bad + 2);
}

// S1 and S2 through the predicate-taking overload.
void test_mismatch_single_segment_pred()
{
   int vals[] = {1, 2, 3, 4, 5, 6};
   test_detail::seg_vector<int> sv;
   test_detail::make_range(sv, "s", vals, 6, -1);

   typedef test_mismatch_double_eq double_eq;
   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t last = test_detail::iter_at(sv, 6);

   int ref[] = {2, 4, 6, 8, 10, 12};
   std::pair<iter_t, int*> r = segmented_mismatch(sv.begin(), last, ref, double_eq());
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == ref + 6);

   const iter_t first = test_detail::iter_at(sv, 2);
   int ref_bad[] = {6, 8, 99, 999};
   r = segmented_mismatch(first, last, ref_bad, double_eq());
   BOOST_TEST(r.first  == test_detail::iter_at(sv, 4));
   BOOST_TEST(r.second == ref_bad + 2);
}

// S5: one outer segment holding several inner segments.
void test_mismatch_single_segment_seg2_outer()
{
   int vals[] = {10, 20, 30, 40, 50};
   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "sm", vals, 5, -1);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t last = test_detail::iter_at(sv2, 5);

   int ref[] = {10, 20, 30, 40, 50, 777};
   std::pair<iter_t, int*> r = segmented_mismatch(sv2.begin(), last, ref);
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == ref + 5);

   int ref_bad[] = {10, 20, 30, 99, 50, 777};
   r = segmented_mismatch(sv2.begin(), last, ref_bad);
   BOOST_TEST(r.first  == test_detail::iter_at(sv2, 3));
   BOOST_TEST(r.second == ref_bad + 3);
}

// S6: single segment at both levels of recursion.
void test_mismatch_single_segment_seg2_both_levels()
{
   int vals[] = {10, 20, 30, 40, 50};
   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "ss", vals, 5, -1);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 4);

   int ref[] = {20, 30, 40, 777};
   std::pair<iter_t, int*> r = segmented_mismatch(first, last, ref);
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == ref + 3);

   int ref_bad[] = {20, 99, 40, 777};
   r = segmented_mismatch(first, last, ref_bad);
   BOOST_TEST(r.first  == test_detail::iter_at(sv2, 2));
   BOOST_TEST(r.second == ref_bad + 1);
}

// M4: multi-segment first range against a single-segment second range.
void test_mismatch_single_segment_second_range()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   sv1.add_segment_range(a1, a1 + 3);
   sv1.add_segment_range(a2, a2 + 2);

   const int N = 5;
   int vals[] = {10, 20, 30, 40, 50, 60, 70};
   typedef test_detail::seg_vector<int>::iterator iter_t;

   test_detail::seg_vector<int> sv2;
   sv2.add_segment_range(vals, vals + 7);
   std::pair<iter_t, iter_t> r = segmented_mismatch(sv1.begin(), sv1.end(), sv2.begin());
   BOOST_TEST(r.first  == sv1.end());
   BOOST_TEST(r.second == test_detail::iter_at(sv2, 5));

   for(int pos = 0; pos < N; ++pos) {
      int ref[7];
      for(int j = 0; j < 7; ++j) ref[j] = vals[j];
      ref[pos] = -1;

      test_detail::seg_vector<int> sv_bad;
      sv_bad.add_segment_range(ref, ref + 7);
      r = segmented_mismatch(sv1.begin(), sv1.end(), sv_bad.begin());
      BOOST_TEST(r.first  == test_detail::iter_at(sv1, static_cast<std::size_t>(pos)));
      BOOST_TEST(r.second == test_detail::iter_at(sv_bad, static_cast<std::size_t>(pos)));
   }
}

// M4 reversed: single-segment first range against a multi-segment second one.
void test_mismatch_single_segment_first_range()
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

   std::pair<iter_t, iter_t> r = segmented_mismatch(first, last, sv2.begin());
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == test_detail::iter_at(sv2, 5));

   test_detail::seg_vector<int> sv3;
   int c1[] = {10, 20};
   int c2[] = {30};
   int c3[] = {99, 50, 60};
   sv3.add_segment_range(c1, c1 + 2);
   sv3.add_segment_range(c2, c2 + 1);
   sv3.add_segment_range(c3, c3 + 3);

   r = segmented_mismatch(first, last, sv3.begin());
   BOOST_TEST(r.first  == test_detail::iter_at(sv1, 4));
   BOOST_TEST(r.second == test_detail::iter_at(sv3, 3));
}

// M3: both ranges single-segment, second range longer than the first.
void test_mismatch_single_segment_both_ranges()
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

   std::pair<iter_t, iter_t> r = segmented_mismatch(first, last, sv2.begin());
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == test_detail::iter_at(sv2, 5));

   test_detail::seg_vector<int> sv3;
   int c[] = {10, 20, 30, 40, 99, 60, 70};
   sv3.add_segment_range(c, c + 7);
   r = segmented_mismatch(first, last, sv3.begin());
   BOOST_TEST(r.first  == test_detail::iter_at(sv1, 5));
   BOOST_TEST(r.second == test_detail::iter_at(sv3, 4));

   r = segmented_mismatch(first, first, sv2.begin());
   BOOST_TEST(r.first  == first);
   BOOST_TEST(r.second == sv2.begin());
}

//////////////////////////////////////////////////////////////////////////////
// Single-segment coverage for the two-range (bounded) overloads.
//////////////////////////////////////////////////////////////////////////////

// S1/S2 on the first range, flat second range.
void test_mismatch_2r_single_segment_first_range()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   int ref[] = {10, 20, 30, 40, 50};
   std::pair<iter_t, int*> r = segmented_mismatch(first, last, ref, ref + 5);
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == ref + 5);

   int ref_at_first[] = {99, 20, 30, 40, 50};
   r = segmented_mismatch(first, last, ref_at_first, ref_at_first + 5);
   BOOST_TEST(r.first  == first);
   BOOST_TEST(r.second == ref_at_first);

   int ref_at_last[] = {10, 20, 30, 40, 99};
   r = segmented_mismatch(first, last, ref_at_last, ref_at_last + 5);
   BOOST_TEST(r.first  == test_detail::iter_at(sv, 5));
   BOOST_TEST(r.second == ref_at_last + 4);

   int vals[] = {10, 20, 30, 40, 50, 60};
   test_detail::seg_vector<int> sv_edge;
   test_detail::make_range(sv_edge, "s", vals, 6, -1);
   const iter_t edge_last = test_detail::iter_at(sv_edge, 6);
   std::pair<iter_t, int*> re =
      segmented_mismatch(sv_edge.begin(), edge_last, vals, vals + 6);
   BOOST_TEST(re.first  == edge_last);
   BOOST_TEST(re.second == vals + 6);
}

// S3: empty first range positioned mid-segment.
void test_mismatch_2r_single_segment_empty_range()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, 60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t mid = test_detail::iter_at(sv, 3);

   int ref[] = {99, 99, 99};
   std::pair<iter_t, int*> r = segmented_mismatch(mid, mid, ref, ref + 3);
   BOOST_TEST(r.first  == mid);
   BOOST_TEST(r.second == ref);
}

// S4: interior sub-range closed by a sentinel.
void test_mismatch_2r_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   int ref[] = {10, 20, 30, 40, 50};
   std::pair<iter_t, int*> r =
      segmented_mismatch(first, test_detail::make_sentinel(last), ref, ref + 5);
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == ref + 5);

   int ref_bad[] = {10, 20, 99, 40, 50};
   r = segmented_mismatch(first, test_detail::make_sentinel(last), ref_bad, ref_bad + 5);
   BOOST_TEST(r.first  == test_detail::iter_at(sv, 3));
   BOOST_TEST(r.second == ref_bad + 2);
}

// S5/S6 on the first range.
void test_mismatch_2r_single_segment_seg2()
{
   int vals[] = {10, 20, 30, 40, 50};
   typedef test_detail::seg2_vector<int>::iterator iter_t;

   test_detail::seg2_vector<int> outer_single;
   test_detail::make_range(outer_single, "sm", vals, 5, -1);
   const iter_t last_sm = test_detail::iter_at(outer_single, 5);

   int ref[] = {10, 20, 30, 40, 50};
   std::pair<iter_t, int*> r =
      segmented_mismatch(outer_single.begin(), last_sm, ref, ref + 5);
   BOOST_TEST(r.first  == last_sm);
   BOOST_TEST(r.second == ref + 5);

   int ref_bad[] = {10, 20, 30, 99, 50};
   r = segmented_mismatch(outer_single.begin(), last_sm, ref_bad, ref_bad + 5);
   BOOST_TEST(r.first  == test_detail::iter_at(outer_single, 3));
   BOOST_TEST(r.second == ref_bad + 3);

   test_detail::seg2_vector<int> both_single;
   test_detail::make_range(both_single, "ss", vals, 5, -1);
   const iter_t first_ss = test_detail::iter_at(both_single, 1);
   const iter_t last_ss  = test_detail::iter_at(both_single, 4);

   int ref_ss[] = {20, 30, 40};
   r = segmented_mismatch(first_ss, last_ss, ref_ss, ref_ss + 3);
   BOOST_TEST(r.first  == last_ss);
   BOOST_TEST(r.second == ref_ss + 3);

   int ref_ss_bad[] = {20, 30, 99};
   r = segmented_mismatch(first_ss, last_ss, ref_ss_bad, ref_ss_bad + 3);
   BOOST_TEST(r.first  == test_detail::iter_at(both_single, 3));
   BOOST_TEST(r.second == ref_ss_bad + 2);
}

// M4: multi-segment first range against a second range that is an interior
// sub-range of a single segment, i.e. bounded on both sides.
void test_mismatch_2r_single_segment_second_range()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   sv1.add_segment_range(a1, a1 + 3);
   sv1.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int> sv2;
   int b[] = {-9, 10, 20, 30, 40, 50, -6};
   sv2.add_segment_range(b, b + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first2 = test_detail::iter_at(sv2, 1);
   const iter_t last2  = test_detail::iter_at(sv2, 6);

   std::pair<iter_t, iter_t> r =
      segmented_mismatch(sv1.begin(), sv1.end(), first2, last2);
   BOOST_TEST(r.first  == sv1.end());
   BOOST_TEST(r.second == last2);

   test_detail::seg_vector<int> sv3;
   int c[] = {-9, 10, 20, 99, 40, 50, -6};
   sv3.add_segment_range(c, c + 7);
   r = segmented_mismatch(sv1.begin(), sv1.end(),
                          test_detail::iter_at(sv3, 1), test_detail::iter_at(sv3, 6));
   BOOST_TEST(r.first  == test_detail::iter_at(sv1, 2));
   BOOST_TEST(r.second == test_detail::iter_at(sv3, 3));

   // Mismatch on the very first element of the single-segment second range.
   test_detail::seg_vector<int> sv4;
   int d[] = {-9, 99, 20, 30, 40, 50, -6};
   sv4.add_segment_range(d, d + 7);
   r = segmented_mismatch(sv1.begin(), sv1.end(),
                          test_detail::iter_at(sv4, 1), test_detail::iter_at(sv4, 6));
   BOOST_TEST(r.first  == sv1.begin());
   BOOST_TEST(r.second == test_detail::iter_at(sv4, 1));

   // Mismatch on the very last element of the single-segment second range.
   test_detail::seg_vector<int> sv5;
   int e[] = {-9, 10, 20, 30, 40, 99, -6};
   sv5.add_segment_range(e, e + 7);
   r = segmented_mismatch(sv1.begin(), sv1.end(),
                          test_detail::iter_at(sv5, 1), test_detail::iter_at(sv5, 6));
   BOOST_TEST(r.first  == test_detail::iter_at(sv1, 4));
   BOOST_TEST(r.second == test_detail::iter_at(sv5, 5));
}

// M3: both ranges are interior sub-ranges of a single segment.
void test_mismatch_2r_single_segment_both_ranges()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-9, 10, 20, 30, 40, 50, -6};
   sv2.add_segment_range(b, b + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first1 = test_detail::iter_at(sv1, 1);
   const iter_t last1  = test_detail::iter_at(sv1, 6);
   const iter_t first2 = test_detail::iter_at(sv2, 1);
   const iter_t last2  = test_detail::iter_at(sv2, 6);

   std::pair<iter_t, iter_t> r = segmented_mismatch(first1, last1, first2, last2);
   BOOST_TEST(r.first  == last1);
   BOOST_TEST(r.second == last2);

   test_detail::seg_vector<int> sv3;
   int c[] = {-9, 10, 99, 30, 40, 50, -6};
   sv3.add_segment_range(c, c + 7);
   r = segmented_mismatch(first1, last1,
                          test_detail::iter_at(sv3, 1), test_detail::iter_at(sv3, 6));
   BOOST_TEST(r.first  == test_detail::iter_at(sv1, 2));
   BOOST_TEST(r.second == test_detail::iter_at(sv3, 2));

   // Both empty, both positioned mid-segment.
   r = segmented_mismatch(first1, first1, first2, first2);
   BOOST_TEST(r.first  == first1);
   BOOST_TEST(r.second == first2);
}

// Unequal lengths in both directions, with single-segment ranges: the walk
// stops either because the first range ran out or because the second did.
void test_mismatch_2r_single_segment_unequal_lengths()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-9, 10, 20, 30, 40, 50, -6};
   sv2.add_segment_range(b, b + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;

   // Second range shorter: stops at last2, first range still has elements.
   std::pair<iter_t, iter_t> r = segmented_mismatch
      ( test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6)
      , test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 4));
   BOOST_TEST(r.first  == test_detail::iter_at(sv1, 4));
   BOOST_TEST(r.second == test_detail::iter_at(sv2, 4));

   // First range shorter: stops at last1, second range still has elements.
   r = segmented_mismatch
      ( test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 3)
      , test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6));
   BOOST_TEST(r.first  == test_detail::iter_at(sv1, 3));
   BOOST_TEST(r.second == test_detail::iter_at(sv2, 3));

   // Second range empty while the first still has elements.
   r = segmented_mismatch
      ( test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6)
      , test_detail::iter_at(sv2, 2), test_detail::iter_at(sv2, 2));
   BOOST_TEST(r.first  == test_detail::iter_at(sv1, 1));
   BOOST_TEST(r.second == test_detail::iter_at(sv2, 2));
}

// The comparator-taking two-range overload on single-segment ranges.
void test_mismatch_2r_single_segment_with_pred()
{
   int vals[] = {1, 2, 3, 4, 5, 6};
   test_detail::seg_vector<int> sv;
   test_detail::make_range(sv, "s", vals, 6, -1);

   typedef test_mismatch_double_eq double_eq;
   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   int ref[] = {4, 6, 8, 10};
   std::pair<iter_t, int*> r =
      segmented_mismatch(first, last, ref, ref + 4, double_eq());
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == ref + 4);

   int ref_bad[] = {4, 6, 99, 10};
   r = segmented_mismatch(first, last, ref_bad, ref_bad + 4, double_eq());
   BOOST_TEST(r.first  == test_detail::iter_at(sv, 3));
   BOOST_TEST(r.second == ref_bad + 2);
}

// M4 with a recursively segmented second range whose outer level holds a
// single segment.
void test_mismatch_2r_single_segment_second_range_seg2()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   sv1.add_segment_range(a1, a1 + 3);
   sv1.add_segment_range(a2, a2 + 2);

   int b[] = {10, 20, 30, 40, 50};
   typedef test_detail::seg_vector<int>::iterator iter1_t;
   typedef test_detail::seg2_vector<int>::iterator iter2_t;

   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "sm", b, 5, -1);
   std::pair<iter1_t, iter2_t> r = segmented_mismatch
      (sv1.begin(), sv1.end(), sv2.begin(), test_detail::iter_at(sv2, 5));
   BOOST_TEST(r.first  == sv1.end());
   BOOST_TEST(r.second == test_detail::iter_at(sv2, 5));

   int c[] = {10, 20, 99, 40, 50};
   test_detail::seg2_vector<int> sv3;
   test_detail::make_range(sv3, "ss", c, 5, -1);
   r = segmented_mismatch
      (sv1.begin(), sv1.end(), sv3.begin(), test_detail::iter_at(sv3, 5));
   BOOST_TEST(r.first  == test_detail::iter_at(sv1, 2));
   BOOST_TEST(r.second == test_detail::iter_at(sv3, 2));
}

// Mismatch at every position of a single-segment first range, and nowhere.
void test_mismatch_2r_single_segment_every_position()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-7, 10, 20, 30, 40, 50, -8};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   const int N = 5;
   int vals[] = {10, 20, 30, 40, 50};

   std::pair<iter_t, int*> r = segmented_mismatch(first, last, vals, vals + N);
   BOOST_TEST(r.first  == last);
   BOOST_TEST(r.second == vals + N);

   for(int pos = 0; pos < N; ++pos) {
      int ref[5];
      for(int j = 0; j < N; ++j) ref[j] = vals[j];
      ref[pos] = -1;

      r = segmented_mismatch(first, last, ref, ref + N);
      BOOST_TEST(r.first  == test_detail::iter_at(sv, static_cast<std::size_t>(pos) + 1u));
      BOOST_TEST(r.second == ref + pos);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// Like segmented_equal, segmented_mismatch advances two independently
// segmented ranges together, so the cross product of the two ranges' shapes
// is what matters.  Both returned iterators are checked against the position
// the naive scan reports, which is a stronger statement than "the walk
// stopped somewhere sensible": with the 'e' shapes in play, several distinct
// (segment, local) pairs denote the same logical position and only a
// correctly normalised result compares equal to iter_at.
//
// Range 2 is one element longer than range 1 and ends in a value found
// nowhere else, so a walker that reads one element past last1 sees range 1's
// guard against it and reports a mismatch that should not exist.
//////////////////////////////////////////////////////////////////////////////

const int mismatch_shape_tail = 12345;

struct mismatch_shape_check
{
   std::size_t bad_pos;

   explicit mismatch_shape_check(std::size_t p) : bad_pos(p) {}

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
      typedef typename C2::iterator iter2_t;

      const boost::container::vector<int> f1 = test_detail::flatten_n_ints(c1, n1);
      const boost::container::vector<int> f2 = test_detail::flatten_n_ints(c2, n2);

      std::size_t k = f1.size();
      for(std::size_t i = 0; i != f1.size(); ++i) {
         if(f1[i] != f2[i]) { k = i; break; }
      }

      const iter1_t first1 = c1.begin();
      const iter1_t last1  = test_detail::iter_at(c1, n1);
      const iter2_t last2  = test_detail::iter_at(c2, n2);

      // Unbounded second range.
      std::pair<iter1_t, iter2_t> r = segmented_mismatch(first1, last1, c2.begin());
      if(!BOOST_TEST(r.first == test_detail::iter_at(c1, k)))
         this->report(s1, n1, s2);
      if(!BOOST_TEST(r.second == test_detail::iter_at(c2, k)))
         this->report(s1, n1, s2);

      // Second range bounded too.  n2 > n1, so the common prefix is still n1
      // and the answer must not change.
      r = segmented_mismatch(first1, last1, c2.begin(), last2);
      if(!BOOST_TEST(r.first == test_detail::iter_at(c1, k)))
         this->report(s1, n1, s2);
      if(!BOOST_TEST(r.second == test_detail::iter_at(c2, k)))
         this->report(s1, n1, s2);

      // The mirror image: range 2 bounded short, so the walk stops on the
      // second range whenever the ranges agree that far.
      const std::size_t half = n1 / 2u;
      const std::size_t k_half = k < half ? k : half;
      r = segmented_mismatch(first1, last1, c2.begin(), test_detail::iter_at(c2, half));
      if(!BOOST_TEST(r.first == test_detail::iter_at(c1, k_half)))
         this->report(s1, n1, s2);
      if(!BOOST_TEST(r.second == test_detail::iter_at(c2, k_half)))
         this->report(s1, n1, s2);

      if(!BOOST_TEST(test_detail::filler_intact(c1, n1, -999)))
         this->report(s1, n1, s2);
      if(!BOOST_TEST(test_detail::filler_intact(c2, n2, -999)))
         this->report(s1, n1, s2);
   }
};

void test_mismatch_shape_matrix()
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
         v2[n1] = mismatch_shape_tail;
         if(bad != n1)
            v2[bad] = -7;

         test_detail::for_each_shape2_all<int, int>
            (v1, n1, v2, n2, -999, mismatch_shape_check(bad));
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Predicate application count.
//
// [mismatch] mandates "At most last1 - first1 applications of the
// corresponding predicate", and min(last1 - first1, last2 - first2) for the
// four-iterator form.  The lower bound below is what stops the check from
// passing vacuously: the returned position cannot be known before the element
// at it has been looked at.
//////////////////////////////////////////////////////////////////////////////

struct mismatch_eq_int
{
   bool operator()(int a, int b) const { return a == b; }
};

struct mismatch_count_check
{
   // Index of the element of range 2 that was corrupted, or n1 for none.
   std::size_t bad_pos;

   explicit mismatch_count_check(std::size_t p) : bad_pos(p) {}

   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      const std::size_t k = bad_pos < n1 ? bad_pos : n1;

      {
         test_detail::op_counter calls;
         segmented_mismatch(c1.begin(), test_detail::iter_at(c1, n1), c2.begin(),
                            test_detail::counting_pred(calls, mismatch_eq_int()));
         BOOST_TEST(calls.n <= n1);
         BOOST_TEST(calls.n >= (k < n1 ? k + 1u : n1));
      }
      {
         test_detail::op_counter calls;
         segmented_mismatch(c1.begin(), test_detail::iter_at(c1, n1),
                            c2.begin(), test_detail::iter_at(c2, n2),
                            test_detail::counting_pred(calls, mismatch_eq_int()));
         BOOST_TEST(calls.n <= (n1 < n2 ? n1 : n2));
         BOOST_TEST(calls.n >= (k < n1 ? k + 1u : n1));
      }
      {
         // Second range bounded short, so the walk can also stop on it.
         const std::size_t half = n1 / 2u;
         const std::size_t k_half = k < half ? k : half;

         test_detail::op_counter calls;
         segmented_mismatch(c1.begin(), test_detail::iter_at(c1, n1),
                            c2.begin(), test_detail::iter_at(c2, half),
                            test_detail::counting_pred(calls, mismatch_eq_int()));
         BOOST_TEST(calls.n <= half);
         BOOST_TEST(calls.n >= (k_half < half ? k_half + 1u : half));
      }

      BOOST_TEST(s1 != 0 && s2 != 0);
   }
};

void test_mismatch_predicate_count()
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
         v2[n1] = mismatch_shape_tail;
         if(bad != n1)
            v2[bad] = -7;

         test_detail::for_each_shape2_all<int, int>
            (v1, n1, v2, n2, -999, mismatch_count_check(bad));
      }
   }
}

int main()
{
   test_mismatch_shape_matrix();
   test_mismatch_matching();
   test_mismatch_found();
   test_mismatch_first_segment();
   test_mismatch_empty();
   test_mismatch_single_segment();
   test_mismatch_non_segmented();
   test_mismatch_sentinel_segmented();
   test_mismatch_sentinel_non_segmented();
   test_mismatch_seg2();
   test_mismatch_every_position();
   test_mismatch_every_position_seg2();
   test_mismatch_seg_to_seg();
   test_mismatch_seg_to_seg_mismatch();
   test_mismatch_seg2_to_seg2();
   test_mismatch_seg_to_seg_misaligned();
   test_mismatch_seg_to_seg_every_position();

   // Two-range (bounded on both sides) overloads:
   test_mismatch_2r_equal_same_length();
   test_mismatch_2r_second_shorter();
   test_mismatch_2r_first_shorter();
   test_mismatch_2r_mismatch_within_common();
   test_mismatch_2r_both_empty();
   test_mismatch_2r_first_empty();
   test_mismatch_2r_second_empty();
   test_mismatch_2r_with_pred();
   test_mismatch_2r_non_segmented();
   test_mismatch_2r_sentinel_first_range();
   test_mismatch_2r_seg2();
   test_mismatch_2r_seg_to_seg_equal();
   test_mismatch_2r_seg_to_seg_second_shorter();
   test_mismatch_2r_seg_to_seg_mismatch_straddle();
   test_mismatch_2r_seg2_to_seg2();
   test_mismatch_2r_every_position();
   test_mismatch_2r_seg_to_seg_every_position();

   // Single-segment coverage, unbounded second range:
   test_mismatch_single_segment_full_range();
   test_mismatch_single_segment_interior_bounds();
   test_mismatch_single_segment_empty_range();
   test_mismatch_single_segment_sentinel();
   test_mismatch_single_segment_pred();
   test_mismatch_single_segment_seg2_outer();
   test_mismatch_single_segment_seg2_both_levels();
   test_mismatch_single_segment_second_range();
   test_mismatch_single_segment_first_range();
   test_mismatch_single_segment_both_ranges();

   // Single-segment coverage, two-range overloads:
   test_mismatch_2r_single_segment_first_range();
   test_mismatch_2r_single_segment_empty_range();
   test_mismatch_2r_single_segment_sentinel();
   test_mismatch_2r_single_segment_seg2();
   test_mismatch_2r_single_segment_second_range();
   test_mismatch_2r_single_segment_both_ranges();
   test_mismatch_2r_single_segment_unequal_lengths();
   test_mismatch_2r_single_segment_with_pred();
   test_mismatch_2r_single_segment_second_range_seg2();
   test_mismatch_2r_single_segment_every_position();

   test_mismatch_predicate_count();
   return boost::report_errors();
}
