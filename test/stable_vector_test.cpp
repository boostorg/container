//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
// the tests trigger deprecation warnings when compiled with msvc in C++17 mode
#if defined(_MSVC_LANG) && _MSVC_LANG > 201402
// warning STL4009: std::allocator<void> is deprecated in C++17
# define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#endif

#define STABLE_VECTOR_ENABLE_INVARIANT_CHECKING
#include <memory>

#include <boost/container/stable_vector.hpp>
#include <boost/container/node_allocator.hpp>

#include "check_equal_containers.hpp"
#include "movable_int.hpp"
#include "expand_bwd_test_allocator.hpp"
#include "expand_bwd_test_template.hpp"
#include "dummy_test_allocator.hpp"
#include "propagate_allocator_test.hpp"
#include "allocator_test.hpp"
#include <boost/container/new_allocator.hpp>
#include "vector_test.hpp"
#include "default_init_test.hpp"
#include "../../intrusive/test/iterator_test.hpp"
#include "iterator_noexcept_utils.hpp"
#include "container_noexcept_utils.hpp"
#include "unqualified_swap_test.hpp"
#include "propagation_test_allocator.hpp"

using namespace boost::container;

class recursive_vector
{
   public:
   stable_vector<recursive_vector> vector_;
   stable_vector<recursive_vector>::iterator it_;
   stable_vector<recursive_vector>::const_iterator cit_;
   stable_vector<recursive_vector>::reverse_iterator rit_;
   stable_vector<recursive_vector>::const_reverse_iterator crit_;

   recursive_vector (const recursive_vector &o)
      : vector_(o.vector_)
   {}

   recursive_vector &operator=(const recursive_vector &o)
   { vector_ = o.vector_;  return *this; }
};

void recursive_vector_test()//Test for recursive types
{
   stable_vector<recursive_vector> recursive, copy;
   //Test to test both move emulations
   if(!copy.size()){
      copy = recursive;
   }
}

template<class VoidAllocator>
struct GetAllocatorCont
{
   template<class ValueType>
   struct apply
   {
      typedef stable_vector< ValueType
                           , typename allocator_traits<VoidAllocator>
                              ::template portable_rebind_alloc<ValueType>::type
                           > type;
   };
};

template<class VoidAllocator>
int test_cont_variants()
{
   typedef typename GetAllocatorCont<VoidAllocator>::template apply<int>::type MyCont;
   typedef typename GetAllocatorCont<VoidAllocator>::template apply<test::movable_int>::type MyMoveCont;
   typedef typename GetAllocatorCont<VoidAllocator>::template apply<test::movable_and_copyable_int>::type MyCopyMoveCont;
   typedef typename GetAllocatorCont<VoidAllocator>::template apply<test::copyable_int>::type MyCopyCont;
   typedef typename GetAllocatorCont<VoidAllocator>::template apply<test::moveconstruct_int>::type MyMoveConstructCont;

   if (test::vector_test<MyCont>())
      return 1;
   if (test::vector_test<MyMoveCont>())
      return 1;
   if (test::vector_test<MyCopyMoveCont>())
      return 1;
   if (test::vector_test<MyCopyCont>())
      return 1;
   if (test::vector_test<MyMoveConstructCont>())
      return 1;

   return 0;
}

struct boost_container_stable_vector;

namespace boost { namespace container {   namespace test {

template<>
struct alloc_propagate_base<boost_container_stable_vector>
{
   template <class T, class Allocator>
   struct apply
   {
      typedef boost::container::stable_vector<T, Allocator> type;
   };
};

}}}   //namespace boost::container::test


//Test the expected sizeof()
BOOST_CONTAINER_STATIC_ASSERT_MSG(5*sizeof(void*) == sizeof(stable_vector<int>), "sizeof has an unexpected value");

bool test_unqualified_swap()
{
   namespace us = boost_container_test_unqualified_swap;
   {  typedef boost::container::stable_vector<int> cont;
      cont a;  a.push_back(1);  a.push_back(2);
      cont b;  b.push_back(7);  b.push_back(8);  b.push_back(9);
      if(!us::test_unqualified_swap(a, b))
         return false;
   }
   return true;
}

//Predicate for erase_if: true for even values. It throws if it is called with throw_at
struct is_even_or_throw
{
   int throw_at;

   explicit is_even_or_throw(int t = -1)
      : throw_at(t)
   {}

   bool operator()(int v) const
   {
      #ifndef BOOST_NO_EXCEPTIONS
      if(v == throw_at)
         throw throw_at;
      #endif
      return (v & 1) == 0;
   }
};

//The free erase and erase_if do not move the kept elements: references
//and iterators to them keep their value
bool test_erase_if_keeps_references()
{
   typedef boost::container::stable_vector<int> cont;
   typedef cont::size_type size_type;
   const int n = 100;
   {
      cont c;
      for(int i = 0; i != n; ++i)
         c.push_back(i);
      const int *refs[n];
      cont::iterator its[n];
      for(int i = 0; i != n; ++i){
         refs[i] = &c[size_type(i)];
         its[i] = c.begin() + i;
      }
      if(boost::container::erase_if(c, is_even_or_throw()) != size_type(n/2) || c.size() != size_type(n/2))
         return false;
      for(int i = 1; i < n; i += 2){
         if(*refs[i] != i || *its[i] != i || &c[size_type(i/2)] != refs[i])
            return false;
      }
      //The nodes of the erased elements are reused
      for(int i = 0; i != n; ++i)
         c.push_back(i);
      if(c.size() != size_type(n/2 + n) || *refs[1] != 1 || c.back() != n - 1)
         return false;
   }
   {  //erase by value, including the first and the last elements
      cont c;
      for(int i = 0; i != 10; ++i)
         c.push_back(i % 3);
      const int *p = &c[1];
      if(boost::container::erase(c, 0) != 4u || c.size() != 6u || *p != 1 || &c[0] != p)
         return false;
      for(size_type i = 0; i != c.size(); ++i){
         if(c[i] != int(i % 2u) + 1)
            return false;
      }
      if(boost::container::erase(c, 5) != 0u || c.size() != 6u)
         return false;
      if(boost::container::erase(c, 1) != 3u || boost::container::erase(c, 2) != 3u || !c.empty())
         return false;
      if(boost::container::erase(c, 1) != 0u)
         return false;
      c.push_back(7);
      if(c.size() != 1u || c[0] != 7)
         return false;
   }
   #ifndef BOOST_NO_EXCEPTIONS
   {  //If the predicate throws, the elements that are not tested yet are kept
      cont c;
      for(int i = 0; i != n; ++i)
         c.push_back(i);
      const int *p = &c[51];
      bool thrown = false;
      try{
         boost::container::erase_if(c, is_even_or_throw(50));
      }
      catch(int){
         thrown = true;
      }
      //0, 2, ..., 48 are erased
      if(!thrown || c.size() != size_type(n - 25))
         return false;
      for(size_type i = 0; i != c.size(); ++i){
         if(c[i] != (i < 25u ? int(2u*i + 1u) : int(i + 25u)))
            return false;
      }
      if(*p != 51 || &c[26] != p)
         return false;
   }
   #endif   //BOOST_NO_EXCEPTIONS
   return true;
}

//The members remove and remove_if do not move the kept elements and
//return the number of removed elements
bool test_remove_members()
{
   typedef boost::container::stable_vector<int> cont;
   typedef cont::size_type size_type;
   const int n = 100;
   {
      cont c;
      for(int i = 0; i != n; ++i)
         c.push_back(i);
      const int *refs[n];
      for(int i = 0; i != n; ++i)
         refs[i] = &c[size_type(i)];
      if(c.remove_if(is_even_or_throw()) != size_type(n/2) || c.size() != size_type(n/2))
         return false;
      for(int i = 1; i < n; i += 2){
         if(*refs[i] != i || &c[size_type(i/2)] != refs[i])
            return false;
      }
      if(c.remove_if(is_even_or_throw()) != 0u || c.size() != size_type(n/2))
         return false;
      if(c.remove(1) != 1u || c.remove(n - 1) != 1u || c.remove(n) != 0u)
         return false;
      if(c.size() != size_type(n/2 - 2) || c.front() != 3 || c.back() != n - 3 || &c[0] != refs[3])
         return false;
   }
   {  //Empty container and all the elements removed
      cont c;
      if(c.remove(0) != 0u || c.remove_if(is_even_or_throw()) != 0u || !c.empty())
         return false;
      c.push_back(2);
      c.push_back(2);
      if(c.remove(2) != 2u || !c.empty())
         return false;
      c.push_back(3);
      if(c.size() != 1u || c[0] != 3)
         return false;
   }
   #ifndef BOOST_NO_EXCEPTIONS
   {  //If the predicate throws, the elements that are not tested yet are kept
      cont c;
      for(int i = 0; i != n; ++i)
         c.push_back(i);
      bool thrown = false;
      try{
         c.remove_if(is_even_or_throw(50));
      }
      catch(int){
         thrown = true;
      }
      if(!thrown || c.size() != size_type(n - 25) || c[25] != 50)
         return false;
   }
   #endif   //BOOST_NO_EXCEPTIONS
   return true;
}

inline int splice_value(int v)
{  return v;  }

inline int splice_value(const test::non_copymovable_int &v)
{  return v.get_int();  }

//Checks that c holds the values of v, in order, through iterators and operator[]
template<class Cont>
bool splice_check(const Cont &c, const int *v, std::size_t n)
{
   if(c.size() != n)
      return false;
   std::size_t i = 0;
   for(typename Cont::const_iterator it = c.begin(); it != c.end(); ++it, ++i){
      if(splice_value(*it) != v[i] || splice_value(c[i]) != v[i])
         return false;
   }
   return true;
}

//Checks that c holds the values first, first + 1, ... in order
template<class Cont>
bool splice_check_sequence(const Cont &c, int first, std::size_t n)
{
   if(c.size() != n)
      return false;
   for(std::size_t i = 0; i != n; ++i){
      if(splice_value(c[i]) != first + int(i))
         return false;
   }
   return true;
}

//Elements are transferred between two containers without copies, moves or destructions
bool test_splice_other()
{
   typedef stable_vector<test::non_copymovable_int> cont;
   {
      cont a, b, c;
      for(int i = 0; i < 5; ++i)
         a.emplace_back(i);         //0 1 2 3 4
      for(int i = 10; i < 13; ++i)
         b.emplace_back(i);         //10 11 12
      const unsigned int count = test::non_copymovable_int::count;
      const test::non_copymovable_int *const p11 = &b[1], *const p2 = &a[2], *const p4 = &a[4];
      const cont::const_iterator b_end = b.cend();

      //All of b, in the middle of a
      a.splice(a.cbegin() + 2, b);
      {  const int v[] = { 0, 1, 10, 11, 12, 2, 3, 4 };
         if(!splice_check(a, v, 8u) || !b.empty() || b.cend() != b_end)
            return false;
      }
      if(&a[3] != p11 || &a[5] != p2 || &a[7] != p4)
         return false;

      //One element, into an empty container
      c.splice(c.cend(), a, a.cbegin() + 3);
      {  const int v[] = { 11 }, w[] = { 0, 1, 10, 12, 2, 3, 4 };
         if(!splice_check(c, v, 1u) || !splice_check(a, w, 7u) || &c[0] != p11)
            return false;
      }

      //A range at the end of a, to the beginning of c
      c.splice(c.cbegin(), a, a.cbegin() + 4, a.cend());
      {  const int v[] = { 2, 3, 4, 11 }, w[] = { 0, 1, 10, 12 };
         if(!splice_check(c, v, 4u) || !splice_check(a, w, 4u) || &c[0] != p2 || &c[2] != p4)
            return false;
      }

      //An empty range has no effects
      a.splice(a.cend(), c, c.cbegin() + 1, c.cbegin() + 1);
      {  const int v[] = { 2, 3, 4, 11 }, w[] = { 0, 1, 10, 12 };
         if(!splice_check(c, v, 4u) || !splice_check(a, w, 4u))
            return false;
      }

      //The rvalue overloads
      c.splice(c.cend(), boost::move(a), a.cbegin());
      c.splice(c.cbegin(), boost::move(a), a.cbegin() + 1, a.cend());
      {  const int v[] = { 10, 12, 2, 3, 4, 11, 0 }, w[] = { 1 };
         if(!splice_check(c, v, 7u) || !splice_check(a, w, 1u))
            return false;
      }
      a.splice(a.cbegin(), boost::move(c));
      {  const int v[] = { 10, 12, 2, 3, 4, 11, 0, 1 };
         if(!splice_check(a, v, 8u) || !c.empty())
            return false;
      }

      //No element was constructed or destroyed
      if(test::non_copymovable_int::count != count)
         return false;

      //Both containers work as usual after the splice
      a.erase(a.cbegin() + 2);
      a.emplace(a.cbegin(), 7);
      c.emplace_back(20);
      c.splice(c.cbegin(), a);
      {  const int v[] = { 7, 10, 12, 3, 4, 11, 0, 1, 20 };
         if(!splice_check(c, v, 9u) || !a.empty())
            return false;
      }
   }
   {  //Many elements: the index of the receiving container grows, and back
      cont a, b;
      a.emplace_back(-1);
      a.emplace_back(-2);
      for(int i = 0; i < 1000; ++i)
         b.emplace_back(i);
      const test::non_copymovable_int *const p0 = &b[0], *const p999 = &b[999];
      a.splice(a.cbegin() + 1, b);
      if(a.size() != 1002u || !b.empty() || &a[1] != p0 || &a[1000] != p999)
         return false;
      b.splice(b.cend(), a, a.cbegin() + 1, a.cbegin() + 1001);
      if(!splice_check_sequence(b, 0, 1000u) || a.size() != 2u || &b[0] != p0 || &b[999] != p999)
         return false;
      //One element each time
      while(!b.empty())
         a.splice(a.cend() - 1, b, b.cbegin());
      if(a.size() != 1002u || a[0].get_int() != -1 || a[1001].get_int() != -2 || &a[1] != p0 || &a[1000] != p999)
         return false;
   }
   return true;
}

//The receiving container has free nodes in its pool. The index must keep room for them
bool test_splice_other_with_pool()
{
   typedef stable_vector<int> cont;
   cont r, s;
   //Free nodes after reserve
   r.reserve(100u);
   const cont::size_type cap = r.capacity();
   for(int i = 0; i < 50; ++i)
      s.push_back(i);
   r.splice(r.cend(), s);
   if(!splice_check_sequence(r, 0, 50u) || !s.empty() || r.capacity() < cap)
      return false;
   //The free nodes of r that do not fit in its index were moved to the pool of s
   if(s.capacity() < 50u)
      return false;
   for(int i = 0; i < 50; ++i)
      s.push_back(i);
   if(!splice_check_sequence(s, 0, 50u))
      return false;
   s.clear();
   //The new elements use the free nodes
   for(int i = 50; i < 100; ++i)
      r.push_back(i);
   if(!splice_check_sequence(r, 0, 100u))
      return false;
   //Free nodes after erase
   r.erase(r.cbegin(), r.cbegin() + 60);
   for(int i = 0; i < 30; ++i)
      s.push_back(i);
   r.splice(r.cbegin(), s, s.cbegin() + 10, s.cend());
   if(!splice_check_sequence(s, 0, 10u) || r.size() != 60u || r[0] != 10 || r[19] != 29 || r[20] != 60 || r[59] != 99)
      return false;
   r.splice(r.cbegin() + 20, s);
   if(r.size() != 70u || !s.empty() || r[19] != 29 || r[20] != 0 || r[29] != 9 || r[30] != 60)
      return false;
   for(int i = 0; i < 100; ++i)
      r.push_back(i);
   r.shrink_to_fit();
   return r.size() == 170u && r[0] == 10 && r[20] == 0 && r[30] == 60 && r[69] == 99 && r[169] == 99;
}

//Splice inside one container only changes the order of the elements
bool test_splice_self()
{
   typedef stable_vector<int> cont;
   cont c;
   for(int i = 0; i < 10; ++i)
      c.push_back(i);
   const int *addr[10];
   for(int i = 0; i < 10; ++i)
      addr[i] = &c[cont::size_type(i)];
   const cont::const_iterator e = c.cend();

   //Range after p
   c.splice(c.cbegin() + 1, c, c.cbegin() + 5, c.cbegin() + 8);
   {  const int v[] = { 0, 5, 6, 7, 1, 2, 3, 4, 8, 9 };
      if(!splice_check(c, v, 10u))
         return false;
   }
   //Range before p
   c.splice(c.cend(), c, c.cbegin(), c.cbegin() + 2);
   {  const int v[] = { 6, 7, 1, 2, 3, 4, 8, 9, 0, 5 };
      if(!splice_check(c, v, 10u))
         return false;
   }
   //p == first and p == last have no effects, as an empty range
   c.splice(c.cbegin() + 3, c, c.cbegin() + 3, c.cbegin() + 5);
   c.splice(c.cbegin() + 5, c, c.cbegin() + 3, c.cbegin() + 5);
   c.splice(c.cbegin(), c, c.cbegin() + 4, c.cbegin() + 4);
   {  const int v[] = { 6, 7, 1, 2, 3, 4, 8, 9, 0, 5 };
      if(!splice_check(c, v, 10u))
         return false;
   }
   //One element
   c.splice(c.cbegin(), c, c.cend() - 1);
   c.splice(c.cbegin() + 2, c, c.cbegin() + 1);   //p == i + 1: no effects
   c.splice(c.cbegin() + 2, c, c.cbegin() + 2);   //p == i: no effects
   c.splice(c.cend(), c, c.cbegin() + 2);
   {  const int v[] = { 5, 6, 1, 2, 3, 4, 8, 9, 0, 7 };
      if(!splice_check(c, v, 10u))
         return false;
   }
   //The rvalue overloads
   c.splice(c.cbegin(), boost::move(c), c.cend() - 2, c.cend());
   c.splice(c.cend(), boost::move(c), c.cbegin() + 2);
   {  const int v[] = { 0, 7, 6, 1, 2, 3, 4, 8, 9, 5 };
      if(!splice_check(c, v, 10u))
         return false;
   }
   //The elements were not moved and end() is still valid
   for(cont::size_type i = 0; i != c.size(); ++i){
      if(&c[i] != addr[c[i]])
         return false;
   }
   return c.cend() == e;
}

#ifndef BOOST_NO_EXCEPTIONS

struct splice_alloc_state
{
   static bool throw_on_allocate;
};

bool splice_alloc_state::throw_on_allocate = false;

//Allocator with a small max_size() that throws on request
template<class T>
class splice_test_allocator
{
   public:
   typedef T value_type;

   template<class U>
   struct rebind
   {  typedef splice_test_allocator<U> other;  };

   splice_test_allocator()
   {}

   template<class U>
   splice_test_allocator(const splice_test_allocator<U> &)
   {}

   T* allocate(std::size_t n)
   {
      if(splice_alloc_state::throw_on_allocate)
         throw std::bad_alloc();
      return static_cast<T*>(::operator new(n*sizeof(T)));
   }

   void deallocate(T *p, std::size_t)
   {  ::operator delete(p);  }

   std::size_t max_size() const
   {  return 300u;  }

   friend bool operator==(const splice_test_allocator &, const splice_test_allocator &)
   {  return true;  }

   friend bool operator!=(const splice_test_allocator &, const splice_test_allocator &)
   {  return false;  }
};

//If splice throws, neither container changes
bool test_splice_exceptions()
{
   typedef stable_vector<int, splice_test_allocator<int> > cont;
   {  //length_error: the result would be greater than max_size()
      cont a, b;
      for(int i = 0; i < 205; ++i)
         a.push_back(i);
      for(int i = 0; i < 150; ++i)
         b.push_back(1000 + i);
      bool thrown = false;
      try{
         a.splice(a.cbegin() + 10, b);
      }
      catch(length_error_t &){
         thrown = true;
      }
      if(!thrown || !splice_check_sequence(a, 0, 205u) || !splice_check_sequence(b, 1000, 150u))
         return false;
      thrown = false;
      try{
         a.splice(a.cbegin(), b, b.cbegin() + 50, b.cbegin() + 150);
      }
      catch(length_error_t &){
         thrown = true;
      }
      if(!thrown || !splice_check_sequence(a, 0, 205u) || !splice_check_sequence(b, 1000, 150u))
         return false;
      //The result is equal to max_size(): the free nodes of a do not fit in its index
      //and are moved to the pool of b
      a.erase(a.cend() - 5, a.cend());
      b.erase(b.cbegin(), b.cbegin() + 53);
      if(a.size() + b.size() != a.max_size())
         return false;
      a.splice(a.cend(), b);
      if(a.size() != a.max_size() || !b.empty() || a.capacity() != a.size() || b.capacity() < 58u)
         return false;
      if(a[199] != 199 || a[200] != 1053 || a.back() != 1149)
         return false;
   }
   {  //bad_alloc: the index must grow
      cont a, b;
      for(int i = 0; i < 10; ++i)
         a.push_back(i);
      for(int i = 0; i < 5; ++i)
         b.push_back(100 + i);
      a.shrink_to_fit();
      bool thrown = false;
      splice_alloc_state::throw_on_allocate = true;
      try{
         a.splice(a.cbegin() + 3, b);
      }
      catch(std::bad_alloc &){
         thrown = true;
      }
      splice_alloc_state::throw_on_allocate = false;
      if(!thrown || !splice_check_sequence(a, 0, 10u) || !splice_check_sequence(b, 100, 5u))
         return false;
      //Splice inside one container does not allocate
      splice_alloc_state::throw_on_allocate = true;
      a.splice(a.cbegin(), a, a.cend() - 2, a.cend());
      splice_alloc_state::throw_on_allocate = false;
      a.splice(a.cbegin() + 3, b);
      const int v[] = { 8, 9, 0, 100, 101, 102, 103, 104, 1, 2, 3, 4, 5, 6, 7 };
      if(!splice_check(a, v, 15u) || !b.empty())
         return false;
   }
   return true;
}

#endif   //BOOST_NO_EXCEPTIONS

//The operations of the iterators cannot throw
#if !defined(BOOST_NO_CXX11_NOEXCEPT)
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::random_access_container_iterators_are_nothrow
   < boost::container::stable_vector<int> >::value));

//The operations that do not allocate memory or move elements cannot throw
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::reversible_container_ops_are_nothrow
   < boost::container::stable_vector<int> >::value));
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::container_erase_is_nothrow
   < boost::container::stable_vector<int> >::value));

//swap cannot throw, also with an allocator that is not propagated on swap and is not always equal
typedef boost::container::stable_vector<int, propagation_test_allocator<int, 0> > nonpropagating_stable_vector_t;
BOOST_CONTAINER_STATIC_ASSERT(( noexcept(boost::move_detail::declval<nonpropagating_stable_vector_t&>().swap(boost::move_detail::declval<nonpropagating_stable_vector_t&>())) ));
BOOST_CONTAINER_STATIC_ASSERT(( noexcept(swap(boost::move_detail::declval<nonpropagating_stable_vector_t&>(), boost::move_detail::declval<nonpropagating_stable_vector_t&>())) ));
#endif

int main()
{
   if(!test_unqualified_swap())
      return 1;

   recursive_vector_test();
   {
      //Now test move semantics
      stable_vector<recursive_vector> original;
      stable_vector<recursive_vector> move_ctor(boost::move(original));
      stable_vector<recursive_vector> move_assign;
      move_assign = boost::move(move_ctor);
      move_assign.swap(original);
   }

   //Test non-copy-move operations
   {
      stable_vector<test::non_copymovable_int> sv;
      sv.emplace_back();
      sv.resize(10);
      sv.resize(1);
   }

   ////////////////////////////////////
   //    Void value_type allocator
   ////////////////////////////////////
   if(!test::test_void_allocator
         < stable_vector<int, new_allocator<void> >
         , new_allocator<int> >()) {
      std::cerr << "test_void_allocator stable_vector failed" << std::endl;
      return 1;
   }

   ////////////////////////////////////
   //    Testing allocator implementations
   ////////////////////////////////////
   //       std:allocator
   if(test_cont_variants< std::allocator<void> >()){
      std::cerr << "test_cont_variants< std::allocator<void> > failed" << std::endl;
      return 1;
   }
   //       boost::container::node_allocator
   if(test_cont_variants< node_allocator<void> >()){
      std::cerr << "test_cont_variants< node_allocator<void> > failed" << std::endl;
      return 1;
   }

   ////////////////////////////////////
   //    Default init test
   ////////////////////////////////////
   if(!test::default_init_test< stable_vector<int, test::default_init_allocator<int> > >()){
      std::cerr << "Default init test failed" << std::endl;
      return 1;
   }

   ////////////////////////////////////
   //    Emplace testing
   ////////////////////////////////////
   const test::EmplaceOptions Options = (test::EmplaceOptions)(test::EMPLACE_BACK | test::EMPLACE_BEFORE);
   if(!boost::container::test::test_emplace
      < stable_vector<test::EmplaceInt>, Options>())
      return 1;

   ////////////////////////////////////
   //    Allocator propagation testing
   ////////////////////////////////////
   if(!boost::container::test::test_propagate_allocator<boost_container_stable_vector>())
      return 1;

   ////////////////////////////////////
   //    Initializer lists testing
   ////////////////////////////////////
   if(!boost::container::test::test_vector_methods_with_initializer_list_as_argument_for
      < boost::container::stable_vector<int> >())
   {
       std::cerr << "test_methods_with_initializer_list_as_argument failed" << std::endl;
       return 1;
   }

   ////////////////////////////////////
   //    Iterator testing
   ////////////////////////////////////
   {
      typedef boost::container::stable_vector<int> cont_int;
      for (std::size_t i = 10; i <= 10000; i *= 10) {
         cont_int a;
         for (int j = 0; j < (int)i; ++j)
            a.push_back((int)j);
         boost::intrusive::test::test_iterator_random< cont_int >(a);
         if (boost::report_errors() != 0) {
            return 1;
         }
      }
   }

#ifndef BOOST_CONTAINER_NO_CXX17_CTAD
   ////////////////////////////////////
   //    Constructor Template Auto Deduction testing
   ////////////////////////////////////
   {
      auto gold = std::vector{ 1, 2, 3 };
      auto test = boost::container::stable_vector(gold.begin(), gold.end());
      if (test.size() != 3) {
         return 1;
      }
      if (!(test[0] == 1 && test[1] == 2 && test[2] == 3)) {
         return 1;
      }
   }
   {
      boost::container::stable_vector test(5, 3);
      boost::container::stable_vector test2(5, 3, boost::container::new_allocator<int>());
      if (test.size() != 5u || test2.size() != 5u || test[4] != 3 || test2[4] != 3) {
         return 1;
      }
   }
#endif

   ////////////////////////////////////
   //    has_trivial_destructor_after_move testing
   ////////////////////////////////////
   // default allocator
   {
      typedef boost::container::stable_vector<int> cont;
      typedef cont::allocator_type allocator_type;
      typedef boost::container::allocator_traits<allocator_type>::pointer pointer;
      BOOST_CONTAINER_STATIC_ASSERT_MSG(
        !(boost::has_trivial_destructor_after_move<cont>::value !=
          boost::has_trivial_destructor_after_move<allocator_type>::value &&
          boost::has_trivial_destructor_after_move<pointer>::value)
        , "has_trivial_destructor_after_move(default allocator) test failed");
   }
   // std::allocator
   {
      typedef boost::container::stable_vector<int, std::allocator<int> > cont;
      typedef cont::allocator_type allocator_type;
      typedef boost::container::allocator_traits<allocator_type>::pointer pointer;
      BOOST_CONTAINER_STATIC_ASSERT_MSG(
        !(boost::has_trivial_destructor_after_move<cont>::value !=
          boost::has_trivial_destructor_after_move<allocator_type>::value &&
          boost::has_trivial_destructor_after_move<pointer>::value)
        , "has_trivial_destructor_after_move(std::allocator) test failed");
   }

   if(!test_erase_if_keeps_references())
      return 1;

   if(!test_remove_members())
      return 1;

   if(!test_splice_other())
      return 1;

   if(!test_splice_other_with_pool())
      return 1;

   if(!test_splice_self())
      return 1;

   #ifndef BOOST_NO_EXCEPTIONS
   if(!test_splice_exceptions())
      return 1;
   #endif

   return 0;
}
