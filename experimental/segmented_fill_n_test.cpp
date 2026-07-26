//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_fill_n.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

//A count that spans several segments, with the returned iterator checked
//against the first element left untouched.
void test_fill_n_partial()
{
   test_detail::seg_vector<int> sv;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5, 6, 7};
   int a3[] = {8, 9};
   sv.add_segment_range(a1, a1 + 3);
   sv.add_segment_range(a2, a2 + 4);
   sv.add_segment_range(a3, a3 + 2);

   test_detail::seg_vector<int>::iterator result = segmented_fill_n(sv.begin(), 5, 0);

   test_detail::seg_vector<int>::iterator it = sv.begin();
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST(it == result);
   BOOST_TEST_EQ(*it, 6); ++it;
   BOOST_TEST_EQ(*it, 7); ++it;
   BOOST_TEST_EQ(*it, 8); ++it;
   BOOST_TEST_EQ(*it, 9);
}

//A count that stops short of the end of the only segment.
void test_fill_n_single_segment()
{
   test_detail::seg_vector<int> sv;
   sv.add_segment(5, 0);

   segmented_fill_n(sv.begin(), 3, 77);

   test_detail::seg_vector<int>::iterator it = sv.begin();
   BOOST_TEST_EQ(*it, 77); ++it;
   BOOST_TEST_EQ(*it, 77); ++it;
   BOOST_TEST_EQ(*it, 77); ++it;
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

void test_fill_n_non_segmented()
{
   boost::container::vector<int> v(5, 0);
   boost::container::vector<int>::iterator result = segmented_fill_n(v.begin(), 3, 7);

   BOOST_TEST(result == v.begin() + 3);
   BOOST_TEST_EQ(v[0], 7);
   BOOST_TEST_EQ(v[1], 7);
   BOOST_TEST_EQ(v[2], 7);
   BOOST_TEST_EQ(v[3], 0);
   BOOST_TEST_EQ(v[4], 0);
}

// Fills count elements from a chosen offset into a range whose segmentation
// shape is dictated by a branch spec, so that every level of the recursive
// dispatch is exercised on its single-segment path, on its multi-segment path
// and on the multi-segment path with empty segments interleaved.  Sweeping
// both the offset and the count covers a count that stops short of the end of
// a segment, one that exactly consumes it and one that spans several; a count
// larger than the range is deliberately never tried, because the public API
// takes no end bound and so has nothing to stop against.
//
// The returned iterator is compared against iter_at, which normalises by
// stepping: at an exact segment boundary that lands on the next non-empty
// segment's begin, so a return value left dangling at the old segment's end
// compares unequal.
struct fill_n_shape_check
{
   std::size_t offset;
   std::size_t count;

   fill_n_shape_check(std::size_t o, std::size_t k) : offset(o), count(k) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const boost::container::vector<int> before = test_detail::flatten_n_ints(c, n);

      const iter_t r = segmented_fill_n(test_detail::iter_at(c, offset), count, 42);
      BOOST_TEST(r == test_detail::iter_at(c, offset + count));

      const boost::container::vector<int> after = test_detail::flatten_n_ints(c, n);
      BOOST_TEST_EQ(after.size(), before.size());
      for(std::size_t i = 0; i != after.size(); ++i)
         BOOST_TEST_EQ(after[i], (i >= offset && i < offset + count) ? 42 : before[i]);

      BOOST_TEST(test_detail::filler_intact(c, n, -999));
      BOOST_TEST(spec != 0);
   }
};

void test_fill_n_shape_matrix()
{
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = i + 1;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      for(std::size_t offset = 0; offset <= n; ++offset)
         for(std::size_t k = 0; offset + k <= n; ++k)
            test_detail::for_each_shape_all<int>(vals, n, -999, fill_n_shape_check(offset, k));
   }
}

int main()
{
   test_fill_n_shape_matrix();
   test_fill_n_partial();
   test_fill_n_single_segment();
   test_fill_n_non_segmented();
   return boost::report_errors();
}
