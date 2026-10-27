//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_COPY_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_COPY_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/detail/segmented_common_algo.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/container/detail/type_traits.hpp>
#include <cstddef>
#include <utility>

namespace boost {
namespace container {

template <class InIter, class Sent, class OutIter1, class OutIter2, class Pred>
std::pair<OutIter1, OutIter2>
segmented_partition_copy(InIter first, Sent last, OutIter1 out_true, OutIter2 out_false, Pred pred);

namespace detail_algo {

template <std::size_t BlockSize, class TIter, class FIter, class Diff>
BOOST_CONTAINER_FORCEINLINE bool partition_copy_room_enough
   (TIter t_first, TIter t_last, FIter f_first, FIter f_last, Diff avail)
{
   const Diff block_size = static_cast<Diff>(BlockSize);
   return avail >= block_size &&
      static_cast<Diff>(t_last - t_first) >= block_size &&
      static_cast<Diff>(f_last - f_first) >= block_size;
}

template <std::size_t BlockSize, class TIter, class FIter, class Diff>
BOOST_CONTAINER_FORCEINLINE bool partition_copy_room_enough
   (TIter, unreachable_sentinel_t, FIter f_first, FIter f_last, Diff avail)
{
   const Diff block_size = static_cast<Diff>(BlockSize);
   return avail >= block_size &&
      static_cast<Diff>(f_last - f_first) >= block_size;
}

template <std::size_t BlockSize, class TIter, class FIter, class Diff>
BOOST_CONTAINER_FORCEINLINE bool partition_copy_room_enough
   (TIter t_first, TIter t_last, FIter, unreachable_sentinel_t, Diff avail)
{
   const Diff block_size = static_cast<Diff>(BlockSize);
   return avail >= block_size &&
      static_cast<Diff>(t_last - t_first) >= block_size;
}

template <std::size_t BlockSize, class TIter, class FIter, class Diff>
BOOST_CONTAINER_FORCEINLINE bool partition_copy_room_enough
   (TIter, unreachable_sentinel_t, FIter, unreachable_sentinel_t, Diff avail)
{ return avail >= static_cast<Diff>(BlockSize); }

template <class It>
struct pc_output_is_ra
{
   static const bool value = dtl::is_convertible
      < typename boost::container::iterator_traits<It>::iterator_category
      , std::random_access_iterator_tag >::value;
};

//Carry of the partition_copy workers: an element that was tested and could not
//be written because its output was full (*first, not yet incremented).
//0: no carry, 1: *first goes to out_true, 2: *first goes to out_false.

template <class SrcIter, class Sent, class TIter, class TSent, class FIter, class FSent,
          class Pred, class SrcCat>
BOOST_CONTAINER_FORCEINLINE
segquartet<SrcIter, TIter, FIter, unsigned>
partition_copy_leaf
   (SrcIter first, Sent last, TIter t_first, TSent t_last, FIter f_first, FSent f_last,
    Pred pred, unsigned carry, const SrcCat &)
{
   // pred is applied exactly last - first times.  An element is tested first and
   // then the room of its output is checked: if that output is full, the leaf stops
   // and returns the element as a carry (fourth member).  The walker of that output
   // passes the carry back with its next range, and the element is written without
   // testing it again.  With unreachable_sentinel_t outputs the room tests fold
   // away, so the flat path is unchanged.
   typedef segquartet<SrcIter, TIter, FIter, unsigned> result_t;

   //The caller gives a carry only with a range of that output that has room
   if(carry == 1u) {
      *t_first = *first;
      ++t_first;
      ++first;
      carry = 0u;
   }
   else if(carry == 2u) {
      *f_first = *first;
      ++f_first;
      ++first;
      carry = 0u;
   }

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   while(first != last) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(t_first == t_last)) {
            carry = 1u;
            break;
         }
         *t_first = *first;
         ++t_first;
      }
      else {
         if(BOOST_UNLIKELY(f_first == f_last)) {
            carry = 2u;
            break;
         }
         *f_first = *first;
         ++f_first;
      }
      ++first;
   }
   return result_t(first, t_first, f_first, carry);
}

#if BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS
// Random-access-source fast path (needs random-access outputs too).
// Two loops: both outputs have room for a full block; residual with
// per-write checks.  An unbounded output folds its capacity tests away via
// unreachable_sentinel_t.
template <class RASrcIter, class TIter, class TSent, class FIter, class FSent, class Pred>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c
   < pc_output_is_ra<TIter>::value && pc_output_is_ra<FIter>::value
   , segquartet<RASrcIter, TIter, FIter, unsigned> >::type
partition_copy_leaf
   (RASrcIter first, RASrcIter last, TIter t_first, TSent t_last, FIter f_first, FSent f_last,
    Pred pred, unsigned carry, const std::random_access_iterator_tag &)
{
   //Same carry contract as the generic leaf
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;
   typedef segquartet<RASrcIter, TIter, FIter, unsigned> result_t;
   const difference_type block_size = 16;

   //The caller gives a carry only with a range of that output that has room
   if(carry == 1u) {
      *t_first = *first;
      ++t_first;
      ++first;
      carry = 0u;
   }
   else if(carry == 2u) {
      *f_first = *first;
      ++f_first;
      ++first;
      carry = 0u;
   }
   difference_type n = last - first;

   //Avoid output checks if source and both outputs are big enough
   while((partition_copy_room_enough<16>)
            (t_first, t_last, f_first, f_last, n)) {
      n -= block_size;
      BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL
      for(difference_type chunk = block_size; chunk; ) {
         --chunk;
         if(pred(*first)) {
            *t_first = *first;
            ++t_first;
         }
         else {
            *f_first = *first;
            ++f_first;
         }
         ++first;
      }
   }

   //n >= 0 here. An unsigned counter avoids a false GCC
   //-Waggressive-loop-optimizations warning
   std::size_t rem = static_cast<std::size_t>(n);
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   while(rem) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(t_first == t_last)) {
            carry = 1u;
            break;
         }
         *t_first = *first;
         ++t_first;
      }
      else {
         if(BOOST_UNLIKELY(f_first == f_last)) {
            carry = 2u;
            break;
         }
         *f_first = *first;
         ++f_first;
      }
      ++first;
      --rem;
   }
   return result_t(first, t_first, f_first, carry);
}
#endif   // BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS

//////////////////////////////////////////////////////////////////////////////
// Inner stage (out_false).  out_true has been reduced to a flat [t_first,
// t_last) partner by the outer stage (t_last real when out_true is segmented,
// unreachable when it is flat).  The carry received from the outer stage is
// passed to the first leaf call.
//////////////////////////////////////////////////////////////////////////////

// out_false local flat: out_false is a flat bounded range, hand both to the
// leaf (the out_true partner end may still be a sentinel when out_true is flat).
template <class SrcIter, class Sent, class TIter, class TSent, class FIter,
          class Pred, class Cat>
BOOST_CONTAINER_FORCEINLINE
segquartet<SrcIter, TIter, FIter, unsigned>
partition_copy_false_bounded
   (SrcIter first, Sent last, TIter t_first, TSent t_last, FIter f_first, FIter f_last,
    Pred pred, unsigned carry, non_segmented_iterator_tag, const Cat &cat)
{
   return (partition_copy_leaf)(first, last, t_first, t_last, f_first, f_last, pred, carry, cat);
}

// out_false local segmented (nested): walk the bounded [f_first, f_last) span,
// recursing on the local structure and threading the source and the out_true
// partner.  A false carry continues in the next sub-segment; any other stop
// (source drained, true carry) is handed back to the outer stage.
template <class SrcIter, class Sent, class TIter, class TSent, class SegFIter,
          class Pred, class Cat>
segquartet<SrcIter, TIter, SegFIter, unsigned>
partition_copy_false_bounded
   (SrcIter first, Sent last, TIter t_first, TSent t_last, SegFIter f_first, SegFIter f_last,
    Pred pred, unsigned carry, segmented_iterator_tag, const Cat &cat)
{
   typedef segmented_iterator_traits<SegFIter> ftr;
   typedef typename ftr::local_iterator        floc_t;
   typedef typename ftr::segment_iterator      fseg_t;
   typedef typename segmented_iterator_traits<floc_t>::is_segmented_iterator floc_seg_t;

   fseg_t       fsfirst = ftr::segment(f_first);
   const fseg_t fslast  = ftr::segment(f_last);

   floc_t fb = ftr::local(f_first);

   for(;;) {
      const bool last_seg = fsfirst == fslast;
      const floc_t fe = last_seg ? ftr::local(f_last) : ftr::end(fsfirst);
      //Segments are never empty, but the range can end at the start of the last one
      if(BOOST_UNLIKELY(carry == 2u && fb == fe))
         return segquartet<SrcIter, TIter, SegFIter, unsigned>
            (first, t_first, ftr::compose(fsfirst, fb), carry);
      const segquartet<SrcIter, TIter, floc_t, unsigned> r = (partition_copy_false_bounded)
         (first, last, t_first, t_last, fb, fe, pred, carry, floc_seg_t(), cat);
      first   = r.first;
      t_first = r.second;
      carry   = r.fourth;
      if(carry != 2u || last_seg)
         return segquartet<SrcIter, TIter, SegFIter, unsigned>
            (first, t_first, ftr::compose(fsfirst, r.third), carry);
      ++fsfirst;
      fb = ftr::begin(fsfirst);
   }
}

// out_false flat (driver): out_false has no end, hand it to the leaf as an
// unbounded output alongside the (bounded or unbounded) out_true partner.
// It stops only when the source is drained or with a true carry.
template <class SrcIter, class Sent, class TIter, class TSent, class FIter,
          class Pred, class Cat>
BOOST_CONTAINER_FORCEINLINE
segtrio<SrcIter, TIter, FIter>
partition_copy_false_dispatch
   (SrcIter first, Sent last, TIter t_first, TSent t_last, FIter f_first,
    Pred pred, unsigned carry, non_segmented_iterator_tag, const Cat &cat)
{
   const segquartet<SrcIter, TIter, FIter, unsigned> r = (partition_copy_leaf)
      (first, last, t_first, t_last, f_first, unreachable_sentinel_t(), pred, carry, cat);
   return segtrio<SrcIter, TIter, FIter>(r.first, r.second, r.third);
}

// out_false segmented (driver): refill over out_false segments without an
// overall end.  A false carry continues in the rest of the first segment, then
// in the next ones.  Any other stop is handed back to the outer stage: then
// first != last means a true carry.
template <class SrcIter, class Sent, class TIter, class TSent, class SegFIter,
          class Pred, class Cat>
segtrio<SrcIter, TIter, SegFIter>
partition_copy_false_dispatch
   (SrcIter first, Sent last, TIter t_first, TSent t_last, SegFIter f_first,
    Pred pred, unsigned carry, segmented_iterator_tag, const Cat &cat)
{
   typedef segmented_iterator_traits<SegFIter> ftr;
   typedef typename ftr::local_iterator        floc_t;
   typedef typename ftr::segment_iterator      fseg_t;
   typedef typename segmented_iterator_traits<floc_t>::is_segmented_iterator floc_seg_t;

   fseg_t fs    = ftr::segment(f_first);
   floc_t f_lo  = ftr::local(f_first);
   //The first range is empty: end() of the first segment is computed only when
   //an element must be written.  If nothing is written, the segment can be
   //singular (e.g. begin() of an empty deque).
   floc_t f_end = f_lo;
   bool bounded = false;

   for(;;) {
      const segquartet<SrcIter, TIter, floc_t, unsigned> r = (partition_copy_false_bounded)
         (first, last, t_first, t_last, f_lo, f_end, pred, carry, floc_seg_t(), cat);
      first   = r.first;
      t_first = r.second;
      carry   = r.fourth;
      if(BOOST_LIKELY(carry != 2u))
         return segtrio<SrcIter, TIter, SegFIter>(first, t_first, ftr::compose(fs, r.third));
      if(!bounded) {
         bounded = true;
      }
      else {
         ++fs;
         f_lo = ftr::begin(fs);
      }
      f_end = ftr::end(fs);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Outer stage (out_true).  out_false is threaded whole (its own segmentation is
// resolved by the inner stage), so these functions never carry an out_false end.
// The inner stage returns with source left only with a true carry.
//////////////////////////////////////////////////////////////////////////////

// out_true local flat: out_true is now a flat bounded partner, peel out_false.
template <class SrcIter, class Sent, class TIter, class FIter,
          class Pred, class FTag, class Cat>
BOOST_CONTAINER_FORCEINLINE
segtrio<SrcIter, TIter, FIter>
partition_copy_true_bounded
   (SrcIter first, Sent last, TIter t_first, TIter t_last, FIter f_first,
    Pred pred, unsigned carry, non_segmented_iterator_tag, FTag f_tag, const Cat &cat)
{
   return (partition_copy_false_dispatch)
      (first, last, t_first, t_last, f_first, pred, carry, f_tag, cat);
}

// out_true local segmented (nested): walk the bounded [t_first, t_last) span,
// recursing on the local structure.  A recursive call returns with source left
// only with a true carry, which continues in the next sub-segment.
template <class SrcIter, class Sent, class SegTIter, class FIter,
          class Pred, class FTag, class Cat>
segtrio<SrcIter, SegTIter, FIter>
partition_copy_true_bounded
   (SrcIter first, Sent last, SegTIter t_first, SegTIter t_last, FIter f_first,
    Pred pred, unsigned carry, segmented_iterator_tag, FTag f_tag, const Cat &cat)
{
   typedef segmented_iterator_traits<SegTIter> ttr;
   typedef typename ttr::local_iterator        tloc_t;
   typedef typename ttr::segment_iterator      tseg_t;
   typedef typename segmented_iterator_traits<tloc_t>::is_segmented_iterator tloc_seg_t;

   tseg_t       tsfirst = ttr::segment(t_first);
   const tseg_t tslast  = ttr::segment(t_last);

   tloc_t tb = ttr::local(t_first);

   for(;;) {
      const bool last_seg = tsfirst == tslast;
      const tloc_t te = last_seg ? ttr::local(t_last) : ttr::end(tsfirst);
      //Segments are never empty, but the range can end at the start of the last one
      if(BOOST_UNLIKELY(carry == 1u && tb == te))
         return segtrio<SrcIter, SegTIter, FIter>(first, ttr::compose(tsfirst, tb), f_first);
      const segtrio<SrcIter, tloc_t, FIter> r = (partition_copy_true_bounded)
         (first, last, tb, te, f_first, pred, carry, tloc_seg_t(), f_tag, cat);
      first   = r.first;
      f_first = r.third;
      if(first == last || last_seg)
         return segtrio<SrcIter, SegTIter, FIter>(first, ttr::compose(tsfirst, r.second), f_first);
      carry = 1u;
      ++tsfirst;
      tb = ttr::begin(tsfirst);
   }
}

// out_true flat (driver): out_true has no end, peel out_false directly marking
// out_true unbounded.
template <class SrcIter, class Sent, class TIter, class FIter,
          class Pred, class FTag, class Cat>
BOOST_CONTAINER_FORCEINLINE
segtrio<SrcIter, TIter, FIter>
partition_copy_true_dispatch
   (SrcIter first, Sent last, TIter t_first, FIter f_first,
    Pred pred, non_segmented_iterator_tag, FTag f_tag, const Cat &cat)
{
   return (partition_copy_false_dispatch)
      (first, last, t_first, unreachable_sentinel_t(), f_first, pred, 0u, f_tag, cat);
}

// out_true segmented (driver): refill over out_true segments without an overall
// end.  A return with source left means a true carry, which continues in the
// rest of the first segment, then in the next ones.
template <class SrcIter, class Sent, class SegTIter, class FIter,
          class Pred, class FTag, class Cat>
segtrio<SrcIter, SegTIter, FIter>
partition_copy_true_dispatch
   (SrcIter first, Sent last, SegTIter t_first, FIter f_first,
    Pred pred, segmented_iterator_tag, FTag f_tag, const Cat &cat)
{
   typedef segmented_iterator_traits<SegTIter> ttr;
   typedef typename ttr::local_iterator        tloc_t;
   typedef typename ttr::segment_iterator      tseg_t;
   typedef typename segmented_iterator_traits<tloc_t>::is_segmented_iterator tloc_seg_t;

   tseg_t ts    = ttr::segment(t_first);
   tloc_t t_lo  = ttr::local(t_first);
   //The first range is empty: end() of the first segment is computed only when
   //an element must be written.  If nothing is written, the segment can be
   //singular (e.g. begin() of an empty deque).
   tloc_t t_end = t_lo;
   bool bounded = false;
   unsigned carry = 0u;

   for(;;) {
      const segtrio<SrcIter, tloc_t, FIter> r = (partition_copy_true_bounded)
         (first, last, t_lo, t_end, f_first, pred, carry, tloc_seg_t(), f_tag, cat);
      first   = r.first;
      f_first = r.third;
      if(BOOST_LIKELY(first == last))
         return segtrio<SrcIter, SegTIter, FIter>(first, ttr::compose(ts, r.second), f_first);
      carry = 1u;
      if(!bounded) {
         bounded = true;
      }
      else {
         ++ts;
         t_lo = ttr::begin(ts);
      }
      t_end = ttr::end(ts);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Source dispatch: walks the source (read pointer) segments.  The random
// access / unrolling fast path lives in the leaf, so the non-segmented source
// overload just enters the outer driver (which owns the output ends) and
// forwards its iterator category down to the leaf.
//////////////////////////////////////////////////////////////////////////////

template <class InIter, class Sent, class OutIter1, class OutIter2, class Pred, class Tag, class Cat>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c<
   !Tag::value || is_sentinel<Sent, InIter>::value, std::pair<OutIter1, OutIter2> >::type
segmented_partition_copy_dispatch
   (InIter first, Sent last, OutIter1 out_true, OutIter2 out_false, Pred pred, Tag, Cat cat)
{
   typedef typename segmented_iterator_traits<OutIter1>::is_segmented_iterator t_seg_t;
   typedef typename segmented_iterator_traits<OutIter2>::is_segmented_iterator f_seg_t;
   segtrio<InIter, OutIter1, OutIter2> r = (partition_copy_true_dispatch)
      (first, last, out_true, out_false, pred, t_seg_t(), f_seg_t(), cat);
   return std::pair<OutIter1, OutIter2>(r.second, r.third);
}

template <class SegIter, class OutIter1, class OutIter2, class Pred, class Cat>
std::pair<OutIter1, OutIter2>
segmented_partition_copy_dispatch
   (SegIter first, SegIter last, OutIter1 out_true, OutIter2 out_false, Pred pred, segmented_iterator_tag, Cat)
{
   typedef segmented_iterator_traits<SegIter>  traits;
   typedef typename traits::local_iterator   local_iterator;
   typedef typename traits::segment_iterator segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;
   typedef std::pair<OutIter1, OutIter2> pair_t;

   segment_iterator sfirst = traits::segment(first);
   segment_iterator slast  = traits::segment(last);

   local_iterator lb = traits::local(first);

   for(;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = sfirst == slast;
      const local_iterator le = last_seg ? traits::local(last) : traits::end(sfirst);
      // NOT converted to the scoped-const form: both members are full output
      // iterators, so carrying them through out_true/out_false costs two
      // copies per iteration.  There is no early return here to pay for them,
      // and it measured +3.6% on these walkers under GCC and +0.1% under
      // Clang, so direct threading stays.
      pair_t p = (segmented_partition_copy_dispatch)(lb, le, out_true, out_false, pred, is_local_seg_t(), local_cat_t());
      if(BOOST_UNLIKELY(last_seg))
         return p;

      //Middle segments keep their own call site: begin/end both come from
      //sfirst, so the leaf can be specialised for full segments
      for(++sfirst; sfirst != slast; ++sfirst) {
         p = (segmented_partition_copy_dispatch)(traits::begin(sfirst), traits::end(sfirst), p.first, p.second, pred, is_local_seg_t(), local_cat_t());
      }

      lb        = traits::begin(sfirst);
      out_true  = p.first;
      out_false = p.second;
   }
}

} // namespace detail_algo

//! Copies elements from [first, last) to one of two output ranges
//! depending on whether \c pred returns true or false.
//! Returns a pair of output iterators past the last elements written.
template <class InIter, class Sent, class OutIter1, class OutIter2, class Pred>
BOOST_CONTAINER_FORCEINLINE
std::pair<OutIter1, OutIter2>
segmented_partition_copy(InIter first, Sent last, OutIter1 out_true, OutIter2 out_false, Pred pred)
{
   typedef segmented_iterator_traits<InIter> traits;
   return detail_algo::segmented_partition_copy_dispatch
      (first, last, out_true, out_false, pred, typename traits::is_segmented_iterator(), typename iterator_traits<InIter>::iterator_category());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_PARTITION_COPY_HPP
