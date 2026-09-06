//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//
// What a block costs the heap, and whether blocks are neighbours, asked
// through inspect_all() alone.
//
// chunk_size() used to report the first of these. It was dropped: it hands
// out an internal quantity that means neither "bytes I may touch"
// (usable_size) nor "what this costs the heap" (a directly mapped block
// costs more than its chunk), so it was a white-box hook, not an interface.
//
// inspect_all() gives what is needed instead. It reports every block, in
// address order, with the bytes of it that are in use, which for a block in
// a segment is exactly its usable size; the chunk is that plus a fixed
// overhead, the same for every block. Measuring the overhead once, from a
// heap holding one block, turns allocated_memory() into a per-block figure -
// and the measurement is itself a cross-check, since it holds the walker
// against the accounting.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_TEST_DLMALLOC_WALK_UTILS_HPP
#define BOOST_CONTAINER_TEST_DLMALLOC_WALK_UTILS_HPP

#include <boost/container/detail/dlmalloc.hpp>
#include <cstddef>

namespace boost {
namespace container {
namespace test {

typedef dlmalloc::size_type walk_size_type;

struct walk_result
{
   walk_size_type blocks;
   walk_size_type used;
   void          *wanted;      //when set, only this block is counted
};

inline void collect_block(void *start, void *end, walk_size_type used, void *arg)
{
   (void)end;
   walk_result &w = *static_cast<walk_result *>(arg);
   if(!used)
      return;                               //a free block
   if(w.wanted && start != w.wanted)
      return;
   ++w.blocks;
   w.used += used;
}

inline walk_result walk_live(const dlmalloc &h, void *only = 0)
{
   walk_result w;
   w.blocks = 0;  w.used = 0;  w.wanted = only;
   h.inspect_all(&collect_block, &w);
   return w;
}

//The fixed cost a chunk carries over the bytes the caller may use. Measured
//from a heap of its own, so it says nothing about the heap being asked about.
inline walk_size_type chunk_overhead()
{
   dlmalloc h;
   void *const p = h.allocate(64);
   const walk_result w = walk_live(h);
   const walk_size_type over = (w.blocks == 1) ? (h.allocated_memory() - w.used) : 0;
   h.deallocate(p);
   return over;
}

//What one block costs the heap: the bytes in use, plus that overhead.
inline walk_size_type chunk_cost(const dlmalloc &h, void *p)
{
   const walk_result w = walk_live(h, p);
   return (w.blocks == 1) ? (w.used + chunk_overhead()) : 0;
}

//inspect_all() reports the blocks of a heap in address order. This says
//whether four given blocks turn up in it consecutively, which is what "these
//blocks are neighbours" means.
struct neighbours
{
   std::size_t count;      //how many of of[] have been matched, in order
   void      **of;         //the four blocks, in the order they must appear
};

inline void note_run(void *start, void *end, std::size_t used, void *arg)
{
   (void)end;
   neighbours &n = *static_cast<neighbours *>(arg);
   if(!used)
      return;                                   //a free block
   if(n.count == 4)
      return;                                   //already found
   if(start == n.of[n.count])
      ++n.count;                                //the next one, where expected
   else
      n.count = (start == n.of[0]) ? 1u : 0u;   //broken; this may start a new run
}

//True when a, b, c and d are four blocks of h that follow one another.
inline bool are_neighbours(const dlmalloc &h, void *a, void *b, void *c, void *d)
{
   void *seen[4];
   neighbours n;
   seen[0] = a;  seen[1] = b;  seen[2] = c;  seen[3] = d;
   n.count = 0;  n.of = seen;
   h.inspect_all(&note_run, &n);
   return n.count == 4;
}

}  //namespace test {
}  //namespace container {
}  //namespace boost {

#endif   //BOOST_CONTAINER_TEST_DLMALLOC_WALK_UTILS_HPP
