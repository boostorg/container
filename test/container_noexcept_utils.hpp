//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_CONTAINER_NOEXCEPT_UTILS_HPP
#define BOOST_CONTAINER_TEST_CONTAINER_NOEXCEPT_UTILS_HPP

//Traits that check that the container operations that do not allocate memory,
//call the comparator or move elements cannot throw.
//Use them in static asserts only when BOOST_NO_CXX11_NOEXCEPT is not defined.

#include <boost/config.hpp>
#include <boost/move/utility_core.hpp>

#if !defined(BOOST_NO_CXX11_NOEXCEPT)

namespace boost {
namespace container {
namespace test {

//get_allocator and get_stored_allocator
template<class C>
struct container_allocator_ops_are_nothrow
{
   static const bool value =
      noexcept(boost::move_detail::declval<const C&>().get_allocator()) &&
      noexcept(boost::move_detail::declval<C&>().get_stored_allocator()) &&
      noexcept(boost::move_detail::declval<const C&>().get_stored_allocator());
};

//Forward iterators, capacity observers and clear
template<class C>
struct container_basic_ops_are_nothrow
{
   static const bool value =
      noexcept(boost::move_detail::declval<C&>().begin()) &&
      noexcept(boost::move_detail::declval<const C&>().begin()) &&
      noexcept(boost::move_detail::declval<const C&>().cbegin()) &&
      noexcept(boost::move_detail::declval<C&>().end()) &&
      noexcept(boost::move_detail::declval<const C&>().end()) &&
      noexcept(boost::move_detail::declval<const C&>().cend()) &&
      noexcept(boost::move_detail::declval<const C&>().empty()) &&
      noexcept(boost::move_detail::declval<const C&>().size()) &&
      noexcept(boost::move_detail::declval<const C&>().max_size()) &&
      noexcept(boost::move_detail::declval<C&>().clear());
};

//Reverse iterators
template<class C>
struct container_reverse_ops_are_nothrow
{
   static const bool value =
      noexcept(boost::move_detail::declval<C&>().rbegin()) &&
      noexcept(boost::move_detail::declval<const C&>().rbegin()) &&
      noexcept(boost::move_detail::declval<const C&>().crbegin()) &&
      noexcept(boost::move_detail::declval<C&>().rend()) &&
      noexcept(boost::move_detail::declval<const C&>().rend()) &&
      noexcept(boost::move_detail::declval<const C&>().crend());
};

//Erasure: node-based containers only unlink and destroy elements. Other
//containers can use it only if their erase moves elements with a nothrow
//move assignment
template<class C>
struct container_erase_is_nothrow
{
   typedef typename C::const_iterator cit;

   static const bool value =
      noexcept(boost::move_detail::declval<C&>().erase(boost::move_detail::declval<cit>())) &&
      noexcept(boost::move_detail::declval<C&>().erase( boost::move_detail::declval<cit>()
                                                      , boost::move_detail::declval<cit>()));
};

//All of the above for an allocator-aware reversible container
template<class C>
struct reversible_container_ops_are_nothrow
{
   static const bool value =
      container_allocator_ops_are_nothrow<C>::value &&
      container_basic_ops_are_nothrow<C>::value &&
      container_reverse_ops_are_nothrow<C>::value;
};

//Tree-based associative containers: also with a comparator whose copy can throw
template<class C>
struct tree_container_ops_are_nothrow
{
   typedef typename C::const_iterator cit;

   static const bool value =
      reversible_container_ops_are_nothrow<C>::value &&
      container_erase_is_nothrow<C>::value &&
      noexcept(boost::move_detail::declval<C&>().extract(boost::move_detail::declval<cit>()));
};

}  //namespace test {
}  //namespace container {
}  //namespace boost {

#endif   //#if !defined(BOOST_NO_CXX11_NOEXCEPT)

#endif   //#ifndef BOOST_CONTAINER_TEST_CONTAINER_NOEXCEPT_UTILS_HPP
