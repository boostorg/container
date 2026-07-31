//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ITERATOR_TRAITS_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ITERATOR_TRAITS_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>

namespace boost {
namespace container {

//! Tag type indicating a segmented iterator.
struct segmented_iterator_tag
{
   static const bool value = true;
};

//! Tag type indicating a non-segmented (flat) iterator.
struct non_segmented_iterator_tag
{
   static const bool value = false;
};

//! Traits class to detect and decompose segmented iterators.
//!
//! The default definition marks all iterators as non-segmented.
//! Specializations for segmented iterator types must provide:
//!
//!   typedef segmented_iterator_tag         is_segmented_iterator;
//!   typedef <implementation-defined>       segment_iterator;
//!   typedef <implementation-defined>       local_iterator;
//!
//!   static segment_iterator segment(Iterator it);
//!   static local_iterator   local(Iterator it);
//!   static Iterator         compose(segment_iterator s, local_iterator l);
//!   static local_iterator   begin(segment_iterator s);
//!   static local_iterator   end(segment_iterator s);
//!
//! An explicit specialization is not required when the iterator type
//! provides the intrusive interface:
//!
//!   typedef segmented_iterator_tag         is_segmented_iterator;
//!   typedef <implementation-defined>       segment_iterator;
//!   typedef <implementation-defined>       local_iterator;
//!   segment_iterator segment() const;
//!   local_iterator   local()   const;
//!   Iterator(segment_iterator s, local_iterator l);  // composing constructor
//!
//! and segment_iterator provides begin()/end() returning local_iterator.
//!
//! Based on: M. Austern, "Segmented Iterators and Hierarchical Algorithms"
template <class Iterator>
struct segmented_iterator_traits
{
   typedef non_segmented_iterator_tag is_segmented_iterator;
};

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ITERATOR_TRAITS_HPP
