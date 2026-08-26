//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Shared library "A" of the intermodule test: instantiates its own inline copy
//of intermodule_globals<> over the shared test types.
#include "intermodule_test_types.hpp"

BOOST_CONTAINER_TEST_INTERMODULE_LIB(lib_a)
