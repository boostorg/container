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
#include <set>
#include <boost/container/set.hpp>
#include <boost/container/allocator.hpp>
#include <boost/container/node_allocator.hpp>
#include <boost/container/adaptive_pool.hpp>

#include "print_container.hpp"
#include "movable_int.hpp"
#include "dummy_test_allocator.hpp"
#include "set_test.hpp"
#include "propagate_allocator_test.hpp"
#include "emplace_test.hpp"

using namespace boost::container;

namespace boost {
namespace container {

//Explicit instantiation to detect compilation errors

//set
template class set
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , test::dummy_test_allocator<test::movable_and_copyable_int>
   >;

template class set
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , test::simple_allocator<test::movable_and_copyable_int>
   >;

template class set
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , std::allocator<test::movable_and_copyable_int>
   >;

template class set
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , allocator<test::movable_and_copyable_int>
   >;

template class set
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , adaptive_pool<test::movable_and_copyable_int>
   >;

template class set
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , node_allocator<test::movable_and_copyable_int>
   >;

//multiset
template class multiset
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , test::dummy_test_allocator<test::movable_and_copyable_int>
   >;

template class multiset
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , test::simple_allocator<test::movable_and_copyable_int>
   >;

template class multiset
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , std::allocator<test::movable_and_copyable_int>
   >;

template class multiset
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , allocator<test::movable_and_copyable_int>
   >;

template class multiset
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , adaptive_pool<test::movable_and_copyable_int>
   >;

template class multiset
   < test::movable_and_copyable_int
   , std::less<test::movable_and_copyable_int>
   , node_allocator<test::movable_and_copyable_int>
   >;

}} //boost::container

//Test recursive structures
class recursive_set
{
public:
   recursive_set & operator=(const recursive_set &x)
   {  id_ = x.id_;  set_ = x.set_; return *this; }

   int id_;
   set<recursive_set> set_;
   friend bool operator< (const recursive_set &a, const recursive_set &b)
   {  return a.id_ < b.id_;   }
};

//Test recursive structures
class recursive_multiset
{
   public:
   recursive_multiset & operator=(const recursive_multiset &x)
   {  id_ = x.id_;  multiset_ = x.multiset_; return *this;  }

   int id_;
   multiset<recursive_multiset> multiset_;
   friend bool operator< (const recursive_multiset &a, const recursive_multiset &b)
   {  return a.id_ < b.id_;   }
};

template<class C>
void test_move()
{
   //Now test move semantics
   C original;
   original.emplace();
   C move_ctor(boost::move(original));
   C move_assign;
   move_assign.emplace();
   move_assign = boost::move(move_ctor);
   move_assign.swap(original);
}

template<class T, class A>
class tree_propagate_test_wrapper
   : public container_detail::tree<T, T, container_detail::identity<T>, std::less<T>, A, red_black_tree>
{
   BOOST_COPYABLE_AND_MOVABLE(tree_propagate_test_wrapper)
   typedef container_detail::tree<T, T, container_detail::identity<T>, std::less<T>, A, red_black_tree> Base;
   public:
   tree_propagate_test_wrapper()
      : Base()
   {}

   tree_propagate_test_wrapper(const tree_propagate_test_wrapper &x)
      : Base(x)
   {}

   tree_propagate_test_wrapper(BOOST_RV_REF(tree_propagate_test_wrapper) x)
      : Base(boost::move(static_cast<Base&>(x)))
   {}

   tree_propagate_test_wrapper &operator=(BOOST_COPY_ASSIGN_REF(tree_propagate_test_wrapper) x)
   {  this->Base::operator=(x);  return *this; }

   tree_propagate_test_wrapper &operator=(BOOST_RV_REF(tree_propagate_test_wrapper) x)
   {  this->Base::operator=(boost::move(static_cast<Base&>(x)));  return *this; }

   void swap(tree_propagate_test_wrapper &x)
   {  this->Base::swap(x);  }
};

template<class VoidAllocator>
struct GetAllocatorSet
{
   template<class ValueType>
   struct apply
   {
      typedef set < ValueType
                  , std::less<ValueType>
                  , typename allocator_traits<VoidAllocator>
                     ::template portable_rebind_alloc<ValueType>::type
                  > set_type;

      typedef multiset < ValueType
                  , std::less<ValueType>
                  , typename allocator_traits<VoidAllocator>
                     ::template portable_rebind_alloc<ValueType>::type
                  > multiset_type;
   };
};

template<class VoidAllocator>
int test_set_variants()
{
   typedef typename GetAllocatorSet<VoidAllocator>::template apply<int>::set_type MySet;
   typedef typename GetAllocatorSet<VoidAllocator>::template apply<test::movable_int>::set_type MyMoveSet;
   typedef typename GetAllocatorSet<VoidAllocator>::template apply<test::movable_and_copyable_int>::set_type MyCopyMoveSet;
   typedef typename GetAllocatorSet<VoidAllocator>::template apply<test::copyable_int>::set_type MyCopySet;

   typedef typename GetAllocatorSet<VoidAllocator>::template apply<int>::multiset_type MyMultiSet;
   typedef typename GetAllocatorSet<VoidAllocator>::template apply<test::movable_int>::multiset_type MyMoveMultiSet;
   typedef typename GetAllocatorSet<VoidAllocator>::template apply<test::movable_and_copyable_int>::multiset_type MyCopyMoveMultiSet;
   typedef typename GetAllocatorSet<VoidAllocator>::template apply<test::copyable_int>::multiset_type MyCopyMultiSet;

   typedef std::set<int>                                          MyStdSet;
   typedef std::multiset<int>                                     MyStdMultiSet;

   if (0 != test::set_test<
                  MySet
                  ,MyStdSet
                  ,MyMultiSet
                  ,MyStdMultiSet>()){
      std::cout << "Error in set_test<MyBoostSet>" << std::endl;
      return 1;
   }

   if (0 != test::set_test<
                  MyMoveSet
                  ,MyStdSet
                  ,MyMoveMultiSet
                  ,MyStdMultiSet>()){
      std::cout << "Error in set_test<MyBoostSet>" << std::endl;
      return 1;
   }

   if (0 != test::set_test<
                  MyCopyMoveSet
                  ,MyStdSet
                  ,MyCopyMoveMultiSet
                  ,MyStdMultiSet>()){
      std::cout << "Error in set_test<MyBoostSet>" << std::endl;
      return 1;
   }

   if (0 != test::set_test<
                  MyCopySet
                  ,MyStdSet
                  ,MyCopyMultiSet
                  ,MyStdMultiSet>()){
      std::cout << "Error in set_test<MyBoostSet>" << std::endl;
      return 1;
   }

   return 0;
}


int main ()
{
   //Recursive container instantiation
   {
      set<recursive_set> set_;
      multiset<recursive_multiset> multiset_;
   }
   //Allocator argument container
   {
      set<int> set_((std::allocator<int>()));
      multiset<int> multiset_((std::allocator<int>()));
   }
   //Now test move semantics
   {
      test_move<set<recursive_set> >();
      test_move<multiset<recursive_multiset> >();
   }

   if(test_set_variants< std::allocator<void> >()){
      std::cerr << "test_set_variants< std::allocator<void> > failed" << std::endl;
      return 1;
   }

   if(test_set_variants< allocator<void> >()){
      std::cerr << "test_set_variants< allocator<void> > failed" << std::endl;
      return 1;
   }

   if(test_set_variants< node_allocator<void> >()){
      std::cerr << "test_set_variants< node_allocator<void> > failed" << std::endl;
      return 1;
   }

   if(test_set_variants< adaptive_pool<void> >()){
      std::cerr << "test_set_variants< adaptive_pool<void> > failed" << std::endl;
      return 1;
   }

   const test::EmplaceOptions SetOptions = (test::EmplaceOptions)(test::EMPLACE_HINT | test::EMPLACE_ASSOC);
   if(!boost::container::test::test_emplace<set<test::EmplaceInt>, SetOptions>())
      return 1;
   if(!boost::container::test::test_emplace<multiset<test::EmplaceInt>, SetOptions>())
      return 1;
   if(!boost::container::test::test_propagate_allocator<tree_propagate_test_wrapper>())
      return 1;

   return 0;
}

#include <boost/container/detail/config_end.hpp>
