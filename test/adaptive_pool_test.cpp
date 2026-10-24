//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#define BOOST_CONTAINER_ADAPTIVE_NODE_POOL_CHECK_INVARIANTS
#include <boost/container/list.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/adaptive_pool.hpp>
#include <boost/move/detail/to_raw_pointer.hpp>
#include "movable_int.hpp"
#include "void_allocator_test.hpp"
#include "list_test.hpp"
#include "vector_test.hpp"
#include "extended_allocator_test.hpp"

using namespace boost::container;

typedef test::overaligned_copyable_int oint_t;

//Alias adaptive pools that allocates ints
typedef adaptive_pool<int> adapt_node_allocator_t;
typedef adaptive_pool<oint_t> adapt_onode_allocator_t;

typedef adaptive_pool<int, 256, 2, 1, 1> adapt_node_allocator_v1_t;
typedef adaptive_pool<oint_t, 256, 2, 1, 1> adapt_onode_allocator_v1_t;

namespace boost {
namespace container {

//Explicit instantiations to catch compilation errors
template class adaptive_pool<int>;
template class adaptive_pool<oint_t>;
template class private_adaptive_pool<int>;
template class adaptive_pool<void>;
template class private_adaptive_pool<void>;

}} //boost::container

//Alias list types
typedef boost::container::list<int, adapt_node_allocator_t>       MyList;
typedef boost::container::list<oint_t, adapt_onode_allocator_t>   MyOList;

typedef boost::container::list<int, adapt_node_allocator_v1_t>       MyListV1;
typedef boost::container::list<oint_t, adapt_onode_allocator_v1_t>   MyOListV1;

//Alias vector types
typedef boost::container::vector<int, adapt_node_allocator_t>     MyVector;
typedef boost::container::vector<oint_t, adapt_onode_allocator_t> MyOVector;

typedef boost::container::vector<int, adapt_node_allocator_v1_t>     MyVectorV1;
typedef boost::container::vector<oint_t, adapt_onode_allocator_v1_t> MyOVectorV1;

//private_adaptive_pool: copies have another pool, so they don't compare equal (also void)
template<class Alloc>
bool test_private_pool_comparison()
{
   const Alloc a;
   const Alloc b(a);
   return (a == a) && !(a != a) && !(a == b) && (a != b);
}

//Propagation traits seen by the containers
template<class Alloc, bool POCCA, bool POCMA, bool POCS, bool AlwaysEqual>
void test_propagation_traits()
{
   typedef boost::container::allocator_traits<Alloc> traits_t;
   BOOST_CONTAINER_STATIC_ASSERT((traits_t::propagate_on_container_copy_assignment::value == POCCA));
   BOOST_CONTAINER_STATIC_ASSERT((traits_t::propagate_on_container_move_assignment::value == POCMA));
   BOOST_CONTAINER_STATIC_ASSERT((traits_t::propagate_on_container_swap::value == POCS));
   BOOST_CONTAINER_STATIC_ASSERT((traits_t::is_always_equal::value == AlwaysEqual));
}

//private_adaptive_pool: each allocator owns its pool. The nodes must stay with the pool that
//allocated them when containers are moved, move assigned or swapped, also after the source
//container (and its allocator) is destroyed.
template<class List>
bool test_private_pool_transfer()
{
   typedef typename List::allocator_type alloc_t;
   {  //Copies have another pool: they don't compare equal
      alloc_t a;
      alloc_t b(a);
      if (a == b || !(a == a))
         return false;
   }
   const int n = 300;
   for (int op = 0; op != 4; ++op) {
      List *src = new List;
      for (int i = 0; i != n; ++i) src->push_back(i);
      List dst;
      dst.push_back(-1);
      if (op == 0) {
         List tmp(boost::move(*src));
         dst.swap(tmp);
      }
      else if (op == 1) {
         alloc_t a;
         List tmp(boost::move(*src), a);
         dst.swap(tmp);
      }
      else if (op == 2) {
         dst = boost::move(*src);
      }
      else {
         dst.swap(*src);
      }
      delete src;
      dst.push_back(n);
      if (int(dst.size()) != n + 1 || dst.front() != 0 || dst.back() != n)
         return false;
   }
   return true;
}

#ifndef BOOST_NO_EXCEPTIONS

//allocate throws bad_alloc when the memory can not be allocated: max_size()
//elements pass the size check, but no heap has that much memory
template<class Allocator>
bool test_allocate_throws_bad_alloc()
{
   Allocator a;
   bool thrown = false;
   BOOST_CONTAINER_TRY{
      (void)a.allocate(a.max_size());
   }
   BOOST_CONTAINER_CATCH(const boost::container::bad_alloc_t &){
      thrown = true;
   }
   BOOST_CONTAINER_CATCH_END
   return thrown;
}

#else

template<class Allocator>
bool test_allocate_throws_bad_alloc()
{  return true;  }

#endif   //BOOST_NO_EXCEPTIONS

//Alignment-only pools whose header and nodes fill a power of two exactly: the
//block must also hold the allocation payload, or it has no room for a node
//(a division by zero when the pool computes the number of blocks to allocate)
struct BOOST_ALIGNMENT(256) aligned_256
{  char c[256];  };

struct BOOST_ALIGNMENT(4096) aligned_4096
{  char c[4096];  };

template<class Allocator>
bool test_alignment_only_pool()
{
   typedef typename Allocator::value_type value_type;
   Allocator a;
   value_type *p[4];
   for(int i = 0; i != 4; ++i){
      p[i] = boost::movelib::to_raw_pointer(a.allocate_one());
      if(reinterpret_cast<std::size_t>(p[i]) % boost::container::dtl::alignment_of<value_type>::value)
         return false;
   }
   for(int i = 0; i != 4; ++i){
      a.deallocate_one(p[i]);
   }
   typename Allocator::multiallocation_chain chain;
   a.allocate_individual(5u, chain);
   if(chain.size() != 5u)
      return false;
   a.deallocate_individual(chain);
   return true;
}

int main ()
{
   if(!test::test_void_allocator_conversions< adaptive_pool<void> >() ||
      !test::test_void_allocator_conversions< adaptive_pool<void, 256, 2, 1, 1> >() ||
      !test::test_void_allocator_conversions< private_adaptive_pool<void> >() ||
      !test::test_void_allocator_conversions< private_adaptive_pool<void, 256, 2, 1, 1> >())
      return 1;
   if(!test_private_pool_comparison< private_adaptive_pool<void> >() ||
      !test_private_pool_comparison< private_adaptive_pool<int> >())
      return 1;

   //adaptive_pool shares a pool for each type: all instances are equal and nothing propagates
   test_propagation_traits<adaptive_pool<int>, false, false, false, true>();
   test_propagation_traits<adaptive_pool<int, 256, 2, 1, 1>, false, false, false, true>();
   //private_adaptive_pool: the pool moves with the allocator on container move assignment and swap
   test_propagation_traits<private_adaptive_pool<int>, false, true, true, false>();
   test_propagation_traits<private_adaptive_pool<int, 256, 2, 1, 1>, false, true, true, false>();

   if(!test_private_pool_transfer< boost::container::list<int, private_adaptive_pool<int> > >())
      return 1;
   if(!test_private_pool_transfer< boost::container::list<int, private_adaptive_pool<int, 256, 2, 1, 1> > >())
      return 1;
   if(test::list_test<MyList, true>())
      return 1;
   if(test::list_test<MyOList, true>())
      return 1;
   if(test::list_test<MyListV1, true>())
      return 1;
   if(test::list_test<MyOListV1, true>())
      return 1;

   if(test::vector_test<MyVector>())
      return 1;
   if(test::vector_test<MyOVector>())
      return 1;
   if(test::vector_test<MyVectorV1>())
      return 1;
   if(test::vector_test<MyOVectorV1>())
      return 1;

   //The Version 2 chain interface: containers reach it only for some
   //sizes, so exercise it directly.
   if(test::extended_pool_allocator_test<adapt_node_allocator_t>("adaptive_pool<int>"))
      return 1;
   if(test::extended_pool_allocator_test< adaptive_pool<double> >("adaptive_pool<double>"))
      return 1;
   //Over-aligned type (bigger than the dlmalloc alignment)
   if(test::extended_pool_allocator_test<adapt_onode_allocator_t>("adaptive_pool<overaligned>"))
      return 1;
   if(test::extended_allocator_test< private_adaptive_pool<oint_t> >("private_adaptive_pool<overaligned>"))
      return 1;

   if(!test_alignment_only_pool< adaptive_pool<void*, 1, 2, ADP_only_alignment, 2> >() ||
      !test_alignment_only_pool< adaptive_pool<aligned_256, 1, 0, ADP_only_alignment, 2> >() ||
      !test_alignment_only_pool< adaptive_pool<aligned_4096, 1, 0, ADP_only_alignment, 2> >() ||
      !test_alignment_only_pool< private_adaptive_pool<void*, 1, 2, ADP_only_alignment, 2> >() ||
      !test_alignment_only_pool< private_adaptive_pool<aligned_256, 1, 0, ADP_only_alignment, 2> >() ||
      !test_alignment_only_pool< private_adaptive_pool<aligned_4096, 1, 0, ADP_only_alignment, 2> >())
      return 1;
   if(!test_allocate_throws_bad_alloc< adaptive_pool<int> >() ||
      !test_allocate_throws_bad_alloc< adaptive_pool<int, 256, 2, 1, 2> >() ||
      !test_allocate_throws_bad_alloc< private_adaptive_pool<int> >() ||
      !test_allocate_throws_bad_alloc< private_adaptive_pool<int, 256, 2, 1, 2> >())
      return 1;

   return 0;
}
