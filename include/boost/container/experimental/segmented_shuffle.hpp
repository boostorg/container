//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SHUFFLE_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SHUFFLE_HPP

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
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

//! Randomly shuffles the elements in [first, last) using
//! the uniform random number generator \c g. Requires random access iterators.
//! \c SizedSent must support \c operator- with \c RandomIt (sized sentinel).
template <class RandomIt, class SizedSent, class URNG>
inline void segmented_shuffle(RandomIt first, SizedSent last, URNG& g)
{
   typedef typename boost::container::iterator_traits<RandomIt>::difference_type diff_t;
   for(diff_t i = (last - first) - 1; i > 0; --i) {
      diff_t j = static_cast<diff_t>(g() % static_cast<unsigned long long>(i + 1));
      if(i != j) {
         boost::adl_move_swap(first[i], first[j]);
      }
   }
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SHUFFLE_HPP
