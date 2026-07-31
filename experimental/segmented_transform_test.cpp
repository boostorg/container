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

void test_transform_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator result =
      segmented_transform(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), out.begin(), times_two());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - out.begin()), 4u);
   BOOST_TEST_EQ(out[0], 4);
   BOOST_TEST_EQ(out[1], 6);
   BOOST_TEST_EQ(out[2], 8);
   BOOST_TEST_EQ(out[3], 10);
   BOOST_TEST_EQ(out[4], 0);
   BOOST_TEST_EQ(out[5], 0);
}

void test_transform_single_segment_empty_mid()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int>::iterator mid = test_detail::iter_at(sv, 3);
   boost::container::vector<int> out(3, 99);
   boost::container::vector<int>::iterator result = segmented_transform(mid, mid, out.begin(), negate_val());

   BOOST_TEST(result == out.begin());
   BOOST_TEST_EQ(out[0], 99);
   BOOST_TEST_EQ(out[1], 99);
   BOOST_TEST_EQ(out[2], 99);
}

void test_transform_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator result =
      segmented_transform(test_detail::iter_at(sv, 1),
                          test_detail::make_sentinel(test_detail::iter_at(sv, 5)), out.begin(), negate_val());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - out.begin()), 4u);
   BOOST_TEST_EQ(out[0], -2);
   BOOST_TEST_EQ(out[1], -3);
   BOOST_TEST_EQ(out[2], -4);
   BOOST_TEST_EQ(out[3], -5);
   BOOST_TEST_EQ(out[4], 0);
}

void test_transform_single_segment_seg2_outer()
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
   boost::container::vector<int>::iterator result =
      segmented_transform(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin(), times_two());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - out.begin()), 5u);
   for(int i = 0; i < 5; ++i)
      BOOST_TEST_EQ(out[static_cast<std::size_t>(i)], (i + 2) * 2);
   BOOST_TEST_EQ(out[5], 0);
}

void test_transform_single_segment_seg2_both()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a[] = {1, 2, 3, 4, 5, 6};
   inner.add_segment_range(a, a + 6);
   sv2.add_segment(inner);

   boost::container::vector<int> out(6, 0);
   boost::container::vector<int>::iterator result =
      segmented_transform(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 5), out.begin(), times_two());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - out.begin()), 4u);
   BOOST_TEST_EQ(out[0], 4);
   BOOST_TEST_EQ(out[1], 6);
   BOOST_TEST_EQ(out[2], 8);
   BOOST_TEST_EQ(out[3], 10);
   BOOST_TEST_EQ(out[4], 0);
}

void test_transform_single_segment_src_multi_dst()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6, 7};
   sv.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> out;
   out.add_segment(2, 0);
   out.add_segment(3, 0);
   out.add_segment(3, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_transform(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 6), out.begin(), times_two());

   BOOST_TEST(result == test_detail::iter_at(out, 5));
   iter_t it = out.begin();
   for(int i = 0; i < 5; ++i, ++it)
      BOOST_TEST_EQ(*it, (i + 2) * 2);
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, 0);
}

void test_transform_single_segment_src_and_dst()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int> out;
   out.add_segment(6, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_transform(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), out.begin(), negate_val());

   BOOST_TEST(result == test_detail::iter_at(out, 4));
   iter_t it = out.begin();
   for(int i = 0; i < 4; ++i, ++it)
      BOOST_TEST_EQ(*it, -(i + 2));
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

void test_transform_single_segment_dst_from_flat()
{
   boost::container::vector<int> in;
   in.push_back(1); in.push_back(2); in.push_back(3); in.push_back(4);

   test_detail::seg_vector<int> out;
   out.add_segment(6, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_transform(in.begin(), in.end(), out.begin(), times_two());

   BOOST_TEST(result == test_detail::iter_at(out, 4));
   iter_t it = out.begin();
   for(int i = 0; i < 4; ++i, ++it)
      BOOST_TEST_EQ(*it, (i + 1) * 2);
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
// The reference answer is the unary operation applied to flatten_n_ints over
// the logical source range; flatten_all_ints is not used, as it deliberately
// includes the guard.  Size pairs with n_dst == n_src place the destination
// guard immediately after the last slot the transform may write.
//////////////////////////////////////////////////////////////////////////////

const int shape_filler = -999;   // guard just past the end of every range
const int shape_fill   = -1;     // every destination slot before the call

const int shape_src_vals[] = {5, 3, 9, 1, 7, 2, 8, 4, 6, 11};
const int shape_dst_vals[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

template<class Op>
struct transform_shape_check
{
   Op op;

   explicit transform_shape_check(Op o) : op(o) {}

   template<class CSrc, class CDst>
   void operator()(CSrc& src, std::size_t n_src, const char* src_spec,
                   CDst& dst, std::size_t n_dst, const char* dst_spec) const
   {
      typedef typename CDst::iterator dst_iter_t;

      const boost::container::vector<int> ref = test_detail::flatten_n_ints(src, n_src);

      const dst_iter_t r = segmented_transform
         (src.begin(), test_detail::iter_at(src, n_src), dst.begin(), op);

      BOOST_TEST(r == test_detail::iter_at(dst, n_src));
      BOOST_TEST(test_detail::filler_intact(src, n_src, shape_filler));
      BOOST_TEST(test_detail::filler_intact(dst, n_dst, shape_filler));

      const boost::container::vector<int> got = test_detail::flatten_n_ints(dst, n_dst);
      BOOST_TEST_EQ(got.size(), n_dst);
      for(std::size_t i = 0; i != n_dst; ++i)
         BOOST_TEST_EQ(got[i], i < n_src ? op(ref[i]) : shape_fill);

      // The source is read-only, so it has to come back unchanged.
      const boost::container::vector<int> src_after = test_detail::flatten_n_ints(src, n_src);
      for(std::size_t i = 0; i != n_src; ++i)
         BOOST_TEST_EQ(src_after[i], ref[i]);

      BOOST_TEST(src_spec != 0 && dst_spec != 0);
   }
};

void test_transform_shape_matrix()
{
   static const std::size_t sizes[][2] =
      { {0, 0}, {1, 1}, {1, 3}, {2, 2}, {3, 3}, {3, 6}, {5, 5}, {6, 10} };

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
      test_detail::for_each_shape2_all<int, int>
         (shape_src_vals, sizes[i][0], shape_dst_vals, sizes[i][1],
          shape_filler, transform_shape_check<times_two>(times_two()));
      test_detail::for_each_shape2_all<int, int>
         (shape_src_vals, sizes[i][0], shape_dst_vals, sizes[i][1],
          shape_filler, transform_shape_check<negate_val>(negate_val()));
   }
}

//////////////////////////////////////////////////////////////////////////////
// Operation application count.
//
// [alg.transform] mandates "Exactly last1 - first1 applications of op",
// whatever the segmentation of either range, so an element handed to op again
// after the destination walker steps to the next segment is caught here.
//////////////////////////////////////////////////////////////////////////////

struct transform_count_check
{
   template<class CSrc, class CDst>
   void operator()(CSrc& src, std::size_t n_src, const char* src_spec,
                   CDst& dst, std::size_t n_dst, const char* dst_spec) const
   {
      test_detail::op_counter calls;
      segmented_transform(src.begin(), test_detail::iter_at(src, n_src), dst.begin(),
                          test_detail::counting_fun<times_two, int>(calls, times_two()));

      BOOST_TEST_EQ(calls.n, n_src);
      BOOST_TEST(test_detail::filler_intact(dst, n_dst, shape_filler));
      BOOST_TEST(src_spec != 0 && dst_spec != 0);
   }
};

void test_transform_operation_count()
{
   static const std::size_t sizes[][2] =
      { {0, 0}, {1, 1}, {1, 3}, {2, 2}, {3, 3}, {3, 6}, {5, 5}, {6, 10} };

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i)
      test_detail::for_each_shape2_all<int, int>
         (shape_src_vals, sizes[i][0], shape_dst_vals, sizes[i][1],
          shape_filler, transform_count_check());

   // Destination segments of a handful of elements each, so that boundary
   // crossings dominate rather than being a single mid-range event.
   static const std::size_t blocks[] = {1u, 2u, 3u, 8u, 16u};
   const std::size_t n = 64u;
   boost::container::vector<int> in;
   in.reserve(n);
   for(std::size_t i = 0; i != n; ++i)
      in.push_back(static_cast<int>(i) + 1);

   for(std::size_t b = 0; b != sizeof(blocks)/sizeof(blocks[0]); ++b) {
      test_detail::seg_vector<int> out;
      for(std::size_t room = 0; room < n; room += blocks[b])
         out.add_segment(blocks[b], 0);

      test_detail::op_counter calls;
      segmented_transform(in.begin(), in.end(), out.begin(),
                          test_detail::counting_fun<times_two, int>(calls, times_two()));
      BOOST_TEST_EQ(calls.n, n);
   }
}

int main()
{
   test_transform_full_range();
   test_transform_empty();
   test_transform_single_segment();
   test_transform_single_segment_interior();
   test_transform_single_segment_empty_mid();
   test_transform_single_segment_sentinel();
   test_transform_single_segment_seg2_outer();
   test_transform_single_segment_seg2_both();
   test_transform_non_segmented();
   test_transform_sentinel_segmented();
   test_transform_sentinel_non_segmented();
   test_transform_seg2();
   test_transform_segmented_output();
   test_transform_seg2_to_seg2();
   test_transform_seg_to_seg_misaligned();
   test_transform_single_segment_src_multi_dst();
   test_transform_single_segment_src_and_dst();
   test_transform_single_segment_dst_from_flat();

   test_transform_shape_matrix();
   test_transform_operation_count();
   return boost::report_errors();
}
