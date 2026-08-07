//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_partition.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct is_even
{
   bool operator()(int v) const { return v % 2 == 0; }
   bool operator()(const test_detail::movable_int& v) const { return v.value() % 2 == 0; }
};

void test_partition_basic()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6, 7};
   int a3[] = {8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 4);
   sv.add_segment_range(a3, a3 + 2);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t mid = segmented_partition(sv.begin(), sv.end(), is_even());

   for(iter_t it = sv.begin(); it != mid; ++it)
      BOOST_TEST(*it % 2 == 0);
   for(iter_t it = mid; it != sv.end(); ++it)
      BOOST_TEST(*it % 2 != 0);

   int even_count = 0;
   for(iter_t it = sv.begin(); it != mid; ++it)
      ++even_count;
   BOOST_TEST_EQ(even_count, 4);
}

void test_partition_empty()
{
   test_detail::seg_vector<int> sv;
   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t mid = segmented_partition(sv.begin(), sv.end(), is_even());
   BOOST_TEST(mid == sv.end());
}

void test_partition_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(5); v.push_back(2); v.push_back(7); v.push_back(4); v.push_back(1);

   boost::container::vector<int>::iterator mid = segmented_partition(v.begin(), v.end(), is_even());

   for(boost::container::vector<int>::iterator it = v.begin(); it != mid; ++it)
      BOOST_TEST(*it % 2 == 0);
   for(boost::container::vector<int>::iterator it = mid; it != v.end(); ++it)
      BOOST_TEST(*it % 2 != 0);

   int even_count = 0;
   for(boost::container::vector<int>::iterator it = v.begin(); it != mid; ++it)
      ++even_count;
   BOOST_TEST_EQ(even_count, 2);
}

void test_partition_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6, 7};
   int a3[] = {8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 4);
   sv.add_segment_range(a3, a3 + 2);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   iter_t mid = segmented_partition(sv.begin(), test_detail::make_sentinel(sv.end()), is_even());

   for(iter_t it = sv.begin(); it != mid; ++it)
      BOOST_TEST(*it % 2 == 0);
   for(iter_t it = mid; it != sv.end(); ++it)
      BOOST_TEST(*it % 2 != 0);

   int even_count = 0;
   for(iter_t it = sv.begin(); it != mid; ++it)
      ++even_count;
   BOOST_TEST_EQ(even_count, 4);
}

void test_partition_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(5); v.push_back(2); v.push_back(7); v.push_back(4); v.push_back(1);

   boost::container::vector<int>::iterator mid = segmented_partition(v.begin(), test_detail::make_sentinel(v.end()), is_even());

   for(boost::container::vector<int>::iterator it = v.begin(); it != mid; ++it)
      BOOST_TEST(*it % 2 == 0);
   for(boost::container::vector<int>::iterator it = mid; it != v.end(); ++it)
      BOOST_TEST(*it % 2 != 0);

   int even_count = 0;
   for(boost::container::vector<int>::iterator it = v.begin(); it != mid; ++it)
      ++even_count;
   BOOST_TEST_EQ(even_count, 2);
}

void test_partition_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);

   typedef test_detail::seg2_vector<int>::iterator iter_t;
   iter_t mid = segmented_partition(sv2.begin(), sv2.end(), is_even());

   for(iter_t it = sv2.begin(); it != mid; ++it)
      BOOST_TEST(*it % 2 == 0);
   for(iter_t it = mid; it != sv2.end(); ++it)
      BOOST_TEST(*it % 2 != 0);

   int even_count = 0;
   for(iter_t it = sv2.begin(); it != mid; ++it)
      ++even_count;
   BOOST_TEST_EQ(even_count, 3);
}

void test_partition_movable_seg()
{
   typedef test_detail::movable_int mi;
   test_detail::seg_vector<mi> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6, 7};
   int a3[] = {8, 9};
   sv.add_segment_from_ints(a1, a1 + 3);
   sv.add_segment_from_ints(a2, a2 + 4);
   sv.add_segment_from_ints(a3, a3 + 2);

   typedef test_detail::seg_vector<mi>::iterator iter_t;
   iter_t mid = segmented_partition(sv.begin(), sv.end(), is_even());

   for(iter_t it = sv.begin(); it != mid; ++it)
      BOOST_TEST(it->value() % 2 == 0);
   for(iter_t it = mid; it != sv.end(); ++it)
      BOOST_TEST(it->value() % 2 != 0);

   int even_count = 0;
   for(iter_t it = sv.begin(); it != mid; ++it)
      ++even_count;
   BOOST_TEST_EQ(even_count, 4);
}

void test_partition_movable_seg2()
{
   typedef test_detail::movable_int mi;
   test_detail::seg2_vector<mi> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6};
   sv2.add_flat_segment_from_ints(a1, a1 + 3);
   sv2.add_flat_segment_from_ints(a2, a2 + 3);

   typedef test_detail::seg2_vector<mi>::iterator iter_t;
   iter_t mid = segmented_partition(sv2.begin(), sv2.end(), is_even());

   for(iter_t it = sv2.begin(); it != mid; ++it)
      BOOST_TEST(it->value() % 2 == 0);
   for(iter_t it = mid; it != sv2.end(); ++it)
      BOOST_TEST(it->value() % 2 != 0);

   int even_count = 0;
   for(iter_t it = sv2.begin(); it != mid; ++it)
      ++even_count;
   BOOST_TEST_EQ(even_count, 3);
}

//Checks that [first, mid) holds every even and [mid, last) every odd value of
//1..6, each exactly once, i.e. that the partition neither lost nor duplicated
//an element of the range.
template<class Iter>
void check_partition_of_1_to_6(Iter first, Iter mid, Iter last)
{
   int seen[7] = {0, 0, 0, 0, 0, 0, 0};
   for(Iter it = first; it != mid; ++it) {
      BOOST_TEST(*it % 2 == 0);
      ++seen[*it];
   }
   for(Iter it = mid; it != last; ++it) {
      BOOST_TEST(*it % 2 != 0);
      ++seen[*it];
   }
   for(int i = 1; i <= 6; ++i)
      BOOST_TEST_EQ(seen[i], 1);
}

// Partitions a sub-range whose segmentation shape is dictated by a branch
// spec, so that every level of the recursive dispatch is exercised on its
// single-segment path, on its multi-segment path and on the multi-segment path
// with empty segments interleaved.  Besides the partition point and the two
// sides, the check verifies element conservation: every value present before
// the call is present exactly as many times after it, so a lost or duplicated
// element is caught rather than only a wrong partition point.  The guard just
// past the end must also survive, since partition writes.
struct partition_shape_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      const boost::container::vector<int> before = test_detail::flatten_n_ints(c, n);
      std::size_t ntrue = 0;
      for(std::size_t i = 0; i != before.size(); ++i)
         if(before[i] % 2 == 0) ++ntrue;

      const iter_t mid = segmented_partition(first, last, is_even());
      BOOST_TEST(mid == test_detail::iter_at(c, ntrue));

      const boost::container::vector<int> after = test_detail::flatten_n_ints(c, n);
      BOOST_TEST_EQ(after.size(), before.size());
      for(std::size_t i = 0; i != after.size(); ++i)
         BOOST_TEST((after[i] % 2 == 0) == (i < ntrue));

      //Element conservation: same multiset before and after.
      for(std::size_t i = 0; i != before.size(); ++i) {
         std::size_t nb = 0, na = 0;
         for(std::size_t j = 0; j != before.size(); ++j) {
            if(before[j] == before[i]) ++nb;
            if(after[j]  == before[i]) ++na;
         }
         BOOST_TEST_EQ(na, nb);
      }

      BOOST_TEST(test_detail::filler_intact(c, n, -999));
      BOOST_TEST(spec != 0);
   }
};

//The filler is odd, so an algorithm that reaches past the end sees an element
//that fails the predicate and would have to move it.
template<class F>
void run_partition_shapes(const int* vals, std::size_t n, F f)
{
   test_detail::for_each_shape_all<int>(vals, n, -999, f);
   //Forward iterators reach a separate segmented implementation.
   test_detail::for_each_shape_all_fwd<int>(vals, n, -999, f);
}

void test_partition_shape_matrix()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];

      //Alternating parities.
      for(int i = 0; i != 16; ++i)
         vals[i] = i + 1;
      run_partition_shapes(vals, n, partition_shape_check());

      //Nothing to move: already partitioned, all true, all false.
      for(int i = 0; i != 16; ++i)
         vals[i] = 2*i + 2;
      run_partition_shapes(vals, n, partition_shape_check());
      for(int i = 0; i != 16; ++i)
         vals[i] = 2*i + 1;
      run_partition_shapes(vals, n, partition_shape_check());
      for(int i = 0; i != 16; ++i)
         vals[i] = (i < 8) ? 2*i + 2 : 2*i + 1;
      run_partition_shapes(vals, n, partition_shape_check());

      //Fully reversed: every odd precedes every even.
      for(int i = 0; i != 16; ++i)
         vals[i] = (i < 8) ? 2*i + 1 : 2*i + 2;
      run_partition_shapes(vals, n, partition_shape_check());

      //A single element on one side of the predicate, at each position in
      //turn, so that the elements to move all sit in an earlier segment than
      //the last one walked.
      for(std::size_t p = 0; p != n; ++p) {
         for(std::size_t i = 0; i != 16u; ++i)
            vals[i] = (i == p) ? int(2*i + 2) : int(2*i + 1);
         run_partition_shapes(vals, n, partition_shape_check());

         for(std::size_t i = 0; i != 16u; ++i)
            vals[i] = (i == p) ? int(2*i + 1) : int(2*i + 2);
         run_partition_shapes(vals, n, partition_shape_check());
      }
   }
}

void test_partition_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {98, 1, 2, 3, 4, 5, 6, 99};
   sv.add_segment_range(a, a + 8);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   const iter_t first = test_detail::iter_at(sv, 1);
   const iter_t last  = test_detail::iter_at(sv, 7);

   const iter_t mid = segmented_partition(first, test_detail::make_sentinel(last), is_even());
   BOOST_TEST(mid == test_detail::iter_at(sv, 4));
   check_partition_of_1_to_6(first, mid, last);
   BOOST_TEST_EQ(*sv.begin(), 98);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 7), 99);
}

void test_partition_forward_matches_in_earlier_segment()
{
   typedef test_detail::seg_vector<int, std::forward_iterator_tag> fwd_seg_t;
   typedef fwd_seg_t::iterator iter_t;

   //Every element satisfying the predicate lives in an earlier segment; the
   //last segment walked contributes nothing to the partition point.
   fwd_seg_t sv;
   int a1[] = {1, 2, 4};
   int a2[] = {3, 5, 6, 99};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 4);

   const iter_t first = sv.begin();
   const iter_t last  = test_detail::iter_at(sv, 5);

   const iter_t mid = segmented_partition(first, last, is_even());
   BOOST_TEST(mid == test_detail::iter_at(sv, 2));
   for(iter_t it = first; it != mid; ++it)
      BOOST_TEST(*it % 2 == 0);
   for(iter_t it = mid; it != last; ++it)
      BOOST_TEST(*it % 2 != 0);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 5), 6);
   BOOST_TEST_EQ(*test_detail::iter_at(sv, 6), 99);
}

//////////////////////////////////////////////////////////////////////////////
// Predicate application count.
//
// [alg.partitions] mandates that partition performs "Exactly N applications of
// the predicate", N = last - first, whichever iterator category is used, so
// neither the two-ended bidirectional scan nor the forward one may retest an
// element when it crosses a segment boundary.
//////////////////////////////////////////////////////////////////////////////

struct partition_count_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      test_detail::op_counter calls;
      segmented_partition(c.begin(), test_detail::iter_at(c, n),
                          test_detail::counting_pred(calls, is_even()));

      BOOST_TEST_EQ(calls.n, n);
      BOOST_TEST(test_detail::filler_intact(c, n, -999));
      BOOST_TEST(spec != 0);
   }
};

void test_partition_predicate_count()
{
   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      int vals[16];

      for(int i = 0; i != 16; ++i)
         vals[i] = i + 1;
      run_partition_shapes(vals, n, partition_count_check());

      for(int i = 0; i != 16; ++i)
         vals[i] = 2*i + 2;
      run_partition_shapes(vals, n, partition_count_check());
      for(int i = 0; i != 16; ++i)
         vals[i] = 2*i + 1;
      run_partition_shapes(vals, n, partition_count_check());

      for(int i = 0; i != 16; ++i)
         vals[i] = (i < 8) ? 2*i + 1 : 2*i + 2;
      run_partition_shapes(vals, n, partition_count_check());
   }
}

int main()
{
   test_partition_shape_matrix();
   test_partition_basic();
   test_partition_empty();
   test_partition_non_segmented();
   test_partition_sentinel_segmented();
   test_partition_sentinel_non_segmented();
   test_partition_seg2();
   test_partition_movable_seg();
   test_partition_movable_seg2();
   test_partition_single_segment_sentinel();
   test_partition_forward_matches_in_earlier_segment();
   test_partition_predicate_count();
   return boost::report_errors();
}
