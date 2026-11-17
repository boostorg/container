//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_ITERATOR_NOEXCEPT_UTILS_HPP
#define BOOST_CONTAINER_TEST_ITERATOR_NOEXCEPT_UTILS_HPP

//Traits that check that the operations of an iterator cannot throw, so that
//functions that take an iterator range (for example the range functions of
//Boost.Intrusive) can be noexcept with these iterators.
//Use them in static asserts only when BOOST_NO_CXX11_NOEXCEPT is not defined.

#include <boost/config.hpp>
#include <boost/move/utility_core.hpp>
#include <cstddef>

#if !defined(BOOST_NO_CXX11_NOEXCEPT)

namespace boost {
namespace container {
namespace test {

//Declared only: a call converts the argument to T as a function parameter does
template<class T>
T noexcept_implicit_conversion(T) BOOST_NOEXCEPT;

template<class It>
struct arrow_is_nothrow
{
   static const bool value =
      noexcept(boost::move_detail::declval<It&>().operator->()) &&
      noexcept(boost::move_detail::declval<const It&>().operator->());
};

template<class T>
struct arrow_is_nothrow<T*>
{
   static const bool value = true;
};

template<class It, class Ref>
struct forward_ops_are_nothrow
{
   static const bool value =
      //Copy construction and destruction
      noexcept(It(boost::move_detail::declval<It&>())) &&
      noexcept(It(boost::move_detail::declval<const It&>())) &&
      noexcept(boost::move_detail::declval<It&>().~It()) &&
      //Comparisons on non-const and const lvalues and conversion of the result to bool
      noexcept(static_cast<bool>(boost::move_detail::declval<It&>() != boost::move_detail::declval<It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<It&>() == boost::move_detail::declval<It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() != boost::move_detail::declval<const It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() == boost::move_detail::declval<const It&>())) &&
      //Increment
      noexcept(++boost::move_detail::declval<It&>()) &&
      noexcept(boost::move_detail::declval<It&>()++) &&
      //Dereference and conversion of the result to the reference type
      noexcept(noexcept_implicit_conversion<Ref>(*boost::move_detail::declval<It&>())) &&
      noexcept(noexcept_implicit_conversion<Ref>(*boost::move_detail::declval<const It&>())) &&
      arrow_is_nothrow<It>::value;
};

template<class It>
struct bidirectional_ops_are_nothrow
{
   static const bool value =
      noexcept(--boost::move_detail::declval<It&>()) &&
      noexcept(boost::move_detail::declval<It&>()--);
};

template<class It, class Ref, class Diff>
struct random_access_ops_are_nothrow
{
   static const bool value =
      noexcept(boost::move_detail::declval<It&>() += Diff()) &&
      noexcept(boost::move_detail::declval<It&>() -= Diff()) &&
      noexcept(It(boost::move_detail::declval<const It&>() + Diff())) &&
      noexcept(It(Diff() + boost::move_detail::declval<const It&>())) &&
      noexcept(It(boost::move_detail::declval<const It&>() - Diff())) &&
      noexcept(Diff(boost::move_detail::declval<const It&>() - boost::move_detail::declval<const It&>())) &&
      noexcept(noexcept_implicit_conversion<Ref>(boost::move_detail::declval<const It&>()[Diff()])) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() <  boost::move_detail::declval<const It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() <= boost::move_detail::declval<const It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() >  boost::move_detail::declval<const It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() >= boost::move_detail::declval<const It&>()));
};

//Conversion from iterator to const_iterator, explicit and implicit
template<class It, class CIt>
struct conversion_is_nothrow
{
   static const bool value =
      noexcept(CIt(boost::move_detail::declval<It&>())) &&
      noexcept(CIt(boost::move_detail::declval<const It&>())) &&
      noexcept(noexcept_implicit_conversion<CIt>(boost::move_detail::declval<It&>())) &&
      noexcept(noexcept_implicit_conversion<CIt>(boost::move_detail::declval<const It&>()));
};

//Iterators of a container with forward iterators
template<class C>
struct forward_container_iterators_are_nothrow
{
   typedef typename C::iterator        it;
   typedef typename C::const_iterator  cit;

   static const bool value =
      forward_ops_are_nothrow<it,  typename C::reference>::value &&
      forward_ops_are_nothrow<cit, typename C::const_reference>::value &&
      conversion_is_nothrow<it, cit>::value;
};

//Iterators of a container with bidirectional iterators
template<class C>
struct bidirectional_container_iterators_are_nothrow
{
   typedef typename C::iterator        it;
   typedef typename C::const_iterator  cit;

   static const bool value =
      forward_container_iterators_are_nothrow<C>::value &&
      bidirectional_ops_are_nothrow<it>::value &&
      bidirectional_ops_are_nothrow<cit>::value;
};

//Iterators of a container with random access iterators
template<class C>
struct random_access_container_iterators_are_nothrow
{
   typedef typename C::iterator        it;
   typedef typename C::const_iterator  cit;
   typedef typename C::difference_type diff;

   static const bool value =
      bidirectional_container_iterators_are_nothrow<C>::value &&
      random_access_ops_are_nothrow<it,  typename C::reference, diff>::value &&
      random_access_ops_are_nothrow<cit, typename C::const_reference, diff>::value;
};

//Iterators that only count positions (the insertion proxy iterators):
//construction, copy, destruction, comparisons, increment, decrement and
//random access arithmetic, without dereference
template<class It>
struct counting_ops_are_nothrow
{
   typedef std::ptrdiff_t diff;

   static const bool value =
      noexcept(It()) &&
      noexcept(It(boost::move_detail::declval<It&>())) &&
      noexcept(It(boost::move_detail::declval<const It&>())) &&
      noexcept(boost::move_detail::declval<It&>().~It()) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<It&>() != boost::move_detail::declval<It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<It&>() == boost::move_detail::declval<It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() <  boost::move_detail::declval<const It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() <= boost::move_detail::declval<const It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() >  boost::move_detail::declval<const It&>())) &&
      noexcept(static_cast<bool>(boost::move_detail::declval<const It&>() >= boost::move_detail::declval<const It&>())) &&
      noexcept(++boost::move_detail::declval<It&>()) &&
      noexcept(boost::move_detail::declval<It&>()++) &&
      noexcept(--boost::move_detail::declval<It&>()) &&
      noexcept(boost::move_detail::declval<It&>()--) &&
      noexcept(boost::move_detail::declval<It&>() += diff()) &&
      noexcept(boost::move_detail::declval<It&>() -= diff()) &&
      noexcept(It(boost::move_detail::declval<const It&>() + diff())) &&
      noexcept(It(diff() + boost::move_detail::declval<const It&>())) &&
      noexcept(It(boost::move_detail::declval<const It&>() - diff())) &&
      noexcept(diff(boost::move_detail::declval<const It&>() - boost::move_detail::declval<const It&>()));
};

}  //namespace test {
}  //namespace container {
}  //namespace boost {

#endif   //#if !defined(BOOST_NO_CXX11_NOEXCEPT)

#endif   //#ifndef BOOST_CONTAINER_TEST_ITERATOR_NOEXCEPT_UTILS_HPP
