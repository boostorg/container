//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_is_partitioned.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct less_than_5
{
   bool operator()(int x) const { return x < 5; }
};

struct less_than_threshold
{
   int t;
   less_than_threshold(int v) : t(v) {}
   bool operator()(int x) const { return x < t; }
};

void test_is_partitioned_true()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4};
   int a3[] = {5, 6, 7, 8};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 1);
   sv.add_segment_range(a3, a3 + 4);

   BOOST_TEST(segmented_is_partitioned(sv.begin(), sv.end(), less_than_5()));
}

void test_is_partitioned_empty()
{
   test_detail::seg_vector<int> sv;
   BOOST_TEST(segmented_is_partitioned(sv.begin(), sv.end(), less_than_5()));
}

void test_is_partitioned_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);
   v.push_back(5); v.push_back(6);
   BOOST_TEST(segmented_is_partitioned(v.begin(), v.end(), less_than_5()));

   boost::container::vector<int> v2;
   v2.push_back(1); v2.push_back(6); v2.push_back(3);
   BOOST_TEST(!segmented_is_partitioned(v2.begin(), v2.end(), less_than_5()));
}

void test_is_partitioned_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4};
   int a3[] = {5, 6, 7, 8};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 1);
   sv.add_segment_range(a3, a3 + 4);

   BOOST_TEST(segmented_is_partitioned(sv.begin(), test_detail::make_sentinel(sv.end()), less_than_5()));
}

void test_is_partitioned_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);
   v.push_back(5); v.push_back(6);
   BOOST_TEST(segmented_is_partitioned(v.begin(), test_detail::make_sentinel(v.end()), less_than_5()));

   boost::container::vector<int> v2;
   v2.push_back(1); v2.push_back(6); v2.push_back(3);
   BOOST_TEST(!segmented_is_partitioned(v2.begin(), test_detail::make_sentinel(v2.end()), less_than_5()));
}

void test_is_partitioned_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2};
   int a2[] = {3, 4};
   int a3[] = {5, 7, 8};
   sv2.add_flat_segment_range(a1, a1 + 2);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 3);

   BOOST_TEST(segmented_is_partitioned(sv2.begin(), sv2.end(), less_than_5()));
}

// Runs segmented_is_partitioned over a range whose segmentation shape is
// dictated by a branch spec, so that every level of the recursive dispatch is
// exercised on its single-segment, its multi-segment and its empty-segment
// path. segmented_is_partitioned is segmented_find_if_not followed by
// segmented_none_of over the remainder, so what this adds to those two
// matrices is the handoff between them: the sub-range the second call gets
// starts wherever the first one stopped, which for a multi-segment shape is in
// the middle of a segment.
struct is_partitioned_shape_check
{
   int threshold;

   explicit is_partitioned_shape_check(int t) : threshold(t) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive scan over a flattened copy of the logical range,
      // looking for a satisfying element after a non-satisfying one. The guard
      // past the end is deliberately not part of it; it does satisfy the
      // predicate, so a range ending in non-satisfying elements that overruns
      // reports itself as not partitioned.
      const boost::container::vector<int> flat = test_detail::flatten_n_ints(c, n);
      bool seen_false = false;
      bool expected   = true;
      for(std::size_t i = 0; i != flat.size(); ++i) {
         if(flat[i] < threshold) {
            if(seen_false) { expected = false; break; }
         }
         else {
            seen_false = true;
         }
      }

      BOOST_TEST_EQ(segmented_is_partitioned(first, last, less_than_threshold(threshold)), expected);
      BOOST_TEST(spec != 0);
   }
};

// Bidirectional and forward iterators instantiate every dispatch template
// separately, so each shape is driven through both.
void run_is_partitioned_shapes(const int* vals, std::size_t n, int threshold)
{
   test_detail::for_each_shape_all<int>(vals, n, -999, is_partitioned_shape_check(threshold));
   test_detail::for_each_shape_all_fwd<int>(vals, n, -999, is_partitioned_shape_check(threshold));
}

void test_is_partitioned_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      std::size_t i = 0;
      int vals[16] = { 0 };

      // Properly partitioned, with the boundary at the first position, at
      // every interior one and at the last one, plus the all-satisfying case.
      for(std::size_t p = 0; p <= n; ++p) {
         for(i = 0; i != n; ++i)
            vals[i] = (i < p) ? int(i) : 1000 + int(i);
         run_is_partitioned_shapes(vals, n, 500);
      }

      // One satisfying element in an otherwise non-satisfying range: the
      // violating adjacent pair is the first one when q is 1 and the last one
      // when q is n - 1, and the range is partitioned only when q is 0.
      for(std::size_t q = 0; q != n; ++q) {
         for(i = 0; i != n; ++i)
            vals[i] = (i == q) ? 0 : 1000 + int(i);
         run_is_partitioned_shapes(vals, n, 500);
      }

      // The mirror image: one non-satisfying element in an otherwise
      // satisfying range, which is partitioned only when that element is last.
      for(std::size_t q = 0; q != n; ++q) {
         for(i = 0; i != n; ++i)
            vals[i] = (i == q) ? 1000 : int(i);
         run_is_partitioned_shapes(vals, n, 500);
      }
   }
}

//----------------------------------------------------------------------------
// Single-segment cases with data before the start of the range. The shape
// matrix always starts its range at the container's first element, so the
// lower bound is only exercised here: the element before each range below
// breaks the partition, and so does the guard just past its end.
//----------------------------------------------------------------------------

void test_is_partitioned_single_segment_interior()
{
   //The element before the range breaks the partition, the guard after it does
   //too, so both bounds have to be honoured for these to come out right.
   test_detail::seg_vector<int> ok;
   int a[] = {9, 1, 2, 3, 7, 8, 4};
   ok.add_segment_range(a, a + 7);
   BOOST_TEST(segmented_is_partitioned
      (test_detail::iter_at(ok, 1), test_detail::iter_at(ok, 6), less_than_5()));

   test_detail::seg_vector<int> bad;
   int b[] = {9, 1, 7, 2, 8, 3, 4};
   bad.add_segment_range(b, b + 7);
   BOOST_TEST(!segmented_is_partitioned
      (test_detail::iter_at(bad, 1), test_detail::iter_at(bad, 6), less_than_5()));
}

void test_is_partitioned_single_segment_sentinel()
{
   test_detail::seg_vector<int> ok;
   int a[] = {9, 1, 2, 3, 7, 8, 4};
   ok.add_segment_range(a, a + 7);
   BOOST_TEST(segmented_is_partitioned(test_detail::iter_at(ok, 1),
      test_detail::make_sentinel(test_detail::iter_at(ok, 6)), less_than_5()));

   test_detail::seg_vector<int> bad;
   int b[] = {9, 1, 7, 2, 8, 3, 4};
   bad.add_segment_range(b, b + 7);
   BOOST_TEST(!segmented_is_partitioned(test_detail::iter_at(bad, 1),
      test_detail::make_sentinel(test_detail::iter_at(bad, 6)), less_than_5()));
}

void test_is_partitioned_single_segment_seg2_inner_multi()
{
   test_detail::seg_vector<int> ok_inner;
   int a1[] = {9, 1, 2};
   int a2[] = {3, 7, 8, 4};
   ok_inner.add_segment_range(a1, a1 + 3);
   ok_inner.add_segment_range(a2, a2 + 4);
   test_detail::seg2_vector<int> ok;
   ok.add_segment(ok_inner);
   BOOST_TEST(segmented_is_partitioned
      (test_detail::iter_at(ok, 1), test_detail::iter_at(ok, 6), less_than_5()));

   test_detail::seg_vector<int> bad_inner;
   int b1[] = {9, 1, 7};
   int b2[] = {2, 8, 3, 4};
   bad_inner.add_segment_range(b1, b1 + 3);
   bad_inner.add_segment_range(b2, b2 + 4);
   test_detail::seg2_vector<int> bad;
   bad.add_segment(bad_inner);
   BOOST_TEST(!segmented_is_partitioned
      (test_detail::iter_at(bad, 1), test_detail::iter_at(bad, 6), less_than_5()));
}

void test_is_partitioned_single_segment_seg2_single_inner()
{
   test_detail::seg_vector<int> ok_inner;
   int a[] = {9, 1, 2, 3, 7, 8, 4};
   ok_inner.add_segment_range(a, a + 7);
   test_detail::seg2_vector<int> ok;
   ok.add_segment(ok_inner);
   BOOST_TEST(segmented_is_partitioned
      (test_detail::iter_at(ok, 1), test_detail::iter_at(ok, 6), less_than_5()));

   test_detail::seg_vector<int> bad_inner;
   int b[] = {9, 1, 7, 2, 8, 3, 4};
   bad_inner.add_segment_range(b, b + 7);
   test_detail::seg2_vector<int> bad;
   bad.add_segment(bad_inner);
   BOOST_TEST(!segmented_is_partitioned
      (test_detail::iter_at(bad, 1), test_detail::iter_at(bad, 6), less_than_5()));
}

// The whole suite instantiates the bidirectional category only; the forward
// one is a different instantiation of every dispatch template.
void test_is_partitioned_single_segment_forward()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;

   cont_t whole;
   int w[] = {1, 2, 3, 7, 8, 9, 4};
   whole.add_segment_range(w, w + 7);
   BOOST_TEST(segmented_is_partitioned(whole.begin(), test_detail::iter_at(whole, 6), less_than_5()));

   cont_t ok;
   int a[] = {9, 1, 2, 3, 7, 8, 4};
   ok.add_segment_range(a, a + 7);
   BOOST_TEST(segmented_is_partitioned
      (test_detail::iter_at(ok, 1), test_detail::iter_at(ok, 6), less_than_5()));

   cont_t bad;
   int b[] = {9, 1, 7, 2, 8, 3, 4};
   bad.add_segment_range(b, b + 7);
   BOOST_TEST(!segmented_is_partitioned
      (test_detail::iter_at(bad, 1), test_detail::iter_at(bad, 6), less_than_5()));
}

// Forward category, multi-segment, with the partition violation in an earlier
// segment while the last segment is partitioned on its own.
void test_is_partitioned_forward_violation_before_last_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;

   cont_t sv;
   int a1[] = {1, 2, 3};
   int a2[] = {7, 4};
   int a3[] = {8, 9, 10};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 3);

   BOOST_TEST(!segmented_is_partitioned(sv.begin(), test_detail::iter_at(sv, 7), less_than_5()));
}

int main()
{
   test_is_partitioned_shape_matrix();
   test_is_partitioned_true();
   test_is_partitioned_empty();
   test_is_partitioned_non_segmented();
   test_is_partitioned_sentinel_segmented();
   test_is_partitioned_sentinel_non_segmented();
   test_is_partitioned_seg2();
   test_is_partitioned_single_segment_interior();
   test_is_partitioned_single_segment_sentinel();
   test_is_partitioned_single_segment_seg2_inner_multi();
   test_is_partitioned_single_segment_seg2_single_inner();
   test_is_partitioned_single_segment_forward();
   test_is_partitioned_forward_violation_before_last_segment();
   return boost::report_errors();
}
