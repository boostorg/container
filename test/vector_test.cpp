//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#include <boost/container/detail/config_begin.hpp>
#include <algorithm>
#include <memory>
#include <vector>
#include <iostream>
#include <functional>

#include <boost/container/vector.hpp>
#include <boost/container/allocator.hpp>
#include <boost/container/node_allocator.hpp>
#include <boost/container/adaptive_pool.hpp>

#include <boost/move/utility_core.hpp>
#include "check_equal_containers.hpp"
#include "movable_int.hpp"
#include "expand_bwd_test_allocator.hpp"
#include "expand_bwd_test_template.hpp"
#include "dummy_test_allocator.hpp"
#include "propagate_allocator_test.hpp"
#include "vector_test.hpp"
#include "default_init_test.hpp"

using namespace boost::container;

namespace boost {
namespace container {

//Explicit instantiation to detect compilation errors
template class boost::container::vector
   < test::movable_and_copyable_int
   , test::simple_allocator<test::movable_and_copyable_int> >;

template class boost::container::vector
   < test::movable_and_copyable_int
   , test::dummy_test_allocator<test::movable_and_copyable_int> >;

template class boost::container::vector
   < test::movable_and_copyable_int
   , std::allocator<test::movable_and_copyable_int> >;

template class boost::container::vector
   < test::movable_and_copyable_int
   , allocator<test::movable_and_copyable_int> >;

template class boost::container::vector
   < test::movable_and_copyable_int
   , adaptive_pool<test::movable_and_copyable_int> >;

template class boost::container::vector
   < test::movable_and_copyable_int
   , node_allocator<test::movable_and_copyable_int> >;

namespace container_detail {

#ifndef BOOST_CONTAINER_VECTOR_ITERATOR_IS_POINTER

template class vec_iterator<int*, true >;
template class vec_iterator<int*, false>;

#endif   //BOOST_CONTAINER_VECTOR_ITERATOR_IS_POINTER

}

}}

int test_expand_bwd()
{
   //Now test all back insertion possibilities

   //First raw ints
   typedef test::expand_bwd_test_allocator<int>
      int_allocator_type;
   typedef vector<int, int_allocator_type>
      int_vector;
   if(!test::test_all_expand_bwd<int_vector>())
      return 1;

   //Now user defined copyable int
   typedef test::expand_bwd_test_allocator<test::copyable_int>
      copyable_int_allocator_type;
   typedef vector<test::copyable_int, copyable_int_allocator_type>
      copyable_int_vector;
   if(!test::test_all_expand_bwd<copyable_int_vector>())
      return 1;

   return 0;
}

class recursive_vector
{
   public:
   int id_;
   vector<recursive_vector> vector_;
   vector<recursive_vector>::iterator it_;
   vector<recursive_vector>::const_iterator cit_;
   vector<recursive_vector>::reverse_iterator rit_;
   vector<recursive_vector>::const_reverse_iterator crit_;
};

void recursive_vector_test()//Test for recursive types
{
   vector<recursive_vector> recursive_vector_vector;
}

template <class Iterator>
bool test_value_init_iterator()
{
   // Create jumbled up memory to seat the iterator objects:
   char c_arr[2*sizeof(Iterator)];
   for(std::size_t i = 0; i < 2*sizeof(Iterator); ++i)
      c_arr[i] = i;
   Iterator* p_it1 = reinterpret_cast<Iterator*>(c_arr);
   Iterator* p_it2 = reinterpret_cast<Iterator*>(c_arr + sizeof(Iterator));
   // Do a placement value-initialization of iterators:
   new(p_it1) Iterator();
   new(p_it2) Iterator();
   bool result = (*p_it1 == *p_it2);
   p_it2->~Iterator();
   p_it1->~Iterator();
   return result;
}

enum Test
{
   zero, one, two, three, four, five, six
};

template<class VoidAllocator>
struct GetAllocatorCont
{
   template<class ValueType>
   struct apply
   {
      typedef vector< ValueType
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

   if(test::vector_test<MyCont>())
      return 1;
   if(test::vector_test<MyMoveCont>())
      return 1;
   if(test::vector_test<MyCopyMoveCont>())
      return 1;
   if(test::vector_test<MyCopyCont>())
      return 1;

   return 0;
}

int main()
{
   {
      const std::size_t positions_length = 10;
      std::size_t positions[positions_length];
      vector<int> vector_int;
      vector<int> vector_int2(positions_length);
      for(std::size_t i = 0; i != positions_length; ++i){
         positions[i] = 0u;
      }
      for(std::size_t i = 0, max = vector_int2.size(); i != max; ++i){
         vector_int2[i] = i;
      }

      vector_int.insert(vector_int.begin(), 999);

      vector_int.insert_ordered_at(positions_length, positions + positions_length, vector_int2.end());

      for(std::size_t i = 0, max = vector_int.size(); i != max; ++i){
         std::cout << vector_int[i] << std::endl;
      }
   }
   recursive_vector_test();
   {
      //Now test move semantics
      vector<recursive_vector> original;
      vector<recursive_vector> move_ctor(boost::move(original));
      vector<recursive_vector> move_assign;
      move_assign = boost::move(move_ctor);
      move_assign.swap(original);
   }

   ////////////////////////////////////
   //    Testing allocator implementations
   ////////////////////////////////////
   //       std:allocator
   if(test_cont_variants< std::allocator<void> >()){
      std::cerr << "test_cont_variants< std::allocator<void> > failed" << std::endl;
      return 1;
   }
   //       boost::container::allocator
   if(test_cont_variants< allocator<void> >()){
      std::cerr << "test_cont_variants< allocator<void> > failed" << std::endl;
      return 1;
   }
   //       boost::container::node_allocator
   if(test_cont_variants< node_allocator<void> >()){
      std::cerr << "test_cont_variants< node_allocator<void> > failed" << std::endl;
      return 1;
   }
   //       boost::container::adaptive_pool
   if(test_cont_variants< adaptive_pool<void> >()){
      std::cerr << "test_cont_variants< adaptive_pool<void> > failed" << std::endl;
      return 1;
   }

   {
      typedef vector<Test, std::allocator<Test> > MyEnumCont;
      MyEnumCont v;
      Test t;
      v.push_back(t);
      v.push_back(::boost::move(t));
      v.push_back(Test());
   }

   ////////////////////////////////////
   //    Backwards expansion test
   ////////////////////////////////////
   if(test_expand_bwd())
      return 1;

   ////////////////////////////////////
   //    Default init test
   ////////////////////////////////////
   if(!test::default_init_test< vector<int, test::default_init_allocator<int> > >()){
      std::cerr << "Default init test failed" << std::endl;
      return 1;
   }

   ////////////////////////////////////
   //    Emplace testing
   ////////////////////////////////////
   const test::EmplaceOptions Options = (test::EmplaceOptions)(test::EMPLACE_BACK | test::EMPLACE_BEFORE);
   if(!boost::container::test::test_emplace< vector<test::EmplaceInt>, Options>()){
      return 1;
   }

   ////////////////////////////////////
   //    Allocator propagation testing
   ////////////////////////////////////
   if(!boost::container::test::test_propagate_allocator<vector>()){
      return 1;
   }

   ////////////////////////////////////
   //    Initializer lists testing
   ////////////////////////////////////
   if(!boost::container::test::test_vector_methods_with_initializer_list_as_argument_for<
       boost::container::vector<int>
   >()) {
      return 1;
   }

   ////////////////////////////////////
   //    n3644 "Null forward iterator" test:
   ////////////////////////////////////
   { 
      typedef vector<int> cont_t;
      if(!test_value_init_iterator<cont_t::iterator>()){
         std::cerr << "test for n3644 failed on iterator on vector" << std::endl;
         return 1;
      }
      if(!test_value_init_iterator<cont_t::const_iterator>()){
         std::cerr << "test for n3644 failed on const_iterator on vector" << std::endl;
         return 1;
      }
      if(!test_value_init_iterator<cont_t::reverse_iterator>()){
         std::cerr << "test for n3644 failed on reverse_iterator on vector" << std::endl;
         return 1;
      }
      if(!test_value_init_iterator<cont_t::const_reverse_iterator>()){
         std::cerr << "test for n3644 failed on const_reverse_iterator on vector" << std::endl;
         return 1;
      }
   }

   return 0;

}
#include <boost/container/detail/config_end.hpp>
