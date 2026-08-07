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
#include "../test/lightweight_test.hpp"
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

// Multi-segmented output with non-segmented inputs.  Exercises the
// merge_dst_dispatch(segmented_iterator_tag) walker that walks output
// segments and bounds each merge call to the current segment.
void test_merge_segmented_output()
{
   int a[] = {1, 3, 5, 7, 9};
   int b[] = {2, 4, 6, 8};

   test_detail::seg_vector<int> out;
   out.add_segment(3, 0);
   out.add_segment(4, 0);
   out.add_segment(2, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(a, a + 5, b, b + 4, out.begin());

   BOOST_TEST(r == out.end());
   iter_t it = out.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, i + 1);
}

// Segmented inputs and segmented output: stresses the per-src1-segment
// walker threading first2/result through the multi-segmented dst walker.
void test_merge_segmented_inputs_and_output()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {1, 3};
   int a2[] = {5, 7, 9};
   sv1.add_segment_range(a1, a1 + 2);
   sv1.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int> sv2;
   int b1[] = {2, 4};
   int b2[] = {6, 8};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 2);

   test_detail::seg_vector<int> out;
   out.add_segment(2, 0);
   out.add_segment(3, 0);
   out.add_segment(4, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(
      sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST(r == out.end());
   iter_t it = out.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, i + 1);
}

// First source larger than second: forces the first2-exhausted branch
// of merge_dst_dispatch(segmented_iterator_tag) which drains the
// remaining first1 via segmented_copy through the rest of dst.
void test_merge_segmented_output_first_longer()
{
   int a[] = {1, 3, 5, 7, 9, 11, 13};
   int b[] = {2, 4};

   test_detail::seg_vector<int> out;
   out.add_segment(2, 0);
   out.add_segment(2, 0);
   out.add_segment(2, 0);
   out.add_segment(3, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(a, a + 7, b, b + 2, out.begin());

   BOOST_TEST(r == out.end());
   int expected[] = {1, 2, 3, 4, 5, 7, 9, 11, 13};
   iter_t it = out.begin();
   for(std::size_t i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
}

// Second source larger than first: forces the first1-exhausted branch
// where the leaf returns and then the remaining first2 is flushed via
// segmented_copy at the top-level segmented_merge_dispatch.
void test_merge_segmented_output_second_longer()
{
   int a[] = {1, 3};
   int b[] = {2, 4, 6, 8, 10, 12, 14};

   test_detail::seg_vector<int> out;
   out.add_segment(3, 0);
   out.add_segment(3, 0);
   out.add_segment(3, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(a, a + 2, b, b + 7, out.begin());

   BOOST_TEST(r == out.end());
   int expected[] = {1, 2, 3, 4, 6, 8, 10, 12, 14};
   iter_t it = out.begin();
   for(std::size_t i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
}

// Stress test for the unrolled main loop.  With N=64 in both ranges, the
// unrolled fast path of merge_dst_bounded executes at least 16 times and
// the count-based tail handles the remainder.  Exercises the various
// dst-flavour code paths (unbounded, bounded RA, segmented).
void test_merge_stress_long_ranges()
{
   const int N = 64;
   boost::container::vector<int> v1, v2;
   v1.reserve(N); v2.reserve(N);
   for(int i = 0; i < N; ++i) v1.push_back(2 * i);          // 0, 2, 4, ...
   for(int i = 0; i < N; ++i) v2.push_back(2 * i + 1);      // 1, 3, 5, ...

   // Flat dst (RA, unbounded path through dual-RA when sized exactly).
   boost::container::vector<int> out_flat(2 * N, -1);
   boost::container::vector<int>::iterator r1 = segmented_merge(
      v1.begin(), v1.end(), v2.begin(), v2.end(), out_flat.begin());
   BOOST_TEST_EQ(static_cast<std::size_t>(r1 - out_flat.begin()),
                 static_cast<std::size_t>(2 * N));
   for(int i = 0; i < 2 * N; ++i)
      BOOST_TEST_EQ(out_flat[static_cast<std::size_t>(i)], i);

   // Segmented dst with non-uniform segments: exercises unrolled body
   // inside the segmented walker (each per-segment call hits the
   // unrolled overload with bounded RA dst).
   test_detail::seg_vector<int> out_seg;
   out_seg.add_segment(13, 0);
   out_seg.add_segment(31, 0);
   out_seg.add_segment(7, 0);
   out_seg.add_segment(2 * N - 13 - 31 - 7, 0);

   typedef test_detail::seg_vector<int>::iterator seg_iter_t;
   seg_iter_t r2 = segmented_merge(
      v1.begin(), v1.end(), v2.begin(), v2.end(), out_seg.begin());
   BOOST_TEST(r2 == out_seg.end());
   seg_iter_t it = out_seg.begin();
   for(int i = 0; i < 2 * N; ++i, ++it)
      BOOST_TEST_EQ(*it, i);
}

// Non-segmented first1 + segmented first2 + non-segmented (RA) dst.
// Exercises the new merge_seg2_dispatch walker: each first2 segment is
// fed flat to the bounded leaf, so the dual-RA / unrolled fast paths fire
// per segment rather than degrading to the generic input-iterator merge.
void test_merge_seg2_walker_flat_dst()
{
   int a[] = {2, 4, 6, 8, 10};
   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 3};
   int b2[] = {5, 7};
   int b3[] = {9, 11, 13};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 2);
   sv2.add_segment_range(b3, b3 + 3);

   boost::container::vector<int> out(12, 0);
   boost::container::vector<int>::iterator r = segmented_merge(
      a, a + 5, sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 12u);
   int expected[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// Non-segmented first1 + segmented first2 + segmented dst.
// Exercises the cross product of the seg2 walker with the seg-dst walker
// inside merge_until_exhausts (segmented dst overload).
void test_merge_seg2_walker_seg_dst()
{
   int a[] = {2, 4, 6, 8, 10};
   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 3};
   int b2[] = {5, 7};
   int b3[] = {9, 11, 13};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 2);
   sv2.add_segment_range(b3, b3 + 3);

   test_detail::seg_vector<int> out;
   out.add_segment(3, 0);
   out.add_segment(2, 0);
   out.add_segment(5, 0);
   out.add_segment(2, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(a, a + 5, sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST(r == out.end());
   int expected[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13};
   iter_t it = out.begin();
   for(std::size_t i = 0; i < 12; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
}

// first1 longer than first2: forces seg2 walker to fully consume all
// first2 segments and then drain the remaining first1 via segmented_copy.
void test_merge_seg2_walker_first1_longer()
{
   int a[] = {1, 3, 5, 7, 9, 11, 13, 15, 17};
   test_detail::seg_vector<int> sv2;
   int b1[] = {2};
   int b2[] = {4, 6};
   sv2.add_segment_range(b1, b1 + 1);
   sv2.add_segment_range(b2, b2 + 2);

   boost::container::vector<int> out(12, 0);
   boost::container::vector<int>::iterator r = segmented_merge(
      a, a + 9, sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 12u);
   int expected[] = {1, 2, 3, 4, 5, 6, 7, 9, 11, 13, 15, 17};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// first1 shorter than first2: first1 exhausts while we are mid-first2-segment.
// Validates that seg2 walker composes the partial-segment first2 position on
// return so the top-level segmented_copy can flush the leftover first2 tail.
void test_merge_seg2_walker_first2_longer()
{
   int a[] = {3, 5};
   test_detail::seg_vector<int> sv2;
   int b1[] = {1, 2};
   int b2[] = {4, 6, 8};
   int b3[] = {10, 12};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 3);
   sv2.add_segment_range(b3, b3 + 2);

   boost::container::vector<int> out(9, 0);
   boost::container::vector<int>::iterator r = segmented_merge(
      a, a + 2, sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 9u);
   int expected[] = {1, 2, 3, 4, 5, 6, 8, 10, 12};
   for(std::size_t i = 0; i < 9; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// Stress: long flat first1 + multi-segmented first2 with non-uniform segments
// + segmented dst.  Each first2 segment gets fed flat into the bounded leaf,
// so the unrolled / dual-RA fast paths fire on every segment with size >= 4.
// Total elements 64 + 64 = 128, exercising the unrolled body multiple times.
void test_merge_seg2_walker_stress()
{
   const int N1 = 64;
   const int N2 = 64;
   boost::container::vector<int> v1;
   v1.reserve(N1);
   for(int i = 0; i < N1; ++i) v1.push_back(2 * i);              // 0, 2, 4, ...

   test_detail::seg_vector<int> sv2;
   int seg_sizes[] = {17, 9, 23, 4, 11};   // sums to 64
   int next = 1;
   for(std::size_t s = 0; s < sizeof(seg_sizes)/sizeof(seg_sizes[0]); ++s) {
      boost::container::vector<int> seg;
      seg.reserve(static_cast<std::size_t>(seg_sizes[s]));
      for(int j = 0; j < seg_sizes[s]; ++j) {
         seg.push_back(next);
         next += 2;
      }
      sv2.add_segment_range(seg.begin(), seg.end());
   }

   const int total = N1 + N2;
   test_detail::seg_vector<int> out;
   out.add_segment(11, 0);
   out.add_segment(50, 0);
   out.add_segment(40, 0);
   out.add_segment(total - 11 - 50 - 40, 0);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(
      v1.begin(), v1.end(), sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST(r == out.end());
   iter_t it = out.begin();
   for(int i = 0; i < total; ++i, ++it)
      BOOST_TEST_EQ(*it, i);
}

// Asymmetric stress: src1 and src2 of very different sizes plus pathological
// data distribution where one source dominates a long contiguous run.
// Forces the count-based tail of the unrolled overload to handle the
// remaining > 4 elements once one side dropped below 4 in the main loop.
void test_merge_stress_asymmetric()
{
   boost::container::vector<int> v1, v2;
   for(int i = 0; i < 5; ++i) v1.push_back(1000 + i);            // [1000..1004]
   for(int i = 0; i < 50; ++i) v2.push_back(i);                  // [0..49]

   boost::container::vector<int> out(55, -1);
   boost::container::vector<int>::iterator r = segmented_merge(
      v1.begin(), v1.end(), v2.begin(), v2.end(), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 55u);
   for(int i = 0; i < 50; ++i)
      BOOST_TEST_EQ(out[static_cast<std::size_t>(i)], i);
   for(int i = 0; i < 5; ++i)
      BOOST_TEST_EQ(out[static_cast<std::size_t>(50 + i)], 1000 + i);
}

//////////////////////////////////////////////////////////////////////////////
// Single-segment coverage.
//
// A range spanning a whole seg_vector never takes the single-segment branch
// of a segment walker, because its end iterator lives in the trailing
// sentinel segment.  The tests below build one oversized segment and merge a
// proper sub-range of it, so that segment(first) == segment(last).
//
// Every test pins the complete output sequence, including the slots left
// untouched past the returned iterator, plus the returned position itself.
//
// Shared data: {1, 2, 3, 5, 8} merged with {2, 3, 4, 8, 9} gives
// {1, 2, 2, 3, 3, 4, 5, 8, 8, 9}; the duplicate keys shared by both inputs
// make the take-from-the-first-range-on-ties rule observable.
//////////////////////////////////////////////////////////////////////////////

// M4: single-segment first input, flat second input.
void test_merge_single_segment_input1()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   int b[] = {2, 3, 4, 8, 9};
   boost::container::vector<int> out(14, -1);

   boost::container::vector<int>::iterator r = segmented_merge(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      b, b + 5, out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 10u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 14; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// M4: flat first input, single-segment second input.
void test_merge_single_segment_input2()
{
   int a[] = {1, 2, 3, 5, 8};

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   boost::container::vector<int> out(14, -1);

   boost::container::vector<int>::iterator r = segmented_merge(
      a, a + 5,
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 10u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 14; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// M4: both inputs single-segment, both bounded on both sides.
void test_merge_single_segment_both_inputs()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   boost::container::vector<int> out(14, -1);

   boost::container::vector<int>::iterator r = segmented_merge(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 10u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 14; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// M1: multi-segment inputs, single-segment segmented output.
void test_merge_single_segment_output()
{
   test_detail::seg_vector<int> sv1;
   int a1[] = {1, 2};
   int a2[] = {3};
   int a3[] = {5, 8};
   sv1.add_segment_range(a1, a1 + 2);
   sv1.add_segment_range(a2, a2 + 1);
   sv1.add_segment_range(a3, a3 + 2);

   test_detail::seg_vector<int> sv2;
   int b1[] = {2, 3};
   int b2[] = {4, 8, 9};
   sv2.add_segment_range(b1, b1 + 2);
   sv2.add_segment_range(b2, b2 + 3);

   test_detail::seg_vector<int> out;
   out.add_segment(14, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 10));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 14u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// M3: single-segment inputs and single-segment output.
void test_merge_single_segment_inputs_and_output()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   test_detail::seg_vector<int> out;
   out.add_segment(14, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 10));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 14u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// M2: single-segment inputs, multi-segment output.  The output segments are
// small enough that the merge crosses two of them and the tail copy of the
// residue crosses a third.
void test_merge_single_segment_inputs_multi_output()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   test_detail::seg_vector<int> out;
   out.add_segment(3, -1);
   out.add_segment(4, -1);
   out.add_segment(5, -1);
   out.add_segment(2, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 10));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 14u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// The first input is exhausted long before the second, so the whole tail of
// the second input has to be copied through into a single-segment output.
void test_merge_single_segment_first_input_exhausted()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 100};
   sv1.add_segment_range(a, a + 4);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 3, 4, 5, 6, 7, 100};
   sv2.add_segment_range(b, b + 7);

   test_detail::seg_vector<int> out;
   out.add_segment(10, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 3),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 7));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 10u);
   int expected[] = {1, 2, 3, 4, 5, 6, 7, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// Mirror image: the second input is exhausted first and the tail of the
// first input is copied through.
void test_merge_single_segment_second_input_exhausted()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 3, 4, 5, 6, 7, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 1, 2, 100};
   sv2.add_segment_range(b, b + 4);

   test_detail::seg_vector<int> out;
   out.add_segment(10, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 3), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 7));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 10u);
   int expected[] = {1, 2, 3, 4, 5, 6, 7, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// S5 and S6 on both inputs: one outer segment holding several inner ones,
// and one outer segment holding exactly one inner segment.
void test_merge_single_segment_seg2_inputs()
{
   int a[] = {1, 2, 3, 5, 8};
   int b[] = {2, 3, 4, 8, 9};

   test_detail::seg2_vector<int> sv1;
   test_detail::make_range(sv1, "sm", a, 5, 100);
   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "ss", b, 5, 100);

   boost::container::vector<int> out(14, -1);
   boost::container::vector<int>::iterator r = segmented_merge(
      sv1.begin(), test_detail::iter_at(sv1, 5),
      sv2.begin(), test_detail::iter_at(sv2, 5), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 10u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 14; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// A recursively segmented output whose outer level holds a single segment
// with room to spare.
void test_merge_single_segment_seg2_output()
{
   int a[] = {1, 2, 3, 5, 8};
   int b[] = {2, 3, 4, 8, 9};

   test_detail::seg_vector<int> inner;
   inner.add_segment(6, -1);
   inner.add_segment(8, -1);
   test_detail::seg2_vector<int> out;
   out.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(a, a + 5, b, b + 5, out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 10));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 14u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// Same, but the first outer segment of the output is filled completely, so
// the merge has to carry on into the second outer segment.
void test_merge_single_segment_seg2_output_segment_full()
{
   int a[] = {1, 2, 3, 5, 8};
   int b[] = {2, 3, 4, 8, 9};

   test_detail::seg_vector<int> inner0;
   inner0.add_segment(2, -1);
   inner0.add_segment(2, -1);
   test_detail::seg_vector<int> inner1;
   inner1.add_segment(10, -1);
   test_detail::seg2_vector<int> out;
   out.add_segment(inner0);
   out.add_segment(inner1);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(a, a + 5, b, b + 5, out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 10));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 14u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// S3: an empty first input positioned mid-segment; the second input is
// copied through unchanged.
void test_merge_single_segment_empty_input()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 6);

   int b[] = {2, 3, 4};
   boost::container::vector<int> out(6, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t mid = test_detail::iter_at(sv1, 3);
   boost::container::vector<int>::iterator r =
      segmented_merge(mid, mid, b, b + 3, out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   int expected[] = {2, 3, 4, -1, -1, -1};
   for(std::size_t i = 0; i < 6; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// S4: single-segment sub-ranges closed by sentinels.
void test_merge_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   boost::container::vector<int> out(14, -1);
   boost::container::vector<int>::iterator r = segmented_merge(
      test_detail::iter_at(sv1, 1), test_detail::make_sentinel(test_detail::iter_at(sv1, 6)),
      test_detail::iter_at(sv2, 1), test_detail::make_sentinel(test_detail::iter_at(sv2, 6)),
      out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 10u);
   int expected[] = {1, 2, 2, 3, 3, 4, 5, 8, 8, 9, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 14; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// The comparator-taking overload on single-segment inputs and output.
void test_merge_single_segment_with_comp()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {100, 8, 5, 3, 2, 1, -100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {100, 9, 8, 4, 3, 2, -100};
   sv2.add_segment_range(b, b + 7);

   test_detail::seg_vector<int> out;
   out.add_segment(14, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_merge(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6),
      out.begin(), greater_comp());

   BOOST_TEST(r == test_detail::iter_at(out, 10));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 14u);
   int expected[] = {9, 8, 8, 5, 4, 3, 3, 2, 2, 1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// segmented_merge has three independently segmented ranges -- two inputs and
// one destination -- and the destination walker is where both bugs found in
// this algorithm so far have lived.  for_each_shape3_all drives the full
// cross product including the 'e' shapes, whose empty segments are the only
// way to reach the sfirst == slast branch of merge_dst_bounded with an empty
// destination segment.
//
// Destination sizes.  Two geometries are run for every combination:
//
//   * exactly n1 + n2 slots, so the last element of the merge lands on the
//     last slot of the destination and every intermediate destination
//     segment fills exactly.  That is the geometry that hid the last bug:
//     merge_dst_bounded handing the original rather than the loop-carried
//     input iterators to its final segment call was only observable when an
//     intermediate segment filled without either input running out.
//   * three slots more, so the destination keeps an unwritten tail that both
//     the fill comparison and the guard check have to see intact.
//
// The inputs carry duplicates, within a range and shared between them, so
// that merge's tie-breaking -- take from range 1 when neither compares less
// -- is pinned down rather than left unobserved.
//////////////////////////////////////////////////////////////////////////////

struct merge_shape_check
{
   void report(const char* s1, std::size_t n1, const char* s2, std::size_t n2,
               const char* s3, std::size_t n3) const
   {
      BOOST_LIGHTWEIGHT_TEST_OSTREAM
         << "   shapes \"" << s1 << "\"(" << n1 << ") / \"" << s2 << "\"(" << n2
         << ") -> \"" << s3 << "\"(" << n3 << ")" << std::endl;
   }

   template<class C1, class C2, class C3>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2,
                   C3& c3, std::size_t n3, const char* s3) const
   {
      typedef typename C3::iterator iter3_t;

      const boost::container::vector<int> f1 = test_detail::flatten_n_ints(c1, n1);
      const boost::container::vector<int> f2 = test_detail::flatten_n_ints(c2, n2);

      // Naive reference: the classic two-finger merge, taking from range 1
      // whenever the two compare equal.
      boost::container::vector<int> ref;
      {
         std::size_t i = 0, j = 0;
         while(i != f1.size() && j != f2.size())
            ref.push_back(f2[j] < f1[i] ? f2[j++] : f1[i++]);
         for(; i != f1.size(); ++i) ref.push_back(f1[i]);
         for(; j != f2.size(); ++j) ref.push_back(f2[j]);
      }

      const iter3_t r = segmented_merge
         ( c1.begin(), test_detail::iter_at(c1, n1)
         , c2.begin(), test_detail::iter_at(c2, n2)
         , c3.begin());

      if(!BOOST_TEST(r == test_detail::iter_at(c3, ref.size())))
         this->report(s1, n1, s2, n2, s3, n3);

      // flatten_n_ints, not flatten_all_ints: the reference answer covers the
      // logical range only.  The guard past it is checked separately, below.
      const boost::container::vector<int> got = test_detail::flatten_n_ints(c3, n3);
      if(!BOOST_TEST_EQ(got.size(), n3)) {
         this->report(s1, n1, s2, n2, s3, n3);
         return;
      }
      for(std::size_t i = 0; i != n3; ++i) {
         const int want = i < ref.size() ? ref[i] : -1;
         if(!BOOST_TEST_EQ(got[i], want)) {
            this->report(s1, n1, s2, n2, s3, n3);
            break;
         }
      }

      // Nothing may be written past the destination's logical end, and
      // neither input may be written at all.
      if(!BOOST_TEST(test_detail::filler_intact(c3, n3, -999)))
         this->report(s1, n1, s2, n2, s3, n3);
      if(!BOOST_TEST(test_detail::filler_intact(c1, n1, -999)))
         this->report(s1, n1, s2, n2, s3, n3);
      if(!BOOST_TEST(test_detail::filler_intact(c2, n2, -999)))
         this->report(s1, n1, s2, n2, s3, n3);
   }
};

void test_merge_shape_matrix()
{
   // Sorted, with duplicates inside each range and shared between them.
   const int v1[] = {1, 2, 2, 3, 5, 8};
   const int v2[] = {2, 3, 3, 4, 8, 9};

   static const std::size_t pairs[][2] =
      { {0u, 0u}, {0u, 3u}, {3u, 0u}, {1u, 1u}, {2u, 4u}, {4u, 2u}, {5u, 6u} };

   // 0 fills the destination exactly; 3 leaves an unwritten tail.
   static const std::size_t extras[] = { 0u, 3u };

   for(std::size_t p = 0; p != sizeof(pairs)/sizeof(pairs[0]); ++p) {
      for(std::size_t e = 0; e != sizeof(extras)/sizeof(extras[0]); ++e) {
         const std::size_t n1 = pairs[p][0];
         const std::size_t n2 = pairs[p][1];
         const std::size_t n3 = n1 + n2 + extras[e];

         boost::container::vector<int> fill(n3 ? n3 : 1u, -1);
         test_detail::for_each_shape3_all<int, int, int>
            (v1, n1, v2, n2, &fill[0], n3, -999, merge_shape_check());
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Comparison count.
//
// [alg.merge] mandates "At most N - 1 comparisons", N being the total input
// length.  That is far tighter than the set operations allow, so a single
// element re-compared at a destination boundary already breaks it.
//////////////////////////////////////////////////////////////////////////////

struct less_comp
{
   bool operator()(int a, int b) const { return a < b; }
};

inline std::size_t merge_comparison_bound(std::size_t n1, std::size_t n2)
{
   const std::size_t total = n1 + n2;
   return total ? total - 1u : 0u;
}

struct merge_count_check
{
   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      boost::container::vector<int> flat(n1 + n2 + 1u, -1);
      {
         test_detail::op_counter calls;
         segmented_merge(c1.begin(), test_detail::iter_at(c1, n1),
                         c2.begin(), test_detail::iter_at(c2, n2),
                         flat.begin(), test_detail::counting_pred(calls, less_comp()));
         BOOST_TEST(calls.n <= merge_comparison_bound(n1, n2));
      }

      for(std::size_t block = 1u; block <= 3u; ++block) {
         test_detail::seg_vector<int> out;
         for(std::size_t room = 0; room <= n1 + n2; room += block)
            out.add_segment(block, -1);

         test_detail::op_counter calls;
         segmented_merge(c1.begin(), test_detail::iter_at(c1, n1),
                         c2.begin(), test_detail::iter_at(c2, n2),
                         out.begin(), test_detail::counting_pred(calls, less_comp()));
         BOOST_TEST(calls.n <= merge_comparison_bound(n1, n2));
      }

      BOOST_TEST(s1 != 0 && s2 != 0);
   }
};

void test_merge_comparison_count()
{
   const int v1[] = {1, 2, 2, 3, 5, 8};
   const int v2[] = {2, 3, 3, 4, 8, 9};

   static const std::size_t pairs[][2] =
      { {0u, 0u}, {0u, 3u}, {3u, 0u}, {1u, 1u}, {2u, 4u}, {4u, 2u}, {5u, 6u} };

   for(std::size_t p = 0; p != sizeof(pairs)/sizeof(pairs[0]); ++p)
      test_detail::for_each_shape2_all<int, int>
         (v1, pairs[p][0], v2, pairs[p][1], -999, merge_count_check());
}

int main()
{
   test_merge_shape_matrix();
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
   test_merge_segmented_output();
   test_merge_segmented_inputs_and_output();
   test_merge_segmented_output_first_longer();
   test_merge_segmented_output_second_longer();
   test_merge_seg2_walker_flat_dst();
   test_merge_seg2_walker_seg_dst();
   test_merge_seg2_walker_first1_longer();
   test_merge_seg2_walker_first2_longer();
   test_merge_seg2_walker_stress();
   test_merge_stress_long_ranges();
   test_merge_stress_asymmetric();

   // Single-segment coverage:
   test_merge_single_segment_input1();
   test_merge_single_segment_input2();
   test_merge_single_segment_both_inputs();
   test_merge_single_segment_output();
   test_merge_single_segment_inputs_and_output();
   test_merge_single_segment_inputs_multi_output();
   test_merge_single_segment_first_input_exhausted();
   test_merge_single_segment_second_input_exhausted();
   test_merge_single_segment_seg2_inputs();
   test_merge_single_segment_seg2_output();
   test_merge_single_segment_seg2_output_segment_full();
   test_merge_single_segment_empty_input();
   test_merge_single_segment_sentinel();
   test_merge_single_segment_with_comp();

   test_merge_comparison_count();
   return boost::report_errors();
}
