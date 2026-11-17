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
#include <boost/container/detail/transform_iterator.hpp>
#include <boost/container/new_allocator.hpp>
#include <boost/container/vector.hpp>
#include "lightweight_test.hpp"
#include "iterator_noexcept_utils.hpp"

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

void test_constant_iterator()
{
   const int x = 7;
   const std::ptrdiff_t n = 3;
   test_counting_iterator(constant_iterator<int>(x, std::size_t(n)), n);
   BOOST_TEST(*constant_iterator<int>(x, std::size_t(n)) == 7);
}

void test_emplace_iterator()
{
   typedef emplace_functor_type<int>::type functor_t;
   typedef emplace_iterator<int, functor_t> emplace_it_t;
   int x = 7;
   functor_t f(boost::move(x));
   test_counting_iterator(emplace_it_t(f), 1);

   //Both construction paths forward the stored argument
   new_allocator<int> a;
   int v = 0;
   emplace_it_t(f).construct_in_place(a, &v);
   BOOST_TEST(v == 7);
   v = 0;
   emplace_it_t(f).assign_in_place(&v);
   BOOST_TEST(v == 7);
}

struct twice
{
   typedef int result_type;
   int operator()(int i) const {  return 2*i;  }
};

void test_transform_iterator()
{
   typedef vector<int>::iterator vit_t;
   typedef transform_iterator<vit_t, twice> it_t;
   vector<int> v;
   for(int i = 0; i != 4; ++i)
      v.push_back(i);
   const it_t b = make_transform_iterator(v.begin(), twice());
   const it_t e = make_transform_iterator(v.end(), twice());
   BOOST_TEST(e - b == 4);
   BOOST_TEST(b - e == -4);
   BOOST_TEST(b + 4 == e);
   BOOST_TEST(e - 4 == b);
   BOOST_TEST(*(b + 3) == 6);
}

//The operations of the insertion proxy iterators cannot throw
#if !defined(BOOST_NO_CXX11_NOEXCEPT)

struct noexcept_emplace_functor_t
{
   template<class Allocator>
   void operator()(Allocator &, int *);
   template<class DestIt>
   void operator()(DestIt);
};

typedef boost::container::constant_iterator<int>                                       noexcept_constant_it;
typedef boost::container::value_init_construct_iterator<int>                           noexcept_value_init_it;
typedef boost::container::default_init_construct_iterator<int>                         noexcept_default_init_it;
typedef boost::container::emplace_iterator<int, noexcept_emplace_functor_t>            noexcept_emplace_it;

BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::counting_ops_are_nothrow<noexcept_constant_it>::value));
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::forward_ops_are_nothrow<noexcept_constant_it, const int&>::value));
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::random_access_ops_are_nothrow<noexcept_constant_it, const int&, std::ptrdiff_t>::value));
BOOST_CONTAINER_STATIC_ASSERT((noexcept(noexcept_constant_it(boost::move_detail::declval<const int&>(), std::size_t()))));
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::counting_ops_are_nothrow<noexcept_value_init_it>::value));
BOOST_CONTAINER_STATIC_ASSERT((noexcept(noexcept_value_init_it(std::size_t()))));
BOOST_CONTAINER_STATIC_ASSERT((noexcept(*boost::move_detail::declval<const noexcept_value_init_it&>())));
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::counting_ops_are_nothrow<noexcept_default_init_it>::value));
BOOST_CONTAINER_STATIC_ASSERT((noexcept(noexcept_default_init_it(std::size_t()))));
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::counting_ops_are_nothrow<noexcept_emplace_it>::value));
BOOST_CONTAINER_STATIC_ASSERT((noexcept(noexcept_emplace_it(boost::move_detail::declval<noexcept_emplace_functor_t&>()))));

#endif   //#if !defined(BOOST_NO_CXX11_NOEXCEPT)

int main()
{
   test_constant_iterator();
   test_counting_iterator(value_init_construct_iterator<int>(3u), 3);
   test_counting_iterator(default_init_construct_iterator<int>(3u), 3);
   test_emplace_iterator();
   test_transform_iterator();
   return ::boost::report_errors();
}
