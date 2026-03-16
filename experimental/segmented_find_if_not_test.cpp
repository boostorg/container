//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_find_if_not.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

struct is_positive
{
   bool operator()(int x) const { return x > 0; }
};

void test_find_if_not_present()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {-4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_find_if_not(sv.begin(), sv.end(), is_positive());
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, -4);
}

void test_find_if_not_not_present()
{
   test_detail::seg_vector<int> sv;
   sv.add_segment(3, 1);
   sv.add_segment(2, 2);

   test_detail::seg_vector<int>::iterator it =
      segmented_find_if_not(sv.begin(), sv.end(), is_positive());
   BOOST_TEST(it == sv.end());
}

void test_find_if_not_empty()
{
   test_detail::seg_vector<int> sv;
   test_detail::seg_vector<int>::iterator it =
      segmented_find_if_not(sv.begin(), sv.end(), is_positive());
   BOOST_TEST(it == sv.end());
}

void test_find_if_not_non_segmented()
{
   std::vector<int> v;
   v.push_back(1);
   v.push_back(-2);
   v.push_back(3);

   std::vector<int>::iterator it =
      segmented_find_if_not(v.begin(), v.end(), is_positive());
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, -2);

   v.clear();
   v.push_back(1);
   v.push_back(2);
   v.push_back(3);
   it = segmented_find_if_not(v.begin(), v.end(), is_positive());
   BOOST_TEST(it == v.end());
}

void test_find_if_not_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {-4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_find_if_not(sv.begin(), test_detail::make_sentinel(sv.end()), is_positive());
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, -4);
}

void test_find_if_not_sentinel_non_segmented()
{
   std::vector<int> v;
   v.push_back(1);
   v.push_back(-2);
   v.push_back(3);

   std::vector<int>::iterator it =
      segmented_find_if_not(v.begin(), test_detail::make_sentinel(v.end()), is_positive());
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, -2);
}

void test_find_if_not_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {-4, 5, 6};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);

   test_detail::seg2_vector<int>::iterator it =
      segmented_find_if_not(sv2.begin(), sv2.end(), is_positive());
   BOOST_TEST(it != sv2.end());
   BOOST_TEST_EQ(*it, -4);
}

int main()
{
   test_find_if_not_present();
   test_find_if_not_not_present();
   test_find_if_not_empty();
   test_find_if_not_non_segmented();
   test_find_if_not_sentinel_segmented();
   test_find_if_not_sentinel_non_segmented();
   test_find_if_not_seg2();
   return boost::report_errors();
}
