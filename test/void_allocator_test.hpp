//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_VOID_ALLOCATOR_TEST_HEADER
#define BOOST_CONTAINER_TEST_VOID_ALLOCATOR_TEST_HEADER

#include <boost/container/allocator_traits.hpp>
#include <boost/container/detail/type_traits.hpp>
#include <boost/container/detail/workaround.hpp>

namespace boost {
namespace container {
namespace test {

//! Checks that \c Container (instantiated with an allocator whose
//! \c value_type is \c void, or with \c void itself) exposes the expected
//! rebound \c allocator_type and can insert one default-constructed value.
template<class Container, class ExpectedAllocator>
inline bool test_void_allocator()
{
   typedef typename Container::allocator_type allocator_type;
   typedef typename Container::value_type value_type;

   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<allocator_type, ExpectedAllocator>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same
      < typename allocator_traits<allocator_type>::value_type
      , value_type >::value));

   Container c;
   c.insert(c.cend(), value_type());
   return c.size() == typename Container::size_type(1);
}

}  //namespace test {
}  //namespace container {
}  //namespace boost {

#endif   //BOOST_CONTAINER_TEST_VOID_ALLOCATOR_TEST_HEADER
