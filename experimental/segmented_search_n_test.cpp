//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_search_n.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

void test_search_n_found()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 2};
   int a2[] = {2, 3};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int>::iterator it =
      segmented_search_n(sv.begin(), sv.end(), 3, 2);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_n_not_found()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 2};
   int a2[] = {3, 2};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int>::iterator it =
      segmented_search_n(sv.begin(), sv.end(), 3, 2);
   BOOST_TEST(it == sv.end());
}

void test_search_n_zero_count()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_search_n(sv.begin(), sv.end(), 0, 99);
   BOOST_TEST(it == sv.begin());
}

void test_search_n_non_segmented()
{
   int src[] = {1, 2, 2, 2, 3};
   std::vector<int> v(src, src + 5);
   std::vector<int>::iterator it = segmented_search_n(v.begin(), v.end(), 3, 2);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 2);
   BOOST_TEST_EQ(static_cast<std::size_t>(it - v.begin()), 1u);
}

void test_search_n_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 2};
   int a2[] = {2, 3};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int>::iterator it =
      segmented_search_n(sv.begin(), test_detail::make_sentinel(sv.end()), 3, 2);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_n_sentinel_non_segmented()
{
   int src[] = {1, 2, 2, 2, 3};
   std::vector<int> v(src, src + 5);
   std::vector<int>::iterator it =
      segmented_search_n(v.begin(), test_detail::make_sentinel(v.end()), 3, 2);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_n_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 2};
   int a2[] = {2, 2};
   int a3[] = {3, 4};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 2);

   test_detail::seg2_vector<int>::iterator it =
      segmented_search_n(sv2.begin(), sv2.end(), 4, 2);
   BOOST_TEST(it != sv2.end());
   BOOST_TEST_EQ(*it, 2);
}

int main()
{
   test_search_n_found();
   test_search_n_not_found();
   test_search_n_zero_count();
   test_search_n_non_segmented();
   test_search_n_sentinel_segmented();
   test_search_n_sentinel_non_segmented();
   test_search_n_seg2();
   return boost::report_errors();
}
