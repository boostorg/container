//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_N_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_N_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/experimental/detail/segmented_common_algo.hpp>
#include <cstddef>
#include <boost/container/detail/iterators.hpp>
#include <boost/container/detail/std_fwd.hpp>

namespace boost {
namespace container {

template <class OutIt, class Size, class Generator>
OutIt segmented_generate_n(OutIt first, Size count, Generator gen);

namespace detail_algo {

#if BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS
template <class OutIter, class Generator>
BOOST_CONTAINER_FORCEINLINE
segduo<OutIter, std::size_t> generate_n_scan
   ( OutIter first, OutIter last, std::size_t count
   , Generator &BOOST_RESTRICT gen, non_segmented_iterator_tag, const std::random_access_iterator_tag &)
{
   //count and the segment length are std::size_t (see segmented_generate_n): no truncation
   const std::size_t range_sz = static_cast<std::size_t>(last - first);
   #if BOOST_CONTAINER_SEGMENTED_SPLIT_FULL_SEGMENT_LOOP
   //Two loops: for a full segment of a fixed size range_sz is a compile-time constant
   if (count >= range_sz) {
      count -= range_sz;
      BOOST_CONTAINER_SEGMENTED_UNROLL(4)
      for (std::size_t cnt = 0; cnt != range_sz; ++first, ++cnt)
         *first = gen();
      return segduo<OutIter, std::size_t>(first, count);
   }
   else {
      BOOST_CONTAINER_SEGMENTED_UNROLL(4)
      for (std::size_t cnt = 0; cnt != count; ++first, ++cnt)
         *first = gen();
      return segduo<OutIter, std::size_t>(first, std::size_t(0));
   }
   #else
   const std::size_t length = count < range_sz ? count : range_sz;
   count -= length;
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for (std::size_t cnt = 0; cnt != length; ++first, ++cnt){
      *first = gen();
   }
   return segduo<OutIter, std::size_t>(first, count);
   #endif
}
#endif   // BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS

template <class OutIter, class Generator, class Cat>
BOOST_CONTAINER_FORCEINLINE
segduo<OutIter, std::size_t> generate_n_scan
   (OutIter first, OutIter last, std::size_t count, Generator &BOOST_RESTRICT gen, non_segmented_iterator_tag, const Cat &)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for (; count > 0 && first != last; ++first, --count)
      *first = gen();

   return segduo<OutIter, std::size_t>(first, count);
}

template <class SegIt, class Generator, class Cat>
segduo<SegIt, std::size_t> generate_n_scan(SegIt first, SegIt last, std::size_t count, Generator& BOOST_RESTRICT gen, segmented_iterator_tag, const Cat &)
{
   typedef segmented_iterator_traits<SegIt>                                            traits;
   typedef typename traits::local_iterator                                             local_iterator;
   typedef typename traits::segment_iterator                                           segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator   is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category                 local_cat_t;

   segment_iterator       scur  = traits::segment(first);
   segment_iterator const slast = traits::segment(last);

   local_iterator lb = traits::local(first);

   for(;;) {
      //Partial segments (first and last) share this call site
      const bool last_seg = scur == slast;
      const local_iterator le = last_seg ? traits::local(last) : traits::end(scur);
      {
         const segduo<local_iterator, std::size_t> r = generate_n_scan(lb, le, count, gen, is_local_seg_t(), local_cat_t());
         count = r.second;
         if (BOOST_UNLIKELY(!count))
            return segduo<SegIt, std::size_t>(traits::compose(scur, r.first), count);
         if (BOOST_UNLIKELY(last_seg))
            return segduo<SegIt, std::size_t>(last, count);
      }

      //Middle segments keep their own call site: begin/end both come from
      //scur, so the leaf can be specialised for full segments
      for (++scur; scur != slast; ++scur) {
         const segduo<local_iterator, std::size_t> r = generate_n_scan(traits::begin(scur), traits::end(scur), count, gen, is_local_seg_t(), local_cat_t());
         count = r.second;
         if (BOOST_UNLIKELY(!count))
            return segduo<SegIt, std::size_t>(traits::compose(scur, r.first), count);
      }

      lb = traits::begin(scur);
   }
}

template <class SegIter, class Generator>
SegIter segmented_generate_n_ref
   (SegIter first, std::size_t count, Generator& BOOST_RESTRICT gen, segmented_iterator_tag)
{
   typedef segmented_iterator_traits<SegIter> traits;
   typedef typename traits::local_iterator    local_iterator;
   typedef typename traits::segment_iterator  segment_iterator;
   typedef typename segmented_iterator_traits<local_iterator>::is_segmented_iterator is_local_seg_t;
   typedef typename iterator_traits<local_iterator>::iterator_category local_cat_t;

   segment_iterator scur = traits::segment(first);
   local_iterator   lcur;

   {
      const segduo<local_iterator, std::size_t> r = generate_n_scan(traits::local(first), traits::end(scur), count, gen, is_local_seg_t(), local_cat_t());
      lcur  = r.first;
      count = r.second;
   }

   if(count){
      ++scur;
      while(1) {
         const segduo<local_iterator, std::size_t> r = generate_n_scan(traits::begin(scur), traits::end(scur), count, gen, is_local_seg_t(), local_cat_t());
         lcur  = r.first;
         count = r.second;
         if(BOOST_UNLIKELY(count == 0))
            break;
         ++scur;
      }
   }
   return traits::compose(scur, lcur);
}

template <class OutIt, class Generator>
OutIt segmented_generate_n_ref
   (OutIt first, std::size_t count, Generator& BOOST_RESTRICT gen, non_segmented_iterator_tag)
{
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; count > 0; ++first, --count)
      *first = gen();
   return first;
}

} // namespace detail_algo

//! Assigns the result of successive calls to \c gen to the first
//! \c count elements starting at \c first. Generator state is
//! preserved across segment boundaries.
//! Returns an iterator past the last generated element. If \c count is not
//! positive, does nothing and returns \c first.
template <class OutIt, class Size, class Generator>
BOOST_CONTAINER_FORCEINLINE
OutIt segmented_generate_n(OutIt first, Size count, Generator gen)
{
   typedef segmented_iterator_traits<OutIt> traits;
   if (BOOST_UNLIKELY(!(count > 0)))
      return first;
   //A valid count is not bigger than the range, so it fits in std::size_t. The internal
   //functions use only std::size_t: a segment length converted to a narrower Size is truncated.
   return detail_algo::segmented_generate_n_ref(first, static_cast<std::size_t>(count), gen,
      typename traits::is_segmented_iterator());
}

} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif // BOOST_CONTAINER_EXPERIMENTAL_SEGMENTED_GENERATE_N_HPP
