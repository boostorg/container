//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_is_sorted.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_is_sorted_sorted_range()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 2);

   BOOST_TEST(segmented_is_sorted(sv.begin(), sv.end()));
}

void test_is_sorted_unsorted_at_boundary()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 5};
   int a2[] = {3, 6, 7};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   BOOST_TEST(!segmented_is_sorted(sv.begin(), sv.end()));
}

void test_is_sorted_empty()
{
   test_detail::seg_vector<int> sv;
   BOOST_TEST(segmented_is_sorted(sv.begin(), sv.end()));
}

struct greater_comp
{
   bool operator()(int a, int b) const { return a > b; }
};

void test_is_sorted_non_segmented()
{
   {
      int data[] = {1, 2, 3, 4, 5};
      boost::container::vector<int> v(data, data + 5);
      BOOST_TEST(segmented_is_sorted(v.begin(), v.end()));
   }
   {
      int data[] = {1, 3, 2, 4, 5};
      boost::container::vector<int> v(data, data + 5);
      BOOST_TEST(!segmented_is_sorted(v.begin(), v.end()));
   }
}

void test_is_sorted_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);
   sv.add_segment_range(a3, a3 + 2);

   BOOST_TEST(segmented_is_sorted(sv.begin(), test_detail::make_sentinel(sv.end())));
}

void test_is_sorted_sentinel_non_segmented()
{
   {
      int data[] = {1, 2, 3, 4, 5};
      boost::container::vector<int> v(data, data + 5);
      BOOST_TEST(segmented_is_sorted(v.begin(), test_detail::make_sentinel(v.end())));
   }
   {
      int data[] = {1, 3, 2, 4, 5};
      boost::container::vector<int> v(data, data + 5);
      BOOST_TEST(!segmented_is_sorted(v.begin(), test_detail::make_sentinel(v.end())));
   }
}

void test_is_sorted_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);
   sv2.add_flat_segment_range(a3, a3 + 2);

   BOOST_TEST(segmented_is_sorted(sv2.begin(), sv2.end()));
}

// Runs segmented_is_sorted over a sub-range whose segmentation shape is
// dictated by a branch spec, so that every level of the recursive dispatch is
// exercised on its single-segment path, on its multi-segment path and on the
// multi-segment path with empty segments interleaved.  Empty segments matter
// here because the comparison of the last element of one segment against the
// first of the next has to survive however many empty segments lie between.
struct is_sorted_shape_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive adjacent scan over a flattened copy of the range.
      const boost::container::vector<int> flat = test_detail::flatten_ints(first, last);
      bool expected = true;
      for(std::size_t i = 1u; i < flat.size(); ++i) {
         if(flat[i] < flat[i - 1u]) { expected = false; break; }
      }

      BOOST_TEST(segmented_is_sorted(first, last) == expected);
      BOOST_TEST(spec != 0);
   }
};

//The same for the explicit-comparator overload, which is a separate template.
struct is_sorted_greater_shape_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      const boost::container::vector<int> flat = test_detail::flatten_ints(first, last);
      bool expected = true;
      for(std::size_t i = 1u; i < flat.size(); ++i) {
         if(flat[i] > flat[i - 1u]) { expected = false; break; }
      }

      BOOST_TEST(segmented_is_sorted(first, last, greater_comp()) == expected);
      BOOST_TEST(spec != 0);
   }
};

void test_is_sorted_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];

      //Ascending.  The filler is smaller than every in-range element, so an
      //algorithm that reads it reports a sorted range as unsorted.
      for(int i = 0; i != 16; ++i)
         vals[i] = (i + 1) * 10;
      test_detail::for_each_shape_all<int>(vals, n, -999, is_sorted_shape_check());

      //One inversion, at each adjacent pair in turn: p == 1 is the first pair
      //and p == n-1 the last one.
      for(std::size_t p = 1u; p < n; ++p) {
         for(int i = 0; i != 16; ++i)
            vals[i] = (i + 1) * 10;
         vals[p] = vals[p - 1u] - 1;
         test_detail::for_each_shape_all<int>(vals, n, -999, is_sorted_shape_check());
      }

      //Descending, under the explicit comparator.  The filler is larger than
      //every in-range element for the same reason as above.
      for(int i = 0; i != 16; ++i)
         vals[i] = (16 - i) * 10;
      test_detail::for_each_shape_all<int>(vals, n, 99999, is_sorted_greater_shape_check());

      for(std::size_t p = 1u; p < n; ++p) {
         for(int i = 0; i != 16; ++i)
            vals[i] = (16 - i) * 10;
         vals[p] = vals[p - 1u] + 1;
         test_detail::for_each_shape_all<int>(vals, n, 99999, is_sorted_greater_shape_check());
      }
   }
}

void test_is_sorted_single_segment_sentinel()
{
   typedef test_detail::seg_vector<int>::iterator iter_t;

   {
      test_detail::seg_vector<int> sv;
      int a[] = {100, 10, 20, 30, 40, 50, 5};
      sv.add_segment_range(a, a + 7);
      const iter_t first = test_detail::iter_at(sv, 1);
      const iter_t last  = test_detail::iter_at(sv, 6);
      BOOST_TEST(segmented_is_sorted(first, test_detail::make_sentinel(last)));
   }
   {
      test_detail::seg_vector<int> sv;
      int a[] = {100, 10, 20, 30, 40, 35, 5};
      sv.add_segment_range(a, a + 7);
      const iter_t first = test_detail::iter_at(sv, 1);
      const iter_t last  = test_detail::iter_at(sv, 6);
      BOOST_TEST(!segmented_is_sorted(first, test_detail::make_sentinel(last)));
   }
}

int main()
{
   test_is_sorted_shape_matrix();
   test_is_sorted_sorted_range();
   test_is_sorted_unsorted_at_boundary();
   test_is_sorted_empty();
   test_is_sorted_non_segmented();
   test_is_sorted_sentinel_segmented();
   test_is_sorted_sentinel_non_segmented();
   test_is_sorted_seg2();
   test_is_sorted_single_segment_sentinel();
   return boost::report_errors();
}
