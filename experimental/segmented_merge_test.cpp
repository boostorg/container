//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_merge.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_merge_segmented_inputs()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {1, 3};
   int a2[] = {5, 7};
   sv1.add_segment_range(a1, a1 + 2);
   sv1.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int> sv2;
   int b1[] = {2, 4};
   int b2[] = {6, 8};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 2);

   boost::container::vector<int> out(8, 0);
   boost::container::vector<int>::iterator r = segmented_merge(
      sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 8u);
   for(int i = 0; i < 8; ++i)
      BOOST_TEST_EQ(out[static_cast<std::size_t>(i)], i + 1);
}

void test_merge_first_empty()
{
   test_detail::seg_vector<int> sv1;
   test_detail::seg_vector<int> sv2;
   int a[] = {1, 2, 3};
   sv2.add_segment_range(a, a + 3);

   boost::container::vector<int> out(3, 0);
   boost::container::vector<int>::iterator r = segmented_merge(
      sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 3);
}

void test_merge_second_empty()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {1, 2, 3};
   sv1.add_segment_range(a, a + 3);
   test_detail::seg_vector<int> sv2;

   boost::container::vector<int> out(3, 0);
   boost::container::vector<int>::iterator r = segmented_merge(
      sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 3);
}

void test_merge_both_empty()
{
   test_detail::seg_vector<int> sv1;
   test_detail::seg_vector<int> sv2;
   boost::container::vector<int> out;
   boost::container::vector<int>::iterator r = segmented_merge(
      sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), out.begin());
   BOOST_TEST(r == out.begin());
}

struct greater_comp
{
   bool operator()(int a, int b) const { return a > b; }
};

void test_merge_with_comp()
{
   int a[] = {5, 3, 1};
   int b[] = {6, 4, 2};
   boost::container::vector<int> v1(a, a + 3);
   boost::container::vector<int> v2(b, b + 3);
   boost::container::vector<int> out(6, 0);

   boost::container::vector<int>::iterator r = segmented_merge(
      v1.begin(), v1.end(), v2.begin(), v2.end(), out.begin(), greater_comp());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 6u);
   BOOST_TEST_EQ(out[0], 6);
   BOOST_TEST_EQ(out[1], 5);
   BOOST_TEST_EQ(out[2], 4);
   BOOST_TEST_EQ(out[3], 3);
   BOOST_TEST_EQ(out[4], 2);
   BOOST_TEST_EQ(out[5], 1);
}

void test_merge_non_segmented()
{
   int a[] = {1, 3, 5};
   int b[] = {2, 4, 6};
   boost::container::vector<int> v1(a, a + 3);
   boost::container::vector<int> v2(b, b + 3);
   boost::container::vector<int> out(6, 0);

   boost::container::vector<int>::iterator r = segmented_merge(
      v1.begin(), v1.end(), v2.begin(), v2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 6u);
   for(int i = 0; i < 6; ++i)
      BOOST_TEST_EQ(out[static_cast<std::size_t>(i)], i + 1);
}

void test_merge_duplicates()
{
   int a[] = {1, 2, 3};
   int b[] = {2, 3, 4};
   boost::container::vector<int> v1(a, a + 3);
   boost::container::vector<int> v2(b, b + 3);
   boost::container::vector<int> out(6, 0);

   boost::container::vector<int>::iterator r = segmented_merge(
      v1.begin(), v1.end(), v2.begin(), v2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 6u);
   int expected[] = {1, 2, 2, 3, 3, 4};
   for(std::size_t i = 0; i < 6; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

void test_merge_sentinel_segmented()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {1, 3};
   int a2[] = {5, 7};
   sv1.add_segment_range(a1, a1 + 2);
   sv1.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int> sv2;
   int b1[] = {2, 4};
   int b2[] = {6, 8};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 2);

   boost::container::vector<int> out(8, 0);
   boost::container::vector<int>::iterator r = segmented_merge(
      sv1.begin(), test_detail::make_sentinel(sv1.end()),
      sv2.begin(), test_detail::make_sentinel(sv2.end()), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 8u);
   for(int i = 0; i < 8; ++i)
      BOOST_TEST_EQ(out[static_cast<std::size_t>(i)], i + 1);
}

void test_merge_sentinel_non_segmented()
{
   int a[] = {1, 3, 5};
   int b[] = {2, 4, 6};
   boost::container::vector<int> v1(a, a + 3);
   boost::container::vector<int> v2(b, b + 3);
   boost::container::vector<int> out(6, 0);

   boost::container::vector<int>::iterator r = segmented_merge(
      v1.begin(), test_detail::make_sentinel(v1.end()),
      v2.begin(), test_detail::make_sentinel(v2.end()), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 6u);
   for(int i = 0; i < 6; ++i)
      BOOST_TEST_EQ(out[static_cast<std::size_t>(i)], i + 1);
}

void test_merge_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 3, 5};
   int a2[] = {7, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);

   int b[] = {2, 4, 6, 8};
   boost::container::vector<int> out(9, 0);
   boost::container::vector<int>::iterator r = segmented_merge(
      sv2.begin(), sv2.end(), b, b + 4, out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 9u);
   for(int i = 0; i < 9; ++i)
      BOOST_TEST_EQ(out[static_cast<std::size_t>(i)], i + 1);
}

int main()
{
   test_merge_segmented_inputs();
   test_merge_first_empty();
   test_merge_second_empty();
   test_merge_both_empty();
   test_merge_with_comp();
   test_merge_non_segmented();
   test_merge_duplicates();
   test_merge_sentinel_segmented();
   test_merge_sentinel_non_segmented();
   test_merge_seg2();
   return boost::report_errors();
}
