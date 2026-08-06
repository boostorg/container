//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_TEST_LIGHTWEIGHT_TEST_HPP
#define BOOST_CONTAINER_TEST_LIGHTWEIGHT_TEST_HPP

// MSVC warns C4530 when compiling without /EHsc because Boost.Core's
// lightweight_test pulls in <iostream> and instantiates ostream operators
// that use try/catch. Silence that for this include only.
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4530) // C++ exception handler used, but unwind semantics are not enabled
#endif

#include <boost/core/lightweight_test.hpp>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // BOOST_CONTAINER_TEST_LIGHTWEIGHT_TEST_HPP
