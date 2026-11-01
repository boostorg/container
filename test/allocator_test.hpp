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

//Tests an allocator of void, used as a type-erased allocator (test_void_allocator checks that
//containers rebind it): allocator_traits types, rebinding between void and non-void allocators,
//conversions in both directions and comparison.
template<class VoidAlloc>
inline bool test_void_allocator_conversions()
{
   typedef allocator_traits<VoidAlloc>                                        void_traits;
   typedef typename void_traits::template portable_rebind_alloc<int>::type    int_alloc_t;
   typedef typename void_traits::template portable_rebind_alloc<double>::type double_alloc_t;
   typedef allocator_traits<int_alloc_t>                                      int_traits;

   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename void_traits::value_type, void>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename int_traits::value_type, int>::value));
   //The pointers of the void allocator are the void pointers of the other ones
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename void_traits::pointer, typename int_traits::void_pointer>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename void_traits::const_pointer, typename int_traits::const_void_pointer>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename void_traits::void_pointer, typename void_traits::pointer>::value));
   //Rebinding back to void gives the same allocator
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename int_traits::template portable_rebind_alloc<void>::type, VoidAlloc>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_same<typename int_traits::template portable_rebind_alloc<double>::type, double_alloc_t>::value));

   //The void allocator propagates like the other types
   BOOST_CONTAINER_STATIC_ASSERT((void_traits::propagate_on_container_copy_assignment::value == int_traits::propagate_on_container_copy_assignment::value));
   BOOST_CONTAINER_STATIC_ASSERT((void_traits::propagate_on_container_move_assignment::value == int_traits::propagate_on_container_move_assignment::value));
   BOOST_CONTAINER_STATIC_ASSERT((void_traits::propagate_on_container_swap::value == int_traits::propagate_on_container_swap::value));
   BOOST_CONTAINER_STATIC_ASSERT((void_traits::is_always_equal::value == int_traits::is_always_equal::value));

   //Conversions in both directions and between non-void types
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_convertible<int_alloc_t, VoidAlloc>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_convertible<VoidAlloc, int_alloc_t>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_convertible<VoidAlloc, double_alloc_t>::value));
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_convertible<int_alloc_t, double_alloc_t>::value));
   const int_alloc_t    i;
   const VoidAlloc      v(i);
   const int_alloc_t    i2(v);
   const double_alloc_t d(v);
   const VoidAlloc      v2(d);
   VoidAlloc v3(v);
   v3 = v2;
   (void)i2; (void)v3;

   //An allocator is equal to itself, and always-equal allocators are equal to any other one
   if (!(v == v) || (v != v))
      return false;
   if (void_traits::is_always_equal::value && !(v == v2))
      return false;
   return true;
}

}  //namespace test {
}  //namespace container {
}  //namespace boost {

#endif   //BOOST_CONTAINER_TEST_VOID_ALLOCATOR_TEST_HEADER
