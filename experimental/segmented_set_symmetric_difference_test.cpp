//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_set_symmetric_difference.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_set_symmetric_difference_basic()
{
   int a[] = {1, 3, 5, 7};
   int b[] = {2, 3, 6, 7, 8};
   boost::container::vector<int> out(9, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_symmetric_difference(a, a + 4, b, b + 5, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 5u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 5);
   BOOST_TEST_EQ(out[3], 6);
   BOOST_TEST_EQ(out[4], 8);
}

void test_set_symmetric_difference_empty()
{
   int a[] = {1, 2, 3};
   int* empty = a;
   boost::container::vector<int> out(3, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_symmetric_difference(a, a + 3, empty, empty, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 3u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 3);
}

void test_set_symmetric_difference_identical()
{
   int a[] = {1, 2, 3};
   int b[] = {1, 2, 3};
   boost::container::vector<int> out(3, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_symmetric_difference(a, a + 3, b, b + 3, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 0u);
}

void test_set_symmetric_difference_disjoint()
{
   int a[] = {1, 3, 5};
   int b[] = {2, 4, 6};
   boost::container::vector<int> out(6, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_symmetric_difference(a, a + 3, b, b + 3, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 6u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 3);
   BOOST_TEST_EQ(out[3], 4);
   BOOST_TEST_EQ(out[4], 5);
   BOOST_TEST_EQ(out[5], 6);
}

struct greater_int
{
   bool operator()(int a, int b) const { return a > b; }
};

void test_set_symmetric_difference_with_comp()
{
   int a[] = {7, 5, 3, 1};
   int b[] = {8, 7, 6, 3, 2};
   boost::container::vector<int> out(9, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_symmetric_difference(a, a + 4, b, b + 5, out.begin(), greater_int());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 5u);
   BOOST_TEST_EQ(out[0], 8);
   BOOST_TEST_EQ(out[1], 6);
   BOOST_TEST_EQ(out[2], 5);
   BOOST_TEST_EQ(out[3], 2);
   BOOST_TEST_EQ(out[4], 1);
}

void test_set_symmetric_difference_segmented_input()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 3};
   int a2[] = {5, 7};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   int b[] = {2, 3, 5, 9};
   boost::container::vector<int> out(8, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_symmetric_difference(sv.begin(), sv.end(), b, b + 4, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 4u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 7);
   BOOST_TEST_EQ(out[3], 9);
}

void test_set_symmetric_difference_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 3};
   int a2[] = {5, 7};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   int b[] = {2, 3, 5, 9};
   boost::container::vector<int> out(8, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_symmetric_difference(sv.begin(), test_detail::make_sentinel(sv.end()),
                                         b, test_detail::make_sentinel(b + 4), out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 4u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 7);
   BOOST_TEST_EQ(out[3], 9);
}

void test_set_symmetric_difference_sentinel_non_segmented()
{
   int a[] = {1, 3, 5, 7};
   int b[] = {2, 3, 6, 7, 8};
   boost::container::vector<int> out(9, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_symmetric_difference(a, test_detail::make_sentinel(a + 4),
                                         b, test_detail::make_sentinel(b + 5), out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 5u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 5);
   BOOST_TEST_EQ(out[3], 6);
   BOOST_TEST_EQ(out[4], 8);
}

void test_set_symmetric_difference_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 3, 5};
   int a2[] = {7, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);

   int b[] = {2, 3, 7, 10};
   boost::container::vector<int> out(9, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_symmetric_difference(sv2.begin(), sv2.end(), b, b + 4, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 5u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 5);
   BOOST_TEST_EQ(out[3], 9);
   BOOST_TEST_EQ(out[4], 10);
}

//////////////////////////////////////////////////////////////////////////////
// Single-segment coverage.
//
// A range spanning a whole seg_vector never takes the single-segment branch
// of a segment walker, because its end iterator lives in the trailing
// sentinel segment.  The tests below build one oversized segment and use a
// proper sub-range of it, so that segment(first) == segment(last).
//
// Every test pins the complete output sequence, including the slots left
// untouched past the returned iterator, plus the returned position itself.
//
// Shared data: {1, 2, 3, 5, 8} and {2, 3, 4, 8, 9} share the keys 2, 3 and 8,
// so their symmetric difference is {1, 4, 5, 9}.
//////////////////////////////////////////////////////////////////////////////

// M4: single-segment first input, flat second input.
void test_set_symmetric_difference_single_segment_input1()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   int b[] = {2, 3, 4, 8, 9};
   boost::container::vector<int> out(12, -1);

   boost::container::vector<int>::iterator r = segmented_set_symmetric_difference(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      b, b + 5, out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 4u);
   int expected[] = {1, 4, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// M4: flat first input, single-segment second input.
void test_set_symmetric_difference_single_segment_input2()
{
   int a[] = {1, 2, 3, 5, 8};

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   boost::container::vector<int> out(12, -1);

   boost::container::vector<int>::iterator r = segmented_set_symmetric_difference(
      a, a + 5,
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 4u);
   int expected[] = {1, 4, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// M4: both inputs single-segment.
void test_set_symmetric_difference_single_segment_both_inputs()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   boost::container::vector<int> out(12, -1);

   boost::container::vector<int>::iterator r = segmented_set_symmetric_difference(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 4u);
   int expected[] = {1, 4, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// M1: multi-segment inputs, single-segment segmented output.
void test_set_symmetric_difference_single_segment_output()
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
   out.add_segment(12, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_set_symmetric_difference(
      sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 4));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 12u);
   int expected[] = {1, 4, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// M3: single-segment inputs and single-segment output.
void test_set_symmetric_difference_single_segment_inputs_and_output()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   test_detail::seg_vector<int> out;
   out.add_segment(12, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_set_symmetric_difference(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 4));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 12u);
   int expected[] = {1, 4, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// M2: single-segment inputs, multi-segment output.  The first output segment
// is filled exactly by the scan and the residue drain lands in the next one.
void test_set_symmetric_difference_single_segment_inputs_multi_output()
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

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_set_symmetric_difference(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 4));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 12u);
   int expected[] = {1, 4, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// The first input is exhausted well before the second, so the tail of the
// second one is drained into a single-segment output.
void test_set_symmetric_difference_single_segment_first_input_exhausted()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 100};
   sv1.add_segment_range(a, a + 4);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 5, 6, 100};
   sv2.add_segment_range(b, b + 7);

   test_detail::seg_vector<int> out;
   out.add_segment(10, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_set_symmetric_difference(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 3),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 5));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 10u);
   int expected[] = {1, 3, 4, 5, 6, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// Mirror image: the second input is exhausted first.
void test_set_symmetric_difference_single_segment_second_input_exhausted()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 2, 3, 4, 5, 6, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 1, 2, 100};
   sv2.add_segment_range(b, b + 4);

   test_detail::seg_vector<int> out;
   out.add_segment(10, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_set_symmetric_difference(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 3), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 5));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 10u);
   int expected[] = {1, 3, 4, 5, 6, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// S5 and S6: one outer segment holding several inner segments, and one outer
// segment holding exactly one inner segment.
void test_set_symmetric_difference_single_segment_seg2_inputs()
{
   int a[] = {1, 2, 3, 5, 8};
   int b[] = {2, 3, 4, 8, 9};

   test_detail::seg2_vector<int> sv1;
   test_detail::make_range(sv1, "sm", a, 5, 100);
   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "ss", b, 5, 100);

   boost::container::vector<int> out(12, -1);
   boost::container::vector<int>::iterator r = segmented_set_symmetric_difference(
      sv1.begin(), test_detail::iter_at(sv1, 5),
      sv2.begin(), test_detail::iter_at(sv2, 5), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 4u);
   int expected[] = {1, 4, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// S3: an empty first input positioned mid-segment.
void test_set_symmetric_difference_single_segment_empty_input()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 6);

   int b[] = {2, 3, 4};
   boost::container::vector<int> out(6, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t mid = test_detail::iter_at(sv1, 3);
   boost::container::vector<int>::iterator r =
      segmented_set_symmetric_difference(mid, mid, b, b + 3, out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   int expected[] = {2, 3, 4, -1, -1, -1};
   for(std::size_t i = 0; i < 6; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// S4: single-segment sub-ranges closed by sentinels.
void test_set_symmetric_difference_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   boost::container::vector<int> out(12, -1);
   boost::container::vector<int>::iterator r = segmented_set_symmetric_difference(
      test_detail::iter_at(sv1, 1), test_detail::make_sentinel(test_detail::iter_at(sv1, 6)),
      test_detail::iter_at(sv2, 1), test_detail::make_sentinel(test_detail::iter_at(sv2, 6)),
      out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 4u);
   int expected[] = {1, 4, 5, 9, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// The comparator-taking overload on single-segment inputs and output.
void test_set_symmetric_difference_single_segment_with_comp()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {100, 8, 5, 3, 2, 1, -100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {100, 9, 8, 4, 3, 2, -100};
   sv2.add_segment_range(b, b + 7);

   test_detail::seg_vector<int> out;
   out.add_segment(12, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t r = segmented_set_symmetric_difference(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6),
      out.begin(), greater_int());

   BOOST_TEST(r == test_detail::iter_at(out, 4));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 12u);
   int expected[] = {9, 5, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// Two inputs and one destination.  The two inputs are driven by
// for_each_shape2_all, so their shapes -- including the 'e' ones, whose
// empty segments the two source walkers must skip independently of each
// other -- are enumerated in full.
//
// The destination is enumerated at depth 1 only.
// segmented_set_symmetric_difference has no segmented_iterator_tag overload
// of set_symmetric_difference_dst_bounded, so a two-level segmented
// destination does not compile: the until_exhausts layer would have to hand
// the leaf kernel a local iterator that is itself segmented.  That gap in
// the header is known and queued for a separate fix; once it lands this can
// become a plain for_each_shape3_all.
//
// Two destination sizes are run per combination.  The output length is
// data-dependent, so the guard at index n3 only catches an overrun of the
// *allocated* destination; sizing the destination to exactly the expected
// output length is what turns it into a check on the *written* prefix.  The
// second size leaves three unwritten slots, which must still hold the fill.
//////////////////////////////////////////////////////////////////////////////

struct set_symmetric_difference_shape_check
{
   std::size_t extra;   // destination slots beyond the expected output length

   explicit set_symmetric_difference_shape_check(std::size_t e) : extra(e) {}

   void report(const char* s1, std::size_t n1, const char* s2, std::size_t n2,
               const char* s3, std::size_t n3) const
   {
      BOOST_LIGHTWEIGHT_TEST_OSTREAM
         << "   shapes \"" << s1 << "\"(" << n1 << ") / \"" << s2 << "\"(" << n2
         << ") -> \"" << s3 << "\"(" << n3 << ")" << std::endl;
   }

   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      typedef test_detail::seg_vector<int>           dst_t;
      typedef test_detail::seg_vector<int>::iterator dst_iter_t;

      const boost::container::vector<int> f1 = test_detail::flatten_n_ints(c1, n1);
      const boost::container::vector<int> f2 = test_detail::flatten_n_ints(c2, n2);

      // Naive reference: elements found in exactly one of the two ranges,
      // duplicates surviving by the difference of the two counts.
      boost::container::vector<int> ref;
      {
         std::size_t i = 0, j = 0;
         while(i != f1.size() && j != f2.size()) {
            if      (f1[i] < f2[j]) ref.push_back(f1[i++]);
            else if (f2[j] < f1[i]) ref.push_back(f2[j++]);
            else                  { ++i; ++j; }
         }
         for(; i != f1.size(); ++i) ref.push_back(f1[i]);
         for(; j != f2.size(); ++j) ref.push_back(f2[j]);
      }

      const std::size_t n3 = ref.size() + extra;

      for(std::size_t fam = 0; fam != test_detail::shape_all_families(); ++fam) {
         std::size_t cnt = 0;
         const char* const* dspecs = test_detail::shape_specs_family(fam, 1u, cnt);
         for(std::size_t d = 0; d != cnt; ++d) {
            if(!test_detail::shape_feasible(dspecs[d], n3)) continue;

            dst_t out;
            test_detail::make_dest_range(out, dspecs[d], n3, -1, -999);

            const dst_iter_t r = segmented_set_symmetric_difference
               ( c1.begin(), test_detail::iter_at(c1, n1)
               , c2.begin(), test_detail::iter_at(c2, n2)
               , out.begin());

            if(!BOOST_TEST(r == test_detail::iter_at(out, ref.size())))
               this->report(s1, n1, s2, n2, dspecs[d], n3);

            // flatten_n_ints, not flatten_all_ints: the guard past index n3
            // is not part of the answer, and is checked on its own below.
            const boost::container::vector<int> got = test_detail::flatten_n_ints(out, n3);
            for(std::size_t k = 0; k != n3; ++k) {
               const int want = k < ref.size() ? ref[k] : -1;
               if(!BOOST_TEST_EQ(got[k], want)) {
                  this->report(s1, n1, s2, n2, dspecs[d], n3);
                  break;
               }
            }

            if(!BOOST_TEST(test_detail::filler_intact(out, n3, -999)))
               this->report(s1, n1, s2, n2, dspecs[d], n3);
         }
      }

      // Neither input is an output.
      if(!BOOST_TEST(test_detail::filler_intact(c1, n1, -999)))
         this->report(s1, n1, s2, n2, "-", 0);
      if(!BOOST_TEST(test_detail::filler_intact(c2, n2, -999)))
         this->report(s1, n1, s2, n2, "-", 0);
   }
};

void test_set_symmetric_difference_shape_matrix()
{
   // Sorted, with duplicates inside each range and shared between them.
   const int v1[] = {1, 2, 2, 3, 5, 8};
   const int v2[] = {2, 3, 3, 4, 8, 9};

   static const std::size_t pairs[][2] =
      { {0u, 0u}, {0u, 3u}, {3u, 0u}, {1u, 1u}, {2u, 4u}, {4u, 2u}, {5u, 6u} };
   static const std::size_t extras[] = { 0u, 3u };

   for(std::size_t p = 0; p != sizeof(pairs)/sizeof(pairs[0]); ++p) {
      for(std::size_t e = 0; e != sizeof(extras)/sizeof(extras[0]); ++e) {
         test_detail::for_each_shape2_all<int, int>
            (v1, pairs[p][0], v2, pairs[p][1], -999,
             set_symmetric_difference_shape_check(extras[e]));
      }
   }
}

int main()
{
   test_set_symmetric_difference_shape_matrix();
   test_set_symmetric_difference_basic();
   test_set_symmetric_difference_empty();
   test_set_symmetric_difference_identical();
   test_set_symmetric_difference_disjoint();
   test_set_symmetric_difference_with_comp();
   test_set_symmetric_difference_segmented_input();
   test_set_symmetric_difference_sentinel_segmented();
   test_set_symmetric_difference_sentinel_non_segmented();
   test_set_symmetric_difference_seg2();

   // Single-segment coverage:
   test_set_symmetric_difference_single_segment_input1();
   test_set_symmetric_difference_single_segment_input2();
   test_set_symmetric_difference_single_segment_both_inputs();
   test_set_symmetric_difference_single_segment_output();
   test_set_symmetric_difference_single_segment_inputs_and_output();
   test_set_symmetric_difference_single_segment_inputs_multi_output();
   test_set_symmetric_difference_single_segment_first_input_exhausted();
   test_set_symmetric_difference_single_segment_second_input_exhausted();
   test_set_symmetric_difference_single_segment_seg2_inputs();
   test_set_symmetric_difference_single_segment_empty_input();
   test_set_symmetric_difference_single_segment_sentinel();
   test_set_symmetric_difference_single_segment_with_comp();

   return boost::report_errors();
}
