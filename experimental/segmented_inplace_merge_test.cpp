//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_inplace_merge.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>
#include <functional>

using namespace boost::container;

void test_inplace_merge_basic()
{
   std::vector<int> v;
   v.push_back(1); v.push_back(3); v.push_back(5);
   v.push_back(2); v.push_back(4); v.push_back(6);

   segmented_inplace_merge(v.begin(), v.begin() + 3, v.end());

   BOOST_TEST_EQ(v[0], 1);
   BOOST_TEST_EQ(v[1], 2);
   BOOST_TEST_EQ(v[2], 3);
   BOOST_TEST_EQ(v[3], 4);
   BOOST_TEST_EQ(v[4], 5);
   BOOST_TEST_EQ(v[5], 6);
}

void test_inplace_merge_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 4, 7};
   int a2[] = {2, 3, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 4);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t mid = sv.begin();
   for(int i = 0; i < 3; ++i) ++mid;

   segmented_inplace_merge(sv.begin(), mid, sv.end());

   iter_t it = sv.begin();
   BOOST_TEST_EQ(*it, 1); ++it;
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 3); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 5); ++it;
   BOOST_TEST_EQ(*it, 6); ++it;
   BOOST_TEST_EQ(*it, 7);
}

void test_inplace_merge_empty_first()
{
   std::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);
   segmented_inplace_merge(v.begin(), v.begin(), v.end());
   BOOST_TEST_EQ(v[0], 1);
   BOOST_TEST_EQ(v[1], 2);
   BOOST_TEST_EQ(v[2], 3);
}

void test_inplace_merge_empty_second()
{
   std::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);
   segmented_inplace_merge(v.begin(), v.end(), v.end());
   BOOST_TEST_EQ(v[0], 1);
   BOOST_TEST_EQ(v[1], 2);
   BOOST_TEST_EQ(v[2], 3);
}

struct greater_int
{
   bool operator()(int a, int b) const { return a > b; }
};

void test_inplace_merge_with_comp()
{
   std::vector<int> v;
   v.push_back(5); v.push_back(3); v.push_back(1);
   v.push_back(6); v.push_back(4); v.push_back(2);

   segmented_inplace_merge(v.begin(), v.begin() + 3, v.end(), greater_int());

   BOOST_TEST_EQ(v[0], 6);
   BOOST_TEST_EQ(v[1], 5);
   BOOST_TEST_EQ(v[2], 4);
   BOOST_TEST_EQ(v[3], 3);
   BOOST_TEST_EQ(v[4], 2);
   BOOST_TEST_EQ(v[5], 1);
}

void test_inplace_merge_single_elements()
{
   std::vector<int> v;
   v.push_back(3); v.push_back(1);
   segmented_inplace_merge(v.begin(), v.begin() + 1, v.end());
   BOOST_TEST_EQ(v[0], 1);
   BOOST_TEST_EQ(v[1], 3);
}

void test_inplace_merge_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 3, 5};
   int a2[] = {2, 4, 6};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t mid = sv2.begin();
   for(int i = 0; i < 3; ++i) ++mid;

   segmented_inplace_merge(sv2.begin(), mid, sv2.end());

   iter_t it = sv2.begin();
   for(int i = 1; i <= 6; ++i, ++it)
      BOOST_TEST_EQ(*it, i);
}

int main()
{
   test_inplace_merge_basic();
   test_inplace_merge_segmented();
   test_inplace_merge_empty_first();
   test_inplace_merge_empty_second();
   test_inplace_merge_with_comp();
   test_inplace_merge_single_elements();
   test_inplace_merge_seg2();
   return boost::report_errors();
}
