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
#include "void_allocator_test.hpp"
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

//Checks that sv holds the values of v, in order, through iterators and operator[]
bool test_splice_check(const stable_vector<test::non_copymovable_int> &sv,
                       const int *v, std::size_t n)
{
   if(sv.size() != n)
      return false;
   std::size_t i = 0;
   for( stable_vector<test::non_copymovable_int>::const_iterator it = sv.begin()
      ; it != sv.end(); ++it, ++i){
      if(it->get_int() != v[i] || sv[i].get_int() != v[i])
         return false;
   }
   return true;
}

bool test_splice()
{
   typedef stable_vector<test::non_copymovable_int> cont;
   cont a, b, c;
   for(int i = 0; i < 5; ++i)
      a.emplace_back(i);         //0 1 2 3 4
   for(int i = 10; i < 13; ++i)
      b.emplace_back(i);         //10 11 12
   const unsigned int count = test::non_copymovable_int::count;
   const test::non_copymovable_int *p11 = &b[1], *p2 = &a[2], *p4 = &a[4];

   //All of b, in the middle of a
   a.splice(a.cbegin() + 2, b);
   {  const int v[] = { 0, 1, 10, 11, 12, 2, 3, 4 };
      if(!test_splice_check(a, v, 8) || !b.empty())
         return false;
   }
   if(&a[3] != p11 || &a[5] != p2 || &a[7] != p4)
      return false;

   //One element, into an empty stable_vector
   c.splice(c.cend(), a, a.cbegin() + 3);
   {  const int v[] = { 11 }, w[] = { 0, 1, 10, 12, 2, 3, 4 };
      if(!test_splice_check(c, v, 1) || !test_splice_check(a, w, 7) || &c[0] != p11)
         return false;
   }

   //A range at the end of a, at the beginning of c
   c.splice(c.cbegin(), a, a.cbegin() + 4, a.cend());
   {  const int v[] = { 2, 3, 4, 11 }, w[] = { 0, 1, 10, 12 };
      if(!test_splice_check(c, v, 4) || !test_splice_check(a, w, 4))
         return false;
   }
   if(&c[0] != p2 || &c[2] != p4)
      return false;

   //An empty range changes nothing
   a.splice(a.cend(), c, c.cbegin(), c.cbegin());
   {  const int v[] = { 2, 3, 4, 11 }, w[] = { 0, 1, 10, 12 };
      if(!test_splice_check(c, v, 4) || !test_splice_check(a, w, 4))
         return false;
   }

   //Splicing constructs and destroys nothing
   if(test::non_copymovable_int::count != count)
      return false;

   //A growth of the index of the receiving stable_vector, and back
   for(int i = 0; i < 1000; ++i)
      b.emplace_back(100 + i);
   const test::non_copymovable_int *p100 = &b[0], *p1099 = &b[999];
   a.splice(a.cbegin() + 1, b);
   if(a.size() != 1004 || !b.empty() || &a[1] != p100 || &a[1000] != p1099)
      return false;
   b.splice(b.cend(), a, a.cbegin() + 1, a.cbegin() + 1001);
   {  const int w[] = { 0, 1, 10, 12 };
      if(!test_splice_check(a, w, 4) || b.size() != 1000 || &b[0] != p100 || &b[999] != p1099)
         return false;
   }

   //Both stable_vectors work as usual afterwards
   a.erase(a.cbegin() + 2);
   a.emplace(a.cbegin(), 7);
   c.splice(c.cend(), boost::move(a));
   {  const int v[] = { 2, 3, 4, 11, 7, 0, 1, 12 };
      if(!test_splice_check(c, v, 8) || !a.empty())
         return false;
   }
   return true;
}

int main()
{
   if(!test_splice()){
      std::cerr << "test_splice failed" << std::endl;
      return 1;
   }

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

   return 0;
}
