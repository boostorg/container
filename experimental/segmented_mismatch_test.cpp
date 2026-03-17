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
#include <vector>
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
   std::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);

   int ref_match[] = {1, 2, 3};
   typedef std::vector<int>::iterator vec_it;
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
   std::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);

   int ref_match[] = {1, 2, 3};
   typedef std::vector<int>::iterator vec_it;
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

int main()
{
   test_mismatch_matching();
   test_mismatch_found();
   test_mismatch_first_segment();
   test_mismatch_empty();
   test_mismatch_single_segment();
   test_mismatch_non_segmented();
   test_mismatch_sentinel_segmented();
   test_mismatch_sentinel_non_segmented();
   test_mismatch_seg2();
   return boost::report_errors();
}
