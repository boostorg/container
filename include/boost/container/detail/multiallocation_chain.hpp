//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_MULTIALLOCATION_CHAIN_HPP
#define BOOST_CONTAINER_DETAIL_MULTIALLOCATION_CHAIN_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
// container
#include <boost/container/container_fwd.hpp>
// container/detail
#include <boost/move/detail/to_raw_pointer.hpp>
#include <boost/container/detail/type_traits.hpp>
#include <boost/container/detail/placement_new.hpp>
#include <boost/container/detail/iterator.hpp>
// intrusive
#include <boost/intrusive/detail/slist_node.hpp>
#include <boost/intrusive/linear_slist_algorithms.hpp>
#include <boost/intrusive/pointer_traits.hpp>
#include <boost/intrusive/detail/twin.hpp>
// move
#include <boost/move/utility_core.hpp>
#include <boost/move/adl_move_swap.hpp>
#include <boost/assert.hpp>
#include <boost/move/detail/iterator_to_raw_pointer.hpp>


namespace boost {
namespace container {
namespace dtl {

//////////////////////////////////////////////////////////////////////////////
//
//    basic_multiallocation_chain
//
//////////////////////////////////////////////////////////////////////////////

template<class VoidPointer>
class basic_multiallocation_chain
{
   private:
   typedef bi::slist_node_traits<VoidPointer>               node_traits;
   typedef bi::linear_slist_algorithms<node_traits>         algo;
   typedef typename node_traits::node                       node;
   typedef typename node_traits::node_ptr                   node_ptr;
   typedef typename boost::intrusive::
      pointer_traits<node_ptr>                              node_ptr_traits;

   typedef typename boost::intrusive::pointer_traits
      <VoidPointer>::template rebind_pointer<char>::type    char_ptr;
   typedef typename boost::intrusive::
      pointer_traits<char_ptr>::difference_type             difference_type;

   public:
   typedef VoidPointer                                      void_pointer;
   typedef typename boost::container::dtl::
      make_unsigned<difference_type>::type                  size_type;
   typedef boost::intrusive::twin<void_pointer>             pointer_pair;

   //Forward iterator over the blocks: it points to the link of a block
   class iterator
      : public boost::container::iterator
         <std::forward_iterator_tag, node, difference_type, node_ptr, node&>
   {
      public:
      iterator() : m_p() {}
      explicit iterator(const node_ptr &p) : m_p(p) {}

      node &operator*() const       {  return *m_p;  }
      node_ptr operator->() const   {  return m_p;  }

      iterator &operator++()        {  m_p = node_traits::get_next(m_p); return *this;  }
      iterator operator++(int)      {  iterator tmp(*this); ++*this; return tmp;  }

      friend bool operator==(const iterator &a, const iterator &b) {  return a.m_p == b.m_p;  }
      friend bool operator!=(const iterator &a, const iterator &b) {  return a.m_p != b.m_p;  }

      const node_ptr &pointed_node() const {  return m_p;  }

      private:
      node_ptr m_p;
   };

   private:
   //The header: its link is the first block. last_ points to it when empty.
   node        root_;
   node_ptr    last_;
   size_type   size_;

   BOOST_MOVABLE_BUT_NOT_COPYABLE(basic_multiallocation_chain)

   node_ptr root_ptr()
   {  return node_ptr_traits::pointer_to(root_);  }

   static node_ptr to_node_ptr(const void_pointer &p)
   {  return node_ptr_traits::static_cast_from(p);   }

   static void_pointer from_node_ptr(const node_ptr &p)
   {  return void_pointer(p);   }

   void init()
   {
      algo::init_header(this->root_ptr());
      last_ = this->root_ptr();
      size_ = 0u;
   }

   //Links [b, before_e], already linked between them, after after_this
   void priv_link_after(const node_ptr &after_this, const node_ptr &b, const node_ptr &before_e, size_type n)
   {
      algo::incorporate_after(after_this, b, before_e);
      if(after_this == last_)
         last_ = before_e;
      size_ += n;
   }

   public:
   basic_multiallocation_chain()
   {  this->init();  }

   basic_multiallocation_chain(const void_pointer &b, const void_pointer &before_e, size_type n)
   {
      this->init();
      if(n){
         node_traits::set_next(to_node_ptr(before_e), node_ptr());
         this->priv_link_after(this->root_ptr(), to_node_ptr(b), to_node_ptr(before_e), n);
      }
   }

   basic_multiallocation_chain(BOOST_RV_REF(basic_multiallocation_chain) other)
   {
      this->init();
      this->swap(other);
   }

   basic_multiallocation_chain& operator=(BOOST_RV_REF(basic_multiallocation_chain) other)
   {
      this->clear();
      this->swap(other);
      return *this;
   }

   bool empty() const
   {  return !size_; }

   size_type size() const
   {  return size_;  }

   iterator before_begin()
   {  return iterator(this->root_ptr()); }

   iterator begin()
   {  return iterator(node_traits::get_next(this->root_ptr())); }

   iterator end()
   {  return iterator(); }

   iterator last()
   {  return iterator(last_); }

   void clear()
   {  this->init(); }

   iterator insert_after(iterator it, void_pointer m)
   {
      const node_ptr n = to_node_ptr(m);
      algo::link_after(it.pointed_node(), n);
      if(it.pointed_node() == last_)
         last_ = n;
      ++size_;
      return iterator(n);
   }

   void push_front(const void_pointer &m)
   {  this->insert_after(this->before_begin(), m);  }

   void push_back(const void_pointer &m)
   {  this->insert_after(this->last(), m);  }

   void_pointer pop_front()
   {
      BOOST_ASSERT(!this->empty());
      const node_ptr root = this->root_ptr();
      const node_ptr f = node_traits::get_next(root);
      algo::unlink_after(root);
      if(last_ == f)
         last_ = root;
      --size_;
      return from_node_ptr(f);
   }

   //Moves the n blocks (before_b, before_e] of x after after_this
   void splice_after(iterator after_this, basic_multiallocation_chain &x, iterator before_b, iterator before_e, size_type n)
   {
      if(!n)
         return;
      const node_ptr bb = before_b.pointed_node();
      const node_ptr be = before_e.pointed_node();
      const node_ptr p  = after_this.pointed_node();
      if(x.last_ == be)
         x.last_ = bb;
      x.size_ -= n;
      algo::transfer_after(p, bb, be);
      if(p == last_)
         last_ = be;
      size_ += n;
   }

   void splice_after(iterator after_this, basic_multiallocation_chain &x)
   {  this->splice_after(after_this, x, x.before_begin(), x.last(), x.size());   }

   //Unlinks the n blocks (before_b, e)
   void erase_after(iterator before_b, iterator e, size_type n)
   {
      const node_ptr bb = before_b.pointed_node();
      algo::unlink_after(bb, e.pointed_node());
      if(!e.pointed_node())
         last_ = bb;
      size_ -= n;
   }

   //Links num_units blocks of unit_bytes bytes, contiguous from b, after after_this.
   //Returns the address after the last block.
   void_pointer incorporate_after(iterator after_this, const void_pointer &b, size_type unit_bytes, size_type num_units)
   {
      typedef typename boost::intrusive::pointer_traits<char_ptr> char_pointer_traits;
      char_ptr elem = char_pointer_traits::static_cast_from(b);
      if(num_units){
         char_ptr prev_elem = elem;
         elem += difference_type(unit_bytes);
         for(size_type i = 0; i != num_units-1u; ++i, elem += difference_type(unit_bytes)){
            node_traits::set_next(to_node_ptr(void_pointer(prev_elem)), to_node_ptr(void_pointer(elem)));
            prev_elem = elem;
         }
         this->priv_link_after(after_this.pointed_node(), to_node_ptr(b), to_node_ptr(void_pointer(prev_elem)), num_units);
      }
      return elem;
   }

   //Links the n blocks [b, before_e], already linked between them, after after_this
   void incorporate_after(iterator after_this, void_pointer b, void_pointer before_e, size_type n)
   {
      if(n)
         this->priv_link_after(after_this.pointed_node(), to_node_ptr(b), to_node_ptr(before_e), n);
   }

   void swap(basic_multiallocation_chain &x)
   {
      const bool this_empty = this->empty();
      const bool x_empty    = x.empty();
      const node_ptr this_last = last_;
      const node_ptr x_last    = x.last_;
      const node_ptr this_first = node_traits::get_next(this->root_ptr());
      node_traits::set_next(this->root_ptr(), node_traits::get_next(x.root_ptr()));
      node_traits::set_next(x.root_ptr(), this_first);
      boost::adl_move_swap(size_, x.size_);
      //An empty chain's last node is its own header
      last_   = x_empty    ? this->root_ptr() : x_last;
      x.last_ = this_empty ? x.root_ptr()     : this_last;
   }

   static iterator iterator_to(const void_pointer &p)
   {  return iterator(to_node_ptr(p));   }

   pointer_pair extract_data()
   {
      if(BOOST_LIKELY(!this->empty())){
         pointer_pair ret(from_node_ptr(node_traits::get_next(this->root_ptr())), from_node_ptr(last_));
         this->init();
         return ret;
      }
      else {
         return pointer_pair();
      }
   }
};

template <class Iterator, class T>
class multialloc_iterator
   : public boost::container::iterator
      < typename Iterator::iterator_category
      , T
      , typename Iterator::difference_type
      , T*
      , T&
      >
{
   public:
   inline explicit multialloc_iterator(const Iterator &it)
      :  m_it(it)
   {}

   inline explicit multialloc_iterator()
      :  m_it()
   {}

   //Constructors
   inline multialloc_iterator& operator++()
   { increment();   return *this;   }

      inline multialloc_iterator operator++(int)
   {
      multialloc_iterator result (*this);
      increment();
      return result;
   }

   inline friend bool operator== (const multialloc_iterator& i, const multialloc_iterator& i2)
   { return i.equal(i2); }

   inline friend bool operator!= (const multialloc_iterator& i, const multialloc_iterator& i2)
   { return !(i == i2); }

   inline friend typename Iterator::difference_type operator- (const multialloc_iterator& i, const multialloc_iterator& i2)
   { return i2.distance_to(i); }

   //Arithmetic
   inline multialloc_iterator& operator+=(typename Iterator::difference_type off)
   {  this->advance(off); return *this;   }

   inline multialloc_iterator operator+(typename Iterator::difference_type off) const
   {
      multialloc_iterator other(*this);
      other.advance(off);
      return other;
   }

   inline friend multialloc_iterator operator+(typename Iterator::difference_type off, const multialloc_iterator& right)
   {  return right + off; }

   inline multialloc_iterator& operator-=(typename Iterator::difference_type off)
   {  this->advance(-off); return *this;   }

   inline multialloc_iterator operator-(typename Iterator::difference_type off) const
   {  return *this + (-off);  }

   inline T& operator*() const
   { return *this->operator->(); }

   inline T* operator->() const
   {  return static_cast<T*>(static_cast<void*>(boost::movelib::iterator_to_raw_pointer(m_it)));   }

   inline Iterator & base()
   {  return m_it;   }

   inline const Iterator & base() const
   {  return m_it;   }

   private:
   Iterator m_it;

   inline void increment()
   { ++m_it; }

   inline void decrement()
   { --m_it; }

   inline bool equal(const multialloc_iterator &other) const
   {  return m_it == other.m_it;   }

   inline bool less(const multialloc_iterator &other) const
   {  return other.m_it < m_it;   }

   inline void advance(typename Iterator::difference_type n)
   {  boost::container::iterator_advance(m_it, n); }

   inline typename Iterator::difference_type distance_to(const multialloc_iterator &other)const
   {  return boost::container::iterator_distance(other.m_it, m_it); }
};


template<class MultiallocationChain, class T>
class transform_multiallocation_chain
   : public MultiallocationChain
{
   private:
   BOOST_MOVABLE_BUT_NOT_COPYABLE(transform_multiallocation_chain)
   //transform_multiallocation_chain(const transform_multiallocation_chain &);
   //transform_multiallocation_chain & operator=(const transform_multiallocation_chain &);

   typedef typename MultiallocationChain::void_pointer   void_pointer;
   typedef typename boost::intrusive::pointer_traits
      <void_pointer>                                     void_pointer_traits;
   typedef typename void_pointer_traits::template
      rebind_pointer<T>::type                            pointer;
   typedef typename boost::intrusive::pointer_traits
      <pointer>                                          pointer_traits;

   static pointer cast(const void_pointer &p)
   {  return pointer_traits::static_cast_from(p);  }

   public:
   typedef multialloc_iterator
      <typename MultiallocationChain::iterator, T>       iterator;
   typedef typename MultiallocationChain::size_type      size_type;
   typedef boost::intrusive::twin<pointer>               pointer_pair;

   transform_multiallocation_chain()
      : MultiallocationChain()
   {}

   transform_multiallocation_chain(BOOST_RV_REF(transform_multiallocation_chain) other)
      : MultiallocationChain(::boost::move(static_cast<MultiallocationChain&>(other)))
   {}

   transform_multiallocation_chain(BOOST_RV_REF(MultiallocationChain) other)
      : MultiallocationChain(::boost::move(static_cast<MultiallocationChain&>(other)))
   {}

   transform_multiallocation_chain& operator=(BOOST_RV_REF(transform_multiallocation_chain) other)
   {
      return static_cast<MultiallocationChain&>
         (this->MultiallocationChain::operator=(::boost::move(static_cast<MultiallocationChain&>(other))));
   }

   void push_front(const pointer &mem)
   {   this->MultiallocationChain::push_front(mem);  }

   void push_back(const pointer &mem)
   {  return this->MultiallocationChain::push_back(mem);   }

   void swap(transform_multiallocation_chain &other_chain)
   {  this->MultiallocationChain::swap(other_chain); }

   void splice_after(iterator after_this, transform_multiallocation_chain &x, iterator before_b, iterator before_e, size_type n)
   {  this->MultiallocationChain::splice_after(after_this.base(), x, before_b.base(), before_e.base(), n);  }

   void incorporate_after(iterator after_this, pointer b, pointer before_e, size_type n)
   {  this->MultiallocationChain::incorporate_after(after_this.base(), b, before_e, n);  }

   pointer pop_front()
   {  return cast(this->MultiallocationChain::pop_front());  }

   bool empty() const
   {  return this->MultiallocationChain::empty(); }

   iterator before_begin()
   {  return iterator(this->MultiallocationChain::before_begin());   }

   iterator begin()
   {  return iterator(this->MultiallocationChain::begin());   }

   iterator last()
   {  return iterator(this->MultiallocationChain::last());  }

   iterator end()
   {  return iterator(this->MultiallocationChain::end());   }

   size_type size() const
   {  return this->MultiallocationChain::size();  }

   void clear()
   {  this->MultiallocationChain::clear(); }

   iterator insert_after(iterator it, pointer m)
   {  return iterator(this->MultiallocationChain::insert_after(it.base(), m)); }

   static iterator iterator_to(const pointer &p)
   {  return iterator(MultiallocationChain::iterator_to(p));  }

   pointer_pair extract_data()
   {
      typename MultiallocationChain::pointer_pair data(this->MultiallocationChain::extract_data());
      return pointer_pair(cast(data.first), cast(data.second));
   }
/*
   MultiallocationChain &extract_multiallocation_chain()
   {  return holder_;  }*/
};

}}}

// namespace dtl {
// namespace container {
// namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_DETAIL_MULTIALLOCATION_CHAIN_HPP
