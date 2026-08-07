//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_find_if_not.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct is_positive
{
   bool operator()(int x) const { return x > 0; }
};

struct not_equals_val
{
   int v;
   not_equals_val(int x) : v(x) {}
   bool operator()(int x) const { return x != v; }
};

// Violated by several elements at once, so that the early exit has to stop at
// the first of them. The x > 0 term keeps the negative out-of-range guard out,
// whatever the modulus.
struct not_multiple_of
{
   int m;
   not_multiple_of(int x) : m(x) {}
   bool operator()(int x) const { return !(x > 0 && (x % m) == 0); }
};

void test_find_if_not_empty()
{
   test_detail::seg_vector<int> sv;
   test_detail::seg_vector<int>::iterator it =
      segmented_find_if_not(sv.begin(), sv.end(), is_positive());
   BOOST_TEST(it == sv.end());
}

void test_find_if_not_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(-2);
   v.push_back(3);

   boost::container::vector<int>::iterator it =
      segmented_find_if_not(v.begin(), v.end(), is_positive());
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, -2);

   v.clear();
   v.push_back(1);
   v.push_back(2);
   v.push_back(3);
   it = segmented_find_if_not(v.begin(), v.end(), is_positive());
   BOOST_TEST(it == v.end());
}

void test_find_if_not_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {-4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   test_detail::seg_vector<int>::iterator it =
      segmented_find_if_not(sv.begin(), test_detail::make_sentinel(sv.end()), is_positive());
   BOOST_TEST(it != sv.end());
   BOOST_TEST_EQ(*it, -4);
}

void test_find_if_not_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(-2);
   v.push_back(3);

   boost::container::vector<int>::iterator it =
      segmented_find_if_not(v.begin(), test_detail::make_sentinel(v.end()), is_positive());
   BOOST_TEST(it != v.end());
   BOOST_TEST_EQ(*it, -2);
}

void test_find_if_not_every_position()
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
      iter_t it = segmented_find_if_not(sv.begin(), sv.end(), not_equals_val(vals[i]));
      BOOST_TEST(it != sv.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }
   BOOST_TEST(segmented_find_if_not(sv.begin(), sv.end(), not_equals_val(999)) == sv.end());
}

void test_find_if_not_every_position_seg2()
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
      iter_t it = segmented_find_if_not(sv2.begin(), sv2.end(), not_equals_val(vals[i]));
      BOOST_TEST(it != sv2.end());
      BOOST_TEST_EQ(*it, vals[i]);
      BOOST_TEST(it == expected);
   }
   BOOST_TEST(segmented_find_if_not(sv2.begin(), sv2.end(), not_equals_val(999)) == sv2.end());
}

// Runs segmented_find_if_not over a range whose segmentation shape is dictated
// by a branch spec, so that every level of the recursive dispatch is exercised
// on its single-segment, its multi-segment and its empty-segment path.
// segmented_find_if_not is segmented_find_if under a negated predicate, so the
// machinery below the wrapper is the one segmented_find_if_test also drives;
// what is checked here is that the negation is applied with the right sense.
template<class Pred>
struct find_if_not_shape_check
{
   Pred pred;

   explicit find_if_not_shape_check(Pred p) : pred(p) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      // Reference: naive scan over a flattened copy of the logical range. The
      // guard past the end is deliberately not part of it.
      const boost::container::vector<int> flat = test_detail::flatten_n_ints(c, n);
      std::size_t expected = flat.size();
      for(std::size_t i = 0; i != flat.size(); ++i) {
         if(!pred(flat[i])) { expected = i; break; }
      }

      const iter_t r = segmented_find_if_not(first, last, pred);
      BOOST_TEST(r == test_detail::iter_at(c, expected));
      BOOST_TEST(spec != 0);
   }
};

// Bidirectional and forward iterators instantiate every dispatch template
// separately, so each shape is driven through both.
template<class Pred>
void run_find_if_not_shapes(const int* vals, std::size_t n, Pred pred)
{
   test_detail::for_each_shape_all<int>(vals, n, -999, find_if_not_shape_check<Pred>(pred));
   test_detail::for_each_shape_all_fwd<int>(vals, n, -999, find_if_not_shape_check<Pred>(pred));
}

void test_find_if_not_shape_matrix()
{
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      // The violating element at the first position, at every interior one and
      // at the last one; then a value no element has; then the guard value,
      // which lies past the end and must never be found.
      for(std::size_t v = 0; v <= n + 1u; ++v)
         run_find_if_not_shapes(vals, n, not_equals_val((v <= n) ? int(v) : -999));
      // Predicates violated by many elements: all of them, every second one
      // and every third one, so a violation in an earlier segment coexists
      // with violations in later ones.
      run_find_if_not_shapes(vals, n, not_multiple_of(1));
      run_find_if_not_shapes(vals, n, not_multiple_of(2));
      run_find_if_not_shapes(vals, n, not_multiple_of(3));
      // Violated by nothing at all, guard included.
      run_find_if_not_shapes(vals, n, is_positive());
   }
}

//----------------------------------------------------------------------------
// Single-segment cases with data before the start of the range. The shape
// matrix always starts its range at the container's first element, so the
// lower bound is only exercised here: each range below leaves a violating
// element just before its start, and a negative guard just past its end.
//----------------------------------------------------------------------------

void test_find_if_not_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, -60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(20)) == first);
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(50)) == test_detail::iter_at(sv, 4));
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(35)) == last);
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(10)) == last);
   BOOST_TEST(segmented_find_if_not(first, last, is_positive()) == last);
}

void test_find_if_not_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {10, 20, 30, 40, 50, -60};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);

   BOOST_TEST(segmented_find_if_not(first, test_detail::make_sentinel(last), not_equals_val(20)) == first);
   BOOST_TEST(segmented_find_if_not(first, test_detail::make_sentinel(last), not_equals_val(50)) == test_detail::iter_at(sv, 4));
   BOOST_TEST(segmented_find_if_not(first, test_detail::make_sentinel(last), is_positive()) == last);
}

void test_find_if_not_single_segment_seg2_inner_multi()
{
   test_detail::seg_vector<int> inner;
   int a1[] = {10, 20, 30};
   int a2[] = {40, 50, 60, -99};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 4);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 6);

   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(20)) == first);
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(40)) == test_detail::iter_at(sv2, 3));
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(60)) == test_detail::iter_at(sv2, 5));
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(10)) == last);
   BOOST_TEST(segmented_find_if_not(first, last, is_positive()) == last);
}

void test_find_if_not_single_segment_seg2_single_inner()
{
   test_detail::seg_vector<int> inner;
   int a[] = {10, 20, 30, 40, 50, 60, -99};
   inner.add_segment_range(a, a + 7);

   test_detail::seg2_vector<int> sv2;
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv2, 1);
   const iter_t last  = test_detail::iter_at(sv2, 5);

   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(20)) == first);
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(50)) == test_detail::iter_at(sv2, 4));
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(35)) == last);
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(10)) == last);
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(60)) == last);
}

// The whole suite instantiates the bidirectional category only; the forward
// one is a different instantiation of every dispatch template.
void test_find_if_not_single_segment_forward()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> cont_t;
   typedef cont_t::iterator iter_t;

   cont_t sv;
   int a[] = {10, 20, 30, 40, 50, 60, -99};
   sv.add_segment_range(a, a + 7);

   const iter_t whole_last = test_detail::iter_at(sv, 6);
   BOOST_TEST(segmented_find_if_not(sv.begin(), whole_last, not_equals_val(10)) == sv.begin());
   BOOST_TEST(segmented_find_if_not(sv.begin(), whole_last, not_equals_val(60)) == test_detail::iter_at(sv, 5));
   BOOST_TEST(segmented_find_if_not(sv.begin(), whole_last, is_positive()) == whole_last);

   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 5);
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(20)) == first);
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(50)) == test_detail::iter_at(sv, 4));
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(10)) == last);
   BOOST_TEST(segmented_find_if_not(first, last, not_equals_val(60)) == last);
}

// Forward category, multi-segment, with the match in an earlier segment and no
// match in the last one: the last-segment call must not discard it.
void test_find_if_not_forward_match_before_last_segment()
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
   BOOST_TEST(segmented_find_if_not(sv.begin(), last, not_equals_val(20)) == test_detail::iter_at(sv, 1));
   BOOST_TEST(segmented_find_if_not(sv.begin(), last, not_equals_val(40)) == test_detail::iter_at(sv, 3));
   BOOST_TEST(segmented_find_if_not(sv.begin(), last, not_equals_val(80)) == last);
}

//////////////////////////////////////////////////////////////////////////////
// Predicate application count.
//
// [alg.find] mandates "At most last - first applications of the corresponding
// predicate".  The lower bound is not in the standard but follows from the
// answer: the result cannot be known without having applied pred to every
// element up to and including the one returned.  Bracketing the count between
// the two catches an element retested when the scan crosses a segment
// boundary as well as a scan that stops short of the answer.
//////////////////////////////////////////////////////////////////////////////

template<class Pred>
struct find_if_not_count_check
{
   Pred pred;

   explicit find_if_not_count_check(Pred p) : pred(p) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      const boost::container::vector<int> flat = test_detail::flatten_n_ints(c, n);
      std::size_t needed = n;
      for(std::size_t i = 0; i != n; ++i)
         if(!pred(flat[i])) { needed = i + 1u; break; }

      test_detail::op_counter calls;
      segmented_find_if_not(c.begin(), test_detail::iter_at(c, n),
                            test_detail::counting_pred(calls, pred));

      BOOST_TEST(calls.n <= n);
      BOOST_TEST(calls.n >= needed);
      BOOST_TEST(spec != 0);
   }
};

template<class Pred>
void run_find_if_not_count_shapes(const int* vals, std::size_t n, Pred pred)
{
   test_detail::for_each_shape_all<int>(vals, n, -999, find_if_not_count_check<Pred>(pred));
   test_detail::for_each_shape_all_fwd<int>(vals, n, -999, find_if_not_count_check<Pred>(pred));
}

void test_find_if_not_predicate_count()
{
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      for(std::size_t v = 0; v <= n + 1u; ++v)
         run_find_if_not_count_shapes(vals, n, not_equals_val((v <= n) ? int(v) : -999));
      run_find_if_not_count_shapes(vals, n, not_multiple_of(1));
      run_find_if_not_count_shapes(vals, n, not_multiple_of(3));
      run_find_if_not_count_shapes(vals, n, is_positive());
   }
}

int main()
{
   test_find_if_not_shape_matrix();
   test_find_if_not_empty();
   test_find_if_not_non_segmented();
   test_find_if_not_sentinel_segmented();
   test_find_if_not_sentinel_non_segmented();
   test_find_if_not_every_position();
   test_find_if_not_every_position_seg2();
   test_find_if_not_single_segment_interior();
   test_find_if_not_single_segment_sentinel();
   test_find_if_not_single_segment_seg2_inner_multi();
   test_find_if_not_single_segment_seg2_single_inner();
   test_find_if_not_single_segment_forward();
   test_find_if_not_forward_match_before_last_segment();
   test_find_if_not_predicate_count();
   return boost::report_errors();
}
