//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_generate.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct counter
{
   int n;
   counter() : n(0) {}
   int operator()() { return ++n; }
};

void test_generate_full_range()
{
   test_detail::seg_vector<int> sv;
   sv.add_segment(3, 0);
   sv.add_segment(2, 0);
   sv.add_segment(4, 0);

   segmented_generate(sv.begin(), sv.end(), counter());

   // Counter state must be preserved across segments: 1,2,3 | 4,5 | 6,7,8,9
   test_detail::seg_vector<int>::iterator it = sv.begin();
   for(int expected = 1; it != sv.end(); ++it, ++expected)
      BOOST_TEST_EQ(*it, expected);
}

void test_generate_empty()
{
   test_detail::seg_vector<int> sv;
   segmented_generate(sv.begin(), sv.end(), counter());
   BOOST_TEST_EQ(sv.total_size(), 0u);
}

void test_generate_non_segmented()
{
   boost::container::vector<int> v(5, 0);
   segmented_generate(v.begin(), v.end(), counter());
   for(int i = 0; i < 5; ++i)
      BOOST_TEST_EQ(v[static_cast<std::size_t>(i)], i + 1);
}

void test_generate_sentinel_segmented()
{
   test_detail::seg_vector<int> sv;
   sv.add_segment(3, 0);
   sv.add_segment(2, 0);
   sv.add_segment(4, 0);

   segmented_generate(sv.begin(), test_detail::make_sentinel(sv.end()), counter());

   test_detail::seg_vector<int>::iterator it = sv.begin();
   for(int expected = 1; it != sv.end(); ++it, ++expected)
      BOOST_TEST_EQ(*it, expected);
}

void test_generate_sentinel_non_segmented()
{
   boost::container::vector<int> v(5, 0);
   segmented_generate(v.begin(), test_detail::make_sentinel(v.end()), counter());
   for(int i = 0; i < 5; ++i)
      BOOST_TEST_EQ(v[static_cast<std::size_t>(i)], i + 1);
}

void test_generate_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int z1[] = {0, 0, 0};
   int z2[] = {0, 0, 0};
   int z3[] = {0, 0, 0};
   sv2.add_flat_segment_range(z1, z1 + 3);
   sv2.add_flat_segment_range(z2, z2 + 3);
   sv2.add_flat_segment_range(z3, z3 + 3);

   segmented_generate(sv2.begin(), sv2.end(), counter());

   test_detail::seg2_vector<int>::iterator it = sv2.begin();
   for(int expected = 1; it != sv2.end(); ++it, ++expected)
      BOOST_TEST_EQ(*it, expected);
}

//Counts through a pointer so that the number of calls made by the algorithm
//stays observable after it returns.
struct ptr_counter
{
   int* pn;
   explicit ptr_counter(int* p) : pn(p) {}
   int operator()() { return ++*pn; }
};

// Generates into a sub-range whose segmentation shape is dictated by a branch
// spec, so that every level of the recursive dispatch is exercised on its
// single-segment path, on its multi-segment path and on the multi-segment path
// with empty segments interleaved.  The generator counts through a pointer, so
// the number of calls stays observable however many times the combinators copy
// it, and it is the counter running unbroken from 1 to n across every segment
// boundary that pins down the order the elements were written in.
struct generate_shape_check
{
   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const iter_t first = c.begin();
      const iter_t last  = test_detail::iter_at(c, n);

      int calls = 0;
      segmented_generate(first, last, ptr_counter(&calls));
      BOOST_TEST_EQ(std::size_t(calls), n);

      const boost::container::vector<int> after = test_detail::flatten_n_ints(c, n);
      BOOST_TEST_EQ(after.size(), n);
      for(std::size_t i = 0; i != after.size(); ++i)
         BOOST_TEST_EQ(after[i], int(i + 1u));

      BOOST_TEST(test_detail::filler_intact(c, n, -999));
      BOOST_TEST(spec != 0);
   }
};

void test_generate_shape_matrix()
{
   int vals[16] = {0};

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s)
      test_detail::for_each_shape_all<int>(vals, sizes[s], -999, generate_shape_check());
}

void test_generate_single_segment_sentinel()
{
   test_detail::seg_vector<int> sv;
   int a[] = {-1, 0, 0, 0, 0, 0, -2};
   sv.add_segment_range(a, a + 7);

   typedef test_detail::seg_vector<int>::iterator iter_t;
   int n = 0;
   segmented_generate(test_detail::iter_at(sv, 1),
                      test_detail::make_sentinel(test_detail::iter_at(sv, 6)), ptr_counter(&n));
   BOOST_TEST_EQ(n, 5);

   int expected[] = {-1, 1, 2, 3, 4, 5, -2};
   iter_t it = sv.begin();
   for(int i = 0; i < 7; ++i, ++it)
      BOOST_TEST_EQ(*it, expected[i]);
}

int main()
{
   test_generate_shape_matrix();
   test_generate_full_range();
   test_generate_empty();
   test_generate_non_segmented();
   test_generate_sentinel_segmented();
   test_generate_sentinel_non_segmented();
   test_generate_seg2();
   test_generate_single_segment_sentinel();
   return boost::report_errors();
}
