//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
#include <boost/container/experimental/segmented_mismatch.hpp>
//Delegating to mismatch means every instantiation materialises an iterator
//category tag, and std_fwd.hpp only forward-declares those.
#include <iterator>

namespace boost {
namespace container {

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
FwdIt1 segmented_search
   (FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last);

namespace detail_algo {

//////////////////////////////////////////////////////////////////////////////
// Find-then-verify.  Verification is segmented_mismatch whenever both ends
// are homogeneous iterators; a sentinel on either side falls back to a
// lock-step equality walk because mismatch's segmented-iter2 walker still
// requires first2/last2 to share a type (only unreachable_sentinel_t is
// shimmed today).
//
// Skipping the already-matched first element keeps a one-element needle
// inside the (last1 - first1) * (last2 - first2) applications [alg.search]
// allows; re-testing it would over-apply by one.
//
// equal_to_deref keeps the search proxy-safe: *s_first is re-evaluated on
// every comparison, so a prvalue proxy never outlives its call.
//////////////////////////////////////////////////////////////////////////////

//Homogeneous ends: reuse mismatch (recursive on both ranges, RA leaf).
template <class FwdIt1, class FwdIt2, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE segduo<FwdIt1, FwdIt2>
segmented_search_verify
   (FwdIt1 it, FwdIt1 last, FwdIt2 s_it, FwdIt2 s_last,
    Tag tag, Cat cat)
{
   return (segmented_mismatch_bounded_dispatch)
      (it, last, s_it, s_last, mismatch_equal(), tag, cat);
}

//Sentinel on either side: flat equality walk with the same stop conditions
//as mismatch (needle exhausted -> match positions at ends, else mismatch).
template <class FwdIt1, class Sent1, class FwdIt2, class Sent2, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c
   <is_sentinel<Sent1, FwdIt1>::value || is_sentinel<Sent2, FwdIt2>::value
   , segduo<FwdIt1, FwdIt2> >::type
segmented_search_verify
   (FwdIt1 it, Sent1 last, FwdIt2 s_it, Sent2 s_last, Tag, Cat)
{
   for(;;) {
      if(!(*it == *s_it))
         return segduo<FwdIt1, FwdIt2>(it, s_it);
      ++it;
      ++s_it;
      if(s_it == s_last)
         return segduo<FwdIt1, FwdIt2>(it, s_it);
      if(it == last)
         return segduo<FwdIt1, FwdIt2>(it, s_it);
   }
}

template <class FwdIt1, class Sent1, class FwdIt2, class Sent2, class Tag>
FwdIt1 segmented_search_dispatch
   (FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last, Tag tag)
{
   if (BOOST_UNLIKELY(s_first == s_last))
      return first;

   typedef typename iterator_traits<FwdIt1>::iterator_category cat_t;

   equal_to_deref<FwdIt2> eq(s_first);

   while (first != last) {
      first = boost::container::segmented_find_if(first, last, eq);
      if (first == last)
         return last;

      FwdIt1 it = first;
      ++it;
      FwdIt2 s_it = s_first;
      ++s_it;
      if (s_it == s_last)
         return first;          // one-element needle -> already matched
      if (it == last)
         return last;           // source exhausted before needle

      const segduo<FwdIt1, FwdIt2> r = (segmented_search_verify)
         (it, last, s_it, s_last, tag, cat_t());

      if (r.second == s_last)
         return first;          // full needle consumed -> match
      if (r.first == last)
         return last;           // source exhausted before needle
      ++first;
   }
   return last;
}

} // namespace detail_algo

//! Finds the first occurrence of the subsequence [s_first, s_last) in [first, last).
//! Returns an iterator to the beginning of the found subsequence, or \c last if not found.
//! Exploits segmentation recursively on both ranges.
template <class FwdIt1, class Sent1, class FwdIt2, class Sent2>
BOOST_CONTAINER_FORCEINLINE
FwdIt1 segmented_search(FwdIt1 first, Sent1 last, FwdIt2 s_first, Sent2 s_last)
{
   typedef segmented_iterator_traits<FwdIt1> traits;
   return detail_algo::segmented_search_dispatch
      (first, last, s_first, s_last, typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_SEARCH_HPP
