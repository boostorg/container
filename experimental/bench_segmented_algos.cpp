//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstring>
#include <utility>
#include <typeinfo>

#include <boost/container/deque.hpp>
#include <boost/container/experimental/nest.hpp>
#include <boost/move/detail/nsec_clock.hpp>

#include <boost/container/experimental/segmented_copy.hpp>
#include <boost/container/experimental/segmented_copy_if.hpp>
#include <boost/container/experimental/segmented_copy_n.hpp>
#include <boost/container/experimental/segmented_count.hpp>
#include <boost/container/experimental/segmented_count_if.hpp>
#include <boost/container/experimental/segmented_equal.hpp>
#include <boost/container/experimental/segmented_fill.hpp>
#include <boost/container/experimental/segmented_fill_n.hpp>
#include <boost/container/experimental/segmented_find.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
#include <boost/container/experimental/segmented_for_each.hpp>
#include <boost/container/experimental/segmented_generate.hpp>
#include <boost/container/experimental/segmented_generate_n.hpp>
#include <boost/container/experimental/segmented_is_partitioned.hpp>
#include <boost/container/experimental/segmented_is_sorted.hpp>
#include <boost/container/experimental/segmented_partition_copy.hpp>
#include <boost/container/experimental/segmented_remove.hpp>
#include <boost/container/experimental/segmented_remove_copy.hpp>
#include <boost/container/experimental/segmented_remove_copy_if.hpp>
#include <boost/container/experimental/segmented_remove_if.hpp>
#include <boost/container/experimental/segmented_replace.hpp>
#include <boost/container/experimental/segmented_replace_if.hpp>
#include <boost/container/experimental/segmented_reverse.hpp>
#include <boost/container/experimental/segmented_reverse_copy.hpp>
#include <boost/container/experimental/segmented_search.hpp>
#include <boost/container/experimental/segmented_search_n.hpp>
#include <boost/container/experimental/segmented_swap_ranges.hpp>
#include <boost/container/experimental/segmented_transform.hpp>

using boost::move_detail::cpu_timer;
using boost::move_detail::cpu_times;
using boost::move_detail::nanosecond_type;

namespace bc = boost::container;

volatile int sink = 0;

inline void escape(void* p)
{
   #if defined(_MSC_VER)
   sink = *static_cast<int*>(p);
   #elif defined(__GNUC__)
   asm volatile("" : : "g"(p) : "memory");
   #endif
}

//////////////////////////////////////////////////////////////////////////////
// Value types
//////////////////////////////////////////////////////////////////////////////

class MyInt
{
   int int_;

   public:
   inline explicit MyInt(int i = 0)
      : int_(i)
   {}

   inline MyInt(const MyInt &other)
      :  int_(other.int_)
   {}

   inline MyInt & operator=(const MyInt &other)
   {
      int_ = other.int_;
      return *this;
   }

   inline ~MyInt()
   {
      int_ = 0;
   }

   inline int int_value() const { return int_; }

   friend inline bool operator==(const MyInt& a, const MyInt& b) { return a.int_ == b.int_; }
   friend inline bool operator!=(const MyInt& a, const MyInt& b) { return a.int_ != b.int_; }
   friend inline bool operator<(const MyInt& a, const MyInt& b) { return a.int_ < b.int_; }
};

class MyFatInt
{
   int int0_;
   int int1_;
   int int2_;
   int int3_;
   int int4_;
   int int5_;
   int int6_;
   int int7_;

   public:
   inline explicit MyFatInt(int i = 0)
      : int0_(i++)
      , int1_(i++)
      , int2_(i++)
      , int3_(i++)
      , int4_(i++)
      , int5_(i++)
      , int6_(i++)
      , int7_(i++)
   {}

   inline MyFatInt(const MyFatInt &other)
      : int0_(other.int0_)
      , int1_(other.int1_)
      , int2_(other.int2_)
      , int3_(other.int3_)
      , int4_(other.int4_)
      , int5_(other.int5_)
      , int6_(other.int6_)
      , int7_(other.int7_)
   {}

   inline MyFatInt & operator=(const MyFatInt &other)
   {
      int0_ = other.int0_;
      int1_ = other.int1_;
      int2_ = other.int2_;
      int3_ = other.int3_;
      int4_ = other.int4_;
      int5_ = other.int5_;
      int6_ = other.int6_;
      int7_ = other.int7_;
      return *this;
   }

   inline ~MyFatInt()
   {
      int0_ = 0;
      int1_ = 0;
      int2_ = 0;
      int3_ = 0;
      int4_ = 0;
      int5_ = 0;
      int6_ = 0;
      int7_ = 0;
   }

   inline int int_value() const { return int0_; }

   friend inline bool operator==(const MyFatInt& a, const MyFatInt& b) { return a.int0_ == b.int0_; }
   friend inline bool operator!=(const MyFatInt& a, const MyFatInt& b) { return a.int0_ != b.int0_; }
   friend inline bool operator<(const MyFatInt& a, const MyFatInt& b) { return a.int0_ < b.int0_; }
};

inline int int_value(int x) { return x; }
inline int int_value(const MyInt& x) { return x.int_value(); }
inline int int_value(const MyFatInt& x) { return x.int_value(); }

//////////////////////////////////////////////////////////////////////////////
// Functors
//////////////////////////////////////////////////////////////////////////////

template<class T>
struct add_one
{
   T operator()(const T& x) const { return T(int_value(x) + 1); }
};

template<class T>
struct is_odd
{
   bool operator()(const T& x) const { return (int_value(x) & 1) != 0; }
};

template<class T>
struct summer
{
   int sum;
   summer() : sum(0) {}
   void operator()(const T& x) { sum = static_cast<int>((static_cast<unsigned>(sum) + static_cast<unsigned>(int_value(x)))); }
};

template<class T>
struct is_negative
{
   bool operator()(const T& x) const { return int_value(x) < 0; }
};

template<class T>
struct counter
{
   int n;
   counter() : n(0) {}
   T operator()() { return T(n++); }
};

//////////////////////////////////////////////////////////////////////////////
// C++03 fallbacks for C++11-only <algorithm> functions
//////////////////////////////////////////////////////////////////////////////

namespace bench_detail {

template<class InIt, class OutIt, class Pred>
OutIt copy_if(InIt first, InIt last, OutIt d_first, Pred pred)
{
   for (; first != last; ++first)
      if (pred(*first))
         *d_first++ = *first;
   return d_first;
}

template<class InIt, class Size, class OutIt>
OutIt copy_n(InIt first, Size count, OutIt result)
{
   for (Size i = 0; i < count; ++i, ++first, ++result)
      *result = *first;
   return result;
}

template<class FwdIt>
bool is_sorted(FwdIt first, FwdIt last)
{
   if (first == last) return true;
   FwdIt next = first;
   for (++next; next != last; first = next, ++next)
      if (*next < *first) return false;
   return true;
}

template<class InIt, class Pred>
bool is_partitioned(InIt first, InIt last, Pred pred)
{
   for (; first != last; ++first)
      if (!pred(*first)) break;
   for (; first != last; ++first)
      if (pred(*first)) return false;
   return true;
}

template<class InIt, class OutIt1, class OutIt2, class Pred>
std::pair<OutIt1, OutIt2>
partition_copy(InIt first, InIt last,
               OutIt1 out_true, OutIt2 out_false, Pred pred)
{
   for (; first != last; ++first) {
      if (pred(*first))
         *out_true++ = *first;
      else
         *out_false++ = *first;
   }
   return std::pair<OutIt1, OutIt2>(out_true, out_false);
}

} // namespace bench_detail

//////////////////////////////////////////////////////////////////////////////
// Fill helpers
//////////////////////////////////////////////////////////////////////////////

template<class C>
void fill_test_data(C& c, std::size_t n)
{
   typedef typename C::value_type VT;
   for (std::size_t i = 0; i < n; ++i)
      c.push_back(VT(static_cast<int>(i)));
}

template<class T, class A, class O>
void fill_test_data(bc::nest<T,A,O>& c, std::size_t n)
{
   for (std::size_t i = 0; i < n; ++i)
      c.insert(T(static_cast<int>(i)));
}

//////////////////////////////////////////////////////////////////////////////
// Benchmark helpers
//////////////////////////////////////////////////////////////////////////////

inline double calc_ns_per_elem(nanosecond_type ns,
                               std::size_t iters, std::size_t elems)
{
   return double(ns) / double(iters * elems);
}

inline void print_ratio(const char* algo, const char* cname,
                        double std_ns, double seg_ns)
{
   double ratio = (seg_ns > 0.0) ? std_ns / seg_ns : 0.0;
   std::cout << std::left << std::setw(24) << algo
             << std::setw(14) << cname
             << std::right << std::setw(10) << std::fixed << std::setprecision(2)
             << ratio << "x\n";
}

//////////////////////////////////////////////////////////////////////////////
// Individual benchmarks
//////////////////////////////////////////////////////////////////////////////

template<class C>
void bench_for_each(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      summer<VT> s;
      s = std::for_each(c.begin(), c.end(), s);
      result = s.sum;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      summer<VT> s;
      s = bc::segmented_for_each(c.begin(), c.end(), s);
      result = s.sum;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("for_each", cname, r1, r2);
}

template<class C>
void bench_copy(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   std::vector<VT> out(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::copy(c.begin(), c.end(), out.begin());
      t1.stop();
   }
   escape(&out[0]);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_copy(c.begin(), c.end(), out.begin());
      t2.stop();
   }
   escape(&out[0]);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("copy", cname, r1, r2);
}

template<class C>
void bench_copy_if(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   std::vector<VT> out(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      bench_detail::copy_if(c.begin(), c.end(), out.begin(), is_odd<VT>());
      t1.stop();
   }
   escape(&out[0]);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_copy_if(c.begin(), c.end(), out.begin(), is_odd<VT>());
      t2.stop();
   }
   escape(&out[0]);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("copy_if", cname, r1, r2);
}

template<class C>
void bench_fill(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   VT val(42);

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::fill(c.begin(), c.end(), val);
      t1.stop();
   }
   escape(&val);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_fill(c.begin(), c.end(), val);
      t2.stop();
   }
   escape(&val);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("fill", cname, r1, r2);
}

template<class C>
void bench_count(C& c, std::size_t iters, const char* cname,
                 const typename C::value_type& val, const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      result = static_cast<int>(std::count(c.begin(), c.end(), val));
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      result = static_cast<int>(bc::segmented_count(c.begin(), c.end(), val));
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C, class Pred>
void bench_count_if(C& c, std::size_t iters, const char* cname,
                    Pred pred, const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      result = static_cast<int>(std::count_if(c.begin(), c.end(), pred));
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      result = static_cast<int>(bc::segmented_count_if(c.begin(), c.end(), pred));
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C>
void bench_find(C& c, std::size_t iters, const char* cname,
                const typename C::value_type& val, const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      typename C::iterator it = std::find(c.begin(), c.end(), val);
      result = (it == c.end()) ? 0 : 1;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      typename C::iterator it = bc::segmented_find(c.begin(), c.end(), val);
      result = (it == c.end()) ? 0 : 1;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C, class Pred>
void bench_find_if(C& c, std::size_t iters, const char* cname,
                   Pred pred, const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      typename C::iterator it = std::find_if(c.begin(), c.end(), pred);
      result = (it != c.end()) ? int_value(*it) : -1;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      typename C::iterator it = bc::segmented_find_if(c.begin(), c.end(), pred);
      result = (it != c.end()) ? int_value(*it) : -1;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C>
void bench_equal(C& c, C& c2, std::size_t iters, const char* cname,
                 const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      result = std::equal(c.begin(), c.end(), c2.begin()) ? 1 : 0;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      result = bc::segmented_equal(c.begin(), c.end(), c2.begin()) ? 1 : 0;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C>
void bench_replace(C& c, std::size_t iters, const char* cname,
                   const typename C::value_type& old_val,
                   const typename C::value_type& new_val, const char* label)
{
   typedef typename C::value_type VT;
   VT v = *c.begin();

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::replace(c.begin(), c.end(), old_val, new_val);
      t1.stop();
   }
   escape(&v);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_replace(c.begin(), c.end(), old_val, new_val);
      t2.stop();
   }
   escape(&v);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C, class Pred>
void bench_replace_if(C& c, std::size_t iters, const char* cname,
                      Pred pred, const typename C::value_type& new_val,
                      const char* label)
{
   typedef typename C::value_type VT;
   VT v = *c.begin();

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::replace_if(c.begin(), c.end(), pred, new_val);
      t1.stop();
   }
   escape(&v);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_replace_if(c.begin(), c.end(), pred, new_val);
      t2.stop();
   }
   escape(&v);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C>
void bench_transform(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   std::vector<VT> out(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::transform(c.begin(), c.end(), out.begin(), add_one<VT>());
      t1.stop();
   }
   escape(&out[0]);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_transform(c.begin(), c.end(), out.begin(), add_one<VT>());
      t2.stop();
   }
   escape(&out[0]);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("transform", cname, r1, r2);
}

template<class C>
void bench_fill_n(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   VT val(42);
   typename C::difference_type n =
      static_cast<typename C::difference_type>(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::fill_n(c.begin(), n, val);
      t1.stop();
   }
   escape(&val);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_fill_n(c.begin(), n, val);
      t2.stop();
   }
   escape(&val);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("fill_n", cname, r1, r2);
}

template<class C>
void bench_copy_n(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   std::vector<VT> out(c.size());
   typename C::difference_type n =
      static_cast<typename C::difference_type>(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      bench_detail::copy_n(c.begin(), n, out.begin());
      t1.stop();
   }
   escape(&out[0]);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_copy_n(c.begin(), n, out.begin());
      t2.stop();
   }
   escape(&out[0]);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("copy_n", cname, r1, r2);
}

template<class C>
void bench_generate(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::generate(c.begin(), c.end(), counter<VT>());
      result = int_value(*c.begin());
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_generate(c.begin(), c.end(), counter<VT>());
      result = int_value(*c.begin());
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("generate", cname, r1, r2);
}

template<class C>
void bench_generate_n(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   int result = 0;
   typename C::difference_type n =
      static_cast<typename C::difference_type>(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::generate_n(c.begin(), n, counter<VT>());
      result = int_value(*c.begin());
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_generate_n(c.begin(), n, counter<VT>());
      result = int_value(*c.begin());
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("generate_n", cname, r1, r2);
}

template<class C>
void bench_remove(C& c, std::size_t iters, const char* cname,
                  const typename C::value_type& val, const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      typename C::iterator it = std::remove(c.begin(), c.end(), val);
      result = (it == c.end()) ? 1 : 0;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      typename C::iterator it = bc::segmented_remove(c.begin(), c.end(), val);
      result = (it == c.end()) ? 1 : 0;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C, class Pred>
void bench_remove_if(C& c, std::size_t iters, const char* cname,
                     Pred pred, const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      typename C::iterator it = std::remove_if(c.begin(), c.end(), pred);
      result = (it == c.end()) ? 1 : 0;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      typename C::iterator it = bc::segmented_remove_if(c.begin(), c.end(), pred);
      result = (it == c.end()) ? 1 : 0;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C>
void bench_remove_copy(C& c, std::size_t iters, const char* cname,
                       const typename C::value_type& val, const char* label)
{
   typedef typename C::value_type VT;
   std::vector<VT> out(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::remove_copy(c.begin(), c.end(), out.begin(), val);
      t1.stop();
   }
   escape(&out[0]);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_remove_copy(c.begin(), c.end(), out.begin(), val);
      t2.stop();
   }
   escape(&out[0]);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C, class Pred>
void bench_remove_copy_if(C& c, std::size_t iters, const char* cname,
                          Pred pred, const char* label)
{
   typedef typename C::value_type VT;
   std::vector<VT> out(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::remove_copy_if(c.begin(), c.end(), out.begin(), pred);
      t1.stop();
   }
   escape(&out[0]);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_remove_copy_if(c.begin(), c.end(), out.begin(), pred);
      t2.stop();
   }
   escape(&out[0]);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C>
void bench_reverse(C& c, std::size_t iters, const char* cname)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::reverse(c.begin(), c.end());
      result = int_value(*c.begin());
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_reverse(c.begin(), c.end());
      result = int_value(*c.begin());
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("reverse", cname, r1, r2);
}

template<class C>
void bench_reverse_copy(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   std::vector<VT> out(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::reverse_copy(c.begin(), c.end(), out.begin());
      t1.stop();
   }
   escape(&out[0]);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_reverse_copy(c.begin(), c.end(), out.begin());
      t2.stop();
   }
   escape(&out[0]);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("reverse_copy", cname, r1, r2);
}

template<class C>
void bench_is_sorted(C& c, std::size_t iters, const char* cname,
                     const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      result = bench_detail::is_sorted(c.begin(), c.end()) ? 1 : 0;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      result = bc::segmented_is_sorted(c.begin(), c.end()) ? 1 : 0;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C, class Pred>
void bench_is_partitioned(C& c, std::size_t iters, const char* cname,
                          Pred pred, const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      result = bench_detail::is_partitioned(c.begin(), c.end(), pred) ? 1 : 0;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      result = bc::segmented_is_partitioned(c.begin(), c.end(), pred) ? 1 : 0;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C>
void bench_swap_ranges(C& c, std::size_t iters, const char* cname)
{
   C c2(c);
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      std::swap_ranges(c.begin(), c.end(), c2.begin());
      result = int_value(*c.begin());
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_swap_ranges(c.begin(), c.end(), c2.begin());
      result = int_value(*c.begin());
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("swap_ranges", cname, r1, r2);
}

template<class C>
void bench_search(C& c, std::size_t iters, const char* cname,
                  const typename C::value_type* pattern, std::size_t pat_size,
                  const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      typename C::iterator it = std::search(c.begin(), c.end(), pattern, pattern + pat_size);
      result = (it == c.end()) ? 0 : 1;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      typename C::iterator it = bc::segmented_search(c.begin(), c.end(), pattern, pattern + pat_size);
      result = (it == c.end()) ? 0 : 1;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C>
void bench_search_n(C& c, std::size_t iters, const char* cname,
                    typename C::difference_type count,
                    const typename C::value_type& val, const char* label)
{
   int result = 0;

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      typename C::iterator it = std::search_n(c.begin(), c.end(), count, val);
      result = (it == c.end()) ? 0 : 1;
      t1.stop();
   }
   escape(&result);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      typename C::iterator it = bc::segmented_search_n(c.begin(), c.end(), count, val);
      result = (it == c.end()) ? 0 : 1;
      t2.stop();
   }
   escape(&result);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio(label, cname, r1, r2);
}

template<class C>
void bench_partition_copy(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;
   std::vector<VT> t_out(c.size());
   std::vector<VT> f_out(c.size());

   cpu_timer t1;
   for (std::size_t i = 0; i < iters; ++i) {
      t1.resume();
      bench_detail::partition_copy(c.begin(), c.end(),
         t_out.begin(), f_out.begin(), is_odd<VT>());
      t1.stop();
   }
   escape(&t_out[0]);
   escape(&f_out[0]);
   double r1 = calc_ns_per_elem(t1.elapsed().wall, iters, c.size());

   cpu_timer t2;
   for (std::size_t i = 0; i < iters; ++i) {
      t2.resume();
      bc::segmented_partition_copy(c.begin(), c.end(),
         t_out.begin(), f_out.begin(), is_odd<VT>());
      t2.stop();
   }
   escape(&t_out[0]);
   escape(&f_out[0]);
   double r2 = calc_ns_per_elem(t2.elapsed().wall, iters, c.size());
   print_ratio("partition_copy", cname, r1, r2);
}

//////////////////////////////////////////////////////////////////////////////
// Run all benchmarks for a container type
//////////////////////////////////////////////////////////////////////////////

template<class C>
void run_all(C& c, std::size_t iters, const char* cname)
{
   typedef typename C::value_type VT;

   bench_for_each(c, iters, cname);
   bench_copy(c, iters, cname);
   bench_copy_if(c, iters, cname);
   bench_copy_n(c, iters, cname);
   bench_fill(c, iters, cname);
   bench_fill_n(c, iters, cname);
   bench_generate(c, iters, cname);
   bench_generate_n(c, iters, cname);

   bench_count(c, iters, cname, VT(-1), "count(miss)");
   bench_count(c, iters, cname, VT(0),  "count(hit)");
   bench_count_if(c, iters, cname, is_odd<VT>(),      "count_if(hit)");
   bench_count_if(c, iters, cname, is_negative<VT>(), "count_if(miss)");

   bench_find(c, iters, cname, VT(-1), "find(miss)");
   bench_find(c, iters, cname, VT(static_cast<int>(c.size() / 2)), "find(hit)");
   bench_find_if(c, iters, cname, is_odd<VT>(),      "find_if(hit)");
   bench_find_if(c, iters, cname, is_negative<VT>(), "find_if(miss)");

   {
      C c2(c);
      bench_equal(c, c2, iters, cname, "equal(hit)");
      typename C::iterator last = c2.end();
      --last;
      *last = VT(-1);
      bench_equal(c, c2, iters, cname, "equal(miss)");
   }

   {
      VT miss_pat[] = {VT(-1), VT(-2), VT(-3)};
      bench_search(c, iters, cname, miss_pat, 3, "search(miss)");
      int half = static_cast<int>(c.size() / 2);
      VT hit_pat[] = {VT(half), VT(half + 1), VT(half + 2)};
      bench_search(c, iters, cname, hit_pat, 3, "search(hit)");
   }
   bench_search_n(c, iters, cname, 3, VT(-1), "search_n(miss)");
   bench_search_n(c, iters, cname, 1, VT(static_cast<int>(c.size() / 2)), "search_n(hit)");

   bench_is_sorted(c, iters, cname, "is_sorted(hit)");
   {
      C c2(c);
      typename C::iterator last = c2.end();
      --last;
      *last = VT(0);
      bench_is_sorted(c2, iters, cname, "is_sorted(miss)");
   }

   bench_is_partitioned(c, iters, cname, is_negative<VT>(), "is_partitioned(hit)");
   {
      C c2(c);
      typename C::iterator last = c2.end();
      --last;
      *last = VT(-1);
      bench_is_partitioned(c2, iters, cname, is_negative<VT>(), "is_partitioned(miss)");
   }

   bench_remove(c, iters, cname, VT(-1), "remove(miss)");
   bench_remove(c, iters, cname, VT(0),  "remove(hit)");
   bench_remove_if(c, iters, cname, is_negative<VT>(), "remove_if(miss)");
   bench_remove_if(c, iters, cname, is_odd<VT>(),      "remove_if(hit)");
   bench_remove_copy(c, iters, cname, VT(-1), "remove_copy(miss)");
   bench_remove_copy(c, iters, cname, VT(0),  "remove_copy(hit)");
   bench_remove_copy_if(c, iters, cname, is_negative<VT>(), "remove_copy_if(miss)");
   bench_remove_copy_if(c, iters, cname, is_odd<VT>(),      "remove_copy_if(hit)");

   bench_replace(c, iters, cname, VT(-1), VT(-2), "replace(miss)");
   bench_replace(c, iters, cname, VT(0),  VT(0),  "replace(hit)");
   bench_replace_if(c, iters, cname, is_odd<VT>(),      VT(-2), "replace_if(hit)");
   bench_replace_if(c, iters, cname, is_negative<VT>(), VT(-2), "replace_if(miss)");

   bench_reverse(c, iters, cname);
   bench_reverse_copy(c, iters, cname);
   bench_swap_ranges(c, iters, cname);
   bench_partition_copy(c, iters, cname);
   bench_transform(c, iters, cname);
}

//////////////////////////////////////////////////////////////////////////////
// Run all benchmarks for a given value type
//////////////////////////////////////////////////////////////////////////////

template<class T>
void run_benchmarks()
{
   #ifdef NDEBUG
   const std::size_t N    = 100000;
   const std::size_t iter = 500;
   #else
   const std::size_t N    = 10000;
   const std::size_t iter = 10;
   #endif

   std::cout << "\n=== Segmented algorithm benchmark [" << typeid(T).name() << "] ===\n"
             << "Elements: " << N << "   Iterations: " << iter << "\n\n";
   
   {
      std::cout << "--- bc::deque<" << typeid(T).name() << "> ---\n";
      bc::deque<T> dq;
      fill_test_data(dq, N);
      run_all(dq, iter, "deque");
      std::cout << "\n";
   }
/*
   {
      std::cout << "--- bc::nest<" << typeid(T).name() << "> ---\n";
      bc::nest<T> nt;
      fill_test_data(nt, N);
      run_all(nt, iter, "nest");
      std::cout << "\n";
   }*/
}

//////////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////////

int main()
{
   //run_benchmarks<int>();
   run_benchmarks<MyInt>();
   //run_benchmarks<MyFatInt>();
   return 0;
}
