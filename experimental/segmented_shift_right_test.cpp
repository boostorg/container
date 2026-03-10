//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_shift_right.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

void test_shift_right_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_begin = segmented_shift_right(sv.begin(), sv.end(), 2);

   // Elements [1,2,3,4] shifted right by 2: positions 2..5 = {1,2,3,4}
   int expected[] = {1, 2, 3, 4};
   iter_t it = new_begin;
   for(int i = 0; i < 4; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST(it == sv.end());
}

void test_shift_right_zero()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_begin = segmented_shift_right(sv.begin(), sv.end(), 0);
   BOOST_TEST(new_begin == sv.begin());
   BOOST_TEST_EQ(*sv.begin(), 1);
}

void test_shift_right_exceeds_size()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_begin = segmented_shift_right(sv.begin(), sv.end(), 5);
   BOOST_TEST(new_begin == sv.end());
}

void test_shift_right_empty()
{
   test_detail::seg_vector<int> sv;
   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_begin = segmented_shift_right(sv.begin(), sv.end(), 1);
   BOOST_TEST(new_begin == sv.end());
}

void test_shift_right_non_segmented()
{
   std::vector<int> v;
   v.push_back(10); v.push_back(20); v.push_back(30); v.push_back(40); v.push_back(50);

   std::vector<int>::iterator new_begin = segmented_shift_right(v.begin(), v.end(), 2);
   BOOST_TEST_EQ(new_begin - v.begin(), 2);
   BOOST_TEST_EQ(v[2], 10);
   BOOST_TEST_EQ(v[3], 20);
   BOOST_TEST_EQ(v[4], 30);
}

void test_shift_right_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t new_begin = segmented_shift_right(sv2.begin(), sv2.end(), 2);

   int expected[] = {1, 2, 3, 4, 5, 6, 7};
   iter_t it = new_begin;
   for(int i = 0; i < 7; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST(it == sv2.end());
}

int main()
{
   test_shift_right_segmented();
   test_shift_right_zero();
   test_shift_right_exceeds_size();
   test_shift_right_empty();
   test_shift_right_non_segmented();
   test_shift_right_seg2();
   return boost::report_errors();
}
