//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2015-2015. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/pmr/small_vector.hpp>
#include <boost/container/detail/type_traits.hpp>
#include "void_allocator_test.hpp"

int main()
{
   using namespace boost::container;
   using boost::container::dtl::is_same;

   typedef small_vector<int, 2, pmr::polymorphic_allocator<int> > intcontainer_t;
   BOOST_CONTAINER_STATIC_ASSERT(( is_same<intcontainer_t, pmr::small_vector_of<int, 2>::type >::value ));
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
      BOOST_CONTAINER_STATIC_ASSERT(( is_same<intcontainer_t, pmr::small_vector<int, 2> >::value ));
   #endif

   intcontainer_t cont(pmr::get_default_resource());
   typedef intcontainer_t::value_type value_type;
   cont.push_back(value_type());

   ////////////////////////////////////
   //    Void value_type allocator
   ////////////////////////////////////
   {
      typedef small_vector<int, 8, pmr::polymorphic_allocator<void> > voidalloc_cont_t;
      if(!test::test_void_allocator
            < voidalloc_cont_t
            , pmr::polymorphic_allocator<int> >()) {
         return 1;
      }
   }

   return 0;
}
