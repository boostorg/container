//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2007-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/detail/dlmalloc.hpp>
#include <boost/container/allocator.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/list.hpp>
#include "extended_allocator_test.hpp"

using namespace boost::container;

bool basic_test()
{
   size_t received = 0;
   if(!dl_all_deallocated())
      return false;
   void *ptr = dl_alloc(50, 98, &received);
   if(dl_size(ptr) != received)
      return false;
   if(dl_allocated_memory() != dl_chunksize(ptr))
      return false;

   if(dl_all_deallocated())
      return false;

   dl_grow(ptr, received + 20, received + 30, &received);

   if(dl_allocated_memory() != dl_chunksize(ptr))
      return false;

   if(dl_size(ptr) != received)
      return false;

   if(!dl_shrink(ptr, 100, 140, &received, 1))
      return false;

   if(dl_allocated_memory() != dl_chunksize(ptr))
      return false;

   if(!dl_shrink(ptr, 0, 140, &received, 1))
      return false;

   if(dl_allocated_memory() != dl_chunksize(ptr))
      return false;

   if(dl_shrink(ptr, 0, received/2, &received, 1))
      return false;

   if(dl_allocated_memory() != dl_chunksize(ptr))
      return false;

   if(dl_size(ptr) != received)
      return false;

   dl_free(ptr);

   dl_malloc_check();
   if(!dl_all_deallocated())
      return false;
   return true;
}

bool vector_test()
{
   typedef boost::container::vector<int, allocator<int> > Vector;
   if(!dl_all_deallocated())
      return false;
   {
      const int NumElem = 1000;
      Vector v;
      v.resize(NumElem);
      int *orig_buf = &v[0];
      int *new_buf  = &v[0];
      while(orig_buf == new_buf){
         Vector::size_type cl = v.capacity() - v.size();
         while(cl--){
            v.push_back(0);
         }
         v.push_back(0);
         new_buf = &v[0];
      }
   }
   if(!dl_all_deallocated())
      return false;
   return true;
}

bool list_test()
{
   typedef boost::container::list<int, allocator<int> > List;
   if(!dl_all_deallocated())
      return false;
   {
      const int NumElem = 1000;
      List l;
      int values[NumElem];
      l.insert(l.end(), &values[0], &values[NumElem]);
   }
   if(!dl_all_deallocated())
      return false;
   return true;
}

int main()
{
   if(!basic_test())
      return 1;
   if(!vector_test())
      return 1;
   if(!list_test())
      return 1;
   //The Version 2 chain interface: containers reach it only for some
   //sizes, so exercise it directly.
   if(boost::container::test::extended_allocator_test< allocator<int, 2> >("allocator<int>"))
      return 1;
   if(boost::container::test::extended_allocator_test< allocator<double, 2> >("allocator<double>"))
      return 1;

   return 0;
}
