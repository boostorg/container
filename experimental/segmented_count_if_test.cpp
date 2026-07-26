//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_count_if.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct is_even
{
   bool operator()(int x) const { return x % 2 == 0; }
};

struct is_negative
{
   bool operator()(int x) const { return x < 0; }
};

// Satisfied by a controllable fraction of the elements. The x > 0 term keeps
// the negative out-of-range guard out, whatever the modulus.
struct multiple_of
{
   int m;
   multiple_of(int x) : m(x) {}
   bool operator()(int x) const { return x > 0 && (x % m) == 0; }
};

void test_count_if_basic()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   BOOST_TEST_EQ(segmented_count_if(sv.begin(), sv.end(), is_even()), 3);
}

void test_count_if_empty()
{
   test_detail::seg_vector<int> sv;
   BOOST_TEST_EQ(segmented_count_if(sv.begin(), sv.end(), is_even()), 0);
}

void test_count_if_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(2);
   v.push_back(1);
   v.push_back(3);
   v.push_back(1);

   BOOST_TEST_EQ(segmented_count_if(v.begin(), v.end(), is_even()), 1);
}

void test_count_if_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   BOOST_TEST_EQ(segmented_count_if(sv.begin(), test_detail::make_sentinel(sv.end()), is_even()), 3);
}

void test_count_if_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(2);
   v.push_back(1);
   v.push_back(3);
   v.push_back(1);

   BOOST_TEST_EQ(segmented_count_if(v.begin(), test_detail::make_sentinel(v.end()), is_even()), 1);
}

void test_count_if_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2};
   int a2[] = {3, 4};
   int a3[] = {5, 6};
   sv2.add_flat_segment_range(a1, a1 + 2);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 2);

   BOOST_TEST_EQ(segmented_count_if(sv2.begin(), sv2.end(), is_even()), 3);
}

// Runs segmented_count_if over a range whose segmentation shape is dictated by
// a branch spec, so that every level of the recursive dispatch is exercised on
// its single-segment, its multi-segment and its empty-segment path. Unlike the
// early-exit algorithms, count_if has to visit the whole range and add up the
// per-segment subtotals, so a segment counted twice or skipped shows up here.
template<class Pred>
struct count_if_shape_check
{
   Pred pred;

   explicit count_if_shape_check(Pred p) : pred(p) {}

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
         if(pred(flat[i])) ++expected;
      }

      BOOST_TEST_EQ(segmented_count_if(first, last, pred), expected);
      BOOST_TEST(spec != 0);
   }
};

// Bidirectional and forward iterators instantiate every dispatch template
// separately, so each shape is driven through both.
template<class Pred>
void run_count_if_shapes(const int* vals, std::size_t n, Pred pred)
{
   test_detail::for_each_shape_all<int>(vals, n, -999, count_if_shape_check<Pred>(pred));
   test_detail::for_each_shape_all_fwd<int>(vals, n, -999, count_if_shape_check<Pred>(pred));
}

// The four predicates below cover, for any layout, no match at all, one match,
// several matches and every element matching. is_negative is additionally an
// overrun check: nothing inside the range is negative but the guard is, so a
// scan running past the end returns one too many.
void run_count_if_layout(const int* vals, std::size_t n)
{
   run_count_if_shapes(vals, n, is_negative());
   run_count_if_shapes(vals, n, is_even());
   run_count_if_shapes(vals, n, multiple_of(1));
   run_count_if_shapes(vals, n, multiple_of(3));
}

void test_count_if_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16] = { 0 };
      std::size_t i = 0;

      // All distinct: is_even matches every second element, multiple_of(3)
      // every third and multiple_of(1) all of them.
      for(i = 0; i != n; ++i)
         vals[i] = int(i) + 1;
      run_count_if_layout(vals, n);

      // Every element matches is_even, none matches multiple_of(3).
      for(i = 0; i != n; ++i)
         vals[i] = 2;
      run_count_if_layout(vals, n);

      // No element matches is_even.
      for(i = 0; i != n; ++i)
         vals[i] = 1;
      run_count_if_layout(vals, n);

      // Exactly one even element, at every position in turn.
      for(std::size_t p = 0; p != n; ++p) {
         for(i = 0; i != n; ++i)
            vals[i] = (i == p) ? 2 : 1;
         run_count_if_layout(vals, n);
      }
   }
}

//----------------------------------------------------------------------------
// Single-segment cases with data before the start of the range. The shape
// matrix always starts its range at the container's first element, so the
// lower bound is only exercised here: every range below keeps matching
// elements just outside both of its bounds, so an off-by-one on either side
// changes the count.
//----------------------------------------------------------------------------

void test_count_if_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {2, 1, 2, 3, 2, 4};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST_EQ(segmented_count_if(first, last, is_even()), 2);
   BOOST_TEST_EQ(segmented_count_if(first, last, is_negative()), 0);
}

void test_count_if_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {2, 1, 2, 3, 2, 4};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST_EQ(segmented_count_if(first, test_detail::make_sentinel(last), is_even()), 2);
   BOOST_TEST_EQ(segmented_count_if(first, test_detail::make_sentinel(last), is_negative()), 0);
}

void test_count_if_single_segment_seg2_inner_multi()
{
   test_detail::seg_vector<int> inner;
   int a1[] = {2, 1, 2};
   int a2[] = {3, 2, 5, 2};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 4);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 6);

   BOOST_TEST_EQ(segmented_count_if(first, last, is_even()), 2);
   BOOST_TEST_EQ(segmented_count_if(first, last, is_negative()), 0);
}

void test_count_if_single_segment_seg2_single_inner()
{
   test_detail::seg_vector<int> inner;
   int a[] = {2, 1, 2, 3, 2, 5, 2};
   inner.add_segment_range(a, a + 7);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 5);

   BOOST_TEST_EQ(segmented_count_if(first, last, is_even()), 2);
   BOOST_TEST_EQ(segmented_count_if(first, last, is_negative()), 0);
}

// The whole suite instantiates the bidirectional category only; the forward
// one is a different instantiation of every dispatch template.
void test_count_if_single_segment_forward()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;
   typedef cont_t::iterator iter_t;

   cont_t sv;
   int a[] = {2, 1, 2, 3, 2, 5, 2};
   sv.add_segment_range(a, a + 7);

   const iter_t whole_last = test_detail::iter_at(sv, 6);
   BOOST_TEST_EQ(segmented_count_if(sv.begin(), whole_last, is_even()), 3);
   BOOST_TEST_EQ(segmented_count_if(sv.begin(), whole_last, is_negative()), 0);

   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);
   BOOST_TEST_EQ(segmented_count_if(first, last, is_even()), 2);
   BOOST_TEST_EQ(segmented_count_if(first, last, is_negative()), 0);
}

// Forward category, multi-segment: the per-segment counts must all be added up.
void test_count_if_forward_multi_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;

   cont_t sv;
   int a1[] = {2, 1, 2};
   int a2[] = {3, 2};
   int a3[] = {2, 5, 2};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 3);

   BOOST_TEST_EQ(segmented_count_if(sv.begin(), test_detail::iter_at(sv, 7), is_even()), 4);
   BOOST_TEST_EQ(segmented_count_if(sv.begin(), test_detail::iter_at(sv, 7), is_negative()), 0);
}

int main()
{
   test_count_if_shape_matrix();
   test_count_if_basic();
   test_count_if_empty();
   test_count_if_non_segmented();
   test_count_if_sentinel_segmented();
   test_count_if_sentinel_non_segmented();
   test_count_if_seg2();
   test_count_if_single_segment_interior();
   test_count_if_single_segment_sentinel();
   test_count_if_single_segment_seg2_inner_multi();
   test_count_if_single_segment_seg2_single_inner();
   test_count_if_single_segment_forward();
   test_count_if_forward_multi_segment();
   return boost::report_errors();
}
