//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_reverse_copy.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_reverse_copy_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 3);

   boost::container::vector<int> out(8, 0);
   boost::container::vector<int>::iterator r = segmented_reverse_copy(sv.begin(), sv.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 8u);

   int expected[] = {8, 7, 6, 5, 4, 3, 2, 1};
   for(std::size_t i = 0; i < 8; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

void test_reverse_copy_empty()
{
   test_detail::seg_vector<int> sv;
   boost::container::vector<int> out;
   boost::container::vector<int>::iterator r = segmented_reverse_copy(sv.begin(), sv.end(), out.begin());
   BOOST_TEST(r == out.begin());
}

void test_reverse_copy_single_segment()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30};
   sv.add_segment_range(a, a + 3);

   boost::container::vector<int> out(3, 0);
   boost::container::vector<int>::iterator r = segmented_reverse_copy(sv.begin(), sv.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   BOOST_TEST_EQ(out[0], 30);
   BOOST_TEST_EQ(out[1], 20);
   BOOST_TEST_EQ(out[2], 10);
}

void test_reverse_copy_single_element()
{
   test_detail::seg_vector<int> sv;
   sv.add_segment(1, 42);

   boost::container::vector<int> out(1, 0);
   segmented_reverse_copy(sv.begin(), sv.end(), out.begin());
   BOOST_TEST_EQ(out[0], 42);
}

void test_reverse_copy_non_segmented()
{
   int src[] = {1, 2, 3, 4, 5};
   boost::container::vector<int> v(src, src + 5);
   boost::container::vector<int> out(5, 0);
   boost::container::vector<int>::iterator r = segmented_reverse_copy(v.begin(), v.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 5u);
   BOOST_TEST_EQ(out[0], 5);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 3);
   BOOST_TEST_EQ(out[3], 2);
   BOOST_TEST_EQ(out[4], 1);
}

void test_reverse_copy_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   boost::container::vector<int> out(9, 0);
   boost::container::vector<int>::iterator r = segmented_reverse_copy(sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 9u);

   int expected[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
   for(std::size_t i = 0; i < 9; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

void test_reverse_copy_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator r =
      segmented_reverse_copy(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 4u);
   BOOST_TEST_EQ(out[0], 5);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 3);
   BOOST_TEST_EQ(out[3], 2);
   BOOST_TEST_EQ(out[4], 0);
   BOOST_TEST_EQ(out[5], 0);
}

void test_reverse_copy_single_segment_empty_mid()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int>::iterator mid = test_detail::iter_at(sv, 3);
   boost::container::vector<int> out(3, 99);
   boost::container::vector<int>::iterator r = segmented_reverse_copy(mid, mid, out.begin());

   BOOST_TEST(r == out.begin());
   BOOST_TEST_EQ(out[0], 99);
   BOOST_TEST_EQ(out[1], 99);
   BOOST_TEST_EQ(out[2], 99);
}

void test_reverse_copy_single_segment_seg2_outer()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 2);
   inner.add_segment_range(a3, a3 + 2);
   sv2.add_segment(inner);

   boost::container::vector<int> out(8, 0);
   boost::container::vector<int>::iterator r =
      segmented_reverse_copy(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 5u);
   int expected[] = {6, 5, 4, 3, 2};
   for(std::size_t i = 0; i < 5; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
   BOOST_TEST_EQ(out[5], 0);
}

void test_reverse_copy_single_segment_seg2_both()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a[] = {1, 2, 3, 4, 5, 6};
   inner.add_segment_range(a, a + 6);
   sv2.add_segment(inner);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator r =
      segmented_reverse_copy(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 5), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 4u);
   BOOST_TEST_EQ(out[0], 5);
   BOOST_TEST_EQ(out[1], 4);
   BOOST_TEST_EQ(out[2], 3);
   BOOST_TEST_EQ(out[3], 2);
   BOOST_TEST_EQ(out[4], 0);
}

void test_reverse_copy_single_segment_src_multi_dst()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6, 7};
   sv.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> out;
   out.add_segment(2, 0);
   out.add_segment(3, 0);
   out.add_segment(3, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_reverse_copy(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 5));
   iter_t it = out.begin();
   for(int i = 0; i < 5; ++i, ++it)
      BOOST_TEST_EQ(*it, 6 - i);
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, 0);
}

void test_reverse_copy_single_segment_src_and_dst()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int> out;
   out.add_segment(6, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_reverse_copy(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 4));
   iter_t it = out.begin();
   for(int i = 0; i < 4; ++i, ++it)
      BOOST_TEST_EQ(*it, 5 - i);
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

void test_reverse_copy_single_segment_dst_from_flat()
{
   int src[] = {1, 2, 3, 4};
   boost::container::vector<int> v(src, src + 4);

   test_detail::seg_vector<int> out;
   out.add_segment(6, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_reverse_copy(v.begin(), v.end(), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 4));
   iter_t it = out.begin();
   for(int i = 0; i < 4; ++i, ++it)
      BOOST_TEST_EQ(*it, 4 - i);
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// for_each_shape2_all walks the twelve branch specs of the source range
// against the twelve of the destination range, so the two segmentations vary
// independently: 144 pairs per size pair, over both segmentation depths.  The
// 'e' specs carry empty segments, which is the only way to reach the
// sfirst == slast branch of the destination bounded helpers.
//
// The reference answer is flatten_n_ints over the logical source range read
// backwards; flatten_all_ints is not used, as it deliberately includes the
// guard.  Size pairs with n_dst == n_src place the destination guard
// immediately after the last slot the copy may write.  Both an odd and an
// even source length are covered, since the backward walk pairs elements up.
//////////////////////////////////////////////////////////////////////////////

const int shape_filler = -999;   // guard just past the end of every range
const int shape_fill   = -1;     // every destination slot before the call

const int shape_src_vals[] = {5, 3, 9, 1, 7, 2, 8, 4, 6, 11};
const int shape_dst_vals[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

struct reverse_copy_shape_check
{
   template<class CSrc, class CDst>
   void operator()(CSrc& src, std::size_t n_src, const char* src_spec,
                   CDst& dst, std::size_t n_dst, const char* dst_spec) const
   {
      typedef typename CDst::iterator dst_iter_t;

      const boost::container::vector<int> ref = test_detail::flatten_n_ints(src, n_src);

      const dst_iter_t r = segmented_reverse_copy
         (src.begin(), test_detail::iter_at(src, n_src), dst.begin());

      BOOST_TEST(r == test_detail::iter_at(dst, n_src));
      BOOST_TEST(test_detail::filler_intact(src, n_src, shape_filler));
      BOOST_TEST(test_detail::filler_intact(dst, n_dst, shape_filler));

      const boost::container::vector<int> got = test_detail::flatten_n_ints(dst, n_dst);
      BOOST_TEST_EQ(got.size(), n_dst);
      for(std::size_t i = 0; i != n_dst; ++i)
         BOOST_TEST_EQ(got[i], i < n_src ? ref[n_src - 1u - i] : shape_fill);

      // The source is read-only, so it has to come back unchanged.
      const boost::container::vector<int> src_after = test_detail::flatten_n_ints(src, n_src);
      for(std::size_t i = 0; i != n_src; ++i)
         BOOST_TEST_EQ(src_after[i], ref[i]);

      BOOST_TEST(src_spec != 0 && dst_spec != 0);
   }
};

void test_reverse_copy_shape_matrix()
{
   static const std::size_t sizes[][2] =
      { {0, 0}, {1, 1}, {1, 3}, {2, 2}, {3, 3}, {3, 6}, {5, 5}, {6, 10} };

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i)
      test_detail::for_each_shape2_all<int, int>
         (shape_src_vals, sizes[i][0], shape_dst_vals, sizes[i][1],
          shape_filler, reverse_copy_shape_check());
}

int main()
{
   test_reverse_copy_segmented();
   test_reverse_copy_empty();
   test_reverse_copy_single_segment();
   test_reverse_copy_single_segment_interior();
   test_reverse_copy_single_segment_empty_mid();
   test_reverse_copy_single_segment_seg2_outer();
   test_reverse_copy_single_segment_seg2_both();
   test_reverse_copy_single_element();
   test_reverse_copy_non_segmented();
   test_reverse_copy_seg2();
   test_reverse_copy_single_segment_src_multi_dst();
   test_reverse_copy_single_segment_src_and_dst();
   test_reverse_copy_single_segment_dst_from_flat();

   test_reverse_copy_shape_matrix();
   return boost::report_errors();
}
