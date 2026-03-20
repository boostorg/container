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
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;

   segment_iterator       sfirst = traits::segment(first);
   const segment_iterator slast  = traits::segment(last);
   const local_iterator      lf  = traits::local(first);

   if(sfirst == slast) {
      const local_iterator ll = traits::local(last);
      local_iterator r = (segmented_find_if)(lf, ll, pred);
      return traits::compose(sfirst, r);
   }
   else {
      //First segment
      {
         const local_iterator le = traits::end(sfirst);
         const local_iterator r = (segmented_find_if)(lf, le, pred);
         if (r != le)
            return traits::compose(sfirst, r);
      }
      //Middle segments
      for (++sfirst; sfirst != slast; ++sfirst) {
         const local_iterator le = traits::end(sfirst);
         const local_iterator r = (segmented_find_if)(traits::begin(sfirst), le, pred);
         if (r != le)
            return traits::compose(sfirst, r);
      }
      //Last segment
      {
         const local_iterator ll = traits::local(last);
         const local_iterator r = (segmented_find_if)(traits::begin(sfirst), ll, pred);
         return traits::compose(sfirst, r);
      }
   }
}

template <class InpIter, class Sent, class Pred, class Tag>
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InpIter>::value, InpIter>::type
segmented_find_if_dispatch(InpIter first, Sent last, Pred pred, Tag)
{
   for(; first != last; ++first)
      if(pred(*first))
         break;
   return first;
}

} // namespace detail_algo

//! Returns an iterator to the first element satisfying \c pred
//! in [first, last), or \c last if not found.
template <class InpIter, class Sent, class Pred>
BOOST_CONTAINER_FORCEINLINE
InpIter segmented_find_if(InpIter first, Sent last, Pred pred)
{
   typedef segmented_iterator_traits<InpIter> traits;
   return detail_algo::segmented_find_if_dispatch(first, last, pred, typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_FIND_IF_HPP
