////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_SET_TEST_HEADER
#define BOOST_CONTAINER_TEST_SET_TEST_HEADER

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/compare_functors.hpp>
#include "check_equal_containers.hpp"
#include "print_container.hpp"
#include "movable_int.hpp"
#include "throwing_utilities.hpp"
#include "shared_input_iterator.hpp"
#include <boost/intrusive/detail/mpl.hpp>
#include <boost/move/detail/meta_utils.hpp>   //declval
#include <boost/move/utility_core.hpp>
#include <boost/move/iterator.hpp>
#include <boost/move/make_unique.hpp>

#if defined(BOOST_GCC) && (BOOST_GCC >= 40600)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
#endif

#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME rebalance
#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEG namespace boost { namespace container { namespace test {
#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END   }}}
#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN 0
#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX 0
#include <boost/intrusive/detail/has_member_function_callable_with.hpp>

//#pragma GCC diagnostic ignored "-Wunused-result"
#if defined(BOOST_GCC) && (BOOST_GCC >= 40600)
#pragma GCC diagnostic pop
#endif

namespace boost{
namespace container {
namespace test{

template<class C>
void set_test_rebalanceable(C &, boost::container::dtl::false_type)
{}

template<class C>
void set_test_rebalanceable(C &c, boost::container::dtl::true_type)
{
   c.rebalance();
}

template<class MyBoostSet
        ,class MyStdSet
        ,class MyBoostMultiSet
        ,class MyStdMultiSet>
int set_test_copyable(boost::container::dtl::false_type)
{  return 0; }

const int MaxElem = 50;

template<class MyBoostSet
        ,class MyStdSet
        ,class MyBoostMultiSet
        ,class MyStdMultiSet>
int set_test_copyable(boost::container::dtl::true_type)
{
   typedef typename MyBoostSet::value_type IntType;

   ::boost::movelib::unique_ptr<MyBoostSet> const pboostset = ::boost::movelib::make_unique<MyBoostSet>();
   ::boost::movelib::unique_ptr<MyStdSet>   const pstdset = ::boost::movelib::make_unique<MyStdSet>();
   ::boost::movelib::unique_ptr<MyBoostMultiSet> const pboostmultiset = ::boost::movelib::make_unique<MyBoostMultiSet>();
   ::boost::movelib::unique_ptr<MyStdMultiSet>   const pstdmultiset   = ::boost::movelib::make_unique<MyStdMultiSet>();

   MyBoostSet &boostset = *pboostset;
   MyStdSet   &stdset   = *pstdset;
   MyBoostMultiSet &boostmultiset = *pboostmultiset;
   MyStdMultiSet   &stdmultiset   = *pstdmultiset;

   //Just to test move aware catch conversions
   boostset.insert(boostset.cbegin(), boostset.cend());
   boostmultiset.insert(boostmultiset.cbegin(), boostmultiset.cend());
   boostset.insert(boostset.begin(), boostset.end());
   boostmultiset.insert(boostmultiset.begin(), boostmultiset.end());

   for(int i = 0; i < MaxElem; ++i){
      IntType move_me(i);
      boostset.insert(boost::move(move_me));
      stdset.insert(i);
      IntType move_me2(i);
      boostmultiset.insert(boost::move(move_me2));
      stdmultiset.insert(i);
   }
   if(!CheckEqualContainers(boostset, stdset)) return 1;
   if(!CheckEqualContainers(boostmultiset, stdmultiset)) return 1;

   {
      //Now, test copy constructor
      MyBoostSet boostsetcopy(boostset);
      MyStdSet stdsetcopy(stdset);

      if(!CheckEqualContainers(boostsetcopy, stdsetcopy))
         return 1;

      MyBoostMultiSet boostmsetcopy(boostmultiset);
      MyStdMultiSet stdmsetcopy(stdmultiset);

      if(!CheckEqualContainers(boostmsetcopy, stdmsetcopy))
         return 1;

      //And now assignment
      boostsetcopy  =boostset;
      stdsetcopy  = stdset;

      if(!CheckEqualContainers(boostsetcopy, stdsetcopy))
         return 1;

      boostmsetcopy = boostmultiset;
      stdmsetcopy = stdmultiset;

      if(!CheckEqualContainers(boostmsetcopy, stdmsetcopy))
         return 1;
   }
   {
      //Now, test copy constructor with allocator
      MyBoostSet boostsetcopy(boostset, typename MyBoostSet::allocator_type());
      MyStdSet stdsetcopy(stdset);

      if(!CheckEqualContainers(boostsetcopy, stdsetcopy))
         return 1;

      MyBoostMultiSet boostmsetcopy(boostmultiset, typename MyBoostSet::allocator_type());
      MyStdMultiSet stdmsetcopy(stdmultiset);

      if(!CheckEqualContainers(boostmsetcopy, stdmsetcopy))
         return 1;
   }
   return 0;
}


//The allocator of a static_vector sequence holds a buffer that is not initialized. GCC 11 and
//later warn (maybe-uninitialized) when a temporary allocator_type() is passed by reference
//to a constructor that is not inlined, although the buffer is not read.
#if defined(BOOST_GCC) && (BOOST_GCC >= 110000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

BOOST_INTRUSIVE_HAS_MEMBER_FUNC_CALLED(set_has_member_nth, nth)

//Only flat sets (they have nth) have insert(ordered_[unique_]range, It, It)
template<class MyBoostSet, class MyStdSet, class OrderedTag, std::size_t N>
int set_single_pass_ordered_insert(const int (&)[N], OrderedTag, dtl::false_type /*has_nth*/)
{
   return 0;
}

template<class MyBoostSet, class MyStdSet, class OrderedTag, std::size_t N>
int set_single_pass_ordered_insert(const int (&sorted)[N], OrderedTag tag, dtl::true_type /*has_nth*/)
{
   typedef typename MyBoostSet::value_type IntType;
   //insert(ordered_[unique_]range, It, It) in an empty set and in a set with elements
   for(int n = 0; n != 2; ++n){
      MyBoostSet boostset;
      MyStdSet stdset;
      for(int i = 0; i != n*10; ++i){
         IntType move_me(100 + i);
         boostset.insert(boost::move(move_me));
         stdset.insert(100 + i);
      }
      shared_input_range<IntType, N> r(sorted);
      boostset.insert(tag, r.mbegin(), r.mend());
      stdset.insert(sorted, sorted + N);
      if(!r.read_once()) return 1;
      if(!CheckEqualContainers(boostset, stdset)) return 1;
   }
   return 0;
}

//Range functions with a single-pass iterator whose copies share the position.
//"values" is not sorted and has duplicates. "sorted" is valid for OrderedTag.
template<class MyBoostSet, class MyStdSet, class OrderedTag, std::size_t N>
int set_single_pass_test_cont(const int (&values)[N], const int (&sorted)[N], OrderedTag tag)
{
   typedef typename MyBoostSet::value_type      IntType;
   typedef typename MyBoostSet::key_compare     key_compare;
   typedef typename MyBoostSet::allocator_type  allocator_type;

   const MyStdSet stdset(values, values + N);
   const MyStdSet stdset_sorted(sorted, sorted + N);
   //Set(It, It), Set(It, It, comp), Set(It, It, alloc), Set(It, It, comp, alloc)
   for(int variant = 0; variant != 4; ++variant){
      shared_input_range<IntType, N> r(values);
      ::boost::movelib::unique_ptr<MyBoostSet> pboostset;
      switch(variant){
         case 0:  pboostset = ::boost::movelib::make_unique<MyBoostSet>(r.mbegin(), r.mend());  break;
         case 1:  pboostset = ::boost::movelib::make_unique<MyBoostSet>(r.mbegin(), r.mend(), key_compare());  break;
         case 2:  pboostset = ::boost::movelib::make_unique<MyBoostSet>(r.mbegin(), r.mend(), allocator_type());  break;
         default: pboostset = ::boost::movelib::make_unique<MyBoostSet>(r.mbegin(), r.mend(), key_compare(), allocator_type());  break;
      }
      if(!r.read_once()) return 1;
      if(!CheckEqualContainers(*pboostset, stdset)) return 1;
   }
   //The same constructors with ordered_[unique_]range
   for(int variant = 0; variant != 4; ++variant){
      shared_input_range<IntType, N> r(sorted);
      ::boost::movelib::unique_ptr<MyBoostSet> pboostset;
      switch(variant){
         case 0:  pboostset = ::boost::movelib::make_unique<MyBoostSet>(tag, r.mbegin(), r.mend());  break;
         case 1:  pboostset = ::boost::movelib::make_unique<MyBoostSet>(tag, r.mbegin(), r.mend(), key_compare());  break;
         case 2:  pboostset = ::boost::movelib::make_unique<MyBoostSet>(tag, r.mbegin(), r.mend(), allocator_type());  break;
         default: pboostset = ::boost::movelib::make_unique<MyBoostSet>(tag, r.mbegin(), r.mend(), key_compare(), allocator_type());  break;
      }
      if(!r.read_once()) return 1;
      if(!CheckEqualContainers(*pboostset, stdset_sorted)) return 1;
   }
   //insert(It, It) in an empty set and in a set with elements
   for(int n = 0; n != 2; ++n){
      MyBoostSet boostset;
      MyStdSet stdset_ins;
      for(int i = 0; i != n*10; ++i){
         IntType move_me(10 + i);
         boostset.insert(boost::move(move_me));
         stdset_ins.insert(10 + i);
      }
      shared_input_range<IntType, N> r(values);
      boostset.insert(r.mbegin(), r.mend());
      stdset_ins.insert(values, values + N);
      if(!r.read_once()) return 1;
      if(!CheckEqualContainers(boostset, stdset_ins)) return 1;
   }
   return set_single_pass_ordered_insert<MyBoostSet, MyStdSet>
      (sorted, tag, dtl::bool_<set_has_member_nth<MyBoostSet>::value>());
}

template<class MyBoostSet
        ,class MyStdSet
        ,class MyBoostMultiSet
        ,class MyStdMultiSet
        ,std::size_t N>
int set_single_pass_test_n()
{
   int values[N];
   int sorted_unique[N];
   int sorted_equal[N];
   for(std::size_t i = 0; i != N; ++i){
      values[i] = int((i*7u) % (N/2u + 1u));
      sorted_unique[i] = int(i*2u);
      sorted_equal[i] = int(i/2u);
   }
   if(0 != set_single_pass_test_cont<MyBoostSet, MyStdSet>(values, sorted_unique, ordered_unique_range))
      return 1;
   if(0 != set_single_pass_test_cont<MyBoostMultiSet, MyStdMultiSet>(values, sorted_equal, ordered_range))
      return 1;
   return 0;
}

template<class MyBoostSet
        ,class MyStdSet
        ,class MyBoostMultiSet
        ,class MyStdMultiSet>
int set_single_pass_test()
{
   if(0 != set_single_pass_test_n<MyBoostSet, MyStdSet, MyBoostMultiSet, MyStdMultiSet, 3u>())
      return 1;
   if(0 != set_single_pass_test_n<MyBoostSet, MyStdSet, MyBoostMultiSet, MyStdMultiSet, 50u>())
      return 1;
   return 0;
}

#if defined(BOOST_GCC) && (BOOST_GCC >= 110000)
#pragma GCC diagnostic pop
#endif

template<class MyBoostSet
        ,class MyStdSet
        ,class MyBoostMultiSet
        ,class MyStdMultiSet>
int set_test ()
{
   typedef typename MyBoostSet::value_type IntType;

   ::boost::movelib::unique_ptr<MyBoostSet> const pboostset = ::boost::movelib::make_unique<MyBoostSet>();
   ::boost::movelib::unique_ptr<MyStdSet>   const pstdset = ::boost::movelib::make_unique<MyStdSet>();
   ::boost::movelib::unique_ptr<MyBoostMultiSet> const pboostmultiset = ::boost::movelib::make_unique<MyBoostMultiSet>();
   ::boost::movelib::unique_ptr<MyStdMultiSet>   const pstdmultiset   = ::boost::movelib::make_unique<MyStdMultiSet>();

   MyBoostSet &boostset = *pboostset;
   MyStdSet   &stdset   = *pstdset;
   MyBoostMultiSet &boostmultiset = *pboostmultiset;
   MyStdMultiSet   &stdmultiset   = *pstdmultiset;

   //Test construction from a range
   {  //Set(beg, end, compare)
      IntType aux_vect[50];
      for(int i = 0; i < 50; ++i){
         aux_vect[i] = i/2;
      }
      int aux_vect2[50];
      for(int i = 0; i < 50; ++i){
         aux_vect2[i] = i/2;
      }
      IntType aux_vect3[50];
      for(int i = 0; i < 50; ++i){
         aux_vect3[i] = i/2;
      }
      ::boost::movelib::unique_ptr<MyBoostSet> const pboostset2 = ::boost::movelib::make_unique<MyBoostSet>
         (boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0]+50), typename MyBoostSet::key_compare());
      ::boost::movelib::unique_ptr<MyStdSet> const pstdset2 = ::boost::movelib::make_unique<MyStdSet>(&aux_vect2[0], &aux_vect2[0]+50);
      if(!test::CheckEqualContainers(*pboostset2, *pstdset2)) return 1;
      ::boost::movelib::unique_ptr<MyBoostMultiSet> const pboostmultiset2 = ::boost::movelib::make_unique<MyBoostMultiSet>
         (boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0]+50), typename MyBoostMultiSet::key_compare());
      ::boost::movelib::unique_ptr<MyStdMultiSet> const pstdmultiset2 = ::boost::movelib::make_unique<MyStdMultiSet>(&aux_vect2[0], &aux_vect2[0]+50);
      if(!test::CheckEqualContainers(*pboostmultiset2, *pstdmultiset2)) return 1;
   }
   {  //Set(beg, end, alloc)
      IntType aux_vect[50];
      for(int i = 0; i < 50; ++i){
         aux_vect[i] = i/2;
      }
      int aux_vect2[50];
      for(int i = 0; i < 50; ++i){
         aux_vect2[i] = i/2;
      }
      IntType aux_vect3[50];
      for(int i = 0; i < 50; ++i){
         aux_vect3[i] = i/2;
      }
      ::boost::movelib::unique_ptr<MyBoostSet> const pboostset2 = ::boost::movelib::make_unique<MyBoostSet>
         (boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0]+50), typename MyBoostSet::allocator_type());
      ::boost::movelib::unique_ptr<MyStdSet> const pstdset2 = ::boost::movelib::make_unique<MyStdSet>(&aux_vect2[0], &aux_vect2[0]+50);
      if(!test::CheckEqualContainers(*pboostset2, *pstdset2)) return 1;
      ::boost::movelib::unique_ptr<MyBoostMultiSet> const pboostmultiset2 = ::boost::movelib::make_unique<MyBoostMultiSet>
         (boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0]+50), typename MyBoostMultiSet::allocator_type());
      ::boost::movelib::unique_ptr<MyStdMultiSet> const pstdmultiset2 = ::boost::movelib::make_unique<MyStdMultiSet>(&aux_vect2[0], &aux_vect2[0]+50);
      if(!test::CheckEqualContainers(*pboostmultiset2, *pstdmultiset2)) return 1;
   }
   {
      IntType aux_vect[50];
      for(int i = 0; i < 50; ++i){
         aux_vect[i] = i/2;
      }
      int aux_vect2[50];
      for(int i = 0; i < 50; ++i){
         aux_vect2[i] = i/2;
      }
      IntType aux_vect3[50];
      for(int i = 0; i < 50; ++i){
         aux_vect3[i] = i/2;
      }

      ::boost::movelib::unique_ptr<MyBoostSet> const pboostset2 = ::boost::movelib::make_unique<MyBoostSet>
            ( boost::make_move_iterator(&aux_vect[0])
            , boost::make_move_iterator(aux_vect + 50));
      ::boost::movelib::unique_ptr<MyStdSet>  const pstdset2 = ::boost::movelib::make_unique<MyStdSet>
            (&aux_vect2[0], &aux_vect2[0] + 50);
      ::boost::movelib::unique_ptr<MyBoostMultiSet> const pboostmultiset2 = ::boost::movelib::make_unique<MyBoostMultiSet>
            ( boost::make_move_iterator(&aux_vect3[0])
            , boost::make_move_iterator(aux_vect3 + 50));
      ::boost::movelib::unique_ptr<MyStdMultiSet>   const pstdmultiset2   = ::boost::movelib::make_unique<MyStdMultiSet>
            (&aux_vect2[0], &aux_vect2[0] + 50);

      MyBoostSet &boostset2 = *pboostset2;
      MyStdSet   &stdset2   = *pstdset2;
      MyBoostMultiSet &boostmultiset2 = *pboostmultiset2;
      MyStdMultiSet   &stdmultiset2   = *pstdmultiset2;

      if(!CheckEqualContainers(boostset2, stdset2)){
         std::cout << "Error in construct<MyBoostSet>(MyBoostSet2)" << std::endl;
         return 1;
      }
      if(!CheckEqualContainers(boostmultiset2, stdmultiset2)){
         std::cout << "Error in construct<MyBoostMultiSet>(MyBoostMultiSet2)" << std::endl;
         return 1;
      }

      //ordered range insertion
      for(int i = 0; i < 50; ++i){
         aux_vect[i] = i;
      }

      for(int i = 0; i < 50; ++i){
         aux_vect2[i] = i;
      }

      for(int i = 0; i < 50; ++i){
         aux_vect3[i] = i;
      }

      //some comparison operators
      if(!(boostset2 == boostset2))
         return 1;
      if(boostset2 != boostset2)
         return 1;
      if(boostset2 < boostset2)
         return 1;
      if(boostset2 > boostset2)
         return 1;
      if(!(boostset2 <= boostset2))
         return 1;
      if(!(boostset2 >= boostset2))
         return 1;

      ::boost::movelib::unique_ptr<MyBoostSet> const pboostset3 = ::boost::movelib::make_unique<MyBoostSet>
            ( ordered_unique_range
            , boost::make_move_iterator(&aux_vect[0])
            , boost::make_move_iterator(&aux_vect[0] + 50));
      ::boost::movelib::unique_ptr<MyStdSet>   const pstdset3 = ::boost::movelib::make_unique<MyStdSet>
            (&aux_vect2[0], &aux_vect2[0] + 50);
      ::boost::movelib::unique_ptr<MyBoostMultiSet> const pboostmultiset3 = ::boost::movelib::make_unique<MyBoostMultiSet>
            ( ordered_range
            , boost::make_move_iterator(&aux_vect3[0])
            , boost::make_move_iterator(aux_vect3 + 50));
      ::boost::movelib::unique_ptr<MyStdMultiSet>   const pstdmultiset3   = ::boost::movelib::make_unique<MyStdMultiSet>
            (&aux_vect2[0], &aux_vect2[0] + 50);

      MyBoostSet &boostset3 = *pboostset3;
      MyStdSet   &stdset3   = *pstdset3;
      MyBoostMultiSet &boostmultiset3 = *pboostmultiset3;
      MyStdMultiSet   &stdmultiset3   = *pstdmultiset3;

      if(!CheckEqualContainers(boostset3, stdset3)){
         std::cout << "Error in construct<MyBoostSet>(MyBoostSet3)" << std::endl;
         return 1;
      }
      if(!CheckEqualContainers(boostmultiset3, stdmultiset3)){
         std::cout << "Error in construct<MyBoostMultiSet>(MyBoostMultiSet3)" << std::endl;
         return 1;
      }
   }

   for(int i = 0; i < MaxElem; ++i){
      IntType move_me(i);
      boostset.insert(boost::move(move_me));
      stdset.insert(i);
      boostset.insert(IntType(i));
      stdset.insert(i);
      IntType move_me2(i);
      boostmultiset.insert(boost::move(move_me2));
      stdmultiset.insert(i);
      boostmultiset.insert(IntType(i));
      stdmultiset.insert(i);
   }

   if(!CheckEqualContainers(boostset, stdset)){
      std::cout << "Error in boostset.insert(boost::move(move_me)" << std::endl;
      return 1;
   }

   if(!CheckEqualContainers(boostmultiset, stdmultiset)){
      std::cout << "Error in boostmultiset.insert(boost::move(move_me)" << std::endl;
      return 1;
   }

   typename MyBoostSet::iterator it = boostset.begin();
   typename MyBoostSet::const_iterator cit = it;
   (void)cit;

   boostset.erase(boostset.begin());
   stdset.erase(stdset.begin());
   boostmultiset.erase(boostmultiset.begin());
   stdmultiset.erase(stdmultiset.begin());
   if(!CheckEqualContainers(boostset, stdset)){
      std::cout << "Error in boostset.erase(boostset.begin())" << std::endl;
      return 1;
   }
   if(!CheckEqualContainers(boostmultiset, stdmultiset)){
      std::cout << "Error in boostmultiset.erase(boostmultiset.begin())" << std::endl;
      return 1;
   }

   boostset.erase(boostset.begin());
   stdset.erase(stdset.begin());
   boostmultiset.erase(boostmultiset.begin());
   stdmultiset.erase(stdmultiset.begin());
   if(!CheckEqualContainers(boostset, stdset)){
      std::cout << "Error in boostset.erase(boostset.begin())" << std::endl;
      return 1;
   }
   if(!CheckEqualContainers(boostmultiset, stdmultiset)){
      std::cout << "Error in boostmultiset.erase(boostmultiset.begin())" << std::endl;
      return 1;
   }

   {
      //Swapping test
      MyBoostSet tmpboosteset2;
      MyStdSet tmpstdset2;
      MyBoostMultiSet tmpboostemultiset2;
      MyStdMultiSet tmpstdmultiset2;
      boostset.swap(tmpboosteset2);
      stdset.swap(tmpstdset2);
      boostmultiset.swap(tmpboostemultiset2);
      stdmultiset.swap(tmpstdmultiset2);
      boostset.swap(tmpboosteset2);
      stdset.swap(tmpstdset2);
      boostmultiset.swap(tmpboostemultiset2);
      stdmultiset.swap(tmpstdmultiset2);
      if(!CheckEqualContainers(boostset, stdset)){
         std::cout << "Error in boostset.swap(tmpboosteset2)" << std::endl;
         return 1;
      }
      if(!CheckEqualContainers(boostmultiset, stdmultiset)){
         std::cout << "Error in boostmultiset.swap(tmpboostemultiset2)" << std::endl;
         return 1;
      }
   }

   //move constructor/assignment
   {
      MyBoostSet tmpboosteset2(boost::move(boostset));
      if(!CheckEqualContainers(tmpboosteset2, stdset)){
         std::cout << "Error in boostset move constructor " << std::endl;
         return 1;
      }
      MyBoostMultiSet tmpboostemultiset2(boost::move(boostmultiset));
      if(!CheckEqualContainers(tmpboostemultiset2, stdmultiset)){
         std::cout << "Error in boostmultiset move constructor " << std::endl;
         return 1;
      }

      boostset = boost::move(tmpboosteset2);
      if(!CheckEqualContainers(boostset, stdset)){
         std::cout << "Error in boostset move assignment" << std::endl;
         return 1;
      }
      boostmultiset = boost::move(tmpboostemultiset2);
      if(!CheckEqualContainers(boostmultiset, stdmultiset)){
         std::cout << "Error in boostmultiset move assignment" << std::endl;
         return 1;
      }
   }
   //Insertion from other container
   //Initialize values
   {
      IntType aux_vect[50];
      for(int i = 0; i < 50; ++i){
         aux_vect[i] = -1;
      }
      int aux_vect2[50];
      for(int i = 0; i < 50; ++i){
         aux_vect2[i] = -1;
      }
      IntType aux_vect3[50];
      for(int i = 0; i < 50; ++i){
         aux_vect3[i] = -1;
      }

      boostset.insert(boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0] + 50));
      stdset.insert(&aux_vect2[0], &aux_vect2[0] + 50);
      if(!CheckEqualContainers(boostset, stdset)){
         std::cout << "Error in boostset.insert(boost::make_move_iterator(&aux_vect3[0])..." << std::endl;
         return 1;
      }
      boostmultiset.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(aux_vect3 + 50));
      stdmultiset.insert(&aux_vect2[0], &aux_vect2[0] + 50);
      if(!CheckEqualContainers(boostmultiset, stdmultiset)){
         std::cout << "Error in boostmultiset.insert(boost::make_move_iterator(&aux_vect3[0]), ..." << std::endl;
         return 1;
      }

      for(int i = 0, j = static_cast<int>(boostset.size()); i < j; ++i){
         IntType erase_me(i);
         boostset.erase(erase_me);
         stdset.erase(i);
         boostmultiset.erase(erase_me);
         stdmultiset.erase(i);
         if(!CheckEqualContainers(boostset, stdset)){
            std::cout << "Error in boostset.erase(erase_me)" << boostset.size() << " " << stdset.size() << std::endl;
            return 1;
         }
         if(!CheckEqualContainers(boostmultiset, stdmultiset)){
            std::cout << "Error in boostmultiset.erase(erase_me)" << std::endl;
            return 1;
         }
      }
   }
   {
      IntType aux_vect[50];
      for(int i = 0; i < 50; ++i){
         aux_vect[i] = -1;
      }
      int aux_vect2[50];
      for(int i = 0; i < 50; ++i){
         aux_vect2[i] = -1;
      }
      IntType aux_vect3[50];
      for(int i = 0; i < 50; ++i){
         aux_vect3[i] = -1;
      }

      IntType aux_vect4[50];
      for(int i = 0; i < 50; ++i){
         aux_vect4[i] = -1;
      }

      IntType aux_vect5[50];
      for(int i = 0; i < 50; ++i){
         aux_vect5[i] = -1;
      }

      boostset.insert(boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0] + 50));
      boostset.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0] + 50));
      stdset.insert(&aux_vect2[0], &aux_vect2[0] + 50);
      stdset.insert(&aux_vect2[0], &aux_vect2[0] + 50);
      if(!CheckEqualContainers(boostset, stdset)){
         std::cout << "Error in boostset.insert(boost::make_move_iterator(&aux_vect3[0])..." << std::endl;
         return 1;
      }
      boostmultiset.insert(boost::make_move_iterator(&aux_vect4[0]), boost::make_move_iterator(&aux_vect4[0] + 50));
      boostmultiset.insert(boost::make_move_iterator(&aux_vect5[0]), boost::make_move_iterator(&aux_vect5[0] + 50));
      stdmultiset.insert(&aux_vect2[0], &aux_vect2[0] + 50);
      stdmultiset.insert(&aux_vect2[0], &aux_vect2[0] + 50);
      if(!CheckEqualContainers(boostmultiset, stdmultiset)){
         std::cout << "Error in boostmultiset.insert(boost::make_move_iterator(&aux_vect5[0])..." << std::endl;
         return 1;
      }

      boostset.erase(*boostset.begin());
      stdset.erase(*stdset.begin());
      if(!CheckEqualContainers(boostset, stdset)){
         std::cout << "Error in boostset.erase(*boostset.begin())" << std::endl;
         return 1;
      }
      boostmultiset.erase(*boostmultiset.begin());
      stdmultiset.erase(*stdmultiset.begin());
      if(!CheckEqualContainers(boostmultiset, stdmultiset)){
         std::cout << "Error in boostmultiset.erase(*boostmultiset.begin())" << std::endl;
         return 1;
      }
   }

   for(int i = 0; i < MaxElem; ++i){
      IntType move_me(i);
      boostset.insert(boost::move(move_me));
      stdset.insert(i);
      IntType move_me2(i);
      boostmultiset.insert(boost::move(move_me2));
      stdmultiset.insert(i);
   }

   if(!CheckEqualContainers(boostset, stdset)){
      std::cout << "Error in boostset.insert(boost::move(move_me)) try 2" << std::endl;
      return 1;
   }
   if(!CheckEqualContainers(boostmultiset, stdmultiset)){
      std::cout << "Error in boostmultiset.insert(boost::move(move_me2)) try 2" << std::endl;
      return 1;
   }

   for(int i = 0; i < MaxElem; ++i){
      {
         IntType move_me(i);
         boostset.insert(boostset.begin(), boost::move(move_me));
         stdset.insert(stdset.begin(), i);
         //PrintContainers(boostset, stdset);
         IntType move_me2(i);
         boostmultiset.insert(boostmultiset.begin(), boost::move(move_me2));
         stdmultiset.insert(stdmultiset.begin(), i);
         //PrintContainers(boostmultiset, stdmultiset);
         if(!CheckEqualContainers(boostset, stdset)){
            std::cout << "Error in boostset.insert(boostset.begin(), boost::move(move_me))" << std::endl;
            return 1;
         }
         if(!CheckEqualContainers(boostmultiset, stdmultiset)){
            std::cout << "Error in boostmultiset.insert(boostmultiset.begin(), boost::move(move_me2))" << std::endl;
            return 1;
         }

         IntType move_me3(i);
         boostset.insert(boostset.end(), boost::move(move_me3));
         stdset.insert(stdset.end(), i);
         IntType move_me4(i);
         boostmultiset.insert(boostmultiset.end(), boost::move(move_me4));
         stdmultiset.insert(stdmultiset.end(), i);
         if(!CheckEqualContainers(boostset, stdset)){
            std::cout << "Error in boostset.insert(boostset.end(), boost::move(move_me3))" << std::endl;
            return 1;
         }
         if(!CheckEqualContainers(boostmultiset, stdmultiset)){
            std::cout << "Error in boostmultiset.insert(boostmultiset.end(), boost::move(move_me4))" << std::endl;
            return 1;
         }
      }
      {
         IntType move_me(i);
         boostset.insert(boostset.upper_bound(move_me), boost::move(move_me));
         stdset.insert(stdset.upper_bound(i), i);
         //PrintContainers(boostset, stdset);
         IntType move_me2(i);
         boostmultiset.insert(boostmultiset.upper_bound(move_me2), boost::move(move_me2));
         stdmultiset.insert(stdmultiset.upper_bound(i), i);
         //PrintContainers(boostmultiset, stdmultiset);
         if(!CheckEqualContainers(boostset, stdset)){
            std::cout << "Error in boostset.insert(boostset.upper_bound(move_me), boost::move(move_me))" << std::endl;
            return 1;
         }
         if(!CheckEqualContainers(boostmultiset, stdmultiset)){
            std::cout << "Error in boostmultiset.insert(boostmultiset.upper_bound(move_me2), boost::move(move_me2))" << std::endl;
            return 1;
         }

      }
      {
         IntType move_me(i);
         IntType move_me2(i);
         boostset.insert(boostset.lower_bound(move_me), boost::move(move_me2));
         stdset.insert(stdset.lower_bound(i), i);
         //PrintContainers(boostset, stdset);
         move_me2 = i;
         boostmultiset.insert(boostmultiset.lower_bound(move_me2), boost::move(move_me2));
         stdmultiset.insert(stdmultiset.lower_bound(i), i);
         //PrintContainers(boostmultiset, stdmultiset);
         if(!CheckEqualContainers(boostset, stdset)){
            std::cout << "Error in boostset.insert(boostset.lower_bound(move_me), boost::move(move_me2))" << std::endl;
            return 1;
         }
         if(!CheckEqualContainers(boostmultiset, stdmultiset)){
            std::cout << "Error in boostmultiset.insert(boostmultiset.lower_bound(move_me2), boost::move(move_me2))" << std::endl;
            return 1;
         }
         set_test_rebalanceable(boostset
            , dtl::bool_<has_member_function_callable_with_rebalance<MyBoostSet>::value>());
         if(!CheckEqualContainers(boostset, stdset)){
            std::cout << "Error in boostset.rebalance()" << std::endl;
            return 1;
         }
         set_test_rebalanceable(boostmultiset
            , dtl::bool_<has_member_function_callable_with_rebalance<MyBoostMultiSet>::value>());
         if(!CheckEqualContainers(boostmultiset, stdmultiset)){
            std::cout << "Error in boostmultiset.rebalance()" << std::endl;
            return 1;
         }
      }
   }

   //Compare count with std containers
   for(int i = 0; i < MaxElem; ++i){
      IntType k(i);
      if(boostset.count(k) != stdset.count(i)){
         return -1;
      }

      if(boostset.contains(k) != (stdset.find(i) != stdset.end())){
         return -1;
      }

      if(boostmultiset.count(k) != stdmultiset.count(i)){
         return -1;
      }

      if(boostmultiset.contains(k) != (stdmultiset.find(i) != stdmultiset.end())){
         return -1;
      }
   }

   //Compare find/lower_bound/upper_bound in set
   {
      typename MyBoostSet::iterator bs_b = boostset.begin();
      typename MyBoostSet::iterator bs_e = boostset.end();
      typename MyStdSet::iterator ss_b   = stdset.begin();

      while(bs_b != bs_e){
         typename MyBoostSet::iterator bs_i;
         typename MyStdSet::iterator ss_i;
         //find
         bs_i = boostset.find(*bs_b);
         ss_i = stdset.find(*ss_b);
         if(!CheckEqualIt(bs_i, ss_i, boostset, stdset)){
            return -1;
         }
         //lower bound
         bs_i = boostset.lower_bound(*bs_b);
         ss_i = stdset.lower_bound(*ss_b);
         if(!CheckEqualIt(bs_i, ss_i, boostset, stdset)){
            return -1;
         }
         //upper bound
         bs_i = boostset.upper_bound(*bs_b);
         ss_i = stdset.upper_bound(*ss_b);
         if(!CheckEqualIt(bs_i, ss_i, boostset, stdset)){
            return -1;
         }
         //equal range
         std::pair<typename MyBoostSet::iterator
                  ,typename MyBoostSet::iterator> bs_ip;
         std::pair<typename MyStdSet::iterator
                  ,typename MyStdSet::iterator>   ss_ip;
         bs_ip = boostset.equal_range(*bs_b);
         ss_ip = stdset.equal_range(*ss_b);
         if(!CheckEqualIt(bs_ip.first, ss_ip.first, boostset, stdset)){
            return -1;
         }
         if(!CheckEqualIt(bs_ip.second, ss_ip.second, boostset, stdset)){
            return -1;
         }
         ++bs_b;
         ++ss_b;
      }
   }
   //Compare find/lower_bound/upper_bound in multiset
   {
      typename MyBoostMultiSet::iterator bm_b = boostmultiset.begin();
      typename MyBoostMultiSet::iterator bm_e = boostmultiset.end();
      typename MyStdMultiSet::iterator sm_b   = stdmultiset.begin();

      while(bm_b != bm_e){
         typename MyBoostMultiSet::iterator bm_i;
         typename MyStdMultiSet::iterator sm_i;
         //find
         bm_i = boostmultiset.find(*bm_b);
         sm_i = stdmultiset.find(*sm_b);
         if(!CheckEqualIt(bm_i, sm_i, boostmultiset, stdmultiset)){
            return -1;
         }
         //lower bound
         bm_i = boostmultiset.lower_bound(*bm_b);
         sm_i = stdmultiset.lower_bound(*sm_b);
         if(!CheckEqualIt(bm_i, sm_i, boostmultiset, stdmultiset)){
            return -1;
         }
         //upper bound
         bm_i = boostmultiset.upper_bound(*bm_b);
         sm_i = stdmultiset.upper_bound(*sm_b);
         if(!CheckEqualIt(bm_i, sm_i, boostmultiset, stdmultiset)){
            return -1;
         }
         //equal range
         std::pair<typename MyBoostMultiSet::iterator
                  ,typename MyBoostMultiSet::iterator> bm_ip;
         std::pair<typename MyStdMultiSet::iterator
                  ,typename MyStdMultiSet::iterator>   sm_ip;
         bm_ip = boostmultiset.equal_range(*bm_b);
         sm_ip = stdmultiset.equal_range(*sm_b);
         if(!CheckEqualIt(bm_ip.first, sm_ip.first, boostmultiset, stdmultiset)){
            return -1;
         }
         if(!CheckEqualIt(bm_ip.second, sm_ip.second, boostmultiset, stdmultiset)){
            return -1;
         }
         ++bm_b;
         ++sm_b;
      }
   }

   //Now do count exercise
   boostset.erase(boostset.begin(), boostset.end());
   boostmultiset.erase(boostmultiset.begin(), boostmultiset.end());
   boostset.clear();
   boostmultiset.clear();

   for(int j = 0; j < 3; ++j)
   for(int i = 0; i < 100; ++i){
      IntType move_me(i);
      boostset.insert(boost::move(move_me));
      IntType move_me2(i);
      boostmultiset.insert(boost::move(move_me2));
      IntType count_me(i);
      if(boostset.count(count_me) != typename MyBoostMultiSet::size_type(1)){
         std::cout << "Error in boostset.count(count_me)" << std::endl;
         return 1;
      }
      if(boostmultiset.count(count_me) != typename MyBoostMultiSet::size_type(j+1)){
         std::cout << "Error in boostmultiset.count(count_me)" << std::endl;
         return 1;
      }
   }

   {  //merge
      ::boost::movelib::unique_ptr<MyBoostSet> const pboostset2 = ::boost::movelib::make_unique<MyBoostSet>();
      ::boost::movelib::unique_ptr<MyBoostMultiSet> const pboostmultiset2 = ::boost::movelib::make_unique<MyBoostMultiSet>();

      MyBoostSet &boostset2 = *pboostset2;
      MyBoostMultiSet &boostmultiset2 = *pboostmultiset2;

      boostset.clear();
      boostset2.clear();
      boostmultiset.clear();
      boostmultiset2.clear();
      stdset.clear();
      stdmultiset.clear();

      {
         IntType aux_vect[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            aux_vect[i] = i;
         }

         IntType aux_vect2[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            aux_vect2[i] = MaxElem/2+i;
         }
         IntType aux_vect3[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            aux_vect3[i] = MaxElem*2/2+i;
         }
         boostset.insert(boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0] + MaxElem));
         boostset2.insert(boost::make_move_iterator(&aux_vect2[0]), boost::make_move_iterator(&aux_vect2[0] + MaxElem));
         boostmultiset2.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      }
      for(int i = 0; i < MaxElem; ++i){
         stdset.insert(i);
      }
      for(int i = 0; i < MaxElem; ++i){
         stdset.insert(MaxElem/2+i);
      }

      boostset.merge(boost::move(boostset2));
      if(!CheckEqualContainers(boostset, stdset)) return 1;

      for(int i = 0; i < MaxElem; ++i){
         stdset.insert(MaxElem*2/2+i);
      }

      boostset.merge(boost::move(boostmultiset2));
      if(!CheckEqualContainers(boostset, stdset)) return 1;

      boostset.clear();
      boostset2.clear();
      boostmultiset.clear();
      boostmultiset2.clear();
      stdset.clear();
      stdmultiset.clear();
      {
         IntType aux_vect[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            aux_vect[i] = i;
         }

         IntType aux_vect2[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            aux_vect2[i] = MaxElem/2+i;
         }
         IntType aux_vect3[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            aux_vect3[i] = MaxElem*2/2+i;
         }
         boostmultiset.insert(boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0] + MaxElem));
         boostmultiset2.insert(boost::make_move_iterator(&aux_vect2[0]), boost::make_move_iterator(&aux_vect2[0] + MaxElem));
         boostset2.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      }
      for(int i = 0; i < MaxElem; ++i){
         stdmultiset.insert(i);
      }
      for(int i = 0; i < MaxElem; ++i){
         stdmultiset.insert(MaxElem/2+i);
      }
      boostmultiset.merge(boost::move(boostmultiset2));
      if(!CheckEqualContainers(boostmultiset, stdmultiset)) return 1;

      for(int i = 0; i < MaxElem; ++i){
         stdmultiset.insert(MaxElem*2/2+i);
      }

      boostmultiset.merge(boost::move(boostset2));
      if(!CheckEqualContainers(boostmultiset, stdmultiset)) return 1;
   }

   {  //erase_if
      boostset.clear();
      boostmultiset.clear();
      stdset.clear();
      stdmultiset.clear();

      {
         IntType aux_vect[(std::size_t)MaxElem];
         IntType aux_vect2[(std::size_t)MaxElem];
         IntType aux_vect3[(std::size_t)MaxElem];

         for(int i = 0; i < MaxElem; ++i){
            aux_vect[i] = i;
            aux_vect2[i] = i;
            aux_vect3[i] = i;
         }

         boostset.     insert(boost::make_move_iterator(&aux_vect[0]),  boost::make_move_iterator(&aux_vect[0]  + MaxElem));
         boostmultiset.insert(boost::make_move_iterator(&aux_vect2[0]), boost::make_move_iterator(&aux_vect2[0] + MaxElem));
         boostmultiset.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      }

      for(int i = 0; i < MaxElem; ++i){
         stdset.insert(i);
         stdmultiset.insert(i);
         stdmultiset.insert(i);
      }

      for(int i = 0; i < MaxElem; ++i) {
         //erase_if
         const int setkey = (i + MaxElem/2) % MaxElem;
         if (1 != erase_if(boostset, equal_to_value<int>(setkey)))
            return 1;
         if (0 != erase_if(boostset, equal_to_value<int>(setkey)))
            return 1;
         stdset.erase(setkey);
         if(!test::CheckEqualContainers(boostset, stdset)) return false;

         //erase_if
         if (2 != erase_if(boostmultiset, equal_to_value<int>(setkey)))
            return 1;
         if (0 != erase_if(boostmultiset, equal_to_value<int>(setkey)))
            return 1;
         stdmultiset.erase(setkey);
         if(!test::CheckEqualContainers(boostmultiset, stdmultiset)) return false;
      }
   }

   if(set_test_copyable<MyBoostSet, MyStdSet, MyBoostMultiSet, MyStdMultiSet>
      (dtl::bool_<boost::container::test::is_copyable<IntType>::value>())){
      return 1;
   }

   if(set_single_pass_test<MyBoostSet, MyStdSet, MyBoostMultiSet, MyStdMultiSet>())
      return 1;

   return 0;
}

template<typename SetType>
bool test_set_methods_with_initializer_list_as_argument_for()
{
#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   std::initializer_list<int> il = { 1, 2, 3, 4, 5, 5 };
   std::initializer_list<int> ilu = { 1, 2, 3, 4, 5 };
   SetType expected(il.begin(), il.end());
   SetType expectedu(ilu.begin(), ilu.end());
   {
      SetType sil((il));
      if (sil != expected)
         return false;

      SetType sila(il, typename SetType::allocator_type());
      if (sila != expected)
         return false;

      SetType silca(il, typename SetType::key_compare(), typename SetType::allocator_type());
      if (silca != expected)
         return false;

      SetType sil_ordered(ordered_unique_range, ilu);
      if (sil_ordered != expectedu)
         return false;

      SetType sil_assign = { 99, 100, 101, 102, 103, 104, 105 };
      sil_assign = il;
      if (sil_assign != expected)
         return false;
   }
   {
      SetType sil;
      sil.insert(il);
      if (sil != expected)
         return false;
   }
   return true;
#else
   return true;
#endif
}

template<typename SetType, typename MultisetType>
bool instantiate_constructors()
{
   {
      typedef typename SetType::value_type value_type;
      typename SetType::key_compare comp;
      typename SetType::allocator_type a;
      value_type value;
      {
         SetType s0;
         SetType s1(comp);
         SetType s2(a);
         SetType s3(comp, a);
      }
      {
         SetType s0(&value, &value);
         SetType s1(&value, &value ,comp);
         SetType s2(&value, &value ,a);
         SetType s3(&value, &value ,comp, a);
      }
      #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
      {
         SetType s0({ 0 });
         SetType s1({ 0 },comp);
         SetType s2({ 0 },a);
         SetType s3({ 0 },comp, a);
      }
      {
         std::initializer_list<value_type> il{0};
         SetType s0(ordered_unique_range, il);
         SetType s1(ordered_unique_range, il,comp);
         SetType s3(ordered_unique_range, il,comp, a);
      }
      #endif
      {
         SetType s0(ordered_unique_range, &value, &value);
         SetType s1(ordered_unique_range, &value, &value ,comp);
         SetType s2(ordered_unique_range, &value, &value ,comp, a);
      }
   }

   {
      typedef typename MultisetType::value_type value_type;
      typename MultisetType::key_compare comp;
      typename MultisetType::allocator_type a;
      value_type value;
      {
         MultisetType s0;
         MultisetType s1(comp);
         MultisetType s2(a);
         MultisetType s3(comp, a);
      }
      {
         MultisetType s0(&value, &value);
         MultisetType s1(&value, &value ,comp);
         MultisetType s2(&value, &value ,a);
         MultisetType s3(&value, &value ,comp, a);
      }
      #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
      {
         MultisetType s0({ 0 });
         MultisetType s1({ 0 },comp);
         MultisetType s2({ 0 },a);
         MultisetType s3({ 0 },comp, a);
      }
      {
         std::initializer_list<value_type>il{0};
         MultisetType s0(ordered_range, il);
         MultisetType s1(ordered_range, il,comp);
         MultisetType s3(ordered_range, il,comp, a);
      }
      #endif
      {
         MultisetType s0(ordered_range, &value, &value);
         MultisetType s1(ordered_range, &value, &value ,comp);
         MultisetType s2(ordered_range, &value, &value ,comp, a);
      }
   }
   return true;
}







template<typename IntSetType, typename IntMultisetType>
bool test_heterogeneous_lookup()
{
   typedef IntSetType set_t;
   typedef IntMultisetType mset_t;
   
   set_t set1;
   mset_t mset1;

   const set_t &cset1 = set1;
   const mset_t &cmset1 = mset1;

   set1.insert(1);
   set1.insert(1);
   set1.insert(2);
   set1.insert(2);
   set1.insert(3);

   mset1.insert(1);
   mset1.insert(1);
   mset1.insert(2);
   mset1.insert(2);
   mset1.insert(3);

   const test::non_copymovable_int find_me(2);

   //find
   if(*set1.find(find_me) != 2)
      return false;
   if(*cset1.find(find_me) != 2)
      return false;
   if(*mset1.find(find_me) != 2)
      return false;
   if(*cmset1.find(find_me) != 2)
      return false;

   //count
   if(set1.count(find_me) != 1)
      return false;
   if(cset1.count(find_me) != 1)
      return false;
   if(mset1.count(find_me) != 2)
      return false;
   if(cmset1.count(find_me) != 2)
      return false;

   //contains
   if(!set1.contains(find_me))
      return false;
   if(!cset1.contains(find_me))
      return false;
   if(!mset1.contains(find_me))
      return false;
   if(!cmset1.contains(find_me))
      return false;

   //lower_bound
   if(*set1.lower_bound(find_me) != 2)
      return false;
   if(*cset1.lower_bound(find_me) != 2)
      return false;
   if(*mset1.lower_bound(find_me) != 2)
      return false;
   if(*cmset1.lower_bound(find_me) != 2)
      return false;

   //upper_bound
   if(*set1.upper_bound(find_me) != 3)
      return false;
   if(*cset1.upper_bound(find_me) != 3)
      return false;
   if(*mset1.upper_bound(find_me) != 3)
      return false;
   if(*cmset1.upper_bound(find_me) != 3)
      return false;

   //equal_range
   if(*set1.equal_range(find_me).first != 2)
      return false;
   if(*cset1.equal_range(find_me).second != 3)
      return false;
   if(*mset1.equal_range(find_me).first != 2)
      return false;
   if(*cmset1.equal_range(find_me).second != 3)
      return false;

   //erase
   if (set1.erase(find_me) != 1)
      return false;
   if (set1.erase(find_me) != 0)
      return false;
   if (mset1.erase(find_me) != 2)
      return false;
   if (mset1.erase(find_me) != 0)
      return false;

   return true;
}

template<typename MovableIntSetType>
bool test_heterogeneous_insert()
{
   typedef MovableIntSetType set_t;

   set_t set1;
   const set_t &cset1 = set1;

   //insert
   if(!set1.insert(1).second)
      return false;
   if (cset1.find(1) == cset1.end())
      return false;
   if(set1.insert(1).second)
      return false;
   if (cset1.find(1) == cset1.end())
      return false;

   //insert with hint
   if(set1.find(2) != set1.end())
      return false;
   typename set_t::iterator i = set1.insert(set1.begin(), 2);
   if(i != set1.insert(set1.end(), 2))
      return false;
   if (cset1.find(2) == cset1.end())
      return false;
   return true;
}







inline bool set_function_pointer_less(int a, int b)
{  return a < b;  }

inline bool set_function_pointer_greater(int a, int b)
{  return b < a;  }

//Tests a set or multiset whose key_compare is bool(*)(int, int)
template<typename SetType>
bool test_function_pointer_comparator_for()
{
   typedef SetType set_t;
   set_t s(&set_function_pointer_less);
   if(s.key_comp() != &set_function_pointer_less)
      return false;
   s.insert(3);
   s.insert(1);
   s.insert(2);
   if(*s.begin() != 1 || s.find(2) == s.end() || s.count(2) != 1u)
      return false;
   if(!s.value_comp()(1, 2) || s.value_comp()(2, 1))
      return false;

   set_t g(&set_function_pointer_greater);
   g.insert(5);
   g.insert(7);
   if(*g.begin() != 7)
      return false;

   //Copy and move construction and assignment
   {
      set_t c(s);
      if(c.key_comp() != &set_function_pointer_less || c.size() != 3u || *c.begin() != 1)
         return false;
      c = g;
      if(c.key_comp() != &set_function_pointer_greater || c.size() != 2u || *c.begin() != 7)
         return false;
      set_t m(boost::move(c));
      if(m.key_comp() != &set_function_pointer_greater || *m.begin() != 7)
         return false;
      c = boost::move(m);
      if(c.key_comp() != &set_function_pointer_greater || *c.begin() != 7)
         return false;
      //Inserted elements are ordered with the propagated comparator
      c.insert(6);
      c.insert(8);
      if(*c.begin() != 8 || *(--c.end()) != 5)
         return false;
      set_t ca(g, g.get_allocator());
      if(ca.key_comp() != &set_function_pointer_greater || *ca.begin() != 7)
         return false;
      set_t ma(boost::move(ca), g.get_allocator());
      if(ma.key_comp() != &set_function_pointer_greater || *ma.begin() != 7)
         return false;
      const int values[] = { 2, 9, 4 };
      set_t r(&values[0], &values[0] + 3, &set_function_pointer_greater);
      if(r.key_comp() != &set_function_pointer_greater || *r.begin() != 9)
         return false;
   }

   //Swap
   s.swap(g);
   if(s.key_comp() != &set_function_pointer_greater || *s.begin() != 7)
      return false;
   if(g.key_comp() != &set_function_pointer_less || *g.begin() != 1)
      return false;

   //Merge from a container with the same comparator type but a different ordering
   g.merge(s);
   if(g.size() != 5u || !s.empty())
      return false;
   if(g.key_comp() != &set_function_pointer_less || s.key_comp() != &set_function_pointer_greater)
      return false;
   int prev = 0;
   for(typename set_t::iterator it = g.begin(), itend = g.end(); it != itend; ++it){
      if(*it <= prev)
         return false;
      prev = *it;
   }
   return true;
}

template<typename SetType, typename MultisetType>
bool test_function_pointer_comparator()
{
   return test_function_pointer_comparator_for<SetType>()
       && test_function_pointer_comparator_for<MultisetType>();
}

//Replaces the comparator of a set type (set has four template parameters,
//flat_set three)
template<class Container, class NewCompare>
struct rebind_set_compare;

template<template<class, class, class> class Cont, class K, class C, class A, class N>
struct rebind_set_compare<Cont<K, C, A>, N>
{  typedef Cont<K, N, A> type;  };

template<template<class, class, class, class> class Cont, class K, class C, class A, class O, class N>
struct rebind_set_compare<Cont<K, C, A, O>, N>
{  typedef Cont<K, N, A, O> type;  };

//The comparator in the default and move constructors and in the move assignment:
//the destination gets the comparator of the source, the moved-from container
//keeps a valid one, the exceptions of the comparator reach the caller and, with
//a comparator that does not throw, the operations are noexcept.
template<class SetType>
bool test_comparator_operations_for()
{
   {
      typedef typename rebind_set_compare<SetType, tagged_less<int> >::type C;
      C s((tagged_less<int>(7)));
      s.insert(typename C::value_type(3));
      s.insert(typename C::value_type(1));
      s.insert(typename C::value_type(2));
      C d(boost::move(s));
      if(s.key_comp().tag_ != 7 || d.key_comp().tag_ != 7 || d.size() != 3u || *d.begin() != 1)
         return false;
      s.insert(typename C::value_type(5));
      s.insert(typename C::value_type(4));
      if(s.size() != 2u || *s.begin() != 4)
         return false;
      C a((tagged_less<int>(3)));
      a.insert(typename C::value_type(9));
      a = boost::move(d);
      if(a.key_comp().tag_ != 7 || a.size() != 3u || *a.begin() != 1)
         return false;
      if(d.key_comp().tag_ != 7)
         return false;
      d.clear();
      d.insert(typename C::value_type(6));
      if(d.size() != 1u)
         return false;
   }
   #ifndef BOOST_NO_EXCEPTIONS
   {
      typedef throwing_less<int> compare_t;
      typedef typename rebind_set_compare<SetType, compare_t>::type C;
      C src, dst;
      src.insert(typename C::value_type(1));
      compare_t::throw_on_copy = true;
      //The default constructor copies the comparator in some containers
      BOOST_CONTAINER_TRY{
         C c;
         (void)c;
      }
      BOOST_CONTAINER_CATCH(int){}
      BOOST_CONTAINER_CATCH_END
      bool thrown_ctor = false, thrown_assign = false;
      BOOST_CONTAINER_TRY{
         C c(boost::move(src));
      }
      BOOST_CONTAINER_CATCH(int){
         thrown_ctor = true;
      }
      BOOST_CONTAINER_CATCH_END
      BOOST_CONTAINER_TRY{
         dst = boost::move(src);
      }
      BOOST_CONTAINER_CATCH(int){
         thrown_assign = true;
      }
      BOOST_CONTAINER_CATCH_END
      compare_t::throw_on_copy = false;
      if(!thrown_ctor || !thrown_assign)
         return false;
   }
   {
      //swap uses the swap function of the comparator, which does not throw,
      //and not its copy or move operations, which throw
      typedef nothrow_swap_less<int> compare_t;
      typedef typename rebind_set_compare<SetType, compare_t>::type C;
      C a((compare_t(1))), b((compare_t(2)));
      a.insert(typename C::value_type(1));
      compare_t::throw_on_copy = true;
      bool thrown = false;
      BOOST_CONTAINER_TRY{
         a.swap(b);
         boost::adl_move_swap(a, b);
         a.swap(b);
      }
      BOOST_CONTAINER_CATCH(int){
         thrown = true;
      }
      BOOST_CONTAINER_CATCH_END
      compare_t::throw_on_copy = false;
      if(thrown || a.key_comp().tag_ != 2 || b.key_comp().tag_ != 1 || !a.empty() || b.size() != 1u)
         return false;
      #if !defined(BOOST_NO_CXX11_NOEXCEPT)
      BOOST_CONTAINER_STATIC_ASSERT(( noexcept(a.swap(b)) ));
      #endif
   }
   #endif
   #if !defined(BOOST_NO_CXX11_NOEXCEPT)
   {
      using boost::move_detail::declval;
      BOOST_CONTAINER_STATIC_ASSERT(( noexcept(SetType()) ));
      BOOST_CONTAINER_STATIC_ASSERT(( noexcept(SetType(declval<SetType>())) ));
      BOOST_CONTAINER_STATIC_ASSERT(( noexcept(declval<SetType&>() = declval<SetType>()) ));
   }
   #endif
   return true;
}

template<class SetType, class MultisetType>
bool test_comparator_operations()
{
   return test_comparator_operations_for<SetType>()
       && test_comparator_operations_for<MultisetType>();
}

}  //namespace test{
}  //namespace container {
}  //namespace boost{

#include <boost/container/detail/config_end.hpp>

#endif
