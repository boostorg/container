//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/segmented_shuffle.hpp>
#include <boost/core/lightweight_test.hpp>
#include "segmented_test_helper.hpp"
#include <vector>
#include <algorithm>

using namespace boost::container;

struct simple_rng {
   unsigned long state;
   simple_rng(unsigned long seed) : state(seed) {}
   unsigned long operator()() { state = state * 1103515245u + 12345u; return state; }
};

void test_shuffle_is_permutation()
{
   std::vector<int> v;
   for(int i = 0; i < 10; ++i)
      v.push_back(i);

   std::vector<int> original(v);
   simple_rng rng(42);
   segmented_shuffle(v.begin(), v.end(), rng);

   // Same size
   BOOST_TEST_EQ(v.size(), original.size());

   // Same elements (sort both and compare)
   std::vector<int> sorted_v(v);
   std::vector<int> sorted_orig(original);
   std::sort(sorted_v.begin(), sorted_v.end());
   std::sort(sorted_orig.begin(), sorted_orig.end());
   for(std::size_t i = 0; i < sorted_v.size(); ++i)
      BOOST_TEST_EQ(sorted_v[i], sorted_orig[i]);

   // Check that the order actually changed (extremely unlikely to stay the same)
   bool changed = false;
   for(std::size_t i = 0; i < v.size(); ++i) {
      if(v[i] != original[i]) { changed = true; break; }
   }
   BOOST_TEST(changed);
}

void test_shuffle_empty()
{
   std::vector<int> v;
   simple_rng rng(1);
   segmented_shuffle(v.begin(), v.end(), rng);
   BOOST_TEST(v.empty());
}

void test_shuffle_single()
{
   std::vector<int> v;
   v.push_back(42);
   simple_rng rng(1);
   segmented_shuffle(v.begin(), v.end(), rng);
   BOOST_TEST_EQ(v[0], 42);
}

void test_shuffle_sized_sentinel()
{
   std::vector<int> v;
   for(int i = 0; i < 10; ++i)
      v.push_back(i);

   std::vector<int> original(v);
   simple_rng rng(42);
   segmented_shuffle(v.begin(), test_detail::make_sized_sentinel(v.end()), rng);

   BOOST_TEST_EQ(v.size(), original.size());

   std::vector<int> sorted_v(v);
   std::vector<int> sorted_orig(original);
   std::sort(sorted_v.begin(), sorted_v.end());
   std::sort(sorted_orig.begin(), sorted_orig.end());
   for(std::size_t i = 0; i < sorted_v.size(); ++i)
      BOOST_TEST_EQ(sorted_v[i], sorted_orig[i]);

   bool changed = false;
   for(std::size_t i = 0; i < v.size(); ++i) {
      if(v[i] != original[i]) { changed = true; break; }
   }
   BOOST_TEST(changed);
}

int main()
{
   test_shuffle_is_permutation();
   test_shuffle_empty();
   test_shuffle_single();
   test_shuffle_sized_sentinel();
   return boost::report_errors();
}
