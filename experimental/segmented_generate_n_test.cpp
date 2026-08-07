//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_generate_n.hpp>
#include "../test/lightweight_test.hpp"
#include "segmented_test_helper.hpp"
#include <boost/container/vector.hpp>

using namespace boost::container;

struct counter_gen
{
   int n;
   counter_gen() : n(0) {}
   int operator()() { return ++n; }
};

//A count that spans several segments, with the returned iterator checked
//against the first element left untouched.  The generator is passed by value,
//so its state has to survive being carried from segment to segment.
void test_generate_n_partial()
{
   test_detail::seg_vector<int> sv;
   sv.add_segment(3, 0);
   sv.add_segment(4, 0);
   sv.add_segment(2, 0);

   test_detail::seg_vector<int>::iterator result =
      segmented_generate_n(sv.begin(), 5, counter_gen());

   test_detail::seg_vector<int>::iterator it = sv.begin();
   BOOST_TEST_EQ(*it, 1); ++it;
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 3); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 5); ++it;
   BOOST_TEST(it == result);
   BOOST_TEST_EQ(*it, 0); ++it;
   BOOST_TEST_EQ(*it, 0);
}

void test_generate_n_non_segmented()
{
   boost::container::vector<int> v(5, 0);
   boost::container::vector<int>::iterator result =
      segmented_generate_n(v.begin(), 3, counter_gen());

   BOOST_TEST(result == v.begin() + 3);
   BOOST_TEST_EQ(v[0], 1);
   BOOST_TEST_EQ(v[1], 2);
   BOOST_TEST_EQ(v[2], 3);
   BOOST_TEST_EQ(v[3], 0);
   BOOST_TEST_EQ(v[4], 0);
}

//Counts through a pointer so that the number of calls made by the algorithm
//stays observable after it returns.
struct ptr_counter_gen
{
   int* pn;
   explicit ptr_counter_gen(int* p) : pn(p) {}
   int operator()() { return ++*pn; }
};

// Generates count elements from a chosen offset into a range whose
// segmentation shape is dictated by a branch spec, so that every level of the
// recursive dispatch is exercised on its single-segment path, on its
// multi-segment path and on the multi-segment path with empty segments
// interleaved.  Sweeping both the offset and the count covers a count that
// stops short of the end of a segment, one that exactly consumes it and one
// that spans several; a count larger than the range is deliberately never
// tried, because the public API takes no end bound.
//
// The generator counts through a pointer, so the number of calls stays
// observable however many times the combinators copy it, and the values it
// produces run unbroken from 1 to count across every segment boundary.
struct generate_n_shape_check
{
   std::size_t offset;
   std::size_t count;

   generate_n_shape_check(std::size_t o, std::size_t k) : offset(o), count(k) {}

   template<class Cont>
   void operator()(Cont& c, std::size_t n, const char* spec) const
   {
      typedef typename Cont::iterator iter_t;
      const boost::container::vector<int> before = test_detail::flatten_n_ints(c, n);

      int calls = 0;
      const iter_t r =
         segmented_generate_n(test_detail::iter_at(c, offset), count, ptr_counter_gen(&calls));
      BOOST_TEST_EQ(std::size_t(calls), count);
      BOOST_TEST(r == test_detail::iter_at(c, offset + count));

      const boost::container::vector<int> after = test_detail::flatten_n_ints(c, n);
      BOOST_TEST_EQ(after.size(), before.size());
      for(std::size_t i = 0; i != after.size(); ++i) {
         const int expected = (i >= offset && i < offset + count)
                            ? int(i - offset + 1u) : before[i];
         BOOST_TEST_EQ(after[i], expected);
      }

      BOOST_TEST(test_detail::filler_intact(c, n, -999));
      BOOST_TEST(spec != 0);
   }
};

void test_generate_n_shape_matrix()
{
   //Placeholders well outside the range the generator produces, so an
   //untouched slot is never mistaken for a generated one.
   int vals[16];
   for(int i = 0; i != 16; ++i)
      vals[i] = 1000 + i;

   const std::size_t sizes[] = { 0u, 1u, 2u, 5u, 12u };
   for(std::size_t s = 0; s != sizeof(sizes)/sizeof(sizes[0]); ++s) {
      const std::size_t n = sizes[s];
      for(std::size_t offset = 0; offset <= n; ++offset)
         for(std::size_t k = 0; offset + k <= n; ++k)
            test_detail::for_each_shape_all<int>(vals, n, -999, generate_n_shape_check(offset, k));
   }
}

int main()
{
   test_generate_n_shape_matrix();
   test_generate_n_partial();
   test_generate_n_non_segmented();
   return boost::report_errors();
}
