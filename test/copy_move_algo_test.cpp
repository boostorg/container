//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable : 4530) // C++ exception handler used, but unwind semantics are not enabled
#endif

// the tests trigger deprecation warnings when compiled with msvc in C++17 mode
#if defined(_MSVC_LANG) && _MSVC_LANG > 201402
// warning STL4009: std::allocator<void> is deprecated in C++17
# define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#endif

#include "lightweight_test.hpp"
#include <boost/container/detail/copy_move_algo.hpp>
#include <boost/container/detail/advanced_insert_int.hpp>
#include <boost/container/detail/algorithm.hpp>
#include <boost/container/detail/placement_new.hpp>
#include <boost/container/allocator_traits.hpp>
#include <boost/container/new_allocator.hpp>
#include <boost/move/unique_ptr.hpp>
#include <boost/move/make_unique.hpp>
#include <boost/container/detail/mpl.hpp>
#include <boost/move/iterator.hpp>
#include <cstddef>
#include <iterator>
#include <vector>
#include "movable_int.hpp"

using namespace boost::container;

namespace boost {
namespace container {
namespace test {

//This function tests all the possible combinations when
//inserting data in a vector and expanding backwards
template<class ValueType>
void test_expand_backward_forward_and_insert_alloc()
{
   typedef ValueType value_type;

   //Distance old and new buffer
   const unsigned int Offset[] =
   { 350, 300, 250, 200, 150, 100, 150, 100,
      150,  50,  50,  50 };
   //Initial vector size
   const unsigned int InitialSize[] =
   { 200, 200, 200, 200, 200, 200, 200, 200,
      200, 200, 200, 200 };
   //Size of the data to insert
   const unsigned int InsertSize[] =
   { 100, 100, 100, 100, 100, 100, 200, 200,
      300,  25, 100, 200 };
   //Number of tests
   const unsigned int Iterations = sizeof(InsertSize) / sizeof(int);

   //Insert position
   const int Position[] =
   { 0, 100,  200 };

   for (unsigned backmove = 2u; backmove != 0u; ) {
      --backmove;
      for (unsigned int pos = 0; pos < sizeof(Position) / sizeof(Position[0]); ++pos) {
         BOOST_TEST(life_count<value_type>::check(0));

         for (unsigned int iteration = 0; iteration < Iterations; ++iteration)
         {
            typedef std::vector<value_type>  std_vector_val_t;
            typedef boost::container::new_allocator<value_type> allocator_type;
            typedef dtl::insert_range_proxy<allocator_type, typename std_vector_val_t::iterator> proxy_t;
            {

               std_vector_val_t initial_data;
               initial_data.resize(InitialSize[iteration]);
               for (unsigned int i = 0; i < InitialSize[iteration]; ++i) {
                  initial_data[i] = static_cast<value_type>((int)i);
               }
               BOOST_TEST(life_count<value_type>::check(InitialSize[iteration]));

               std_vector_val_t data_to_insert;
               data_to_insert.resize(InsertSize[iteration]);
               for (unsigned int i = 0; i < InsertSize[iteration]; ++i) {
                  data_to_insert[i] = static_cast<value_type>(-(int)i);
               }

               BOOST_TEST(life_count<value_type>::check(InitialSize[iteration] + InsertSize[iteration]));

               const unsigned int BufferSize = InitialSize[iteration] + InsertSize[iteration] + Offset[iteration];
               boost::movelib::unique_ptr<char[]> memptr =
                  boost::movelib::make_unique_definit<char[]>(BufferSize * sizeof(value_type));
               value_type* memory = move_detail::force_ptr<value_type*>(memptr.get());
               allocator_type a;

               value_type* final_memory;
               value_type* initial_memory;
               if(backmove){
                  initial_memory = memory + Offset[iteration];
                  final_memory = memory;
               }
               else{
                  initial_memory = memory + BufferSize - InitialSize[iteration] - Offset[iteration];
                  final_memory = memory + BufferSize - InitialSize[iteration] - InsertSize[iteration];
               }

               for (unsigned int i = 0; i < InitialSize[iteration]; ++i) {
                  allocator_traits<allocator_type>::construct(a, &initial_memory[i], (int)i);
               }

               proxy_t proxy(data_to_insert.begin());
               boost::container::expand_backward_forward_and_insert_alloc
               (initial_memory, initial_data.size(), final_memory, initial_memory + Position[pos]
                  , data_to_insert.size(), proxy, a);

               BOOST_TEST(life_count<value_type>::check(InitialSize[iteration] * 2 + InsertSize[iteration] * 2));

               initial_data.insert(initial_data.begin() + Position[pos]
                  , data_to_insert.begin(), data_to_insert.end());

               //Now check that values are equal
               BOOST_TEST(boost::container::algo_equal(initial_data.begin(), initial_data.end(), final_memory));

               boost::container::destroy_alloc_n(a, final_memory, InitialSize[iteration] + InsertSize[iteration]);
            }
            BOOST_TEST(life_count<value_type>::check(0));
         }
      }
   }
}

//////////////////////////////////////////////////////////////////////////////
//
// Overloads of the copy and move algorithms.
//
// - int: memmove for contiguous iterators, element loop for the others.
// - counted: copy and move operations that are not trivial and are counted.
// - user_move: trivial copy operations and destructor, counted move
//   operations with Boost.Move. A copy can use memmove, a move must not.
// - fake_trivial: counted operations, with triviality specialized to true.
//   The counts show which version (memcpy/loop) was selected.
//
// Iterators: raw pointers (contiguous) and iterator (bidirectional, not
// contiguous). The copy algorithms also run with move_iterator sources.
//
//////////////////////////////////////////////////////////////////////////////

struct counters
{
   std::size_t copy_ctor, move_ctor, copy_assign, move_assign, dtor;
};

inline counters &count()
{
   static counters c;
   return c;
}

inline void reset()
{
   counters z = { 0u, 0u, 0u, 0u, 0u };
   count() = z;
}

class counted
{
   BOOST_COPYABLE_AND_MOVABLE(counted)
   int v_;

   public:
   explicit counted(int v = 0) : v_(v) {}

   counted(const counted &o) : v_(o.v_)
   {  ++count().copy_ctor;  }

   counted(BOOST_RV_REF(counted) o) : v_(o.v_)
   {  o.v_ = -1; ++count().move_ctor;  }

   counted &operator=(BOOST_COPY_ASSIGN_REF(counted) o)
   {  v_ = o.v_; ++count().copy_assign; return *this;  }

   counted &operator=(BOOST_RV_REF(counted) o)
   {  v_ = o.v_; o.v_ = -1; ++count().move_assign; return *this;  }

   ~counted()
   {  ++count().dtor;  }

   int value() const {  return v_;  }
};

inline int value(int v)                  {  return v;  }
inline int value(const counted &v)   {  return v.value();  }

//Which operations of a type are counted
template<class T>
struct traits
{
   BOOST_STATIC_CONSTEXPR bool copies = false;
   BOOST_STATIC_CONSTEXPR bool moves  = false;
   BOOST_STATIC_CONSTEXPR bool dtor   = false;
};

template<>
struct traits<counted>
{
   BOOST_STATIC_CONSTEXPR bool copies = true;
   BOOST_STATIC_CONSTEXPR bool moves  = true;
   BOOST_STATIC_CONSTEXPR bool dtor   = true;
};

class user_move
{
   //The _ALT form does not declare a copy assignment, so it can be trivial
   BOOST_COPYABLE_AND_MOVABLE_ALT(user_move)

   public:
   int v;

   explicit user_move(int i = 0) : v(i) {}

   //The copy operations are trivial: implicit in C++03, defaulted otherwise
   #if !defined(BOOST_NO_CXX11_DEFAULTED_FUNCTIONS)
   user_move(const user_move &) = default;
   user_move &operator=(const user_move &) = default;
   #elif !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
   user_move(const user_move &o) : v(o.v) {}
   user_move &operator=(const user_move &o) {  v = o.v; return *this;  }
   #endif

   user_move(BOOST_RV_REF(user_move) o) BOOST_NOEXCEPT : v(o.v)
   {  o.v = -1; ++count().move_ctor;  }

   user_move &operator=(BOOST_RV_REF(user_move) o) BOOST_NOEXCEPT
   {  v = o.v; o.v = -1; ++count().move_assign; return *this;  }
};

inline int value(const user_move &v) {  return v.v;  }

template<>
struct traits<user_move>
{
   BOOST_STATIC_CONSTEXPR bool copies = false;
   BOOST_STATIC_CONSTEXPR bool moves  = true;
   BOOST_STATIC_CONSTEXPR bool dtor   = false;
};

class fake_trivial
{
   BOOST_COPYABLE_AND_MOVABLE(fake_trivial)
   int v_;

   public:
   explicit fake_trivial(int v = 0) : v_(v) {}

   fake_trivial(const fake_trivial &o) : v_(o.v_)
   {  ++count().copy_ctor;  }

   fake_trivial(BOOST_RV_REF(fake_trivial) o) : v_(o.v_)
   {  o.v_ = -1; ++count().move_ctor;  }

   fake_trivial &operator=(BOOST_COPY_ASSIGN_REF(fake_trivial) o)
   {  v_ = o.v_; ++count().copy_assign; return *this;  }

   fake_trivial &operator=(BOOST_RV_REF(fake_trivial) o)
   {  v_ = o.v_; o.v_ = -1; ++count().move_assign; return *this;  }

   ~fake_trivial()
   {  ++count().dtor;  }

   int value() const {  return v_;  }
};

inline int value(const fake_trivial &v)   {  return v.value();  }

template<>
struct traits<fake_trivial>
{
   BOOST_STATIC_CONSTEXPR bool copies = true;
   BOOST_STATIC_CONSTEXPR bool moves  = true;
   BOOST_STATIC_CONSTEXPR bool dtor   = true;
};

//Expected versions with contiguous iterators: memmove for copies, memmove for
//moves, and no destructor call for a trivial destructor
template<class T>
struct expect
{
   BOOST_STATIC_CONSTEXPR bool copy_memmove = true;
   BOOST_STATIC_CONSTEXPR bool move_memmove = true;
   BOOST_STATIC_CONSTEXPR bool trivial_dtor = true;
};

template<>
struct expect<counted>
{
   BOOST_STATIC_CONSTEXPR bool copy_memmove = false;
   BOOST_STATIC_CONSTEXPR bool move_memmove = false;
   BOOST_STATIC_CONSTEXPR bool trivial_dtor = false;
};

template<>
struct expect<user_move>
{
   BOOST_STATIC_CONSTEXPR bool copy_memmove = true;
   BOOST_STATIC_CONSTEXPR bool move_memmove = false;
   BOOST_STATIC_CONSTEXPR bool trivial_dtor = true;
};

}  //namespace test {
}  //namespace container {

namespace move_detail {

//fake_trivial claims to be trivial
template<>
struct is_trivially_copy_constructible< ::boost::container::test::fake_trivial>
{  static const bool value = true;  };

template<>
struct is_trivially_copy_assignable< ::boost::container::test::fake_trivial>
{  static const bool value = true;  };

template<>
struct is_trivially_move_constructible< ::boost::container::test::fake_trivial>
{  static const bool value = true;  };

template<>
struct is_trivially_move_assignable< ::boost::container::test::fake_trivial>
{  static const bool value = true;  };

template<>
struct is_trivially_destructible< ::boost::container::test::fake_trivial>
{  static const bool value = true;  };

}  //namespace move_detail {

namespace container {
namespace test {

//The counters of the operations that T counts have the expected values
template<class T>
bool counts_are(std::size_t copy_ctor, std::size_t move_ctor, std::size_t copy_assign, std::size_t move_assign, std::size_t dtor)
{
   const counters &c = count();
   if(traits<T>::copies && (c.copy_ctor != copy_ctor || c.copy_assign != copy_assign))
      return false;
   if(traits<T>::moves && (c.move_ctor != move_ctor || c.move_assign != move_assign))
      return false;
   if(traits<T>::dtor && c.dtor != dtor)
      return false;
   return true;
}

//Bidirectional iterator that is not contiguous: the element loops run
template<class T>
class iterator
{
   T *p_;

   public:
   typedef T                                 value_type;
   typedef std::ptrdiff_t                    difference_type;
   typedef T*                                pointer;
   typedef T&                                reference;
   typedef std::bidirectional_iterator_tag   iterator_category;

   iterator() : p_() {}
   explicit iterator(T *p) : p_(p) {}

   T &operator*() const {  return *p_;  }
   T *operator->() const {  return p_;  }
   iterator &operator++() {  ++p_; return *this;  }
   iterator operator++(int) {  iterator t(*this); ++p_; return t;  }
   iterator &operator--() {  --p_; return *this;  }
   iterator operator--(int) {  iterator t(*this); --p_; return t;  }
   T *base() const {  return p_;  }

   friend bool operator==(const iterator &a, const iterator &b) {  return a.p_ == b.p_;  }
   friend bool operator!=(const iterator &a, const iterator &b) {  return a.p_ != b.p_;  }
};

struct raw_mode
{
   BOOST_STATIC_CONSTEXPR bool contiguous = true;
   template<class T> struct iter {  typedef T* type;  };
   template<class T> static T *make(T *p) {  return p;  }
   template<class T> static T *base(T *p) {  return p;  }
};

struct wrapped_mode
{
   BOOST_STATIC_CONSTEXPR bool contiguous = false;
   template<class T> struct iter {  typedef iterator<T> type;  };
   template<class T> static iterator<T> make(T *p) {  return iterator<T>(p);  }
   template<class T> static T *base(iterator<T> i) {  return i.base();  }
};

//Uninitialized memory for up to cap elements; the first n are constructed
template<class T>
class buffer
{
   buffer(const buffer &);
   buffer &operator=(const buffer &);

   public:
   typedef new_allocator<T> allocator_type;
   allocator_type a;
   T *p;
   std::size_t cap, n;

   explicit buffer(std::size_t c)
      : a(), p(a.allocate(c)), cap(c), n(0u)
   {}

   ~buffer()
   {
      this->destroy();
      a.deallocate(p, cap);
   }

   //Constructs k elements with the values first, first + 1, ...
   void construct_seq(std::size_t k, int first)
   {
      for(std::size_t i = 0; i != k; ++i)
         allocator_traits<allocator_type>::construct(a, p + i, first + int(i));
      n = k;
   }

   void destroy()
   {
      for(std::size_t i = 0; i != n; ++i)
         allocator_traits<allocator_type>::destroy(a, p + i);
      n = 0u;
   }

   //The k elements from pos have the values first, first + 1, ...
   bool seq_is(std::size_t pos, std::size_t k, int first) const
   {
      for(std::size_t i = 0; i != k; ++i){
         if(value(p[pos + i]) != first + int(i))
            return false;
      }
      return true;
   }

   //The k elements from pos were moved from, if T shows it
   bool moved_from(std::size_t pos, std::size_t k) const
   {
      for(std::size_t i = 0; traits<T>::moves && i != k; ++i){
         if(value(p[pos + i]) != -1)
            return false;
      }
      return true;
   }

   //A memmove keeps the source values first, first + 1, ..., a move
   //constructor or assignment leaves them moved from
   bool moved_from_or_kept(std::size_t pos, std::size_t k, int first, bool memmoved) const
   {  return memmoved ? this->seq_is(pos, k, first) : this->moved_from(pos, k);  }
};

const std::size_t n = 5u;

//The memmove versions are expected: contiguous iterators and a type that allows it
template<class T, class Mode>
struct memmove_copy
{
   BOOST_STATIC_CONSTEXPR bool value = Mode::contiguous && expect<T>::copy_memmove;
};

template<class T, class Mode>
struct memmove_move
{
   BOOST_STATIC_CONSTEXPR bool value = Mode::contiguous && expect<T>::move_memmove;
};

template<class T, class Mode>
void test_uninitialized_move()
{
   typedef typename Mode::template iter<T>::type iter_t;
   const bool mm = memmove_move<T, Mode>::value;
   const std::size_t moves = mm ? 0u : n;
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      reset();
      const iter_t r = boost::container::uninitialized_move_alloc
         (d.a, Mode::make(s.p), Mode::make(s.p + n), Mode::make(d.p));
      d.n = n;
      BOOST_TEST(Mode::base(r) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(s.moved_from_or_kept(0u, n, 1, mm));
      BOOST_TEST(counts_are<T>(0u, moves, 0u, 0u, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      reset();
      const iter_t r = boost::container::uninitialized_move_alloc_n
         (d.a, Mode::make(s.p), n, Mode::make(d.p));
      d.n = n;
      BOOST_TEST(Mode::base(r) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(s.moved_from_or_kept(0u, n, 1, mm));
      BOOST_TEST(counts_are<T>(0u, moves, 0u, 0u, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      reset();
      const iter_t r = boost::container::uninitialized_move_alloc_n_source
         (d.a, Mode::make(s.p), n, Mode::make(d.p));
      d.n = n;
      BOOST_TEST(Mode::base(r) == s.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(s.moved_from_or_kept(0u, n, 1, mm));
      BOOST_TEST(counts_are<T>(0u, moves, 0u, 0u, 0u));
   }
}

//MoveSrc: the source is a move_iterator, so the elements are moved
template<class T, class Mode, bool MoveSrc>
void test_uninitialized_copy()
{
   typedef typename Mode::template iter<T>::type iter_t;
   typedef typename dtl::if_c<MoveSrc, boost::move_iterator<iter_t>, iter_t>::type src_t;
   const bool mm = MoveSrc ? memmove_move<T, Mode>::value : memmove_copy<T, Mode>::value;
   const std::size_t copies = (MoveSrc || mm) ? 0u : n;
   const std::size_t moves  = (MoveSrc && !mm) ? n : 0u;
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      reset();
      const iter_t r = boost::container::uninitialized_copy_alloc
         (d.a, src_t(Mode::make(s.p)), src_t(Mode::make(s.p + n)), Mode::make(d.p));
      d.n = n;
      BOOST_TEST(Mode::base(r) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(MoveSrc ? s.moved_from_or_kept(0u, n, 1, mm) : s.seq_is(0u, n, 1));
      BOOST_TEST(counts_are<T>(copies, moves, 0u, 0u, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      reset();
      const iter_t r = boost::container::uninitialized_copy_alloc_n
         (d.a, src_t(Mode::make(s.p)), n, Mode::make(d.p));
      d.n = n;
      BOOST_TEST(Mode::base(r) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(MoveSrc ? s.moved_from_or_kept(0u, n, 1, mm) : s.seq_is(0u, n, 1));
      BOOST_TEST(counts_are<T>(copies, moves, 0u, 0u, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      reset();
      const src_t r = boost::container::uninitialized_copy_alloc_n_source
         (d.a, src_t(Mode::make(s.p)), n, Mode::make(d.p));
      d.n = n;
      BOOST_TEST(r == src_t(Mode::make(s.p + n)));
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(MoveSrc ? s.moved_from_or_kept(0u, n, 1, mm) : s.seq_is(0u, n, 1));
      BOOST_TEST(counts_are<T>(copies, moves, 0u, 0u, 0u));
   }
}

template<class T, class Mode>
void test_uninitialized_init_and_fill()
{
   typedef typename Mode::template iter<T>::type iter_t;
   {
      buffer<T> d(n);
      const iter_t r = boost::container::uninitialized_value_init_alloc_n(d.a, n, Mode::make(d.p));
      d.n = n;
      BOOST_TEST(Mode::base(r) == d.p + n);
      for(std::size_t i = 0; i != n; ++i)
         BOOST_TEST(value(d.p[i]) == 0);
   }
   {
      buffer<T> d(n);
      const T v(7);
      reset();
      boost::container::uninitialized_fill_alloc(d.a, Mode::make(d.p), Mode::make(d.p + n), v);
      d.n = n;
      for(std::size_t i = 0; i != n; ++i)
         BOOST_TEST(value(d.p[i]) == 7);
      BOOST_TEST(counts_are<T>(n, 0u, 0u, 0u, 0u));
   }
   {
      buffer<T> d(n);
      const T v(7);
      reset();
      const iter_t r = boost::container::uninitialized_fill_alloc_n(d.a, v, n, Mode::make(d.p));
      d.n = n;
      BOOST_TEST(Mode::base(r) == d.p + n);
      for(std::size_t i = 0; i != n; ++i)
         BOOST_TEST(value(d.p[i]) == 7);
      BOOST_TEST(counts_are<T>(n, 0u, 0u, 0u, 0u));
   }
}

//MoveSrc: the source is a move_iterator, so the elements are moved
template<class T, class Mode, bool MoveSrc>
void test_copy()
{
   typedef typename Mode::template iter<T>::type iter_t;
   typedef typename dtl::if_c<MoveSrc, boost::move_iterator<iter_t>, iter_t>::type src_t;
   const bool mm = MoveSrc ? memmove_move<T, Mode>::value : memmove_copy<T, Mode>::value;
   const std::size_t copies = (MoveSrc || mm) ? 0u : n;
   const std::size_t moves  = (MoveSrc && !mm) ? n : 0u;
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      d.construct_seq(n, 100);
      reset();
      const iter_t r = boost::container::copy
         (src_t(Mode::make(s.p)), src_t(Mode::make(s.p + n)), Mode::make(d.p));
      BOOST_TEST(Mode::base(r) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(MoveSrc ? s.moved_from_or_kept(0u, n, 1, mm) : s.seq_is(0u, n, 1));
      BOOST_TEST(counts_are<T>(0u, 0u, copies, moves, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      d.construct_seq(n, 100);
      reset();
      const iter_t r = boost::container::copy_n(src_t(Mode::make(s.p)), n, Mode::make(d.p));
      BOOST_TEST(Mode::base(r) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(MoveSrc ? s.moved_from_or_kept(0u, n, 1, mm) : s.seq_is(0u, n, 1));
      BOOST_TEST(counts_are<T>(0u, 0u, copies, moves, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      d.construct_seq(n, 100);
      reset();
      const src_t r = boost::container::copy_n_source(src_t(Mode::make(s.p)), n, Mode::make(d.p));
      BOOST_TEST(r == src_t(Mode::make(s.p + n)));
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(MoveSrc ? s.moved_from_or_kept(0u, n, 1, mm) : s.seq_is(0u, n, 1));
      BOOST_TEST(counts_are<T>(0u, 0u, copies, moves, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      d.construct_seq(n, 100);
      iter_t dst = Mode::make(d.p);
      reset();
      const src_t r = boost::container::copy_n_source_dest(src_t(Mode::make(s.p)), n, dst);
      BOOST_TEST(r == src_t(Mode::make(s.p + n)));
      BOOST_TEST(Mode::base(dst) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(MoveSrc ? s.moved_from_or_kept(0u, n, 1, mm) : s.seq_is(0u, n, 1));
      BOOST_TEST(counts_are<T>(0u, 0u, copies, moves, 0u));
   }
}

template<class T, class Mode>
void test_move()
{
   typedef typename Mode::template iter<T>::type iter_t;
   const bool mm = memmove_move<T, Mode>::value;
   const std::size_t moves = mm ? 0u : n;
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      d.construct_seq(n, 100);
      reset();
      const iter_t r = boost::container::move(Mode::make(s.p), Mode::make(s.p + n), Mode::make(d.p));
      BOOST_TEST(Mode::base(r) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(s.moved_from_or_kept(0u, n, 1, mm));
      BOOST_TEST(counts_are<T>(0u, 0u, 0u, moves, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      d.construct_seq(n, 100);
      reset();
      const iter_t r = boost::container::move_n(Mode::make(s.p), n, Mode::make(d.p));
      BOOST_TEST(Mode::base(r) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(s.moved_from_or_kept(0u, n, 1, mm));
      BOOST_TEST(counts_are<T>(0u, 0u, 0u, moves, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      d.construct_seq(n, 100);
      reset();
      const iter_t r = boost::container::move_n_source(Mode::make(s.p), n, Mode::make(d.p));
      BOOST_TEST(Mode::base(r) == s.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(s.moved_from_or_kept(0u, n, 1, mm));
      BOOST_TEST(counts_are<T>(0u, 0u, 0u, moves, 0u));
   }
   {
      buffer<T> s(n), d(n);
      s.construct_seq(n, 1);
      d.construct_seq(n, 100);
      iter_t dst = Mode::make(d.p);
      reset();
      const iter_t r = boost::container::move_n_source_dest(Mode::make(s.p), n, dst);
      BOOST_TEST(Mode::base(r) == s.p + n);
      BOOST_TEST(Mode::base(dst) == d.p + n);
      BOOST_TEST(d.seq_is(0u, n, 1));
      BOOST_TEST(s.moved_from_or_kept(0u, n, 1, mm));
      BOOST_TEST(counts_are<T>(0u, 0u, 0u, moves, 0u));
   }
}

//The ranges overlap: [0, n) moves to [2, n + 2)
template<class T, class Mode>
void test_move_backward()
{
   typedef typename Mode::template iter<T>::type iter_t;
   const std::size_t sz = n + 2u;
   const bool mm = memmove_move<T, Mode>::value;
   const std::size_t moves = mm ? 0u : n;
   {
      buffer<T> b(sz);
      b.construct_seq(sz, 1);
      reset();
      const iter_t r = boost::container::move_backward
         (Mode::make(b.p), Mode::make(b.p + n), Mode::make(b.p + sz));
      BOOST_TEST(Mode::base(r) == b.p + 2);
      BOOST_TEST(b.seq_is(2u, n, 1));
      BOOST_TEST(b.moved_from_or_kept(0u, 2u, 1, mm));
      BOOST_TEST(counts_are<T>(0u, 0u, 0u, moves, 0u));
   }
   {
      buffer<T> b(sz);
      b.construct_seq(sz, 1);
      reset();
      const iter_t r = boost::container::move_backward_n(Mode::make(b.p + n), n, Mode::make(b.p + sz));
      BOOST_TEST(Mode::base(r) == b.p + 2);
      BOOST_TEST(b.seq_is(2u, n, 1));
      BOOST_TEST(b.moved_from_or_kept(0u, 2u, 1, mm));
      BOOST_TEST(counts_are<T>(0u, 0u, 0u, moves, 0u));
   }
   {
      buffer<T> b(sz);
      b.construct_seq(sz, 1);
      reset();
      const iter_t r = boost::container::move_backward_n_source(Mode::make(b.p + n), n, Mode::make(b.p + sz));
      BOOST_TEST(Mode::base(r) == b.p);
      BOOST_TEST(b.seq_is(2u, n, 1));
      BOOST_TEST(b.moved_from_or_kept(0u, 2u, 1, mm));
      BOOST_TEST(counts_are<T>(0u, 0u, 0u, moves, 0u));
   }
}

template<class T, class Mode>
void test_destroy()
{
   const std::size_t dtors = expect<T>::trivial_dtor ? 0u : n;
   {
      buffer<T> b(n);
      b.construct_seq(n, 1);
      reset();
      boost::container::destroy_alloc_n(b.a, Mode::make(b.p), n);
      b.n = 0u;
      BOOST_TEST(counts_are<T>(0u, 0u, 0u, 0u, dtors));
   }
   {
      buffer<T> b(n);
      b.construct_seq(n, 1);
      reset();
      boost::container::destroy_alloc(b.a, Mode::make(b.p), Mode::make(b.p + n));
      b.n = 0u;
      BOOST_TEST(counts_are<T>(0u, 0u, 0u, 0u, dtors));
   }
}

//deep_swap_alloc_n: the short range (n_i elements) gets the n_j elements of
//the large range, and the large range keeps n_i elements
template<class T, class Mode>
void test_deep_swap()
{
   const std::size_t n_i = 2u, n_j = n;
   buffer<T> s(n_j), l(n_j);
   s.construct_seq(n_i, 1);
   l.construct_seq(n_j, 101);
   reset();
   boost::container::deep_swap_alloc_n<256u>(s.a, Mode::make(s.p), n_i, Mode::make(l.p), n_j);
   s.n = n_j;
   l.n = n_i;
   BOOST_TEST(s.seq_is(0u, n_j, 101));
   BOOST_TEST(l.seq_is(0u, n_i, 1));
   //memmove: no operation. Element loop: each swap is a move construction, two
   //move assignments and the destruction of the temporary, then n_j - n_i move
   //constructions and destructions
   const bool mm = memmove_move<T, Mode>::value;
   const std::size_t dtors = mm ? 0u : (expect<T>::trivial_dtor ? n_i : n_j);
   BOOST_TEST(mm ? counts_are<T>(0u, 0u, 0u, 0u, 0u)
                 : counts_are<T>(0u, n_j, 0u, 2u*n_i, dtors));
}

//MoveRange: move_assign_range_alloc_n, else copy_assign_range_alloc_n
template<class T, class Mode, bool MoveRange>
void test_assign_range()
{
   //The input is longer than the output and shorter than the output
   const std::size_t n_o[] = { 2u, n + 2u };
   for(std::size_t t = 0; t != 2u; ++t){
      buffer<T> s(n), d(n + 2u);
      s.construct_seq(n, 1);
      d.construct_seq(n_o[t], 100);
      reset();
      if(MoveRange)
         boost::container::move_assign_range_alloc_n(d.a, Mode::make(s.p), n, Mode::make(d.p), n_o[t]);
      else
         boost::container::copy_assign_range_alloc_n(d.a, Mode::make(s.p), n, Mode::make(d.p), n_o[t]);
      d.n = n;
      BOOST_TEST(d.seq_is(0u, n, 1));
      const bool mm = MoveRange ? memmove_move<T, Mode>::value : memmove_copy<T, Mode>::value;
      BOOST_TEST(MoveRange ? s.moved_from_or_kept(0u, n, 1, mm) : s.seq_is(0u, n, 1));
      const std::size_t min_n    = n_o[t] < n ? n_o[t] : n;
      const std::size_t assigned = mm ? 0u : min_n;
      const std::size_t built    = mm ? 0u : n - min_n;
      const std::size_t dtors    = expect<T>::trivial_dtor ? 0u : n_o[t] - min_n;
      BOOST_TEST(MoveRange ? counts_are<T>(0u, built, 0u, assigned, dtors)
                           : counts_are<T>(built, 0u, assigned, 0u, dtors));
   }
}

template<class T, class Mode>
void test_all()
{
   test_uninitialized_move<T, Mode>();
   test_uninitialized_copy<T, Mode, false>();
   test_uninitialized_copy<T, Mode, true>();
   test_uninitialized_init_and_fill<T, Mode>();
   test_copy<T, Mode, false>();
   test_copy<T, Mode, true>();
   test_move<T, Mode>();
   test_move_backward<T, Mode>();
   test_destroy<T, Mode>();
   test_deep_swap<T, Mode>();
   test_assign_range<T, Mode, false>();
   test_assign_range<T, Mode, true>();
}

template<class T>
void test_modes()
{
   test_all<T, raw_mode>();
   test_all<T, wrapped_mode>();
}

void test_copy_move_overloads()
{
   test_modes<int>();
   test_modes<counted>();
   test_modes<user_move>();
   test_modes<fake_trivial>();
}

}  //namespace test {
}  //namespace container {
}  //namespace boost {

using namespace boost::container;

int main()
{
   test::test_expand_backward_forward_and_insert_alloc<test::movable_and_copyable_int>();
   test::test_copy_move_overloads();
   return boost::report_errors();
}
