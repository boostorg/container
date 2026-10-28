//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_SHARED_INPUT_ITERATOR_HPP
#define BOOST_CONTAINER_TEST_SHARED_INPUT_ITERATOR_HPP

#include <boost/container/detail/type_traits.hpp>
#include <boost/container/detail/pair.hpp>
#include <boost/move/utility_core.hpp>
#include <boost/move/detail/meta_utils.hpp>
#include <cstddef>
#include <iterator>
#include <new>
#include <utility>

namespace boost{
namespace container {
namespace test{

//Builds a test value from an int. Pairs need a specialization because
//the members can be move-only and the pair has no constructor from int.
template<class T>
struct shared_input_value_maker
{
   static void construct(void *p, int v)
   {  ::new(p) T(v);  }
};

//The int to char conversion must be explicit to avoid conversion warnings
template<>
struct shared_input_value_maker<char>
{
   static void construct(void *p, int v)
   {  ::new(p) char(static_cast<char>(v));  }
};

template<class T1, class T2>
struct shared_input_value_maker< ::boost::container::dtl::pair<T1, T2> >
{
   static void construct(void *p, int v)
   {
      T1 t1(v);
      T2 t2(v);
      ::new(p) ::boost::container::dtl::pair<T1, T2>(::boost::move(t1), ::boost::move(t2));
   }
};

template<class T1, class T2>
struct shared_input_value_maker< std::pair<T1, T2> >
{
   static void construct(void *p, int v)
   {  ::new(p) std::pair<T1, T2>(T1(v), T2(v));  }
};

//State shared by all copies of a shared_input_iterator
template<class T>
struct shared_input_state
{
   T *cur;
   T *last;
   std::size_t derefs;
   T *last_deref;
   bool repeated;
   bool past_end;

   //Counts a read of *p, to detect reads of the same element and reads past the end
   void read(T *p)
   {
      if(p == last)
         past_end = true;
      if(last_deref == p)
         repeated = true;
      last_deref = p;
      ++derefs;
   }
};

//Reference type of a shared_input_iterator. If Move is true, the elements are
//moved out, so that move-only types can be tested. boost::move_iterator can not
//be used for this: its operator++(int) returns a copy of the iterator, and that
//copy gives the next element.
template<class T, bool Move>
struct shared_input_reference
{
   typedef T& type;

   static type cast(T &r)
   {  return r;  }
};

template<class T>
struct shared_input_reference<T, true>
{
   typedef typename ::boost::move_detail::add_rvalue_reference<T>::type type;

   //The cast is done through a function parameter, because MSVC creates a
   //temporary for static_cast<T&&>(*p) and returns a dangling reference
   static type cast(T &r)
   {  return static_cast<type>(r);  }
};

//Single-pass iterator: all copies share the position, so a copy can not be
//dereferenced after the iterator is incremented. operator++(int) returns a proxy
//that holds the previous element, so "*it++" gives the previous element, as for
//any input iterator. Each dereference is counted, to detect algorithms that read
//an element more than once.
template<class T, bool Move = false>
class shared_input_iterator
{
   public:
   typedef std::input_iterator_tag                             iterator_category;
   typedef T                                                   value_type;
   typedef std::ptrdiff_t                                      difference_type;
   typedef T*                                                  pointer;
   typedef typename shared_input_reference<T, Move>::type      reference;

   class postincrement_proxy
   {
      friend class shared_input_iterator;

      postincrement_proxy(shared_input_state<T> &s, T *p)
         : s_(&s), p_(p)
      {}

      public:
      reference operator*() const
      {
         s_->read(p_);
         return shared_input_reference<T, Move>::cast(*p_);
      }

      private:
      shared_input_state<T> *s_;
      T *p_;
   };

   shared_input_iterator()
      : s_()
   {}

   explicit shared_input_iterator(shared_input_state<T> &s)
      : s_(&s)
   {}

   reference operator*() const
   {
      s_->read(s_->cur);
      return shared_input_reference<T, Move>::cast(*s_->cur);
   }

   T* operator->() const
   {
      s_->read(s_->cur);
      return s_->cur;
   }

   shared_input_iterator& operator++()
   {
      if(s_->cur == s_->last)
         s_->past_end = true;
      else
         ++s_->cur;
      return *this;
   }

   postincrement_proxy operator++(int)
   {
      postincrement_proxy r(*s_, s_->cur);
      ++*this;
      return r;
   }

   friend bool operator==(const shared_input_iterator& a, const shared_input_iterator& b)
   {  return a.done() == b.done();  }

   friend bool operator!=(const shared_input_iterator& a, const shared_input_iterator& b)
   {  return !(a == b);  }

   private:
   bool done() const
   {  return !s_ || s_->cur == s_->last;  }

   shared_input_state<T> *s_;
};

//Owns N values of type T built from an int array and gives single-pass
//iterators to them.
template<class T, std::size_t N>
class shared_input_range
{
   shared_input_range(const shared_input_range&);
   shared_input_range &operator=(const shared_input_range&);

   public:
   typedef shared_input_iterator<T>       iterator;
   typedef shared_input_iterator<T, true> move_iterator;

   explicit shared_input_range(const int *values)
   {
      T *const p = this->data();
      for(std::size_t i = 0; i != N; ++i){
         shared_input_value_maker<T>::construct(p + i, values[i]);
      }
      s_.cur = p;
      s_.last = p + N;
      s_.derefs = 0u;
      s_.last_deref = 0;
      s_.repeated = false;
      s_.past_end = false;
   }

   ~shared_input_range()
   {
      T *const p = this->data();
      for(std::size_t i = 0; i != N; ++i){
         p[i].~T();
      }
   }

   iterator begin()
   {  return iterator(s_);  }

   iterator end()
   {  return iterator();  }

   move_iterator mbegin()
   {  return move_iterator(s_);  }

   move_iterator mend()
   {  return move_iterator();  }

   //True if the whole range was read and each element was dereferenced once
   bool read_once() const
   {  return s_.cur == s_.last && s_.derefs == N && !s_.repeated && !s_.past_end;  }

   private:
   T *data()
   {  return static_cast<T*>(static_cast<void*>(&storage_));  }

   typename ::boost::container::dtl::aligned_storage
      < sizeof(T)*N, ::boost::container::dtl::alignment_of<T>::value>::type storage_;
   shared_input_state<T> s_;
};

}  //namespace test{
}  //namespace container {
}  //namespace boost{

#endif //BOOST_CONTAINER_TEST_SHARED_INPUT_ITERATOR_HPP
