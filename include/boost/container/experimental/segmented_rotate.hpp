//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ROTATE_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ROTATE_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/move/adl_move_swap.hpp>

namespace boost {
namespace container {

//! Rotates the elements in [first, last) such that \c n_first becomes
//! the first element. Returns the iterator to the new position of the
//! element that was originally at \c first.
template <class FwdIt, class Sent>
inline FwdIt segmented_rotate(FwdIt first, FwdIt n_first, Sent last)
{
   if(first == n_first) return last;
   if(n_first == last)  return first;

   FwdIt next = n_first;
   do {
      boost::adl_move_swap(*first, *next);
      ++first; ++next;
      if(first == n_first) n_first = next;
   } while(next != last);

   FwdIt result = first;

   while(n_first != last) {
      next = n_first;
      do {
         boost::adl_move_swap(*first, *next);
         ++first; ++next;
         if(first == n_first) n_first = next;
      } while(next != last);
   }

   return result;
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_ROTATE_HPP
