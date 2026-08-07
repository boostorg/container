//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_for_each.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct summer
{
   int* psum;
   explicit summer(int* p) : psum(p) {}
   void operator()(int x) { *psum += x; }
};

struct doubler
{
   void operator()(int& x) const { x *= 2; }
};

void test_for_each_sum()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   int sum = 0;
   segmented_for_each(sv.begin(), sv.end(), summer(&sum));
   BOOST_TEST_EQ(sum, 15);
}

void test_for_each_empty()
{
   test_detail::seg_vector<int> sv;
   int sum = 0;
   segmented_for_each(sv.begin(), sv.end(), summer(&sum));
   BOOST_TEST_EQ(sum, 0);
}

void test_for_each_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(2);
   v.push_back(3);
   int sum = 0;
   segmented_for_each(v.begin(), v.end(), summer(&sum));
   BOOST_TEST_EQ(sum, 6);
}

void test_for_each_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 2);

   int sum = 0;
   segmented_for_each(sv.begin(), test_detail::make_sentinel(sv.end()), summer(&sum));
   BOOST_TEST_EQ(sum, 15);
}

void test_for_each_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(2);
   v.push_back(3);
   int sum = 0;
   segmented_for_each(v.begin(), test_detail::make_sentinel(v.end()), summer(&sum));
   BOOST_TEST_EQ(sum, 6);
}

void test_for_each_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   int sum = 0;
   segmented_for_each(sv2.begin(), sv2.end(), summer(&sum));
   BOOST_TEST_EQ(sum, 45);
}

//Accumulates in its own state so that the returned function object can be
//inspected, rather than through a pointer to an external variable.
struct value_summer
{
   int sum;
   value_summer() : sum(0) {}
   void operator()(int x) { sum += x; }
};

// Walks a sub-range whose segmentation shape is dictated by a branch spec, so
// that every level of the recursive dispatch is exercised on its
// single-segment path, on its multi-segment path and on the multi-segment path
// with empty segments interleaved.  Both flavours of stateful callable are
// used: one whose state travels back in the returned function object and one
// whose state lives behind a pointer, since the combinators copy the callable
// by value and state kept in a plain member of a discarded copy would vanish.
struct for_each_shape_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      const boost::container::vector<int> before = test_detail::flatten_n_ints(c, n);
      int total = 0;
      for(std::size_t i = 0; i != before.size(); ++i)
         total += before[i];

      BOOST_TEST_EQ(segmented_for_each(first, last, value_summer()).sum, total);

      int sum = 0;
      segmented_for_each(first, last, summer(&sum));
      BOOST_TEST_EQ(sum, total);

      //A callable that writes: every element doubled and the guard untouched.
      segmented_for_each(first, last, doubler());
      const boost::container::vector<int> after = test_detail::flatten_n_ints(c, n);
      BOOST_TEST_EQ(after.size(), before.size());
      for(std::size_t i = 0; i != after.size(); ++i)
         BOOST_TEST_EQ(after[i], before[i] * 2);

      BOOST_TEST(test_detail::filler_intact(c, n, -999));
      BOOST_TEST(spec != 0);
   }
};

void test_for_each_shape_matrix()
{
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s)
      test_detail::for_each_shape_all<int>(vals, sizes[s], -999, for_each_shape_check());
}

void test_for_each_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {100, 1, 2, 3, 4, 5, 200};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 6);

   BOOST_TEST_EQ(segmented_for_each(first, test_detail::make_sentinel(last), value_summer()).sum, 15);

   segmented_for_each(first, test_detail::make_sentinel(last), doubler());
   int expected[] = {100, 2, 4, 6, 8, 10, 200};
   iter_t it = sv.begin();
   for(int i = 0; i < 7; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
}

//////////////////////////////////////////////////////////////////////////////
// Function application count.
//
// [alg.foreach] mandates "Applies f exactly last - first times", so a segment
// walked twice or one skipped is caught here even in the cases where the
// accumulated answer still comes out right.
//////////////////////////////////////////////////////////////////////////////

struct for_each_count_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      test_detail::op_counter calls;
      segmented_for_each(c.begin(), test_detail::iter_at(c, n),
                         test_detail::counting_fun<doubler, void>(calls, doubler()));

      BOOST_TEST_EQ(calls.n, n);
      BOOST_TEST(test_detail::filler_intact(c, n, -999));
      BOOST_TEST(spec != 0);
   }
};

void test_for_each_application_count()
{
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s)
      test_detail::for_each_shape_all<int>(vals, sizes[s], -999, for_each_count_check());
}

int main()
{
   test_for_each_shape_matrix();
   test_for_each_sum();
   test_for_each_empty();
   test_for_each_non_segmented();
   test_for_each_sentinel_segmented();
   test_for_each_sentinel_non_segmented();
   test_for_each_seg2();
   test_for_each_single_segment_sentinel();
   test_for_each_application_count();
   return boost::report_errors();
}
