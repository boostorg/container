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
#include <boost/container/experimental/segmented_iterator_traits.hpp>
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

template <std::size_t BlockSize, class RASrcIter, class TIter, class TSent,
          class FIter, class FSent, class Pred, class Diff>
BOOST_CONTAINER_FORCEINLINE segquartet<RASrcIter, TIter, FIter, Diff>
partition_copy_cleanup_blocks
   (RASrcIter cur, TIter t_first, TSent t_last, FIter f_first, FSent f_last,
    Pred pred, Diff avail, dtl::true_type)
{
   const Diff block_size = static_cast<Diff>(BlockSize);
   while((partition_copy_room_enough<BlockSize>)
            (t_first, t_last, f_first, f_last, avail)) {
      avail -= block_size;
      BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL
      for(Diff chunk = block_size; chunk; ) {
         --chunk;
         if(pred(*cur)) {
            *t_first = *cur;
            ++t_first;
         }
         else {
            *f_first = *cur;
            ++f_first;
         }
         ++cur;
      }
   }
   return segquartet<RASrcIter, TIter, FIter, Diff>
      (cur, t_first, f_first, avail);
}

template <std::size_t BlockSize, class RASrcIter, class TIter, class TSent,
          class FIter, class FSent, class Pred, class Diff>
BOOST_CONTAINER_FORCEINLINE segquartet<RASrcIter, TIter, FIter, Diff>
partition_copy_cleanup_blocks
   (RASrcIter cur, TIter t_first, TSent, FIter f_first, FSent, Pred, Diff avail,
    dtl::false_type)
{
   return segquartet<RASrcIter, TIter, FIter, Diff>
      (cur, t_first, f_first, avail);
}

template <class It>
struct pc_output_is_ra
{
   static const bool value = dtl::is_convertible
      < typename boost::container::iterator_traits<It>::iterator_category
      , std::random_access_iterator_tag >::value;
};

template <class SrcIter, class Sent, class TIter, class TSent, class FIter, class FSent,
          class Pred, class SrcCat>
BOOST_CONTAINER_FORCEINLINE
segquartet<SrcIter, TIter, FIter, bool>
partition_copy_leaf
   (SrcIter first, Sent last, TIter t_first, TSent t_last, FIter f_first, FSent f_last,
    Pred pred, const SrcCat &)
{
   // fourth says whether [f_first, f_last) filled, the only stop a caller
   // walking out_false segments can resume from, so a caller that would
   // otherwise also re-test source exhaustion just tests the flag.  A source and
   // an output running out on the same element counts as source exhaustion,
   // which is what those callers require, so the out_false exits report the flag
   // as "source left".
   //
   // [alg.partitions] mandates exactly last - first applications of pred.
   // Testing an element, discovering that the output it belongs to is full and
   // returning makes the enclosing walker call this leaf again on the same
   // element, which re-applies pred.  Each output is therefore tested once on
   // entry and again after each write to it: when one fills, `first` has already
   // moved past the element that was written, so the next call resumes on an
   // untested element, and the loop body can take for granted that both outputs
   // have room.  With unreachable_sentinel_t outputs the tests fold away, so the
   // flat path is unchanged.
   if(BOOST_CONTAINER_SEG_UNLIKELY(t_first == t_last))
      return segquartet<SrcIter, TIter, FIter, bool>(first, t_first, f_first, false);
   if(BOOST_CONTAINER_SEG_UNLIKELY(f_first == f_last))
      return segquartet<SrcIter, TIter, FIter, bool>(first, t_first, f_first, first != last);

   bool false_output_full = false;
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         *t_first = *first;
         ++t_first;
         if(BOOST_CONTAINER_SEG_UNLIKELY(t_first == t_last)) {
            ++first;
            break;
         }
      }
      else {
         *f_first = *first;
         ++f_first;
         if(BOOST_CONTAINER_SEG_UNLIKELY(f_first == f_last)) {
            ++first;
            false_output_full = first != last;
            break;
         }
      }
   }
   return segquartet<SrcIter, TIter, FIter, bool>
      (first, t_first, f_first, false_output_full);
}

// Random-access-source fast path (needs random-access outputs too).  Process
// fixed 32-element source blocks while both outputs have room for the worst
// case (all 32 elements routed to either output), use 8-element cleanup blocks
// when an output is bounded, then finish with the generic checked loop.  An
// unbounded output reports the available source count as its room.
template <class RASrcIter, class TIter, class TSent, class FIter, class FSent, class Pred>
BOOST_CONTAINER_FORCEINLINE
typename algo_enable_if_c
   < pc_output_is_ra<TIter>::value && pc_output_is_ra<FIter>::value
   , segquartet<RASrcIter, TIter, FIter, bool> >::type
partition_copy_leaf
   (RASrcIter first, RASrcIter last, TIter t_first, TSent t_last, FIter f_first, FSent f_last,
    Pred pred, const std::random_access_iterator_tag &)
{
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;
   typedef segquartet<RASrcIter, TIter, FIter, difference_type> cleanup_result;

   const cleanup_result r32 = (partition_copy_cleanup_blocks<32>)
      (first, t_first, t_last, f_first, f_last, pred, last - first,
       dtl::true_type());

   typedef dtl::integral_constant
      < bool
      , !dtl::is_same<TSent, unreachable_sentinel_t>::value ||
        !dtl::is_same<FSent, unreachable_sentinel_t>::value
      > has_bounded_output_t;
   const cleanup_result r8 = (partition_copy_cleanup_blocks<8>)
      (r32.first, r32.second, t_last, r32.third, f_last, pred, r32.fourth,
       has_bounded_output_t());

   return (partition_copy_leaf)
      (r8.first, last, r8.second, t_last, r8.third, f_last, pred,
       int());
}

//////////////////////////////////////////////////////////////////////////////
// Inner stage (out_false).  out_true has been reduced to a flat [t_first,
// t_last) partner by the outer stage (t_last real when out_true is segmented,
// unreachable when it is flat).
//////////////////////////////////////////////////////////////////////////////

// out_false local flat: out_false is a flat bounded range, hand both to the
// leaf (the out_true partner end may still be a sentinel when out_true is flat).
template <class SrcIter, class Sent, class TIter, class TSent, class FIter,
          class Pred, class Cat>
BOOST_CONTAINER_FORCEINLINE
segquartet<SrcIter, TIter, FIter, bool>
partition_copy_false_bounded
   (SrcIter first, Sent last, TIter t_first, TSent t_last, FIter f_first, FIter f_last,
    Pred pred, non_segmented_iterator_tag, const Cat &cat)
{
   return (partition_copy_leaf)(first, last, t_first, t_last, f_first, f_last, pred, cat);
}

// out_false local segmented (nested): walk the bounded [f_first, f_last) span,
// recursing on the local structure.  Follows the classic same-segment /
// initial-middle-final segmented walk, threading the source and the out_true
// partner.  The recursive result says whether the sub-segment filled, the only
// stop that can be resumed by moving on to the next one; anything else (source
// drained, out_true blocked) has to be handed back to the outer stage.
template <class SrcIter, class Sent, class TIter, class TSent, class SegFIter,
          class Pred, class Cat>
segquartet<SrcIter, TIter, SegFIter, bool>
partition_copy_false_bounded
   (SrcIter first, Sent last, TIter t_first, TSent t_last, SegFIter f_first, SegFIter f_last,
    Pred pred, segmented_iterator_tag, const Cat &cat)
{
   typedef segmented_iterator_traits<SegFIter> ftr;
   typedef typename ftr::local_iterator        floc_t;
   typedef typename ftr::segment_iterator      fseg_t;
   typedef typename segmented_iterator_traits<floc_t>::is_segmented_iterator floc_seg_t;

   fseg_t       fsfirst = ftr::segment(f_first);
   const fseg_t fslast  = ftr::segment(f_last);

   floc_t fb = ftr::local(f_first);

   for(;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = fsfirst == fslast;
      const floc_t fe = last_seg ? ftr::local(f_last) : ftr::end(fsfirst);
      {
         const segquartet<SrcIter, TIter, floc_t, bool> r = (partition_copy_false_bounded)
            (first, last, t_first, t_last, fb, fe, pred, floc_seg_t(), cat);
         first   = r.first;
         t_first = r.second;
         if(last_seg || BOOST_CONTAINER_SEG_UNLIKELY(!r.fourth))
            return segquartet<SrcIter, TIter, SegFIter, bool>
               (first, t_first, ftr::compose(fsfirst, r.third), r.fourth);
      }

      //Middle segments keep their own call site: begin/end both come from
      //fsfirst, so the leaf can be specialised for full segments
      for(++fsfirst; fsfirst != fslast; ++fsfirst) {
         const segquartet<SrcIter, TIter, floc_t, bool> r = (partition_copy_false_bounded)
            (first, last, t_first, t_last, ftr::begin(fsfirst), ftr::end(fsfirst), pred, floc_seg_t(), cat);
         first   = r.first;
         t_first = r.second;
         if(BOOST_CONTAINER_SEG_UNLIKELY(!r.fourth))
            return segquartet<SrcIter, TIter, SegFIter, bool>
               (first, t_first, ftr::compose(fsfirst, r.third), false);
      }

      fb = ftr::begin(fsfirst);
   }
}

// out_false flat (driver): out_false has no end, hand it to the leaf as an
// unbounded output alongside the (bounded or unbounded) out_true partner.
template <class SrcIter, class Sent, class TIter, class TSent, class FIter,
          class Pred, class Cat>
BOOST_CONTAINER_FORCEINLINE
segtrio<SrcIter, TIter, FIter>
partition_copy_false_dispatch
   (SrcIter first, Sent last, TIter t_first, TSent t_last, FIter f_first,
    Pred pred, non_segmented_iterator_tag, const Cat &cat)
{
   const segquartet<SrcIter, TIter, FIter, bool> r = (partition_copy_leaf)
      (first, last, t_first, t_last, f_first, unreachable_sentinel_t(), pred, cat);
   return segtrio<SrcIter, TIter, FIter>(r.first, r.second, r.third);
}

// out_false segmented (driver): refill over out_false segments without an
// overall end, bounding each segment with its real end() and delegating to the
// worker.  The current out_false segment filling is the only stop that can be
// resumed here, so the flag alone decides whether to advance to the next
// segment or hand control back to the out_true stage.
template <class SrcIter, class Sent, class TIter, class TSent, class SegFIter,
          class Pred, class Cat>
segtrio<SrcIter, TIter, SegFIter>
partition_copy_false_dispatch
   (SrcIter first, Sent last, TIter t_first, TSent t_last, SegFIter f_first,
    Pred pred, segmented_iterator_tag, const Cat &cat)
{
   typedef segmented_iterator_traits<SegFIter> ftr;
   typedef typename ftr::local_iterator        floc_t;
   typedef typename ftr::segment_iterator      fseg_t;
   typedef typename segmented_iterator_traits<floc_t>::is_segmented_iterator floc_seg_t;

   fseg_t fs   = ftr::segment(f_first);
   floc_t f_lo = ftr::local(f_first);
   for(;;) {
      segquartet<SrcIter, TIter, floc_t, bool> r = (partition_copy_false_bounded)
         (first, last, t_first, t_last, f_lo, ftr::end(fs), pred, floc_seg_t(), cat);
      first   = r.first;
      t_first = r.second;
      if(BOOST_CONTAINER_SEG_UNLIKELY(!r.fourth))
         return segtrio<SrcIter, TIter, SegFIter>(first, t_first, ftr::compose(fs, r.third));
      ++fs;
      f_lo = ftr::begin(fs);
   }
}

//////////////////////////////////////////////////////////////////////////////
// Outer stage (out_true).  out_false is threaded whole (its own segmentation is
// resolved by the inner stage), so these functions never carry an out_false end.
//////////////////////////////////////////////////////////////////////////////

// out_true local flat: out_true is now a flat bounded partner, peel out_false.
template <class SrcIter, class Sent, class TIter, class FIter,
          class Pred, class FTag, class Cat>
BOOST_CONTAINER_FORCEINLINE
segtrio<SrcIter, TIter, FIter>
partition_copy_true_bounded
   (SrcIter first, Sent last, TIter t_first, TIter t_last, FIter f_first,
    Pred pred, non_segmented_iterator_tag, FTag f_tag, const Cat &cat)
{
   return (partition_copy_false_dispatch)
      (first, last, t_first, t_last, f_first, pred, f_tag, cat);
}

// out_true local segmented (nested): walk the bounded [t_first, t_last) span,
// recursing on the local structure.  Since out_false is fully handled by the
// inner false stage below, a recursive call returns only when the source drains
// or its out_true sub-range fills (a true element was blocked); the latter means
// the sub-segment is full, so we advance to the next one.  Follows the classic
// same-segment / initial-middle-final segmented walk.
template <class SrcIter, class Sent, class SegTIter, class FIter,
          class Pred, class FTag, class Cat>
segtrio<SrcIter, SegTIter, FIter>
partition_copy_true_bounded
   (SrcIter first, Sent last, SegTIter t_first, SegTIter t_last, FIter f_first,
    Pred pred, segmented_iterator_tag, FTag f_tag, const Cat &cat)
{
   typedef segmented_iterator_traits<SegTIter> ttr;
   typedef typename ttr::local_iterator        tloc_t;
   typedef typename ttr::segment_iterator      tseg_t;
   typedef typename segmented_iterator_traits<tloc_t>::is_segmented_iterator tloc_seg_t;

   tseg_t       tsfirst = ttr::segment(t_first);
   const tseg_t tslast  = ttr::segment(t_last);

   tloc_t tb = ttr::local(t_first);

   for(;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = tsfirst == tslast;
      const tloc_t te = last_seg ? ttr::local(t_last) : ttr::end(tsfirst);
      {
         const segtrio<SrcIter, tloc_t, FIter> r = (partition_copy_true_bounded)
            (first, last, tb, te, f_first, pred, tloc_seg_t(), f_tag, cat);
         first   = r.first;
         f_first = r.third;
         if(last_seg || BOOST_CONTAINER_SEG_UNLIKELY(first == last))
            return segtrio<SrcIter, SegTIter, FIter>(first, ttr::compose(tsfirst, r.second), f_first);
      }

      //Middle segments keep their own call site: begin/end both come from
      //tsfirst, so the leaf can be specialised for full segments
      for(++tsfirst; tsfirst != tslast; ++tsfirst) {
         const segtrio<SrcIter, tloc_t, FIter> r = (partition_copy_true_bounded)
            (first, last, ttr::begin(tsfirst), ttr::end(tsfirst), f_first, pred, tloc_seg_t(), f_tag, cat);
         first   = r.first;
         f_first = r.third;
         if(BOOST_CONTAINER_SEG_UNLIKELY(first == last))
            return segtrio<SrcIter, SegTIter, FIter>(first, ttr::compose(tsfirst, r.second), f_first);
      }

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
      (first, last, t_first, unreachable_sentinel_t(), f_first, pred, f_tag, cat);
}

// out_true segmented (driver): refill over out_true segments without an overall
// end, bounding each segment with its real end() and delegating to the worker.
// out_false is unbounded here, so the only stop is source exhaustion; otherwise
// the current out_true segment filled, so advance to the next one.
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

   tseg_t ts   = ttr::segment(t_first);
   tloc_t t_lo = ttr::local(t_first);
   for(;;) {
      segtrio<SrcIter, tloc_t, FIter> r = (partition_copy_true_bounded)
         (first, last, t_lo, ttr::end(ts), f_first, pred, tloc_seg_t(), f_tag, cat);
      first   = r.first;
      f_first = r.third;
      if(BOOST_CONTAINER_SEG_UNLIKELY(first == last))
         return segtrio<SrcIter, SegTIter, FIter>(first, ttr::compose(ts, r.second), f_first);
      ++ts;
      t_lo = ttr::begin(ts);
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
      if(BOOST_CONTAINER_SEG_UNLIKELY(last_seg))
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
