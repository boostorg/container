//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_search.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

void test_search_found()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   int pattern[] = {3, 4, 5};
   test_detail::seg_vector<int>::iterator it =
      segmented_search(sv.begin(), sv.end(), pattern, pattern + 3);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_search_not_found()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   int pattern[] = {3, 5};
   test_detail::seg_vector<int>::iterator it =
      segmented_search(sv.begin(), sv.end(), pattern, pattern + 2);
   BOOST_TEST(it == sv.end());
}

void test_search_empty_pattern()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   int* empty = a;
   test_detail::seg_vector<int>::iterator it =
      segmented_search(sv.begin(), sv.end(), empty, empty);
   BOOST_TEST(it == sv.begin());
}

void test_search_at_end()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2};
   int a2[] = {3, 4};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   int pattern[] = {3, 4};
   test_detail::seg_vector<int>::iterator it =
      segmented_search(sv.begin(), sv.end(), pattern, pattern + 2);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_search_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   std::vector<int> v(src, src + 5);
   int pattern[] = {2, 3, 4};
   std::vector<int>::iterator it = segmented_search(v.begin(), v.end(), pattern, pattern + 3);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   int pattern[] = {3, 4, 5};
   test_detail::seg_vector<int>::iterator it =
      segmented_search(sv.begin(), test_detail::make_sentinel(sv.end()),
                       pattern, test_detail::make_sentinel(pattern + 3));
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_search_sentinel_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   std::vector<int> v(src, src + 5);
   int pattern[] = {2, 3, 4};
   std::vector<int>::iterator it =
      segmented_search(v.begin(), test_detail::make_sentinel(v.end()),
                       pattern, test_detail::make_sentinel(pattern + 3));
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int pattern[] = {4, 5, 6};
   test_detail::seg2_vector<int>::iterator it =
      segmented_search(sv2.begin(), sv2.end(), pattern, pattern + 3);
   BOOST_TEST(it != sv2.end());
   BOOST_TEST_EQ(*it, 4);
}

int main()
{
   test_search_found();
   test_search_not_found();
   test_search_empty_pattern();
   test_search_at_end();
   test_search_non_segmented();
   test_search_sentinel_segmented();
   test_search_sentinel_non_segmented();
   test_search_seg2();
   return boost::report_errors();
}
