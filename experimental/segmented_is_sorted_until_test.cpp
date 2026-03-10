//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_is_sorted_until.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

void test_is_sorted_until_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 2, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it = segmented_is_sorted_until(sv.begin(), sv.end());
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_is_sorted_until_boundary()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 5};
   int a2[] = {3, 7};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int>::iterator it = segmented_is_sorted_until(sv.begin(), sv.end());
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_is_sorted_until_all_sorted()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2};
   int a2[] = {3, 4};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int>::iterator it = segmented_is_sorted_until(sv.begin(), sv.end());
   BOOST_TEST(it == sv.end());
}

void test_is_sorted_until_empty()
{
   test_detail::seg_vector<int> sv;
   test_detail::seg_vector<int>::iterator it = segmented_is_sorted_until(sv.begin(), sv.end());
   BOOST_TEST(it == sv.end());
}

void test_is_sorted_until_single()
{
   test_detail::seg_vector<int> sv;
   sv.add_segment(1, 42);
   test_detail::seg_vector<int>::iterator it = segmented_is_sorted_until(sv.begin(), sv.end());
   BOOST_TEST(it == sv.end());
}

struct greater_comp
{
   bool operator()(int a, int b) const { return a > b; }
};

void test_is_sorted_until_with_comp()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {9, 7, 5};
   int a2[] = {3, 1};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int>::iterator it =
      segmented_is_sorted_until(sv.begin(), sv.end(), greater_comp());
   BOOST_TEST(it == sv.end());
}

void test_is_sorted_until_non_segmented()
{
   int data[] = {1, 2, 5, 3, 4};
   std::vector<int> v(data, data + 5);
   std::vector<int>::iterator it = segmented_is_sorted_until(v.begin(), v.end());
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_is_sorted_until_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 2, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_is_sorted_until(sv.begin(), test_detail::make_sentinel(sv.end()));
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_is_sorted_until_sentinel_non_segmented()
{
   int data[] = {1, 2, 5, 3, 4};
   std::vector<int> v(data, data + 5);
   std::vector<int>::iterator it =
      segmented_is_sorted_until(v.begin(), test_detail::make_sentinel(v.end()));
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_is_sorted_until_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 2, 6};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);

   test_detail::seg2_vector<int>::iterator it =
      segmented_is_sorted_until(sv2.begin(), sv2.end());
   BOOST_TEST(it != sv2.end());
   BOOST_TEST_EQ(*it, 2);
}

int main()
{
   test_is_sorted_until_segmented();
   test_is_sorted_until_boundary();
   test_is_sorted_until_all_sorted();
   test_is_sorted_until_empty();
   test_is_sorted_until_single();
   test_is_sorted_until_with_comp();
   test_is_sorted_until_non_segmented();
   test_is_sorted_until_sentinel_segmented();
   test_is_sorted_until_sentinel_non_segmented();
   test_is_sorted_until_seg2();
   return boost::report_errors();
}
