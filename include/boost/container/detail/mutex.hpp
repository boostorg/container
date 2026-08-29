//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Stephen Cleary 2000
// (C) Copyright Ion Gaztanaga  2015-2026.
//
// Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_MUTEX_HPP
#define BOOST_CONTAINER_MUTEX_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/detail/spin_mutex.hpp>   //dtl::spin_mutex

namespace boost {
namespace container {
namespace dtl {

typedef spin_mutex default_mutex;

template<class Mutex>
class scoped_lock
{
   public:
   scoped_lock(Mutex &m)
      :  m_(m)
   { m_.lock(); }
   ~scoped_lock()
   { m_.unlock(); }

   private:
   Mutex &m_;
};

} // namespace dtl
} // namespace container
} // namespace boost

#include <boost/container/detail/config_end.hpp>

#endif   //BOOST_CONTAINER_MUTEX_HPP
