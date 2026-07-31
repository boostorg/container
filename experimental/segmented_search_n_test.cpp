//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_search_n.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_search_n_found()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 2};
   int a2[] = {2, 3};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int>::iterator it =
      segmented_search_n(sv.begin(), sv.end(), 3, 2);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_n_not_found()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 2};
   int a2[] = {3, 2};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int>::iterator it =
      segmented_search_n(sv.begin(), sv.end(), 3, 2);
   BOOST_TEST(it == sv.end());
}

void test_search_n_zero_count()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_search_n(sv.begin(), sv.end(), 0, 99);
   BOOST_TEST(it == sv.begin());
}

void test_search_n_non_segmented()
{
   int src[] = {1, 2, 2, 2, 3};
   boost::container::vector<int> v(src, src + 5);
   boost::container::vector<int>::iterator it = segmented_search_n(v.begin(), v.end(), 3, 2);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 2);
   BOOST_TEST_EQ(static_cast<std::size_t>(it - v.begin()), 1u);
}

void test_search_n_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 2};
   int a2[] = {2, 3};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   test_detail::seg_vector<int>::iterator it =
      segmented_search_n(sv.begin(), test_detail::make_sentinel(sv.end()), 3, 2);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_n_sentinel_non_segmented()
{
   int src[] = {1, 2, 2, 2, 3};
   boost::container::vector<int> v(src, src + 5);
   boost::container::vector<int>::iterator it =
      segmented_search_n(v.begin(), test_detail::make_sentinel(v.end()), 3, 2);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_n_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 2};
   int a2[] = {2, 2};
   int a3[] = {3, 4};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 2);

   test_detail::seg2_vector<int>::iterator it =
      segmented_search_n(sv2.begin(), sv2.end(), 4, 2);
   BOOST_TEST(it != sv2.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_search_n_every_position()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   int a3[] = {60, 70, 80, 90};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   int vals[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
   const int N = 9;
   typedef test_detail::seg_vector<int>::iterator iter_t;

   iter_t expected = sv.begin();
   for(int i = 0; i < N; ++i, ++expected) {
      iter_t it = segmented_search_n(sv.begin(), sv.end(), 1, vals[i]);
      BOOST_TEST(it != sv.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }

   BOOST_TEST(segmented_search_n(sv.begin(), sv.end(), 1, 999) == sv.end());
}

void test_search_n_every_position_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   int a3[] = {60, 70, 80, 90};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int vals[] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
   const int N = 9;
   typedef test_detail::seg2_vector<int>::iterator iter_t;

   iter_t expected = sv2.begin();
   for(int i = 0; i < N; ++i, ++expected) {
      iter_t it = segmented_search_n(sv2.begin(), sv2.end(), 1, vals[i]);
      BOOST_TEST(it != sv2.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }

   BOOST_TEST(segmented_search_n(sv2.begin(), sv2.end(), 1, 999) == sv2.end());
}

// Runs segmented_search_n over a range whose segmentation shape is dictated by
// a branch spec, so that every level of the recursive dispatch is exercised on
// its single-segment, its multi-segment and its empty-segment path.
//
// search_n is the algorithm in this group that threads state across segment
// boundaries: a partial run at the end of one segment has to be carried into
// the next one, extended, and abandoned if the next segment breaks it. The
// off-by-one fixed in search_n_scan_segment was exactly such a carry error,
// which is why the run positions, the run lengths and the counts are all swept
// here rather than sampled, and why the empty-segment shapes matter: an empty
// segment in the middle of a run must neither break the carry nor consume it.
struct search_n_shape_check
{
   int count;
   int value;

   search_n_shape_check(int cnt, int v) : count(cnt), value(v) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive scan for the first run of "count" consecutive
      // elements over a flattened copy of the logical range. The guard past
      // the end is deliberately not part of it, so a scan that overruns
      // reports a run the reference does not have.
      const boost::container::vector<int> flat = test_detail::flatten_n_ints(c, n);
      std::size_t expected = flat.size();
      if(count <= 0) {
         expected = 0;    // a non-positive count matches at once, per std::search_n
      }
      else {
         const std::size_t need = std::size_t(count);
         std::size_t run = 0;
         for(std::size_t i = 0; i != flat.size(); ++i) {
            run = (flat[i] == value) ? run + 1u : 0u;
            if(run == need) { expected = i + 1u - need; break; }
         }
      }

      const iter_t r = segmented_search_n(first, last, count, value);
      BOOST_TEST(r == test_detail::iter_at(c, expected));
      BOOST_TEST(spec != 0);
   }
};

// Bidirectional and forward iterators instantiate every dispatch template
// separately, so each shape is driven through both.
void run_search_n_shapes(const int* vals, std::size_t n, int count, int value)
{
   test_detail::for_each_shape_all<int>(vals, n, -999, search_n_shape_check(count, value));
   test_detail::for_each_shape_all_fwd<int>(vals, n, -999, search_n_shape_check(count, value));
}

// Every count from the two dispatch special cases (0, which returns "first"
// without looking at anything, and 1, which routes through segmented_find
// instead of the run scanner) up to one past the range length, which no run
// can ever satisfy.
void run_search_n_counts(const int* vals, std::size_t n)
{
   for(int count = 0; count <= int(n) + 1; ++count)
      run_search_n_shapes(vals, n, count, 7);
}

void test_search_n_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 3u, 5u, 8u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      std::size_t i = 0;
      int vals[16] = { 0 };

      // A single run of every length at every start position, against a
      // background of distinct non-matching values. Since a multi-segment
      // shape splits the range at its midpoint, this sweep places runs wholly
      // inside the first segment, wholly inside the last one, and straddling
      // the boundary between them by every possible overlap.
      for(std::size_t p = 0; p != n; ++p) {
         for(std::size_t len = 1u; p + len <= n; ++len) {
            for(i = 0; i != n; ++i)
               vals[i] = (i >= p && i < p + len) ? 7 : 100 + int(i);
            run_search_n_counts(vals, n);
         }
      }

      // Two runs separated by a single non-matching element, at every
      // position: the carry from the first run has to be abandoned at the
      // hole and started afresh, and whichever run first reaches the count
      // is the answer.
      for(std::size_t g = 0; g != n; ++g) {
         for(i = 0; i != n; ++i)
            vals[i] = (i == g) ? 100 : 7;
         run_search_n_counts(vals, n);
      }

      // Alternating values: every run is one element long, so only counts of
      // 0 and 1 can match however the range is segmented.
      for(i = 0; i != n; ++i)
         vals[i] = (i % 2u == 0u) ? 7 : 100;
      run_search_n_counts(vals, n);

      // No element matches at all.
      for(i = 0; i != n; ++i)
         vals[i] = 100 + int(i);
      run_search_n_counts(vals, n);

      // Searching for the out-of-range guard value: it sits just past the end
      // and must never be reached, so no count can ever match it.
      for(i = 0; i != n; ++i)
         vals[i] = 7;
      run_search_n_shapes(vals, n, 1, -999);
      run_search_n_shapes(vals, n, 2, -999);
   }
}

//----------------------------------------------------------------------------
// Single-segment cases with data before the start of the range. The shape
// matrix always starts its range at the container's first element, so the
// lower bound is only exercised here. Every range below ends right after a run
// of the searched value and is followed by more of it, so a scan that overruns
// its end reports a longer run than it should.
//----------------------------------------------------------------------------

void test_search_n_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {2, 1, 2, 2, 2, 2};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST(segmented_search_n(first, last, 3, 2) == test_detail::iter_at(sv, 2));
   BOOST_TEST(segmented_search_n(first, last, 1, 2) == test_detail::iter_at(sv, 2));
   BOOST_TEST(segmented_search_n(first, last, 1, 1) == first);
   BOOST_TEST(segmented_search_n(first, last, 4, 2) == last);
   BOOST_TEST(segmented_search_n(first, last, 1, 99) == last);
   BOOST_TEST(segmented_search_n(first, last, 7, 2) == last);
   BOOST_TEST(segmented_search_n(first, last, 0, 2) == first);
}

void test_search_n_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {2, 1, 2, 2, 2, 2};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST(segmented_search_n(first, test_detail::make_sentinel(last), 3, 2)
              == test_detail::iter_at(sv, 2));
   BOOST_TEST(segmented_search_n(first, test_detail::make_sentinel(last), 4, 2) == last);
   BOOST_TEST(segmented_search_n(first, test_detail::make_sentinel(last), 0, 2) == first);
}

void test_search_n_single_segment_seg2_inner_multi()
{
   test_detail::seg_vector<int> inner;
   int a1[] = {2, 1, 2};
   int a2[] = {2, 2, 2, 2};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 4);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 6);

   BOOST_TEST(segmented_search_n(first, last, 4, 2) == test_detail::iter_at(sv2, 2));
   BOOST_TEST(segmented_search_n(first, last, 1, 1) == first);
   BOOST_TEST(segmented_search_n(first, last, 5, 2) == last);
   BOOST_TEST(segmented_search_n(first, last, 1, 99) == last);
   BOOST_TEST(segmented_search_n(first, last, 0, 2) == first);
}

void test_search_n_single_segment_seg2_single_inner()
{
   test_detail::seg_vector<int> inner;
   int a[] = {2, 1, 2, 2, 2, 2, 2};
   inner.add_segment_range(a, a + 7);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 5);

   BOOST_TEST(segmented_search_n(first, last, 3, 2) == test_detail::iter_at(sv2, 2));
   BOOST_TEST(segmented_search_n(first, last, 1, 1) == first);
   BOOST_TEST(segmented_search_n(first, last, 4, 2) == last);
   BOOST_TEST(segmented_search_n(first, last, 8, 2) == last);
   BOOST_TEST(segmented_search_n(first, last, 0, 2) == first);
}

// The whole suite instantiates the bidirectional category only; the forward
// one is a different instantiation of every dispatch template.
void test_search_n_single_segment_forward()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;
   typedef cont_t::iterator iter_t;

   cont_t sv;
   int a[] = {1, 3, 4, 2, 2, 2, 2};
   sv.add_segment_range(a, a + 7);

   const iter_t whole_last = test_detail::iter_at(sv, 6);
   BOOST_TEST(segmented_search_n(sv.begin(), whole_last, 3, 2) == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_search_n(sv.begin(), whole_last, 4, 2) == whole_last);
   BOOST_TEST(segmented_search_n(sv.begin(), whole_last, 0, 2) == sv.begin());

   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);
   BOOST_TEST(segmented_search_n(first, last, 2, 2) == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_search_n(first, last, 3, 2) == last);
   BOOST_TEST(segmented_search_n(first, last, 1, 1) == last);
   BOOST_TEST(segmented_search_n(first, last, 0, 2) == first);
}

// Forward category, multi-segment, with the only complete run finishing before
// the last segment: the last-segment scan must not discard it.
void test_search_n_forward_run_before_last_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;

   cont_t sv;
   int a1[] = {1, 2, 2};
   int a2[] = {2, 3};
   int a3[] = {2, 4, 2};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 3);

   const cont_t::iterator last = test_detail::iter_at(sv, 7);
   BOOST_TEST(segmented_search_n(sv.begin(), last, 3, 2) == test_detail::iter_at(sv, 1));
   BOOST_TEST(segmented_search_n(sv.begin(), last, 4, 2) == last);
}

//////////////////////////////////////////////////////////////////////////////
// Comparison count.
//
// [alg.search] mandates "At most last - first comparisons" for search_n --
// LWG 714 lowered the original N * count bound, so the run scanner is not
// allowed to re-compare the elements of an abandoned partial run.  There is
// no predicate overload, so the count comes from the value type.
//////////////////////////////////////////////////////////////////////////////

struct search_n_comparison_check
{
   int count;
   int value;

   search_n_comparison_check(int cnt, int v) : count(cnt), value(v) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      test_detail::counted_int_ops().reset();
      segmented_search_n(c.begin(), test_detail::iter_at(c, n), count,
                         test_detail::counted_int(value));
      const std::size_t applied = test_detail::counted_int_ops().cmp;

      BOOST_TEST(applied <= n);
      BOOST_TEST(spec != 0);
   }
};

void run_search_n_count_shapes(const int* vals, std::size_t n, int count, int value)
{
   test_detail::for_each_shape_all<test_detail::counted_int>
      (vals, n, -999, search_n_comparison_check(count, value));
   test_detail::for_each_shape_all_fwd<test_detail::counted_int>
      (vals, n, -999, search_n_comparison_check(count, value));
}

void test_search_n_comparison_count()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 3u, 5u, 8u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      std::size_t i = 0;
      int vals[16] = { 0 };

      for(int count = 0; count <= int(n) + 1; ++count) {
         // A single run of every length at every start position.
         for(std::size_t p = 0; p != n; ++p) {
            for(std::size_t len = 1u; p + len <= n; ++len) {
               for(i = 0; i != n; ++i)
                  vals[i] = (i >= p && i < p + len) ? 7 : 100 + int(i);
               run_search_n_count_shapes(vals, n, count, 7);
            }
         }

         // Two runs separated by a single hole, so a partial run has to be
         // abandoned and restarted without looking at anything twice.
         for(std::size_t g = 0; g != n; ++g) {
            for(i = 0; i != n; ++i)
               vals[i] = (i == g) ? 100 : 7;
            run_search_n_count_shapes(vals, n, count, 7);
         }

         for(i = 0; i != n; ++i)
            vals[i] = (i % 2u == 0u) ? 7 : 100;
         run_search_n_count_shapes(vals, n, count, 7);

         for(i = 0; i != n; ++i)
            vals[i] = 100 + int(i);
         run_search_n_count_shapes(vals, n, count, 7);
      }
   }
}

int main()
{
   test_search_n_shape_matrix();
   test_search_n_found();
   test_search_n_not_found();
   test_search_n_zero_count();
   test_search_n_non_segmented();
   test_search_n_sentinel_segmented();
   test_search_n_sentinel_non_segmented();
   test_search_n_seg2();
   test_search_n_every_position();
   test_search_n_every_position_seg2();
   test_search_n_single_segment_interior();
   test_search_n_single_segment_sentinel();
   test_search_n_single_segment_seg2_inner_multi();
   test_search_n_single_segment_seg2_single_inner();
   test_search_n_single_segment_forward();
   test_search_n_forward_run_before_last_segment();
   test_search_n_comparison_count();
   return boost::report_errors();
}
