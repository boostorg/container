//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//
// Simple segmented container for testing segmented algorithms.
// Models a vector-of-vectors with a sentinel empty segment at the end.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_TEST_SEGMENTED_TEST_HELPER_HPP
#define BOOST_CONTAINER_TEST_SEGMENTED_TEST_HELPER_HPP

#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/container/vector.hpp>
#include <cstddef>
#include <ostream>

namespace test_detail {

class movable_int
{
   BOOST_MOVABLE_BUT_NOT_COPYABLE(movable_int)

   int val_;
public:
   explicit movable_int(int v = 0) : val_(v) {}
   movable_int(BOOST_RV_REF(movable_int) other) BOOST_NOEXCEPT
      : val_(other.val_)
   { other.val_ = -1; }

   movable_int& operator=(BOOST_RV_REF(movable_int) other) BOOST_NOEXCEPT
   { val_ = other.val_; other.val_ = -1; return *this; }

   int value() const { return val_; }

   friend bool operator==(const movable_int& a, const movable_int& b)      { return a.val_ == b.val_; }
   friend bool operator!=(const movable_int& a, const movable_int& b)      { return a.val_ != b.val_; }
   friend std::ostream& operator<<(std::ostream& os, const movable_int& m) { return os << m.val_; }
};

// Cat selects the advertised iterator category.  Algorithms such as
// segmented_find_last have distinct segmented implementations for forward and
// for bidirectional iterators, so both have to be instantiable from a test.
template<class T, class Cat = std::bidirectional_iterator_tag>
class seg_vector_iterator
{
public:
   typedef T                               value_type;
   typedef T&                              reference;
   typedef T*                              pointer;
   typedef std::ptrdiff_t                  difference_type;
   typedef Cat                             iterator_category;

   typedef boost::container::vector<boost::container::vector<T> >    segments_type;
   typedef typename segments_type::iterator          seg_iter_t;
   typedef typename boost::container::vector<T>::iterator         local_iter_t;

   seg_iter_t    seg_;
   local_iter_t  local_;

   seg_vector_iterator()
      : seg_(), local_()
   {}

   seg_vector_iterator(seg_iter_t s, local_iter_t l)
      : seg_(s), local_(l)
   {}

   reference operator*()  const { return *local_; }
   pointer   operator->() const { return &*local_; }

   seg_vector_iterator& operator++()
   {
      ++local_;
      // Loop, not if: any number of empty segments may follow.  The loop is
      // bounded by the sentinel segment, which is never empty.
      while(local_ == seg_->end()) {
         ++seg_;
         local_ = seg_->begin();
      }
      return *this;
   }

   seg_vector_iterator operator++(int)
   {
      seg_vector_iterator tmp = *this;
      ++*this;
      return tmp;
   }

   seg_vector_iterator& operator--()
   {
      while(local_ == seg_->begin()) {
         --seg_;
         local_ = seg_->end();
      }
      --local_;
      return *this;
   }

   seg_vector_iterator operator--(int)
   {
      seg_vector_iterator tmp = *this;
      --*this;
      return tmp;
   }

   friend bool operator==(const seg_vector_iterator& a, const seg_vector_iterator& b)
   { return a.seg_ == b.seg_ && a.local_ == b.local_; }

   friend bool operator!=(const seg_vector_iterator& a, const seg_vector_iterator& b)
   { return !(a == b); }
};

template<class T, class Cat = std::bidirectional_iterator_tag>
class seg_vector
{
   // Last entry is always a sentinel segment, following the convention from
   // Austern's segmented-iterator paper.  It holds one dummy element that is
   // never part of any range: end() points at it, so it is never read.  The
   // dummy is what makes the sentinel distinguishable from an empty interior
   // segment, which is in turn what lets the normalising loops in the
   // iterators skip empty segments and still know where to stop.  T therefore
   // has to be constructible from int, which add_segment_from_ints already
   // requires of every element type used with this helper.
   boost::container::vector<boost::container::vector<T> > segments_;

   BOOST_COPYABLE_AND_MOVABLE(seg_vector)

public:
   typedef seg_vector_iterator<T, Cat> iterator;

   seg_vector()
   {
      boost::container::vector<T> sentinel;
      sentinel.push_back(T(0));
      segments_.push_back(boost::move(sentinel));
   }

   seg_vector(const seg_vector &x)
      : segments_(x.segments_)
   {}

   seg_vector(BOOST_RV_REF(seg_vector) x)
      : segments_(boost::move(x.segments_))
   {}

   seg_vector &operator= (BOOST_RV_REF(seg_vector) x)
   {
      segments_ = boost::move(x.segments_);
      return *this;
   }

   seg_vector &operator= (BOOST_COPY_ASSIGN_REF(seg_vector) x)
   {
      segments_ = x.segments_;
      return *this;
   }

   void add_segment(std::size_t n, const T& val)
   {
      segments_.insert(segments_.end() - 1, boost::container::vector<T>(n, val));
   }

   template<class InpIt>
   void add_segment_range(InpIt first, InpIt last)
   {
      segments_.insert(segments_.end() - 1, boost::container::vector<T>(first, last));
   }

   void add_segment_from_ints(const int* first, const int* last)
   {
      boost::container::vector<T> v;
      v.reserve(static_cast<std::size_t>(last - first));
      for(; first != last; ++first)
         v.push_back(T(*first));
      segments_.insert(segments_.end() - 1, boost::move(v));
   }

   iterator begin()
   {
      typename boost::container::vector<boost::container::vector<T> >::iterator s = segments_.begin();
      typename boost::container::vector<T>::iterator l = s->begin();
      while(l == s->end()) {        // skip empty leading segments
         ++s;
         l = s->begin();
      }
      return iterator(s, l);
   }

   iterator end()
   {
      typename boost::container::vector<boost::container::vector<T> >::iterator s = segments_.end() - 1;
      return iterator(s, s->begin());
   }

   std::size_t total_size() const
   {
      std::size_t n = 0;
      for(std::size_t i = 0; i + 1 < segments_.size(); ++i)
         n += segments_[i].size();
      return n;
   }

   boost::container::vector<boost::container::vector<T> >& segments() { return segments_; }
   const boost::container::vector<boost::container::vector<T> >& segments() const { return segments_; }
};

template<class Iter>
class sentinel_wrapper
{
   Iter it_;
public:
   explicit sentinel_wrapper(Iter it) : it_(it) {}

   operator Iter() const { return it_; }

   friend bool operator==(const Iter& a, const sentinel_wrapper& b) { return a == b.it_; }
   friend bool operator!=(const Iter& a, const sentinel_wrapper& b) { return !(a == b.it_); }
   friend bool operator==(const sentinel_wrapper& a, const Iter& b) { return a.it_ == b; }
   friend bool operator!=(const sentinel_wrapper& a, const Iter& b) { return !(a.it_ == b); }
};

template<class Iter>
sentinel_wrapper<Iter> make_sentinel(Iter it) { return sentinel_wrapper<Iter>(it); }

template<class Iter>
class sized_sentinel_wrapper
{
   Iter it_;
public:
   explicit sized_sentinel_wrapper(Iter it) : it_(it) {}

   operator Iter() const { return it_; }

   friend bool operator==(const Iter& a, const sized_sentinel_wrapper& b) { return a == b.it_; }
   friend bool operator!=(const Iter& a, const sized_sentinel_wrapper& b) { return !(a == b.it_); }
   friend bool operator==(const sized_sentinel_wrapper& a, const Iter& b) { return a.it_ == b; }
   friend bool operator!=(const sized_sentinel_wrapper& a, const Iter& b) { return !(a.it_ == b); }

   friend typename boost::container::iterator_traits<Iter>::difference_type
   operator-(const sized_sentinel_wrapper& a, const Iter& b) { return a.it_ - b; }

   friend typename boost::container::iterator_traits<Iter>::difference_type
   operator-(const Iter& a, const sized_sentinel_wrapper& b) { return a - b.it_; }
};

template<class Iter>
sized_sentinel_wrapper<Iter> make_sized_sentinel(Iter it) { return sized_sentinel_wrapper<Iter>(it); }

template<class T, class Cat = std::bidirectional_iterator_tag>
class seg2_vector_iterator
{
public:
   typedef T                               value_type;
   typedef T&                              reference;
   typedef T*                              pointer;
   typedef std::ptrdiff_t                  difference_type;
   typedef Cat                             iterator_category;

   typedef boost::container::vector<seg_vector<T, Cat> > segments_type;
   typedef typename segments_type::iterator seg_iter_t;
   typedef seg_vector_iterator<T, Cat>      local_iter_t;

   seg_iter_t   seg_;
   local_iter_t local_;

   seg2_vector_iterator()
      : seg_(), local_()
   {}

   seg2_vector_iterator(seg_iter_t s, local_iter_t l)
      : seg_(s), local_(l)
   {}

   reference operator*()  const { return *local_; }
   pointer   operator->() const { return &*local_; }

   seg2_vector_iterator& operator++()
   {
      ++local_;
      // As in seg_vector_iterator: skip however many outer segments hold a
      // logically empty inner container.
      while(local_ == seg_->end()) {
         ++seg_;
         local_ = seg_->begin();
      }
      return *this;
   }

   seg2_vector_iterator operator++(int)
   {
      seg2_vector_iterator tmp = *this;
      ++*this;
      return tmp;
   }

   seg2_vector_iterator& operator--()
   {
      while(local_ == seg_->begin()) {
         --seg_;
         local_ = seg_->end();
      }
      --local_;
      return *this;
   }

   seg2_vector_iterator operator--(int)
   {
      seg2_vector_iterator tmp = *this;
      --*this;
      return tmp;
   }

   friend bool operator==(const seg2_vector_iterator& a, const seg2_vector_iterator& b)
   { return a.seg_ == b.seg_ && a.local_ == b.local_; }

   friend bool operator!=(const seg2_vector_iterator& a, const seg2_vector_iterator& b)
   { return !(a == b); }
};

template<class T, class Cat = std::bidirectional_iterator_tag>
class seg2_vector
{
   boost::container::vector<seg_vector<T, Cat> > segments_;

public:
   typedef seg2_vector_iterator<T, Cat> iterator;

   seg2_vector()
   {
      // The sentinel outer segment must be non-empty as seen through its own
      // begin()/end(), for the same reason seg_vector's sentinel segment is:
      // a logically empty inner container is a legal interior segment here.
      static const int dummy = 0;
      seg_vector<T, Cat> sentinel;
      sentinel.add_segment_from_ints(&dummy, &dummy + 1);
      segments_.push_back(boost::move(sentinel));
   }

   void add_segment(const seg_vector<T, Cat>& sv)
   {
      segments_.insert(segments_.end() - 1, sv);
   }

   //! Move overload, so that a segment can be handed over without requiring
   //! T to be copyable.  Without it seg2_vector<movable_int> is unusable.
   void add_segment(BOOST_RV_REF(seg_vector<T BOOST_MOVE_I Cat>) sv)
   {
      segments_.insert(segments_.end() - 1, boost::move(sv));
   }

   template<class InpIt>
   void add_flat_segment_range(InpIt first, InpIt last)
   {
      seg_vector<T, Cat> sv;
      sv.add_segment_range(first, last);
      segments_.insert(segments_.end() - 1, boost::move(sv));
   }

   void add_flat_segment_from_ints(const int* first, const int* last)
   {
      seg_vector<T, Cat> sv;
      sv.add_segment_from_ints(first, last);
      segments_.insert(segments_.end() - 1, boost::move(sv));
   }

   iterator begin()
   {
      typename boost::container::vector<seg_vector<T, Cat> >::iterator s = segments_.begin();
      seg_vector_iterator<T, Cat> l = s->begin();
      while(l == s->end()) {        // skip leading empty inner containers
         ++s;
         l = s->begin();
      }
      return iterator(s, l);
   }

   iterator end()
   {
      typename boost::container::vector<seg_vector<T, Cat> >::iterator s = segments_.end() - 1;
      return iterator(s, s->begin());
   }

   std::size_t total_size() const
   {
      std::size_t n = 0;
      for(std::size_t i = 0; i + 1 < segments_.size(); ++i)
         n += segments_[i].total_size();
      return n;
   }

   boost::container::vector<seg_vector<T, Cat> >& segments() { return segments_; }
   const boost::container::vector<seg_vector<T, Cat> >& segments() const { return segments_; }
};

//////////////////////////////////////////////////////////////////////////////
//
// Shape-driven construction of 1- and 2-level segmented ranges.
//
// Each level of a segmented algorithm independently decides between a
// "single segment" branch (the whole sub-range lives in one segment of that
// level) and a "multi segment" branch.  A *branch spec* is a string with one
// character per level, outermost first: 's' drives that level down the
// single-segment branch, 'm' down the multi-segment one.  The length of the
// spec is the segmentation depth, so "m" and "s" describe a seg_vector and
// "mm", "ms", "sm" and "ss" describe a seg2_vector.  Only those two depths
// exist, because seg2_vector is the deepest container modelled here.
//
// A third character, 'e', is a multi-segment level that additionally carries
// empty segments: one before the data, one between the two halves and one
// after.  At the outer level of a seg2_vector those are outer segments whose
// inner container is logically empty.  Empty segments are where carry-across-
// boundary bugs live, and 'e' is the only way to produce them.  'e' is not
// part of the default enumeration: for_each_shape() and friends walk the m/s
// specs exactly as they always did, and the _all variants walk both tables.
//
// make_range() builds a container of exactly n+1 elements.  The first n hold
// the requested values and form the range [c.begin(), iter_at(c, n)); the
// trailing element holds a filler value that sits outside the range, so an
// algorithm that walks past the end shows up as a wrong result rather than as
// silently correct behaviour.
//
// Note that a range covering a whole container can never take the
// single-segment branch at the outermost level: the end iterator lives in the
// sentinel segment.  Reaching that branch requires a proper sub-range, which
// is what make_range() produces.
//
// Empty segments and the sentinel.  The iterators normalise by skipping
// forward (or backward) over however many empty segments they meet, which
// only terminates if the sentinel segment they stop at cannot itself look
// empty.  The trailing sentinel therefore holds one dummy element: end()
// points at it, no range ever includes it, and neither total_size() nor
// flatten_all_ints() can see it.  The one visible consequence is that
// segments().back() has size 1 rather than 0, and that T must be
// constructible from int -- which add_segment_from_ints() already required.
//
//----------------------------------------------------------------------------
// API summary
//----------------------------------------------------------------------------
//
// Building blocks
//   make_range(c, spec, vals, n, filler)
//      Fills an empty seg_vector/seg2_vector with the n values of vals laid
//      out according to spec, plus the trailing guard element.
//   make_dest_range(c, spec, n, fill, filler)
//      Same, for an output range: n copies of fill, plus the guard.
//   iter_at(c, i)          i-th iterator from c.begin(); iter_at(c, n) is the
//                          logical end of the range and points at the guard.
//   flatten_ints(f, l)     values of [f, l) as a vector<int>.
//   flatten_n_ints(c, n)   values of the logical range, guard excluded.
//   flatten_all_ints(c)    values of the whole container, guard included.
//   filler_intact(c, n, filler)
//                          true while the guard still holds filler, i.e. the
//                          algorithm has not written past the end.
//
// Combinators.  Each one builds every range afresh for every combination, so
// a callable is free to mutate the ranges it is handed.  The callable is taken
// by value and copied, so any state it accumulates must live behind a pointer
// or reference.
//   for_each_shape<T>(vals, n, filler, f)
//      f(c, n, spec) once per feasible spec, depth 1 then depth 2,
//      bidirectional iterators.
//   for_each_shape_fwd<T>(vals, n, filler, f)
//      the same with forward iterators, for algorithms that have a distinct
//      forward-iterator implementation.
//   for_each_shape_cat<T, Cat>(vals, n, filler, f)
//      the same for an explicit iterator category.
//   for_each_shape2<T1, T2>(v1, n1, v2, n2, filler, f)
//      f(c1, n1, spec1, c2, n2, spec2) over the cross product of the two
//      ranges' specs.  Range 2 doubles as the output range of a copy-style
//      algorithm; build it with an array of fill values.
//   for_each_shape3<T1, T2, T3>(v1, n1, v2, n2, v3, n3, filler, f)
//      the same for three ranges, e.g. two inputs and one output.
//   for_each_shape_all / _all_cat / _all_fwd / for_each_shape2_all /
//   for_each_shape3_all
//      the same enumerations extended with the 'e' specs.  The core specs
//      still come first, in their original order, so switching a test from
//      for_each_shape to for_each_shape_all only ever adds cases.
//
// A feasible spec needs at least one element per 'm' or 'e' level, so a small
// n yields fewer shapes than a large one; the combinators skip the rest.
// With n large enough each range contributes 6 core shapes and 6 more with
// empty segments:
//
//   n        for_each_shape   for_each_shape_all
//   0        2                2
//   1        5                8
//   >= 2     6                12
//
// so for_each_shape2 runs 36 combinations and for_each_shape2_all 144, while
// for_each_shape3 runs 216 and for_each_shape3_all 1728.
//
// Intended usage for a test:
//
//    struct check
//    {
//       int* bad;
//       template<class Cont>
//       void operator()(Cont& c, std::size_t n, const char* spec) const
//       {
//          typedef typename Cont::iterator iter_t;
//          const iter_t first = c.begin();
//          const iter_t last  = iter_at(c, n);
//          // naive reference answer over a flat copy of the range
//          const boost::container::vector<int> ref = flatten_n_ints(c, n);
//          ...
//          const iter_t r = segmented_xxx(first, last, ...);
//          BOOST_TEST(r == iter_at(c, expected_offset));
//          BOOST_TEST(filler_intact(c, n, -999));
//          BOOST_TEST(spec != 0);
//       }
//    };
//    for_each_shape<int>(vals, n, -999, check(...));
//
//////////////////////////////////////////////////////////////////////////////

//! Number of elements placed before the last segment of a multi-segment level.
inline std::size_t seg_split_point(std::size_t n)
{  return n < 2u ? 1u : n/2u;   }

//! A spec needs at least one element per level asking for a multi-segment
//! branch, because an empty range always lands in a single segment.  'e' is a
//! multi-segment branch too, so it carries the same requirement as 'm'.
inline bool shape_feasible(const char* spec, std::size_t n)
{
   for(; *spec; ++spec) {
      if(*spec == 'm' || *spec == 'e') {
         if(!n)
            return false;
         n -= seg_split_point(n);
      }
   }
   return true;
}

template<class T, class Cat>
void make_range(seg_vector<T, Cat>& c, const char* spec, const int* vals, std::size_t n, int filler)
{
   boost::container::vector<int> tmp;
   tmp.reserve(n + 1u);
   for(std::size_t i = 0; i != n; ++i)
      tmp.push_back(vals[i]);
   tmp.push_back(filler);

   const int* const base = &tmp[0];
   if(*spec == 's') {
      c.add_segment_from_ints(base, base + n + 1u);
   }
   else {
      // 'm' and 'e' both split the data in two; 'e' additionally wraps it in
      // empty segments, at the front, between the halves and at the back.
      const bool empties = (*spec == 'e');
      const std::size_t a = seg_split_point(n);
      if(empties) c.add_segment_from_ints(base, base);
      c.add_segment_from_ints(base, base + a);
      if(empties) c.add_segment_from_ints(base, base);
      c.add_segment_from_ints(base + a, base + n + 1u);
      if(empties) c.add_segment_from_ints(base, base);
   }
}

template<class T, class Cat>
void make_range(seg2_vector<T, Cat>& c, const char* spec, const int* vals, std::size_t n, int filler)
{
   if(*spec == 's') {
      seg_vector<T, Cat> inner;
      make_range(inner, spec + 1, vals, n, filler);
      c.add_segment(boost::move(inner));
   }
   else {
      // 'e' surrounds the two data segments with outer segments whose inner
      // container is logically empty, which is the shape that drives a
      // destination bounded helper into its sfirst == slast branch.
      const bool empties = (*spec == 'e');
      const std::size_t a = seg_split_point(n);
      if(empties) c.add_segment(seg_vector<T, Cat>());
      seg_vector<T, Cat> head;
      head.add_segment_from_ints(vals, vals + a);
      c.add_segment(boost::move(head));
      if(empties) c.add_segment(seg_vector<T, Cat>());
      seg_vector<T, Cat> tail;
      make_range(tail, spec + 1, vals + a, n - a, filler);
      c.add_segment(boost::move(tail));
      if(empties) c.add_segment(seg_vector<T, Cat>());
   }
}

//! Destination range for a copy-style algorithm: n elements all equal to
//! fill, shaped by spec, followed by the usual guard element.  Saves the test
//! from inventing an array of placeholder values.
template<class Cont>
void make_dest_range(Cont& c, const char* spec, std::size_t n, int fill, int filler)
{
   boost::container::vector<int> tmp(n ? n : 1u, fill);
   make_range(c, spec, &tmp[0], n, filler);
}

template<class Cont>
typename Cont::iterator iter_at(Cont& c, std::size_t n)
{
   typename Cont::iterator it = c.begin();
   for(; n; --n)
      ++it;
   return it;
}

inline int seg_value_of(int v)                { return v; }
inline int seg_value_of(const movable_int& m) { return m.value(); }

template<class It>
boost::container::vector<int> flatten_ints(It first, It last)
{
   boost::container::vector<int> r;
   for(; first != last; ++first)
      r.push_back(seg_value_of(*first));
   return r;
}

//! Every element of the container, the trailing guard included.
template<class Cont>
boost::container::vector<int> flatten_all_ints(Cont& c)
{  return flatten_ints(c.begin(), c.end());   }

//! Only the logical range [c.begin(), iter_at(c, n)), i.e. what an algorithm
//! is allowed to touch.  This is the naive reference answer to compare a
//! segmented algorithm's result against.
template<class Cont>
boost::container::vector<int> flatten_n_ints(Cont& c, std::size_t n)
{  return flatten_ints(c.begin(), iter_at(c, n));   }

//! Deepest segmentation depth with a container type to build it from:
//! 1 is seg_vector, 2 is seg2_vector, and there is no deeper container.
inline std::size_t max_shape_depth() { return 2u; }

//! All branch specs for a given segmentation depth, outermost level first.
//! Only depths 1 and 2 exist: seg2_vector is the deepest modelled container.
inline const char* const* shape_specs(std::size_t depth, std::size_t& count)
{
   static const char* const d1[] = { "m", "s" };
   static const char* const d2[] = { "mm", "ms", "sm", "ss" };

   if(depth == 1u) { count = 2u; return d1; }
   count = 4u;
   return d2;
}

//! Branch specs in which at least one level also carries empty segments.
//! Kept in a table of its own so that shape_specs(), and therefore
//! for_each_shape(), enumerate exactly what they enumerated before 'e'
//! existed.  The _all combinators walk both tables.
inline const char* const* shape_specs_empty(std::size_t depth, std::size_t& count)
{
   static const char* const d1[] = { "e" };
   static const char* const d2[] = { "ee", "em", "es", "me", "se" };

   if(depth == 1u) { count = 1u; return d1; }
   count = 5u;
   return d2;
}

//! Spec families: 0 is the core m/s table, 1 the empty-segment table.
inline const char* const* shape_specs_family(std::size_t family, std::size_t depth, std::size_t& count)
{  return family == 0u ? shape_specs(depth, count) : shape_specs_empty(depth, count);   }

//! Number of spec families a plain for_each_shape* walks, and the number the
//! _all variants walk.
inline std::size_t shape_core_families() { return 1u; }
inline std::size_t shape_all_families()  { return 2u; }

//! Builds the container that spec describes at the given depth and hands it to
//! g as g(container, n, spec).  Keeping construction here lets every
//! combinator hand out a freshly built range on every single call.
template<class T, class Cat, class G>
void with_shape(std::size_t depth, const char* spec, const int* vals, std::size_t n, int filler, G g)
{
   if(depth == 1u) {
      seg_vector<T, Cat> c;
      make_range(c, spec, vals, n, filler);
      g(c, n, spec);
   }
   else {
      seg2_vector<T, Cat> c;
      make_range(c, spec, vals, n, filler);
      g(c, n, spec);
   }
}

//! Walks the first nfam spec families over depths 1 and 2, calling
//! f(container, n, spec) once per feasible spec.  With nfam == 1 the
//! enumeration is exactly the core m/s one, in exactly its original order.
template<class T, class Cat, class F>
void for_each_shape_fam_cat(std::size_t nfam, const int* vals, std::size_t n, int filler, F f)
{
   for(std::size_t fam = 0u; fam != nfam; ++fam) {
      for(std::size_t d = 1u; d <= max_shape_depth(); ++d) {
         std::size_t cnt = 0;
         const char* const* s = shape_specs_family(fam, d, cnt);
         for(std::size_t i = 0; i != cnt; ++i) {
            if(!shape_feasible(s[i], n)) continue;
            with_shape<T, Cat>(d, s[i], vals, n, filler, f);
         }
      }
   }
}

//! Calls f(container, n, spec) once per reachable branch spec at depths 1 and 2.
//! f must be callable with both container types.
template<class T, class Cat, class F>
void for_each_shape_cat(const int* vals, std::size_t n, int filler, F f)
{  for_each_shape_fam_cat<T, Cat>(shape_core_families(), vals, n, filler, f);  }

//! Bidirectional-iterator shapes, the default for most algorithms.
template<class T, class F>
void for_each_shape(const int* vals, std::size_t n, int filler, F f)
{  for_each_shape_cat<T, std::bidirectional_iterator_tag>(vals, n, filler, f);  }

//! Forward-iterator shapes.  Algorithms with a separate forward-iterator
//! segmented implementation (segmented_find_last, segmented_find_last_if)
//! only reach it through these.
template<class T, class F>
void for_each_shape_fwd(const int* vals, std::size_t n, int filler, F f)
{  for_each_shape_cat<T, std::forward_iterator_tag>(vals, n, filler, f);  }

//! Core shapes followed by the empty-segment ones.  Use these wherever the
//! algorithm has to cope with a segment that contributes no elements.
template<class T, class Cat, class F>
void for_each_shape_all_cat(const int* vals, std::size_t n, int filler, F f)
{  for_each_shape_fam_cat<T, Cat>(shape_all_families(), vals, n, filler, f);  }

template<class T, class F>
void for_each_shape_all(const int* vals, std::size_t n, int filler, F f)
{  for_each_shape_all_cat<T, std::bidirectional_iterator_tag>(vals, n, filler, f);  }

template<class T, class F>
void for_each_shape_all_fwd(const int* vals, std::size_t n, int filler, F f)
{  for_each_shape_all_cat<T, std::forward_iterator_tag>(vals, n, filler, f);  }

template<class C1, class F>
struct shape2_bind
{
   C1* c1;
   std::size_t n1;
   const char* s1;
   F f;

   shape2_bind(C1& a, std::size_t b, const char* c, F d)
      : c1(&a), n1(b), s1(c), f(d)
   {}

   template<class C2>
   void operator()(C2& c2, std::size_t n2, const char* s2) const
   {  f(*c1, n1, s1, c2, n2, s2);  }
};

//! Builds range 2 for one fixed (depth, spec) pair and pairs it with range 1.
//! Both ranges are rebuilt for every combination, so f may mutate either of
//! them without the next combination inheriting the damage.
template<class T2, class F>
struct shape2_outer
{
   std::size_t d2;
   const char* s2;
   const int* v2;
   std::size_t n2;
   int filler;
   F f;

   shape2_outer(std::size_t d, const char* s, const int* a, std::size_t b, int c, F g)
      : d2(d), s2(s), v2(a), n2(b), filler(c), f(g)
   {}

   template<class C1>
   void operator()(C1& c1, std::size_t n1, const char* s1) const
   {
      with_shape<T2, std::bidirectional_iterator_tag>
         (d2, s2, v2, n2, filler, shape2_bind<C1, F>(c1, n1, s1, f));
   }
};

//! Cross product over the first nfam spec families of two ranges.
template<class T1, class T2, class F>
void for_each_shape2_fam(std::size_t nfam, const int* v1, std::size_t n1,
                         const int* v2, std::size_t n2, int filler, F f)
{
   for(std::size_t f1 = 0u; f1 != nfam; ++f1) {
      for(std::size_t d1 = 1u; d1 <= max_shape_depth(); ++d1) {
         std::size_t cnt1 = 0;
         const char* const* sp1 = shape_specs_family(f1, d1, cnt1);
         for(std::size_t i = 0; i != cnt1; ++i) {
            if(!shape_feasible(sp1[i], n1)) continue;
            for(std::size_t f2 = 0u; f2 != nfam; ++f2) {
               for(std::size_t d2 = 1u; d2 <= max_shape_depth(); ++d2) {
                  std::size_t cnt2 = 0;
                  const char* const* sp2 = shape_specs_family(f2, d2, cnt2);
                  for(std::size_t j = 0; j != cnt2; ++j) {
                     if(!shape_feasible(sp2[j], n2)) continue;
                     with_shape<T1, std::bidirectional_iterator_tag>
                        (d1, sp1[i], v1, n1, filler,
                         shape2_outer<T2, F>(d2, sp2[j], v2, n2, filler, f));
                  }
               }
            }
         }
      }
   }
}

//! Cross product of the branch specs of two independently segmented ranges.
//! Calls f(c1, n1, spec1, c2, n2, spec2).
template<class T1, class T2, class F>
void for_each_shape2(const int* v1, std::size_t n1, const int* v2, std::size_t n2, int filler, F f)
{  for_each_shape2_fam<T1, T2>(shape_core_families(), v1, n1, v2, n2, filler, f);  }

//! The same, including the empty-segment shapes for both ranges.
template<class T1, class T2, class F>
void for_each_shape2_all(const int* v1, std::size_t n1, const int* v2, std::size_t n2, int filler, F f)
{  for_each_shape2_fam<T1, T2>(shape_all_families(), v1, n1, v2, n2, filler, f);  }

template<class C1, class C2, class F>
struct shape3_bind
{
   C1* c1; std::size_t n1; const char* s1;
   C2* c2; std::size_t n2; const char* s2;
   F f;

   shape3_bind(C1& a1, std::size_t b1, const char* d1,
               C2& a2, std::size_t b2, const char* d2, F g)
      : c1(&a1), n1(b1), s1(d1), c2(&a2), n2(b2), s2(d2), f(g)
   {}

   template<class C3>
   void operator()(C3& c3, std::size_t n3, const char* s3) const
   {  f(*c1, n1, s1, *c2, n2, s2, c3, n3, s3);  }
};

//! Builds range 3 for one fixed (depth, spec) pair; the first two ranges are
//! supplied by for_each_shape2, which already rebuilds them per combination.
template<class T3, class F>
struct shape3_outer
{
   std::size_t d3;
   const char* s3;
   const int* v3;
   std::size_t n3;
   int filler;
   F f;

   shape3_outer(std::size_t d, const char* s, const int* a, std::size_t b, int c, F g)
      : d3(d), s3(s), v3(a), n3(b), filler(c), f(g)
   {}

   template<class C1, class C2>
   void operator()(C1& c1, std::size_t n1, const char* s1,
                   C2& c2, std::size_t n2, const char* s2) const
   {
      with_shape<T3, std::bidirectional_iterator_tag>
         (d3, s3, v3, n3, filler, shape3_bind<C1, C2, F>(c1, n1, s1, c2, n2, s2, f));
   }
};

//! Cross product over the first nfam spec families of three ranges.
template<class T1, class T2, class T3, class F>
void for_each_shape3_fam(std::size_t nfam, const int* v1, std::size_t n1,
                         const int* v2, std::size_t n2,
                         const int* v3, std::size_t n3, int filler, F f)
{
   for(std::size_t f3 = 0u; f3 != nfam; ++f3) {
      for(std::size_t d3 = 1u; d3 <= max_shape_depth(); ++d3) {
         std::size_t cnt3 = 0;
         const char* const* sp3 = shape_specs_family(f3, d3, cnt3);
         for(std::size_t k = 0; k != cnt3; ++k) {
            if(!shape_feasible(sp3[k], n3)) continue;
            for_each_shape2_fam<T1, T2>(nfam, v1, n1, v2, n2, filler,
                                        shape3_outer<T3, F>(d3, sp3[k], v3, n3, filler, f));
         }
      }
   }
}

//! Cross product of the branch specs of three independently segmented ranges.
//! Calls f(c1, n1, spec1, c2, n2, spec2, c3, n3, spec3).  Typically two input
//! ranges and one output range.
template<class T1, class T2, class T3, class F>
void for_each_shape3(const int* v1, std::size_t n1, const int* v2, std::size_t n2,
                     const int* v3, std::size_t n3, int filler, F f)
{  for_each_shape3_fam<T1, T2, T3>(shape_core_families(), v1, n1, v2, n2, v3, n3, filler, f);  }

//! The same, including the empty-segment shapes for all three ranges.
template<class T1, class T2, class T3, class F>
void for_each_shape3_all(const int* v1, std::size_t n1, const int* v2, std::size_t n2,
                         const int* v3, std::size_t n3, int filler, F f)
{  for_each_shape3_fam<T1, T2, T3>(shape_all_families(), v1, n1, v2, n2, v3, n3, filler, f);  }

//! True if the guard element that make_range() placed just past the range end
//! still holds the filler value, i.e. the algorithm did not write past the end.
template<class Cont>
bool filler_intact(Cont& c, std::size_t n, int filler)
{  return seg_value_of(*iter_at(c, n)) == filler;   }

} // namespace test_detail

namespace boost {
namespace container {

template<class T, class Cat>
struct segmented_iterator_traits<test_detail::seg_vector_iterator<T, Cat> >
{
   typedef segmented_iterator_tag is_segmented_iterator;

   typedef typename boost::container::vector<boost::container::vector<T> >::iterator segment_iterator;
   typedef typename boost::container::vector<T>::iterator               local_iterator;
   typedef test_detail::seg_vector_iterator<T, Cat>        iterator;

   static segment_iterator segment(iterator it) { return it.seg_; }
   static local_iterator   local(iterator it)   { return it.local_; }

   //! Normalises a (segment, local) pair onto the first position at or after
   //! it that actually holds an element, skipping any empty segments in
   //! between.  Stops at the sentinel segment, which is never empty.
   static iterator compose(segment_iterator s, local_iterator l)
   {
      while(l == s->end()) {
         ++s;
         l = s->begin();
      }
      return iterator(s, l);
   }

   static local_iterator begin(segment_iterator s) { return s->begin(); }
   static local_iterator end(segment_iterator s)   { return s->end(); }
};

template<class T, class Cat>
struct segmented_iterator_traits<test_detail::seg2_vector_iterator<T, Cat> >
{
   typedef segmented_iterator_tag is_segmented_iterator;

   typedef typename boost::container::vector<test_detail::seg_vector<T, Cat> >::iterator segment_iterator;
   typedef test_detail::seg_vector_iterator<T, Cat>                    local_iterator;
   typedef test_detail::seg2_vector_iterator<T, Cat>                   iterator;

   static segment_iterator segment(iterator it) { return it.seg_; }
   static local_iterator   local(iterator it)   { return it.local_; }

   //! See the seg_vector_iterator specialisation: skips outer segments whose
   //! inner container is logically empty, stopping at the sentinel.
   static iterator compose(segment_iterator s, local_iterator l)
   {
      while(l == s->end()) {
         ++s;
         l = s->begin();
      }
      return iterator(s, l);
   }

   static local_iterator begin(segment_iterator s) { return s->begin(); }
   static local_iterator end(segment_iterator s)   { return s->end(); }
};

} // namespace container
} // namespace boost

#endif // BOOST_CONTAINER_TEST_SEGMENTED_TEST_HELPER_HPP
