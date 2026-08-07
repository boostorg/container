//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_replace.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

void test_replace_basic()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 1};
   int a2[] = {3, 1, 4};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   segmented_replace(sv.begin(), sv.end(), 1, 99);

   test_detail::seg_vector<int>::iterator it = sv.begin();
   BOOST_TEST_EQ(*it, 99); ++it;
   BOOST_TEST_EQ(*it, 2);  ++it;
   BOOST_TEST_EQ(*it, 99); ++it;
   BOOST_TEST_EQ(*it, 3);  ++it;
   BOOST_TEST_EQ(*it, 99); ++it;
   BOOST_TEST_EQ(*it, 4);
}

void test_replace_empty()
{
   test_detail::seg_vector<int> sv;
   segmented_replace(sv.begin(), sv.end(), 1, 2);
   BOOST_TEST_EQ(sv.total_size(), 0u);
}

void test_replace_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(2);
   v.push_back(1);
   v.push_back(3);

   segmented_replace(v.begin(), v.end(), 1, 0);
   BOOST_TEST_EQ(v[0], 0);
   BOOST_TEST_EQ(v[1], 2);
   BOOST_TEST_EQ(v[2], 0);
   BOOST_TEST_EQ(v[3], 3);
}

void test_replace_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 1};
   int a2[] = {3, 1, 4};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 3);

   segmented_replace(sv.begin(), test_detail::make_sentinel(sv.end()), 1, 99);

   test_detail::seg_vector<int>::iterator it = sv.begin();
   BOOST_TEST_EQ(*it, 99); ++it;
   BOOST_TEST_EQ(*it, 2);  ++it;
   BOOST_TEST_EQ(*it, 99); ++it;
   BOOST_TEST_EQ(*it, 3);  ++it;
   BOOST_TEST_EQ(*it, 99); ++it;
   BOOST_TEST_EQ(*it, 4);
}

void test_replace_sentinel_non_segmented()
{
   boost::container::vector<int> v;
   v.push_back(1);
   v.push_back(2);
   v.push_back(1);
   v.push_back(3);

   segmented_replace(v.begin(), test_detail::make_sentinel(v.end()), 1, 0);
   BOOST_TEST_EQ(v[0], 0);
   BOOST_TEST_EQ(v[1], 2);
   BOOST_TEST_EQ(v[2], 0);
   BOOST_TEST_EQ(v[3], 3);
}

void test_replace_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {2, 4, 2};
   int a3[] = {5, 2};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 3);
   sv2.add_flat_segment_range(a3, a3 + 2);

   segmented_replace(sv2.begin(), sv2.end(), 2, 99);

   int expected[] = {1, 99, 3, 99, 4, 99, 5, 99};
   test_detail::seg2_vector<int>::iterator it = sv2.begin();
   for(int i = 0; i < 8; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
}

// Replaces within a sub-range whose segmentation shape is dictated by a branch
// spec, so that every level of the recursive dispatch is exercised on its
// single-segment path, on its multi-segment path and on the multi-segment path
// with empty segments interleaved.  The guard element just past the end holds
// the very value being replaced, so an algorithm that writes past the end is
// caught by filler_intact rather than going unnoticed.
struct replace_shape_check
{
   int oldv;
   int newv;

   replace_shape_check(int o, int nv) : oldv(o), newv(nv) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      const boost::container::vector<int> before = test_detail::flatten_n_ints(c, n);

      segmented_replace(first, last, oldv, newv);

      const boost::container::vector<int> after = test_detail::flatten_n_ints(c, n);
      BOOST_TEST_EQ(after.size(), before.size());
      for(std::size_t i = 0; i != after.size(); ++i)
         BOOST_TEST_EQ(after[i], before[i] == oldv ? newv : before[i]);

      BOOST_TEST(test_detail::filler_intact(c, n, oldv));
      BOOST_TEST(spec != 0);
   }
};

void test_replace_shape_matrix()
{
   //Every value appears twice, so a replacement has to happen more than once
   //and, for the smaller values, in more than one segment.
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i/2 + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      //Every value present in the range plus one that is absent from it.
      for(std::size_t v = 0; v <= n/2u + 1u; ++v) {
         const int oldv = int(v);
         test_detail::for_each_shape_all<int>(vals, n, oldv, replace_shape_check(oldv, 999));
      }
   }
}

void test_replace_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {1, 2, 1, 3, 1, 4, 1};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   segmented_replace(test_detail::iter_at(sv, 1),
                     test_detail::make_sentinel(test_detail::iter_at(sv, 6)), 1, 99);

   int expected[] = {1, 2, 99, 3, 99, 4, 1};
   iter_t it = sv.begin();
   for(int i = 0; i < 7; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
}

//////////////////////////////////////////////////////////////////////////////
// Comparison count.
//
// [alg.replace] mandates "Exactly last - first applications of the
// corresponding predicate", so an element compared twice at a segment
// boundary is a conformance failure.  There is no predicate overload, so the
// count comes from the value type.
//////////////////////////////////////////////////////////////////////////////

struct replace_comparison_check
{
   int oldv;

   explicit replace_comparison_check(int o) : oldv(o) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      test_detail::counted_int_ops().reset();
      segmented_replace(c.begin(), test_detail::iter_at(c, n),
                        test_detail::counted_int(oldv), test_detail::counted_int(999));
      const std::size_t applied = test_detail::counted_int_ops().cmp;

      BOOST_TEST_EQ(applied, n);
      BOOST_TEST(spec != 0);
   }
};

void test_replace_comparison_count()
{
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i/2 + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      for(std::size_t v = 0; v <= n/2u + 1u; ++v) {
         const int oldv = int(v);
         test_detail::for_each_shape_all<test_detail::counted_int>
            (vals, n, oldv, replace_comparison_check(oldv));
      }
   }
}

int main()
{
   test_replace_shape_matrix();
   test_replace_basic();
   test_replace_empty();
   test_replace_non_segmented();
   test_replace_sentinel_segmented();
   test_replace_sentinel_non_segmented();
   test_replace_seg2();
   test_replace_single_segment_sentinel();
   test_replace_comparison_count();
   return boost::report_errors();
}
