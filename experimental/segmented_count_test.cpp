//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_count.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_count_empty()
{
   test_detail::seg_vector<int> sv;
   BOOST_TEST_EQ(segmented_count(sv.begin(), sv.end(), 0), 0);
}

void test_count_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(2);
   v.push_back(1);
   v.push_back(3);
   v.push_back(1);

   BOOST_TEST_EQ(segmented_count(v.begin(), v.end(), 1), 3);
}

void test_count_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 1};
   int a2[] = {3, 1, 4};
   int a3[] = {1, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 2);

   BOOST_TEST_EQ(segmented_count(sv.begin(), test_detail::make_sentinel(sv.end()), 1), 4);
   BOOST_TEST_EQ(segmented_count(sv.begin(), test_detail::make_sentinel(sv.end()), 99), 0);
}

void test_count_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(2);
   v.push_back(1);
   v.push_back(3);
   v.push_back(1);

   BOOST_TEST_EQ(segmented_count(v.begin(), test_detail::make_sentinel(v.end()), 1), 3);
}

// Runs segmented_count over a range whose segmentation shape is dictated by a
// branch spec, so that every level of the recursive dispatch is exercised on
// its single-segment, its multi-segment and its empty-segment path. Unlike the
// early-exit algorithms, count has to visit the whole range and add up the
// per-segment subtotals, so a segment counted twice or skipped shows up here.
struct count_shape_check
{
   int value;

   explicit count_shape_check(int v) : value(v) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive count over a flattened copy of the logical range. The
      // guard past the end is deliberately not part of it.
      const boost::container::vector<int> flat = test_detail::flatten_n_ints(c, n);
      std::ptrdiff_t expected = 0;
      for(std::size_t i = 0; i != flat.size(); ++i) {
         if(flat[i] == value) ++expected;
      }

      BOOST_TEST_EQ(segmented_count(first, last, value), expected);
      BOOST_TEST(spec != 0);
   }
};

// Bidirectional and forward iterators instantiate every dispatch template
// separately, so each shape is driven through both.
void run_count_shapes(const int* vals, std::size_t n, int value)
{
   test_detail::for_each_shape_all<int>(vals, n, -999, count_shape_check(value));
   test_detail::for_each_shape_all_fwd<int>(vals, n, -999, count_shape_check(value));
}

void test_count_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16] = { 0 };
      std::size_t i = 0;

      // All distinct: the single occurrence is walked across the first, every
      // interior and the last position by the target sweep, which also asks
      // for a value no element has and for the out-of-range guard value.
      for(i = 0; i != n; ++i)
         vals[i] = int(i) + 1;
      for(std::size_t v = 0; v <= n + 1u; ++v)
         run_count_shapes(vals, n, (v <= n) ? int(v) : -999);

      // Every element matches, so the count equals the whole range length.
      for(i = 0; i != n; ++i)
         vals[i] = 7;
      run_count_shapes(vals, n, 7);
      run_count_shapes(vals, n, 8);
      run_count_shapes(vals, n, -999);

      // Alternating and every-third layouts: several matches, spread so that
      // every segment of every shape contributes a subtotal.
      for(i = 0; i != n; ++i)
         vals[i] = 7 + int(i % 2u);
      run_count_shapes(vals, n, 7);
      run_count_shapes(vals, n, 8);

      for(i = 0; i != n; ++i)
         vals[i] = (i % 3u == 0u) ? 7 : 100 + int(i);
      run_count_shapes(vals, n, 7);

      // Exactly one match, at every position in turn, against a uniform
      // background: the complementary count is then n - 1.
      for(std::size_t p = 0; p != n; ++p) {
         for(i = 0; i != n; ++i)
            vals[i] = (i == p) ? 7 : 8;
         run_count_shapes(vals, n, 7);
         run_count_shapes(vals, n, 8);
      }
   }
}

//----------------------------------------------------------------------------
// Single-segment cases with data before the start of the range. The shape
// matrix always starts its range at the container's first element, so the
// lower bound is only exercised here: every range below keeps occurrences of
// the counted value just outside both of its bounds, so an off-by-one on
// either side changes the count.
//----------------------------------------------------------------------------

void test_count_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 1, 3, 1, 4};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST_EQ(segmented_count(first, last, 1), 2);
   BOOST_TEST_EQ(segmented_count(first, last, 2), 1);
   BOOST_TEST_EQ(segmented_count(first, last, 4), 0);
   BOOST_TEST_EQ(segmented_count(first, last, 99), 0);
}

void test_count_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 1, 3, 1, 4};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST_EQ(segmented_count(first, test_detail::make_sentinel(last), 1), 2);
   BOOST_TEST_EQ(segmented_count(first, test_detail::make_sentinel(last), 4), 0);
}

void test_count_single_segment_seg2_inner_multi()
{
   test_detail::seg_vector<int> inner;
   int a1[] = {1, 2, 1};
   int a2[] = {3, 1, 4, 1};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 4);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 6);

   BOOST_TEST_EQ(segmented_count(first, last, 1), 2);
   BOOST_TEST_EQ(segmented_count(first, last, 4), 1);
   BOOST_TEST_EQ(segmented_count(first, last, 99), 0);
}

void test_count_single_segment_seg2_single_inner()
{
   test_detail::seg_vector<int> inner;
   int a[] = {1, 2, 1, 3, 1, 4, 1};
   inner.add_segment_range(a, a + 7);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 5);

   BOOST_TEST_EQ(segmented_count(first, last, 1), 2);
   BOOST_TEST_EQ(segmented_count(first, last, 4), 0);
   BOOST_TEST_EQ(segmented_count(first, last, 99), 0);
}

// The whole suite instantiates the bidirectional category only; the forward
// one is a different instantiation of every dispatch template.
void test_count_single_segment_forward()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;
   typedef cont_t::iterator iter_t;

   cont_t sv;
   int a[] = {1, 2, 1, 3, 1, 4, 1};
   sv.add_segment_range(a, a + 7);

   const iter_t whole_last = test_detail::iter_at(sv, 6);
   BOOST_TEST_EQ(segmented_count(sv.begin(), whole_last, 1), 3);
   BOOST_TEST_EQ(segmented_count(sv.begin(), whole_last, 99), 0);

   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);
   BOOST_TEST_EQ(segmented_count(first, last, 1), 2);
   BOOST_TEST_EQ(segmented_count(first, last, 4), 0);
}

// Forward category, multi-segment: the per-segment counts must all be added up.
void test_count_forward_multi_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;

   cont_t sv;
   int a1[] = {1, 2, 1};
   int a2[] = {3, 1};
   int a3[] = {1, 4, 1};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 3);

   BOOST_TEST_EQ(segmented_count(sv.begin(), test_detail::iter_at(sv, 7), 1), 4);
   BOOST_TEST_EQ(segmented_count(sv.begin(), test_detail::iter_at(sv, 7), 99), 0);
}

//////////////////////////////////////////////////////////////////////////////
// Comparison count.
//
// [alg.count] mandates "Exactly last - first applications of the corresponding
// predicate", so an element compared twice at a segment boundary is a
// conformance failure, not just a slowdown.  There is no comparator overload,
// so the count is taken from the value type itself.
//////////////////////////////////////////////////////////////////////////////

struct count_comparison_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      const test_detail::counted_int target(3);

      test_detail::counted_int_ops().reset();
      segmented_count(c.begin(), test_detail::iter_at(c, n), target);
      const std::size_t applied = test_detail::counted_int_ops().cmp;

      BOOST_TEST_EQ(applied, n);
      BOOST_TEST(spec != 0);
   }
};

void test_count_comparison_count()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];

      // No match, every element a match, and a scattering of matches.
      for(std::size_t i = 0; i != 16u; ++i)
         vals[i] = int(i) + 100;
      test_detail::for_each_shape_all<test_detail::counted_int>
         (vals, n, -999, count_comparison_check());

      for(std::size_t i = 0; i != 16u; ++i)
         vals[i] = 3;
      test_detail::for_each_shape_all<test_detail::counted_int>
         (vals, n, -999, count_comparison_check());

      for(std::size_t i = 0; i != 16u; ++i)
         vals[i] = int(i) % 3 == 0 ? 3 : int(i) + 100;
      test_detail::for_each_shape_all<test_detail::counted_int>
         (vals, n, -999, count_comparison_check());
   }
}

int main()
{
   test_count_shape_matrix();
   test_count_empty();
   test_count_non_segmented();
   test_count_sentinel_segmented();
   test_count_sentinel_non_segmented();
   test_count_single_segment_interior();
   test_count_single_segment_sentinel();
   test_count_single_segment_seg2_inner_multi();
   test_count_single_segment_seg2_single_inner();
   test_count_single_segment_forward();
   test_count_forward_multi_segment();
   test_count_comparison_count();
   return boost::report_errors();
}
