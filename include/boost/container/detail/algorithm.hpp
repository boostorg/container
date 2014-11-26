//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2014-2014.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_ALGORITHM_HPP
#define BOOST_CONTAINER_DETAIL_ALGORITHM_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

namespace boost {
namespace container {

template<class InputIt1, class InputIt2>
bool algo_equal(InputIt1 first1, InputIt1 last1, InputIt2 first2)
{
   for (; first1 != last1; ++first1, ++first2) {
      if (!(*first1 == *first2)) {
         return false;
      }
   }
   return true;
}

template<class InputIt1, class InputIt2, class BinaryPredicate>
bool algo_equal(InputIt1 first1, InputIt1 last1, InputIt2 first2, BinaryPredicate p)
{
    for (; first1 != last1; ++first1, ++first2) {
        if (!p(*first1, *first2)) {
            return false;
        }
    }
    return true;
}

template <class InputIterator1, class InputIterator2>
  bool algo_lexicographical_compare (InputIterator1 first1, InputIterator1 last1,
                                     InputIterator2 first2, InputIterator2 last2)
{
   while (first1 != last1){
      if (first2 == last2 || *first2 < *first1) return false;
      else if (*first1 < *first2) return true;
      ++first1; ++first2;
   }
   return (first2 != last2);
}

}  //namespace container {
}  //namespace boost {

#endif   //#ifndef BOOST_CONTAINER_DETAIL_ALGORITHM_HPP
