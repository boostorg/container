//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//Registers many distinct globals in one process. With the mapped-registry
//backend this drives the linear hashing well past its initial bucket count,
//so bucket splits, round changes and collision chains are all exercised;
//with the default backend it checks that many independent instances coexist.
#include <boost/container/detail/intermodule_globals.hpp>
#include "lightweight_test.hpp"

#include <cstddef>

using namespace boost::container;

namespace {

unsigned constructor_calls = 0;

struct BOOST_SYMBOL_VISIBLE counted_global
{
   counted_global() : magic(0x5A5Au), self(this) {  ++constructor_calls;  }
   unsigned  magic;
   void     *self;
};

//One Options type (and so one rendezvous key) per index
#define BOOST_CONTAINER_TEST_OPT(N)                                            \
   struct BOOST_SYMBOL_VISIBLE opt_##N                                         \
   {  static const char *name() { return "many" #N; }  };                      \
   /**/

#define BOOST_CONTAINER_TEST_OPT8(B)                                           \
   BOOST_CONTAINER_TEST_OPT(B##0) BOOST_CONTAINER_TEST_OPT(B##1)               \
   BOOST_CONTAINER_TEST_OPT(B##2) BOOST_CONTAINER_TEST_OPT(B##3)               \
   BOOST_CONTAINER_TEST_OPT(B##4) BOOST_CONTAINER_TEST_OPT(B##5)               \
   BOOST_CONTAINER_TEST_OPT(B##6) BOOST_CONTAINER_TEST_OPT(B##7)               \
   /**/

BOOST_CONTAINER_TEST_OPT8(1) BOOST_CONTAINER_TEST_OPT8(2)
BOOST_CONTAINER_TEST_OPT8(3) BOOST_CONTAINER_TEST_OPT8(4)
BOOST_CONTAINER_TEST_OPT8(5) BOOST_CONTAINER_TEST_OPT8(6)
BOOST_CONTAINER_TEST_OPT8(7) BOOST_CONTAINER_TEST_OPT8(8)

const unsigned num_globals = 64u;

#define BOOST_CONTAINER_TEST_GET(N)                                            \
   dst[i++] = &dtl::intermodule_globals<counted_global, opt_##N>();            \
   /**/

#define BOOST_CONTAINER_TEST_GET8(B)                                           \
   BOOST_CONTAINER_TEST_GET(B##0) BOOST_CONTAINER_TEST_GET(B##1)               \
   BOOST_CONTAINER_TEST_GET(B##2) BOOST_CONTAINER_TEST_GET(B##3)               \
   BOOST_CONTAINER_TEST_GET(B##4) BOOST_CONTAINER_TEST_GET(B##5)               \
   BOOST_CONTAINER_TEST_GET(B##6) BOOST_CONTAINER_TEST_GET(B##7)               \
   /**/

void fetch_all(counted_global **dst)
{
   unsigned i = 0;
   BOOST_CONTAINER_TEST_GET8(1) BOOST_CONTAINER_TEST_GET8(2)
   BOOST_CONTAINER_TEST_GET8(3) BOOST_CONTAINER_TEST_GET8(4)
   BOOST_CONTAINER_TEST_GET8(5) BOOST_CONTAINER_TEST_GET8(6)
   BOOST_CONTAINER_TEST_GET8(7) BOOST_CONTAINER_TEST_GET8(8)
   BOOST_TEST(i == num_globals);
}

}  //namespace

int main()
{
   counted_global *first[num_globals];
   fetch_all(first);

   //Every key produced its own instance, constructed exactly once and
   //reachable at the address its constructor saw
   BOOST_TEST(constructor_calls == num_globals);
   for(unsigned i = 0; i != num_globals; ++i){
      BOOST_TEST(first[i] != 0);
      BOOST_TEST(first[i]->magic == 0x5A5Au);
      BOOST_TEST(first[i]->self == (void *)first[i]);
   }

   //All distinct: a lookup that aliased two keys would show up here
   for(unsigned i = 0; i != num_globals; ++i){
      for(unsigned j = i + 1u; j != num_globals; ++j){
         BOOST_TEST(first[i] != first[j]);
      }
   }

   //Looking them up again finds the same objects, i.e. every entry is still
   //reachable after all the bucket splits that inserting them caused
   counted_global *again[num_globals];
   fetch_all(again);
   BOOST_TEST(constructor_calls == num_globals);
   for(unsigned i = 0; i != num_globals; ++i){
      BOOST_TEST(again[i] == first[i]);
   }

   return boost::report_errors();
}
