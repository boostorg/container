//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

//Runs the set tests with the deprecated insert(hint, node_type&&) that
//returns insert_return_type
#define BOOST_DISABLE_PRAGMA_MESSAGE
#define BOOST_CONTAINER_HINT_NODE_INSERT_RETURNS_INSERT_RETURN_TYPE
#include "set_test.cpp"
