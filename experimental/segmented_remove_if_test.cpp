//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_remove_if.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct is_even
{
   bool operator()(int x) const { return x % 2 == 0; }
   bool operator()(const test_detail::movable_int& x) const { return x.value() % 2 == 0; }
};

struct is_odd
{
   bool operator()(int x) const { return x % 2 != 0; }
   bool operator()(const test_detail::movable_int& x) const { return x.value() % 2 != 0; }
};

void test_remove_if_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a1 + 3, a1 + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(sv.begin(), sv.end(), is_even());

   int expected[] = {1, 3, 5};
   iter_t it = sv.begin();
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST(it == new_end);
}

void test_remove_if_no_match()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 3, 5};
   sv.add_segment_range(a1, a1 + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(sv.begin(), sv.end(), is_even());
   BOOST_TEST(new_end == sv.end());
}

void test_remove_if_all_match()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {2, 4, 6};
   sv.add_segment_range(a1, a1 + 3);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(sv.begin(), sv.end(), is_even());
   BOOST_TEST(new_end == sv.begin());
}

void test_remove_if_empty()
{
   test_detail::seg_vector<int> sv;
   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(sv.begin(), sv.end(), is_even());
   BOOST_TEST(new_end == sv.begin());
}

void test_remove_if_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3); v.push_back(4); v.push_back(5);

   boost::container::vector<int>::iterator new_end = segmented_remove_if(v.begin(), v.end(), is_odd());
   BOOST_TEST_EQ(new_end - v.begin(), 2);
   BOOST_TEST_EQ(v[0], 2);
   BOOST_TEST_EQ(v[1], 4);
}

void test_remove_if_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a1 + 3, a1 + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(sv.begin(), test_detail::make_sentinel(sv.end()), is_even());

   int expected[] = {1, 3, 5};
   iter_t it = sv.begin();
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST(it == new_end);
}

void test_remove_if_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3); v.push_back(4); v.push_back(5);

   boost::container::vector<int>::iterator new_end =
      segmented_remove_if(v.begin(), test_detail::make_sentinel(v.end()), is_odd());
   BOOST_TEST_EQ(new_end - v.begin(), 2);
   BOOST_TEST_EQ(v[0], 2);
   BOOST_TEST_EQ(v[1], 4);
}

void test_remove_if_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(sv2.begin(), sv2.end(), is_even());

   int expected[] = {1, 3, 5};
   iter_t it = sv2.begin();
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST(it == new_end);
}

void test_remove_if_movable_seg()
{
   typedef test_detail::movable_int mi;
   test_detail::seg_vector<mi> sv;
   int a1[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_from_ints(a1, a1 + 3);
   sv.add_segment_from_ints(a1 + 3, a1 + 6);

   typedef test_detail::seg_vector<mi>::iterator iter_t;
   iter_t new_end = segmented_remove_if(sv.begin(), sv.end(), is_even());

   int expected[] = {1, 3, 5};
   iter_t it = sv.begin();
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(it->value(), expected[i]);
   BOOST_TEST(it == new_end);
}

void test_remove_if_movable_seg2()
{
   typedef test_detail::movable_int mi;
   test_detail::seg2_vector<mi> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv2.add_flat_segment_from_ints(a1, a1 + 3);
   sv2.add_flat_segment_from_ints(a2, a2 + 3);

   typedef test_detail::seg2_vector<mi>::iterator iter_t;
   iter_t new_end = segmented_remove_if(sv2.begin(), sv2.end(), is_even());

   int expected[] = {1, 3, 5};
   iter_t it = sv2.begin();
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(it->value(), expected[i]);
   BOOST_TEST(it == new_end);
}

void test_remove_if_single_segment_interior()
{
   test_detail::seg_vector<int> sv;
   int a[] = {101, 1, 2, 3, 4, 5, 201};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 6), is_even());

   BOOST_TEST(new_end == test_detail::iter_at(sv, 4));
   int expected[] = {1, 3, 5};
   iter_t it = test_detail::iter_at(sv, 1);
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 0), 101);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 6), 201);
}

void test_remove_if_single_segment_interior_odd()
{
   test_detail::seg_vector<int> sv;
   int a[] = {102, 2, 1, 4, 3, 6, 202};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 6), is_odd());

   BOOST_TEST(new_end == test_detail::iter_at(sv, 4));
   int expected[] = {2, 4, 6};
   iter_t it = test_detail::iter_at(sv, 1);
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 0), 102);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 6), 202);
}

void test_remove_if_single_segment_first_element()
{
   test_detail::seg_vector<int> sv;
   int a[] = {101, 2, 1, 3, 5, 201};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), is_even());

   BOOST_TEST(new_end == test_detail::iter_at(sv, 4));
   int expected[] = {1, 3, 5};
   iter_t it = test_detail::iter_at(sv, 1);
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 0), 101);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 5), 201);
}

void test_remove_if_single_segment_last_element()
{
   test_detail::seg_vector<int> sv;
   int a[] = {101, 1, 3, 5, 2, 201};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), is_even());

   BOOST_TEST(new_end == test_detail::iter_at(sv, 4));
   int expected[] = {1, 3, 5};
   iter_t it = test_detail::iter_at(sv, 1);
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 0), 101);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 5), 201);
}

void test_remove_if_single_segment_no_match()
{
   test_detail::seg_vector<int> sv;
   int a[] = {101, 1, 3, 5, 7, 201};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(test_detail::iter_at(sv, 1), test_detail::iter_at(sv, 5), is_even());

   BOOST_TEST(new_end == test_detail::iter_at(sv, 5));
   iter_t it = sv.begin();
   for(int i = 0; i < 6; ++i, ++it)
      BOOST_TEST_EQ(*it, a[i]);
}

void test_remove_if_single_segment_empty_mid()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 3, 4, 5, 6};
   sv.add_segment_range(a, a + 6);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t mid = test_detail::iter_at(sv, 3);
   iter_t new_end = segmented_remove_if(mid, mid, is_even());

   BOOST_TEST(new_end == mid);
   iter_t it = sv.begin();
   for(int i = 0; i < 6; ++i, ++it)
      BOOST_TEST_EQ(*it, a[i]);
}

void test_remove_if_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {101, 1, 2, 3, 4, 5, 201};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(test_detail::iter_at(sv, 1),
                                        test_detail::make_sentinel(test_detail::iter_at(sv, 6)), is_even());

   BOOST_TEST(new_end == test_detail::iter_at(sv, 4));
   int expected[] = {1, 3, 5};
   iter_t it = test_detail::iter_at(sv, 1);
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 0), 101);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 6), 201);
}

void test_remove_if_single_segment_seg2_outer()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a1[] = {101, 1, 2};
   int a2[] = {3, 4};
   int a3[] = {5, 201};
   inner.add_segment_range(a1, a1 + 3);
   inner.add_segment_range(a2, a2 + 2);
   inner.add_segment_range(a3, a3 + 2);
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), is_even());

   BOOST_TEST(new_end == test_detail::iter_at(sv2, 4));
   int expected[] = {1, 3, 5};
   iter_t it = test_detail::iter_at(sv2, 1);
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST_EQ(*test_detail::iter_at(sv2, 0), 101);
   BOOST_TEST_EQ(*test_detail::iter_at(sv2, 6), 201);
}

void test_remove_if_single_segment_seg2_both()
{
   test_detail::seg2_vector<int> sv2;
   test_detail::seg_vector<int> inner;
   int a[] = {101, 1, 2, 3, 4, 5, 201};
   inner.add_segment_range(a, a + 7);
   sv2.add_segment(inner);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t new_end = segmented_remove_if(test_detail::iter_at(sv2, 1), test_detail::iter_at(sv2, 6), is_even());

   BOOST_TEST(new_end == test_detail::iter_at(sv2, 4));
   int expected[] = {1, 3, 5};
   iter_t it = test_detail::iter_at(sv2, 1);
   for(int i = 0; i < 3; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
   BOOST_TEST_EQ(*test_detail::iter_at(sv2, 0), 101);
   BOOST_TEST_EQ(*test_detail::iter_at(sv2, 6), 201);
}

//////////////////////////////////////////////////////////////////////////////
// Shape matrix.
//
// segmented_remove_if is in place, so there is only one range and
// for_each_shape_all is the right combinator: the twelve branch specs over
// both segmentation depths, the 'e' ones carrying empty segments.  Both
// predicates are driven, so the removed and the kept half swap over.
//
// What can be asserted after the call is the returned new logical end, the
// surviving prefix in front of it, and the guard just past the end of the
// range.  The elements between the new end and the old one are left in a
// valid-but-unspecified state by definition, so nothing is asserted about
// them.  The reference is filtered out of flatten_n_ints over the logical
// range, not flatten_all_ints, which deliberately includes the guard.
//////////////////////////////////////////////////////////////////////////////

const int shape_filler = -999;   // guard just past the end of the range

template<class Pred>
struct remove_if_shape_check
{
   template<class C>
   void operator()(C& c, std::size_t n, const char* spec) const
   {
      typedef typename C::iterator iter_t;

      const boost::container::vector<int> in = test_detail::flatten_n_ints(c, n);
      boost::container::vector<int> ref;
      for(std::size_t i = 0; i != n; ++i)
         if(!Pred()(in[i]))
            ref.push_back(in[i]);

      const iter_t new_end = segmented_remove_if(c.begin(), test_detail::iter_at(c, n), Pred());

      BOOST_TEST(new_end == test_detail::iter_at(c, ref.size()));
      BOOST_TEST(test_detail::filler_intact(c, n, shape_filler));

      const boost::container::vector<int> got = test_detail::flatten_n_ints(c, ref.size());
      BOOST_TEST_EQ(got.size(), ref.size());
      for(std::size_t i = 0; i != ref.size(); ++i)
         BOOST_TEST_EQ(got[i], ref[i]);

      BOOST_TEST(spec != 0);
   }
};

void test_remove_if_shape_matrix()
{
   // Alternating, a leading and a trailing match, all odd and all even, so
   // that each predicate sees an empty, a full and a partial answer.
   static const int mixed[]  = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
   static const int edges[]  = {2, 1, 3, 5, 7, 4};
   static const int odds[]   = {1, 3, 5, 7, 9, 11};
   static const int evens[]  = {2, 4, 6, 8, 10, 12};
   static const int* const sets[] = {mixed, edges, odds, evens};
   static const std::size_t set_len[] = {10u, 6u, 6u, 6u};
   static const std::size_t sizes[] = {0u, 1u, 2u, 3u, 5u, 6u, 10u};

   for(std::size_t s = 0; s != sizeof(sets)/sizeof(sets[0]); ++s) {
      for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
         const std::size_t n = sizes[i];
         if(n > set_len[s])
            continue;
         test_detail::for_each_shape_all<int>
            (sets[s], n, shape_filler, remove_if_shape_check<is_even>());
         test_detail::for_each_shape_all<int>
            (sets[s], n, shape_filler, remove_if_shape_check<is_odd>());
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Predicate application count.
//
// [alg.remove] mandates "Exactly last - first applications of the corresponding
// predicate".  segmented_remove_if reaches that total in two pieces, the
// find_if that locates the first match and the compacting pass over the rest,
// so an element tested by both, or one re-tested when the write pointer crosses
// a segment boundary, shows up as a surplus here.
//////////////////////////////////////////////////////////////////////////////

template<class Pred>
struct remove_if_count_check
{
   template<class C>
   void operator()(C& c, std::size_t n, const char* spec) const
   {
      test_detail::op_counter calls;
      segmented_remove_if(c.begin(), test_detail::iter_at(c, n),
                          test_detail::counting_pred(calls, Pred()));

      BOOST_TEST_EQ(calls.n, n);
      BOOST_TEST(test_detail::filler_intact(c, n, shape_filler));
      BOOST_TEST(spec != 0);
   }
};

void test_remove_if_predicate_count()
{
   static const int mixed[]  = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
   static const int edges[]  = {2, 1, 3, 5, 7, 4};
   static const int odds[]   = {1, 3, 5, 7, 9, 11};
   static const int evens[]  = {2, 4, 6, 8, 10, 12};
   static const int* const sets[] = {mixed, edges, odds, evens};
   static const std::size_t set_len[] = {10u, 6u, 6u, 6u};
   static const std::size_t sizes[] = {0u, 1u, 2u, 3u, 5u, 6u, 10u};

   for(std::size_t s = 0; s != sizeof(sets)/sizeof(sets[0]); ++s) {
      for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
         const std::size_t n = sizes[i];
         if(n > set_len[s])
            continue;
         test_detail::for_each_shape_all<int>
            (sets[s], n, shape_filler, remove_if_count_check<is_even>());
         test_detail::for_each_shape_all<int>
            (sets[s], n, shape_filler, remove_if_count_check<is_odd>());
      }
   }
}

int main()
{
   test_remove_if_segmented();
   test_remove_if_no_match();
   test_remove_if_all_match();
   test_remove_if_single_segment_interior();
   test_remove_if_single_segment_interior_odd();
   test_remove_if_single_segment_first_element();
   test_remove_if_single_segment_last_element();
   test_remove_if_single_segment_no_match();
   test_remove_if_single_segment_empty_mid();
   test_remove_if_single_segment_sentinel();
   test_remove_if_single_segment_seg2_outer();
   test_remove_if_single_segment_seg2_both();
   test_remove_if_empty();
   test_remove_if_non_segmented();
   test_remove_if_sentinel_segmented();
   test_remove_if_sentinel_non_segmented();
   test_remove_if_seg2();
   test_remove_if_movable_seg();
   test_remove_if_movable_seg2();
   test_remove_if_shape_matrix();
   test_remove_if_predicate_count();
   return boost::report_errors();
}
