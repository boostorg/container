//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//Checks that small by-value types (iterators, allocators, pair) are trivially copyable
//so that compilers pass and return them in registers instead of by invisible reference.

#include <boost/config.hpp>
#include <boost/move/detail/type_traits.hpp>

//Without compiler intrinsics is_trivially_copyable falls back to is_pod, which is
//false for any class type, so the checks are only meaningful when it is exact.
#if defined(BOOST_MOVE_IS_TRIVIALLY_COPYABLE)

#include <boost/container/vector.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/stable_vector.hpp>
#include <boost/container/list.hpp>
#include <boost/container/slist.hpp>
#include <boost/container/set.hpp>
#include <boost/container/map.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/string.hpp>
#include <boost/container/allocator.hpp>
#include <boost/container/new_allocator.hpp>
#include <boost/container/node_allocator.hpp>
#include <boost/container/adaptive_pool.hpp>
#include <boost/container/scoped_allocator.hpp>
#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/container/detail/pair.hpp>
#include <boost/container/detail/workaround.hpp>

namespace bc = boost::container;

namespace bmd = ::boost::move_detail;

//Iterators
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::vector<int>::iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::vector<int>::const_iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::deque<int>::iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::deque<int>::const_iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::stable_vector<int>::iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::stable_vector<int>::const_iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::list<int>::iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::list<int>::const_iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::slist<int>::iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::set<int>::iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::set<int>::const_iterator>::value);
BOOST_CONTAINER_STATIC_ASSERT((bmd::is_trivially_copyable<bc::map<int, int>::iterator>::value));
BOOST_CONTAINER_STATIC_ASSERT((bmd::is_trivially_copyable<bc::flat_map<int, int>::iterator>::value));
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::string::iterator>::value);

//Pair used as map node value: defaulted operations need C++11
BOOST_CONTAINER_STATIC_ASSERT((!bmd::is_trivially_copyable<bc::dtl::pair<int, bc::string> >::value));
#if !defined(BOOST_NO_CXX11_DEFAULTED_MOVES)
BOOST_CONTAINER_STATIC_ASSERT((bmd::is_trivially_copyable<bc::dtl::pair<int, int> >::value));
#if defined(BOOST_MOVE_HAS_TRIVIAL_MOVE_CONSTRUCTOR)
BOOST_CONTAINER_STATIC_ASSERT((::boost::move_detail::is_trivially_move_constructible<bc::dtl::pair<int, int> >::value));
#endif
#endif

//Pair assignment assigns through reference members and is deleted for const members, so it is not a memcpy
BOOST_CONTAINER_STATIC_ASSERT((::boost::move_detail::is_trivially_copy_assignable<bc::dtl::pair<int, int> >::value));
BOOST_CONTAINER_STATIC_ASSERT((::boost::move_detail::is_trivially_move_assignable<bc::dtl::pair<int, int> >::value));
BOOST_CONTAINER_STATIC_ASSERT((!::boost::move_detail::is_trivially_copy_assignable<bc::dtl::pair<int&, int> >::value));
BOOST_CONTAINER_STATIC_ASSERT((!::boost::move_detail::is_trivially_move_assignable<bc::dtl::pair<int, int&> >::value));
BOOST_CONTAINER_STATIC_ASSERT((!::boost::move_detail::is_trivially_copy_assignable<bc::dtl::pair<const int, int> >::value));
BOOST_CONTAINER_STATIC_ASSERT((!::boost::move_detail::is_trivially_move_assignable<bc::dtl::pair<int, const int> >::value));

//Allocators that are copied by value all the time
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::new_allocator<int> >::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::allocator<int> >::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::node_allocator<int> >::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::adaptive_pool<int> >::value);
//Defaulted operations need C++11
#if !defined(BOOST_NO_CXX11_DEFAULTED_MOVES)
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::pmr::polymorphic_allocator<int> >::value);
BOOST_CONTAINER_STATIC_ASSERT(bmd::is_trivially_copyable<bc::scoped_allocator_adaptor<bc::new_allocator<int> > >::value);
#if !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)
BOOST_CONTAINER_STATIC_ASSERT((bmd::is_trivially_copyable<bc::scoped_allocator_adaptor<bc::new_allocator<int>, bc::pmr::polymorphic_allocator<int> > >::value));
#endif
#endif

#endif   //BOOST_MOVE_IS_TRIVIALLY_COPYABLE

int main()
{
   return 0;
}
