//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_sample.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>

using namespace boost::container;

struct simple_rng {
   unsigned long state;
   simple_rng(unsigned long seed) : state(seed) {}
   unsigned long operator()() { state = state * 1103515245u + 12345u; return state; }
};

void test_sample_basic()
{
   std::vector<int> pop;
   for(int i = 0; i < 10; ++i)
      pop.push_back(i);

   std::vector<int> out(3, -1);
   simple_rng rng(42);
   std::vector<int>::iterator result = segmented_sample(
      pop.begin(), pop.end(), out.begin(), 3, rng);

   // Exactly 3 elements sampled
   BOOST_TEST_EQ(result - out.begin(), 3);

   // All sampled elements are in original range [0, 10)
   for(int i = 0; i < 3; ++i) {
      BOOST_TEST(out[std::size_t(i)] >= 0 && out[std::size_t(i)] < 10);
   }
}

void test_sample_all()
{
   std::vector<int> pop;
   for(int i = 0; i < 5; ++i)
      pop.push_back(i * 10);

   std::vector<int> out(5, -1);
   simple_rng rng(7);
   std::vector<int>::iterator result = segmented_sample(
      pop.begin(), pop.end(), out.begin(), 5, rng);

   BOOST_TEST_EQ(result - out.begin(), 5);
   // When sampling all, we get all elements
   for(int i = 0; i < 5; ++i)
      BOOST_TEST_EQ(out[std::size_t(i)], i * 10);
}

void test_sample_zero()
{
   std::vector<int> pop;
   pop.push_back(1); pop.push_back(2); pop.push_back(3);

   std::vector<int> out;
   simple_rng rng(1);
   std::vector<int>::iterator result = segmented_sample(
      pop.begin(), pop.end(), out.begin(), 0, rng);

   BOOST_TEST(result == out.begin());
}

void test_sample_empty_population()
{
   std::vector<int> pop;
   std::vector<int> out;
   simple_rng rng(1);
   std::vector<int>::iterator result = segmented_sample(
      pop.begin(), pop.end(), out.begin(), 3, rng);

   BOOST_TEST(result == out.begin());
}

void test_sample_sentinel()
{
   std::vector<int> pop;
   for(int i = 0; i < 10; ++i)
      pop.push_back(i);

   std::vector<int> out(3, -1);
   simple_rng rng(42);
   std::vector<int>::iterator result = segmented_sample(
      pop.begin(), test_detail::make_sentinel(pop.end()), out.begin(), 3, rng);

   BOOST_TEST_EQ(result - out.begin(), 3);

   for(int i = 0; i < 3; ++i) {
      BOOST_TEST(out[std::size_t(i)] >= 0 && out[std::size_t(i)] < 10);
   }
}

void test_sample_seg2()
{
   test_detail::seg2_vector<int> sv2;
   int a1[] = {1, 2, 3};
   int a2[] = {4, 5};
   int a3[] = {6, 7, 8, 9};
   sv2.add_flat_segment_range(a1, a1 + 3);
   sv2.add_flat_segment_range(a2, a2 + 2);
   sv2.add_flat_segment_range(a3, a3 + 4);

   std::vector<int> out(3, -1);
   simple_rng rng(42);
   std::vector<int>::iterator result = segmented_sample(
      sv2.begin(), sv2.end(), out.begin(), 3, rng);

   BOOST_TEST_EQ(result - out.begin(), 3);

   for(int i = 0; i < 3; ++i) {
      BOOST_TEST(out[std::size_t(i)] >= 1 && out[std::size_t(i)] <= 9);
   }
}

int main()
{
   test_sample_basic();
   test_sample_all();
   test_sample_zero();
   test_sample_empty_population();
   test_sample_sentinel();
   test_sample_seg2();
   return boost::report_errors();
}
