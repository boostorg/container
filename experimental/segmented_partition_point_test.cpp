//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_partition_point.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct less_than_5 {
   bool operator()(int x) const { return x < 5; }
};

struct less_than_threshold {
   int t;
   less_than_threshold(int v) : t(v) {}
   bool operator()(int x) const { return x < t; }
};

void test_partition_point_empty()
{
   test_detail::seg_vector<int> sv;
   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t pp = segmented_partition_point(sv.begin(), sv.end(), less_than_5());
   BOOST_TEST(pp == sv.end());
}

void test_partition_point_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(3); v.push_back(4);
   v.push_back(7); v.push_back(8); v.push_back(9);

   boost::container::vector<int>::iterator pp = segmented_partition_point(v.begin(), v.end(), less_than_5());
   BOOST_TEST_EQ(*pp, 7);
}

void test_partition_point_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 2);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t pp = segmented_partition_point(sv.begin(), test_detail::make_sentinel(sv.end()), less_than_5());
   BOOST_TEST_EQ(*pp, 5);
}

void test_partition_point_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(3); v.push_back(4);
   v.push_back(7); v.push_back(8); v.push_back(9);

   boost::container::vector<int>::iterator pp = segmented_partition_point(v.begin(), test_detail::make_sentinel(v.end()), less_than_5());
   BOOST_TEST_EQ(*pp, 7);
}

void test_partition_point_every_position()
{
   const int N = 9;

   for(int p = 0; p <= N; ++p) {
      int data[9];
      for(int j = 0; j < N; ++j)
         data[j] = (j < p) ? j : j + 100;

      test_detail::seg_vector<int> sv;
      sv.add_segment_range(data, data + 3);
      sv.add_segment_range(data + 3, data + 5);
      sv.add_segment_range(data + 5, data + 9);

      typedef test_detail::seg_vector<int>::iterator iter_t;
      iter_t result = segmented_partition_point(sv.begin(), sv.end(), less_than_threshold(100));

      iter_t expected = sv.begin();
      for(int j = 0; j < p; ++j) ++expected;

      BOOST_TEST(result == expected);
   }
}

void test_partition_point_every_position_seg2()
{
   const int N = 9;

   for(int p = 0; p <= N; ++p) {
      int data[9];
      for(int j = 0; j < N; ++j)
         data[j] = (j < p) ? j : j + 100;

      test_detail::seg2_vector<int> sv2;
      sv2.add_flat_segment_range(data, data + 3);
      sv2.add_flat_segment_range(data + 3, data + 5);
      sv2.add_flat_segment_range(data + 5, data + 9);

      typedef test_detail::seg2_vector<int>::iterator iter_t;
      iter_t result = segmented_partition_point(sv2.begin(), sv2.end(), less_than_threshold(100));

      iter_t expected = sv2.begin();
      for(int j = 0; j < p; ++j) ++expected;

      BOOST_TEST(result == expected);
   }
}

// Runs segmented_partition_point over a range whose segmentation shape is
// dictated by a branch spec, so that every level of the recursive dispatch is
// exercised on its single-segment, its multi-segment and its empty-segment
// path.
struct partition_point_shape_check
{
   int threshold;

   explicit partition_point_shape_check(int t) : threshold(t) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive scan over a flattened copy of the logical range. The
      // guard past the end is deliberately not part of it; it does satisfy the
      // predicate, so an all-true range that overruns reports a point past
      // "last" instead of "last" itself.
      const boost::container::vector<int> flat = test_detail::flatten_n_ints(c, n);
      std::size_t expected = flat.size();
      for(std::size_t i = 0; i != flat.size(); ++i) {
         if(!(flat[i] < threshold)) { expected = i; break; }
      }

      const iter_t r = segmented_partition_point(first, last, less_than_threshold(threshold));
      BOOST_TEST(r == test_detail::iter_at(c, expected));
      BOOST_TEST(spec != 0);
   }
};

// Bidirectional and forward iterators instantiate every dispatch template
// separately, so each shape is driven through both.
void run_partition_point_shapes(const int* vals, std::size_t n, int threshold)
{
   test_detail::for_each_shape_all<int>(vals, n, -999, partition_point_shape_check(threshold));
   test_detail::for_each_shape_all_fwd<int>(vals, n, -999, partition_point_shape_check(threshold));
}

void test_partition_point_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      // A properly partitioned range with the partition point at the first
      // position, at every interior one and at the last one, plus the
      // all-true case where the point is "last" itself.
      for(std::size_t p = 0; p <= n; ++p) {
         int vals[16] = { 0 };
         for(std::size_t i = 0; i != n; ++i)
            vals[i] = (i < p) ? int(i) : 1000 + int(i);
         run_partition_point_shapes(vals, n, 500);
      }
   }
}

//----------------------------------------------------------------------------
// Single-segment cases with data before the start of the range. The shape
// matrix always starts its range at the container's first element, so the
// lower bound is only exercised here. Each range is followed by an element
// that satisfies the predicate and then by one that does not, so an all-true
// range that overruns its end reports a partition point past "last" instead
// of "last" itself.
//----------------------------------------------------------------------------

void test_partition_point_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {0, 1, 2, 300, 400, 500, 3, 5000};
   sv.add_segment_range(a, a + 8);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   BOOST_TEST(segmented_partition_point(first, last, less_than_threshold(100)) == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_partition_point(first, last, less_than_threshold(1000)) == last);
   BOOST_TEST(segmented_partition_point(first, last, less_than_threshold(1)) == first);
}

void test_partition_point_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {0, 1, 2, 300, 400, 500, 3, 5000};
   sv.add_segment_range(a, a + 8);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   BOOST_TEST(segmented_partition_point(first, test_detail::make_sentinel(last), less_than_threshold(100))
              == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_partition_point(first, test_detail::make_sentinel(last), less_than_threshold(1000)) == last);
}

void test_partition_point_single_segment_seg2_inner_multi()
{
   test_detail::seg_vector<int> inner;
   int a1[] = {0, 1, 2};
   int a2[] = {300, 400, 500, 3, 5000};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 5);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 6);

   BOOST_TEST(segmented_partition_point(first, last, less_than_threshold(100)) == test_detail::iter_at(sv2, 3));
   BOOST_TEST(segmented_partition_point(first, last, less_than_threshold(1000)) == last);
   BOOST_TEST(segmented_partition_point(first, last, less_than_threshold(1)) == first);
}

void test_partition_point_single_segment_seg2_single_inner()
{
   test_detail::seg_vector<int> inner;
   int a[] = {0, 1, 2, 300, 400, 500, 3, 5000};
   inner.add_segment_range(a, a + 8);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 6);

   BOOST_TEST(segmented_partition_point(first, last, less_than_threshold(100)) == test_detail::iter_at(sv2, 3));
   BOOST_TEST(segmented_partition_point(first, last, less_than_threshold(1000)) == last);
   BOOST_TEST(segmented_partition_point(first, last, less_than_threshold(1)) == first);
}

// The whole suite instantiates the bidirectional category only; the forward
// one is a different instantiation of every dispatch template.
void test_partition_point_single_segment_forward()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;
   typedef cont_t::iterator iter_t;

   cont_t sv;
   int a[] = {0, 1, 2, 300, 400, 500, 3, 5000};
   sv.add_segment_range(a, a + 8);

   const iter_t whole_last = test_detail::iter_at(sv, 6);
   BOOST_TEST(segmented_partition_point(sv.begin(), whole_last, less_than_threshold(100))
              == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_partition_point(sv.begin(), whole_last, less_than_threshold(1000)) == whole_last);

   const iter_t first = test_detail::iter_at(sv, 1);
   BOOST_TEST(segmented_partition_point(first, whole_last, less_than_threshold(100))
              == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_partition_point(first, whole_last, less_than_threshold(1)) == first);
}

// Forward category, multi-segment, with the partition point in an earlier
// segment and none in the last one: the last-segment call must not discard it.
void test_partition_point_forward_point_before_last_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;

   cont_t sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 500};
   int a3[] = {600, 700, 800};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 3);

   BOOST_TEST(segmented_partition_point(sv.begin(), test_detail::iter_at(sv, 7), less_than_threshold(100))
              == test_detail::iter_at(sv, 4));
}

int main()
{
   test_partition_point_shape_matrix();
   test_partition_point_empty();
   test_partition_point_non_segmented();
   test_partition_point_sentinel_segmented();
   test_partition_point_sentinel_non_segmented();
   test_partition_point_every_position();
   test_partition_point_every_position_seg2();
   test_partition_point_single_segment_interior();
   test_partition_point_single_segment_sentinel();
   test_partition_point_single_segment_seg2_inner_multi();
   test_partition_point_single_segment_seg2_single_inner();
   test_partition_point_single_segment_forward();
   test_partition_point_forward_point_before_last_segment();
   return boost::report_errors();
}
