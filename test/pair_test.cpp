//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2011-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#include <boost/container/detail/pair.hpp>
#include "movable_int.hpp"
#include "emplace_test.hpp"
#include<boost/move/utility_core.hpp>
#include<boost/move/detail/fwd_macros.hpp>
#include "lightweight_test.hpp"

//non_copymovable_int
//copyable_int
//movable_int
//movable_and_copyable_int


#include <boost/tuple/tuple.hpp>

#if !defined(BOOST_NO_CXX11_HDR_TUPLE) || (defined(BOOST_MSVC) && (BOOST_MSVC == 1700 || BOOST_MSVC == 1600))
#define BOOST_CONTAINER_PAIR_TEST_HAS_HEADER_TUPLE
#endif

#if defined(BOOST_CONTAINER_PAIR_TEST_HAS_HEADER_TUPLE)
#include <tuple>
#endif

using namespace ::boost::container;

//Can be assigned from an int, but not constructed from it
struct assign_only
{
   assign_only() : value(0) {}
   assign_only &operator=(int v) {  value = v;  return *this;  }
   int value;
};

//Assignment from a pair with only one equal member type assigns each member
void test_heterogeneous_assignment()
{
   {
      dtl::pair<int, assign_only> p;
      const dtl::pair<int, int> q(1, 7);
      p = q;
      BOOST_TEST(p.first == 1 && p.second.value == 7);
      p = dtl::pair<int, int>(2, 8);
      BOOST_TEST(p.first == 2 && p.second.value == 8);
   }
   {
      dtl::pair<assign_only, int> p;
      const dtl::pair<int, int> q(3, 9);
      p = q;
      BOOST_TEST(p.first.value == 3 && p.second == 9);
      p = dtl::pair<int, int>(4, 10);
      BOOST_TEST(p.first.value == 4 && p.second == 10);
   }
}

//A pair with a reference member assigns through the reference
void test_reference_member_assignment()
{
   #if defined(BOOST_CONTAINER_HAS_CONDITIONALLY_TRIVIAL_SPECIAL_MEMBERS)
   int c = 3, d = 4, e = 5;
   dtl::pair<int&, int> p1(c, 10), p2(d, 20), p3(e, 30);
   p1 = p2;
   BOOST_TEST(c == 4 && d == 4 && p1.second == 20 && &p1.first == &c);
   p1 = boost::move(p3);
   BOOST_TEST(c == 5 && e == 5 && p1.second == 30 && &p1.first == &c);

   dtl::pair<int, int&> q1(10, c), q2(20, d);
   q1 = q2;
   BOOST_TEST(c == 4 && q1.first == 20 && &q1.second == &c);

   //Without reference members the assignments stay trivial
   BOOST_CONTAINER_STATIC_ASSERT((boost::move_detail::is_trivially_copyable< dtl::pair<int, int> >::value));
   #endif
}

int main ()
{
   {
      dtl::pair<test::non_copymovable_int, test::non_copymovable_int> p1;
      dtl::pair<test::copyable_int, test::copyable_int> p2;
      dtl::pair<test::movable_int, test::movable_int> p3;
      dtl::pair<test::movable_and_copyable_int, test::movable_and_copyable_int> p4;
   }
   {  //Constructible from two values
      dtl::pair<test::non_copymovable_int, test::non_copymovable_int> p1(1, 2);
      dtl::pair<test::copyable_int, test::copyable_int> p2(1, 2);
      dtl::pair<test::movable_int, test::movable_int> p3(1, 2);
      dtl::pair<test::movable_and_copyable_int, test::movable_and_copyable_int> p4(1, 2);
   }

   {  //Constructible from internal types
      dtl::pair<test::copyable_int, test::copyable_int> p2(test::copyable_int(1), test::copyable_int(2));
      {
         test::movable_int a(1), b(2);
         dtl::pair<test::movable_int, test::movable_int> p3(::boost::move(a), ::boost::move(b));
      }
      {
         test::movable_and_copyable_int a(1), b(2);
         dtl::pair<test::movable_and_copyable_int, test::movable_and_copyable_int> p4(::boost::move(a), ::boost::move(b));
      }
   }
   {  //piecewise construct from boost tuple
      using namespace boost::tuples;
      {
         boost::container::dtl::pair<int, float> p(piecewise_construct, tuple<>(), tuple<>());
         BOOST_TEST(p.first == 0);
         BOOST_TEST(p.second == 0.f);
      }
      {
         boost::container::dtl::pair<int, float> p(piecewise_construct, tuple<>(), tuple<float>(2.f));
         BOOST_TEST(p.first == 0);
         BOOST_TEST(p.second == 2.f);
      }
      {
         boost::container::dtl::pair<int, float> p(piecewise_construct, tuple<int>(2), tuple<float>(1.f));
         BOOST_TEST(p.first == 2);
         BOOST_TEST(p.second == 1.f);
      }
      {
         boost::container::dtl::pair
            < boost::container::dtl::pair<int, float>
            , boost::container::dtl::pair<double, char>
            > p(piecewise_construct, tuple<int, float>(3, 4.f), tuple<double, char>(8.,'a'));
         BOOST_TEST(p.first.first   == 3);
         BOOST_TEST(p.first.second  == 4.f);
         BOOST_TEST(p.second.first  == 8.);
         BOOST_TEST(p.second.second == 'a');
      }
      {
         boost::container::dtl::pair
            < tuple<int, float, double>
            , char
            > p(piecewise_construct, tuple<int, float, double>(3, 16.f, 32.), tuple<char>('b'));
         BOOST_TEST(p.first.get<0>() == 3);
         BOOST_TEST(p.first.get<1>() == 16.f);
         BOOST_TEST(p.first.get<2>() == 32.);
         BOOST_TEST(p.second == 'b');
      }
   }
   #if defined(BOOST_CONTAINER_PAIR_TEST_HAS_HEADER_TUPLE)
   {  //piecewise construct from std tuple
      using std::tuple;
      {
         boost::container::dtl::pair<int, float> p(piecewise_construct, tuple<>(), tuple<>());
         BOOST_TEST(p.first == 0);
         BOOST_TEST(p.second == 0.f);
      }
      {
         boost::container::dtl::pair<int, float> p(piecewise_construct, tuple<>(), tuple<float>(2.f));
         BOOST_TEST(p.first == 0);
         BOOST_TEST(p.second == 2.f);
      }
      {
         boost::container::dtl::pair<int, float> p(piecewise_construct, tuple<int>(2), tuple<float>(1.f));
         BOOST_TEST(p.first == 2);
         BOOST_TEST(p.second == 1.f);
      }
      {
         boost::container::dtl::pair
            < boost::container::dtl::pair<int, float>
            , boost::container::dtl::pair<double, char>
            > p(piecewise_construct, tuple<int, float>(3, 4.f), tuple<double, char>(8.,'a'));
         BOOST_TEST(p.first.first   == 3);
         BOOST_TEST(p.first.second  == 4.f);
         BOOST_TEST(p.second.first  == 8.);
         BOOST_TEST(p.second.second == 'a');
      }
      {
         boost::container::dtl::pair
            < tuple<int, float, double>
            , char
            > p(piecewise_construct, tuple<int, float, double>(3, 16.f, 32.), tuple<char>('b'));
         BOOST_TEST(std::get<0>(p.first) == 3);
         BOOST_TEST(std::get<1>(p.first) == 16.f);
         BOOST_TEST(std::get<2>(p.first) == 32.);
         BOOST_TEST(p.second == 'b');
      }
      #if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
      typedef dtl::pair<test::movable_int, test::movable_int> movable_pair_t;
      typedef dtl::pair<movable_pair_t, movable_pair_t> movable_pair_pair_t;
      test::movable_int a(1), b(2), c(3), d(4);
      movable_pair_pair_t p( piecewise_construct
                           , dtl::forward_as_tuple_impl(boost::move(a), boost::move(b))
                           , dtl::forward_as_tuple_impl(boost::move(c), boost::move(d))
                           );
      BOOST_TEST(p.first.first   == 1);
      BOOST_TEST(p.first.second  == 2);
      BOOST_TEST(p.second.first  == 3);
      BOOST_TEST(p.second.second == 4);
      #endif
   }
   #endif   //#!defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) && !defined(BOOST_NO_CXX11_HDR_TUPLE)
   test_heterogeneous_assignment();
   test_reference_member_assignment();
   return ::boost::report_errors();
}
