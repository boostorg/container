//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_PMR_SEGMENTED_VECTOR_HPP
#define BOOST_CONTAINER_PMR_SEGMENTED_VECTOR_HPP

#if defined (_MSC_VER)
#  pragma once
#endif

#include <boost/container/segmented_vector.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>

namespace boost {
namespace container {
namespace pmr {

#if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)

template <class T>
using segmented_vector = boost::container::segmented_vector<T, polymorphic_allocator<T>>;

#endif

//! A portable metafunction to obtain a segmented_vector
//! that uses a polymorphic allocator.
template<class T>
struct segmented_vector_of
{
   typedef boost::container::segmented_vector
      < T, polymorphic_allocator<T> > type;
};

}  //namespace pmr {
}  //namespace container {
}  //namespace boost {

#endif   //BOOST_CONTAINER_PMR_SEGMENTED_VECTOR_HPP
