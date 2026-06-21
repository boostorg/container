//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2007-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//
// Compares boost::container::small_vector against std::vector and
// boost::container::vector.
//
//////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <memory>    //std::allocator
#include <boost/container/vector.hpp>
#include <boost/container/small_vector.hpp>

#include "bench_vector_common.hpp"

template<class IntType, class Operation>
void run_containers(unsigned numit, unsigned numele, bool bp)
{
   vector_test_template< bc::small_vector<IntType, 0, std::allocator<IntType> >, Operation >(numit, numele, "small_vector   ", bp);
   vector_test_template< std::vector<IntType, std::allocator<IntType> >,         Operation >(numit, numele, "std::vector    ", bp);
   vector_test_template< bc::vector<IntType, std::allocator<IntType> >,          Operation >(numit, numele, "vector         ", bp);
}

int main()
{
   test_vectors<int>();
   return 0;
}
