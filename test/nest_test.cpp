//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/experimental/nest.hpp>
#include <boost/core/lightweight_test.hpp>
#include <algorithm>
#include <functional>

using namespace boost::container;

void test_default_construction()
{
   nest<int> h;
   BOOST_TEST(h.empty());
   BOOST_TEST_EQ(h.size(), 0u);
   BOOST_TEST(h.begin() == h.end());
}

void test_count_construction()
{
   nest<int> h(5);
   BOOST_TEST_EQ(h.size(), 5u);
   // value-initialized ints should be 0
   for(nest<int>::const_iterator it = h.begin(); it != h.end(); ++it) {
      BOOST_TEST_EQ(*it, 0);
   }
}

void test_count_value_construction()
{
   nest<int> h(3, 42);
   BOOST_TEST_EQ(h.size(), 3u);
   for(nest<int>::const_iterator it = h.begin(); it != h.end(); ++it) {
      BOOST_TEST_EQ(*it, 42);
   }
}

void test_range_construction()
{
   int arr[] = {1, 2, 3, 4, 5};
   nest<int> h(arr, arr + 5);
   BOOST_TEST_EQ(h.size(), 5u);
}

void test_copy_construction()
{
   nest<int> h1(3, 7);
   nest<int> h2(h1);
   BOOST_TEST_EQ(h2.size(), 3u);
   for(nest<int>::const_iterator it = h2.begin(); it != h2.end(); ++it) {
      BOOST_TEST_EQ(*it, 7);
   }
}

void test_move_construction()
{
   nest<int> h1(3, 7);
   nest<int> h2(boost::move(h1));
   BOOST_TEST_EQ(h2.size(), 3u);
   BOOST_TEST(h1.empty());
}

void test_insert_erase()
{
   nest<int> h;
   nest<int>::iterator it1 = h.insert(10);
   nest<int>::iterator it2 = h.insert(20);
   nest<int>::iterator it3 = h.insert(30);
   BOOST_TEST_EQ(h.size(), 3u);

   h.erase(it2);
   BOOST_TEST_EQ(h.size(), 2u);

   h.erase(it1);
   BOOST_TEST_EQ(h.size(), 1u);

   h.erase(it3);
   BOOST_TEST(h.empty());
}

void test_emplace()
{
   nest<int> h;
   nest<int>::iterator it = h.emplace(42);
   BOOST_TEST_EQ(*it, 42);
   BOOST_TEST_EQ(h.size(), 1u);
}

void test_assign()
{
   nest<int> h(5, 1);
   h.assign(3u, 42);
   BOOST_TEST_EQ(h.size(), 3u);
   for(nest<int>::const_iterator it = h.begin(); it != h.end(); ++it) {
      BOOST_TEST_EQ(*it, 42);
   }
}

void test_copy_assignment()
{
   nest<int> h1(3, 7);
   nest<int> h2;
   h2 = h1;
   BOOST_TEST_EQ(h2.size(), 3u);
}

void test_move_assignment()
{
   nest<int> h1(3, 7);
   nest<int> h2;
   h2 = boost::move(h1);
   BOOST_TEST_EQ(h2.size(), 3u);
   BOOST_TEST(h1.empty());
}

void test_swap()
{
   nest<int> h1(3, 1);
   nest<int> h2(5, 2);
   h1.swap(h2);
   BOOST_TEST_EQ(h1.size(), 5u);
   BOOST_TEST_EQ(h2.size(), 3u);
}

void test_clear()
{
   nest<int> h(10, 5);
   BOOST_TEST_EQ(h.size(), 10u);
   h.clear();
   BOOST_TEST(h.empty());
}

void test_iterators()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(3);

   int count = 0;
   for(nest<int>::iterator it = h.begin(); it != h.end(); ++it) {
      ++count;
   }
   BOOST_TEST_EQ(count, 3);

   count = 0;
   for(nest<int>::const_iterator it = h.cbegin(); it != h.cend(); ++it) {
      ++count;
   }
   BOOST_TEST_EQ(count, 3);
}

void test_reverse_iterators()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(3);

   int count = 0;
   for(nest<int>::reverse_iterator it = h.rbegin(); it != h.rend(); ++it) {
      ++count;
   }
   BOOST_TEST_EQ(count, 3);
}

void test_capacity()
{
   nest<int> h;
   BOOST_TEST_EQ(h.capacity(), 0u);
   h.reserve(100);
   BOOST_TEST(h.capacity() >= 100u);
   h.trim_capacity();
   BOOST_TEST_EQ(h.capacity(), 0u);
}

void test_sort()
{
   nest<int> h;
   h.insert(30);
   h.insert(10);
   h.insert(20);
   h.sort();
   nest<int>::const_iterator it = h.begin();
   BOOST_TEST_EQ(*it, 10); ++it;
   BOOST_TEST_EQ(*it, 20); ++it;
   BOOST_TEST_EQ(*it, 30);
}

void test_unique()
{
   nest<int> h;
   h.insert(1);
   h.insert(1);
   h.insert(2);
   h.insert(2);
   h.insert(3);
   h.sort();
   nest<int>::size_type removed = h.unique();
   BOOST_TEST_EQ(removed, 2u);
   BOOST_TEST_EQ(h.size(), 3u);
}

struct less_3
   : public std::less<int>
{
   less_3(int val)
      : std::less<int>()
      , val_(val)
   {}

   template<class T>
   bool operator() (const T & val) const
   {
      return std::less<int>::operator()(val, val_);
   }

   int val_;
};

void test_erase_if()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(3);
   h.insert(4);
   h.insert(5);
   nest<int>::size_type removed = erase_if(h,less_3(3));
   BOOST_TEST_EQ(removed, 2u);
   BOOST_TEST_EQ(h.size(), 3u);
}

void test_erase_value()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(1);
   h.insert(3);
   nest<int>::size_type removed = erase(h, 1);
   BOOST_TEST_EQ(removed, 2u);
   BOOST_TEST_EQ(h.size(), 2u);
}

void test_splice()
{
   nest<int> h1(3, 1);
   nest<int> h2(2, 2);
   h1.splice(h2);
   BOOST_TEST_EQ(h1.size(), 5u);
   BOOST_TEST(h2.empty());
}

void test_large_insert_erase()
{
   nest<int> h;
   // Insert many elements
   for(int i = 0; i < 1000; ++i) {
      h.insert(i);
   }
   BOOST_TEST_EQ(h.size(), 1000u);

   // Erase all elements one by one
   while(!h.empty()) {
      h.erase(h.begin());
   }
   BOOST_TEST(h.empty());
}

void test_shrink_to_fit()
{
   nest<int> h;
   for(int i = 0; i < 100; ++i) {
      h.insert(i);
   }
   // Erase half
   int count = 0;
   nest<int>::iterator it = h.begin();
   while(it != h.end()) {
      if(count % 2 == 0) {
         it = h.erase(it);
      } else {
         ++it;
      }
      ++count;
   }
   nest<int>::size_type cap_before = h.capacity();
   h.shrink_to_fit();
   BOOST_TEST(h.capacity() <= cap_before);
}

#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
void test_initializer_list()
{
   nest<int> h = {1, 2, 3, 4, 5};
   BOOST_TEST_EQ(h.size(), 5u);
}
#endif

template<class T>
class unequal_allocator
{
public:
   typedef T value_type;

   template<class U>
   struct rebind { typedef unequal_allocator<U> other; };

   int id_;

   unequal_allocator() : id_(0) {}
   explicit unequal_allocator(int id) : id_(id) {}

   template<class U>
   unequal_allocator(const unequal_allocator<U>& other) : id_(other.id_) {}

   T* allocate(std::size_t n) { return static_cast<T*>(::operator new(n * sizeof(T))); }
   void deallocate(T* p, std::size_t) { ::operator delete(p); }

   friend bool operator==(const unequal_allocator& a, const unequal_allocator& b)
   { return a.id_ == b.id_; }
   friend bool operator!=(const unequal_allocator& a, const unequal_allocator& b)
   { return a.id_ != b.id_; }
};

void test_move_construction_unequal_allocator()
{
   typedef unequal_allocator<int> alloc_t;
   typedef nest<int, alloc_t> nest_t;

   alloc_t a1(1);
   alloc_t a2(2);

   nest_t h1(a1);
   h1.insert(10);
   h1.insert(20);
   h1.insert(30);

   nest_t h2(boost::move(h1), a2);

   BOOST_TEST_EQ(h2.size(), 3u);
   BOOST_TEST_EQ(h2.get_allocator().id_, 2);
   h2.sort();
   nest_t::const_iterator it = h2.begin();
   BOOST_TEST_EQ(*it, 10); ++it;
   BOOST_TEST_EQ(*it, 20); ++it;
   BOOST_TEST_EQ(*it, 30);
   BOOST_TEST_EQ(h1.size(), 0u);
}

void test_move_assignment_unequal_allocator()
{
   typedef unequal_allocator<int> alloc_t;
   typedef nest<int, alloc_t> nest_t;

   alloc_t a1(1);
   alloc_t a2(2);

   nest_t h1(a1);
   h1.insert(10);
   h1.insert(20);
   h1.insert(30);

   nest_t h2(a2);
   h2.insert(100);

   h2 = boost::move(h1);

   BOOST_TEST_EQ(h2.size(), 3u);
   BOOST_TEST_EQ(h2.get_allocator().id_, 2);
   h2.sort();
   nest_t::const_iterator it = h2.begin();
   BOOST_TEST_EQ(*it, 10); ++it;
   BOOST_TEST_EQ(*it, 20); ++it;
   BOOST_TEST_EQ(*it, 30);
   BOOST_TEST(h1.empty());
}

void test_copy_construction_with_allocator()
{
   typedef unequal_allocator<int> alloc_t;
   typedef nest<int, alloc_t> nest_t;

   alloc_t a1(1);
   alloc_t a2(2);

   nest_t h1(a1);
   h1.insert(10);
   h1.insert(20);
   h1.insert(30);

   nest_t h2(h1, a2);
   BOOST_TEST_EQ(h2.size(), 3u);
   BOOST_TEST_EQ(h2.get_allocator().id_, 2);
   BOOST_TEST_EQ(h1.size(), 3u);
}

void test_max_size()
{
   nest<int> h;
   BOOST_TEST(h.max_size() > 0u);
}

void test_trim_capacity_with_bound()
{
   nest<int> h;
   h.reserve(200);
   nest<int>::size_type cap_before = h.capacity();
   BOOST_TEST(cap_before >= 200u);
   h.trim_capacity(cap_before / 2);
   BOOST_TEST(h.capacity() <= cap_before / 2);
   BOOST_TEST(h.capacity() < cap_before);
}

void test_emplace_hint()
{
   nest<int> h;
   h.insert(1);
   nest<int>::iterator it = h.emplace_hint(h.begin(), 42);
   BOOST_TEST_EQ(*it, 42);
   BOOST_TEST_EQ(h.size(), 2u);
}

void test_insert_move()
{
   nest<int> h;
   int val = 42;
   nest<int>::iterator it = h.insert(boost::move(val));
   BOOST_TEST_EQ(*it, 42);
   BOOST_TEST_EQ(h.size(), 1u);
}

void test_insert_with_hint()
{
   nest<int> h;
   h.insert(1);
   nest<int>::iterator it = h.insert(h.begin(), 42);
   BOOST_TEST_EQ(*it, 42);
   int val = 99;
   nest<int>::iterator it2 = h.insert(h.begin(), boost::move(val));
   BOOST_TEST_EQ(*it2, 99);
   BOOST_TEST_EQ(h.size(), 3u);
}

void test_erase_void()
{
   nest<int> h;
   nest<int>::iterator it1 = h.insert(10);
   h.insert(20);
   h.insert(30);
   BOOST_TEST_EQ(h.size(), 3u);
   h.erase_void(it1);
   BOOST_TEST_EQ(h.size(), 2u);
}

void test_erase_range()
{
   nest<int> h;
   for(int i = 0; i < 10; ++i) h.insert(i);
   BOOST_TEST_EQ(h.size(), 10u);
   nest<int>::iterator first = h.begin();
   ++first; ++first;
   nest<int>::iterator last = first;
   ++last; ++last; ++last;
   nest<int>::iterator ret = h.erase(first, last);
   BOOST_TEST_EQ(h.size(), 7u);
   (void)ret;
}

void test_range_assign()
{
   int arr[] = {10, 20, 30};
   nest<int> h(5, 1);
   h.assign(arr, arr + 3);
   BOOST_TEST_EQ(h.size(), 3u);
   h.sort();
   nest<int>::const_iterator it = h.begin();
   BOOST_TEST_EQ(*it, 10); ++it;
   BOOST_TEST_EQ(*it, 20); ++it;
   BOOST_TEST_EQ(*it, 30);
}

void test_const_reverse_iterators()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(3);

   int count = 0;
   for(nest<int>::const_reverse_iterator it = h.crbegin(); it != h.crend(); ++it) {
      ++count;
   }
   BOOST_TEST_EQ(count, 3);

   const nest<int>& ch = h;
   count = 0;
   for(nest<int>::const_reverse_iterator it = ch.rbegin(); it != ch.rend(); ++it) {
      ++count;
   }
   BOOST_TEST_EQ(count, 3);
}

void test_sort_with_comparator()
{
   nest<int> h;
   h.insert(10);
   h.insert(30);
   h.insert(20);
   h.sort(std::greater<int>());
   nest<int>::const_iterator it = h.begin();
   BOOST_TEST_EQ(*it, 30); ++it;
   BOOST_TEST_EQ(*it, 20); ++it;
   BOOST_TEST_EQ(*it, 10);
}

void test_unique_with_predicate()
{
   nest<int> h;
   h.insert(1);
   h.insert(1);
   h.insert(2);
   h.insert(2);
   h.insert(3);
   h.sort();
   nest<int>::size_type removed = h.unique(std::equal_to<int>());
   BOOST_TEST_EQ(removed, 2u);
   BOOST_TEST_EQ(h.size(), 3u);
}

void test_get_iterator()
{
   nest<int> h;
   nest<int>::iterator it = h.insert(42);
   nest<int>::const_pointer p = &(*it);

   nest<int>::iterator found = h.get_iterator(p);
   BOOST_TEST(found != h.end());
   BOOST_TEST_EQ(*found, 42);

   const nest<int>& ch = h;
   nest<int>::const_iterator cfound = ch.get_iterator(p);
   BOOST_TEST(cfound != ch.end());
   BOOST_TEST_EQ(*cfound, 42);

   nest<int>::const_pointer bad = 0;
   nest<int>::iterator not_found = h.get_iterator(bad);
   (void)not_found;
}

struct sum_functor
{
   int* psum;
   explicit sum_functor(int* p) : psum(p) {}
   void operator()(int x) { *psum += x; }
};

struct doubler_functor
{
   void operator()(int& x) const { x *= 2; }
};

struct conditional_sum_functor
{
   int* psum;
   int limit;
   conditional_sum_functor(int* p, int lim) : psum(p), limit(lim) {}
   bool operator()(int x) {
      if(*psum + x > limit) return false;
      *psum += x;
      return true;
   }
};

void test_visit()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(3);

   h.visit(h.begin(), h.end(), doubler_functor());
   h.sort();
   nest<int>::const_iterator it = h.begin();
   BOOST_TEST_EQ(*it, 2); ++it;
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 6);

   int sum = 0;
   const nest<int>& ch = h;
   ch.visit(ch.begin(), ch.end(), sum_functor(&sum));
   BOOST_TEST_EQ(sum, 12);

   h.visit_all(doubler_functor());
   h.sort();
   it = h.begin();
   BOOST_TEST_EQ(*it, 4); ++it;
   BOOST_TEST_EQ(*it, 8); ++it;
   BOOST_TEST_EQ(*it, 12);

   sum = 0;
   ch.visit_all(sum_functor(&sum));
   BOOST_TEST_EQ(sum, 24);
}

void test_visit_while()
{
   nest<int> h;
   h.insert(1);
   h.insert(2);
   h.insert(3);
   h.insert(4);
   h.insert(5);
   h.sort();

   int sum = 0;
   nest<int>::iterator stop_it = h.visit_while(
      h.begin(), h.end(), conditional_sum_functor(&sum, 6));
   BOOST_TEST(sum <= 6);
   BOOST_TEST(stop_it != h.end());

   sum = 0;
   stop_it = h.visit_all_while(conditional_sum_functor(&sum, 3));
   BOOST_TEST(sum <= 3);

   const nest<int>& ch = h;
   sum = 0;
   nest<int>::const_iterator cstop_it = ch.visit_while(
      ch.begin(), ch.end(), conditional_sum_functor(&sum, 6));
   BOOST_TEST(sum <= 6);
   BOOST_TEST(cstop_it != ch.end());

   sum = 0;
   cstop_it = ch.visit_all_while(conditional_sum_functor(&sum, 3));
   BOOST_TEST(sum <= 3);
}

void test_free_swap()
{
   nest<int> h1(3, 1);
   nest<int> h2(5, 2);
   swap(h1, h2);
   BOOST_TEST_EQ(h1.size(), 5u);
   BOOST_TEST_EQ(h2.size(), 3u);
}

void test_splice_rvalue()
{
   nest<int> h1(3, 1);
   nest<int> h2(2, 2);
   h1.splice(boost::move(h2));
   BOOST_TEST_EQ(h1.size(), 5u);
   BOOST_TEST(h2.empty());
}

void test_options()
{
   {
      typedef nest_options<store_data_in_block<true> >::type sdb_opts;
      typedef nest<int, void, sdb_opts> nest_sdb_t;
      nest_sdb_t h;
      h.insert(10);
      h.insert(20);
      h.insert(30);
      BOOST_TEST_EQ(h.size(), 3u);
      h.sort();
      nest_sdb_t::const_iterator it = h.begin();
      BOOST_TEST_EQ(*it, 10); ++it;
      BOOST_TEST_EQ(*it, 20); ++it;
      BOOST_TEST_EQ(*it, 30);
      nest_sdb_t h2(h);
      BOOST_TEST_EQ(h2.size(), 3u);
      nest_sdb_t h3(boost::move(h));
      BOOST_TEST_EQ(h3.size(), 3u);
      BOOST_TEST(h.empty());
      h3.clear();
      BOOST_TEST(h3.empty());
   }
   {
      typedef nest_options<prefetch<false> >::type np_opts;
      typedef nest<int, void, np_opts> nest_np_t;
      nest_np_t h;
      h.insert(1);
      h.insert(2);
      h.insert(3);
      BOOST_TEST_EQ(h.size(), 3u);
      h.sort();
      nest_np_t::const_iterator it = h.begin();
      BOOST_TEST_EQ(*it, 1); ++it;
      BOOST_TEST_EQ(*it, 2); ++it;
      BOOST_TEST_EQ(*it, 3);
   }
}

#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
void test_initializer_list_operations()
{
   {
      nest<int> h(3, 99);
      h = {10, 20, 30};
      BOOST_TEST_EQ(h.size(), 3u);
   }
   {
      nest<int> h(5, 1);
      h.assign({10, 20, 30});
      BOOST_TEST_EQ(h.size(), 3u);
   }
   {
      nest<int> h;
      h.insert({1, 2, 3, 4, 5});
      BOOST_TEST_EQ(h.size(), 5u);
   }
}
#endif

void test_segment_iterator_operations()
{
   typedef nest<int> nest_t;
   typedef nest_t::iterator iterator;
   typedef segmented_iterator_traits<iterator> traits;
   typedef traits::segment_iterator segment_iterator;
   typedef traits::local_iterator local_iterator;

   nest_t h;
   for (int i = 0; i < 200; ++i)
      h.insert(i);

   segment_iterator seg_begin = traits::segment(h.begin());
   segment_iterator seg_end   = traits::segment(h.end());

   // Default construction compiles
   {
      segment_iterator s;
      (void)s;
   }

   // segment() + equality / inequality
   {
      segment_iterator s1 = traits::segment(h.begin());
      segment_iterator s2 = traits::segment(h.begin());
      BOOST_TEST(s1 == s2);
      BOOST_TEST(!(s1 != s2));
      BOOST_TEST(s1 != seg_end);
      BOOST_TEST(!(s1 == seg_end));
   }

   // Pre-increment returns reference to self and advances
   {
      segment_iterator s = seg_begin;
      segment_iterator& ref = ++s;
      BOOST_TEST(&ref == &s);
      BOOST_TEST(s != seg_begin);
      BOOST_TEST(s != seg_end);
   }

   // Post-increment returns old value and advances
   {
      segment_iterator s = seg_begin;
      segment_iterator old = s++;
      BOOST_TEST(old == seg_begin);
      segment_iterator expected = seg_begin;
      ++expected;
      BOOST_TEST(s == expected);
   }

   // Pre-decrement returns reference to self and retreats
   {
      segment_iterator s = seg_begin;
      ++s;
      segment_iterator& ref = --s;
      BOOST_TEST(&ref == &s);
      BOOST_TEST(s == seg_begin);
   }

   // Post-decrement returns old value and retreats
   {
      segment_iterator second = seg_begin;
      ++second;
      segment_iterator old = second--;
      BOOST_TEST(second == seg_begin);
      segment_iterator expected = seg_begin;
      ++expected;
      BOOST_TEST(old == expected);
   }

   // Increment then decrement is identity
   {
      segment_iterator s = seg_begin;
      ++s; ++s;
      --s; --s;
      BOOST_TEST(s == seg_begin);
   }

   // Count segments (200 elements, 64 per block => at least 4 segments)
   {
      int num_segments = 0;
      for (segment_iterator s = seg_begin; s != seg_end; ++s)
         ++num_segments;
      BOOST_TEST(num_segments >= 4);
   }

   // Full segmented traversal yields all elements
   {
      int count = 0;
      segment_iterator s = seg_begin;
      local_iterator loc = traits::local(h.begin());
      while (s != seg_end) {
         local_iterator loc_end = traits::end(s);
         for (; loc != loc_end; ++loc)
            ++count;
         ++s;
         if (s != seg_end)
            loc = traits::begin(s);
      }
      BOOST_TEST_EQ(count, 200);
   }

   // Reverse traversal over segments
   {
      int fwd_count = 0;
      for (segment_iterator s = seg_begin; s != seg_end; ++s)
         ++fwd_count;

      segment_iterator s = seg_end;
      int rev_count = 0;
      while (s != seg_begin) {
         --s;
         ++rev_count;
      }
      BOOST_TEST_EQ(fwd_count, rev_count);
   }

}

void test_local_iterator_operations()
{
   typedef nest<int> nest_t;
   typedef nest_t::iterator iterator;
   typedef segmented_iterator_traits<iterator> traits;
   typedef traits::segment_iterator segment_iterator;
   typedef traits::local_iterator local_iterator;

   // ---- Dense case (no gaps in bitmask) ----
   nest_t h;
   for (int i = 0; i < 200; ++i)
      h.insert(i);

   segment_iterator seg = traits::segment(h.begin());

   // Default construction compiles
   {
      local_iterator l;
      (void)l;
   }

   // begin() / end() of a segment
   {
      local_iterator b = traits::begin(seg);
      local_iterator e = traits::end(seg);
      BOOST_TEST(b != e);
   }

   // operator* dereference
   {
      local_iterator b = traits::begin(seg);
      int val = *b;
      (void)val;
   }

   // operator-> returns valid pointer
   {
      local_iterator b = traits::begin(seg);
      BOOST_TEST(b.operator->() != 0);
      BOOST_TEST_EQ(*b.operator->(), *b);
   }

   // Equality / inequality
   {
      local_iterator b1 = traits::begin(seg);
      local_iterator b2 = traits::begin(seg);
      local_iterator e  = traits::end(seg);
      BOOST_TEST(b1 == b2);
      BOOST_TEST(!(b1 != b2));
      BOOST_TEST(b1 != e);
      BOOST_TEST(!(b1 == e));
   }

   // Pre-increment returns reference to self and advances
   {
      local_iterator b = traits::begin(seg);
      local_iterator orig = b;
      local_iterator& ref = ++b;
      BOOST_TEST(&ref == &b);
      BOOST_TEST(b != orig);
   }

   // Post-increment returns old value
   {
      local_iterator b = traits::begin(seg);
      local_iterator old = b++;
      BOOST_TEST(old == traits::begin(seg));
      local_iterator expected = traits::begin(seg);
      ++expected;
      BOOST_TEST(b == expected);
   }

   // Pre-decrement returns reference to self and retreats
   {
      local_iterator b = traits::begin(seg);
      local_iterator second = b;
      ++second;
      local_iterator& ref = --second;
      BOOST_TEST(&ref == &second);
      BOOST_TEST(second == b);
   }

   // Post-decrement returns old value
   {
      local_iterator b = traits::begin(seg);
      local_iterator second = b;
      ++second;
      local_iterator old = second--;
      BOOST_TEST(second == b);
      local_iterator expected = b;
      ++expected;
      BOOST_TEST(old == expected);
   }

   // Increment then decrement is identity
   {
      local_iterator b = traits::begin(seg);
      ++b; ++b; ++b;
      --b; --b; --b;
      BOOST_TEST(b == traits::begin(seg));
   }

   // operator- (distance) on a dense (fully packed) block
   {
      local_iterator b = traits::begin(seg);
      local_iterator e = traits::end(seg);

      int count = 0;
      for (local_iterator tmp = b; tmp != e; ++tmp)
         ++count;

      BOOST_TEST_EQ(e - b, count);
      BOOST_TEST_EQ(b - b, 0);
      BOOST_TEST_EQ(e - e, 0);

      local_iterator second = b;
      ++second;
      BOOST_TEST_EQ(second - b, 1);
   }

   // operator- at intermediate positions
   {
      local_iterator b = traits::begin(seg);
      local_iterator it = b;
      for (int i = 0; i < 5; ++i) ++it;
      BOOST_TEST_EQ(it - b, 5);
   }

   // compose() round-trip: compose(segment(it), local(it)) == it
   {
      iterator it = h.begin();
      segment_iterator s = traits::segment(it);
      local_iterator   l = traits::local(it);
      iterator composed  = traits::compose(s, l);
      BOOST_TEST(composed == it);
      BOOST_TEST_EQ(*composed, *it);
   }

   // compose() round-trip after advancing the nest iterator
   {
      iterator it = h.begin();
      ++it; ++it; ++it;
      segment_iterator s = traits::segment(it);
      local_iterator   l = traits::local(it);
      iterator composed  = traits::compose(s, l);
      BOOST_TEST(composed == it);
      BOOST_TEST_EQ(*composed, *it);
   }

   // local() gives an iterator that dereferences to the same value
   {
      iterator it = h.begin();
      local_iterator l = traits::local(it);
      BOOST_TEST_EQ(*l, *it);
   }

   // begin(seg) matches local(compose(seg, begin(seg)))
   {
      local_iterator b = traits::begin(seg);
      iterator it = traits::compose(seg, b);
      local_iterator l = traits::local(it);
      BOOST_TEST(l == b);
   }

   // ---- Sparse case (gaps in bitmask after erasures) ----
   {
      nest_t h2;
      for (int i = 0; i < 128; ++i)
         h2.insert(i);

      int remaining = 128;
      int erase_idx = 0;
      for (iterator it = h2.begin(); it != h2.end(); ) {
         if (erase_idx % 3 == 0) {
            it = h2.erase(it);
            --remaining;
         } else {
            ++it;
         }
         ++erase_idx;
      }
      BOOST_TEST_EQ(h2.size(), (nest_t::size_type)remaining);

      segment_iterator s     = traits::segment(h2.begin());
      segment_iterator s_end = traits::segment(h2.end());

      int total_count = 0;
      local_iterator loc = traits::local(h2.begin());
      while (s != s_end) {
         local_iterator lb = traits::begin(s);
         local_iterator le = traits::end(s);

         int seg_count = 0;
         for (local_iterator tmp = lb; tmp != le; ++tmp)
            ++seg_count;

         // operator- must match the manual count
         BOOST_TEST_EQ(le - lb, seg_count);
         BOOST_TEST_EQ(lb - lb, 0);

         // Intermediate distances
         if (seg_count >= 4) {
            local_iterator mid = lb;
            ++mid; ++mid; ++mid;
            BOOST_TEST_EQ(mid - lb, 3);
            BOOST_TEST_EQ(le - mid, seg_count - 3);
         }

         for (; loc != le; ++loc)
            ++total_count;

         ++s;
         if (s != s_end)
            loc = traits::begin(s);
      }
      BOOST_TEST_EQ(total_count, remaining);
   }
}

int main()
{
   test_default_construction();
   test_count_construction();
   test_count_value_construction();
   test_range_construction();
   test_copy_construction();
   test_move_construction();
   test_insert_erase();
   test_emplace();
   test_assign();
   test_copy_assignment();
   test_move_assignment();
   test_swap();
   test_clear();
   test_iterators();
   test_reverse_iterators();
   test_capacity();
   test_sort();
   test_unique();
   test_erase_if();
   test_erase_value();
   test_splice();
   test_large_insert_erase();
   test_shrink_to_fit();
   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   test_initializer_list();
   #endif
   test_move_construction_unequal_allocator();
   test_move_assignment_unequal_allocator();
   test_copy_construction_with_allocator();
   test_max_size();
   test_trim_capacity_with_bound();
   test_emplace_hint();
   test_insert_move();
   test_insert_with_hint();
   test_erase_void();
   test_erase_range();
   test_range_assign();
   test_const_reverse_iterators();
   test_sort_with_comparator();
   test_unique_with_predicate();
   test_get_iterator();
   test_visit();
   test_visit_while();
   test_free_swap();
   test_splice_rvalue();
   test_options();
   #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   test_initializer_list_operations();
   #endif
   test_segment_iterator_operations();
   test_local_iterator_operations();
   return boost::report_errors();
}
