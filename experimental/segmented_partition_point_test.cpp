//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_partition_point.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

struct less_than_5 {
   bool operator()(int x) const { return x < 5; }
};

void test_partition_point_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 2);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t pp = segmented_partition_point(sv.begin(), sv.end(), less_than_5());
   BOOST_TEST_EQ(*pp, 5);
}

void test_partition_point_all_true()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4};
   sv.add_segment_range(a, a + 4);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t pp = segmented_partition_point(sv.begin(), sv.end(), less_than_5());
   BOOST_TEST(pp == sv.end());
}

void test_partition_point_all_false()
{
   test_detail::seg_vector<int> sv;
   int a[] = {5, 6, 7};
   sv.add_segment_range(a, a + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t pp = segmented_partition_point(sv.begin(), sv.end(), less_than_5());
   BOOST_TEST(pp == sv.begin());
}

void test_partition_point_empty()
{
   test_detail::seg_vector<int> sv;
   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t pp = segmented_partition_point(sv.begin(), sv.end(), less_than_5());
   BOOST_TEST(pp == sv.end());
}

void test_partition_point_non_segmented()
{
   std::vector<int> v;
   v.push_back(1); v.push_back(3); v.push_back(4);
   v.push_back(7); v.push_back(8); v.push_back(9);

   std::vector<int>::iterator pp = segmented_partition_point(v.begin(), v.end(), less_than_5());
   BOOST_TEST_EQ(*pp, 7);
}

void test_partition_point_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 2);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t pp = segmented_partition_point(sv.begin(), test_detail::make_sentinel(sv.end()), less_than_5());
   BOOST_TEST_EQ(*pp, 5);
}

void test_partition_point_sentinel_non_segmented()
{
   std::vector<int> v;
   v.push_back(1); v.push_back(3); v.push_back(4);
   v.push_back(7); v.push_back(8); v.push_back(9);

   std::vector<int>::iterator pp = segmented_partition_point(v.begin(), test_detail::make_sentinel(v.end()), less_than_5());
   BOOST_TEST_EQ(*pp, 7);
}

void test_partition_point_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);
   sv2.add_flat_segment_range(a3, a3 + 2);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t pp = segmented_partition_point(sv2.begin(), sv2.end(), less_than_5());
   BOOST_TEST_EQ(*pp, 5);
}

int main()
{
   test_partition_point_segmented();
   test_partition_point_all_true();
   test_partition_point_all_false();
   test_partition_point_empty();
   test_partition_point_non_segmented();
   test_partition_point_sentinel_segmented();
   test_partition_point_sentinel_non_segmented();
   test_partition_point_seg2();
   return boost::report_errors();
}
