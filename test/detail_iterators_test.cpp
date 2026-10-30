//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/detail/iterators.hpp>
#include "lightweight_test.hpp"

#include <cstddef>

using namespace boost::container;

//The counting iterators of a range of n elements: increment, decrement and
//the random access operations must agree with each other
template<class It>
void test_counting_iterator(const It b, const std::ptrdiff_t n)
{
   const std::ptrdiff_t one = 1;
   const It e = b + n;
   BOOST_TEST(e - b == n);
   BOOST_TEST(b < e);
   BOOST_TEST(e == It());

   //Prefix increment and decrement
   {
      It i = b;
      ++i;
      BOOST_TEST(i - b == one);
      --i;
      BOOST_TEST(i == b);
   }
   //Postfix increment and decrement
   {
      It i = b;
      const It i0 = i++;
      BOOST_TEST(i0 == b);
      BOOST_TEST(i == b + one);
      const It i1 = i--;
      BOOST_TEST(i1 == b + one);
      BOOST_TEST(i == b);
   }
   //Walk to the end and back
   {
      It i = b;
      for(std::ptrdiff_t k = 0; k != n; ++k)
         ++i;
      BOOST_TEST(i == e);
      for(std::ptrdiff_t k = 0; k != n; ++k)
         --i;
      BOOST_TEST(i == b);
   }
   //Decrement from the end
   {
      It i = e;
      --i;
      BOOST_TEST(e - i == one);
      BOOST_TEST(i == e - one);
      BOOST_TEST(i < e);
   }
   //Compound assignment
   {
      It i = b;
      i += n;
      BOOST_TEST(i == e);
      i -= n;
      BOOST_TEST(i == b);
   }
}

void test_repeat_iterator()
{
   int x = 7;
   const std::ptrdiff_t n = 3;
   test_counting_iterator(repeat_iterator<int>(x, std::size_t(n)), n);

   //All positions refer to the same object
   repeat_iterator<int> i(x, std::size_t(n));
   ++i;
   --i;
   BOOST_TEST(&*i == &x);
   BOOST_TEST(i[2] == 7);
}

void test_constant_iterator()
{
   const int x = 7;
   const std::ptrdiff_t n = 3;
   test_counting_iterator(constant_iterator<int>(x, std::size_t(n)), n);
   BOOST_TEST(*constant_iterator<int>(x, std::size_t(n)) == 7);
}

int main()
{
   test_constant_iterator();
   test_counting_iterator(value_init_construct_iterator<int>(3u), 3);
   test_counting_iterator(default_init_construct_iterator<int>(3u), 3);
   test_repeat_iterator();
   return ::boost::report_errors();
}
