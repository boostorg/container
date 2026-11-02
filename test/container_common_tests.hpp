//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2014. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_CONTAINER_COMMON_TESTS_HPP
#define BOOST_CONTAINER_TEST_CONTAINER_COMMON_TESTS_HPP

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/throw_exception.hpp>
#include "lightweight_test.hpp"

namespace boost{
namespace container {
namespace test{


template<class Container>
const Container &as_const(Container &c)
{  return c;   }

//nth, index_of
template<class Container>
bool test_nth_index_of(Container &c)
{
   typename Container::iterator it;
   typename Container::const_iterator cit;
   typename Container::size_type sz, csz;
   typedef typename Container::difference_type difference_type;

   //index 0
   it = c.nth(0);
   sz = c.index_of(it);
   cit = (as_const)(c).nth(0);
   csz = (as_const)(c).index_of(cit);

   if(it != c.begin())
      return false;
   if(cit != c.cbegin())
      return false;
   if(sz != 0)
      return false;
   if(csz != 0)
      return false;

   //index size()/2
   const typename Container::size_type sz_div_2 = c.size()/2;
   it = c.nth(sz_div_2);
   sz = c.index_of(it);
   cit = (as_const)(c).nth(sz_div_2);
   csz = (as_const)(c).index_of(cit);

   if(it != (c.begin()+difference_type(sz_div_2)))
      return false;
   if(cit != (c.cbegin()+difference_type(sz_div_2)))
      return false;
   if(sz != sz_div_2)
      return false;
   if(csz != sz_div_2)
      return false;

   //index size()
   it = c.nth(c.size());
   sz = c.index_of(it);
   cit = (as_const)(c).nth(c.size());
   csz = (as_const)(c).index_of(cit);

   if(it != c.end())
      return false;
   if(cit != c.cend())
      return false;
   if(sz != c.size())
      return false;
   if(csz != c.size())
      return false;
   return true;
}

//With a small stored_size the container grows to max_size() and max_size()
//elements can be stored. Insertions in the middle can stop up to "slack"
//elements before max_size() (deque: the free positions of the first block
//can not be moved)
template<class Container>
void test_stored_size_reach_back(typename Container::size_type slack)
{
   #ifndef BOOST_NO_EXCEPTIONS
   typedef typename Container::size_type size_type;
   typedef typename Container::value_type value_type;
   {
      Container c;
      const size_type max = c.max_size();
      while(c.size() != max)
         c.push_back(value_type(c.size()));
      BOOST_TEST_THROWS(c.push_back(value_type(0)), boost::container::length_error_t);
      BOOST_TEST(c.size() == max);
      BOOST_TEST(c.front() == value_type(0) && c.back() == value_type(max - 1u));
   }
   {
      Container c;
      c.resize(c.max_size());
      BOOST_TEST(c.size() == c.max_size());
   }
   {
      Container c;
      const size_type max = c.max_size();
      //Only the last insertions are in the middle (they are O(n))
      while(c.size() + 1000u < max)
         c.push_back(value_type(0));
      bool thrown = false;
      while(!thrown && c.size() != max){
         try{  c.insert(c.begin() + typename Container::difference_type(c.size()/2u), value_type(c.size()));  }
         catch(const boost::container::length_error_t &){  thrown = true;  }
      }
      BOOST_TEST(c.size() + slack >= max);
   }
   #else
   (void)slack;
   #endif   //BOOST_NO_EXCEPTIONS
}

//push_front and a fifo pattern (two push_back, one pop_front) can stop up to
//"slack" elements before max_size()
template<class Container>
void test_stored_size_reach_front(typename Container::size_type slack)
{
   #ifndef BOOST_NO_EXCEPTIONS
   typedef typename Container::size_type size_type;
   typedef typename Container::value_type value_type;
   const size_type min_size = size_type(Container().max_size() - slack);
   {
      Container c;
      bool thrown = false;
      while(!thrown && c.size() != c.max_size()){
         try{  c.push_front(value_type(c.size()));  }
         catch(const boost::container::length_error_t &){  thrown = true;  }
      }
      BOOST_TEST(c.size() >= min_size);
   }
   {
      Container c;
      bool thrown = false;
      while(!thrown && c.size() != c.max_size()){
         try{
            c.push_back(value_type(1));
            if(c.size() != c.max_size()){
               c.push_back(value_type(2));
               c.pop_front();
            }
         }
         catch(const boost::container::length_error_t &){  thrown = true;  }
      }
      BOOST_TEST(c.size() >= min_size);
   }
   #else
   (void)slack;
   #endif   //BOOST_NO_EXCEPTIONS
}

}  //namespace test{
}  //namespace container {
}  //namespace boost{

#include <boost/container/detail/config_end.hpp>

#endif //#ifndef BOOST_CONTAINER_TEST_CONTAINER_COMMON_TESTS_HPP
