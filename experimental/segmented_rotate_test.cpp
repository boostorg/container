//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_rotate.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

void test_rotate_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t mid = sv.begin();
   ++mid; ++mid; // points to 3

   iter_t ret = segmented_rotate(sv.begin(), mid, sv.end());

   int expected[] = {3, 4, 5, 1, 2};
   iter_t it = sv.begin();
   for(int i = 0; i < 5; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);

   // ret should point to the element that was originally at first (value 1)
   BOOST_TEST_EQ(*ret, 1);
}

void test_rotate_at_begin()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t ret = segmented_rotate(sv.begin(), sv.begin(), sv.end());
   BOOST_TEST(ret == sv.end());
   BOOST_TEST_EQ(*sv.begin(), 1);
}

void test_rotate_at_end()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t ret = segmented_rotate(sv.begin(), sv.end(), sv.end());
   BOOST_TEST(ret == sv.begin());
   BOOST_TEST_EQ(*sv.begin(), 1);
}

void test_rotate_non_segmented()
{
   std::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3); v.push_back(4); v.push_back(5);

   std::vector<int>::iterator ret = segmented_rotate(v.begin(), v.begin() + 2, v.end());

   BOOST_TEST_EQ(v[0], 3);
   BOOST_TEST_EQ(v[1], 4);
   BOOST_TEST_EQ(v[2], 5);
   BOOST_TEST_EQ(v[3], 1);
   BOOST_TEST_EQ(v[4], 2);
   BOOST_TEST_EQ(*ret, 1);
}

void test_rotate_segmented_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t mid = sv.begin();
   ++mid; ++mid;

   iter_t ret = segmented_rotate(sv.begin(), mid, test_detail::make_sentinel(sv.end()));

   int expected[] = {3, 4, 5, 1, 2};
   iter_t it = sv.begin();
   for(int i = 0; i < 5; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);

   BOOST_TEST_EQ(*ret, 1);
}

void test_rotate_non_segmented_sentinel()
{
   std::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3); v.push_back(4); v.push_back(5);

   std::vector<int>::iterator ret = segmented_rotate(v.begin(), v.begin() + 2, test_detail::make_sentinel(v.end()));

   BOOST_TEST_EQ(v[0], 3);
   BOOST_TEST_EQ(v[1], 4);
   BOOST_TEST_EQ(v[2], 5);
   BOOST_TEST_EQ(v[3], 1);
   BOOST_TEST_EQ(v[4], 2);
   BOOST_TEST_EQ(*ret, 1);
}

void test_rotate_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t mid = sv2.begin();
   ++mid; ++mid; ++mid; // points to 4

   iter_t ret = segmented_rotate(sv2.begin(), mid, sv2.end());

   int expected[] = {4, 5, 6, 7, 8, 9, 1, 2, 3};
   iter_t it = sv2.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);

   BOOST_TEST_EQ(*ret, 1);
}

int main()
{
   test_rotate_segmented();
   test_rotate_at_begin();
   test_rotate_at_end();
   test_rotate_non_segmented();
   test_rotate_segmented_sentinel();
   test_rotate_non_segmented_sentinel();
   test_rotate_seg2();
   return boost::report_errors();
}
