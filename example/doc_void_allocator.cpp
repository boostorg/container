//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/detail/type_traits.hpp>

//[doc_void_allocator
#include <boost/container/vector.hpp>
#include <boost/container/map.hpp>
#include <boost/container/allocator.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <utility>
//=#include <type_traits>


int main()
{
   using namespace boost::container;

//
// void as Allocator template argument
//
   //Allocator argument is void: the library selects its default allocator.
   vector<int>       v_default;          // Allocator defaults to void
   vector<int, void> v_explicit_void;    // Explicit void
   map<int, double>  m_default;          // Allocator defaults to void
   map<int, double, std::less<int>, void> m_explicit_void;

   //Same types
   v_default.push_back(1);
   v_explicit_void = v_default;

   m_default[3] = 3.0;
   m_explicit_void = m_default;

//
// Automatic rebinding with allocator::value_type == void, 
//
   typedef pmr::polymorphic_allocator<void>              pmr_void_t;
   typedef vector<int, pmr_void_t >                      vector_alloc_of_void_t;
   typedef map<int, double, std::less<int>, pmr_void_t > map_alloc_of_void_t;
   typedef std::pair<const int, double>                  map_value_t;

   /*<-*/
   BOOST_CONTAINER_STATIC_ASSERT
      ((dtl::is_same< vector_alloc_of_void_t::allocator_type
      , pmr::polymorphic_allocator<int> >::value));
   BOOST_CONTAINER_STATIC_ASSERT
      ((dtl::is_same< map_alloc_of_void_t::allocator_type
      , pmr::polymorphic_allocator<map_value_t> >::value));
   /*->*/
   //Container::allocator_type is the expected type
   //=static_assert
   //=   (std::is_same< vector_alloc_of_void_t >::allocator_type, pmr::polymorphic_allocator<int> >::value);
   //=static_assert
   //=   (std::is_same< map_alloc_of_void_t >::allocator_type, pmr::polymorphic_allocator<map_value_t> >::value);

   //Usually the Allocator<void> type is convertible to the rebound allocator, no need
   //to explicitly rebind it.
   pmr_void_t alloc;
   vector_alloc_of_void_t v(alloc);
   map_alloc_of_void_t m(std::less<int>(), alloc);

   return 0;
}
//]
