//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_IF_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_IF_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>

namespace boost {
namespace container {

template <class InpIter, class Sent, class Pred>
InpIter segmented_find_if(InpIter first, Sent last, Pred pred);

namespace detail_algo {

template <class SegIter, class Pred>
SegIter segmented_find_if_dispatch
   (SegIter first, SegIter last, Pred pred, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   if(first == last) return last;

   typename traits::segment_iterator sfirst = traits::segment(first);
   typename traits::segment_iterator slast  = traits::segment(last);

   if(sfirst == slast) {
      typename traits::local_iterator r =
         boost::container::segmented_find_if(traits::local(first), traits::local(last), pred);
      if(r != traits::local(last))
         return traits::compose(sfirst, r);
      return last;
   }
   {
      typename traits::local_iterator lend = traits::end(sfirst);
      typename traits::local_iterator r =
         boost::container::segmented_find_if(traits::local(first), lend, pred);
      if(r != lend) return traits::compose(sfirst, r);
   }
   for(++sfirst; sfirst != slast; ++sfirst) {
      typename traits::local_iterator lb = traits::begin(sfirst);
      typename traits::local_iterator le = traits::end(sfirst);
      typename traits::local_iterator r =
         boost::container::segmented_find_if(lb, le, pred);
      if(r != le) return traits::compose(sfirst, r);
   }
   {
      typename traits::local_iterator lb = traits::begin(sfirst);
      typename traits::local_iterator ll = traits::local(last);
      typename traits::local_iterator r =
         boost::container::segmented_find_if(lb, ll, pred);
      if(r != ll) return traits::compose(sfirst, r);
   }
   return last;
}

template <class InpIter, class Sent, class Pred, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InpIter>::value, InpIter>::type
segmented_find_if_dispatch
   (InpIter first, Sent last, Pred pred, Tag)
{
   for(; first != last; ++first)
      if(pred(*first))
         return first;
   return last;
}

} // namespace detail_algo

//! Returns an iterator to the first element satisfying \c pred
//! in [first, last), or \c last if not found.
template <class InpIter, class Sent, class Pred>
inline InpIter segmented_find_if(InpIter first, Sent last, Pred pred)
{
   typedef segmented_iterator_traits<InpIter> traits;
   return detail_algo::segmented_find_if_dispatch(first, last, pred,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_IF_HPP
