///////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
///////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_TEST_THROWING_UTILITIES_HPP
#define BOOST_CONTAINER_TEST_THROWING_UTILITIES_HPP

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/move/utility_core.hpp>
#include <cstddef>

namespace boost {
namespace container {
namespace test {

//A standard allocator whose default constructor is not noexcept.
template<class T>
struct throwing_default_ctor_allocator
{
   typedef T value_type;

   throwing_default_ctor_allocator() BOOST_NOEXCEPT_IF(false) {}
   template<class U>
   throwing_default_ctor_allocator(const throwing_default_ctor_allocator<U> &) BOOST_NOEXCEPT_OR_NOTHROW {}

   T* allocate(std::size_t n)
   {  return static_cast<T*>(::operator new(n*sizeof(T)));  }
   void deallocate(T *p, std::size_t) BOOST_NOEXCEPT_OR_NOTHROW
   {  ::operator delete(p);  }

   friend bool operator==(const throwing_default_ctor_allocator &, const throwing_default_ctor_allocator &)
   {  return true;  }
   friend bool operator!=(const throwing_default_ctor_allocator &, const throwing_default_ctor_allocator &)
   {  return false;  }
};

//A less-than comparator with a state. A moved-from comparator has tag -1, so a
//container that keeps a moved-from comparator is detected.
template<class T>
class tagged_less
{
   BOOST_COPYABLE_AND_MOVABLE(tagged_less)

   public:
   explicit tagged_less(int tag = 0) : tag_(tag) {}
   tagged_less(const tagged_less &o) : tag_(o.tag_) {}
   tagged_less(BOOST_RV_REF(tagged_less) o) : tag_(o.tag_) {  o.tag_ = -1;  }
   tagged_less &operator=(BOOST_COPY_ASSIGN_REF(tagged_less) o)
   {  tag_ = o.tag_; return *this;  }
   tagged_less &operator=(BOOST_RV_REF(tagged_less) o)
   {  tag_ = o.tag_; o.tag_ = -1; return *this;  }

   bool operator()(const T &l, const T &r) const
   {  return l < r;  }

   int tag_;
};

}  //namespace test {
}  //namespace container {
}  //namespace boost {

//Element types whose copy or move operations throw an int when a static
//flag is set. They test that the exceptions of the element operations reach
//the caller, so they are only available with exceptions.
#ifndef BOOST_NO_EXCEPTIONS

namespace boost {
namespace container {
namespace test {

//The flags are static members of a class template, so that this header can
//be included in several translation units. Each type has its own flags.
template<class Derived>
struct throw_on_move_flag
{
   static bool throw_on_move;
};

template<class Derived>
bool throw_on_move_flag<Derived>::throw_on_move = false;

template<class Derived>
struct throw_on_copy_flag
{
   static bool throw_on_copy;
};

template<class Derived>
bool throw_on_copy_flag<Derived>::throw_on_copy = false;

//The move constructor throws if throw_on_move is set. The move and copy
//assignments never throw.
class throwing_move_ctor_value
   : public throw_on_move_flag<throwing_move_ctor_value>
{
   BOOST_COPYABLE_AND_MOVABLE(throwing_move_ctor_value)

   public:
   explicit throwing_move_ctor_value(int v = 0) : m_v(v) {}
   throwing_move_ctor_value(const throwing_move_ctor_value &o) : m_v(o.m_v) {}
   throwing_move_ctor_value(BOOST_RV_REF(throwing_move_ctor_value) o) : m_v(o.m_v)
   {  if(throw_on_move) throw int(1);  }
   throwing_move_ctor_value &operator=(BOOST_COPY_ASSIGN_REF(throwing_move_ctor_value) o) BOOST_NOEXCEPT_OR_NOTHROW
   {  m_v = o.m_v; return *this;  }
   throwing_move_ctor_value &operator=(BOOST_RV_REF(throwing_move_ctor_value) o) BOOST_NOEXCEPT_OR_NOTHROW
   {  m_v = o.m_v; return *this;  }

   int m_v;
};

//A copy-only type (a move is a copy): the copy constructor and the copy
//assignment throw if throw_on_copy is set.
class throwing_copy_value
   : public throw_on_copy_flag<throwing_copy_value>
{
   public:
   explicit throwing_copy_value(int v = 0) : m_v(v) {}
   throwing_copy_value(const throwing_copy_value &o) : m_v(o.m_v)
   {  if(throw_on_copy) throw int(1);  }
   throwing_copy_value &operator=(const throwing_copy_value &o)
   {
      if(throw_on_copy) throw int(1);
      m_v = o.m_v;
      return *this;
   }

   int m_v;
};

//A less-than comparator whose copy constructor, copy assignment and swap throw
//if throw_on_copy is set. The default constructor and the moves do not throw.
template<class T>
class throwing_less
   : public throw_on_copy_flag< throwing_less<T> >
{
   typedef throw_on_copy_flag< throwing_less<T> > flag_t;

   public:
   throwing_less() BOOST_NOEXCEPT_OR_NOTHROW {}
   throwing_less(const throwing_less &) : flag_t()
   {  if(flag_t::throw_on_copy) throw int(1);  }
   throwing_less &operator=(const throwing_less &)
   {
      if(flag_t::throw_on_copy) throw int(1);
      return *this;
   }
   #if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
   throwing_less(throwing_less &&) BOOST_NOEXCEPT_OR_NOTHROW : flag_t() {}
   throwing_less &operator=(throwing_less &&) BOOST_NOEXCEPT_OR_NOTHROW {  return *this;  }
   #endif

   friend void swap(throwing_less &, throwing_less &)
   {  if(throwing_less::throw_on_copy) throw int(1);  }

   bool operator()(const T &l, const T &r) const
   {  return l < r;  }
};

}  //namespace test {
}  //namespace container {
}  //namespace boost {

#endif   //BOOST_NO_EXCEPTIONS

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_TEST_THROWING_UTILITIES_HPP
