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

#include <memory>
#include <deque>
#ifdef _MSC_VER
#pragma warning(disable : 4530) // C++ exception handler used, but unwind semantics are not enabled
#endif
#include <iostream>
#include <list>

#include <boost/container/deque.hpp>
#include <boost/container/allocator.hpp>


#include "print_container.hpp"
#include "check_equal_containers.hpp"
#include "dummy_test_allocator.hpp"
#include "movable_int.hpp"
#include <boost/move/utility_core.hpp>
#include <boost/move/iterator.hpp>
#include <boost/container/detail/mpl.hpp>
#include <boost/container/detail/type_traits.hpp>
#include <string>
#include "emplace_test.hpp"
#include "propagate_allocator_test.hpp"
#include "allocator_test.hpp"
#include "vector_test.hpp"
#include "default_init_test.hpp"
#include "../../intrusive/test/iterator_test.hpp"
#include "iterator_noexcept_utils.hpp"
#include "segmented_test.hpp"
#include "container_noexcept_utils.hpp"

#include "lightweight_test.hpp"
#include "unqualified_swap_test.hpp"

using namespace boost::container;

//Test recursive structures
class recursive_deque
{
public:

   recursive_deque (const recursive_deque &x)
      : deque_(x.deque_)
   {}

   recursive_deque & operator=(const recursive_deque &x)
   {  this->deque_ = x.deque_;   return *this; }

   deque<recursive_deque> deque_;
   deque<recursive_deque>::iterator it_;
   deque<recursive_deque>::const_iterator cit_;
   deque<recursive_deque>::reverse_iterator rit_;
   deque<recursive_deque>::const_reverse_iterator crit_;
};

bool do_recursive_deque_test()
{
   //Test for recursive types
   {
      deque<recursive_deque> recursive_deque_deque;
   }

   {
      //Now test move semantics
      deque<recursive_deque> original;
      deque<recursive_deque> move_ctor(boost::move(original));
      deque<recursive_deque> move_assign;
      move_assign = boost::move(move_ctor);
      move_assign.swap(original);
   }
   return true;
}

template<class IntType, bool Reservable>
bool do_test()
{
   typedef typename deque_options<reservable<Reservable> >::type Options;
   {
      typedef deque<IntType, void, Options>  MyCnt;
      ::boost::movelib::unique_ptr<MyCnt> const pcntc = ::boost::movelib::make_unique<MyCnt>();
      pcntc->erase(pcntc->cbegin(), pcntc->cend());
   }

   //Alias deque types
   typedef deque<IntType, void, Options>  MyCnt;
   typedef std::deque<int> MyStd;
   const int max = 100;
   {
      ::boost::movelib::unique_ptr<MyCnt> const pcntc = ::boost::movelib::make_unique<MyCnt>();
      ::boost::movelib::unique_ptr<MyStd> const pstdc = ::boost::movelib::make_unique<MyStd>();
      MyCnt &cntc = *pcntc;
      MyStd &stdc = *pstdc;
      for(int i = 0; i < max*100; ++i){
         IntType move_me(i);
         cntc.insert(cntc.end(), boost::move(move_me));
         stdc.insert(stdc.end(), i);
         //A full comparison after each insertion is quadratic and slow in debug
         //so the containers are compared each "max" insertions
         if((i % max) == 0 && !test::CheckEqualContainers(cntc, stdc))
            return false;
      }
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      cntc.clear();
      stdc.clear();

      for(int i = 0; i < max*100; ++i){
         IntType move_me(i);
         cntc.push_back(boost::move(move_me));
         stdc.push_back(i);
         if((i % max) == 0 && !test::CheckEqualContainers(cntc, stdc))
            return false;
      }
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      cntc.clear();
      stdc.clear();

      for(int i = 0; i < max*100; ++i){
         IntType move_me(i);
         cntc.push_front(boost::move(move_me));
         stdc.push_front(i);
         if((i % max) == 0 && !test::CheckEqualContainers(cntc, stdc))
            return false;
      }
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      typename MyCnt::iterator it;
      typename MyCnt::const_iterator cit = it;
      (void)cit;

      cntc.erase(cntc.begin()++);
      stdc.erase(stdc.begin()++);
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      cntc.erase(cntc.erase(cntc.begin()++));
      stdc.erase(stdc.erase(stdc.begin()++));
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      cntc.erase(cntc.erase(cntc.begin()+3));
      stdc.erase(stdc.erase(stdc.begin()+3));
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      cntc.erase(cntc.erase(cntc.end()-2));
      stdc.erase(stdc.erase(stdc.end()-2));
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      cntc.erase(cntc.erase(cntc.end()-4));
      stdc.erase(stdc.erase(stdc.end()-4));
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      cntc.erase(cntc.begin());
      stdc.erase(stdc.begin());
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      cntc.erase(cntc.end()-1);
      stdc.erase(stdc.end()-1);
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      {
         //Initialize values
         IntType aux_vect[50];
         for(int i = 0; i < 50; ++i){
            IntType move_me (-1);
            aux_vect[i] = boost::move(move_me);
         }
         int aux_vect2[50];
         for(int i = 0; i < 50; ++i){
            aux_vect2[i] = -1;
         }

         cntc.insert(cntc.end()
                           ,boost::make_move_iterator(&aux_vect[0])
                           ,boost::make_move_iterator(aux_vect + 50));
         stdc.insert(stdc.end(), aux_vect2, aux_vect2 + 50);
         if(!test::CheckEqualContainers(cntc, stdc)) return false;

         for(int i = 0; i < 50; ++i){
            IntType move_me (i);
            aux_vect[i] = boost::move(move_me);
         }
         for(int i = 0; i < 50; ++i){
            aux_vect2[i] = i;
         }

         cntc.insert(cntc.begin()+std::ptrdiff_t(cntc.size())
                           ,boost::make_move_iterator(&aux_vect[0])
                           ,boost::make_move_iterator(aux_vect + 50));
         stdc.insert(stdc.begin()+std::ptrdiff_t(stdc.size()), aux_vect2, aux_vect2 + 50);
         if(!test::CheckEqualContainers(cntc, stdc)) return false;

         for(int i = 0, j = static_cast<int>(cntc.size()); i < j; ++i){
            cntc.erase(cntc.begin());
            stdc.erase(stdc.begin());
         }
         if(!test::CheckEqualContainers(cntc, stdc)) return false;
      }
      {
         IntType aux_vect[50];
         for(int i = 0; i < 50; ++i){
            IntType move_me(-1);
            aux_vect[i] = boost::move(move_me);
         }
         int aux_vect2[50];
         for(int i = 0; i < 50; ++i){
            aux_vect2[i] = -1;
         }
         cntc.insert(cntc.begin()
                           ,boost::make_move_iterator(&aux_vect[0])
                           ,boost::make_move_iterator(aux_vect + 50));
         stdc.insert(stdc.begin(), aux_vect2, aux_vect2 + 50);
         if(!test::CheckEqualContainers(cntc, stdc)) return false;
      }

      if(!test::segmented_copyable_only(cntc, stdc
                     ,dtl::bool_<boost::container::test::is_copyable<IntType>::value>())){
         return false;
      }

      cntc.erase(cntc.begin());
      stdc.erase(stdc.begin());

      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      for(int i = 0; i < max; ++i){
         IntType move_me(i);
         cntc.insert(cntc.begin(), boost::move(move_me));
         stdc.insert(stdc.begin(), i);
      }
      if(!test::CheckEqualContainers(cntc, stdc)) return false;

      //Test insertion from list
      {
         std::list<int> l(50, int(1));
         cntc.insert(cntc.begin(), l.begin(), l.end());
         stdc.insert(stdc.begin(), l.begin(), l.end());
         if(!test::CheckEqualContainers(cntc, stdc)) return 1;
         cntc.assign(l.begin(), l.end());
         stdc.assign(l.begin(), l.end());
         if(!test::CheckEqualContainers(cntc, stdc)) return 1;
      }

      cntc.resize(100);
      stdc.resize(100);
      if(!test::CheckEqualContainers(cntc, stdc)) return 1;

      cntc.resize(200);
      stdc.resize(200);
      if(!test::CheckEqualContainers(cntc, stdc)) return 1;
   }

   std::cout << std::endl << "Test OK!" << std::endl;
   return true;
}

bool test_ctad()  //Older clang versions suffer from ICE here
#if !defined(BOOST_CONTAINER_NO_CXX17_CTAD) && (!defined(BOOST_CLANG) || (BOOST_CLANG_VERSION >= 80000))
{
   typedef std::vector<int> MyStd;
   //Check Constructor Template Auto Deduction
   {
      MyStd gold = MyStd{ 1, 2, 3 };
      deque<int> test = deque(gold.begin(), gold.end());
      if (!test::CheckEqualContainers(gold, test)) return false;
   }
   {
      MyStd gold = MyStd{ 1, 2, 3 };
      deque<int>  test = deque<int>(gold.begin(), gold.end(), new_allocator<int>());
      if (!test::CheckEqualContainers(gold, test)) return false;
   }
   {
      deque test(5, 3);
      deque test2(5, 3, new_allocator<int>());
      if (test.size() != 5u || test2.size() != 5u || test[4] != 3 || test2[4] != 3) return false;
   }
   return true;
}
#else
{ return true;  }
#endif

struct boost_container_deque;

namespace boost { namespace container {   namespace test {

template<>
struct alloc_propagate_base<boost_container_deque>
{
   template <class T, class Allocator>
   struct apply
   {
      typedef boost::container::deque<T, Allocator> type;
   };
};

}}}   //namespace boost::container::test

//Test the expected sizeof()
BOOST_CONTAINER_STATIC_ASSERT_MSG(4*sizeof(void*) == sizeof(deque<int>), "sizeof has an unexpected value");

bool test_unqualified_swap()
{
   namespace us = boost_container_test_unqualified_swap;
   {  typedef boost::container::deque<int> cont;
      cont a;  a.push_back(1);  a.push_back(2);
      cont b;  b.push_back(7);  b.push_back(8);  b.push_back(9);
      if(!us::test_unqualified_swap(a, b))
         return false;
   }
   return true;
}

//A size whose number of blocks times the block size overflows size_type must throw
template<bool Reservable>
bool test_huge_size_length_error()
{
   #ifndef BOOST_NO_EXCEPTIONS
   typedef typename boost::container::deque_options<boost::container::reservable<Reservable> >::type options_t;
   boost::container::deque<char, void, options_t> d(10u, 'a');
   //Sizes above max_size(): a smaller size can throw bad_alloc instead
   const std::size_t max = d.max_size();
   const std::size_t sizes[] = { std::size_t(-1), std::size_t(-1) - 1u, max == std::size_t(-1) ? max : std::size_t(max + 1u) };
   for(std::size_t i = 0; i != sizeof(sizes)/sizeof(sizes[0]); ++i){
      bool length_error_thrown = false;
      try{
         d.resize(sizes[i]);
      }
      catch(const boost::container::length_error_t &){
         length_error_thrown = true;
      }
      catch(...){
      }
      if(!length_error_thrown || d.size() != 10u)
         return false;
   }
   #endif   //BOOST_NO_EXCEPTIONS
   return true;
}

bool test_reserve_capacity()
{
   //reserve_back and reserve_front reach the requested capacity also when the
   //missing capacity is not bigger than the free slots
   typedef deque<int, void, deque_options< reservable<true>, block_size<8u> >::type> deque_t;
   deque_t d;
   d.reserve_back(10u);
   if(d.back_capacity() < 10u)
      return false;
   const deque_t::size_type c = d.back_capacity();
   d.reserve_back(c + 5u);
   if(d.back_capacity() < c + 5u)
      return false;
   d.push_back(1);
   d.push_back(2);
   d.reserve_front(13u);
   if(d.front_capacity() < 13u)
      return false;
   const deque_t::size_type cf = d.front_capacity();
   d.reserve_front(cf + 5u);
   if(d.front_capacity() < cf + 5u)
      return false;
   return d.size() == 2u && d.front() == 1 && d.back() == 2;
}

//The operations of the iterators cannot throw
#if !defined(BOOST_NO_CXX11_NOEXCEPT)
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::random_access_container_iterators_are_nothrow
   < boost::container::deque<int> >::value));
#endif   //#if !defined(BOOST_NO_CXX11_NOEXCEPT)

//The operations that do not allocate memory or move elements cannot throw
#if !defined(BOOST_NO_CXX11_NOEXCEPT)
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::reversible_container_ops_are_nothrow
   < boost::container::deque<int> >::value));
BOOST_CONTAINER_STATIC_ASSERT((boost::container::test::container_erase_is_nothrow
   < boost::container::deque<int> >::value));
//shrink_to_fit only deallocates memory
BOOST_CONTAINER_STATIC_ASSERT((noexcept(boost::move_detail::declval< boost::container::deque<int>& >().shrink_to_fit())));
#endif   //#if !defined(BOOST_NO_CXX11_NOEXCEPT)

int main ()
{
   if(!test_huge_size_length_error<false>() || !test_huge_size_length_error<true>())
      return 1;

   if(!test_unqualified_swap())
      return 1;

   if(!do_recursive_deque_test())
      return 1;

   //Non-reservable deque
   if(!do_test<int, false>())
      return 1;

   if(!do_test<test::movable_int, true>())
      return 1;

   if(!do_test<test::movable_and_copyable_int, false>())
      return 1;

   if(!do_test<test::copyable_int, true>())
      return 1;

   if (!test_ctad())
      return 1;

   //Default block size
   if(!test::test_segmented_default_block_size<deque>())
      return 1;

   //Test non-copy-move operations
   {
      deque<test::non_copymovable_int> d;
      d.emplace_back();
      d.emplace_front(1);
      d.resize(10);
      d.resize(1);
   }


   ////////////////////////////////////
   //    Void value_type allocator
   ////////////////////////////////////
   if(!test::test_void_allocator
         < deque<int, new_allocator<void> >
         , new_allocator<int> >()) {
      std::cerr << "test_void_allocator deque failed" << std::endl;
      return 1;
   }

   ////////////////////////////////////
   //    Allocator implementations
   ////////////////////////////////////
   //       std:allocator
   if(test::test_segmented_cont_variants< deque, std::allocator<void>, false >()){
      std::cerr << "test_cont_variants< std::allocator<void> > failed" << std::endl;
      return 1;
   }

   //       boost::container::allocator
   if(test::test_segmented_cont_variants< deque, allocator<void>, true >()){
      std::cerr << "test_cont_variants< allocator<void> > failed" << std::endl;
      return 1;
   }

   ////////////////////////////////////
   //    Default init test
   ////////////////////////////////////
   if(!test::default_init_test< deque<int, test::default_init_allocator<int> > >()){
      std::cerr << "Default init test failed" << std::endl;
      return 1;
   }

   ////////////////////////////////////
   //    Emplace testing
   ////////////////////////////////////
   const test::EmplaceOptions Options = (test::EmplaceOptions)(test::EMPLACE_BACK | test::EMPLACE_FRONT | test::EMPLACE_BEFORE);

   if(!boost::container::test::test_emplace
      < deque<test::EmplaceInt>, Options>())
      return 1;
   ////////////////////////////////////
   //    Allocator propagation testing
   ////////////////////////////////////
   if(!boost::container::test::test_propagate_allocator<boost_container_deque>())
      return 1;

   ////////////////////////////////////
   //    Initializer lists testing
   ////////////////////////////////////
   if(!boost::container::test::test_vector_methods_with_initializer_list_as_argument_for
      < boost::container::deque<int> >()) {
      return 1;
   }

   ////////////////////////////////////
   //    Iterator testing
   ////////////////////////////////////
   {
      typedef boost::container::deque<int> cont_int;
      for(std::size_t i = 1; i <= 10000; i*=10){
         cont_int a;
         for (int j = 0; j < (int)i; ++j)
            a.push_back((int)j);
         boost::intrusive::test::test_iterator_random< cont_int >(a);
         if(boost::report_errors() != 0) {
            return 1;
         }
      }
   }

   ////////////////////////////////////
   //    has_trivial_destructor_after_move testing
   ////////////////////////////////////
   // default allocator
   {
      typedef boost::container::deque<int> cont;
      typedef cont::allocator_type allocator_type;
      typedef boost::container::allocator_traits<allocator_type>::pointer pointer;
      BOOST_CONTAINER_STATIC_ASSERT_MSG(!(boost::has_trivial_destructor_after_move<cont>::value !=
                                boost::has_trivial_destructor_after_move<allocator_type>::value &&
                                boost::has_trivial_destructor_after_move<pointer>::value)
                             , "has_trivial_destructor_after_move(std::allocator) test failed");
   }
   // std::allocator
   {
      typedef boost::container::deque<int, std::allocator<int> > cont;
      typedef cont::allocator_type allocator_type;
      typedef boost::container::allocator_traits<allocator_type>::pointer pointer;
      BOOST_CONTAINER_STATIC_ASSERT_MSG(!(boost::has_trivial_destructor_after_move<cont>::value !=
                               boost::has_trivial_destructor_after_move<allocator_type>::value &&
                               boost::has_trivial_destructor_after_move<pointer>::value)
                             , "has_trivial_destructor_after_move(std::allocator) test failed");
   }

   if(!test_reserve_capacity())
      return 1;

   if(!test::test_segmented_shrink_to_fit<deque, true>())
      return 1;

   return 0;
}
