//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SAMPLE_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SAMPLE_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {

//! Selects \c n random elements from [first, last) using selection sampling
//! and writes them to \c out. Returns the output iterator past the last
//! sampled element.
template <class PopulationIt, class Sent, class SampleIt, class URNG>
inline SampleIt segmented_sample
   (PopulationIt first, Sent last, SampleIt out,
    typename boost::container::iterator_traits<PopulationIt>::difference_type n, URNG& g)
{
   typedef typename boost::container::iterator_traits<PopulationIt>::difference_type diff_t;
   diff_t remaining = 0;
   for(PopulationIt it = first; it != last; ++it)
      ++remaining;

   diff_t selected = 0;
   for(; first != last && selected < n; ++first, --remaining) {
      diff_t r = static_cast<diff_t>(g() % static_cast<unsigned long long>(remaining));
      if(r < (n - selected)) {
         *out = *first;
         ++out;
         ++selected;
      }
   }
   return out;
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SAMPLE_HPP
