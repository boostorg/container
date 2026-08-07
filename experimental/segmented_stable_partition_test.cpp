//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_stable_partition.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct is_even
{
   bool operator()(int v) const { return v % 2 == 0; }
   bool operator()(const test_detail::movable_int& v) const { return v.value() % 2 == 0; }
};

void test_stable_partition_basic()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(2); v.push_back(3);
   v.push_back(4); v.push_back(5); v.push_back(6);

   boost::container::vector<int>::iterator mid = segmented_stable_partition(v.begin(), v.end(), is_even());

   BOOST_TEST_EQ(v[0], 2);
   BOOST_TEST_EQ(v[1], 4);
   BOOST_TEST_EQ(v[2], 6);
   BOOST_TEST_EQ(v[3], 1);
   BOOST_TEST_EQ(v[4], 3);
   BOOST_TEST_EQ(v[5], 5);

   int dist = 0;
   for(boost::container::vector<int>::iterator it = v.begin(); it != mid; ++it)
      ++dist;
   BOOST_TEST_EQ(dist, 3);
}

void test_stable_partition_empty()
{
   boost::container::vector<int> v;
   boost::container::vector<int>::iterator mid = segmented_stable_partition(v.begin(), v.end(), is_even());
   BOOST_TEST(mid == v.end());
}

void test_stable_partition_all_true()
{
   boost::container::vector<int> v;
   v.push_back(2); v.push_back(4); v.push_back(6);
   boost::container::vector<int>::iterator mid = segmented_stable_partition(v.begin(), v.end(), is_even());
   BOOST_TEST(mid == v.end());
   BOOST_TEST_EQ(v[0], 2);
   BOOST_TEST_EQ(v[1], 4);
   BOOST_TEST_EQ(v[2], 6);
}

void test_stable_partition_all_false()
{
   boost::container::vector<int> v;
   v.push_back(1); v.push_back(3); v.push_back(5);
   boost::container::vector<int>::iterator mid = segmented_stable_partition(v.begin(), v.end(), is_even());
   BOOST_TEST(mid == v.begin());
   BOOST_TEST_EQ(v[0], 1);
   BOOST_TEST_EQ(v[1], 3);
   BOOST_TEST_EQ(v[2], 5);
}

void test_stable_partition_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(5); v.push_back(2); v.push_back(7); v.push_back(4); v.push_back(1);

   boost::container::vector<int>::iterator mid = segmented_stable_partition(v.begin(), v.end(), is_even());

   BOOST_TEST_EQ(v[0], 2);
   BOOST_TEST_EQ(v[1], 4);
   BOOST_TEST_EQ(v[2], 5);
   BOOST_TEST_EQ(v[3], 7);
   BOOST_TEST_EQ(v[4], 1);

   int even_count = 0;
   for(boost::container::vector<int>::iterator it = v.begin(); it != mid; ++it)
      ++even_count;
   BOOST_TEST_EQ(even_count, 2);
}


void test_stable_partition_movable_seg()
{
   typedef test_detail::movable_int mi;
   test_detail::seg_vector<mi> sv;
   int a1[] = {3, 2, 1};
   int a2[] = {6, 5, 4};
   sv.add_segment_from_ints(a1, a1 + 3);
   sv.add_segment_from_ints(a2, a2 + 3);

   typedef test_detail::seg_vector<mi>::iterator iter_t;
   iter_t mid = segmented_stable_partition(sv.begin(), sv.end(), is_even());

   iter_t it = sv.begin();
   BOOST_TEST_EQ(it->value(), 2); ++it;
   BOOST_TEST_EQ(it->value(), 6); ++it;
   BOOST_TEST_EQ(it->value(), 4); ++it;
   BOOST_TEST(it == mid);
   BOOST_TEST_EQ(it->value(), 3); ++it;
   BOOST_TEST_EQ(it->value(), 1); ++it;
   BOOST_TEST_EQ(it->value(), 5);
}

void test_stable_partition_movable_seg2()
{
   typedef test_detail::movable_int mi;
   test_detail::seg2_vector<mi> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv2.add_flat_segment_from_ints(a1, a1 + 3);
   sv2.add_flat_segment_from_ints(a2, a2 + 3);

   typedef test_detail::seg2_vector<mi>::iterator iter_t;
   iter_t mid = segmented_stable_partition(sv2.begin(), sv2.end(), is_even());

   iter_t it = sv2.begin();
   BOOST_TEST_EQ(it->value(), 2); ++it;
   BOOST_TEST_EQ(it->value(), 4); ++it;
   BOOST_TEST_EQ(it->value(), 6); ++it;
   BOOST_TEST(it == mid);
   BOOST_TEST_EQ(it->value(), 1); ++it;
   BOOST_TEST_EQ(it->value(), 3); ++it;
   BOOST_TEST_EQ(it->value(), 5);
}

// Stable-partitions a sub-range whose segmentation shape is dictated by a
// branch spec, so that every level of the recursive dispatch is exercised on
// its single-segment path, on its multi-segment path and on the multi-segment
// path with empty segments interleaved.  The whole resulting sequence is
// compared against a naive stable partition of a flat copy taken beforehand,
// which pins the relative order within each side as well as the partition
// point, and the guard just past the end must survive untouched.
struct stable_partition_shape_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      const boost::container::vector<int> before = test_detail::flatten_n_ints(c, n);
      boost::container::vector<int> expected;
      expected.reserve(before.size());
      for(std::size_t i = 0; i != before.size(); ++i)
         if(before[i] % 2 == 0) expected.push_back(before[i]);
      const std::size_t ntrue = expected.size();
      for(std::size_t i = 0; i != before.size(); ++i)
         if(before[i] % 2 != 0) expected.push_back(before[i]);

      const iter_t mid = segmented_stable_partition(first, last, is_even());
      BOOST_TEST(mid == test_detail::iter_at(c, ntrue));

      const boost::container::vector<int> after = test_detail::flatten_n_ints(c, n);
      BOOST_TEST_EQ(after.size(), expected.size());
      for(std::size_t i = 0; i != after.size(); ++i)
         BOOST_TEST_EQ(after[i], expected[i]);

      BOOST_TEST(test_detail::filler_intact(c, n, -999));
      BOOST_TEST(spec != 0);
   }
};

void test_stable_partition_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];

      //Alternating parities.  The filler is odd, so an algorithm that reaches
      //past the end sees an element it would have to move.
      for(int i = 0; i != 16; ++i)
         vals[i] = i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_shape_check());

      //Nothing to move: already partitioned, all true, all false.
      for(int i = 0; i != 16; ++i)
         vals[i] = 2*i + 2;
      test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_shape_check());
      for(int i = 0; i != 16; ++i)
         vals[i] = 2*i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_shape_check());
      for(int i = 0; i != 16; ++i)
         vals[i] = (i < 8) ? 2*i + 2 : 2*i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_shape_check());

      //Fully reversed: every odd precedes every even.
      for(int i = 0; i != 16; ++i)
         vals[i] = (i < 8) ? 2*i + 1 : 2*i + 2;
      test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_shape_check());

      //A single element on one side of the predicate, at each position in turn.
      for(std::size_t p = 0; p != n; ++p) {
         for(std::size_t i = 0; i != 16u; ++i)
            vals[i] = (i == p) ? int(2*i + 2) : int(2*i + 1);
         test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_shape_check());

         for(std::size_t i = 0; i != 16u; ++i)
            vals[i] = (i == p) ? int(2*i + 1) : int(2*i + 2);
         test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_shape_check());
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
// Predicate application count.
//
// [alg.partitions] mandates that stable_partition performs "Exactly N
// applications of the predicate", N = last - first, however much extra memory
// it uses and however the range is segmented.
//////////////////////////////////////////////////////////////////////////////

struct stable_partition_count_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      test_detail::op_counter calls;
      segmented_stable_partition(c.begin(), test_detail::iter_at(c, n),
                                 test_detail::counting_pred(calls, is_even()));

      BOOST_TEST_EQ(calls.n, n);
      BOOST_TEST(test_detail::filler_intact(c, n, -999));
      BOOST_TEST(spec != 0);
   }
};

void test_stable_partition_predicate_count()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];

      for(int i = 0; i != 16; ++i)
         vals[i] = i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_count_check());

      for(int i = 0; i != 16; ++i)
         vals[i] = 2*i + 2;
      test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_count_check());
      for(int i = 0; i != 16; ++i)
         vals[i] = 2*i + 1;
      test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_count_check());

      for(int i = 0; i != 16; ++i)
         vals[i] = (i < 8) ? 2*i + 1 : 2*i + 2;
      test_detail::for_each_shape_all<int>(vals, n, -999, stable_partition_count_check());
   }
}

int main()
{
   test_stable_partition_shape_matrix();
   test_stable_partition_basic();
   test_stable_partition_empty();
   test_stable_partition_all_true();
   test_stable_partition_all_false();
   test_stable_partition_non_segmented();
   test_stable_partition_movable_seg();
   test_stable_partition_movable_seg2();
   test_stable_partition_predicate_count();
   return boost::report_errors();
}
