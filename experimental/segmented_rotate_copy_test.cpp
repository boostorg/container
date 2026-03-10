//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_rotate_copy.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

void test_rotate_copy_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 2);

   // Rotate around element 4 (index 3)
   test_detail::seg_vector<int>::iterator n_first = sv.begin();
   ++n_first; ++n_first; ++n_first; // points to 4

   std::vector<int> out(7, 0);
   std::vector<int>::iterator r = segmented_rotate_copy(sv.begin(), n_first, sv.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 7u);

   int expected[] = {4, 5, 6, 7, 1, 2, 3};
   for(std::size_t i = 0; i < 7; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

void test_rotate_copy_at_begin()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2};
   int a2[] = {3, 4};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   std::vector<int> out(4, 0);
   std::vector<int>::iterator r = segmented_rotate_copy(sv.begin(), sv.begin(), sv.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 4u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 3);
   BOOST_TEST_EQ(out[3], 4);
}

void test_rotate_copy_at_end()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2};
   int a2[] = {3, 4};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   std::vector<int> out(4, 0);
   std::vector<int>::iterator r = segmented_rotate_copy(sv.begin(), sv.end(), sv.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 4u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 3);
   BOOST_TEST_EQ(out[3], 4);
}

void test_rotate_copy_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   std::vector<int> v(src, src + 5);

   std::vector<int> out(5, 0);
   std::vector<int>::iterator r = segmented_rotate_copy(v.begin(), v.begin() + 2, v.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 5u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 5);
   BOOST_TEST_EQ(out[3], 1);
   BOOST_TEST_EQ(out[4], 2);
}

void test_rotate_copy_empty()
{
   test_detail::seg_vector<int> sv;
   std::vector<int> out;
   std::vector<int>::iterator r = segmented_rotate_copy(sv.begin(), sv.begin(), sv.end(), out.begin());
   BOOST_TEST(r == out.begin());
}

void test_rotate_copy_segmented_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 2);

   test_detail::seg_vector<int>::iterator n_first = sv.begin();
   ++n_first; ++n_first; ++n_first;

   std::vector<int> out(7, 0);
   std::vector<int>::iterator r = segmented_rotate_copy(sv.begin(), n_first, test_detail::make_sentinel(sv.end()), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 7u);

   int expected[] = {4, 5, 6, 7, 1, 2, 3};
   for(std::size_t i = 0; i < 7; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

void test_rotate_copy_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   test_detail::seg2_vector<int>::iterator n_first = sv2.begin();
   ++n_first; ++n_first; ++n_first; // points to 4

   std::vector<int> out(9, 0);
   std::vector<int>::iterator r = segmented_rotate_copy(sv2.begin(), n_first, sv2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 9u);

   int expected[] = {4, 5, 6, 7, 8, 9, 1, 2, 3};
   for(std::size_t i = 0; i < 9; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

int main()
{
   test_rotate_copy_segmented();
   test_rotate_copy_at_begin();
   test_rotate_copy_at_end();
   test_rotate_copy_non_segmented();
   test_rotate_copy_empty();
   test_rotate_copy_segmented_sentinel();
   test_rotate_copy_seg2();
   return boost::report_errors();
}
