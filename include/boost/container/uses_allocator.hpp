//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2011-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_USES_ALLOCATOR_HPP
#define BOOST_CONTAINER_USES_ALLOCATOR_HPP

#include <boost/container/uses_allocator_fwd.hpp>
#include <boost/container/detail/type_traits.hpp>

namespace boost {
namespace container {

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

namespace dtl {

template<typename T, typename Allocator>
struct uses_allocator_imp
{
   // Use SFINAE (Substitution Failure Is Not An Error) to detect the
   // presence of an 'allocator_type' nested type convertilble from Allocator.
   private:
   typedef char yes_type;
   struct no_type{ char dummy[2]; };

   // Match this function if T::allocator_type exists and is
   // implicitly convertible from Allocator
   template <class U>
   static yes_type test(typename U::allocator_type);

   // Match this function if T::allocator_type exists and its type is `erased_type`.
   template <class U, class V>
   static typename dtl::enable_if
      < dtl::is_same<typename U::allocator_type, erased_type>
      , yes_type
      >::type  test(const V&);

   // Match this function if TypeT::allocator_type does not exist or is
   // not convertible from Allocator.
   template <typename U>
   static no_type test(...);
   static Allocator alloc;  // Declared but not defined

   public:
   BOOST_STATIC_CONSTEXPR bool value = sizeof(test<T>(alloc)) == sizeof(yes_type);
};

}  //namespace dtl {

template <class T>
struct constructible_with_allocator_prefix
{  BOOST_STATIC_CONSTEXPR bool value = false; };

template <class T>
struct constructible_with_allocator_suffix
{  BOOST_STATIC_CONSTEXPR bool value = false; };

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//! <b>Remark</b>: Automatically detects whether T has a nested allocator_type that is convertible from
//! Allocator. Meets the BinaryTypeTrait requirements ([meta.rqmts] 20.4.1). A program may
//! specialize this type to define `uses_allocator<X>::value` as true for a T of user-defined type if T does not
//! have a nested allocator_type but is nonetheless constructible using the specified Allocator where either:
//! the first argument of a constructor has type `allocator_arg_t` and the second argument has type `Allocator` or
//! the last argument of a constructor has type `Allocator`.
//!
//! <b>Result</b>: `uses_allocator<T, Allocator>::value == true` if a type `T::allocator_type`
//! exists and either `is_convertible<Allocator, T::allocator_type>::value != false` or `T::allocator_type`
//! is an alias of `erased_type`. False otherwise.
template <typename T, typename Allocator>
struct uses_allocator
   : dtl::uses_allocator_imp<T, Allocator>
{};

}} //namespace boost::container

#endif   //BOOST_CONTAINER_USES_ALLOCATOR_HPP
