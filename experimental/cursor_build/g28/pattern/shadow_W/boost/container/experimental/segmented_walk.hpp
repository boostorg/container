//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
// g28 experiment: shared generic segment walkers (variant W).
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_WALK_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_WALK_HPP
#define BOOST_CONTAINER_G28_SHADOW_WALK 'W'

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/iterator.hpp>

namespace boost {
namespace container {
namespace detail_algo {

// Shared full-scan walker.  Applies f to every maximal flat subrange of
// [first, last), recursing while the local iterator is itself segmented.
// F::operator()(It, Sent) must process one flat leaf range; state (an
// accumulator, the fill value...) lives in the functor, which is passed by
// reference through every level.

template <class It, class Sent, class F, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, It>::value>::type
segmented_walk(It first, Sent last, F& f, Tag, Cat)
{  f(first, last);  }

template <class SegIter, class F, class Cat>
void segmented_walk(SegIter first, SegIter last, F& f, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>   traits;
   typedef typename traits::local_iterator      local_iterator;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   segment_iterator sfirst = traits::segment(first);
   segment_iterator slast  = traits::segment(last);

   local_iterator lb = traits::local(first);

   if(BOOST_LIKELY(sfirst != slast)) {
      (segmented_walk)(lb, traits::end(sfirst), f, is_local_seg_t(), local_cat_t());

      for(++sfirst; sfirst != slast; ++sfirst)
         (segmented_walk)(traits::begin(sfirst), traits::end(sfirst), f, is_local_seg_t(), local_cat_t());

      lb = traits::begin(slast);
   }
   (segmented_walk)(lb, traits::local(last), f, is_local_seg_t(), local_cat_t());
}

// Shared early-exit walker.  F::operator()(It, Sent) processes one flat leaf
// range and returns the position where it stopped; a stop strictly before the
// leaf's end is a hit.  On hit, out receives the composed iterator of the hit
// at this level and true is returned; on miss out is untouched.

template <class It, class Sent, class F, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, It>::value, bool>::type
segmented_walk_until(It first, Sent last, F& f, It& out, Tag, Cat)
{
   out = f(first, last);
   return out != last;
}

template <class SegIter, class F, class Cat>
bool segmented_walk_until(SegIter first, SegIter last, F& f, SegIter& out, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>   traits;
   typedef typename traits::local_iterator      local_iterator;
   typedef typename traits::segment_iterator    segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   segment_iterator       sfirst = traits::segment(first);
   const segment_iterator slast  = traits::segment(last);

   local_iterator lb = traits::local(first);
   local_iterator lr;

   if(BOOST_LIKELY(sfirst != slast)) {
      if((segmented_walk_until)(lb, traits::end(sfirst), f, lr, is_local_seg_t(), local_cat_t())) {
         out = traits::compose(sfirst, lr);
         return true;
      }
      for(++sfirst; sfirst != slast; ++sfirst) {
         if((segmented_walk_until)(traits::begin(sfirst), traits::end(sfirst), f, lr, is_local_seg_t(), local_cat_t())) {
            out = traits::compose(sfirst, lr);
            return true;
         }
      }
      lb = traits::begin(slast);
   }
   if((segmented_walk_until)(lb, traits::local(last), f, lr, is_local_seg_t(), local_cat_t())) {
      out = traits::compose(sfirst, lr);
      return true;
   }
   return false;
}

} // namespace detail_algo
} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_WALK_HPP
