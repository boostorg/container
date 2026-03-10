//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_shift_left.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

void test_shift_left_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_shift_left(sv.begin(), sv.end(), 2);

   int expected[] = {3, 4, 5, 6};
   iter_t it = sv.begin();
   for(int i = 0; i < 4; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST(it == new_end);
}

void test_shift_left_zero()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_shift_left(sv.begin(), sv.end(), 0);
   BOOST_TEST(new_end == sv.end());
   BOOST_TEST_EQ(*sv.begin(), 1);
}

void test_shift_left_exceeds_size()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_shift_left(sv.begin(), sv.end(), 5);
   BOOST_TEST(new_end == sv.begin());
}

void test_shift_left_empty()
{
   test_detail::seg_vector<int> sv;
   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_shift_left(sv.begin(), sv.end(), 1);
   BOOST_TEST(new_end == sv.begin());
}

void test_shift_left_non_segmented()
{
   std::vector<int> v;
   v.push_back(10); v.push_back(20); v.push_back(30); v.push_back(40); v.push_back(50);

   std::vector<int>::iterator new_end = segmented_shift_left(v.begin(), v.end(), 2);
   BOOST_TEST_EQ(new_end - v.begin(), 3);
   BOOST_TEST_EQ(v[0], 30);
   BOOST_TEST_EQ(v[1], 40);
   BOOST_TEST_EQ(v[2], 50);
}

void test_shift_left_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_shift_left(sv.begin(), test_detail::make_sentinel(sv.end()), 2);

   int expected[] = {3, 4, 5, 6};
   iter_t it = sv.begin();
   for(int i = 0; i < 4; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST(it == new_end);
}

void test_shift_left_sentinel_non_segmented()
{
   std::vector<int> v;
   v.push_back(10); v.push_back(20); v.push_back(30); v.push_back(40); v.push_back(50);

   std::vector<int>::iterator new_end = segmented_shift_left(v.begin(), test_detail::make_sentinel(v.end()), 2);
   BOOST_TEST_EQ(new_end - v.begin(), 3);
   BOOST_TEST_EQ(v[0], 30);
   BOOST_TEST_EQ(v[1], 40);
   BOOST_TEST_EQ(v[2], 50);
}

void test_shift_left_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t new_end = segmented_shift_left(sv2.begin(), sv2.end(), 2);

   int expected[] = {3, 4, 5, 6, 7, 8, 9};
   iter_t it = sv2.begin();
   for(int i = 0; i < 7; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST(it == new_end);
}

int main()
{
   test_shift_left_segmented();
   test_shift_left_zero();
   test_shift_left_exceeds_size();
   test_shift_left_empty();
   test_shift_left_non_segmented();
   test_shift_left_sentinel_segmented();
   test_shift_left_sentinel_non_segmented();
   test_shift_left_seg2();
   return boost::report_errors();
}
