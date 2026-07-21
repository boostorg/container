//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_transform.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct times_two
{
   int operator()(int x) const { return x * 2; }
};

struct negate_val
{
   int operator()(int x) const { return -x; }
};

void test_transform_full_range()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   boost::container::vector<int> out(5, 0);
   boost::container::vector<int>::iterator result =
      segmented_transform(sv.begin(), sv.end(), out.begin(), times_two());

   BOOST_TEST(result == out.end());
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
   BOOST_TEST_EQ(out[3], 8);
   BOOST_TEST_EQ(out[4], 10);
}

void test_transform_empty()
{
   test_detail::seg_vector<int> sv;
   boost::container::vector<int> out;
   boost::container::vector<int>::iterator result =
      segmented_transform(sv.begin(), sv.end(), out.begin(), times_two());
   BOOST_TEST(result == out.begin());
}

void test_transform_single_segment()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30};
   sv.add_segment_range(a, a + 3);

   boost::container::vector<int> out(3, 0);
   segmented_transform(sv.begin(), sv.end(), out.begin(), negate_val());

   BOOST_TEST_EQ(out[0], -10);
   BOOST_TEST_EQ(out[1], -20);
   BOOST_TEST_EQ(out[2], -30);
}

void test_transform_non_segmented()
{
   boost::container::vector<int> in;
   in.push_back(1);
   in.push_back(2);
   in.push_back(3);

   boost::container::vector<int> out(3, 0);
   segmented_transform(in.begin(), in.end(), out.begin(), times_two());

   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
}

void test_transform_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   boost::container::vector<int> out(5, 0);
   boost::container::vector<int>::iterator result =
      segmented_transform(sv.begin(), test_detail::make_sentinel(sv.end()), out.begin(), times_two());

   BOOST_TEST(result == out.end());
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
   BOOST_TEST_EQ(out[3], 8);
   BOOST_TEST_EQ(out[4], 10);
}

void test_transform_sentinel_non_segmented()
{
   boost::container::vector<int> in;
   in.push_back(1);
   in.push_back(2);
   in.push_back(3);

   boost::container::vector<int> out(3, 0);
   segmented_transform(in.begin(), test_detail::make_sentinel(in.end()), out.begin(), times_two());

   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
}

void test_transform_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   boost::container::vector<int> out(9, 0);
   boost::container::vector<int>::iterator result =
      segmented_transform(sv2.begin(), sv2.end(), out.begin(), times_two());

   BOOST_TEST(result == out.end());
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 6);
   BOOST_TEST_EQ(out[3], 8);
   BOOST_TEST_EQ(out[4], 10);
   BOOST_TEST_EQ(out[5], 12);
   BOOST_TEST_EQ(out[6], 14);
   BOOST_TEST_EQ(out[7], 16);
   BOOST_TEST_EQ(out[8], 18);
}

void test_transform_segmented_output()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   test_detail::seg_vector<int> out;
   out.add_segment(4, 0);
   out.add_segment(3, 0);
   out.add_segment(2, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_transform(sv.begin(), sv.end(), out.begin(), times_two());

   BOOST_TEST(result == out.end());
   iter_t it = out.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, (i + 1) * 2);
}

void test_transform_seg2_to_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   test_detail::seg2_vector<int> out;
   {
      test_detail::seg_vector<int> s1; s1.add_segment(5, 0);
      test_detail::seg_vector<int> s2; s2.add_segment(4, 0);
      out.add_segment(s1);
      out.add_segment(s2);
   }

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t result = segmented_transform(sv2.begin(), sv2.end(), out.begin(), times_two());

   BOOST_TEST(result == out.end());
   iter_t it = out.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, (i + 1) * 2);
}

void test_transform_seg_to_seg_misaligned()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3, 4, 5};
   int a2[] = {6, 7, 8};
   sv.add_segment_range(a1, a1 + 5);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int> out;
   out.add_segment(2, 0);
   out.add_segment(3, 0);
   out.add_segment(3, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_transform(sv.begin(), sv.end(), out.begin(), negate_val());

   BOOST_TEST(result == out.end());
   iter_t it = out.begin();
   for(int i = 0; i < 8; ++i, ++it)
      BOOST_TEST_EQ(*it, -(i + 1));
}

int main()
{
   test_transform_full_range();
   test_transform_empty();
   test_transform_single_segment();
   test_transform_non_segmented();
   test_transform_sentinel_segmented();
   test_transform_sentinel_non_segmented();
   test_transform_seg2();
   test_transform_segmented_output();
   test_transform_seg2_to_seg2();
   test_transform_seg_to_seg_misaligned();
   return boost::report_errors();
}
