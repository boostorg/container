//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_TEST_UNQUALIFIED_SWAP_TEST_HPP
#define BOOST_CONTAINER_TEST_UNQUALIFIED_SWAP_TEST_HPP

#include <boost/container/detail/config_begin.hpp>
#include <cstddef>

//Unqualified swap(x, y), the way generic user code writes it, must reach the
//container's own swap through ADL.
//
//These helpers deliberately live outside namespace boost::container
//to test ADL

namespace boost_container_test_unqualified_swap {

//Contents are exchanged, and exchanged back, leaving both operands as they
//started. Needs only copy construction and operator== from Container.
template<class Container>
bool test_unqualified_swap(const Container &proto_a, const Container &proto_b)
{
   const Container a0(proto_a);
   const Container b0(proto_b);
   if(a0 == b0)   //the prototypes must differ or nothing below proves anything
      return false;

   Container a(proto_a);
   Container b(proto_b);

   swap(a, b);
   if(!(a == b0) || !(b == a0))
      return false;

   swap(a, b);
   if(!(a == a0) || !(b == b0))
      return false;

   //Swapping a container with itself must leave it alone
   swap(a, a);
   if(!(a == a0))
      return false;

   return true;
}

//Same, plus the check that catches the issue-342 class of bug: a container
//with an internal buffer must still be using its OWN object's storage after
//the swap. When the base's swap runs by mistake the values often still look
//right, while each object has quietly taken over the other's buffer.
//
//data_of must return a byte pointer to the container's elements, and
//uses_internal_storage must say whether the container is currently using its
//own inline buffer.
template<class Container, class DataOf, class UsesInternal>
bool test_unqualified_swap_internal_buffer
   (const Container &proto_a, const Container &proto_b,
    DataOf data_of, UsesInternal uses_internal_storage)
{
   if(!test_unqualified_swap(proto_a, proto_b))
      return false;

   //Both prototypes must fit in the internal buffer, or the check below is
   //vacuous.
   {
      Container probe_a(proto_a);
      Container probe_b(proto_b);
      if(!uses_internal_storage(probe_a) || !uses_internal_storage(probe_b))
         return false;
   }

   Container a(proto_a);
   Container b(proto_b);
   swap(a, b);

   if(!uses_internal_storage(a) || !uses_internal_storage(b))
      return false;

   //Each object's elements must live inside that object, not the other one
   const char *const a_begin = reinterpret_cast<const char *>(&a);
   const char *const b_begin = reinterpret_cast<const char *>(&b);
   const char *const a_data  = data_of(a);
   const char *const b_data  = data_of(b);

   if(!(a_data >= a_begin && a_data < a_begin + sizeof(Container)))
      return false;
   if(!(b_data >= b_begin && b_data < b_begin + sizeof(Container)))
      return false;

   return true;
}

}  //namespace boost_container_test_unqualified_swap {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_TEST_UNQUALIFIED_SWAP_TEST_HPP
