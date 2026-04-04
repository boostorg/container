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

#include <boost/move/utility_core.hpp>

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

//! Sentinel whose comparison with any iterator always yields false (not equal).
//! Passes through segmented_remove_if_result_bounded to express an unbounded
//! destination, letting the compiler eliminate the dead destination-full branch.
struct unreachable_sentinel_t
{
   template <class It>
   BOOST_CONTAINER_FORCEINLINE friend bool operator==(const It&, unreachable_sentinel_t) { return false; }
   
   template <class It>
   BOOST_CONTAINER_FORCEINLINE friend bool operator==(unreachable_sentinel_t, const It&) { return false; }
   
   template <class It>
   BOOST_CONTAINER_FORCEINLINE friend bool operator!=(const It&, unreachable_sentinel_t) { return true; }
   
   template <class It>
   BOOST_CONTAINER_FORCEINLINE friend bool operator!=(unreachable_sentinel_t, const It&) { return true; }
};

namespace detail_algo {

template <bool B> struct void_if_true;
template <> struct void_if_true<true> { typedef void type; };

template <class T> struct make_void { typedef void type; };

template <bool, class T = void> struct algo_enable_if_c {};
template <class T> struct algo_enable_if_c<true, T> { typedef T type; };

template <class T, class Enable = void>
struct has_iterator_category { static const bool value = false; };

template <class T>
struct has_iterator_category<T, typename make_void<typename T::iterator_category>::type>
{ static const bool value = true; };

template <class Iterator, class Enable = void>
struct segmented_iterator_traits_impl
{
   typedef non_segmented_iterator_tag is_segmented_iterator;
};

template <class Iterator>
struct segmented_iterator_traits_impl<Iterator,
   typename void_if_true<Iterator::is_segmented_iterator::value>::type>
{
   typedef segmented_iterator_tag                is_segmented_iterator;
   typedef typename Iterator::segment_iterator   segment_iterator;
   typedef typename Iterator::local_iterator     local_iterator;

   static segment_iterator segment(Iterator it)  { return it.segment(); }
   static local_iterator   local(Iterator it)    { return it.local(); }

   static Iterator compose(segment_iterator s, local_iterator l)
   { return Iterator(s, l); }

   static local_iterator begin(segment_iterator s) { return s.begin(); }
   static local_iterator end(segment_iterator s)   { return s.end(); }
};

template <class T>
struct constref_generator
{
    const T& value;
    BOOST_CONTAINER_FORCEINLINE explicit constref_generator(const T& v)
       : value(v)
    {}

    BOOST_CONTAINER_FORCEINLINE const T& operator()() const { return value; }
};

//////////////////////////////////////////////////////////////////////////////
// Transfer helper: copy (Move=false) or move (Move=true) a single element.
//////////////////////////////////////////////////////////////////////////////

template <bool> struct transfer_op;
template <> struct transfer_op<false>
{
   template <class D, class S>
   BOOST_CONTAINER_FORCEINLINE static void apply(D& d, S& s) { d = s; }
};
template <> struct transfer_op<true>
{
   template <class D, class S>
   BOOST_CONTAINER_FORCEINLINE static void apply(D& d, S& s) { d = boost::move(s); }
};

} // namespace detail_algo

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
   : detail_algo::segmented_iterator_traits_impl<Iterator>
{};

//! Detects whether \c Sent is a true sentinel for \c Iter.
//! A sentinel is a type that is not the same as the iterator and
//! does not model an iterator (lacks \c iterator_category).
//! When \c is_sentinel is false, algorithms use the \c (Iter, Iter)
//! overload, avoiding extra template instantiations from mixed
//! iterator types such as \c iterator / \c const_iterator.
template <class Sent, class Iter>
struct is_sentinel
{
   static const bool value = !detail_algo::has_iterator_category<Sent>::value;
};

template <class Iter>
struct is_sentinel<Iter, Iter>
{
   static const bool value = false;
};

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

//#define BOOST_CONTAINER_ENABLE_SEGMENTED_LOOP_UNROLLING
#define BOOST_CONTAINER_DISABLE_SEGMENTED_LOOP_UNROLLING

#if defined(BOOST_CONTAINER_ENABLE_SEGMENTED_LOOP_UNROLLING) && defined(BOOST_CONTAINER_DISABLE_SEGMENTED_LOOP_UNROLLING)
   #error "Cannot define both BOOST_CONTAINER_ENABLE_SEGMENTED_LOOP_UNROLLING and BOOST_CONTAINER_DISABLE_SEGMENTED_LOOP_UNROLLING"
#elif !defined(BOOST_CONTAINER_ENABLE_SEGMENTED_LOOP_UNROLLING) && !defined(BOOST_CONTAINER_DISABLE_SEGMENTED_LOOP_UNROLLING)
   //Disable loop unrolling for clang, which generates suboptimal code in some case as clang auto-vectorizes
   //loops more aggressively than other compilers, and loop unrolling can interfere with this optimization.
   #if !defined(BOOST_CLANG)
      #define BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING
   #endif
#elif defined(BOOST_CONTAINER_ENABLE_SEGMENTED_LOOP_UNROLLING)
   //Force loop unrolling
   #define BOOST_CONTAINER_SEGMENTED_LOOP_UNROLLING
#else //defined(BOOST_CONTAINER_DISABLE_SEGMENTED_LOOP_UNROLLING)
   // Force no loop unrolling
#endif

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ITERATOR_TRAITS_HPP
