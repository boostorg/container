//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/nest.hpp>
#include <boost/core/lightweight_test.hpp>
#include <algorithm>
#include <functional>

using namespace boost::container;

void test_default_construction()
{
   nest<int> h;
   BOOST_TEST(h.empty());
   BOOST_TEST_EQ(h.size(), 0u);
   BOOST_TEST(h.begin() == h.end());
}

void test_count_construction()
{
   nest<int> h(5);
   BOOST_TEST_EQ(h.size(), 5u);
   // value-initialized ints should be 0
   for(nest<int>::const_iterator it = h.begin(); it != h.end(); ++it) {
      BOOST_TEST_EQ(*it, 0);
   }
}

void test_count_value_construction()
{
   nest<int> h(3, 42);
   BOOST_TEST_EQ(h.size(), 3u);
   for(nest<int>::const_iterator it = h.begin(); it != h.end(); ++it) {
      BOOST_TEST_EQ(*it, 42);
   }
}

void test_range_construction()
{
   int arr[] = {1, 2, 3, 4, 5};
   nest<int> h(arr, arr + 5);
   BOOST_TEST_EQ(h.size(), 5u);
}

void test_copy_construction()
{
   nest<int> h1(3, 7);
   nest<int> h2(h1);
   BOOST_TEST_EQ(h2.size(), 3u);
   for(nest<int>::const_iterator it = h2.begin(); it != h2.end(); ++it) {
      BOOST_TEST_EQ(*it, 7);
   }
}

void test_move_construction()
{
   nest<int> h1(3, 7);
   nest<int> h2(boost::move(h1));
   BOOST_TEST_EQ(h2.size(), 3u);
   BOOST_TEST(h1.empty());
}

void test_insert_erase()
{
   nest<int> h;
   nest<int>::iterator it1 = h.insert(10);
   nest<int>::iterator it2 = h.insert(20);
   nest<int>::iterator it3 = h.insert(30);
   BOOST_TEST_EQ(h.size(), 3u);

   h.erase(it2);
   BOOST_TEST_EQ(h.size(), 2u);

   h.erase(it1);
   BOOST_TEST_EQ(h.size(), 1u);

   h.erase(it3);
   BOOST_TEST(h.empty());
}

void test_emplace()
{
   nest<int> h;
   nest<int>::iterator it = h.emplace(42);
   BOOST_TEST_EQ(*it, 42);
   BOOST_TEST_EQ(h.size(), 1u);
}

void test_assign()
{
   nest<int> h(5, 1);
   h.assign(3u, 42);
   BOOST_TEST_EQ(h.size(), 3u);
   for(nest<int>::const_iterator it = h.begin(); it != h.end(); ++it) {
      BOOST_TEST_EQ(*it, 42);
   }
}

void test_copy_assignment()
{
   nest<int> h1(3, 7);
   nest<int> h2;
   h2 = h1;
   BOOST_TEST_EQ(h2.size(), 3u);
}

void test_move_assignment()
{
   nest<int> h1(3, 7);
   nest<int> h2;
   h2 = boost::move(h1);
   BOOST_TEST_EQ(h2.size(), 3u);
   BOOST_TEST(h1.empty());
}

void test_swap()
{
   nest<int> h1(3, 1);
   nest<int> h2(5, 2);
   h1.swap(h2);
   BOOST_TEST_EQ(h1.size(), 5u);
   BOOST_TEST_EQ(h2.size(), 3u);
}

void test_clear()
{
   nest<int> h(10, 5);
   BOOST_TEST_EQ(h.size(), 10u);
   h.clear();
   BOOST_TEST(h.empty());
}

void test_iterators()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(3);

   int count = 0;
   for(nest<int>::iterator it = h.begin(); it != h.end(); ++it) {
      ++count;
   }
   BOOST_TEST_EQ(count, 3);

   count = 0;
   for(nest<int>::const_iterator it = h.cbegin(); it != h.cend(); ++it) {
      ++count;
   }
   BOOST_TEST_EQ(count, 3);
}

void test_reverse_iterators()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(3);

   int count = 0;
   for(nest<int>::reverse_iterator it = h.rbegin(); it != h.rend(); ++it) {
      ++count;
   }
   BOOST_TEST_EQ(count, 3);
}

void test_capacity()
{
   nest<int> h;
   BOOST_TEST_EQ(h.capacity(), 0u);
   h.reserve(100);
   BOOST_TEST(h.capacity() >= 100u);
   h.trim_capacity();
   BOOST_TEST_EQ(h.capacity(), 0u);
}

void test_sort()
{
   nest<int> h;
   h.insert(30);
   h.insert(10);
   h.insert(20);
   h.sort();
   nest<int>::const_iterator it = h.begin();
   BOOST_TEST_EQ(*it, 10); ++it;
   BOOST_TEST_EQ(*it, 20); ++it;
   BOOST_TEST_EQ(*it, 30);
}

void test_unique()
{
   nest<int> h;
   h.insert(1);
   h.insert(1);
   h.insert(2);
   h.insert(2);
   h.insert(3);
   h.sort();
   nest<int>::size_type removed = h.unique();
   BOOST_TEST_EQ(removed, 2u);
   BOOST_TEST_EQ(h.size(), 3u);
}

struct less_3
   : public std::less<int>
{
   less_3(int val)
      : std::less<int>()
      , val_(val)
   {}

   template<class T>
   bool operator() (const T & val) const
   {
      return std::less<int>::operator()(val, val_);
   }

   int val_;
};

void test_erase_if()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(3);
   h.insert(4);
   h.insert(5);
   nest<int>::size_type removed = erase_if(h,less_3(3));
   BOOST_TEST_EQ(removed, 2u);
   BOOST_TEST_EQ(h.size(), 3u);
}

void test_erase_value()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(1);
   h.insert(3);
   nest<int>::size_type removed = erase(h, 1);
   BOOST_TEST_EQ(removed, 2u);
   BOOST_TEST_EQ(h.size(), 2u);
}

void test_splice()
{
   nest<int> h1(3, 1);
   nest<int> h2(2, 2);
   h1.splice(h2);
   BOOST_TEST_EQ(h1.size(), 5u);
   BOOST_TEST(h2.empty());
}

void test_large_insert_erase()
{
   nest<int> h;
   // Insert many elements
   for(int i = 0; i < 1000; ++i) {
      h.insert(i);
   }
   BOOST_TEST_EQ(h.size(), 1000u);

   // Erase all elements one by one
   while(!h.empty()) {
      h.erase(h.begin());
   }
   BOOST_TEST(h.empty());
}

void test_shrink_to_fit()
{
   nest<int> h;
   for(int i = 0; i < 100; ++i) {
      h.insert(i);
   }
   // Erase half
   int count = 0;
   nest<int>::iterator it = h.begin();
   while(it != h.end()) {
      if(count % 2 == 0) {
         it = h.erase(it);
      } else {
         ++it;
      }
      ++count;
   }
   nest<int>::size_type cap_before = h.capacity();
   h.shrink_to_fit();
   BOOST_TEST(h.capacity() <= cap_before);
}

#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
void test_initializer_list()
{
   nest<int> h = {1, 2, 3, 4, 5};
   BOOST_TEST_EQ(h.size(), 5u);
}
#endif

int main()
{
   test_default_construction();
   test_count_construction();
   test_count_value_construction();
   test_range_construction();
   test_copy_construction();
   test_move_construction();
   test_insert_erase();
   test_emplace();
   test_assign();
   test_copy_assignment();
   test_move_assignment();
   test_swap();
   test_clear();
   test_iterators();
   test_reverse_iterators();
   test_capacity();
   test_sort();
   test_unique();
   test_erase_if();
   test_erase_value();
   test_splice();
   test_large_insert_erase();
   test_shrink_to_fit();
   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   test_initializer_list();
   #endif
   return boost::report_errors();
}
