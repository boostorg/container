//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_find.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_find_present_first_segment()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it = segmented_find(sv.begin(), sv.end(), 2);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 2);
}

void test_find_present_second_segment()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it = segmented_find(sv.begin(), sv.end(), 5);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 5);
}

void test_find_not_present()
{
   test_detail::seg_vector<int> sv;
   sv.add_segment(3, 1);
   sv.add_segment(2, 2);

   test_detail::seg_vector<int>::iterator it = segmented_find(sv.begin(), sv.end(), 99);
   BOOST_TEST(it == sv.end());
}

void test_find_empty()
{
   test_detail::seg_vector<int> sv;
   test_detail::seg_vector<int>::iterator it = segmented_find(sv.begin(), sv.end(), 1);
   BOOST_TEST(it == sv.end());
}

void test_find_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(10);
   v.push_back(20);
   v.push_back(30);

   boost::container::vector<int>::iterator it = segmented_find(v.begin(), v.end(), 20);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 20);

   it = segmented_find(v.begin(), v.end(), 99);
   BOOST_TEST(it == v.end());
}

void test_find_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_find(sv.begin(), test_detail::make_sentinel(sv.end()), 5);
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, 5);

   it = segmented_find(sv.begin(), test_detail::make_sentinel(sv.end()), 99);
   BOOST_TEST(it == sv.end());
}

void test_find_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(10);
   v.push_back(20);
   v.push_back(30);

   boost::container::vector<int>::iterator it =
      segmented_find(v.begin(), test_detail::make_sentinel(v.end()), 20);
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, 20);

   it = segmented_find(v.begin(), test_detail::make_sentinel(v.end()), 99);
   BOOST_TEST(it == v.end());
}

void test_find_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   test_detail::seg2_vector<int>::iterator it = segmented_find(sv2.begin(), sv2.end(), 5);
   BOOST_TEST(it != sv2.end());
   BOOST_TEST_EQ(*it, 5);

   it = segmented_find(sv2.begin(), sv2.end(), 99);
   BOOST_TEST(it == sv2.end());
}

void test_find_every_position()
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
      iter_t it = segmented_find(sv.begin(), sv.end(), vals[i]);
      BOOST_TEST(it != sv.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }
   BOOST_TEST(segmented_find(sv.begin(), sv.end(), 999) == sv.end());
}

void test_find_every_position_seg2()
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
      iter_t it = segmented_find(sv2.begin(), sv2.end(), vals[i]);
      BOOST_TEST(it != sv2.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }
   BOOST_TEST(segmented_find(sv2.begin(), sv2.end(), 999) == sv2.end());
}

// Runs segmented_find over a sub-range whose segmentation shape is dictated by
// a branch spec, so that every level of the recursive dispatch is exercised on
// both its single-segment and its multi-segment path.
//
// A shape matrix runs the same assertions dozens of times over, so a bare
// failure report names a line that says nothing about which of the shapes
// broke.  Every assertion below therefore prints the spec, the size and the
// target when it fails; that is what the spec parameter is for.
struct find_shape_check
{
   int value;

   explicit find_shape_check(int v) : value(v) {}

   void report(const char* spec, std::size_t n) const
   {
      BOOST_LIGHTWEIGHT_TEST_OSTREAM
         << "   shape \"" << spec << "\", n = " << n
         << ", target " << value << std::endl;
   }

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive scan over a flattened copy of the logical range.
      // flatten_n_ints, not flatten_all_ints: the guard element past the end
      // is not part of the answer the algorithm may produce.
      const boost::container::vector<int> flat = test_detail::flatten_n_ints(c, n);
      std::size_t expected = flat.size();
      for(std::size_t i = 0; i != flat.size(); ++i) {
         if(flat[i] == value) { expected = i; break; }
      }

      const iter_t r = segmented_find(first, last, value);
      if(!BOOST_TEST(r == test_detail::iter_at(c, expected)))
         this->report(spec, n);

      // A hit must really hold the value and a miss must land exactly on the
      // end bound; neither follows from the iterator comparison alone once
      // empty segments make several distinct (segment, local) pairs denote
      // the same position.
      if(expected == flat.size()) {
         if(!BOOST_TEST(r == last))
            this->report(spec, n);
      }
      else {
         if(!BOOST_TEST_EQ(test_detail::seg_value_of(*r), value))
            this->report(spec, n);
      }

      // The guard just past the end must never have been read as a match.
      if(!BOOST_TEST(test_detail::filler_intact(c, n, -999)))
         this->report(spec, n);
   }
};

void test_find_shape_matrix()
{
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      // Every value in the range, one absent value, and the out-of-range
      // filler, which must never be found.
      for(std::size_t v = 0; v <= n + 1u; ++v) {
         const int target = (v <= n) ? int(v) : -999;
         test_detail::for_each_shape_all<int>(vals, n, -999, find_shape_check(target));
      }
   }
}

//----------------------------------------------------------------------------
// Single-segment cases. The single-segment range shares its call site with the
// last segment of the multi-segment walk, so it only gets exercised when both
// range bounds live in the same segment, ideally strictly inside it. Each
// range keeps a guard element just past its end, which the algorithm must
// never look at.
//----------------------------------------------------------------------------

void test_find_single_segment_whole()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, 60, 99};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = sv.begin();
   const iter_t last  = test_detail::iter_at(sv, 6);

   BOOST_TEST(segmented_find(first, last, 10) == first);
   BOOST_TEST(segmented_find(first, last, 60) == test_detail::iter_at(sv, 5));
   BOOST_TEST(segmented_find(first, last, 35) == last);
   BOOST_TEST(segmented_find(first, last, 99) == last);
}

void test_find_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, 60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST(segmented_find(first, last, 20) == first);
   BOOST_TEST(segmented_find(first, last, 50) == test_detail::iter_at(sv, 4));
   BOOST_TEST(segmented_find(first, last, 35) == last);
   BOOST_TEST(segmented_find(first, last, 10) == last);
   BOOST_TEST(segmented_find(first, last, 60) == last);
}

void test_find_single_segment_empty_mid()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, 60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t mid = test_detail::iter_at(sv, 3);

   BOOST_TEST(segmented_find(mid, mid, 40) == mid);
   BOOST_TEST(segmented_find(mid, mid, 99) == mid);
}

void test_find_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, 60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST(segmented_find(first, test_detail::make_sentinel(last), 20) == first);
   BOOST_TEST(segmented_find(first, test_detail::make_sentinel(last), 50) == test_detail::iter_at(sv, 4));
   BOOST_TEST(segmented_find(first, test_detail::make_sentinel(last), 60) == last);
}

void test_find_single_segment_seg2_inner_multi()
{
   test_detail::seg_vector<int> inner;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50, 60, 99};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 4);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 6);

   BOOST_TEST(segmented_find(first, last, 20) == first);
   BOOST_TEST(segmented_find(first, last, 40) == test_detail::iter_at(sv2, 3));
   BOOST_TEST(segmented_find(first, last, 60) == test_detail::iter_at(sv2, 5));
   BOOST_TEST(segmented_find(first, last, 10) == last);
   BOOST_TEST(segmented_find(first, last, 99) == last);
}

void test_find_single_segment_seg2_single_inner()
{
   test_detail::seg_vector<int> inner;
   int a[] = {10, 20, 30, 40, 50, 60, 99};
   inner.add_segment_range(a, a + 7);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 5);

   BOOST_TEST(segmented_find(first, last, 20) == first);
   BOOST_TEST(segmented_find(first, last, 50) == test_detail::iter_at(sv2, 4));
   BOOST_TEST(segmented_find(first, last, 35) == last);
   BOOST_TEST(segmented_find(first, last, 10) == last);
   BOOST_TEST(segmented_find(first, last, 60) == last);
}

// The whole suite instantiates the bidirectional category only; the forward
// one is a different instantiation of every dispatch template.
void test_find_single_segment_forward()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;
   typedef cont_t::iterator iter_t;

   cont_t sv;
   int a[] = {10, 20, 30, 40, 50, 60, 99};
   sv.add_segment_range(a, a + 7);

   const iter_t whole_last = test_detail::iter_at(sv, 6);
   BOOST_TEST(segmented_find(sv.begin(), whole_last, 10) == sv.begin());
   BOOST_TEST(segmented_find(sv.begin(), whole_last, 60) == test_detail::iter_at(sv, 5));
   BOOST_TEST(segmented_find(sv.begin(), whole_last, 99) == whole_last);

   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);
   BOOST_TEST(segmented_find(first, last, 20) == first);
   BOOST_TEST(segmented_find(first, last, 50) == test_detail::iter_at(sv, 4));
   BOOST_TEST(segmented_find(first, last, 10) == last);
   BOOST_TEST(segmented_find(first, last, 60) == last);
}

// Forward category, multi-segment, with the match in an earlier segment and no
// match in the last one: the last-segment call must not discard it.
void test_find_forward_match_before_last_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;
   typedef cont_t::iterator iter_t;

   cont_t sv;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50};
   int a3[] = {60, 70, 80};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);
   sv.add_segment_range(a3, a3 + 3);

   const iter_t last = test_detail::iter_at(sv, 7);
   BOOST_TEST(segmented_find(sv.begin(), last, 20) == test_detail::iter_at(sv, 1));
   BOOST_TEST(segmented_find(sv.begin(), last, 40) == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_find(sv.begin(), last, 80) == last);
}

//////////////////////////////////////////////////////////////////////////////
// Comparison count.
//
// [alg.find] mandates "At most last - first applications of the corresponding
// predicate".  The lower bound is what stops the check from passing vacuously:
// the answer cannot be known before the element at it has been compared.
// There is no predicate overload, so the count comes from the value type.
//////////////////////////////////////////////////////////////////////////////

struct find_comparison_check
{
   int value;

   explicit find_comparison_check(int v) : value(v) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      const boost::container::vector<int> flat = test_detail::flatten_n_ints(c, n);
      std::size_t expected = flat.size();
      for(std::size_t i = 0; i != flat.size(); ++i) {
         if(flat[i] == value) { expected = i; break; }
      }

      test_detail::counted_int_ops().reset();
      segmented_find(c.begin(), test_detail::iter_at(c, n), test_detail::counted_int(value));
      const std::size_t applied = test_detail::counted_int_ops().cmp;

      BOOST_TEST(applied <= n);
      BOOST_TEST(applied >= (expected < n ? expected + 1u : n));
      BOOST_TEST(spec != 0);
   }
};

void test_find_comparison_count()
{
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      for(std::size_t v = 0; v <= n + 1u; ++v) {
         const int target = (v <= n) ? int(v) : -999;
         test_detail::for_each_shape_all<test_detail::counted_int>
            (vals, n, -999, find_comparison_check(target));
      }
   }
}

int main()
{
   test_find_shape_matrix();
   test_find_present_first_segment();
   test_find_present_second_segment();
   test_find_not_present();
   test_find_empty();
   test_find_non_segmented();
   test_find_sentinel_segmented();
   test_find_sentinel_non_segmented();
   test_find_seg2();
   test_find_every_position();
   test_find_every_position_seg2();
   test_find_single_segment_whole();
   test_find_single_segment_interior();
   test_find_single_segment_empty_mid();
   test_find_single_segment_sentinel();
   test_find_single_segment_seg2_inner_multi();
   test_find_single_segment_seg2_single_inner();
   test_find_single_segment_forward();
   test_find_forward_match_before_last_segment();
   test_find_comparison_count();
   return boost::report_errors();
}
