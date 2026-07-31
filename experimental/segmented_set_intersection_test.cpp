//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_set_intersection.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_set_intersection_basic()
{
   int a[] = {1, 3, 5, 7};
   int b[] = {2, 3, 6, 7, 8};
   boost::container::vector<int> out(5, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_intersection(a, a + 4, b, b + 5, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 2u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 7);
}

void test_set_intersection_empty()
{
   int a[] = {1, 2, 3};
   int* empty = a;
   boost::container::vector<int> out(3, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_intersection(a, a + 3, empty, empty, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 0u);
}

void test_set_intersection_disjoint()
{
   int a[] = {1, 3, 5};
   int b[] = {2, 4, 6};
   boost::container::vector<int> out(3, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_intersection(a, a + 3, b, b + 3, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 0u);
}

void test_set_intersection_identical()
{
   int a[] = {1, 2, 3};
   int b[] = {1, 2, 3};
   boost::container::vector<int> out(3, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_intersection(a, a + 3, b, b + 3, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 3u);
   BOOST_TEST_EQ(out[0], 1);
   BOOST_TEST_EQ(out[1], 2);
   BOOST_TEST_EQ(out[2], 3);
}

struct greater_int
{
   bool operator()(int a, int b) const { return a > b; }
};

void test_set_intersection_with_comp()
{
   int a[] = {7, 5, 3, 1};
   int b[] = {8, 7, 6, 3, 2};
   boost::container::vector<int> out(5, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_intersection(a, a + 4, b, b + 5, out.begin(), greater_int());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 2u);
   BOOST_TEST_EQ(out[0], 7);
   BOOST_TEST_EQ(out[1], 3);
}

void test_set_intersection_segmented_input()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 3};
   int a2[] = {5, 7};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   int b[] = {2, 3, 5, 9};
   boost::container::vector<int> out(4, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_intersection(sv.begin(), sv.end(), b, b + 4, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 2u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 5);
}

void test_set_intersection_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 3};
   int a2[] = {5, 7};
   sv.add_segment_range(a1, a1 + 2);
   sv.add_segment_range(a2, a2 + 2);

   int b[] = {2, 3, 5, 9};
   boost::container::vector<int> out(4, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_intersection(sv.begin(), test_detail::make_sentinel(sv.end()),
                                 b, test_detail::make_sentinel(b + 4), out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 2u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 5);
}

void test_set_intersection_sentinel_non_segmented()
{
   int a[] = {1, 3, 5, 7};
   int b[] = {2, 3, 6, 7, 8};
   boost::container::vector<int> out(5, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_intersection(a, test_detail::make_sentinel(a + 4),
                                 b, test_detail::make_sentinel(b + 5), out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 2u);
   BOOST_TEST_EQ(out[0], 3);
   BOOST_TEST_EQ(out[1], 7);
}

void test_set_intersection_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);
   sv2.add_flat_segment_range(a3, a3 + 3);

   int b[] = {2, 5, 8};
   boost::container::vector<int> out(9, 0);

   boost::container::vector<int>::iterator end_it =
      segmented_set_intersection(sv2.begin(), sv2.end(), b, b + 3, out.begin());

   std::size_t n = static_cast<std::size_t>(end_it - out.begin());
   BOOST_TEST_EQ(n, 3u);
   BOOST_TEST_EQ(out[0], 2);
   BOOST_TEST_EQ(out[1], 5);
   BOOST_TEST_EQ(out[2], 8);
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
// so their intersection is {2, 3, 8}.
//////////////////////////////////////////////////////////////////////////////

// M4: single-segment first input, flat second input.
void test_set_intersection_single_segment_input1()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   int b[] = {2, 3, 4, 8, 9};
   boost::container::vector<int> out(12, -1);

   boost::container::vector<int>::iterator r = segmented_set_intersection(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      b, b + 5, out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   int expected[] = {2, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// M4: flat first input, single-segment second input.
void test_set_intersection_single_segment_input2()
{
   int a[] = {1, 2, 3, 5, 8};

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   boost::container::vector<int> out(12, -1);

   boost::container::vector<int>::iterator r = segmented_set_intersection(
      a, a + 5,
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   int expected[] = {2, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// M4: both inputs single-segment.
void test_set_intersection_single_segment_both_inputs()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   boost::container::vector<int> out(12, -1);

   boost::container::vector<int>::iterator r = segmented_set_intersection(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   int expected[] = {2, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// M1: multi-segment inputs, single-segment segmented output.
void test_set_intersection_single_segment_output()
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
   iter_t r = segmented_set_intersection(
      sv1.begin(), sv1.end(), sv2.begin(), sv2.end(), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 3));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 12u);
   int expected[] = {2, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// M3: single-segment inputs and single-segment output.
void test_set_intersection_single_segment_inputs_and_output()
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
   iter_t r = segmented_set_intersection(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 3));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 12u);
   int expected[] = {2, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// M2: single-segment inputs, multi-segment output.  The first output segment
// is filled exactly, so the walker has to step to the next one.
void test_set_intersection_single_segment_inputs_multi_output()
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
   iter_t r = segmented_set_intersection(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 3));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 12u);
   int expected[] = {2, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// The first input is exhausted well before the second.
void test_set_intersection_single_segment_first_input_exhausted()
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
   iter_t r = segmented_set_intersection(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 3),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 1));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 10u);
   int expected[] = {2, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// Mirror image: the second input is exhausted first.
void test_set_intersection_single_segment_second_input_exhausted()
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
   iter_t r = segmented_set_intersection(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 3), out.begin());

   BOOST_TEST(r == test_detail::iter_at(out, 1));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 10u);
   int expected[] = {2, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < got.size(); ++i)
      BOOST_TEST_EQ(got[i], expected[i]);
}

// S5 and S6: one outer segment holding several inner segments, and one outer
// segment holding exactly one inner segment.
void test_set_intersection_single_segment_seg2_inputs()
{
   int a[] = {1, 2, 3, 5, 8};
   int b[] = {2, 3, 4, 8, 9};

   test_detail::seg2_vector<int> sv1;
   test_detail::make_range(sv1, "sm", a, 5, 100);
   test_detail::seg2_vector<int> sv2;
   test_detail::make_range(sv2, "ss", b, 5, 100);

   boost::container::vector<int> out(12, -1);
   boost::container::vector<int>::iterator r = segmented_set_intersection(
      sv1.begin(), test_detail::iter_at(sv1, 5),
      sv2.begin(), test_detail::iter_at(sv2, 5), out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   int expected[] = {2, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// S3: an empty first input positioned mid-segment.
void test_set_intersection_single_segment_empty_input()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 6);

   int b[] = {2, 3, 4};
   boost::container::vector<int> out(6, -1);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t mid = test_detail::iter_at(sv1, 3);
   boost::container::vector<int>::iterator r =
      segmented_set_intersection(mid, mid, b, b + 3, out.begin());

   BOOST_TEST(r == out.begin());
   for(std::size_t i = 0; i < 6; ++i)
      BOOST_TEST_EQ(out[i], -1);
}

// S4: single-segment sub-ranges closed by sentinels.
void test_set_intersection_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv1;
   int a[] = {-100, 1, 2, 3, 5, 8, 100};
   sv1.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> sv2;
   int b[] = {-100, 2, 3, 4, 8, 9, 100};
   sv2.add_segment_range(b, b + 7);

   boost::container::vector<int> out(12, -1);
   boost::container::vector<int>::iterator r = segmented_set_intersection(
      test_detail::iter_at(sv1, 1), test_detail::make_sentinel(test_detail::iter_at(sv1, 6)),
      test_detail::iter_at(sv2, 1), test_detail::make_sentinel(test_detail::iter_at(sv2, 6)),
      out.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(r - out.begin()), 3u);
   int expected[] = {2, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1};
   for(std::size_t i = 0; i < 12; ++i)
      BOOST_TEST_EQ(out[i], expected[i]);
}

// The comparator-taking overload on single-segment inputs and output.
void test_set_intersection_single_segment_with_comp()
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
   iter_t r = segmented_set_intersection(
      test_detail::iter_at(sv1, 1), test_detail::iter_at(sv1, 6),
      test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6),
      out.begin(), greater_int());

   BOOST_TEST(r == test_detail::iter_at(out, 3));
   boost::container::vector<int> got = test_detail::flatten_all_ints(out);
   BOOST_TEST_EQ(got.size(), 12u);
   int expected[] = {8, 3, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1};
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
// The destination is enumerated by for_each_dest_shape_all, so it is walked at
// both depths and over both spec families.  A two-level destination is what
// reaches the segmented_iterator_tag overload of set_intersection_dst_bounded,
// where the local iterator handed to the level below is itself still
// segmented.
//
// Two destination sizes are run per combination.  The output length is
// data-dependent, so the guard at index n3 only catches an overrun of the
// *allocated* destination; sizing the destination to exactly the expected
// output length is what turns it into a check on the *written* prefix.  The
// second size leaves three unwritten slots, which must still hold the fill.
//////////////////////////////////////////////////////////////////////////////

//! One destination shape: runs the algorithm into it and checks the result,
//! the written prefix, the untouched tail and the guard.
struct set_intersection_dst_check
{
   const boost::container::vector<int>* ref;
   const char* s1;
   std::size_t n1;
   const char* s2;
   std::size_t n2;

   set_intersection_dst_check(const boost::container::vector<int>& r,
                              const char* a1, std::size_t b1,
                              const char* a2, std::size_t b2)
      : ref(&r), s1(a1), n1(b1), s2(a2), n2(b2)
   {}

   void report(const char* s3, std::size_t n3) const
   {
      BOOST_LIGHTWEIGHT_TEST_OSTREAM
         << "   shapes \"" << s1 << "\"(" << n1 << ") / \"" << s2 << "\"(" << n2
         << ") -> \"" << s3 << "\"(" << n3 << ")" << std::endl;
   }

   template<class C1, class C2, class Dst>
   void run(C1& c1, C2& c2, Dst& out, std::size_t n3, const char* s3) const
   {
      typedef typename Dst::iterator dst_iter_t;

      const dst_iter_t r = segmented_set_intersection
         ( c1.begin(), test_detail::iter_at(c1, n1)
         , c2.begin(), test_detail::iter_at(c2, n2)
         , out.begin());

      if(!BOOST_TEST(r == test_detail::iter_at(out, ref->size())))
         this->report(s3, n3);

      // flatten_n_ints, not flatten_all_ints: the guard past index n3
      // is not part of the answer, and is checked on its own below.
      const boost::container::vector<int> got = test_detail::flatten_n_ints(out, n3);
      for(std::size_t k = 0; k != n3; ++k) {
         const int want = k < ref->size() ? (*ref)[k] : -1;
         if(!BOOST_TEST_EQ(got[k], want)) {
            this->report(s3, n3);
            break;
         }
      }

      if(!BOOST_TEST(test_detail::filler_intact(out, n3, -999)))
         this->report(s3, n3);
   }
};

//! Binds the two inputs so the destination combinator can supply the third.
template<class C1, class C2>
struct set_intersection_dst_bind
{
   C1* c1;
   C2* c2;
   set_intersection_dst_check chk;

   set_intersection_dst_bind(C1& a, C2& b, const set_intersection_dst_check& c)
      : c1(&a), c2(&b), chk(c)
   {}

   template<class Dst>
   void operator()(Dst& out, std::size_t n3, const char* s3) const
   {  chk.run(*c1, *c2, out, n3, s3);  }
};

struct set_intersection_shape_check
{
   std::size_t extra;   // destination slots beyond the expected output length

   explicit set_intersection_shape_check(std::size_t e) : extra(e) {}

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
      const boost::container::vector<int> f1 = test_detail::flatten_n_ints(c1, n1);
      const boost::container::vector<int> f2 = test_detail::flatten_n_ints(c2, n2);

      // Naive reference: elements present in both, duplicates taken the
      // smaller number of times the two ranges hold them.
      boost::container::vector<int> ref;
      {
         std::size_t i = 0, j = 0;
         while(i != f1.size() && j != f2.size()) {
            if      (f1[i] < f2[j]) ++i;
            else if (f2[j] < f1[i]) ++j;
            else                  { ref.push_back(f1[i++]); ++j; }
         }
      }

      const std::size_t n3 = ref.size() + extra;

      const set_intersection_dst_bind<C1, C2> dst
         (c1, c2, set_intersection_dst_check(ref, s1, n1, s2, n2));
      test_detail::for_each_dest_shape_all<int>(n3, -1, -999, dst);

      // Neither input is an output.
      if(!BOOST_TEST(test_detail::filler_intact(c1, n1, -999)))
         this->report(s1, n1, s2, n2, "-", 0);
      if(!BOOST_TEST(test_detail::filler_intact(c2, n2, -999)))
         this->report(s1, n1, s2, n2, "-", 0);
   }
};

void test_set_intersection_shape_matrix()
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
             set_intersection_shape_check(extras[e]));
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Comparison count.
//
// [set.intersection] mandates "At most 2 * ((last1 - first1) + (last2 -
// first2)) - 1 comparisons".  The destination is laid out in segments of one,
// two and three elements as well as flat, since it is the destination
// boundaries that make the leaf return mid-merge and the walker call it again.
//////////////////////////////////////////////////////////////////////////////

struct less_int
{
   bool operator()(int a, int b) const { return a < b; }
};

inline std::size_t set_intersection_comparison_bound(std::size_t n1, std::size_t n2)
{
   const std::size_t total = n1 + n2;
   return total ? 2u*total - 1u : 0u;
}

struct set_intersection_count_check
{
   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      boost::container::vector<int> flat(n1 + n2 + 1u, -1);
      {
         test_detail::op_counter calls;
         segmented_set_intersection(c1.begin(), test_detail::iter_at(c1, n1),
                                    c2.begin(), test_detail::iter_at(c2, n2),
                                    flat.begin(), test_detail::counting_pred(calls, less_int()));
         BOOST_TEST(calls.n <= set_intersection_comparison_bound(n1, n2));
      }

      for(std::size_t block = 1u; block <= 3u; ++block) {
         test_detail::seg_vector<int> out;
         for(std::size_t room = 0; room <= n1 + n2; room += block)
            out.add_segment(block, -1);

         test_detail::op_counter calls;
         segmented_set_intersection(c1.begin(), test_detail::iter_at(c1, n1),
                                    c2.begin(), test_detail::iter_at(c2, n2),
                                    out.begin(), test_detail::counting_pred(calls, less_int()));
         BOOST_TEST(calls.n <= set_intersection_comparison_bound(n1, n2));
      }

      BOOST_TEST(s1 != 0 && s2 != 0);
   }
};

void test_set_intersection_comparison_count()
{
   const int v1[] = {1, 2, 2, 3, 5, 8};
   const int v2[] = {2, 3, 3, 4, 8, 9};

   static const std::size_t pairs[][2] =
      { {0u, 0u}, {0u, 3u}, {3u, 0u}, {1u, 1u}, {2u, 4u}, {4u, 2u}, {5u, 6u} };

   for(std::size_t p = 0; p != sizeof(pairs)/sizeof(pairs[0]); ++p)
      test_detail::for_each_shape2_all<int, int>
         (v1, pairs[p][0], v2, pairs[p][1], -999, set_intersection_count_check());
}

int main()
{
   test_set_intersection_shape_matrix();
   test_set_intersection_basic();
   test_set_intersection_empty();
   test_set_intersection_disjoint();
   test_set_intersection_identical();
   test_set_intersection_with_comp();
   test_set_intersection_segmented_input();
   test_set_intersection_sentinel_segmented();
   test_set_intersection_sentinel_non_segmented();
   test_set_intersection_seg2();

   // Single-segment coverage:
   test_set_intersection_single_segment_input1();
   test_set_intersection_single_segment_input2();
   test_set_intersection_single_segment_both_inputs();
   test_set_intersection_single_segment_output();
   test_set_intersection_single_segment_inputs_and_output();
   test_set_intersection_single_segment_inputs_multi_output();
   test_set_intersection_single_segment_first_input_exhausted();
   test_set_intersection_single_segment_second_input_exhausted();
   test_set_intersection_single_segment_seg2_inputs();
   test_set_intersection_single_segment_empty_input();
   test_set_intersection_single_segment_sentinel();
   test_set_intersection_single_segment_with_comp();

   test_set_intersection_comparison_count();
   return boost::report_errors();
}
