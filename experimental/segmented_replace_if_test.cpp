//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_replace_if.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct is_negative
{
   bool operator()(int x) const { return x < 0; }
};

struct is_even
{
   bool operator()(int x) const { return x % 2 == 0; }
};

void test_replace_if_basic()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, -2, 3};
   int a2[] = {-4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   segmented_replace_if(sv.begin(), sv.end(), is_negative(), 0);

   test_detail::seg_vector<int>::iterator it = sv.begin();
   BOOST_TEST_EQ(*it, 1); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 3); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 5);
}

void test_replace_if_empty()
{
   test_detail::seg_vector<int> sv;
   segmented_replace_if(sv.begin(), sv.end(), is_negative(), 0);
   BOOST_TEST_EQ(sv.total_size(), 0u);
}

void test_replace_if_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(-2);
   v.push_back(3);
   v.push_back(-4);

   segmented_replace_if(v.begin(), v.end(), is_negative(), 0);
   BOOST_TEST_EQ(v[0], 1);
   BOOST_TEST_EQ(v[1], 0);
   BOOST_TEST_EQ(v[2], 3);
   BOOST_TEST_EQ(v[3], 0);
}

void test_replace_if_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, -2, 3};
   int a2[] = {-4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   segmented_replace_if(sv.begin(), test_detail::make_sentinel(sv.end()), is_negative(), 0);

   test_detail::seg_vector<int>::iterator it = sv.begin();
   BOOST_TEST_EQ(*it, 1); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 3); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 5);
}

void test_replace_if_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(-2);
   v.push_back(3);
   v.push_back(-4);

   segmented_replace_if(v.begin(), test_detail::make_sentinel(v.end()), is_negative(), 0);
   BOOST_TEST_EQ(v[0], 1);
   BOOST_TEST_EQ(v[1], 0);
   BOOST_TEST_EQ(v[2], 3);
   BOOST_TEST_EQ(v[3], 0);
}

void test_replace_if_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   int a3[] = {7, 8};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);
   sv2.add_flat_segment_range(a3, a3 + 2);

   segmented_replace_if(sv2.begin(), sv2.end(), is_even(), 0);

   int expected[] = {1, 0, 3, 0, 5, 0, 7, 0};
   test_detail::seg2_vector<int>::iterator it = sv2.begin();
   for(int i = 0; i < 8; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
}

// Replaces within a sub-range whose segmentation shape is dictated by a branch
// spec, so that every level of the recursive dispatch is exercised on its
// single-segment path, on its multi-segment path and on the multi-segment path
// with empty segments interleaved.  The guard element just past the end
// satisfies the predicate, so an algorithm that writes past the end is caught
// by filler_intact rather than going unnoticed.
template<class Pred>
struct replace_if_shape_check
{
   Pred pred;
   int newv;
   int filler;

   replace_if_shape_check(Pred p, int nv, int f) : pred(p), newv(nv), filler(f) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      const boost::container::vector<int> before = test_detail::flatten_n_ints(c, n);

      segmented_replace_if(first, last, pred, newv);

      const boost::container::vector<int> after = test_detail::flatten_n_ints(c, n);
      BOOST_TEST_EQ(after.size(), before.size());
      for(std::size_t i = 0; i != after.size(); ++i)
         BOOST_TEST_EQ(after[i], pred(before[i]) ? newv : before[i]);

      BOOST_TEST(test_detail::filler_intact(c, n, filler));
      BOOST_TEST(spec != 0);
   }
};

template<class Pred>
replace_if_shape_check<Pred> make_replace_if_check(Pred p, int nv, int f)
{  return replace_if_shape_check<Pred>(p, nv, f);   }

void test_replace_if_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];

      //Alternating signs, then no match at all, then every element a match.
      for(int i = 0; i != 16; ++i)
         vals[i] = (i % 2) ? (i + 1) : -(i + 1);
      test_detail::for_each_shape_all<int>(vals, n, -6, make_replace_if_check(is_negative(), 0, -6));

      for(int i = 0; i != 16; ++i)
         vals[i] = i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -6, make_replace_if_check(is_negative(), 0, -6));

      for(int i = 0; i != 16; ++i)
         vals[i] = -(i + 1);
      test_detail::for_each_shape_all<int>(vals, n, -6, make_replace_if_check(is_negative(), 0, -6));

      //Exactly one match, at each position in turn.
      for(std::size_t p = 0; p != n; ++p) {
         for(std::size_t i = 0; i != 16u; ++i)
            vals[i] = (i == p) ? -int(i + 1u) : int(i + 1u);
         test_detail::for_each_shape_all<int>(vals, n, -6,
                                              make_replace_if_check(is_negative(), 0, -6));
      }

      //A second predicate, over values that are all positive: the guard is
      //even, so it too would be rewritten by an overrun.
      for(int i = 0; i != 16; ++i)
         vals[i] = i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -998,
                                           make_replace_if_check(is_even(), -1, -998));
   }
}

void test_replace_if_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-1, 2, -3, 4, -5, 6, -6};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   segmented_replace_if(test_detail::iter_at(sv, 1),
                        test_detail::make_sentinel(test_detail::iter_at(sv, 6)),
                        is_negative(), 0);

   int expected[] = {-1, 2, 0, 4, 0, 6, -6};
   iter_t it = sv.begin();
   for(int i = 0; i < 7; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
}

//////////////////////////////////////////////////////////////////////////////
// Predicate application count.
//
// [alg.replace] mandates "Exactly last - first applications of the
// corresponding predicate", so a segment walked twice or an element skipped is
// caught here even where the rewritten values still come out right.
//////////////////////////////////////////////////////////////////////////////

struct replace_if_count_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      test_detail::op_counter calls;
      segmented_replace_if(c.begin(), test_detail::iter_at(c, n),
                           test_detail::counting_pred(calls, is_negative()), 0);

      BOOST_TEST_EQ(calls.n, n);
      BOOST_TEST(spec != 0);
   }
};

void test_replace_if_predicate_count()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];

      for(int i = 0; i != 16; ++i)
         vals[i] = (i % 2) ? (i + 1) : -(i + 1);
      test_detail::for_each_shape_all<int>(vals, n, -6, replace_if_count_check());

      for(int i = 0; i != 16; ++i)
         vals[i] = i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -6, replace_if_count_check());

      for(int i = 0; i != 16; ++i)
         vals[i] = -(i + 1);
      test_detail::for_each_shape_all<int>(vals, n, -6, replace_if_count_check());
   }
}

int main()
{
   test_replace_if_shape_matrix();
   test_replace_if_basic();
   test_replace_if_empty();
   test_replace_if_non_segmented();
   test_replace_if_sentinel_segmented();
   test_replace_if_sentinel_non_segmented();
   test_replace_if_seg2();
   test_replace_if_single_segment_sentinel();
   test_replace_if_predicate_count();
   return boost::report_errors();
}
