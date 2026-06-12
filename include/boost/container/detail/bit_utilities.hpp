//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_DETAIL_BIT_UTILITIES_HPP
#define BOOST_CONTAINER_DETAIL_BIT_UTILITIES_HPP

#ifndef BOOST_CONFIG_HPP
#  include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/assert.hpp>
#include <boost/cstdint.hpp>
#include <boost/core/bit.hpp>

//_BitScanForward64 / _BitScanReverse64 used by the bit helpers below.
#if defined(BOOST_MSVC) && (defined(_M_X64) || defined(_M_ARM64))
#  include <intrin.h>
#endif

namespace boost {
namespace container {
namespace dtl {

//! Count trailing zero bits of a 64-bit value.
//! Precondition: x != 0 (the result is undefined otherwise).
BOOST_CONTAINER_FORCEINLINE int unchecked_countr_zero(boost::uint64_t x)
{
#if defined(BOOST_MSVC) && (defined(_M_X64) || defined(_M_ARM64))
   unsigned long r;
   _BitScanForward64(&r, x);
   return (int)r;
#elif defined(BOOST_GCC) || defined(BOOST_CLANG)
   return (int)__builtin_ctzll(x);
#else
   BOOST_CONTAINER_ASSUME(x != 0);
   return (int)boost::core::countr_zero(x);
#endif
}

//! Count trailing one bits of a 64-bit value.
//! Precondition: x != (uint64_t)-1 (the result is undefined otherwise).
BOOST_CONTAINER_FORCEINLINE int unchecked_countr_one(boost::uint64_t x)
{
   return unchecked_countr_zero(~x);
}

//! Count leading zero bits of a 64-bit value.
//! Precondition: x != 0 (the result is undefined otherwise).
BOOST_CONTAINER_FORCEINLINE int unchecked_countl_zero(boost::uint64_t x)
{
#if defined(BOOST_MSVC) && (defined(_M_X64) || defined(_M_ARM64))
   unsigned long r;
   _BitScanReverse64(&r, x);
   return (int)(63 - r);
#elif defined(BOOST_GCC) || defined(BOOST_CLANG)
   return (int)__builtin_clzll(x);
#else
   BOOST_CONTAINER_ASSUME(x != 0);
   return (int)boost::core::countl_zero(x);
#endif
}

} //namespace dtl {
} //namespace container {
} //namespace boost {

#include <boost/container/detail/config_end.hpp>

#endif //BOOST_CONTAINER_DETAIL_BIT_UTILITIES_HPP
