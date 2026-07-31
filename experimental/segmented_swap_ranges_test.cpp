//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_swap_ranges.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_swap_ranges_full()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   boost::container::vector<int> other(9, 0);
   for(int i = 0; i < 9; ++i)
      other[static_cast<std::size_t>(i)] = (i + 1) * 10;

   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(sv.begin(), sv.end(), other.begin());

   BOOST_TEST(result == other.end());

   test_detail::seg_vector<int>::iterator it = sv.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, (i + 1) * 10);

   for(int i = 0; i < 9; ++i)
      BOOST_TEST_EQ(other[static_cast<std::size_t>(i)], i + 1);
}

void test_swap_ranges_empty()
{
   test_detail::seg_vector<int> sv;
   boost::container::vector<int> other;
   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(sv.begin(), sv.end(), other.begin());
   BOOST_TEST(result == other.begin());
}

void test_swap_ranges_single_segment()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3};
   sv.add_segment_range(a, a + 3);

   int other[] = {10, 20, 30};
   segmented_swap_ranges(sv.begin(), sv.end(), other);

   test_detail::seg_vector<int>::iterator it = sv.begin();
   BOOST_TEST_EQ(*it, 10); ++it;
   BOOST_TEST_EQ(*it, 20); ++it;
   BOOST_TEST_EQ(*it, 30);

   BOOST_TEST_EQ(other[0], 1);
   BOOST_TEST_EQ(other[1], 2);
   BOOST_TEST_EQ(other[2], 3);
}

void test_swap_ranges_non_segmented()
{
   boost::container::vector<int> v1;
   v1.push_back(1); v1.push_back(2); v1.push_back(3);
   boost::container::vector<int> v2;
   v2.push_back(10); v2.push_back(20); v2.push_back(30);

   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(v1.begin(), v1.end(), v2.begin());
   BOOST_TEST(result == v2.end());

   BOOST_TEST_EQ(v1[0], 10);
   BOOST_TEST_EQ(v1[1], 20);
   BOOST_TEST_EQ(v1[2], 30);
   BOOST_TEST_EQ(v2[0], 1);
   BOOST_TEST_EQ(v2[1], 2);
   BOOST_TEST_EQ(v2[2], 3);
}

void test_swap_ranges_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   boost::container::vector<int> other(9, 0);
   for(int i = 0; i < 9; ++i)
      other[static_cast<std::size_t>(i)] = (i + 1) * 10;

   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(sv.begin(), test_detail::make_sentinel(sv.end()), other.begin());

   BOOST_TEST(result == other.end());

   test_detail::seg_vector<int>::iterator it = sv.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, (i + 1) * 10);

   for(int i = 0; i < 9; ++i)
      BOOST_TEST_EQ(other[static_cast<std::size_t>(i)], i + 1);
}

void test_swap_ranges_sentinel_non_segmented()
{
   boost::container::vector<int> v1;
   v1.push_back(1); v1.push_back(2); v1.push_back(3);
   boost::container::vector<int> v2;
   v2.push_back(10); v2.push_back(20); v2.push_back(30);

   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(v1.begin(), test_detail::make_sentinel(v1.end()), v2.begin());
   BOOST_TEST(result == v2.end());

   BOOST_TEST_EQ(v1[0], 10);
   BOOST_TEST_EQ(v1[1], 20);
   BOOST_TEST_EQ(v1[2], 30);
   BOOST_TEST_EQ(v2[0], 1);
   BOOST_TEST_EQ(v2[1], 2);
   BOOST_TEST_EQ(v2[2], 3);
}

void test_swap_ranges_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   boost::container::vector<int> other(9, 0);
   for(int i = 0; i < 9; ++i)
      other[static_cast<std::size_t>(i)] = (i + 1) * 10;

   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(sv2.begin(), sv2.end(), other.begin());

   BOOST_TEST(result == other.end());

   test_detail::seg2_vector<int>::iterator it = sv2.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, (i + 1) * 10);

   for(int i = 0; i < 9; ++i)
      BOOST_TEST_EQ(other[static_cast<std::size_t>(i)], i + 1);
}

void test_swap_ranges_movable_seg()
{
   typedef test_detail::movable_int mi;
   test_detail::seg_vector<mi> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_from_ints(a1, a1 + 3);
   sv.add_segment_from_ints(a2, a2 + 2);
   sv.add_segment_from_ints(a3, a3 + 4);

   boost::container::vector<mi> other;
   for(int i = 0; i < 9; ++i)
      other.push_back(mi((i + 1) * 10));

   boost::container::vector<mi>::iterator result =
      segmented_swap_ranges(sv.begin(), sv.end(), other.begin());

   BOOST_TEST(result == other.end());

   test_detail::seg_vector<mi>::iterator it = sv.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(it->value(), (i + 1) * 10);

   for(int i = 0; i < 9; ++i)
      BOOST_TEST_EQ(other[static_cast<std::size_t>(i)].value(), i + 1);
}

void test_swap_ranges_movable_seg2()
{
   typedef test_detail::movable_int mi;
   test_detail::seg2_vector<mi> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_from_ints(a1, a1 + 3);
   sv2.add_flat_segment_from_ints(a2, a2 + 2);
   sv2.add_flat_segment_from_ints(a3, a3 + 4);

   boost::container::vector<mi> other;
   for(int i = 0; i < 9; ++i)
      other.push_back(mi((i + 1) * 10));

   boost::container::vector<mi>::iterator result =
      segmented_swap_ranges(sv2.begin(), sv2.end(), other.begin());

   BOOST_TEST(result == other.end());

   test_detail::seg2_vector<mi>::iterator it = sv2.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(it->value(), (i + 1) * 10);

   for(int i = 0; i < 9; ++i)
      BOOST_TEST_EQ(other[static_cast<std::size_t>(i)].value(), i + 1);
}

void test_swap_ranges_segmented_second()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 4);

   test_detail::seg_vector<int> other;
   int b1[] = {10, 20, 30, 40};
   int b2[] = {50, 60, 70};
   int b3[] = {80, 90};
   other.add_segment_range(b1, b1 + 4);
   other.add_segment_range(b2, b2 + 3);
   other.add_segment_range(b3, b3 + 2);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_swap_ranges(sv.begin(), sv.end(), other.begin());

   BOOST_TEST(result == other.end());

   iter_t it = sv.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, (i + 1) * 10);

   it = other.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, i + 1);
}

void test_swap_ranges_seg2_to_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   test_detail::seg2_vector<int> other;
   int b1[] = {10, 20, 30, 40, 50};
   int b2[] = {60, 70, 80, 90};
   other.add_flat_segment_range(b1, b1 + 5);
   other.add_flat_segment_range(b2, b2 + 4);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t result = segmented_swap_ranges(sv2.begin(), sv2.end(), other.begin());

   BOOST_TEST(result == other.end());

   iter_t it = sv2.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, (i + 1) * 10);

   it = other.begin();
   for(int i = 0; i < 9; ++i, ++it)
      BOOST_TEST_EQ(*it, i + 1);
}

void test_swap_ranges_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   int b[] = {10, 20, 30, 40, 50, 60};
   boost::container::vector<int> other(b, b + 6);

   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), other.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - other.begin()), 4u);

   int expected_sv[] = {1, 10, 20, 30, 40, 6};
   test_detail::seg_vector<int>::iterator it = sv.begin();
   for(int i = 0; i < 6; ++i, ++it)
      BOOST_TEST_EQ(*it, expected_sv[i]);

   int expected_other[] = {2, 3, 4, 5, 50, 60};
   for(std::size_t i = 0; i < 6; ++i)
      BOOST_TEST_EQ(other[i], expected_other[i]);
}

void test_swap_ranges_single_segment_empty_mid()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   int b[] = {10, 20, 30};
   boost::container::vector<int> other(b, b + 3);

   test_detail::seg_vector<int>::iterator mid = test_detail::iter_at(sv, 3);
   boost::container::vector<int>::iterator result = segmented_swap_ranges(mid, mid, other.begin());

   BOOST_TEST(result == other.begin());
   test_detail::seg_vector<int>::iterator it = sv.begin();
   for(int i = 0; i < 6; ++i, ++it)
      BOOST_TEST_EQ(*it, a[i]);
   for(std::size_t i = 0; i < 3; ++i)
      BOOST_TEST_EQ(other[i], b[i]);
}

void test_swap_ranges_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   int b[] = {10, 20, 30, 40, 50, 60};
   boost::container::vector<int> other(b, b + 6);

   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(test_detail::iter_at(sv, 1),
                            test_detail::make_sentinel(test_detail::iter_at(sv, 5)), other.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - other.begin()), 4u);

   int expected_sv[] = {1, 10, 20, 30, 40, 6};
   test_detail::seg_vector<int>::iterator it = sv.begin();
   for(int i = 0; i < 6; ++i, ++it)
      BOOST_TEST_EQ(*it, expected_sv[i]);

   int expected_other[] = {2, 3, 4, 5, 50, 60};
   for(std::size_t i = 0; i < 6; ++i)
      BOOST_TEST_EQ(other[i], expected_other[i]);
}

void test_swap_ranges_single_segment_seg2_outer()
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

   int b[] = {10, 20, 30, 40, 50, 60, 70};
   boost::container::vector<int> other(b, b + 7);

   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), other.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - other.begin()), 5u);

   int expected_sv[] = {1, 10, 20, 30, 40, 50, 7};
   test_detail::seg2_vector<int>::iterator it = sv2.begin();
   for(int i = 0; i < 7; ++i, ++it)
      BOOST_TEST_EQ(*it, expected_sv[i]);

   int expected_other[] = {2, 3, 4, 5, 6, 60, 70};
   for(std::size_t i = 0; i < 7; ++i)
      BOOST_TEST_EQ(other[i], expected_other[i]);
}

void test_swap_ranges_single_segment_seg2_both()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a[] = {1, 2, 3, 4, 5, 6};
   inner.add_segment_range(a, a + 6);
   sv2.add_segment(inner);

   int b[] = {10, 20, 30, 40, 50, 60};
   boost::container::vector<int> other(b, b + 6);

   boost::container::vector<int>::iterator result =
      segmented_swap_ranges(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 5), other.begin());

   BOOST_TEST_EQ(static_cast<std::size_t>(result - other.begin()), 4u);

   int expected_sv[] = {1, 10, 20, 30, 40, 6};
   test_detail::seg2_vector<int>::iterator it = sv2.begin();
   for(int i = 0; i < 6; ++i, ++it)
      BOOST_TEST_EQ(*it, expected_sv[i]);

   int expected_other[] = {2, 3, 4, 5, 50, 60};
   for(std::size_t i = 0; i < 6; ++i)
      BOOST_TEST_EQ(other[i], expected_other[i]);
}

void test_swap_ranges_single_segment_first_multi_second()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6, 7};
   sv.add_segment_range(a, a + 7);

   test_detail::seg_vector<int> other;
   int b1[] = {10, 20};
   int b2[] = {30, 40, 50};
   int b3[] = {60, 70, 80};
   other.add_segment_range(b1, b1 + 2);
   other.add_segment_range(b2, b2 + 3);
   other.add_segment_range(b3, b3 + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_swap_ranges(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 6), other.begin());

   BOOST_TEST(result == test_detail::iter_at(other, 5));

   int expected_sv[] = {1, 10, 20, 30, 40, 50, 7};
   iter_t it = sv.begin();
   for(int i = 0; i < 7; ++i, ++it)
      BOOST_TEST_EQ(*it, expected_sv[i]);

   int expected_other[] = {2, 3, 4, 5, 6, 60, 70, 80};
   it = other.begin();
   for(int i = 0; i < 8; ++i, ++it)
      BOOST_TEST_EQ(*it, expected_other[i]);
}

void test_swap_ranges_single_segment_first_and_second()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   test_detail::seg_vector<int> other;
   int b[] = {10, 20, 30, 40, 50, 60};
   other.add_segment_range(b, b + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_swap_ranges(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), other.begin());

   BOOST_TEST(result == test_detail::iter_at(other, 4));

   int expected_sv[] = {1, 10, 20, 30, 40, 6};
   iter_t it = sv.begin();
   for(int i = 0; i < 6; ++i, ++it)
      BOOST_TEST_EQ(*it, expected_sv[i]);

   int expected_other[] = {2, 3, 4, 5, 50, 60};
   it = other.begin();
   for(int i = 0; i < 6; ++i, ++it)
      BOOST_TEST_EQ(*it, expected_other[i]);
}

void test_swap_ranges_single_segment_second_from_flat()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3); v.push_back(4);

   test_detail::seg_vector<int> other;
   int b[] = {10, 20, 30, 40, 50, 60};
   other.add_segment_range(b, b + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t result = segmented_swap_ranges(v.begin(), v.end(), other.begin());

   BOOST_TEST(result == test_detail::iter_at(other, 4));

   for(std::size_t i = 0; i < 4; ++i)
      BOOST_TEST_EQ(v[i], static_cast<int>(i + 1) * 10);

   iter_t it = other.begin();
   for(int i = 0; i < 4; ++i, ++it)
      BOOST_TEST_EQ(*it, i + 1);
   BOOST_TEST_EQ(*it, 50); ++it;
   BOOST_TEST_EQ(*it, 60);
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// for_each_shape2_all walks the twelve branch specs of the first range
// against the twelve of the second, so the two segmentations vary
// independently: 144 pairs per size pair, over both segmentation depths.  The
// 'e' specs carry empty segments, which is the only way to reach the
// sfirst == slast branch of the second range's bounded helpers.
//
// segmented_swap_ranges writes into both ranges, so both are checked after
// every call: the first range must hold the second range's prefix, the second
// range must hold the first range's contents followed by its own untouched
// tail, and neither guard may be disturbed.  A stale range shared across the
// inner loop would show up here as the previous combination's leftovers.
//
// The two ranges hold disjoint value sets so that a swap that moved the wrong
// way, or did not happen at all, cannot look correct.
//////////////////////////////////////////////////////////////////////////////

const int shape_filler = -999;   // guard just past the end of every range

const int shape_first_vals[]  = {5, 3, 9, 1, 7, 2, 8, 4, 6, 11};
const int shape_second_vals[] = {50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160};

struct swap_ranges_shape_check
{
   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      typedef typename C2::iterator iter2_t;

      const boost::container::vector<int> ref1 = test_detail::flatten_n_ints(c1, n1);
      const boost::container::vector<int> ref2 = test_detail::flatten_n_ints(c2, n2);

      const iter2_t r = segmented_swap_ranges
         (c1.begin(), test_detail::iter_at(c1, n1), c2.begin());

      BOOST_TEST(r == test_detail::iter_at(c2, n1));
      BOOST_TEST(test_detail::filler_intact(c1, n1, shape_filler));
      BOOST_TEST(test_detail::filler_intact(c2, n2, shape_filler));

      const boost::container::vector<int> got1 = test_detail::flatten_n_ints(c1, n1);
      const boost::container::vector<int> got2 = test_detail::flatten_n_ints(c2, n2);
      BOOST_TEST_EQ(got1.size(), n1);
      BOOST_TEST_EQ(got2.size(), n2);

      for(std::size_t i = 0; i != n1; ++i)
         BOOST_TEST_EQ(got1[i], ref2[i]);
      for(std::size_t i = 0; i != n2; ++i)
         BOOST_TEST_EQ(got2[i], i < n1 ? ref1[i] : ref2[i]);

      BOOST_TEST(s1 != 0 && s2 != 0);
   }
};

void test_swap_ranges_shape_matrix()
{
   static const std::size_t sizes[][2] =
      { {0, 0}, {1, 1}, {1, 3}, {2, 2}, {3, 3}, {3, 6}, {5, 5}, {6, 10} };

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i)
      test_detail::for_each_shape2_all<int, int>
         (shape_first_vals, sizes[i][0], shape_second_vals, sizes[i][1],
          shape_filler, swap_ranges_shape_check());
}

//////////////////////////////////////////////////////////////////////////////
// Swap count.
//
// [alg.swap] mandates "Exactly last1 - first1 swaps", so a pair swapped twice
// at a segment boundary is a conformance failure -- and, unlike a re-read, it
// would also put the values back where they started.
//////////////////////////////////////////////////////////////////////////////

struct swap_ranges_count_check
{
   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      test_detail::counted_int_ops().reset();
      segmented_swap_ranges(c1.begin(), test_detail::iter_at(c1, n1), c2.begin());
      const std::size_t applied = test_detail::counted_int_ops().swp;

      BOOST_TEST_EQ(applied, n1);
      BOOST_TEST(s1 != 0 && s2 != 0 && n2 >= n1);
   }
};

void test_swap_ranges_swap_count()
{
   static const std::size_t sizes[][2] =
      { {0, 0}, {1, 1}, {1, 3}, {2, 2}, {3, 3}, {3, 6}, {5, 5}, {6, 10} };

   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i)
      test_detail::for_each_shape2_all<test_detail::counted_int, test_detail::counted_int>
         (shape_first_vals, sizes[i][0], shape_second_vals, sizes[i][1],
          shape_filler, swap_ranges_count_check());
}

int main()
{
   test_swap_ranges_full();
   test_swap_ranges_empty();
   test_swap_ranges_single_segment();
   test_swap_ranges_single_segment_interior();
   test_swap_ranges_single_segment_empty_mid();
   test_swap_ranges_single_segment_sentinel();
   test_swap_ranges_single_segment_seg2_outer();
   test_swap_ranges_single_segment_seg2_both();
   test_swap_ranges_non_segmented();
   test_swap_ranges_sentinel_segmented();
   test_swap_ranges_sentinel_non_segmented();
   test_swap_ranges_seg2();
   test_swap_ranges_movable_seg();
   test_swap_ranges_movable_seg2();
   test_swap_ranges_segmented_second();
   test_swap_ranges_seg2_to_seg2();
   test_swap_ranges_single_segment_first_multi_second();
   test_swap_ranges_single_segment_first_and_second();
   test_swap_ranges_single_segment_second_from_flat();

   test_swap_ranges_shape_matrix();
   test_swap_ranges_swap_count();
   return boost::report_errors();
}
