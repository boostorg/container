//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_is_sorted_until.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_is_sorted_until_empty()
{
   test_detail::seg_vector<int> sv;
   test_detail::seg_vector<int>::iterator it = segmented_is_sorted_until(sv.begin(), sv.end());
   BOOST_TEST(it == sv.end());
}

struct greater_comp
{
   bool operator()(int a, int b) const { return a > b; }
};

void test_is_sorted_until_non_segmented()
{
   int data[] = {1, 2, 5, 3, 4};
   boost::container::vector<int> v(data, data + 5);
   boost::container::vector<int>::iterator it = segmented_is_sorted_until(v.begin(), v.end());
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_is_sorted_until_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 2, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_is_sorted_until(sv.begin(), test_detail::make_sentinel(sv.end()));
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_is_sorted_until_sentinel_non_segmented()
{
   int data[] = {1, 2, 5, 3, 4};
   boost::container::vector<int> v(data, data + 5);
   boost::container::vector<int>::iterator it =
      segmented_is_sorted_until(v.begin(), test_detail::make_sentinel(v.end()));
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 3);
}

void test_is_sorted_until_every_position()
{
   const int N = 9;

   for(int p = 1; p < N; ++p) {
      int data[9];
      for(int j = 0; j < N; ++j) data[j] = j * 10;
      data[p] = data[p - 1] - 1;

      test_detail::seg_vector<int> sv;
      sv.add_segment_range(data, data + 3);
      sv.add_segment_range(data + 3, data + 5);
      sv.add_segment_range(data + 5, data + 9);

      typedef test_detail::seg_vector<int>::iterator iter_t;
      iter_t result = segmented_is_sorted_until(sv.begin(), sv.end());

      iter_t expected = sv.begin();
      for(int j = 0; j < p; ++j) ++expected;

      BOOST_TEST(result == expected);
      BOOST_TEST_EQ(*result, data[p]);
   }

   {
      int data[] = {0, 10, 20, 30, 40, 50, 60, 70, 80};
      test_detail::seg_vector<int> sv;
      sv.add_segment_range(data, data + 3);
      sv.add_segment_range(data + 3, data + 5);
      sv.add_segment_range(data + 5, data + 9);
      BOOST_TEST(segmented_is_sorted_until(sv.begin(), sv.end()) == sv.end());
   }
}

void test_is_sorted_until_every_position_seg2()
{
   const int N = 9;

   for(int p = 1; p < N; ++p) {
      int data[9];
      for(int j = 0; j < N; ++j) data[j] = j * 10;
      data[p] = data[p - 1] - 1;

      test_detail::seg2_vector<int> sv2;
      sv2.add_flat_segment_range(data, data + 3);
      sv2.add_flat_segment_range(data + 3, data + 5);
      sv2.add_flat_segment_range(data + 5, data + 9);

      typedef test_detail::seg2_vector<int>::iterator iter_t;
      iter_t result = segmented_is_sorted_until(sv2.begin(), sv2.end());

      iter_t expected = sv2.begin();
      for(int j = 0; j < p; ++j) ++expected;

      BOOST_TEST(result == expected);
      BOOST_TEST_EQ(*result, data[p]);
   }

   {
      int data[] = {0, 10, 20, 30, 40, 50, 60, 70, 80};
      test_detail::seg2_vector<int> sv2;
      sv2.add_flat_segment_range(data, data + 3);
      sv2.add_flat_segment_range(data + 3, data + 5);
      sv2.add_flat_segment_range(data + 5, data + 9);
      BOOST_TEST(segmented_is_sorted_until(sv2.begin(), sv2.end()) == sv2.end());
   }
}

// Runs segmented_is_sorted_until over a sub-range whose segmentation shape is
// dictated by a branch spec, so that every level of the recursive dispatch is
// exercised on its single-segment path, on its multi-segment path and on the
// multi-segment path with empty segments interleaved.  The algorithm carries
// the previous element across segment boundaries, so the shapes with empty
// segments are the interesting ones: the carry has to survive however many
// segments contribute nothing.
struct is_sorted_until_shape_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive adjacent scan over a flattened copy of the range.
      const boost::container::vector<int> flat = test_detail::flatten_ints(first, last);
      std::size_t expected = flat.size();
      for(std::size_t i = 1u; i < flat.size(); ++i) {
         if(flat[i] < flat[i - 1u]) { expected = i; break; }
      }

      BOOST_TEST(segmented_is_sorted_until(first, last) == test_detail::iter_at(c, expected));
      BOOST_TEST(spec != 0);
   }
};

//The same for the explicit-comparator overload, which is a separate template.
struct is_sorted_until_greater_shape_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      const boost::container::vector<int> flat = test_detail::flatten_ints(first, last);
      std::size_t expected = flat.size();
      for(std::size_t i = 1u; i < flat.size(); ++i) {
         if(flat[i] > flat[i - 1u]) { expected = i; break; }
      }

      BOOST_TEST(segmented_is_sorted_until(first, last, greater_comp())
                 == test_detail::iter_at(c, expected));
      BOOST_TEST(spec != 0);
   }
};

void test_is_sorted_until_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];

      //Ascending, i.e. sorted all the way to the end.
      for(int i = 0; i != 16; ++i)
         vals[i] = (i + 1) * 10;
      test_detail::for_each_shape_all<int>(vals, n, -999, is_sorted_until_shape_check());

      //One inversion, at each adjacent pair in turn: p == 1 is the first pair
      //and p == n-1 the last one.
      for(std::size_t p = 1u; p < n; ++p) {
         for(int i = 0; i != 16; ++i)
            vals[i] = (i + 1) * 10;
         vals[p] = vals[p - 1u] - 1;
         test_detail::for_each_shape_all<int>(vals, n, -999, is_sorted_until_shape_check());
      }

      //Descending, under the explicit comparator.
      for(int i = 0; i != 16; ++i)
         vals[i] = (16 - i) * 10;
      test_detail::for_each_shape_all<int>(vals, n, 99999, is_sorted_until_greater_shape_check());

      for(std::size_t p = 1u; p < n; ++p) {
         for(int i = 0; i != 16; ++i)
            vals[i] = (16 - i) * 10;
         vals[p] = vals[p - 1u] + 1;
         test_detail::for_each_shape_all<int>(vals, n, 99999, is_sorted_until_greater_shape_check());
      }
   }
}

void test_is_sorted_until_single_segment_sentinel()
{
   typedef test_detail::seg_vector<int>::iterator iter_t;

   {
      test_detail::seg_vector<int> sv;
      int a[] = {100, 10, 20, 30, 40, 50, 5};
      sv.add_segment_range(a, a + 7);
      const iter_t first = test_detail::iter_at(sv, 1);
      const iter_t last  = test_detail::iter_at(sv, 6);
      BOOST_TEST(segmented_is_sorted_until(first, test_detail::make_sentinel(last)) == last);
   }
   {
      test_detail::seg_vector<int> sv;
      int a[] = {100, 10, 20, 30, 40, 35, 5};
      sv.add_segment_range(a, a + 7);
      const iter_t first = test_detail::iter_at(sv, 1);
      const iter_t last  = test_detail::iter_at(sv, 6);
      BOOST_TEST(segmented_is_sorted_until(first, test_detail::make_sentinel(last))
                 == test_detail::iter_at(sv, 5));
   }
}

int main()
{
   test_is_sorted_until_shape_matrix();
   test_is_sorted_until_empty();
   test_is_sorted_until_non_segmented();
   test_is_sorted_until_sentinel_segmented();
   test_is_sorted_until_sentinel_non_segmented();
   test_is_sorted_until_every_position();
   test_is_sorted_until_every_position_seg2();
   test_is_sorted_until_single_segment_sentinel();
   return boost::report_errors();
}
