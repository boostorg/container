////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2006. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_LIST_TEST_HEADER
#define BOOST_CONTAINER_TEST_LIST_TEST_HEADER

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/iterator.hpp>
#include <boost/container/detail/compare_functors.hpp>
#include "check_equal_containers.hpp"
#include "print_container.hpp"
#include "input_from_forward_iterator.hpp"
#include "shared_input_iterator.hpp"
#include <boost/intrusive/detail/mpl.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/move/iterator.hpp>
#include <boost/move/make_unique.hpp>

#include <list>
#include <functional>   //std::greater
#include <cstddef>   //std::size_t

namespace boost{
namespace container {
namespace test{

template<class V1, class V2>
bool list_copyable_only(V1 &, V2 &, boost::container::dtl::false_type)
{
   return true;
}

//Function to check if both sets are equal
template<class V1, class V2>
bool list_copyable_only(V1 &boostlist, V2 &stdlist, boost::container::dtl::true_type)
{
   typedef typename V1::value_type IntType;
   boostlist.insert(boostlist.end(), 50u, IntType(1));
   stdlist.insert(stdlist.end(), 50u, 1);
   if(!test::CheckEqualContainers(boostlist, stdlist)) return false;

   {
      IntType move_me(1);
      boostlist.insert(boostlist.begin(), 50u, boost::move(move_me));
      stdlist.insert(stdlist.begin(), 50u, 1);
      if(!test::CheckEqualContainers(boostlist, stdlist)) return false;
   }
   {
      IntType move_me(2);
      boostlist.assign(boostlist.size()/2, boost::move(move_me));
      stdlist.assign(stdlist.size()/2, 2);
      if(!test::CheckEqualContainers(boostlist, stdlist)) return false;
   }
   {
      IntType move_me(3);
      boostlist.assign(boostlist.size()*3-1, boost::move(move_me));
      stdlist.assign(stdlist.size()*3-1, 3);
      if(!test::CheckEqualContainers(boostlist, stdlist)) return false;
   }

   {
      IntType copy_me(3);
      const IntType ccopy_me(3);
      boostlist.push_front(copy_me);
      stdlist.push_front(int(3));
      boostlist.push_front(ccopy_me);
      stdlist.push_front(int(3));
      if(!test::CheckEqualContainers(boostlist, stdlist)) return false;
   }
   {  //List(const List &)
      ::boost::movelib::unique_ptr<V1> const pv1 = ::boost::movelib::make_unique<V1>(boostlist);
      ::boost::movelib::unique_ptr<V2> const pv2 = ::boost::movelib::make_unique<V2>(stdlist);

      V1 &v1 = *pv1;
      V2 &v2 = *pv2;

      boostlist.clear();
      stdlist.clear();
      boostlist.assign(v1.begin(), v1.end());
      stdlist.assign(v2.begin(), v2.end());
      if(!test::CheckEqualContainers(boostlist, stdlist)) return 1;

      //test erase/erase_if
      IntType aux_vect[50];
      for(int i = 0; i < 50; ++i){
         aux_vect[i] = i;
      }
      int aux_vect2[50];
      for(int i = 0; i < 50; ++i){
         aux_vect2[i] = i;
      }

      boostlist.clear();
      stdlist.clear();
      boostlist.insert(boostlist.end()
                     ,boost::make_move_iterator(&aux_vect[0])
                     ,boost::make_move_iterator(aux_vect + 50));
      stdlist.insert(stdlist.end(), aux_vect2, aux_vect2 + 50);
      if(!test::CheckEqualContainers(boostlist, stdlist)) return false;

      //erase
      if (1 != erase(boostlist, 25))
         return 1;
      if (0 != erase(boostlist, 25))
         return 1;

      stdlist.erase(boost::container::find(stdlist.begin(), stdlist.end(), 25));
      if(!test::CheckEqualContainers(boostlist, stdlist)) return false;

      //erase_if
      if (1 != erase_if(boostlist, equal_to_value<int>(24)))
         return 1;
      if (0 != erase_if(boostlist, equal_to_value<int>(24)))
         return 1;
      stdlist.erase(boost::container::find(stdlist.begin(), stdlist.end(), 24));
      if(!test::CheckEqualContainers(boostlist, stdlist)) return false;

      //remove
      if (1 != boostlist.remove(IntType(23)))
         return 1;
      if (0 != boostlist.remove(IntType(23)))
         return 1;

      stdlist.erase(boost::container::find(stdlist.begin(), stdlist.end(), 23));
      if(!test::CheckEqualContainers(boostlist, stdlist)) return false;

      //remove_if
      if (1 != boostlist.remove_if(equal_to_value<int>(22)))
         return 1;
      if (0 != boostlist.remove_if(equal_to_value<int>(22)))
         return 1;
      stdlist.erase(boost::container::find(stdlist.begin(), stdlist.end(), 22));
      if(!test::CheckEqualContainers(boostlist, stdlist)) return false;

   }
   {  //List(const List &, alloc)
      ::boost::movelib::unique_ptr<V1> const pv1 = ::boost::movelib::make_unique<V1>(boostlist, typename V1::allocator_type());
      ::boost::movelib::unique_ptr<V2> const pv2 = ::boost::movelib::make_unique<V2>(stdlist);

      V1 &v1 = *pv1;
      V2 &v2 = *pv2;

      boostlist.clear();
      stdlist.clear();
      boostlist.assign(v1.begin(), v1.end());
      stdlist.assign(v2.begin(), v2.end());
      if(!test::CheckEqualContainers(boostlist, stdlist)) return 1;
   }

   return true;
}

template<bool DoublyLinked>
struct list_push_data_function
{
   template<class MyBoostList, class MyStdList>
   static int execute(std::size_t max, MyBoostList &boostlist, MyStdList &stdlist)
   {
      typedef typename MyBoostList::value_type IntType;
      for(std::size_t i = 0; i < max; ++i){
         IntType move_me((int)i);
         boostlist.push_back(boost::move(move_me));
         stdlist.push_back((int)i);
         boostlist.push_front(IntType(int(i)));
         stdlist.push_front(int(i));
      }
      if(!CheckEqualContainers(boostlist, stdlist))
         return 1;
      return 0;
   }
};

template<>
struct list_push_data_function<false>
{
   template<class MyBoostList, class MyStdList>
   static int execute(std::size_t max, MyBoostList &boostlist, MyStdList &stdlist)
   {
      typedef typename MyBoostList::value_type IntType;
      for(std::size_t i = 0; i < max; ++i){
         IntType move_me((int)i);
         boostlist.push_front(boost::move(move_me));
         stdlist.push_front((int)i);
         boostlist.push_front(IntType(int(i)));
         stdlist.push_front(int(i));
      }
      if(!CheckEqualContainers(boostlist, stdlist))
         return 1;
      return 0;
   }
};

template<bool DoublyLinked>
struct list_pop_back_function
{
   template<class MyStdList, class MyBoostList>
   static int execute(MyBoostList &boostlist, MyStdList &stdlist)
   {
      boostlist.pop_back();
      stdlist.pop_back();
      if(!CheckEqualContainers(boostlist, stdlist))
         return 1;
      return 0;
   }
};

template<>
struct list_pop_back_function<false>
{
   template<class MyStdList, class MyBoostList>
   static int execute(MyBoostList &boostlist, MyStdList &stdlist)
   {
      (void)boostlist; (void)stdlist;
      return 0;
   }
};

template<class
        , bool>
int list_move_assignable_only(boost::container::dtl::false_type)
{
   return 0;
}

//Function to check if both sets are equal
template < class MyBoostList
         , bool  DoublyLinked >
int list_move_assignable_only(boost::container::dtl::true_type)
{
   typedef std::list<int> MyStdList;
   typedef typename MyBoostList::value_type IntType;
   const std::size_t max = 100u;
   typedef list_push_data_function<DoublyLinked> push_data_t;

   {
      ::boost::movelib::unique_ptr<MyStdList> const stdlistp = ::boost::movelib::make_unique<MyStdList>(100u);
      ::boost::movelib::unique_ptr<MyBoostList> const boostlistp = ::boost::movelib::make_unique<MyBoostList>(100u);
      MyBoostList& boostlist = *boostlistp;
      MyStdList& stdlist = *stdlistp;

      if (push_data_t::execute(max, boostlist, stdlist)) {
         return 1;
      }

      IntType aux_vect[50];
      for (int i = 0; i < 50; ++i) {
         aux_vect[i] = -1;
      }
      int aux_vect2[50];
      for (int i = 0; i < 50; ++i) {
         aux_vect2[i] = -1;
      }
      boostlist.assign(boost::make_move_iterator(&aux_vect[0])
         , boost::make_move_iterator(&aux_vect[50]));
      stdlist.assign(&aux_vect2[0], &aux_vect2[50]);
      if (!CheckEqualContainers(boostlist, stdlist)) return 1;

      for (int i = 0; i < 50; ++i) {
         aux_vect[i] = -1;
      }

      for (int i = 0; i < 50; ++i) {
         aux_vect2[i] = -1;
      }
      boostlist.assign(boost::make_move_iterator(make_input_from_forward_iterator(&aux_vect[0]))
         , boost::make_move_iterator(make_input_from_forward_iterator(&aux_vect[50])));
      stdlist.assign(&aux_vect2[0], &aux_vect2[50]);
      if (!CheckEqualContainers(boostlist, stdlist)) return 1;
   }
   return 0;
}

template<class MyBoostList, class MyStdList>
void list_single_pass_fill(MyBoostList &boostlist, MyStdList &stdlist, int n)
{
   typedef typename MyBoostList::value_type IntType;
   for(int i = 0; i != n; ++i){
      IntType move_me(100 + i);
      boostlist.insert(boostlist.end(), boost::move(move_me));
      stdlist.push_back(100 + i);
   }
}

//True if the list has before_begin(), which tells that it has insert_after
BOOST_INTRUSIVE_HAS_MEMBER_FUNC_CALLED(list_has_before_begin, before_begin)

template<bool HasInsertAfter>
struct list_single_pass_insert_after
{
   template<class MyBoostList, std::size_t N>
   static int execute(const int (&)[N])
   {  return 0;  }
};

template<>
struct list_single_pass_insert_after<true>
{
   //insert_after(pos, It, It) in an empty list, at the beginning, in the middle and at the end
   template<class MyBoostList, std::size_t N>
   static int execute(const int (&values)[N])
   {
      typedef typename MyBoostList::value_type IntType;
      for(int n = 0; n != 4; ++n){
         const int size = n ? 10 : 0;
         const int pos  = n ? (n - 1)*5 : 0;
         MyBoostList boostlist;
         std::list<int> stdlist;
         list_single_pass_fill(boostlist, stdlist, size);
         typename MyBoostList::iterator it = boostlist.before_begin();
         std::list<int>::iterator stdit = stdlist.begin();
         for(int i = 0; i != pos; ++i, ++it, ++stdit){}
         shared_input_range<IntType, N> r(values);
         boostlist.insert_after(it, r.mbegin(), r.mend());
         stdlist.insert(stdit, values, values + N);
         if(!r.read_once()) return 1;
         if(!test::CheckEqualContainers(boostlist, stdlist)) return 1;
      }
      return 0;
   }
};

//Range functions with a single-pass iterator whose copies share the position.
//The iterator moves the values, so that move-only types are also tested.
template<class MyBoostList, std::size_t N>
int list_single_pass_test_n()
{
   typedef typename MyBoostList::value_type IntType;
   int values[N];
   for(std::size_t i = 0; i != N; ++i)
      values[i] = int(i);
   const std::list<int> stdlist_values(values, values + N);
   {  //List(It, It)
      shared_input_range<IntType, N> r(values);
      const MyBoostList boostlist(r.mbegin(), r.mend());
      if(!r.read_once()) return 1;
      if(!test::CheckEqualContainers(boostlist, stdlist_values)) return 1;
   }
   {  //List(It, It, alloc)
      shared_input_range<IntType, N> r(values);
      const MyBoostList boostlist(r.mbegin(), r.mend(), typename MyBoostList::allocator_type());
      if(!r.read_once()) return 1;
      if(!test::CheckEqualContainers(boostlist, stdlist_values)) return 1;
   }
   //insert(pos, It, It) in an empty list, at the beginning, in the middle and at the end
   for(int n = 0; n != 4; ++n){
      const int size = n ? 10 : 0;
      const int pos  = n ? (n - 1)*5 : 0;
      MyBoostList boostlist;
      std::list<int> stdlist;
      list_single_pass_fill(boostlist, stdlist, size);
      typename MyBoostList::iterator it = boostlist.begin();
      std::list<int>::iterator stdit = stdlist.begin();
      for(int i = 0; i != pos; ++i, ++it, ++stdit){}
      shared_input_range<IntType, N> r(values);
      boostlist.insert(it, r.mbegin(), r.mend());
      stdlist.insert(stdit, values, values + N);
      if(!r.read_once()) return 1;
      if(!test::CheckEqualContainers(boostlist, stdlist)) return 1;
   }
   return list_single_pass_insert_after<list_has_before_begin<MyBoostList>::value>::template execute<MyBoostList>(values);
}

//assign(It, It) in an empty list and in a list with elements
template<class MyBoostList, std::size_t N>
int list_single_pass_assign_test_n()
{
   typedef typename MyBoostList::value_type IntType;
   int values[N];
   for(std::size_t i = 0; i != N; ++i)
      values[i] = int(i);
   for(int n = 0; n != 2; ++n){
      MyBoostList boostlist;
      std::list<int> stdlist;
      list_single_pass_fill(boostlist, stdlist, n*10);
      shared_input_range<IntType, N> r(values);
      boostlist.assign(r.mbegin(), r.mend());
      stdlist.assign(values, values + N);
      if(!r.read_once()) return 1;
      if(!test::CheckEqualContainers(boostlist, stdlist)) return 1;
   }
   return 0;
}

//assign assigns to the existing elements, so the values must be move assignable
template<class MyBoostList>
int list_single_pass_assign(boost::container::dtl::false_type /*move_assignable*/)
{
   return 0;
}

template<class MyBoostList>
int list_single_pass_assign(boost::container::dtl::true_type /*move_assignable*/)
{
   if(0 != list_single_pass_assign_test_n<MyBoostList, 3u>())  return 1;
   if(0 != list_single_pass_assign_test_n<MyBoostList, 50u>()) return 1;
   return 0;
}

template<class MyBoostList>
int list_single_pass_test()
{
   typedef typename MyBoostList::value_type IntType;
   if(0 != list_single_pass_test_n<MyBoostList, 3u>())  return 1;
   if(0 != list_single_pass_test_n<MyBoostList, 50u>()) return 1;
   return list_single_pass_assign<MyBoostList>
      (dtl::bool_<boost::container::test::is_move_assignable<IntType>::value>());
}

template<class MyBoostList
        ,bool  DoublyLinked>
int list_test (bool copied_allocators_equal = true)
{
   typedef std::list<int> MyStdList;
   typedef typename MyBoostList::value_type IntType;
   const std::size_t max = 100u;
   typedef list_push_data_function<DoublyLinked> push_data_t;

   {  //List(n)
      ::boost::movelib::unique_ptr<MyBoostList> const pboostlist = ::boost::movelib::make_unique<MyBoostList>(100u);
      ::boost::movelib::unique_ptr<MyStdList> const pstdlist = ::boost::movelib::make_unique<MyStdList>(100u);
      if(!test::CheckEqualContainers(*pboostlist, *pstdlist)) return 1;
   }
   {  //List(n, alloc)
      ::boost::movelib::unique_ptr<MyBoostList> const pboostlist = ::boost::movelib::make_unique<MyBoostList>(100u, typename MyBoostList::allocator_type());
      ::boost::movelib::unique_ptr<MyStdList> const pstdlist = ::boost::movelib::make_unique<MyStdList>(100u);
      if(!test::CheckEqualContainers(*pboostlist, *pstdlist)) return 1;
   }
   {  //List(List &&)
      ::boost::movelib::unique_ptr<MyStdList> const stdlistp = ::boost::movelib::make_unique<MyStdList>(100u);
      ::boost::movelib::unique_ptr<MyBoostList> const boostlistp = ::boost::movelib::make_unique<MyBoostList>(100u);
      ::boost::movelib::unique_ptr<MyBoostList> const boostlistp2 = ::boost::movelib::make_unique<MyBoostList>(::boost::move(*boostlistp));
      if(!test::CheckEqualContainers(*boostlistp2, *stdlistp)) return 1;
   }
   {  //List(List &&, alloc)
      ::boost::movelib::unique_ptr<MyStdList> const stdlistp = ::boost::movelib::make_unique<MyStdList>(100u);
      ::boost::movelib::unique_ptr<MyBoostList> const boostlistp = ::boost::movelib::make_unique<MyBoostList>(100u);
      ::boost::movelib::unique_ptr<MyBoostList> const boostlistp2 = ::boost::movelib::make_unique<MyBoostList>
         (::boost::move(*boostlistp), typename MyBoostList::allocator_type());
      if(!test::CheckEqualContainers(*boostlistp2, *stdlistp)) return 1;
   }
   {  //List operator=(List &&)
      ::boost::movelib::unique_ptr<MyStdList> const stdlistp = ::boost::movelib::make_unique<MyStdList>(100u);
      ::boost::movelib::unique_ptr<MyBoostList> const boostlistp = ::boost::movelib::make_unique<MyBoostList>(100u);
      ::boost::movelib::unique_ptr<MyBoostList> const boostlistp2 = ::boost::movelib::make_unique<MyBoostList>();
      *boostlistp2 = ::boost::move(*boostlistp);
      if (!test::CheckEqualContainers(*boostlistp2, *stdlistp)) return 1;
   }


   ::boost::movelib::unique_ptr<MyBoostList> const pboostlist = ::boost::movelib::make_unique<MyBoostList>();
   ::boost::movelib::unique_ptr<MyStdList>   const pstdlist   = ::boost::movelib::make_unique<MyStdList>();

   MyBoostList &boostlist = *pboostlist;
   MyStdList  &stdlist    = *pstdlist;

   if(push_data_t::execute(max, boostlist, stdlist)){
      return 1;
   }

   boostlist.erase(boostlist.begin()++);
   stdlist.erase(stdlist.begin()++);
   if(!CheckEqualContainers(boostlist, stdlist)) return 1;

   if(list_pop_back_function<DoublyLinked>::execute(boostlist, stdlist)){
      return 1;
   }

   boostlist.pop_front();
   stdlist.pop_front();
   if(!CheckEqualContainers(boostlist, stdlist)) return 1;

   if (0 != list_move_assignable_only<MyBoostList, DoublyLinked>(dtl::bool_<boost::container::test::is_move_assignable<IntType>::value>()))
      return 1;
   
   if(copied_allocators_equal){
      boostlist.sort();
      stdlist.sort();
      if(!CheckEqualContainers(boostlist, stdlist)) return 1;
   }

   boostlist.reverse();
   stdlist.reverse();
   if(!CheckEqualContainers(boostlist, stdlist)) return 1;

   boostlist.reverse();
   stdlist.reverse();
   if(!CheckEqualContainers(boostlist, stdlist)) return 1;

   {
      IntType aux_vect[50];
      for(int i = 0; i < 50; ++i){
         aux_vect[i] = -1;
      }
      int aux_vect2[50];
      for(int i = 0; i < 50; ++i){
         aux_vect2[i] = -1;
      }
      typename MyBoostList::iterator old_begin = boostlist.begin();
      typename MyBoostList::iterator it_insert =
         boostlist.insert(boostlist.begin()
                     ,boost::make_move_iterator(&aux_vect[0])
                     ,boost::make_move_iterator(&aux_vect[50]));
      if(it_insert != boostlist.begin() || boost::container::iterator_distance(it_insert, old_begin) != 50)
         return 1;

      stdlist.insert(stdlist.begin(), &aux_vect2[0], &aux_vect2[50]);
      if(!CheckEqualContainers(boostlist, stdlist))
         return 1;

      for(int i = 0; i < 50; ++i){
         aux_vect[i] = -1;
      }

      for(int i = 0; i < 50; ++i){
         aux_vect2[i] = -1;
      }

      old_begin = boostlist.begin();
      it_insert = boostlist.insert(boostlist.end()
                     ,boost::make_move_iterator(make_input_from_forward_iterator(&aux_vect[0]))
                     ,boost::make_move_iterator(make_input_from_forward_iterator(&aux_vect[50])));
      if(boost::container::iterator_distance(it_insert, boostlist.end()) != 50)
         return 1;
      stdlist.insert(stdlist.end(), &aux_vect2[0], &aux_vect2[50]);
      if(!CheckEqualContainers(boostlist, stdlist))
         return 1;
   }

   {
      std::size_t old_sz = boostlist.size();
      std::size_t bremoved = boostlist.unique();
      if (bremoved != (old_sz - boostlist.size()))
         return 1;
      stdlist.unique();
   }
   if(!CheckEqualContainers(boostlist, stdlist))
      return 1;

   if(copied_allocators_equal){
      boostlist.sort(std::greater<IntType>());
      stdlist.sort(std::greater<int>());
      if(!CheckEqualContainers(boostlist, stdlist))
         return 1;
   }

   for(std::size_t i = 0; i < max; ++i){
      IntType new_int((int)i);
      boostlist.insert(boostlist.end(), boost::move(new_int));
      stdlist.insert(stdlist.end(), (int)i);
      if(!test::CheckEqualContainers(boostlist, stdlist)) return 1;
   }
   if(!test::CheckEqualContainers(boostlist, stdlist)) return 1;

   boostlist.resize(25);
   stdlist.resize(25);
   boostlist.resize(50);
   stdlist.resize(50);
   boostlist.resize(0);
   stdlist.resize(0);
   if(!CheckEqualContainers(boostlist, stdlist))
      return 1;

   //some comparison operators
   if(!(boostlist == boostlist))
      return 1;
   if(boostlist != boostlist)
      return 1;
   if(boostlist < boostlist)
      return 1;
   if(boostlist > boostlist)
      return 1;
   if(!(boostlist <= boostlist))
      return 1;
   if(!(boostlist >= boostlist))
      return 1;

   if(push_data_t::execute(max, boostlist, stdlist)){
      return 1;
   }
   {
      MyBoostList otherboostlist(boostlist.get_allocator());
      MyStdList otherstdlist;

      std::size_t listsize = boostlist.size();

      if(push_data_t::execute(listsize, boostlist, stdlist)){
         return 1;
      }

      if(copied_allocators_equal){
         boostlist.splice(boostlist.begin(), otherboostlist);
         stdlist.splice(stdlist.begin(), otherstdlist);
         if(!CheckEqualContainers(boostlist, stdlist))
            return 1;
      }

      listsize = boostlist.size();

      if(push_data_t::execute(listsize, boostlist, stdlist)){
         return 1;
      }

      if(push_data_t::execute(listsize, otherboostlist, otherstdlist)){
         return 1;
      }

      if(copied_allocators_equal){
         boostlist.sort(std::greater<IntType>());
         stdlist.sort(std::greater<int>());
         if(!CheckEqualContainers(boostlist, stdlist))
            return 1;

         otherboostlist.sort(std::greater<IntType>());
         otherstdlist.sort(std::greater<int>());
         if(!CheckEqualContainers(otherboostlist, otherstdlist))
            return 1;

         boostlist.merge(otherboostlist, std::greater<IntType>());
         stdlist.merge(otherstdlist, std::greater<int>());
         if(!CheckEqualContainers(boostlist, stdlist))
            return 1;
      }

      if(!list_copyable_only(boostlist, stdlist
                     ,dtl::bool_<boost::container::test::is_copyable<IntType>::value>())){
         return 1;
      }
   }

   if (0 != list_single_pass_test<MyBoostList>())
      return 1;
   return 0;
}

template<class List>
bool test_list_methods_with_initializer_list_as_argument_for()
{
#if !defined(BOOST_NO_CXX11_HDR_INITIALIZER_LIST)
   const std::initializer_list<int> il = {5, 10, 15};
   const List expected_list(il.begin(), il.end());
   {
      List sl = il;
      if(sl != expected_list)
         return false;
   }

   {
      List sl = {1, 2};
      sl = il;
      if(sl != expected_list)
         return false;
   }
   {
      List sl({ 1, 2 }, typename List::allocator_type());
      sl = il;
      if (sl != expected_list)
         return false;
   }
   {
      List sl = {4, 5};
      sl.assign(il);
      if(sl != expected_list)
         return false;
   }

   {
      List sl = {15};
      sl.insert(sl.cbegin(), {5, 10});
      if(sl != expected_list)
         return false;
   }

   {
       List sl = {5};
       sl.insert_after(sl.cbegin(), {10, 15});
       if(sl != expected_list)
          return false;
   }
   return true;
#endif
   return true;
}


}  //namespace test{
}  //namespace container {
}  //namespace boost{

#include <boost/container/detail/config_end.hpp>

#endif
