//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_SEGMENTED_TEST_HPP
#define BOOST_CONTAINER_TEST_SEGMENTED_TEST_HPP

//Tests that deque and segtor share. Segmented is the class template
//(boost::container::deque or boost::container::segtor): both take the same options.

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/allocator_traits.hpp>
#include <boost/container/options.hpp>
#include <boost/container/detail/mpl.hpp>
#include <boost/move/utility_core.hpp>
#include "check_equal_containers.hpp"
#include "movable_int.hpp"
#include "vector_test.hpp"
#include "lightweight_test.hpp"
#include <cstddef>
#include <new>

namespace boost {
namespace container {
namespace test {

//Insertions and assignments of n copies: only for copyable types
template<class V1, class V2>
bool segmented_copyable_only(V1 &, V2 &, dtl::false_type)
{
   return true;
}

template<class V1, class V2>
bool segmented_copyable_only(V1 &cntc, V2 &stdc, dtl::true_type)
{
   typedef typename V1::value_type IntType;
   std::size_t size = cntc.size();
   stdc.insert(stdc.end(), 50u, 1);
   cntc.insert(cntc.end(), 50u, IntType(1));
   if(!CheckEqualContainers(cntc, stdc)) return false;
   {
      IntType move_me(1);
      stdc.insert(stdc.begin()+std::ptrdiff_t(size)/2, 50u, 1);
      cntc.insert(cntc.begin()+std::ptrdiff_t(size/2), 50u, boost::move(move_me));
      if(!CheckEqualContainers(cntc, stdc)) return false;
   }
   {
      IntType move_me(2);
      cntc.assign(cntc.size()/2, boost::move(move_me));
      stdc.assign(stdc.size()/2, 2);
      if(!CheckEqualContainers(cntc, stdc)) return false;
   }
   {
      IntType move_me(1);
      stdc.clear();
      cntc.clear();
      stdc.insert(stdc.begin(), 50u, 1);
      cntc.insert(cntc.begin(), 50u, boost::move(move_me));
      if(!CheckEqualContainers(cntc, stdc)) return false;
      stdc.insert(stdc.begin()+20, 50u, 1);
      cntc.insert(cntc.begin()+20, 50u, boost::move(move_me));
      if(!CheckEqualContainers(cntc, stdc)) return false;
      stdc.insert(stdc.begin()+20, 20u, 1);
      cntc.insert(cntc.begin()+20, 20u, boost::move(move_me));
      if(!CheckEqualContainers(cntc, stdc)) return false;
   }
   {
      IntType move_me(1);
      stdc.clear();
      cntc.clear();
      stdc.insert(stdc.end(), 50u, 1);
      cntc.insert(cntc.end(), 50u, boost::move(move_me));
      if(!CheckEqualContainers(cntc, stdc)) return false;
      stdc.insert(stdc.end()-20, 50u, 1);
      cntc.insert(cntc.end()-20, 50u, boost::move(move_me));
      if(!CheckEqualContainers(cntc, stdc)) return false;
      stdc.insert(stdc.end()-20, 20u, 1);
      cntc.insert(cntc.end()-20, 20u, boost::move(move_me));
      if(!CheckEqualContainers(cntc, stdc)) return false;
   }

   return true;
}

template<template<class, class, class> class Segmented, class VoidAllocator, bool Reservable>
struct get_segmented_allocator_cont
{
   template<class ValueType>
   struct apply
   {
      typedef Segmented< ValueType
                       , typename allocator_traits<VoidAllocator>
                           ::template portable_rebind_alloc<ValueType>::type
                       , typename deque_options<reservable<Reservable> >::type
                       > type;
   };
};

//Returns 0 if the vector tests pass for several element types
template<template<class, class, class> class Segmented, class VoidAllocator, bool Reservable>
int test_segmented_cont_variants()
{
   typedef get_segmented_allocator_cont<Segmented, VoidAllocator, Reservable> get_t;
   typedef typename get_t::template apply<int>::type MyCont;
   typedef typename get_t::template apply<movable_int>::type MyMoveCont;
   typedef typename get_t::template apply<movable_and_copyable_int>::type MyCopyMoveCont;
   typedef typename get_t::template apply<copyable_int>::type MyCopyCont;
   typedef typename get_t::template apply<moveconstruct_int>::type MyMoveConstructCont;

   if (vector_test<MyCont>())
      return 1;
   if (vector_test<MyMoveCont>())
      return 1;
   if (vector_test<MyCopyMoveCont>())
      return 1;
   if (vector_test<MyCopyCont>())
      return 1;
   if (vector_test<MyMoveConstructCont>())
      return 1;
   return 0;
}

template<std::size_t N>
struct char_holder
{
   char chars[N];
};

//The default block size is a power of two, with at least 8 elements
template<template<class, class, class> class Segmented>
bool test_segmented_default_block_size()
{
   BOOST_TEST((Segmented<char_holder<8>,   void, void>::get_block_size() == 16*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<12>,  void, void>::get_block_size() == 16*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<16>,  void, void>::get_block_size() == 8*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<20>,  void, void>::get_block_size() == 8*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<24>,  void, void>::get_block_size() == 8*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<28>,  void, void>::get_block_size() == 8*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<32>,  void, void>::get_block_size() == 4*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<36>,  void, void>::get_block_size() == 4*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<40>,  void, void>::get_block_size() == 4*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<44>,  void, void>::get_block_size() == 4*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<48>,  void, void>::get_block_size() == 4*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<52>,  void, void>::get_block_size() == 4*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<56>,  void, void>::get_block_size() == 4*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<60>,  void, void>::get_block_size() == 4*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<64>,  void, void>::get_block_size() == 2*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<68>,  void, void>::get_block_size() == 2*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<72>,  void, void>::get_block_size() == 2*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<76>,  void, void>::get_block_size() == 2*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<80>,  void, void>::get_block_size() == 2*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<84>,  void, void>::get_block_size() == 2*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<88>,  void, void>::get_block_size() == 2*sizeof(void*)));
   BOOST_TEST((Segmented<char_holder<92>,  void, void>::get_block_size() == 2*sizeof(void*)));
   //Minimal 8 elements
   BOOST_TEST((Segmented<char_holder<148>, void, void>::get_block_size() == 8u));
   return 0 == boost::report_errors();
}

//shrink_to_fit reduces the memory, the elements are not moved and the
//container still works after the operation


//Counts the bytes that all the instances have allocated and not deallocated
template<class Dummy = void>
struct shrink_live_bytes_holder
{
   static std::size_t live_bytes;
   static bool fail_allocations;
};

template<class Dummy>
std::size_t shrink_live_bytes_holder<Dummy>::live_bytes = 0u;

template<class Dummy>
bool shrink_live_bytes_holder<Dummy>::fail_allocations = false;

template<class T>
class shrink_counting_allocator
{
   public:
   typedef T value_type;

   shrink_counting_allocator() {}

   template<class U>
   shrink_counting_allocator(const shrink_counting_allocator<U> &) {}

   T* allocate(std::size_t n)
   {
      #ifndef BOOST_NO_EXCEPTIONS
      if(shrink_live_bytes_holder<>::fail_allocations)
         throw std::bad_alloc();
      #endif
      shrink_live_bytes_holder<>::live_bytes += n*sizeof(T);
      return static_cast<T*>(::operator new(n*sizeof(T)));
   }

   void deallocate(T *p, std::size_t n)
   {
      shrink_live_bytes_holder<>::live_bytes -= n*sizeof(T);
      ::operator delete(p);
   }

   friend bool operator==(const shrink_counting_allocator &, const shrink_counting_allocator &)
   {  return true;  }

   friend bool operator!=(const shrink_counting_allocator &, const shrink_counting_allocator &)
   {  return false;  }
};

template<class C>
bool check_shrink_values(C &c, int first, std::size_t n)
{
   if(c.size() != n)
      return false;
   for(std::size_t i = 0; i != n; ++i){
      if(c[i] != int(first + int(i)))
         return false;
   }
   return true;
}

//Container is a deque or a segtor of int with shrink_counting_allocator and
//blocks of BlockSize elements. DoubleEnded tells if push_front and pop_front
//are available.
template<class Container, bool DoubleEnded, std::size_t BlockSize>
struct segmented_shrink_to_fit_tester
{
   static void push_front_n(Container &c, int first, std::size_t n, dtl::true_)
   {  for(std::size_t i = 0; i != n; ++i) c.push_front(int(first - 1 - int(i)));  }

   static void push_front_n(Container &, int, std::size_t, dtl::false_)
   {}

   static void pop_front_n(Container &c, std::size_t n, dtl::true_)
   {  for(std::size_t i = 0; i != n; ++i) c.pop_front();  }

   static void pop_front_n(Container &, std::size_t, dtl::false_)
   {}

   static bool run()
   {
      typedef dtl::bool_<DoubleEnded> double_ended_t;
      std::size_t &live = shrink_live_bytes_holder<>::live_bytes;
      const std::size_t big = 1000u;
      const std::size_t kept = 20u;
      {
         Container c;
         //Grow the container, then keep only a few elements: 0 ... kept-1
         for(std::size_t i = 0; i != big; ++i)
            c.push_back(int(i));
         push_front_n(c, 0, big, double_ended_t());
         pop_front_n(c, big, double_ended_t());
         while(c.size() > kept)
            c.pop_back();
         if(!check_shrink_values(c, 0, kept))
            return false;

         //The elements are not moved
         const int *addr[kept];
         for(std::size_t i = 0; i != kept; ++i)
            addr[i] = &c[i];

         #ifndef BOOST_NO_EXCEPTIONS
         //If the new index can not be allocated, there are no effects
         shrink_live_bytes_holder<>::fail_allocations = true;
         const std::size_t before_fail = live;
         c.shrink_to_fit();
         shrink_live_bytes_holder<>::fail_allocations = false;
         if(live != before_fail || !check_shrink_values(c, 0, kept))
            return false;
         #endif

         const std::size_t before = live;
         c.shrink_to_fit();
         const std::size_t after = live;
         if(after >= before)
            return false;
         //Only the blocks with elements stay: at most kept/BlockSize + 2 blocks
         //and their slots in the index
         const std::size_t max_nodes = kept/BlockSize + 2u;
         if(after > max_nodes*(BlockSize*sizeof(int) + sizeof(int*)))
            return false;
         if(!check_shrink_values(c, 0, kept))
            return false;
         for(std::size_t i = 0; i != kept; ++i){
            if(addr[i] != &c[i])
               return false;
         }

         //A second call has no effect
         c.shrink_to_fit();
         if(live != after || !check_shrink_values(c, 0, kept))
            return false;

         //The container still grows and shrinks at both ends
         for(std::size_t i = kept; i != big; ++i)
            c.push_back(int(i));
         push_front_n(c, 0, big, double_ended_t());
         const int first = DoubleEnded ? -int(big) : 0;
         const std::size_t n = DoubleEnded ? 2u*big : big;
         if(!check_shrink_values(c, first, n))
            return false;
         pop_front_n(c, big, double_ended_t());
         c.shrink_to_fit();
         if(!check_shrink_values(c, 0, big))
            return false;

         //An empty container deallocates all the memory
         c.clear();
         c.shrink_to_fit();
         if(live != 0u || !c.empty())
            return false;
         c.push_back(1);
         if(c.size() != 1u || c[0] != 1)
            return false;
      }
      return live == 0u;
   }
};


//shrink_to_fit of a non-reservable and of a reservable container.
//DoubleEnded tells if push_front and pop_front are available.
template<template<class, class, class> class Segmented, bool DoubleEnded>
bool test_segmented_shrink_to_fit()
{
   typedef Segmented< int, shrink_counting_allocator<int>
                    , typename deque_options< reservable<false>, block_size<8u> >::type> nonres_t;
   typedef Segmented< int, shrink_counting_allocator<int>
                    , typename deque_options< reservable<true>, block_size<8u> >::type> res_t;
   return segmented_shrink_to_fit_tester<nonres_t, DoubleEnded, 8u>::run()
       && segmented_shrink_to_fit_tester<res_t, DoubleEnded, 8u>::run();
}

}  //namespace test {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_TEST_SEGMENTED_TEST_HPP
