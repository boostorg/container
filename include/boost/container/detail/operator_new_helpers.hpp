//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2025. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_DETAIL_OPERATOR_NEW_HELPERS_HPP
#define BOOST_CONTAINER_DETAIL_OPERATOR_NEW_HELPERS_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/std_fwd.hpp>
#include <boost/container/throw_exception.hpp>
#include <boost/container/detail/type_traits.hpp>

namespace boost {
namespace container {
namespace dtl {

BOOST_CONTAINER_FORCEINLINE void* operator_new_raw_allocate(const std::size_t size, const std::size_t alignment)
{
   (void)alignment;
   #if defined(__cpp_aligned_new)
   if(__STDCPP_DEFAULT_NEW_ALIGNMENT__ < alignment) {
      return ::operator new(size, std::align_val_t(alignment));
   }
   #endif
   return ::operator new(size);
}

BOOST_CONTAINER_FORCEINLINE void operator_delete_raw_deallocate
   (void* const ptr, const std::size_t size, const std::size_t alignment) BOOST_NOEXCEPT_OR_NOTHROW
{
   (void)size;
   (void)alignment;
   #ifdef __cpp_aligned_new
   if(__STDCPP_DEFAULT_NEW_ALIGNMENT__ < alignment) {
      # if defined(__cpp_sized_deallocation)
      ::operator delete(ptr, size, std::align_val_t(alignment));
      #else
      ::operator delete(ptr, std::align_val_t(alignment));
      # endif
      return;
   }
   #endif

   # if defined(__cpp_sized_deallocation)
   ::operator delete(ptr, size);
   #else
   ::operator delete(ptr);
   # endif
}

template <class T>
BOOST_CONTAINER_FORCEINLINE T* operator_new_allocate(std::size_t count)
{
   const std::size_t max_count = std::size_t(-1)/(2*sizeof(T));
   if(BOOST_UNLIKELY(count > max_count))
      throw_bad_alloc();
   return static_cast<T*>(operator_new_raw_allocate(count*sizeof(T), alignment_of<T>::value));
}

template <class T>
BOOST_CONTAINER_FORCEINLINE void operator_delete_deallocate(T* ptr, std::size_t n) BOOST_NOEXCEPT_OR_NOTHROW
{
   operator_delete_raw_deallocate((void*)ptr, n * sizeof(T), alignment_of<T>::value);
}


}  //namespace dtl {
}  //namespace container {
}  //namespace boost {

#endif   //#ifndef BOOST_CONTAINER_DETAIL_OPERATOR_NEW_HELPERS_HPP
