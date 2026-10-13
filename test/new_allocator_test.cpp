//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#include <boost/container/new_allocator.hpp>
#include <boost/container/allocator_traits.hpp>
#include <boost/container/throw_exception.hpp>
#include <boost/container/detail/type_traits.hpp>
#include <boost/move/utility_core.hpp>
#include "movable_int.hpp"
#include "void_allocator_test.hpp"
#include "lightweight_test.hpp"
#include <cstddef>
#include <new>

using namespace boost::container;

typedef test::overaligned_copyable_int oint_t;

namespace boost {
namespace container {

//Explicit instantiations to catch compilation errors
template class new_allocator<int>;
template class new_allocator<oint_t>;
//new_allocator<void> is an explicit specialization: it is tested with test_void_allocator_conversions

}} //boost::container

//Types and propagation traits seen by the containers
template<class T>
void test_traits()
{
   using dtl::is_same;
   typedef new_allocator<T>         alloc_t;
   typedef allocator_traits<alloc_t> traits_t;
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename traits_t::value_type, T>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename traits_t::pointer, T*>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename traits_t::const_pointer, const T*>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename traits_t::size_type, std::size_t>::value));
   BOOST_CONTAINER_STATIC_ASSERT((is_same<typename traits_t::difference_type, std::ptrdiff_t>::value));
   BOOST_CONTAINER_STATIC_ASSERT((!traits_t::propagate_on_container_copy_assignment::value));
   BOOST_CONTAINER_STATIC_ASSERT((traits_t::propagate_on_container_move_assignment::value));
   BOOST_CONTAINER_STATIC_ASSERT((!traits_t::propagate_on_container_swap::value));
   BOOST_CONTAINER_STATIC_ASSERT((traits_t::is_always_equal::value));
   //Stateless
   BOOST_CONTAINER_STATIC_ASSERT((dtl::is_empty<alloc_t>::value));
}

//Allocates arrays of several sizes, writes them and checks the alignment
template<class T>
void test_allocate_deallocate()
{
   new_allocator<T> a;
   const std::size_t sizes[] = { 1u, 2u, 7u, 1000u };
   for (std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i) {
      const std::size_t n = sizes[i];
      T *const p = a.allocate(n);
      BOOST_TEST(p != 0);
      //Check through char* so that the compiler can't assume the alignment of p
      const char *const cp = static_cast<const char*>(static_cast<const void*>(p));
      BOOST_TEST(reinterpret_cast<std::size_t>(cp) % dtl::alignment_of<T>::value == 0u);
      for (std::size_t j = 0; j != n; ++j) {
         ::new(static_cast<void*>(p + j)) T();
      }
      for (std::size_t j = 0; j != n; ++j) {
         p[j].~T();
      }
      a.deallocate(p, n);
   }
}

//max_size() and the allocation of more elements than max_size()
template<class T>
void test_max_size()
{
   new_allocator<T> a;
   BOOST_TEST(a.max_size() == std::size_t(-1)/(2u*sizeof(T)));
   #ifndef BOOST_NO_EXCEPTIONS
   //Without exceptions, throw_bad_alloc() aborts
   bool thrown = false;
   try{
      T *const p = a.allocate(a.max_size() + 1u);
      a.deallocate(p, a.max_size() + 1u);
   }
   catch(const bad_alloc_t &){
      thrown = true;
   }
   BOOST_TEST(thrown);
   #endif
}

//Comparison, swap and conversions between allocators of different types
void test_comparison_and_conversions()
{
   new_allocator<int>    i1, i2;
   new_allocator<double> d(i1);
   new_allocator<int>    i3(d);
   BOOST_TEST(i1 == i2);
   BOOST_TEST(!(i1 != i2));
   BOOST_TEST(i1 == i3);
   swap(i1, i2);
   i1 = i2;
   BOOST_TEST(i1 == i2);
}

#if !defined(BOOST_NO_CXX11_NOEXCEPT)
void test_noexcept()
{
   new_allocator<int> a;
   int *p = 0;
   BOOST_CONTAINER_STATIC_ASSERT((noexcept(new_allocator<int>())));
   BOOST_CONTAINER_STATIC_ASSERT((noexcept(new_allocator<int>(a))));
   BOOST_CONTAINER_STATIC_ASSERT((noexcept(new_allocator<double>(a))));
   BOOST_CONTAINER_STATIC_ASSERT((noexcept(a.deallocate(p, 1u))));
   BOOST_CONTAINER_STATIC_ASSERT((noexcept(a.max_size())));
   BOOST_CONTAINER_STATIC_ASSERT((noexcept(a == a)));
   (void)p;
}
#endif

int main()
{
   test_traits<int>();
   test_traits<oint_t>();
   test_allocate_deallocate<int>();
   test_allocate_deallocate<oint_t>();
   test_max_size<int>();
   test_max_size<oint_t>();
   test_comparison_and_conversions();
   #if !defined(BOOST_NO_CXX11_NOEXCEPT)
   test_noexcept();
   #endif
   BOOST_TEST(test::test_void_allocator_conversions< new_allocator<void> >());
   return boost::report_errors();
}
