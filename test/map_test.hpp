////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2006. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_MAP_TEST_HEADER
#define BOOST_CONTAINER_TEST_MAP_TEST_HEADER

#include <boost/container/detail/config_begin.hpp>
#include "check_equal_containers.hpp"
#include "print_container.hpp"
#include "movable_int.hpp"
#include "throwing_utilities.hpp"
#include <boost/move/detail/meta_utils.hpp>   //declval
#include <boost/container/detail/pair.hpp>
#include <boost/container/detail/compare_functors.hpp>
#include <boost/move/iterator.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/move/make_unique.hpp>

#include <boost/intrusive/detail/minimal_pair_header.hpp>      //pair
#include <string>
#include <iostream>

#include <boost/intrusive/detail/mpl.hpp>

namespace boost { namespace container { namespace test {

BOOST_INTRUSIVE_HAS_MEMBER_FUNC_CALLED(has_member_rebalance, rebalance)

}}}

const int MaxElem = 50;

template<class T1, class T2, class T3, class T4>
bool operator ==(std::pair<T1, T2> &p1, std::pair<T1, T2> &p2)
{
   return p1.first == p2.first && p1.second == p2.second;
}

namespace boost{
namespace container {
namespace test{

template<class C>
void map_test_rebalanceable(C &, boost::container::dtl::false_type)
{}

template<class C>
void map_test_rebalanceable(C &c, boost::container::dtl::true_type)
{
   c.rebalance();
}

template<class MyBoostMap
   , class MyStdMap
   , class MyBoostMultiMap
   , class MyStdMultiMap>
int map_move_assignable_only(boost::container::dtl::false_type)
{
   return 0;
}

//Function to check if both sets are equal
template<class MyBoostMap
   , class MyStdMap
   , class MyBoostMultiMap
   , class MyStdMultiMap>
int map_move_assignable_only(boost::container::dtl::true_type)
{
   typedef typename MyBoostMap::key_type  IntType;
   typedef dtl::pair<IntType, IntType>    IntPairType;

   //This is really nasty, but we have no other simple choice
   IntPairType aux_vect[(std::size_t)MaxElem];
   for (int i = 0; i < MaxElem; ++i) {
      IntType i1(i / 2);
      IntType i2(i / 2);
      new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
   }

   typedef typename MyStdMap::value_type StdValueType;
   typedef typename MyStdMap::key_type StdKeyType;
   typedef typename MyStdMap::mapped_type StdMappedType;
   StdValueType aux_vect2[(std::size_t)MaxElem];
   for (int i = 0; i < MaxElem; ++i) {
      new(&aux_vect2[i])StdValueType(StdKeyType(i / 2), StdMappedType(i / 2));
   }

   IntPairType aux_vect3[(std::size_t)MaxElem];
   for (int i = 0; i < MaxElem; ++i) {
      IntType i1(i / 2);
      IntType i2(i / 2);
      new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
   }

   ::boost::movelib::unique_ptr<MyBoostMap> const pboostmap2 = ::boost::movelib::make_unique<MyBoostMap>
      (boost::make_move_iterator(&aux_vect[0])
         , boost::make_move_iterator(&aux_vect[0] + MaxElem));
   ::boost::movelib::unique_ptr<MyStdMap> const pstdmap2 = ::boost::movelib::make_unique<MyStdMap>
      (&aux_vect2[0], &aux_vect2[0] + MaxElem);
   ::boost::movelib::unique_ptr<MyBoostMultiMap> const pboostmultimap2 = ::boost::movelib::make_unique<MyBoostMultiMap>
      (boost::make_move_iterator(&aux_vect3[0])
         , boost::make_move_iterator(&aux_vect3[0] + MaxElem));
   ::boost::movelib::unique_ptr<MyStdMultiMap> const pstdmultimap2 = ::boost::movelib::make_unique<MyStdMultiMap>
      (&aux_vect2[0], &aux_vect2[0] + MaxElem);
   MyBoostMap& boostmap2 = *pboostmap2;
   MyStdMap& stdmap2 = *pstdmap2;

   // fix assignable
   {
      IntType i0(0);
      IntType i1(1);
      boostmap2[::boost::move(i0)] = ::boost::move(i1);
   }
   {
      IntType i1(1);
      boostmap2[IntType(0)] = ::boost::move(i1);
   }
   stdmap2[0] = 1;
   if (!CheckEqualContainers(boostmap2, stdmap2)) return 1;


   return 0;
}

template<class MyBoostMap
        ,class MyStdMap
        ,class MyBoostMultiMap
        ,class MyStdMultiMap>
int map_test_copyable(boost::container::dtl::false_type)
{  return 0; }

template<class MyBoostMap
        ,class MyStdMap
        ,class MyBoostMultiMap
        ,class MyStdMultiMap>
int map_test_copyable(boost::container::dtl::true_type)
{
   typedef typename MyBoostMap::key_type     IntType;
   typedef dtl::pair<IntType, IntType>       IntPairType;
   typedef typename MyStdMap::value_type  StdPairType;

   ::boost::movelib::unique_ptr<MyBoostMap> const pboostmap = ::boost::movelib::make_unique<MyBoostMap>();
   ::boost::movelib::unique_ptr<MyStdMap> const pstdmap = ::boost::movelib::make_unique<MyStdMap>();
   ::boost::movelib::unique_ptr<MyBoostMultiMap> const pboostmultimap = ::boost::movelib::make_unique<MyBoostMultiMap>();
   ::boost::movelib::unique_ptr<MyStdMultiMap> const pstdmultimap = ::boost::movelib::make_unique<MyStdMultiMap>();
   MyBoostMap &boostmap = *pboostmap;
   MyStdMap   &stdmap   = *pstdmap;
   MyBoostMultiMap &boostmultimap = *pboostmultimap;
   MyStdMultiMap   &stdmultimap   = *pstdmultimap;

   //Just to test move aware catch conversions
   boostmap.insert(boostmap.cbegin(), boostmap.cend());
   boostmultimap.insert(boostmultimap.cbegin(), boostmultimap.cend());
   boostmap.insert(boostmap.begin(), boostmap.end());
   boostmultimap.insert(boostmultimap.begin(), boostmultimap.end());

   int i;
   for(i = 0; i < MaxElem; ++i){
      {
      IntType i1(i), i2(i);
      IntPairType intpair1(boost::move(i1), boost::move(i2));
      boostmap.insert(boost::move(intpair1));
      stdmap.insert(StdPairType(i, i));
      }
      {
      IntType i1(i), i2(i);
      IntPairType intpair2(boost::move(i1), boost::move(i2));
      boostmultimap.insert(boost::move(intpair2));
      stdmultimap.insert(StdPairType(i, i));
      }
   }
   if(!CheckEqualContainers(boostmap, stdmap)) return 1;
   if(!CheckEqualContainers(boostmultimap, stdmultimap)) return 1;

   boostmap.clear();
   boostmap.clear();
   boostmultimap.clear();
   stdmultimap.clear();

   //Now try from convertible pair
   for(i = 0; i < MaxElem; ++i){
      {
      boostmap.insert(std::pair<signed short, signed short>((signed short)i, (signed short)i));
      stdmap.insert(StdPairType(i, i));
      }
      {
      boostmultimap.insert(std::pair<signed short, signed short>((signed short)i, (signed short)i));
      stdmultimap.insert(StdPairType(i, i));
      }
   }

   if(!CheckEqualContainers(boostmap, stdmap)) return 1;
   if(!CheckEqualContainers(boostmultimap, stdmultimap)) return 1;

   #if !defined BOOST_NO_CXX11_HDR_INITIALIZER_LIST
   boostmap.clear();
   boostmap.clear();
   boostmultimap.clear();
   stdmultimap.clear();
   //Now try from convertible pair
   for(i = 0; i < MaxElem; ++i){
      {
      boostmap.insert({IntType(i), IntType(i)});
      stdmap.insert(StdPairType(i, i));
      }
      {
      boostmultimap.insert({IntType(i), IntType(i)});
      stdmultimap.insert(StdPairType(i, i));
      }
   }
   if(!CheckEqualContainers(boostmap, stdmap)) return 1;
   if(!CheckEqualContainers(boostmultimap, stdmultimap)) return 1;
   #endif   //BOOST_NO_CXX11_HDR_INITIALIZER_LIST
   {
      //Now, test copy constructor
      MyBoostMap boostmapcopy(boostmap);
      MyStdMap stdmapcopy(stdmap);
      MyBoostMultiMap boostmmapcopy(boostmultimap);
      MyStdMultiMap stdmmapcopy(stdmultimap);

      if(!CheckEqualContainers(boostmapcopy, stdmapcopy))
         return 1;
      if(!CheckEqualContainers(boostmmapcopy, stdmmapcopy))
         return 1;

      //And now assignment
      boostmapcopy  = boostmap;
      stdmapcopy  = stdmap;
      boostmmapcopy = boostmultimap;
      stdmmapcopy = stdmultimap;

      if(!CheckEqualContainers(boostmapcopy, stdmapcopy))
         return 1;
      if(!CheckEqualContainers(boostmmapcopy, stdmmapcopy))
         return 1;
   }

   return 0;
}

template<class MyBoostMap
        ,class MyStdMap
        ,class MyBoostMultiMap
        ,class MyStdMultiMap>
int map_test_range()
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;
   typedef typename MyStdMap::value_type StdValueType;
   typedef typename MyStdMap::key_type StdKeyType;
   typedef typename MyStdMap::mapped_type StdMappedType;

   //Test construction from a range
   {
      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i/2);
         IntType i2(i/2);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      StdValueType aux_vect2[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         new(&aux_vect2[i])StdValueType(StdKeyType(i/2), StdMappedType(i/2));
      }

      IntPairType aux_vect3[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i/2);
         IntType i2(i/2);
         new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      ::boost::movelib::unique_ptr<MyBoostMap> const pboostmap = ::boost::movelib::make_unique<MyBoostMap>
               ( boost::make_move_iterator(&aux_vect[0])
               , boost::make_move_iterator(&aux_vect[0] + MaxElem), typename MyBoostMap::key_compare());
      ::boost::movelib::unique_ptr<MyStdMap> const pstdmap = ::boost::movelib::make_unique<MyStdMap>
         (&aux_vect2[0], &aux_vect2[0] + MaxElem, typename MyStdMap::key_compare());
      if(!CheckEqualContainers(*pboostmap, *pstdmap)) return 1;

      ::boost::movelib::unique_ptr<MyBoostMultiMap> const pboostmultimap = ::boost::movelib::make_unique<MyBoostMultiMap>
               ( boost::make_move_iterator(&aux_vect3[0])
               , boost::make_move_iterator(&aux_vect3[0] + MaxElem), typename MyBoostMap::key_compare());
      ::boost::movelib::unique_ptr<MyStdMultiMap> const pstdmultimap = ::boost::movelib::make_unique<MyStdMultiMap>
         (&aux_vect2[0], &aux_vect2[0] + MaxElem, typename MyStdMap::key_compare());
      if(!CheckEqualContainers(*pboostmultimap, *pstdmultimap)) return 1;
   }
   {
      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i/2);
         IntType i2(i/2);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      StdValueType aux_vect2[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         new(&aux_vect2[i])StdValueType(StdKeyType(i/2), StdMappedType(i/2));
      }

      IntPairType aux_vect3[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i/2);
         IntType i2(i/2);
         new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      ::boost::movelib::unique_ptr<MyBoostMap> const pboostmap = ::boost::movelib::make_unique<MyBoostMap>
               ( boost::make_move_iterator(&aux_vect[0])
               , boost::make_move_iterator(&aux_vect[0] + MaxElem));
      ::boost::movelib::unique_ptr<MyStdMap> const pstdmap = ::boost::movelib::make_unique<MyStdMap>
         (&aux_vect2[0], &aux_vect2[0] + MaxElem, typename MyStdMap::key_compare());
      if(!CheckEqualContainers(*pboostmap, *pstdmap)) return 1;

      ::boost::movelib::unique_ptr<MyBoostMultiMap> const pboostmultimap = ::boost::movelib::make_unique<MyBoostMultiMap>
               ( boost::make_move_iterator(&aux_vect3[0])
               , boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      ::boost::movelib::unique_ptr<MyStdMultiMap> const pstdmultimap = ::boost::movelib::make_unique<MyStdMultiMap>
         (&aux_vect2[0], &aux_vect2[0] + MaxElem, typename MyStdMap::key_compare());
      if(!CheckEqualContainers(*pboostmultimap, *pstdmultimap)) return 1;
   }
   return 0;
}


template<class MyBoostMap
        ,class MyStdMap
        ,class MyBoostMultiMap
        ,class MyStdMultiMap>
int map_test_step(MyBoostMap &, MyStdMap &, MyBoostMultiMap &, MyStdMultiMap &)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;

   {
      //This is really nasty, but we have no other simple choice
      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i/2);
         IntType i2(i/2);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      typedef typename MyStdMap::value_type StdValueType;
      typedef typename MyStdMap::key_type StdKeyType;
      typedef typename MyStdMap::mapped_type StdMappedType;
      StdValueType aux_vect2[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         new(&aux_vect2[i])StdValueType(StdKeyType(i/2), StdMappedType(i/2));
      }

      IntPairType aux_vect3[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i/2);
         IntType i2(i/2);
         new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      ::boost::movelib::unique_ptr<MyBoostMap> const pboostmap2 = ::boost::movelib::make_unique<MyBoostMap>
               ( boost::make_move_iterator(&aux_vect[0])
               , boost::make_move_iterator(&aux_vect[0] + MaxElem));
      ::boost::movelib::unique_ptr<MyStdMap> const pstdmap2 = ::boost::movelib::make_unique<MyStdMap>
         (&aux_vect2[0], &aux_vect2[0] + MaxElem);
      ::boost::movelib::unique_ptr<MyBoostMultiMap> const pboostmultimap2 = ::boost::movelib::make_unique<MyBoostMultiMap>
               ( boost::make_move_iterator(&aux_vect3[0])
               , boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      ::boost::movelib::unique_ptr<MyStdMultiMap> const pstdmultimap2 = ::boost::movelib::make_unique<MyStdMultiMap>
         (&aux_vect2[0], &aux_vect2[0] + MaxElem);
      MyBoostMap &boostmap2 = *pboostmap2;
      MyStdMap   &stdmap2   = *pstdmap2;
      MyBoostMultiMap &boostmultimap2 = *pboostmultimap2;
      MyStdMultiMap   &stdmultimap2   = *pstdmultimap2;

      if(!CheckEqualContainers(boostmap2, stdmap2)) return 1;
      if(!CheckEqualContainers(boostmultimap2, stdmultimap2)) return 1;

      //ordered range insertion
      //This is really nasty, but we have no other simple choice
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      for(int i = 0; i < MaxElem; ++i){
         new(&aux_vect2[i])StdValueType(StdKeyType(i), StdMappedType(i));
      }

      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
      }
      if(!CheckEqualContainers(boostmap2, stdmap2)) return 1;
      if(!CheckEqualContainers(boostmultimap2, stdmultimap2)) return 1;

      //some comparison operators
      if(!(boostmap2 == boostmap2))
         return 1;
      if(boostmap2 != boostmap2)
         return 1;
      if(boostmap2 < boostmap2)
         return 1;
      if(boostmap2 > boostmap2)
         return 1;
      if(!(boostmap2 <= boostmap2))
         return 1;
      if(!(boostmap2 >= boostmap2))
         return 1;

      ::boost::movelib::unique_ptr<MyBoostMap> const pboostmap3 = ::boost::movelib::make_unique<MyBoostMap>
               ( boost::make_move_iterator(&aux_vect[0])
               , boost::make_move_iterator(&aux_vect[0] + MaxElem));
      ::boost::movelib::unique_ptr<MyStdMap> const pstdmap3 = ::boost::movelib::make_unique<MyStdMap>
               (&aux_vect2[0], &aux_vect2[0] + MaxElem);
      ::boost::movelib::unique_ptr<MyBoostMultiMap> const pboostmultimap3 = ::boost::movelib::make_unique<MyBoostMultiMap>
               ( boost::make_move_iterator(&aux_vect3[0])
               , boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      ::boost::movelib::unique_ptr<MyStdMultiMap> const pstdmultimap3 = ::boost::movelib::make_unique<MyStdMultiMap>
               (&aux_vect2[0], &aux_vect2[0] + MaxElem);
      MyBoostMap &boostmap3 = *pboostmap3;
      MyStdMap   &stdmap3   = *pstdmap3;
      MyBoostMultiMap &boostmultimap3 = *pboostmultimap3;
      MyStdMultiMap   &stdmultimap3   = *pstdmultimap3;

      if(!CheckEqualContainers(boostmap3, stdmap3)){
         std::cout << "Error in construct<MyBoostMap>(MyBoostMap3)" << std::endl;
         return 1;
      }
      if(!CheckEqualContainers(boostmultimap3, stdmultimap3)){
         std::cout << "Error in construct<MyBoostMultiMap>(MyBoostMultiMap3)" << std::endl;
         return 1;
      }

      if (0 != map_move_assignable_only<MyBoostMap, MyStdMap, MyBoostMultiMap, MyStdMultiMap>(dtl::bool_<boost::container::test::is_move_assignable<IntType>::value>()))
         return 1;

      {
         IntType i0(0);
         boostmap2.erase(i0);
         boostmultimap2.erase(i0);
         stdmap2.erase(0);
         stdmultimap2.erase(0);
      }
   }
   return 0;
}

template<class MyBoostMap
        , class MyStdMap
        , class MyBoostMultiMap
        , class MyStdMultiMap>
int map_test_insert(MyBoostMap &boostmap, MyStdMap &stdmap, MyBoostMultiMap &boostmultimap, MyStdMultiMap &stdmultimap)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;
   typedef typename MyStdMap::value_type  StdPairType;

   {
      //This is really nasty, but we have no other simple choice
      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }
      IntPairType aux_vect3[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      for(int i = 0; i < MaxElem; ++i){
         boostmap.insert(boost::move(aux_vect[i]));
         stdmap.insert(StdPairType(i, i));
         boostmultimap.insert(boost::move(aux_vect3[i]));
         stdmultimap.insert(StdPairType(i, i));
      }

      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;

      typename MyBoostMap::iterator it = boostmap.begin();
      typename MyBoostMap::const_iterator cit = it;
      (void)cit;

      boostmap.erase(boostmap.begin());
      stdmap.erase(stdmap.begin());
      boostmultimap.erase(boostmultimap.begin());
      stdmultimap.erase(stdmultimap.begin());
      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;

      boostmap.erase(boostmap.begin());
      stdmap.erase(stdmap.begin());
      boostmultimap.erase(boostmultimap.begin());
      stdmultimap.erase(stdmultimap.begin());
      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;

      {
         //Swapping test
         MyBoostMap tmpboostemap2;
         MyStdMap tmpstdmap2;
         MyBoostMultiMap tmpboostemultimap2;
         MyStdMultiMap tmpstdmultimap2;
         boostmap.swap(tmpboostemap2);
         stdmap.swap(tmpstdmap2);
         boostmultimap.swap(tmpboostemultimap2);
         stdmultimap.swap(tmpstdmultimap2);
         boostmap.swap(tmpboostemap2);
         stdmap.swap(tmpstdmap2);
         boostmultimap.swap(tmpboostemultimap2);
         stdmultimap.swap(tmpstdmultimap2);
         if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
         if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;
      }

   //move constructor/assignment
   {
      MyBoostMap tmpboostemap2(boost::move(boostmap));
      if(!CheckEqualContainers(tmpboostemap2, stdmap)){
         std::cout << "Error in boostmap move constructor" << std::endl;
         return 1;
      }
      MyBoostMultiMap tmpboostemultimap2(boost::move(boostmultimap));
      if(!CheckEqualContainers(tmpboostemultimap2, stdmultimap)){
         std::cout << "Error in boostmultimap move constructor" << std::endl;
         return 1;
      }

      boostmap = boost::move(tmpboostemap2);
      if(!CheckEqualContainers(boostmap, stdmap)){
         std::cout << "Error in boostmap move assignment" << std::endl;
         return 1;
      }
      boostmultimap = boost::move(tmpboostemultimap2);
      if(!CheckEqualContainers(boostmultimap, stdmultimap)){
         std::cout << "Error in boostmultimap move assignment" << std::endl;
         return 1;
      }
   }
   }
   return 0;
}

template<class MyBoostMap
        , class MyStdMap
        , class MyBoostMultiMap
        , class MyStdMultiMap>
int map_test_erase(MyBoostMap &boostmap, MyStdMap &stdmap, MyBoostMultiMap &boostmultimap, MyStdMultiMap &stdmultimap)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;
   typedef typename MyStdMap::value_type  StdPairType;

   //Insertion from other container
   //Initialize values
   {
      //This is really nasty, but we have no other simple choice
      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(-1);
         IntType i2(-1);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }
      IntPairType aux_vect3[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(-1);
         IntType i2(-1);
         new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      boostmap.insert(boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0] + MaxElem));
      boostmultimap.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      for(int i = 0; i != MaxElem; ++i){
         StdPairType stdpairtype(-1, -1);
         stdmap.insert(stdpairtype);
         stdmultimap.insert(stdpairtype);
      }
      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;

      for(int i = 0, j = static_cast<int>(boostmap.size()); i < j; ++i){
         IntType k(i);
         boostmap.erase(k);
         stdmap.erase(i);
         boostmultimap.erase(k);
         stdmultimap.erase(i);
      }
      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;
   }
   {
      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(-1);
         IntType i2(-1);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      IntPairType aux_vect3[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(-1);
         IntType i2(-1);
         new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      IntPairType aux_vect4[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(-1);
         IntType i2(-1);
         new(&aux_vect4[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      IntPairType aux_vect5[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(-1);
         IntType i2(-1);
         new(&aux_vect5[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      boostmap.insert(boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0] + MaxElem));
      boostmap.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      boostmultimap.insert(boost::make_move_iterator(&aux_vect4[0]), boost::make_move_iterator(aux_vect4 + MaxElem));
      boostmultimap.insert(boost::make_move_iterator(&aux_vect5[0]), boost::make_move_iterator(aux_vect5 + MaxElem));

      for(int i = 0; i != MaxElem; ++i){
         StdPairType stdpairtype(-1, -1);
         stdmap.insert(stdpairtype);
         stdmultimap.insert(stdpairtype);
         stdmap.insert(stdpairtype);
         stdmultimap.insert(stdpairtype);
      }
      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;

      boostmap.erase(boostmap.begin()->first);
      stdmap.erase(stdmap.begin()->first);
      boostmultimap.erase(boostmultimap.begin()->first);
      stdmultimap.erase(stdmultimap.begin()->first);
      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;
   }

   {  //erase_if
      boostmap.clear();
      boostmultimap.clear();
      stdmap.clear();
      stdmultimap.clear();

      {
         IntPairType aux_vect[(std::size_t)MaxElem];
         IntPairType aux_vect2[(std::size_t)MaxElem];
         IntPairType aux_vect3[(std::size_t)MaxElem];

         for(int i = 0; i < MaxElem; ++i){
            IntType i1(i);
            IntType i2(i);
            new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
         }

         for(int i = 0; i < MaxElem; ++i){
            IntType i1(i);
            IntType i2(i);
            new(&aux_vect2[i])IntPairType(boost::move(i1), boost::move(i2));
         }

         for(int i = 0; i < MaxElem; ++i){
            IntType i1(i);
            IntType i2(i);
            new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
         }

         boostmap.     insert(boost::make_move_iterator(&aux_vect[0]),  boost::make_move_iterator(&aux_vect[0]  + MaxElem));
         boostmultimap.insert(boost::make_move_iterator(&aux_vect2[0]), boost::make_move_iterator(&aux_vect2[0] + MaxElem));
         boostmultimap.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      }

      for(int i = 0; i < MaxElem; ++i){
         stdmap.insert((StdPairType(i, i)));
         stdmultimap.insert((StdPairType(i, i)));
         stdmultimap.insert((StdPairType(i, i)));
      }

      for(int i = 0; i < MaxElem; ++i) {
         //erase_if
         const int key = (i + MaxElem/2) % MaxElem;

         if (1 != erase_if(boostmap, equal_to_value_first<int>(key)))
            return 1;
         if (0 != erase_if(boostmap, equal_to_value_first<int>(key)))
            return 1;
         stdmap.erase(key);
         if(!test::CheckEqualContainers(boostmap, stdmap)) return false;

         //erase_if
         if (2 != erase_if(boostmultimap, equal_to_value_first<int>(key)))
            return 1;
         if (0 != erase_if(boostmultimap, equal_to_value_first<int>(key)))
            return 1;
         stdmultimap.erase(key);
         if(!test::CheckEqualContainers(boostmultimap, stdmultimap)) return false;
      }
   }

   return 0;
}

template<class MyBoostMap
        , class MyStdMap
        , class MyBoostMultiMap
        , class MyStdMultiMap>
int map_test_insert2(MyBoostMap &boostmap, MyStdMap &stdmap, MyBoostMultiMap &boostmultimap, MyStdMultiMap &stdmultimap)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;
   typedef typename MyStdMap::value_type  StdPairType;

   //This is really nasty, but we have no other simple choice
   IntPairType aux_vect[(std::size_t)MaxElem];
   for(int i = 0; i < MaxElem; ++i){
      IntType i1(i);
      IntType i2(i);
      new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
   }
   IntPairType aux_vect3[(std::size_t)MaxElem];
   for(int i = 0; i < MaxElem; ++i){
      IntType i1(i);
      IntType i2(i);
      new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
   }

   for(int i = 0; i < MaxElem; ++i){
      boostmap.insert(boost::move(aux_vect[i]));
      stdmap.insert(StdPairType(i, i));
      boostmultimap.insert(boost::move(aux_vect3[i]));
      stdmultimap.insert(StdPairType(i, i));
   }

   if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
   if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;

   for(int i = 0; i < MaxElem; ++i){
      IntPairType intpair;
      {
         IntType i1(i);
         IntType i2(i);
         new(&intpair)IntPairType(boost::move(i1), boost::move(i2));
      }
      boostmap.insert(boostmap.begin(), boost::move(intpair));
      stdmap.insert(stdmap.begin(), StdPairType(i, i));
      //PrintContainers(boostmap, stdmap);
      {
         IntType i1(i);
         IntType i2(i);
         new(&intpair)IntPairType(boost::move(i1), boost::move(i2));
      }
      boostmultimap.insert(boostmultimap.begin(), boost::move(intpair));
      stdmultimap.insert(stdmultimap.begin(), StdPairType(i, i));
      //PrintContainers(boostmultimap, stdmultimap);
      if(!CheckEqualPairContainers(boostmap, stdmap))
         return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap))
         return 1;
      {
         IntType i1(i);
         IntType i2(i);
         new(&intpair)IntPairType(boost::move(i1), boost::move(i2));
      }
      boostmap.insert(boostmap.end(), boost::move(intpair));
      stdmap.insert(stdmap.end(), StdPairType(i, i));
      {
         IntType i1(i);
         IntType i2(i);
         new(&intpair)IntPairType(boost::move(i1), boost::move(i2));
      }
      boostmultimap.insert(boostmultimap.end(), boost::move(intpair));
      stdmultimap.insert(stdmultimap.end(), StdPairType(i, i));
      if(!CheckEqualPairContainers(boostmap, stdmap))
         return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap))
         return 1;
      {
         IntType i1(i);
         IntType i2(i);
         new(&intpair)IntPairType(boost::move(i1), boost::move(i2));
      }
      IntType k(i);
      boostmap.insert(boostmap.lower_bound(k), boost::move(intpair));
      stdmap.insert(stdmap.lower_bound(i), StdPairType(i, i));
      //PrintContainers(boostmap, stdmap);
      {
         IntType i1(i);
         IntType i2(i);
         new(&intpair)IntPairType(boost::move(i1), boost::move(i2));
      }
      {
         IntType i1(i);
         boostmultimap.insert(boostmultimap.lower_bound(boost::move(i1)), boost::move(intpair));
         stdmultimap.insert(stdmultimap.lower_bound(i), StdPairType(i, i));
      }

      //PrintContainers(boostmultimap, stdmultimap);
      if(!CheckEqualPairContainers(boostmap, stdmap))
         return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap))
         return 1;
      {  //Check equal_range
         std::pair<typename MyBoostMultiMap::iterator, typename MyBoostMultiMap::iterator> bret =
            boostmultimap.equal_range(boostmultimap.begin()->first);

         std::pair<typename MyStdMultiMap::iterator, typename MyStdMultiMap::iterator>   sret =
            stdmultimap.equal_range(stdmultimap.begin()->first);

         if( boost::container::iterator_udistance(bret.first, bret.second) !=
               boost::container::iterator_udistance(sret.first, sret.second) ){
            return 1;
         }
      }
      {
         IntType i1(i);
         boostmap.insert(boostmap.upper_bound(boost::move(i1)), boost::move(intpair));
         stdmap.insert(stdmap.upper_bound(i), StdPairType(i, i));
      }
      //PrintContainers(boostmap, stdmap);
      {
         IntType i1(i);
         IntType i2(i);
         new(&intpair)IntPairType(boost::move(i1), boost::move(i2));
      }
      {
         IntType i1(i);
         boostmultimap.insert(boostmultimap.upper_bound(boost::move(i1)), boost::move(intpair));
         stdmultimap.insert(stdmultimap.upper_bound(i), StdPairType(i, i));
      }
      //PrintContainers(boostmultimap, stdmultimap);
      if(!CheckEqualPairContainers(boostmap, stdmap))
         return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap))
         return 1;

      map_test_rebalanceable(boostmap
         , dtl::bool_<has_member_rebalance<MyBoostMap>::value>());
      if(!CheckEqualContainers(boostmap, stdmap)){
         std::cout << "Error in boostmap.rebalance()" << std::endl;
         return 1;
      }
      map_test_rebalanceable(boostmultimap
         , dtl::bool_<has_member_rebalance<MyBoostMap>::value>());
      if(!CheckEqualContainers(boostmultimap, stdmultimap)){
         std::cout << "Error in boostmultimap.rebalance()" << std::endl;
         return 1;
      }
   }
   return 0;
}


template<class MyBoostMap
        , class MyStdMap
        , class MyBoostMultiMap
        , class MyStdMultiMap>
int map_test_search(MyBoostMap &boostmap, MyStdMap &stdmap, MyBoostMultiMap &boostmultimap, MyStdMultiMap &stdmultimap)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;

   //Compare count/contains with std containers
   for(int i = 0; i < MaxElem; ++i){
      IntType k(i);
      if(boostmap.count(k) != stdmap.count(i)){
         return -1;
      }

      if(boostmap.contains(k) != (stdmap.find(i) != stdmap.end())){
         return -1;
      }

      if(boostmultimap.count(k) != stdmultimap.count(i)){
         return -1;
      }

      if(boostmultimap.contains(k) != (stdmultimap.find(i) != stdmultimap.end())){
         return -1;
      }
   }

   {
      //Now do count exercise
      boostmap.erase(boostmap.begin(), boostmap.end());
      boostmultimap.erase(boostmultimap.begin(), boostmultimap.end());
      boostmap.clear();
      boostmultimap.clear();

      for(int j = 0; j < 3; ++j)
      for(int i = 0; i < 100; ++i){
         IntPairType intpair;
         {
         IntType i1(i), i2(i);
         new(&intpair)IntPairType(boost::move(i1), boost::move(i2));
         }
         boostmap.insert(boost::move(intpair));
         {
            IntType i1(i), i2(i);
            new(&intpair)IntPairType(boost::move(i1), boost::move(i2));
         }
         boostmultimap.insert(boost::move(intpair));
         IntType k(i);
         if(boostmap.count(k) != typename MyBoostMultiMap::size_type(1))
            return 1;
         if(boostmultimap.count(k) != typename MyBoostMultiMap::size_type(j+1))
            return 1;
      }
   }

   return 0;
}

template<class MyBoostMap
        , class MyStdMap
        , class MyBoostMultiMap
        , class MyStdMultiMap
        , class Other>
int map_test_indexing(MyBoostMap &boostmap, MyStdMap &stdmap, MyBoostMultiMap &boostmultimap, MyStdMultiMap &stdmultimap, boost::container::dtl::true_type, Other)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;

   {  //operator[] test
      boostmap.clear();
      boostmultimap.clear();
      stdmap.clear();
      stdmultimap.clear();

      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      for(int i = 0; i < MaxElem; ++i){
         boostmap[boost::move(aux_vect[i].first)] = boost::move(aux_vect[i].second);
         stdmap[i] = i;
      }

      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;
   }
   return 0;
}

template<class MyBoostMap
   , class MyStdMap
   , class MyBoostMultiMap
   , class MyStdMultiMap>
int map_test_indexing_move_assignable(MyBoostMap& boostmap, MyStdMap& stdmap, MyBoostMultiMap& boostmultimap, MyStdMultiMap& stdmultimap, boost::container::dtl::true_type)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;

   {  //operator[] test
      boostmap.clear();
      boostmultimap.clear();
      stdmap.clear();
      stdmultimap.clear();

      IntPairType aux_vect[(std::size_t)MaxElem];
      for (int i = 0; i < MaxElem; ++i) {
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      for (int i = 0; i < MaxElem; ++i) {
         boostmap[boost::move(aux_vect[i].first)] = boost::move(aux_vect[i].second);
         stdmap[i] = i;
      }

      if (!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if (!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;
   }
   return 0;
}

template<class MyBoostMap
   , class MyStdMap
   , class MyBoostMultiMap
   , class MyStdMultiMap>
int map_test_indexing_copyable(MyBoostMap& boostmap, MyStdMap& stdmap, MyBoostMultiMap& boostmultimap, MyStdMultiMap& stdmultimap, boost::container::dtl::true_type)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;

   {  //operator[] test
      boostmap.clear();
      boostmultimap.clear();
      stdmap.clear();
      stdmultimap.clear();

      IntPairType aux_vect[(std::size_t)MaxElem];
      for (int i = 0; i < MaxElem; ++i) {
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      for (int i = 0; i < MaxElem; ++i) {
         boostmap[aux_vect[i].first] = boost::move(aux_vect[i].second);
         stdmap[i] = i;
      }

      if (!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if (!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;
   }
   return 0;
}

template<class MyBoostMap
   , class MyStdMap
   , class MyBoostMultiMap
   , class MyStdMultiMap>
int map_test_indexing_move_assignable(MyBoostMap& , MyStdMap& , MyBoostMultiMap& , MyStdMultiMap&, boost::container::dtl::false_type)
{
   return 0;
}

template<class MyBoostMap
   , class MyStdMap
   , class MyBoostMultiMap
   , class MyStdMultiMap>
int map_test_indexing_copyable(MyBoostMap&, MyStdMap&, MyBoostMultiMap&, MyStdMultiMap&, boost::container::dtl::false_type)
{
   return 0;
}

template< class MyBoostMap, class StdMap, class MaybeMove>
int map_test_insert_or_assign_impl()
{
   typedef typename MyBoostMap::key_type     IntType;
   typedef dtl::pair<IntType, IntType>       IntPairType;
   typedef typename MyBoostMap::iterator     Biterator;
   typedef std::pair<Biterator, bool>        Bpair;

   MaybeMove maybe_move;

   {  //insert_or_assign test
      MyBoostMap boostmap;
      StdMap stdmap;
      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(MaxElem-i);
         new(&aux_vect[i])IntPairType(maybe_move(i1), maybe_move(i2));
      }

      IntPairType aux_vect2[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect2[i])IntPairType(maybe_move(i1), maybe_move(i2));
      }

      for(int i = 0; i < MaxElem; ++i){
         Bpair r = boostmap.insert_or_assign(maybe_move(aux_vect[i].first), maybe_move(aux_vect[i].second));
         stdmap[i] = MaxElem-i;
         if(!r.second)
            return 1;
         const IntType key(i);
         if(r.first->first != key)
            return 1;
         const IntType mapped(MaxElem-i);
         if(r.first->second != mapped)
            return 1;
      }

      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;

      for(int i = 0; i < MaxElem; ++i){
         Bpair r = boostmap.insert_or_assign(maybe_move(aux_vect2[i].first), maybe_move(aux_vect2[i].second));
         stdmap[i] = i;
         if(r.second)
            return 1;
         const IntType key(i);
         if(r.first->first != key)
            return 1;
         const IntType mapped(i);
         if(r.first->second != mapped)
            return 1;
      }

      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
   }
   {  //insert_or_assign test with hint
      MyBoostMap boostmap;
      StdMap stdmap;
      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(MaxElem-i);
         new(&aux_vect[i])IntPairType(maybe_move(i1), maybe_move(i2));
      }

      IntPairType aux_vect2[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect2[i])IntPairType(maybe_move(i1), maybe_move(i2));
      }

      for(int i = 0; i < MaxElem; ++i){
         Biterator r = boostmap.insert_or_assign(boostmap.end(), maybe_move(aux_vect[i].first), maybe_move(aux_vect[i].second));
         stdmap[i] = MaxElem-i;
         const IntType key(i);
         if(r->first != key)
            return 1;
         const IntType mapped(MaxElem-i);
         if(r->second != mapped)
            return 1;
      }

      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;

      for(int i = 0; i < MaxElem; ++i){
         Biterator r = boostmap.insert_or_assign(boostmap.end(), maybe_move(aux_vect2[i].first), maybe_move(aux_vect2[i].second));
         stdmap[i] = i;
         const IntType key(i);
         if(r->first != key)
            return 1;
         const IntType mapped(i);
         if(r->second != mapped)
            return 1;
      }

      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
   }
   return 0;
}

template< class MyBoostMap, class StdMap, class Copyable>
int map_test_insert_or_assign(dtl::bool_<false>, Copyable)//noncopyable
{
   return 0;
}

template< class MyBoostMap, class StdMap>
int map_test_insert_or_assign(dtl::bool_<true>, dtl::bool_<false> )//noncopyable
{
   return map_test_insert_or_assign_impl<MyBoostMap, StdMap, move_op>();
}

template< class MyBoostMap, class StdMap>
int map_test_insert_or_assign(dtl::bool_<true>, dtl::bool_<true> )//copyable
{
   int r = map_test_insert_or_assign_impl<MyBoostMap, StdMap, const_ref_op>();
   if (r)
      r = map_test_insert_or_assign_impl<MyBoostMap, StdMap, move_op>();
   return r;
}

template< class MyBoostMap
        , class MyStdMap
        , class MyBoostMultiMap
        , class MyStdMultiMap>
int map_test_try_emplace(MyBoostMap &boostmap, MyStdMap &stdmap, MyBoostMultiMap &boostmultimap, MyStdMultiMap &stdmultimap)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;

   {  //try_emplace
      boostmap.clear();
      boostmultimap.clear();
      stdmap.clear();
      stdmultimap.clear();

      IntPairType aux_vect[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(i);
         new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      IntPairType aux_vect2[(std::size_t)MaxElem];
      for(int i = 0; i < MaxElem; ++i){
         IntType i1(i);
         IntType i2(MaxElem-i);
         new(&aux_vect2[i])IntPairType(boost::move(i1), boost::move(i2));
      }

      typedef typename MyBoostMap::iterator iterator;
      for(int i = 0; i < MaxElem; ++i){
         iterator it;
         if(i&1){
            std::pair<typename MyBoostMap::iterator, bool> ret =
               boostmap.try_emplace(boost::move(aux_vect[i].first), boost::move(aux_vect[i].second));
            if(!ret.second)
               return 1;
            it = ret.first;
         }
         else{
            it = boostmap.try_emplace
               (boostmap.upper_bound(aux_vect[i].first), boost::move(aux_vect[i].first), boost::move(aux_vect[i].second));
         }
         if(boostmap.end() == it || it->first != aux_vect2[i].first || it->second != aux_vect2[i].first){
            return 1;
         }
         stdmap[i] = i;
      }

      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;

      for(int i = 0; i < MaxElem; ++i){
         iterator it;
         iterator itex = boostmap.find(aux_vect2[i].first);
         if(itex == boostmap.end())
            return 1;
         if(i&1){
            std::pair<typename MyBoostMap::iterator, bool> ret =
               boostmap.try_emplace(boost::move(aux_vect2[i].first), boost::move(aux_vect2[i].second));
            if(ret.second)
               return 1;
            it = ret.first;
         }
         else{
            it = boostmap.try_emplace
               (boostmap.upper_bound(aux_vect2[i].first), boost::move(aux_vect2[i].first), boost::move(aux_vect2[i].second));
         }
         const IntType test_int(i);
         if(boostmap.end() == it || it != itex || it->second != test_int){
            return 1;
         }
      }

      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;
   }
   return 0;
}


template< class MyBoostMap
        , class MyStdMap
        , class MyBoostMultiMap
        , class MyStdMultiMap>
int map_test_merge(MyBoostMap &boostmap, MyStdMap &stdmap, MyBoostMultiMap &boostmultimap, MyStdMultiMap &stdmultimap)
{
   typedef typename MyBoostMap::key_type    IntType;
   typedef dtl::pair<IntType, IntType>         IntPairType;
   typedef typename MyStdMap::value_type  StdPairType;
   
   {  //merge
      ::boost::movelib::unique_ptr<MyBoostMap> const pboostmap2 = ::boost::movelib::make_unique<MyBoostMap>();
      ::boost::movelib::unique_ptr<MyBoostMultiMap> const pboostmultimap2 = ::boost::movelib::make_unique<MyBoostMultiMap>();

      MyBoostMap &boostmap2 = *pboostmap2;
      MyBoostMultiMap &boostmultimap2 = *pboostmultimap2;

      boostmap.clear();
      boostmap2.clear();
      boostmultimap.clear();
      boostmultimap2.clear();
      stdmap.clear();
      stdmultimap.clear();

      {
         IntPairType aux_vect[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            IntType i1(i);
            IntType i2(i);
            new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
         }

         IntPairType aux_vect2[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            IntType i1(MaxElem/2+i);
            IntType i2(MaxElem-i);
            new(&aux_vect2[i])IntPairType(boost::move(i1), boost::move(i2));
         }
         IntPairType aux_vect3[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            IntType i1(MaxElem*2/2+i);
            IntType i2(MaxElem*2+i);
            new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
         }
         boostmap.insert(boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0] + MaxElem));
         boostmap2.insert(boost::make_move_iterator(&aux_vect2[0]), boost::make_move_iterator(&aux_vect2[0] + MaxElem));
         boostmultimap2.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      }
      for(int i = 0; i < MaxElem; ++i){
         stdmap.insert(StdPairType(i, i));
      }
      for(int i = 0; i < MaxElem; ++i){
         stdmap.insert(StdPairType(MaxElem/2+i, MaxElem-i));
      }

      boostmap.merge(boost::move(boostmap2));
      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;

      for(int i = 0; i < MaxElem; ++i){
         stdmap.insert(StdPairType(MaxElem*2/2+i, MaxElem*2+i));
      }

      boostmap.merge(boost::move(boostmultimap2));
      if(!CheckEqualPairContainers(boostmap, stdmap)) return 1;

      boostmap.clear();
      boostmap2.clear();
      boostmultimap.clear();
      boostmultimap2.clear();
      stdmap.clear();
      stdmultimap.clear();
      {
         IntPairType aux_vect[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            IntType i1(i);
            IntType i2(i);
            new(&aux_vect[i])IntPairType(boost::move(i1), boost::move(i2));
         }

         IntPairType aux_vect2[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            IntType i1(MaxElem/2+i);
            IntType i2(MaxElem-i);
            new(&aux_vect2[i])IntPairType(boost::move(i1), boost::move(i2));
         }
         IntPairType aux_vect3[(std::size_t)MaxElem];
         for(int i = 0; i < MaxElem; ++i){
            IntType i1(MaxElem*2/2+i);
            IntType i2(MaxElem*2+i);
            new(&aux_vect3[i])IntPairType(boost::move(i1), boost::move(i2));
         }
         boostmultimap.insert(boost::make_move_iterator(&aux_vect[0]), boost::make_move_iterator(&aux_vect[0] + MaxElem));
         boostmultimap2.insert(boost::make_move_iterator(&aux_vect2[0]), boost::make_move_iterator(&aux_vect2[0] + MaxElem));
         boostmap2.insert(boost::make_move_iterator(&aux_vect3[0]), boost::make_move_iterator(&aux_vect3[0] + MaxElem));
      }
      for(int i = 0; i < MaxElem; ++i){
         stdmultimap.insert(StdPairType(i, i));
      }
      for(int i = 0; i < MaxElem; ++i){
         stdmultimap.insert(StdPairType(MaxElem/2+i, MaxElem-i));
      }
      boostmultimap.merge(boostmultimap2);
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;

      for(int i = 0; i < MaxElem; ++i){
         stdmultimap.insert(StdPairType(MaxElem*2/2+i, MaxElem*2+i));
      }

      boostmultimap.merge(boostmap2);
      if(!CheckEqualPairContainers(boostmultimap, stdmultimap)) return 1;
   }
   return 0;
}


template<class MyBoostMap
        ,class MyStdMap
        ,class MyBoostMultiMap
        ,class MyStdMultiMap>
int map_test()
{
   typedef typename MyBoostMap::key_type    IntType;

   if(map_test_range<MyBoostMap, MyStdMap, MyBoostMultiMap, MyStdMultiMap>())
      return 1;

   ::boost::movelib::unique_ptr<MyBoostMap> const pboostmap = ::boost::movelib::make_unique<MyBoostMap>();
   ::boost::movelib::unique_ptr<MyStdMap> const pstdmap = ::boost::movelib::make_unique<MyStdMap>();
   ::boost::movelib::unique_ptr<MyBoostMultiMap> const pboostmultimap = ::boost::movelib::make_unique<MyBoostMultiMap>();
   ::boost::movelib::unique_ptr<MyStdMultiMap> const pstdmultimap = ::boost::movelib::make_unique<MyStdMultiMap>();
   MyBoostMap &boostmap = *pboostmap;
   MyStdMap   &stdmap   = *pstdmap;
   MyBoostMultiMap &boostmultimap = *pboostmultimap;
   MyStdMultiMap   &stdmultimap   = *pstdmultimap;
   typedef dtl::bool_<boost::container::test::is_copyable<IntType>::value> copyable_t;
   typedef dtl::bool_<boost::container::test::is_move_assignable<IntType>::value> move_assignable_t;

   if (map_test_step(boostmap, stdmap, boostmultimap, stdmultimap))
      return 1;

   if (map_test_insert(boostmap, stdmap, boostmultimap, stdmultimap))
      return 1;

   if (map_test_erase(boostmap, stdmap, boostmultimap, stdmultimap))
      return 1;

   if (map_test_insert2(boostmap, stdmap, boostmultimap, stdmultimap))
      return 1;

   if (map_test_search(boostmap, stdmap, boostmultimap, stdmultimap))
      return 1;

   if (map_test_indexing_move_assignable(boostmap, stdmap, boostmultimap, stdmultimap, move_assignable_t()))
      return 1;

   if (map_test_indexing_copyable(boostmap, stdmap, boostmultimap, stdmultimap, copyable_t()))
      return 1;

   if (map_test_try_emplace(boostmap, stdmap, boostmultimap, stdmultimap))
      return 1;

   if (map_test_merge(boostmap, stdmap, boostmultimap, stdmultimap))
      return 1;

   if (map_test_insert_or_assign<MyBoostMap, MyStdMap>(move_assignable_t(), copyable_t()))
      return 1;

   if(map_test_copyable<MyBoostMap, MyStdMap, MyBoostMultiMap, MyStdMultiMap>(copyable_t()))
      return 1;
   return 0;
}

template<typename MapType>
bool test_map_support_for_initialization_list_for()
{
#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   const std::initializer_list<std::pair<typename MapType::value_type::first_type, typename MapType::mapped_type>> il
      = { std::make_pair(1, 2), std::make_pair(3, 4) };

   const MapType expected_map(il.begin(), il.end());
   {
      const MapType sil = il;
      if (sil != expected_map)
         return false;

      MapType sila(il, typename MapType::allocator_type());
      if (sila != expected_map)
         return false;

      MapType silca(il, typename MapType::key_compare(), typename MapType::allocator_type());
      if (silca != expected_map)
         return false;

      const MapType sil_ordered(ordered_unique_range, il);
      if (sil_ordered != expected_map)
         return false;

      MapType sil_assign = { std::make_pair(99, 100) };
      sil_assign = il;
      if (sil_assign != expected_map)
         return false;
   }
   {
      MapType sil;
      sil.insert(il);
      if (sil != expected_map)
         return false;
   }
   return true;
#else
   return true;
#endif
}

template<typename MapType, typename MultimapType>
bool instantiate_constructors()
{
   {
      typedef typename MapType::value_type value_type;
      typename MapType::key_compare comp;
      typename MapType::allocator_type a;
      value_type value;
      {
         MapType s0;
         MapType s1(comp);
         MapType s2(a);
         MapType s3(comp, a);
      }
      {
         MapType s0(&value, &value);
         MapType s1(&value, &value ,comp);
         MapType s2(&value, &value ,a);
         MapType s3(&value, &value ,comp, a);
      }
      #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
      {
         std::initializer_list<value_type> il;
         MapType s0(il);
         MapType s1(il, comp);
         MapType s2(il, a);
         MapType s3(il, comp, a);
      }
      {
         std::initializer_list<value_type> il;
         MapType s0(ordered_unique_range, il);
         MapType s1(ordered_unique_range, il, comp);
         MapType s3(ordered_unique_range, il, comp, a);
      }
      #endif
      {
         MapType s0(ordered_unique_range, &value, &value);
         MapType s1(ordered_unique_range, &value, &value ,comp);
         MapType s2(ordered_unique_range, &value, &value ,comp, a);
      }
   }

   {
      typedef typename MultimapType::value_type value_type;
      typename MultimapType::key_compare comp;
      typename MultimapType::allocator_type a;
      value_type value;
      {
         MultimapType s0;
         MultimapType s1(comp);
         MultimapType s2(a);
         MultimapType s3(comp, a);
      }
      {
         MultimapType s0(&value, &value);
         MultimapType s1(&value, &value ,comp);
         MultimapType s2(&value, &value ,a);
         MultimapType s3(&value, &value ,comp, a);
      }
      #if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
      {
         std::initializer_list<value_type> il;
         MultimapType s0(il);
         MultimapType s1(il, comp);
         MultimapType s2(il, a);
         MultimapType s3(il, comp, a);
      }
      {
         std::initializer_list<value_type> il;
         MultimapType s0(ordered_range, il);
         MultimapType s1(ordered_range, il, comp);
         MultimapType s3(ordered_range, il, comp, a);
      }
      #endif
      {
         MultimapType s0(ordered_range, &value, &value);
         MultimapType s1(ordered_range, &value, &value ,comp);
         MultimapType s2(ordered_range, &value, &value ,comp, a);
      }
   }
   return true;
}


template<typename IntMapType, typename IntMultimapType>
bool test_heterogeneous_lookup()
{
   typedef IntMapType map_t;
   typedef IntMultimapType mmap_t;
   typedef typename map_t::value_type value_type;
   
   map_t map1;
   const map_t &cmap1 = map1;

   if(!map1.insert_or_assign(1, 'a').second)
      return false;
   if( map1.insert_or_assign(1, 'b').second)
      return false;
   if(!map1.insert_or_assign(2, 'c').second)
      return false;
   if( map1.insert_or_assign(2, 'd').second)
      return false;
   if(!map1.insert_or_assign(3, 'e').second)
      return false;

   mmap_t mmap1;
   const mmap_t &cmmap1 = mmap1;

   mmap1.insert(value_type(1, 'a'));
   mmap1.insert(value_type(1, 'b'));
   mmap1.insert(value_type(2, 'c'));
   mmap1.insert(value_type(2, 'd'));
   mmap1.insert(value_type(3, 'e'));

   const test::non_copymovable_int find_me(2);
   const test::non_copymovable_int not_present(5);

   //find
   if(map1.find(find_me)->second != 'd')
      return false;
   if(cmap1.find(find_me)->second != 'd')
      return false;
   if(mmap1.find(find_me)->second != 'c')
      return false;
   if(cmmap1.find(find_me)->second != 'c')
      return false;
   if(map1.find(not_present)   != map1.end())
      return false;
   if(cmap1.find(not_present)  != cmap1.end())
      return false;
   if(mmap1.find(not_present)  != mmap1.end())
      return false;
   if(cmmap1.find(not_present) != mmap1.cend())
      return false;


   //count
   if(map1.count(find_me) != 1)
      return false;
   if(cmap1.count(find_me) != 1)
      return false;
   if(mmap1.count(find_me) != 2)
      return false;
   if(cmmap1.count(find_me) != 2)
      return false;
   if(map1.count(not_present) != 0)
      return false;
   if(cmap1.count(not_present) != 0)
      return false;
   if(mmap1.count(not_present) != 0)
      return false;
   if(cmmap1.count(not_present) != 0)
      return false;

   //contains
   if(!map1.contains(find_me))
      return false;
   if(!cmap1.contains(find_me))
      return false;
   if(!mmap1.contains(find_me))
      return false;
   if(!cmmap1.contains(find_me))
      return false;
   if(map1.contains(not_present))
      return false;
   if(cmap1.contains(not_present))
      return false;
   if(mmap1.contains(not_present))
      return false;
   if(cmmap1.contains(not_present))
      return false;

   //at
   if(map1.at(find_me)  != 'd')
      return false;
   if(cmap1.at(find_me) != 'd')
      return false;

   //lower_bound
   if(map1.lower_bound(find_me)->second != 'd')
      return false;
   if(cmap1.lower_bound(find_me)->second != 'd')
      return false;
   if(mmap1.lower_bound(find_me)->second != 'c')
      return false;
   if(cmmap1.lower_bound(find_me)->second != 'c')
      return false;

   //upper_bound
   if(map1.upper_bound(find_me)->second != 'e')
      return false;
   if(cmap1.upper_bound(find_me)->second != 'e')
      return false;
   if(mmap1.upper_bound(find_me)->second != 'e')
      return false;
   if(cmmap1.upper_bound(find_me)->second != 'e')
      return false;

   //equal_range
   if(map1.equal_range(find_me).first->second != 'd')
      return false;
   if(cmap1.equal_range(find_me).second->second != 'e')
      return false;
   if(mmap1.equal_range(find_me).first->second != 'c')
      return false;
   if(cmmap1.equal_range(find_me).second->second != 'e')
      return false;

   //erase
   if (map1.erase(find_me) != 1)
      return false;
   if (map1.erase(find_me) != 0)
      return false;
   if (mmap1.erase(find_me) != 2)
      return false;
   if (mmap1.erase(find_me) != 0)
      return false;

   return true;
}

template<typename MovableIntMapType>
bool test_heterogeneous_insert()
{
   {
      typedef MovableIntMapType map_t;

      map_t map1;
      const map_t &cmap1 = map1;

      //insert_or_assign
      if(!map1.insert_or_assign(1, 'e').second)
         return false;
      if (cmap1.find(1)->second != 'e')
         return false;
      if(map1.insert_or_assign(1, 'b').second)
         return false;
      if (cmap1.find(1)->second != 'b')
         return false;

      //insert_or_assign with hint
      if(map1.find(2) != map1.end())
         return false;
      typename map_t::iterator i = map1.insert_or_assign(map1.begin(), 2, 'f');
      if(i != map1.insert_or_assign(map1.end(), 2, 'g'))
         return false;
      if (cmap1.find(2)->second != 'g')
         return false;

      //try_emplace
      map1.clear();
      if(!map1.try_emplace(1, 'a').second)
         return false;
      if (cmap1.find(1)->second != 'a')
         return false;
      if( map1.try_emplace(1, 'b').second)
         return false;
      if (cmap1.find(1)->second != 'a')
         return false;

      //try_emplace with hint
      i = map1.try_emplace(map1.end(), 2, 'c');
      if (cmap1.find(2)->second != 'c')
         return false;
      if (i != map1.try_emplace(map1.begin(), 2, 'd'))
         return false;
      if (cmap1.find(2)->second != 'c')
         return false;

      //operator[]
      typename map_t::mapped_type const *pm = &map1.find(2)->second;
      typename map_t::mapped_type &m = map1[2];
      if(m != 'c')
         return false;
      if(&m != pm)
         return false;
      
      map1[2] = 'd';
      if (cmap1.find(2)->second != 'd')
         return false;
      if(&m != &map1[2])
         return false;

      map1[3] = 'e';
      if (cmap1.find(3)->second != 'e')
         return false;
      if(map1[3] != map1[3])
         return false;

      if (map1[4] != 0)
         return false;

   }
   return true;
}

inline bool map_function_pointer_less(int a, int b)
{  return a < b;  }

inline bool map_function_pointer_greater(int a, int b)
{  return b < a;  }

//Tests a map or multimap whose key_compare is bool(*)(int, int)
template<typename MapType>
bool test_function_pointer_comparator_for()
{
   typedef MapType map_t;
   typedef typename map_t::value_type value_type;
   map_t m(&map_function_pointer_less);
   if(m.key_comp() != &map_function_pointer_less)
      return false;
   m.insert(value_type(3, 3));
   m.insert(value_type(1, 1));
   m.insert(value_type(2, 2));
   if(m.begin()->first != 1 || m.find(2) == m.end() || m.count(2) != 1u)
      return false;
   if(!m.value_comp()(value_type(1, 0), value_type(2, 0)) || m.value_comp()(value_type(2, 0), value_type(1, 0)))
      return false;

   map_t g(&map_function_pointer_greater);
   g.insert(value_type(5, 5));
   g.insert(value_type(7, 7));
   if(g.begin()->first != 7)
      return false;

   //Copy and move construction and assignment
   {
      map_t c(m);
      if(c.key_comp() != &map_function_pointer_less || c.size() != 3u || c.begin()->first != 1)
         return false;
      c = g;
      if(c.key_comp() != &map_function_pointer_greater || c.size() != 2u || c.begin()->first != 7)
         return false;
      map_t mv(boost::move(c));
      if(mv.key_comp() != &map_function_pointer_greater || mv.begin()->first != 7)
         return false;
      c = boost::move(mv);
      if(c.key_comp() != &map_function_pointer_greater || c.begin()->first != 7)
         return false;
      //Inserted elements are ordered with the propagated comparator
      c.insert(value_type(6, 6));
      c.insert(value_type(8, 8));
      if(c.begin()->first != 8 || (--c.end())->first != 5)
         return false;
      map_t ca(g, g.get_allocator());
      if(ca.key_comp() != &map_function_pointer_greater || ca.begin()->first != 7)
         return false;
      map_t ma(boost::move(ca), g.get_allocator());
      if(ma.key_comp() != &map_function_pointer_greater || ma.begin()->first != 7)
         return false;
      const value_type values[] = { value_type(2, 2), value_type(9, 9), value_type(4, 4) };
      map_t r(&values[0], &values[0] + 3, &map_function_pointer_greater);
      if(r.key_comp() != &map_function_pointer_greater || r.begin()->first != 9)
         return false;
   }

   //Swap
   m.swap(g);
   if(m.key_comp() != &map_function_pointer_greater || m.begin()->first != 7)
      return false;
   if(g.key_comp() != &map_function_pointer_less || g.begin()->first != 1)
      return false;

   //Merge from a container with the same comparator type but a different ordering
   g.merge(m);
   if(g.size() != 5u || !m.empty())
      return false;
   if(g.key_comp() != &map_function_pointer_less || m.key_comp() != &map_function_pointer_greater)
      return false;
   int prev = 0;
   for(typename map_t::iterator it = g.begin(), itend = g.end(); it != itend; ++it){
      if(it->first <= prev || it->second != it->first)
         return false;
      prev = it->first;
   }
   return true;
}

template<typename MapType, typename MultimapType>
bool test_function_pointer_comparator()
{
   return test_function_pointer_comparator_for<MapType>()
       && test_function_pointer_comparator_for<MultimapType>();
}

//Replaces the comparator of a map type (map has five template parameters,
//flat_map four)
template<class Container, class NewCompare>
struct rebind_map_compare;

template<template<class, class, class, class> class Cont, class K, class T, class C, class A, class N>
struct rebind_map_compare<Cont<K, T, C, A>, N>
{  typedef Cont<K, T, N, A> type;  };

template<template<class, class, class, class, class> class Cont, class K, class T, class C, class A, class O, class N>
struct rebind_map_compare<Cont<K, T, C, A, O>, N>
{  typedef Cont<K, T, N, A, O> type;  };

//The comparator in the default and move constructors and in the move assignment:
//the destination gets the comparator of the source, the moved-from container
//keeps a valid one, the exceptions of the comparator reach the caller and, with
//a comparator that does not throw, the operations are noexcept.
template<class MapType>
bool test_comparator_operations_for()
{
   {
      typedef typename rebind_map_compare<MapType, tagged_less<int> >::type C;
      C s((tagged_less<int>(7)));
      s.insert(typename C::value_type(3, 3));
      s.insert(typename C::value_type(1, 1));
      s.insert(typename C::value_type(2, 2));
      C d(boost::move(s));
      if(s.key_comp().tag_ != 7 || d.key_comp().tag_ != 7 || d.size() != 3u || d.begin()->first != 1)
         return false;
      s.insert(typename C::value_type(5, 5));
      s.insert(typename C::value_type(4, 4));
      if(s.size() != 2u || s.begin()->first != 4)
         return false;
      C a((tagged_less<int>(3)));
      a.insert(typename C::value_type(9, 9));
      a = boost::move(d);
      if(a.key_comp().tag_ != 7 || a.size() != 3u || a.begin()->first != 1)
         return false;
      if(d.key_comp().tag_ == -1)
         return false;
      d.clear();
      d.insert(typename C::value_type(6, 6));
      if(d.size() != 1u)
         return false;
   }
   #ifndef BOOST_NO_EXCEPTIONS
   {
      typedef throwing_less<int> compare_t;
      typedef typename rebind_map_compare<MapType, compare_t>::type C;
      C src, dst;
      src.insert(typename C::value_type(1, 1));
      compare_t::throw_on_copy = true;
      //The default constructor copies the comparator in some containers
      BOOST_CONTAINER_TRY{
         C c;
         (void)c;
      }
      BOOST_CONTAINER_CATCH(int){}
      BOOST_CONTAINER_CATCH_END
      //The move constructor copies the comparator
      bool thrown = false;
      BOOST_CONTAINER_TRY{
         C c(boost::move(src));
      }
      BOOST_CONTAINER_CATCH(int){
         thrown = true;
      }
      BOOST_CONTAINER_CATCH_END
      //The move assignment copies or swaps the comparator, depending on the
      //container. If the exception passes a noexcept function, std::terminate
      //runs and the test fails.
      BOOST_CONTAINER_TRY{
         dst = boost::move(src);
      }
      BOOST_CONTAINER_CATCH(int){}
      BOOST_CONTAINER_CATCH_END
      compare_t::throw_on_copy = false;
      if(!thrown)
         return false;
   }
   #endif
   #if !defined(BOOST_NO_CXX11_NOEXCEPT)
   {
      using boost::move_detail::declval;
      BOOST_CONTAINER_STATIC_ASSERT(( noexcept(MapType()) ));
      BOOST_CONTAINER_STATIC_ASSERT(( noexcept(MapType(declval<MapType>())) ));
      BOOST_CONTAINER_STATIC_ASSERT(( noexcept(declval<MapType&>() = declval<MapType>()) ));
   }
   #endif
   return true;
}

template<class MapType, class MultimapType>
bool test_comparator_operations()
{
   return test_comparator_operations_for<MapType>()
       && test_comparator_operations_for<MultimapType>();
}

}  //namespace test{
}  //namespace container {
}  //namespace boost{

#include <boost/container/detail/config_end.hpp>

#endif   //#ifndef BOOST_CONTAINER_TEST_MAP_TEST_HEADER
