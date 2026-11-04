//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
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
#include "unqualified_swap_test.hpp"

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

   return 0;
}
