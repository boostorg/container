//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2005-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_DETAIL_POOL_COMMON_HPP
#define BOOST_CONTAINER_DETAIL_POOL_COMMON_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/container_fwd.hpp>   //bi namespace alias

#include <boost/intrusive/slist.hpp>
#include <boost/intrusive/pointer_traits.hpp>
#include <boost/move/detail/to_raw_pointer.hpp>
#include <boost/move/adl_move_swap.hpp>

namespace boost {
namespace container {
namespace dtl {

template<class VoidPointer>
struct node_slist
{
   //This hook will be used to chain the individual nodes
    typedef typename bi::make_slist_base_hook
      <bi::void_pointer<VoidPointer>, bi::link_mode<bi::normal_link> >::type slist_hook_t;

   //A node object will hold node_t when it's not allocated
   typedef slist_hook_t node_t;

   typedef typename bi::make_slist
      <node_t, bi::linear<true>, bi::cache_last<true>, bi::base_hook<slist_hook_t> >::type node_slist_t;
};

template<class T>
struct is_stateless_segment_manager
{
   BOOST_STATIC_CONSTEXPR bool value = false;
};

//Holds the segment manager of a pool and forwards the calls of the pool to it.
//
//A stateful segment manager (e.g. Interprocess) needs to stored a pointer.
template< class SegmentManagerBase
        , bool Stateless = is_stateless_segment_manager<SegmentManagerBase>::value>
class segment_manager_holder
{
   typedef typename boost::intrusive::pointer_traits
      <typename SegmentManagerBase::void_pointer>::template
         rebind_pointer<SegmentManagerBase>::type        segment_mngr_base_ptr_t;
   typedef typename SegmentManagerBase::size_type        size_type;
   typedef typename SegmentManagerBase::multiallocation_chain multiallocation_chain;

   public:
   explicit segment_manager_holder(SegmentManagerBase *segment_mngr_base)
      : mp_segment_mngr_base(segment_mngr_base)
   {}

   SegmentManagerBase* get_segment_manager_base() const
   {  return boost::movelib::to_raw_pointer(mp_segment_mngr_base);  }

   void swap_segment_manager(segment_manager_holder &other)
   {  ::boost::adl_move_swap(mp_segment_mngr_base, other.mp_segment_mngr_base);  }

   //Makes the next allocation fail
   void reset_segment_manager()
   {  mp_segment_mngr_base = segment_mngr_base_ptr_t();  }

   void *segment_manager_allocate_aligned(size_type nbytes, size_type alignment) const
   {  return mp_segment_mngr_base->allocate_aligned(nbytes, alignment);  }

   void segment_manager_deallocate(void *addr) const
   {  mp_segment_mngr_base->deallocate(addr);  }

   void segment_manager_deallocate_many(multiallocation_chain &chain) const
   {  mp_segment_mngr_base->deallocate_many(chain);  }

   private:
   segment_mngr_base_ptr_t mp_segment_mngr_base;
};

//A stateless segment manager
template<class SegmentManagerBase>
class segment_manager_holder<SegmentManagerBase, true>
{
   typedef typename SegmentManagerBase::size_type        size_type;
   typedef typename SegmentManagerBase::multiallocation_chain multiallocation_chain;

   public:
   explicit segment_manager_holder(SegmentManagerBase *)
   {}

   SegmentManagerBase* get_segment_manager_base() const
   {  return 0;  }

   void swap_segment_manager(segment_manager_holder &)
   {}

   //There is no pointer to reset
   void reset_segment_manager()
   {}

   void *segment_manager_allocate_aligned(size_type nbytes, size_type alignment) const
   {  return SegmentManagerBase::allocate_aligned(nbytes, alignment);  }

   void segment_manager_deallocate(void *addr) const
   {  SegmentManagerBase::deallocate(addr);  }

   void segment_manager_deallocate_many(multiallocation_chain &chain) const
   {  SegmentManagerBase::deallocate_many(chain);  }
};

}  //namespace dtl {
}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif   //#ifndef BOOST_CONTAINER_DETAIL_ADAPTIVE_NODE_POOL_IMPL_HPP
