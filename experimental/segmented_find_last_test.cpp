//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_find_last.hpp>
#include <boost/container/experimental/segmented_find.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_find_last_present_last_segment()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 2, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it = segmented_find_last(sv.begin(), sv.end(), 2);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
   test_detail::seg_vector<int>::iterator first_it = segmented_find(sv.begin(), sv.end(), 2);
   BOOST_TEST(it != first_it);
}

void test_find_last_present_first_segment()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it = segmented_find_last(sv.begin(), sv.end(), 2);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_find_last_empty()
{
   test_detail::seg_vector<int> sv;
   test_detail::seg_vector<int>::iterator it = segmented_find_last(sv.begin(), sv.end(), 1);
   BOOST_TEST(it == sv.end());
}

void test_find_last_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(10);
   v.push_back(20);
   v.push_back(10);
   v.push_back(30);

   boost::container::vector<int>::iterator it = segmented_find_last(v.begin(), v.end(), 10);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 10);
   BOOST_TEST(it == v.begin() + 2);

   it = segmented_find_last(v.begin(), v.end(), 99);
   BOOST_TEST(it == v.end());
}

void test_find_last_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 2, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_find_last(sv.begin(), test_detail::make_sentinel(sv.end()), 2);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);

   it = segmented_find_last(sv.begin(), test_detail::make_sentinel(sv.end()), 99);
   BOOST_TEST(it == sv.end());
}

void test_find_last_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(10);
   v.push_back(20);
   v.push_back(10);

   boost::container::vector<int>::iterator it =
      segmented_find_last(v.begin(), test_detail::make_sentinel(v.end()), 10);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 10);
   BOOST_TEST(it == v.begin() + 2);

   it = segmented_find_last(v.begin(), test_detail::make_sentinel(v.end()), 99);
   BOOST_TEST(it == v.end());
}

void test_find_last_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 2};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   test_detail::seg2_vector<int>::iterator it = segmented_find_last(sv2.begin(), sv2.end(), 2);
   BOOST_TEST(it != sv2.end());
   BOOST_TEST_EQ(*it, 2);

   it = segmented_find_last(sv2.begin(), sv2.end(), 99);
   BOOST_TEST(it == sv2.end());
}

void test_find_last_every_position()
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
      iter_t it = segmented_find_last(sv.begin(), sv.end(), vals[i]);
      BOOST_TEST(it != sv.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }
   BOOST_TEST(segmented_find_last(sv.begin(), sv.end(), 999) == sv.end());
}

void test_find_last_every_position_seg2()
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
      iter_t it = segmented_find_last(sv2.begin(), sv2.end(), vals[i]);
      BOOST_TEST(it != sv2.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }
   BOOST_TEST(segmented_find_last(sv2.begin(), sv2.end(), 999) == sv2.end());
}

// Runs segmented_find_last over a sub-range whose segmentation shape is
// dictated by a branch spec, so that every level of the recursive dispatch is
// exercised on its single-segment path, on its multi-segment path and on the
// multi-segment path with empty segments interleaved.
struct find_last_shape_check
{
   int value;

   explicit find_last_shape_check(int v) : value(v) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive backward scan over a flattened copy of the range.
      const boost::container::vector<int> flat = test_detail::flatten_ints(first, last);
      std::size_t expected = flat.size();
      for(std::size_t i = flat.size(); i != 0u; --i) {
         if(flat[i - 1u] == value) { expected = i - 1u; break; }
      }

      const iter_t r = segmented_find_last(first, last, value);
      BOOST_TEST(r == test_detail::iter_at(c, expected));
      BOOST_TEST(spec != 0);
   }
};

void test_find_last_shape_matrix()
{
   //Every value appears twice, so the last match and the first match differ
   //and a result carried across a segment boundary is observable.
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i/2 + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      // Every value in the range, values absent from it, and the out-of-range
      // filler, which must never be found.
      for(std::size_t v = 0; v <= n + 1u; ++v) {
         const int target = (v <= n) ? int(v) : -999;
         test_detail::for_each_shape_all<int>(vals, n, -999, find_last_shape_check(target));
         //Forward iterators reach a separate segmented implementation.
         test_detail::for_each_shape_all_fwd<int>(vals, n, -999, find_last_shape_check(target));
      }
   }
}

void test_find_last_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {9, 1, 2, 3, 2, 5, 9};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   BOOST_TEST(segmented_find_last(first, test_detail::make_sentinel(last), 2)
              == test_detail::iter_at(sv, 4));
   BOOST_TEST(segmented_find_last(first, test_detail::make_sentinel(last), 9) == last);
}

// The shape matrix does cover "a match exists in an earlier segment and must
// survive a miss in the last one", but only implicitly, as one point of a
// value sweep.  This case is the one proven to catch the result-carrying bug,
// so it stays spelled out.
void test_find_last_forward_match_in_earlier_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> fwd_seg_t;
   typedef fwd_seg_t::iterator iter_t;

   fwd_seg_t sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   //The last segment reached is the empty sentinel one and holds no match.
   BOOST_TEST(segmented_find_last(sv.begin(), sv.end(), 2) == test_detail::iter_at(sv, 1));

   //The last segment reached is a real, non-empty segment that holds no match.
   const iter_t last = test_detail::iter_at(sv, 5);
   BOOST_TEST(segmented_find_last(sv.begin(), last, 2) == test_detail::iter_at(sv, 1));
   BOOST_TEST(segmented_find_last(sv.begin(), last, 6) == last);
}

void test_find_last_forward_match_in_earlier_segment_seg2()
{
   typedef test_detail::seg2_vector<int, std::forward_iterator_tag> fwd_seg2_t;
   typedef fwd_seg2_t::iterator iter_t;

   fwd_seg2_t sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);

   BOOST_TEST(segmented_find_last(sv2.begin(), sv2.end(), 2) == test_detail::iter_at(sv2, 1));

   const iter_t last = test_detail::iter_at(sv2, 5);
   BOOST_TEST(segmented_find_last(sv2.begin(), last, 2) == test_detail::iter_at(sv2, 1));
   BOOST_TEST(segmented_find_last(sv2.begin(), last, 6) == last);
}

//////////////////////////////////////////////////////////////////////////////
// Comparison count.
//
// [alg.find.last] mandates "At most last - first applications of the
// corresponding predicate and projection", which holds for the backward scan
// a bidirectional range allows as much as for the forward one a forward range
// forces.  There is no predicate overload, so the count comes from the value
// type.
//////////////////////////////////////////////////////////////////////////////

struct find_last_comparison_check
{
   int value;

   explicit find_last_comparison_check(int v) : value(v) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      test_detail::counted_int_ops().reset();
      segmented_find_last(c.begin(), test_detail::iter_at(c, n), test_detail::counted_int(value));
      const std::size_t applied = test_detail::counted_int_ops().cmp;

      BOOST_TEST(applied <= n);
      BOOST_TEST(spec != 0);
   }
};

void test_find_last_comparison_count()
{
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i/2 + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      for(std::size_t v = 0; v <= n + 1u; ++v) {
         const int target = (v <= n) ? int(v) : -999;
         test_detail::for_each_shape_all<test_detail::counted_int>
            (vals, n, -999, find_last_comparison_check(target));
         test_detail::for_each_shape_all_fwd<test_detail::counted_int>
            (vals, n, -999, find_last_comparison_check(target));
      }
   }
}

int main()
{
   test_find_last_shape_matrix();
   test_find_last_present_last_segment();
   test_find_last_present_first_segment();
   test_find_last_empty();
   test_find_last_non_segmented();
   test_find_last_sentinel_segmented();
   test_find_last_sentinel_non_segmented();
   test_find_last_seg2();
   test_find_last_every_position();
   test_find_last_every_position_seg2();
   test_find_last_single_segment_sentinel();
   test_find_last_forward_match_in_earlier_segment();
   test_find_last_forward_match_in_earlier_segment_seg2();
   test_find_last_comparison_count();
   return boost::report_errors();
}
