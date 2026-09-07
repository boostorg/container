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
// dlmalloc implementation as a C++ class.
//
// The heap state is a data member, so an instance of the class is a heap.
//
// Macro configuration options are now compile-time options.
//////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_CONTAINER_DETAIL_DLMALLOC_HPP
#define BOOST_CONTAINER_DETAIL_DLMALLOC_HPP

#ifndef BOOST_CONFIG_HPP
#include <boost/config.hpp>
#endif

#if defined(BOOST_HAS_PRAGMA_ONCE)
#  pragma once
#endif

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/detail/mpl.hpp>
#include <boost/container/detail/spin_mutex.hpp>
#include <boost/container/detail/intermodule_globals.hpp>
#include <boost/container/detail/allocation_type.hpp>
#include <boost/move/detail/placement_new.hpp>

#include <cstddef>
#include <cstring>
#include <cerrno>
#include <cstdlib>
#include <climits>
#if !defined(BOOST_WINDOWS)
#  include <sys/mman.h>
#  include <unistd.h>
#endif


#if defined(__linux__)
#  define BOOST_CONTAINER_DL_MREMAP           1
#else
#  define BOOST_CONTAINER_DL_MREMAP           0
#endif

#if defined(_WIN32_WCE)
#  define BOOST_CONTAINER_DL_MMAP_CLEARS           0
#else
#  define BOOST_CONTAINER_DL_MMAP_CLEARS           1
#endif

//The heap's assertion macro
#define BOOST_CONTAINER_DL_ASSERT(x)   do{  if(debug && !(x))  assert_failed();  }while(0)

//////////////////////////////////////////////////////////////////////////////
//                        Single-thread detection
//////////////////////////////////////////////////////////////////////////////
//single_threaded() answers wether the process has only one thread.
#if defined(__GLIBC__) && \
    (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 32))
#  define BOOST_CONTAINER_DLMALLOC_GLIBC_IS_SINGLE_THREAD
#  include <sys/single_threaded.h>
#endif

#if defined(BOOST_WINDOWS)

#if defined(_WIN64) || defined(__LP64__)
typedef unsigned __int64 dl_win_size_t;   //matches SIZE_T
#else
typedef unsigned long    dl_win_size_t;
#endif

#ifdef BOOST_USE_WINDOWS_H
#  include <windows.h>
#else
struct _MEMORY_BASIC_INFORMATION;
struct _SYSTEM_INFO;

extern "C" {
__declspec(dllimport) void * __stdcall VirtualAlloc
   (void *lpAddress, dl_win_size_t dwSize, unsigned long flAllocationType, unsigned long flProtect);
__declspec(dllimport) int __stdcall VirtualFree
   (void *lpAddress, dl_win_size_t dwSize, unsigned long dwFreeType);
__declspec(dllimport) dl_win_size_t __stdcall VirtualQuery
   (const void *lpAddress, ::_MEMORY_BASIC_INFORMATION *lpBuffer, dl_win_size_t dwLength);
__declspec(dllimport) void __stdcall GetSystemInfo(::_SYSTEM_INFO *lpSystemInfo);
}  //extern "C"
#endif //BOOST_USE_WINDOWS_H

//Layout-compatible with MEMORY_BASIC_INFORMATION and SYSTEM_INFO, so the
//real definitions are never needed.
struct dl_win_memory_basic_information
{
   void          *BaseAddress;
   void          *AllocationBase;
   unsigned long  AllocationProtect;
#if defined(_WIN64)
   unsigned short PartitionId;
#endif
   dl_win_size_t RegionSize;
   unsigned long  State;
   unsigned long  Protect;
   unsigned long  Type;
};

struct dl_win_system_info
{
   unsigned long  dwOemId;
   unsigned long  dwPageSize;
   void          *lpMinimumApplicationAddress;
   void          *lpMaximumApplicationAddress;
   dl_win_size_t dwActiveProcessorMask;
   unsigned long  dwNumberOfProcessors;
   unsigned long  dwProcessorType;
   unsigned long  dwAllocationGranularity;
   unsigned short wProcessorLevel;
   unsigned short wProcessorRevision;
};

#else    //BOOST_WINDOWS

#if defined(MAP_ANONYMOUS)
#  define BOOST_CONTAINER_DL_MAP_ANONYMOUS MAP_ANONYMOUS
#elif defined(MAP_ANON)
#  define BOOST_CONTAINER_DL_MAP_ANONYMOUS MAP_ANON
#endif

#endif   //BOOST_WINDOWS

namespace boost {
namespace container {

//! Default action for a heap when it has to
//! refuse a request: set errno
struct dlmalloc_errno_action
{
   BOOST_CONTAINER_FORCEINLINE void operator()() const
   {  errno = ENOMEM;  }
};

//! Default action for a heap when one of its own consistency checks fails
//! or when a start-up sanity check says the configuration cannot work.
struct dlmalloc_abort
{
   BOOST_CONTAINER_FORCEINLINE void operator()() const
   {  ::std::abort();  }
};

//! Default configuration perameters for the heap
//!
//! Dlmalloc's macro options are here compile-time options, which enables
//! the use different type of heaps in the same program.
//!
//! Changing one is easiest by inheriting:
//!
//! \code
//! struct my_config : boost::container::dlmalloc_default_config
//! {
//!    static const bool use_locks = false;                    //single-threaded
//!    static const size_type default_granularity = 1u << 20;  //1MB segments
//! };
//! typedef boost::container::basic_dlmalloc<my_config> my_heap;
//! \endcode
struct dlmalloc_default_config
{
   typedef ::std::size_t size_type;

   //! Store in a footer after each block which heap the block came from, so
   //! that free() and realloc() can tell a block of one heap from another.
   //! Costs a word per block.
   static const bool footers              = false;

   //! Skip the sanity checks on addresses, sizes and inuse bits that the
   //! block walkers make before they trust what they read. Faster, and a
   //! corrupt heap then goes undetected.
   static const bool insecure             = false;

   //! On detected corruption, forget every block and carry on from an empty
   //! heap instead of aborting.
   static const bool proceed_on_error     = false;

   //! Whether the block walker is available. It is an ordinary member
   //! function here, which costs nothing until it is called, so it is always
   //! available and this selects nothing. It is kept so that a configuration
   //! can be described in full.
   static const bool malloc_inspect_all   = false;

   //! Never walk past the first segment when trimming or releasing. Nothing
   //! here is built that way, so nothing consults it; it is kept so that a
   //! configuration can be described in full.
   static const bool no_segment_traversal = false;

   //! Guard the heap with a lock. Turning it off removes the lock from the
   //! heap's layout, so the heap must then never be reached from two threads
   //! at once.
   static const bool use_locks            = true;

   //! Compile the heap's own consistency checks in. They walk the bins and
   //! segments and are far too slow for a release build, so this follows
   //! NDEBUG by default.
   #if defined(NDEBUG)
   static const bool debug                = false;
   #else
   static const bool debug                = true;
   #endif

   //! Whether a check that fails calls the abort action. With it off the
   //! checks still run and still cost what they cost, but say nothing -
   //! which is only useful to a debugger watching from outside.
   static const bool abort_on_assert_failure = true;

   //! Called when a request cannot be met, just before the null comes back.
   typedef dlmalloc_errno_action malloc_failure_action;

   //! Called when a consistency check fails, and when the constructor finds
   //! a configuration it cannot build a heap from.
   typedef dlmalloc_abort abort_action;

   //! The alignment every block honours, and the width block-size
   //! arithmetic is carried out in. Must be a power of two, and at least
   //! sizeof(void*).
   static const size_type malloc_alignment =
      ((size_type)(2 * sizeof(void *)));

   //! A segment is never smaller than this, and the system is never asked
   //! for less. Must be a power of two, and a multiple of the page size.
   static const size_type default_granularity =
      ((size_type)64U * (size_type)1024U);

   //! Memory goes back to the system only once this much has accumulated
   //! free at the top.
   static const size_type default_trim_threshold =
      ((size_type)2U * (size_type)1024U * (size_type)1024U);

   //! A request at or above this size is mapped directly instead of carved
   //! out of a segment.
   static const size_type default_mmap_threshold =
      ((size_type)256U * (size_type)1024U);

   //! How many frees may be skipped before the segment list must actually
   //! be walked looking for something to give back.
   static const size_type max_release_check_rate = 4095;

   //!
   //Boost.Container extended configuration options
   //!

   //! Space the small bins at the chunk-size granularity (malloc_alignment)
   //! rather than at a fixed 8 bytes. On a 64-bit target that granularity is
   //! by default 16, so the fixed spacing leaves half the bins unused.
   //! 
   //! Original dlmalloc behaviour is achieved with a "false" value.
   //! 
   //! When true, in 64-bit OSs with 16 byte malloc alignment (the default value),
   //! this reclaims them and extends exact-fit service from 240-byte
   //! chunks to 496-byte ones. On a 32-bit target with the default granularity (8)
   //! this option does nothing. However, if the malloc_alignment is set to 16 or 32,
   //! this option will reclaim the unused bins and extend exact-fit service.
   static const bool wide_smallbins       = true;

   //! Index the small bins from 'min_chunk_size' rather than from zero, so
   //! that the lowest bins - which stand for sizes below the smallest chunk
   //! and can never be filled - become usable. Moves the small/large
   //! boundary up by exactly the bins it reclaims.
   //! 
   //! Original dlmalloc behaviour is achieved with a "false" value which
   //! leaves the first bins unused.
   static const bool rebased_smallbins    = true;
};

//! The optional Config::layout configuration option defines the dlmalloc data
//! layout. If aabsent, the heap uses default_dlmalloc_layout<Config::use_locks>.
//!
//! An unlocked heap has no lock member at all.
template<bool UseLocks>
struct default_dlmalloc_layout
{
   typedef ::std::size_t size_type;
   typedef unsigned int  bindex_t;
   typedef unsigned int  binmap_t;
   typedef unsigned int  flag_t;
   typedef ::boost::container::dtl::spin_mutex_t mlock_t;
   static const size_type nsmallbins = 32U;
   static const size_type ntreebins  = 32U;

   struct malloc_chunk {
      size_type     prev_foot;  // Size of previous chunk (if free).
      size_type     head;       // Size and inuse bits.
      malloc_chunk *fd;         // double links -- used only if free.
      malloc_chunk *bk;
   };

   struct malloc_tree_chunk {
      // The first four fields must be compatible with malloc_chunk
      size_type          prev_foot;
      size_type          head;
      malloc_tree_chunk *fd;
      malloc_tree_chunk *bk;

      malloc_tree_chunk *child[2];
      malloc_tree_chunk *parent;
      bindex_t           index;
   };

   struct malloc_segment {
      char           *base;      // base address
      size_type       size;      // allocated size
      malloc_segment *next;      // ptr to next segment
      flag_t          sflags;    // mmap and extern flag
   };

   //The heap state, with the lock
   struct malloc_state_locked {
      binmap_t           smallmap;
      binmap_t           treemap;
      size_type          dvsize;
      size_type          topsize;
      char              *least_addr;
      malloc_chunk      *dv;
      malloc_chunk      *top;
      size_type          trim_check;
      size_type          release_checks;
      size_type          magic;
      malloc_chunk      *smallbins[(nsmallbins+1)*2];
      malloc_tree_chunk *treebins[ntreebins];
      size_type          footprint;
      size_type          max_footprint;
      size_type          footprint_limit; // zero means no limit
      flag_t             mflags;
      mlock_t            mutex;     // locate lock among fields that rarely change
      malloc_segment     seg;
      void              *extp;      // Unused but available for extensions
      size_type          exts;
   };

   //The heap state without the lock
   struct malloc_state_unlocked {
      binmap_t           smallmap;
      binmap_t           treemap;
      size_type          dvsize;
      size_type          topsize;
      char              *least_addr;
      malloc_chunk      *dv;
      malloc_chunk      *top;
      size_type          trim_check;
      size_type          release_checks;
      size_type          magic;
      malloc_chunk      *smallbins[(nsmallbins+1)*2];
      malloc_tree_chunk *treebins[ntreebins];
      size_type          footprint;
      size_type          max_footprint;
      size_type          footprint_limit; // zero means no limit
      flag_t             mflags;
      malloc_segment     seg;
      void              *extp;      // Unused but available for extensions
      size_type          exts;
   };

   typedef typename dtl::if_c<UseLocks, malloc_state_locked, malloc_state_unlocked>::type
      malloc_state;

   struct malloc_params {
      size_type magic;
      size_type page_size;
      size_type granularity;
      size_type mmap_threshold;
      size_type trim_threshold;
      flag_t    default_mflags;
   };
};

namespace dtl {

//A Config of void means "the defaults"
template<class Config>
struct dlmalloc_config
{  typedef Config type;  };

template<>
struct dlmalloc_config<void>
{  typedef dlmalloc_default_config type;  };

//Config::layout is optional, detect it.
template<class Config>
struct dlmalloc_has_layout
{
   typedef char yes;
   struct no { char c[2]; };
   template<class C> static yes test(typename C::layout *);
   template<class C> static no  test(...);
   static const bool value = sizeof(test<Config>(0)) == sizeof(yes);
};

template<class Config, bool HasLayout = dlmalloc_has_layout<Config>::value>
struct dlmalloc_layout_of
{  typedef typename Config::layout type;  };

template<class Config>
struct dlmalloc_layout_of<Config, false>
{  typedef default_dlmalloc_layout<Config::use_locks> type;  };

//Auxiliary log2 utility to convert between shifts and sizes
template< ::std::size_t N, unsigned Shift = 0>
struct dlmalloc_log2
{  static const unsigned value = dlmalloc_log2<N / 2, Shift + 1>::value;  };

template<unsigned Shift>
struct dlmalloc_log2<1, Shift>
{  static const unsigned value = Shift;  };

}  //namespace dtl {

//! A Dlmalloc heap
//!
//! Each instance owns its basic layout memory. Instances are
//! independent, share nothing, and use no process-wide state.
//!
//! Config carries the options that change the behavior of the heap,
//! "void" means the defaults. See dlmalloc_default_config.
//!
//! Not copyable and not movable: malloc_state holds interior pointers to
//! itself, so its address is part of its meaning.
template<class Config = void>
class basic_dlmalloc
{
   private:
   basic_dlmalloc(const basic_dlmalloc &);
   basic_dlmalloc &operator=(const basic_dlmalloc &);

   //! One link of a memory chain. The link lives in the first bytes of the
   //! block it describes, so a chain costs no memory of its own.
   struct memchain_node
   {
      memchain_node *next_node_ptr;
   };

   public:
   typedef ::std::size_t size_type;

   //! The configuration this heap was built with: Config itself, or
   //! dlmalloc_default_config when Config is void.
   typedef typename dtl::dlmalloc_config<Config>::type config_type;

   //////////////////////////////////////////////////////////////////////////
   //                        Build-time switches
   //////////////////////////////////////////////////////////////////////////
   //Option options from the Config
   static const bool footers              = config_type::footers;
   static const bool insecure             = config_type::insecure;
   static const bool proceed_on_error     = config_type::proceed_on_error;
   static const bool malloc_inspect_all   = config_type::malloc_inspect_all;
   static const bool no_segment_traversal = config_type::no_segment_traversal;
   static const bool use_locks            = config_type::use_locks;
   static const bool wide_smallbins       = config_type::wide_smallbins;
   static const bool rebased_smallbins    = config_type::rebased_smallbins;
   static const bool debug                = config_type::debug;
   static const bool abort_on_assert_failure = config_type::abort_on_assert_failure;
   static const size_type malloc_alignment       = config_type::malloc_alignment;
   static const size_type default_granularity    = config_type::default_granularity;
   static const size_type default_trim_threshold = config_type::default_trim_threshold;
   static const size_type default_mmap_threshold = config_type::default_mmap_threshold;
   static const size_type max_release_check_rate = config_type::max_release_check_rate;
   typedef typename dtl::dlmalloc_layout_of<config_type>::type layout_type;
   typedef typename config_type::malloc_failure_action         malloc_failure_action;
   typedef typename config_type::abort_action                  abort_action;

   //! mallopt() options that can be set
   enum option_t
   {  option_trim_threshold = -1   //!< free bytes at the top before trimming
   ,  option_granularity    = -2   //!< the least a segment may be
   ,  option_mmap_threshold = -3   //!< the size mapped on its own
   };

   //////////////////////////////////////////////////////////////////////////
   //          Types of the Boost.Container extension interface
   //////////////////////////////////////////////////////////////////////////

   //! Forward iterator over a memory chain.
   struct memchain_it
   {
      memchain_node *node_ptr;

      //! Moves to the next block.
      void next()
      {  node_ptr = node_ptr->next_node_ptr;  }

      //! The block this iterator points to.
      void *addr() const
      {  return (void *)node_ptr;  }

      bool operator==(const memchain_it &other) const
      {  return node_ptr == other.node_ptr;  }

      bool operator!=(const memchain_it &other) const
      {  return node_ptr != other.node_ptr;  }
   };

   //! A singly linked list of blocks that multialloc_nodes() and
   //! multialloc_arrays() fill and multidealloc() gives back.
   //!
   //! The data members are the layout the algorithm works on. Use the
   //! member functions.
   struct memchain
   {
      size_type       num_mem;
      memchain_node   root_node;
      memchain_node  *last_node_ptr;

      memchain()
      {  this->init();  }

      //! Leaves the chain empty.
      void init()
      {
         root_node.next_node_ptr = 0;
         last_node_ptr = &root_node;
         num_mem = 0;
      }

      //! Takes over a run of blocks that another chain held.
      //last_block, not last: a parameter named last would hide last(), and
      //GCC 4.8's -Wshadow reports a parameter that hides a member function
      //of its own class.
      void init_from(void *first, void *last_block, size_type num)
      {
         last_node_ptr = (memchain_node *)last_block;
         root_node.next_node_ptr = (memchain_node *)first;
         num_mem = num;
      }

      bool empty() const              {  return num_mem == 0;  }
      size_type size() const          {  return num_mem;  }

      memchain_it before_begin()
      {  memchain_it it = { &root_node };            return it;  }
      memchain_it begin()
      {  memchain_it it = { root_node.next_node_ptr }; return it;  }
      memchain_it last()
      {  memchain_it it = { last_node_ptr };         return it;  }
      memchain_it end()
      {  memchain_it it = { (memchain_node *)0 };    return it;  }

      static bool is_end(const memchain_it &it)      {  return !it.node_ptr;  }

      void *first_mem() const   {  return (void *)root_node.next_node_ptr;  }
      void *last_mem() const    {  return (void *)last_node_ptr;  }

      //! Puts a block at the end of the chain.
      void push_back(void *mem)
      {
         memchain_node *const n = (memchain_node *)mem;
         last_node_ptr->next_node_ptr = n;
         n->next_node_ptr = 0;
         last_node_ptr = n;
         ++num_mem;
      }

      //! Puts a block at the front of the chain.
      void push_front(void *mem)
      {
         memchain_node *const n = (memchain_node *)mem;
         if(!root_node.next_node_ptr)
            last_node_ptr = n;
         n->next_node_ptr = root_node.next_node_ptr;
         root_node.next_node_ptr = n;
         ++num_mem;
      }

      //! Takes out the block after the one the iterator points to.
      //! The iterator must be valid and must not be the end iterator.
      void erase_after(const memchain_it &before)
      {
         memchain_node *const prev = before.node_ptr;
         memchain_node *const dead = prev->next_node_ptr;
         if(last_node_ptr == dead)
            last_node_ptr = &root_node;
         prev->next_node_ptr = dead->next_node_ptr;
         --num_mem;
      }

      //! Takes out the first block. The chain must not be empty.
      void pop_front()
      {  this->erase_after(this->before_begin());  }

      //! Puts a run of num blocks, from first to before_last, after the
      //! block the iterator points to.
      void incorporate_after(const memchain_it &before, void *first,
                             void *before_last, size_type num)
      {
         if(!num)
            return;
         memchain_node *const pnode  = before.node_ptr;
         memchain_node *const next   = pnode->next_node_ptr;
         memchain_node *const pfirst = (memchain_node *)first;
         memchain_node *const blast  = (memchain_node *)before_last;
         if(pnode == last_node_ptr)
            last_node_ptr = blast;
         pnode->next_node_ptr = pfirst;
         blast->next_node_ptr = next;
         num_mem += num;
      }

      //! Moves every block of other to the end of this chain, and leaves
      //! other empty.
      void splice_back(memchain &other)
      {
         if(other.empty())
            return;
         this->incorporate_after(this->last(), other.first_mem(),
                                 other.last_mem(), other.size());
         other.init();
      }
   };

   //! What allocation_command() returns: the block, and whether the block
   //! the caller passed in was reused rather than a new one handed out.
   struct command_ret_t
   {
      void *first;
      int   second;
   };

   //! What allocation_command() may be asked to do. The values are those of
   //! the C-style dl_* interface this class replaced, so a caller that still
   //! spells them the old way keeps working.
   enum allocation_command_t
   {  allocate_new         = 0x01
   ,  expand_fwd           = 0x02
   ,  expand_bwd           = 0x04
   ,  shrink_in_place      = 0x08
   ,  nothrow_allocation   = 0x10
   ,  try_shrink_in_place  = 0x40
   ,  expand_both          = expand_fwd | expand_bwd
   ,  expand_or_new        = allocate_new | expand_both
   };

   //! Every block that multialloc_nodes() or multialloc_arrays() hands out
   //! must come from one contiguous run.
   static const size_type all_contiguous = size_type(-1);

   //! Let multialloc_nodes() or multialloc_arrays() choose how many blocks
   //! share a run.
   static const size_type default_contiguous = 0;


   //////////////////////////////////////////////////////////////////////////
   //                          The public interface
   //////////////////////////////////////////////////////////////////////////

   //! Builds an empty heap. It holds no memory at all - the first allocation
   //! asks the system for a segment.
   basic_dlmalloc()
   {  init_state();  }

   //! Builds a heap that already holds at least `capacity` usable bytes,
   //! taken from the system.
   //!
   //! The heap object is this object, wherever the caller put it, and it
   //! stays outside the memory it manages - so all of that memory is heap
   //! and none of it holds the bookkeeping. That is the difference from
   //! create(), which puts the object at the front of the memory.
   //!
   //! Zero capacity asks for one granularity unit, as create() does.
   //!
   //! `locked` chooses whether the heap serializes its own operations; pass
   //! false only when the instance is reached from one thread.
   //!
   //! A constructor cannot report failure, and it does not have to: memory
   //! the system refuses, or a `capacity` too large to describe, simply
   //! leaves the heap empty. It is then exactly what the default constructor
   //! makes, and every later request asks the system on its own.
   //!
   //! `capacity` has no default value. Zero as a default would make this
   //! constructor ambiguous with the default constructor.
   explicit basic_dlmalloc(size_type capacity, bool locked = true)
   {
      init_state();
      set_lock(&m_state, locked ? 1 : 0);
      if(capacity < (size_type)(0 - (top_foot_size() + m_params.page_size))){
         const size_type rs = (capacity == 0) ? m_params.granularity
                                              : (capacity + top_foot_size());
         const size_type tsize = granularity_align(rs);
         char *const tbase = (char *)(call_mmap(tsize));
         if(tbase != cmfail())
            attach_segment(tbase, tsize, use_mmap_bit);
      }
   }

   //! Builds a heap over memory the caller owns and keeps.
   //!
   //! The heap object stays outside the buffer, so the whole buffer is heap.
   //! create_with_base() is the other arrangement, with the object at the
   //! front of the buffer.
   //!
   //! The buffer must outlive the heap. The destructor releases only what
   //! the heap took from the system, and leaves this buffer alone.
   //!
   //! The heap grows the ordinary way when the buffer runs out, unless a
   //! footprint limit stops it. A null buffer, or one too small to hold a
   //! heap, leaves the heap empty but usable - every request is then served
   //! from memory the heap takes for itself.
   //!
   //! `capacity` counts from `base`. The heap starts at the first correctly
   //! aligned address at or after `base` and uses the rest.
   basic_dlmalloc(void *base, size_type capacity, bool locked = true)
   {
      init_state();
      set_lock(&m_state, locked ? 1 : 0);
      char *const raw = (char *)base;
      if(raw != 0 &&
         capacity > (top_foot_size() + min_chunk_size + chunk_align_mask) &&
         capacity < (size_type)(0 - (top_foot_size() + m_params.page_size))){
         const size_type off = (size_type)(bytes_at(align_as_chunk(raw)) - raw);
         if(capacity > (off + top_foot_size() + min_chunk_size))
            attach_segment(raw, capacity, extern_bit);
      }
   }

   //! Releases every segment this heap obtained.
   //!
   //! Memory still handed out goes with it.
   ~basic_dlmalloc()
   {
      msegmentptr sp = &m_state.seg;
      (void)destroy_lock(lock_address(&m_state));   // destroy before unmapped
      while(sp != 0){
         char *const base = sp->base;
         const size_type size = sp->size;
         const flag_t flag = sp->sflags;
         sp = sp->next;
         if((flag & use_mmap_bit) && !(flag & extern_bit) && base != 0)
            (void)call_munmap(base, size);
      }
   }

   //! Takes memory from the system, puts the heap object at the front of it
   //! and makes the rest the heap's first segment. Zero capacity asks for
   //! one granularity unit.
   //!
   //! `locked` chooses whether the heap serializes its own operations; pass
   //! false only when it is reached from one thread.
   //!
   //! Returns null when the system refuses the memory, or when `capacity` is
   //! too large to describe. destroy() gives it all back, the object with it.
   static basic_dlmalloc *create(size_type capacity = 0, bool locked = true)
   {
      size_type psize, gsize;
      system_sizes(psize, gsize);
      const size_type msize = pad_request(sizeof(basic_dlmalloc));
      if(capacity >= (size_type)(0 - (msize + top_foot_size() + psize)))
         return 0;
      const size_type rs = (capacity == 0) ? gsize
                                           : (capacity + top_foot_size() + msize);
      const size_type tsize = granularity_align_to(rs, gsize);
      char *const tbase = (char *)(call_mmap(tsize));
      if(tbase == cmfail())
         return 0;
      return init_in_place(tbase, tsize, use_mmap_bit, locked);
    }

   //! The same, in memory the caller owns and keeps: the heap object goes at
   //! the front of the buffer and the rest is the first segment.
   //!
   //! The buffer has to outlive the heap, and destroy() leaves it alone -
   //! only the object inside it is destroyed. The heap still grows the
   //! ordinary way when the buffer runs out, unless a footprint limit stops
   //! it.
   //!
   //! Returns null for a null buffer, or one too small to hold the object
   //! and a heap.
   static basic_dlmalloc *create_with_base(void *base, size_type capacity,
                                           bool locked = true)
   {
      size_type psize, gsize;
      system_sizes(psize, gsize);
      (void)gsize;
      const size_type msize = pad_request(sizeof(basic_dlmalloc));
      char *const raw = (char *)base;
      if(raw == 0 ||
         capacity <= (msize + top_foot_size()) ||
         capacity >= (size_type)(0 - (msize + top_foot_size() + psize)))
         return 0;
      return init_in_place(raw, capacity, extern_bit, locked);
   }

   //! Destroys a heap create() or create_with_base() made, and returns how
   //! many bytes went back to the system. Memory still handed out goes with
   //! it, and a buffer given to create_with_base() is not released - it is
   //! the caller's.
   //!
   //! The pointer must not be used afterwards: for a create()d heap the
   //! object itself lived in the memory just released.
   static size_type destroy(basic_dlmalloc *p)
   {
      size_type freed = 0;
      if(p != 0){
         if(!p->ok_magic(&p->m_state)){
            usage_error_action(&p->m_state, p);
            return 0;
         }
         //Summed before anything is released: the destructor is what
         //releases, and the head of the segment list lives in the object,
         //which the first segment holds.
         for(const malloc_segment *sp = &p->m_state.seg; sp != 0; sp = sp->next)
            if((sp->sflags & use_mmap_bit) && !(sp->sflags & extern_bit) && sp->base != 0)
               freed += sp->size;
         p->~basic_dlmalloc();
      }
      return freed;
   }

   //////////////////////////////////////////////////////////////////////////
   //                            Allocation
   //////////////////////////////////////////////////////////////////////////

   //! Takes a block of at least `bytes` usable bytes, or null.
   void *allocate(size_type bytes)
   {  return this->priv_allocate(bytes);  }

   //! Gives a block back. A null pointer is ignored.
   void deallocate(void *mem)
   {  this->priv_deallocate(mem);  }

   //! Takes a block for n_elements of elem_size bytes, zero filled.
   void *allocate_zeroed(size_type n_elements, size_type elem_size)
   {  return this->priv_allocate_zeroed(n_elements, elem_size);  }

   //! Resizes a block, moving and copying it when it cannot grow where it
   //! stands. The old block is kept when the request cannot be met.
   void *reallocate(void *mem, size_type newsize)
   {  return this->priv_reallocate(mem, newsize);  }

   //! Grows or shrinks a block without moving it, or fails. Never copies.
   void *reallocate_in_place(void *mem, size_type newsize)
   {  return this->priv_reallocate_in_place(mem, newsize);  }

   //! Takes a block whose address is a multiple of `alignment`, which must
   //! be a power of two.
   void *allocate_aligned(size_type alignment, size_type bytes)
   {  return this->priv_allocate_aligned(alignment, bytes);  }

   //! Takes n_elements blocks of elem_size bytes, zero filled, out of one
   //! run, and reports them through an array. `chunks` may name that array;
   //! null asks the heap for one, which the caller then frees like a block.
   void **independent_calloc(size_type n_elements, size_type elem_size, void *chunks[])
   {  return this->priv_independent_calloc(n_elements, elem_size, chunks);  }

   //! The same, with a size of its own for each block and no zero filling.
   void **independent_comalloc(size_type n_elements, size_type sizes[], void *chunks[])
   {  return this->priv_independent_comalloc(n_elements, sizes, chunks);  }

   //! Frees an array of pointers under one lock and nulls the entries it
   //! consumed. Returns how many it could not free.
   size_type bulk_free(void *array[], size_type nelem)
   {  return internal_bulk_free(array, nelem);  }

   //////////////////////////////////////////////////////////////////////////
   //                        Queries and maintenance
   //////////////////////////////////////////////////////////////////////////

   //! What a block costs the heap over the bytes the caller may use: the
   //! chunk header, and the footer as well when the configuration keeps
   //! one.
   static const size_type allocation_payload =
      (config_type::footers ? sizeof(size_type)*2u : sizeof(size_type));

   //! Usable bytes of a block this heap returned, which may exceed what was
   //! asked for. Needs no instance state, and takes no lock.
   //!
   //! Not safe against other operations on the same heap, even for a block
   //! the caller owns: a block's header also carries the in-use bit of the
   //! block before it, so allocating or freeing a NEIGHBOUR writes the very
   //! word this reads. Call it while nothing else is touching the heap, or
   //! from the thread that owns the heap.
   static size_type usable_size(const void *mem)
   {
      if(BOOST_LIKELY(mem != 0)){
         mchunkptr p = mem2chunk(mem);
         if(is_inuse(p))
            return chunksize(p) - overhead_for(p);
      }
      return 0;
   }

   //! Bytes obtained from the system.
   size_type footprint() const
   {  return m_state.footprint;  }

   //! High-water mark of footprint().
   size_type max_footprint() const
   {  return m_state.max_footprint;  }

   //! Current cap, or the maximum size_type when there is none.
   size_type footprint_limit() const
   {
      const size_type maf = m_state.footprint_limit;
      return maf == 0 ? max_size_t : maf;
   }

   //! Caps how much this heap may obtain from the system.
   size_type set_footprint_limit(size_type bytes)
   {
      size_type result = 0;
      if(bytes == 0)
         result = granularity_align(1);       // Use minimal size
      else if(bytes < max_size_t)
         result = granularity_align(bytes);
      return m_state.footprint_limit = result;
   }

   //! Releases unused memory back to the system.
   bool trim(size_type pad = 0)
   {
      int result = 0;
      if(!preaction(&m_state)){
         result = sys_trim(pad, true);
         postaction(&m_state);
      }
      return result != 0;
   }

   //! Tunes this heap, and no other. `value` is a size, so the whole range
   //! of size_type is available; the maximum stands for "no limit", which is
   //! also what a plain -1 converts to.
   bool mallopt(option_t param_number, size_type value)
   {  return this->change_mparam(param_number, value) != 0;  }

   //! Keeps large blocks inside the heap's own segments instead of mapping
   //! each one on its own, so that inspect_all() and mallinfo() can see them.
   //!
   //! Returns what the setting was before this call. Note the sense:
   //! tracking a large block means NOT mapping it on its own, so enabling
   //! tracking disables direct mapping, and the value returned is true when
   //! direct mapping was already off - that is, when they were already
   //! tracked.
   //!
   //! Blocks already mapped on their own stay that way; this decides only
   //! what happens to later requests.
   bool track_large_chunks(bool enable)
   {
      bool was_tracking = false;
      mstate m = &m_state;
      if(!preaction(m)){
         if(!use_mmap(m))
            was_tracking = true;
         if(!enable)
            enable_mmap(m);
         else
            disable_mmap(m);
         postaction(m);
      }
      return was_tracking;
   }

   //! What mallinfo() reports. Every field is a count of bytes except
   //! ordblks, which counts chunks.
   //!
   //! smblks, hblks and fsmblks are always zero. The struct keeps the shape
   //! and the field names of the SVID mallinfo, which distinguishes a
   //! small-block arena this heap does not have, so that the two can be
   //! compared field for field.
   struct mallinfo_t
   {
      size_type arena;     //!< bytes of every segment, the top chunk included
      size_type ordblks;   //!< how many free chunks there are, top counted
      size_type smblks;    //!< always 0
      size_type hblks;     //!< always 0
      size_type hblkhd;    //!< bytes of the blocks mapped on their own
      size_type usmblks;   //!< the high-water mark of footprint()
      size_type fsmblks;   //!< always 0
      size_type uordblks;  //!< bytes handed out, segment records included
      size_type fordblks;  //!< bytes free: every free chunk of every segment,
                           //!< plus the top chunk and the padding that ends
                           //!< its segment. A block the heap mapped on its
                           //!< own is in no segment and is never counted -
                           //!< that memory is handed out or already back with
                           //!< the system, never free in here. Nor does this
                           //!< answer "will a request of N bytes fit": it is
                           //!< spread over chunks of many sizes, and only the
                           //!< top chunk can grow
      size_type keepcost;  //!< bytes at the top, which trim() could give back
   };

   //! One walk of the whole heap, filling in every field above.
   //!
   //! Walks the heap, because no running total is kept - which is why this
   //! is meant for tests and diagnostics rather than for a hot path. A heap
   //! that never allocated anything gives all zeros.
   //!
   //! Note uordblks counts the record chunk each segment after the first
   //! carries, which the heap made for itself and never gave to anybody.
   //! allocated_memory() leaves those out, so the two differ by exactly that
   //! much - and by nothing else.
   mallinfo_t mallinfo() const
   {
      mallinfo_t nm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
      basic_dlmalloc *const self = const_cast<basic_dlmalloc *>(this);
      mstate m = &self->m_state;
      if(!preaction(m)){
         if(debug)
            self->do_check_malloc_state();
         if(is_initialized(m)){
            //top is always free, and the segment it ends carries top_foot_size
            //bytes of padding that no chunk can use - both are counted
            size_type nfree = size_t_one;
            size_type mfree = m->topsize + top_foot_size();
            size_type sum   = mfree;
            msegmentptr s = &m->seg;
            while(s != 0){
               mchunkptr q = align_as_chunk(s->base);
               while(segment_holds(s, q) &&
                     q != m->top && q->head != fencepost_head){
                  size_type sz = chunksize(q);
                  sum += sz;
                  if(!is_inuse(q)){
                     mfree += sz;
                     ++nfree;
                  }
                  q = next_chunk(q);
               }
               s = s->next;
            }
            nm.arena    = sum;
            nm.ordblks  = nfree;
            nm.hblkhd   = m->footprint - sum;
            nm.usmblks  = m->max_footprint;
            nm.uordblks = m->footprint - mfree;
            nm.fordblks = mfree;
            nm.keepcost = m->topsize;
         }
         postaction(m);
      }
      return nm;
   }

   //! What malloc_stats() reports: the three figures the classic
   //! malloc_stats() writes to stderr.
   struct malloc_stats_t
   {
      size_type max_system_bytes;  //!< the most the heap ever held
      size_type system_bytes;      //!< what it holds from the system now
      size_type in_use_bytes;      //!< how much of that is handed out
   };

   //! The three figures the classic malloc_stats() prints, returned rather
   //! than written anywhere: the caller picks the destination, and the
   //! header stays free of <cstdio>.
   //!
   //! Every one of them is also a mallinfo() figure - max_system_bytes is
   //! usmblks, system_bytes is uordblks plus fordblks, and in_use_bytes is
   //! uordblks - so this asks nothing mallinfo() does not already answer.
   //! It is here because it is short, because the three names read better
   //! than the SVID ones, and because the interface it mirrors has it.
   //!
   //! Walks the heap, for in_use_bytes; the other two are counters.
   malloc_stats_t malloc_stats() const
   {
      malloc_stats_t st;
      st.max_system_bytes = this->max_footprint();
      st.system_bytes     = this->footprint();
      st.in_use_bytes     = this->mallinfo().uordblks;
      return st;
   }

   //! Type of the function inspect_all() calls for each block the first byte,
   //! one past the last, how many bytes of it are in use - zero for a free block
   //! - and the user provided pointer.
   typedef void (*inspect_handler_t)(void *start, void *end,
                                     size_type used_bytes, void *arg);

   //! Calls the handler once for every block of every segment, in use or
   //! free, in address order.
   //!
   //! `start` and `end` bound the memory the block can hold, so for a free
   //! block they skip the bookkeeping the heap keeps inside it and the range
   //! can be empty - such a block is passed over rather than reported. A
   //! block whose bytes are in use is reported with the size the caller can
   //! rely on, which is the chunk less its overhead.
   //!
   //! Only segments are walked. A block big enough for the heap to map on
   //! its own is in no segment and is not reported; track_large_chunks()
   //! keeps such blocks in segments, where this can see them.
   //!
   //! The handler runs while the heap is locked, so it must not touch this
   //! heap - not even to ask its size.
   void inspect_all(inspect_handler_t handler, void *arg) const
   {
      basic_dlmalloc *const self = const_cast<basic_dlmalloc *>(this);
      mstate m = &self->m_state;
      if(!preaction(m)){
         if(is_initialized(m)){
            mchunkptr top = m->top;
            for(msegmentptr s = &m->seg; s != 0; s = s->next){
               mchunkptr q = align_as_chunk(s->base);
               while(segment_holds(s, q) && q->head != fencepost_head){
                  mchunkptr next = next_chunk(q);
                  size_type sz = chunksize(q);
                  size_type used;
                  void *start;
                  if(is_inuse(q)){
                     used = sz - chunk_overhead;   //must not be mmapped
                     start = chunk2mem(q);
                  }
                  else{
                     used = 0;
                     //offset by the bookkeeping a free chunk of this size keeps
                     if(is_small(sz))
                        start = (void *)((char *)q + sizeof(malloc_chunk));
                     else
                        start = (void *)((char *)q + sizeof(malloc_tree_chunk));
                  }
                  if(start < (void *)next)   //skip if it is all bookkeeping
                     handler(start, next, used, arg);
                  if(q == top)
                     break;
                  q = next;
               }
            }
         }
         postaction(m);
      }
   }

   //! Walks the whole heap checking every invariant, when the configuration
   //! has debug on. A failed check runs the abort action; with debug off this
   //! does nothing. Always returns true, so it can sit inside a test macro.
   bool check() const
   {
      if(debug){
         basic_dlmalloc *const self = const_cast<basic_dlmalloc *>(this);
         if(!preaction(&self->m_state)){
            self->do_check_malloc_state();
            postaction(&self->m_state);
         }
      }
      return true;
   }

   //////////////////////////////////////////////////////////////////////////
   //                Boost.Container extension interface
   //////////////////////////////////////////////////////////////////////////
   //Everything below this line is Boost.Container's own: what the classic
   //allocator interface above does not offer, and what the extended
   //allocators need - in-place growth and shrinkage, allocation in bulk,
   //and the accounting that lets a test say the heap is empty.
   //
   //They keep no running total of what is handed out: allocated_memory()
   //walks the heap instead, so the allocation paths stay free of
   //bookkeeping.

   //! Grows a block in place, forwards only, to somewhere between minbytes
   //! and maxbytes. Returns false and changes nothing when it cannot.
   bool grow(void *oldmem, size_type minbytes, size_type maxbytes,
             size_type *received)
   {
      mstate ms = &m_state;
      if(!preaction(ms)){
         mchunkptr p = mem2chunk(oldmem);
         p = try_realloc_chunk_with_min
            (p, request2size(minbytes), request2size(maxbytes), 0);
         //The check must run before the lock goes, or it races other threads
         if(p){
            check_inuse_chunk(p);
            *received = dl_size_impl(oldmem);
         }
         postaction(ms);
         return 0 != p;
      }
      return false;
   }

   //! Shrinks a block in place to somewhere between minbytes and maxbytes.
   //! With do_commit false it only reports what a shrink would give, and
   //! leaves the block as it is.
   bool shrink(void *oldmem, size_type minbytes, size_type maxbytes,
               size_type *received, bool do_commit)
   {
      mstate ms = &m_state;
      if(!preaction(ms)){
         const int ret = internal_shrink
            (oldmem, minbytes, maxbytes, received, do_commit ? 1 : 0);
         postaction(ms);
         return 0 != ret;
      }
      return false;
   }

   //! Takes a block of at least minbytes, and of preferred_bytes when that
   //! costs nothing more. Reports what it really gave.
   void *alloc(size_type minbytes, size_type preferred_bytes,
               size_type *received_bytes)
   {
      return this->allocation_command
         (allocate_new, 1, 1, minbytes, preferred_bytes, received_bytes, 0).first;
   }

   //! The one entry point that does the lot: grow a block where it stands,
   //! hand out a new one, or shrink one in place, as `command` allows.
   //!
   //! For a shrink the two sizes change their meaning: `limit_size` is then
   //! the largest the block may stay and `preferred_size` the smallest it
   //! may become. For
   //! everything else `limit_size` is the least that will do and
   //! `preferred_size` the most that is wanted, and `limit_size` must not be
   //! the larger of the two.
   //!
   command_ret_t allocation_command
      (unsigned command, size_type sizeof_object, size_type alignof_object,
       size_type limit_size, size_type preferred_size, size_type *received_size,
       void *reuse_ptr)
   {
      command_ret_t ret = { 0, 0 };

      if(command & (shrink_in_place | try_shrink_in_place)){
         const bool success = this->shrink
            ( reuse_ptr, preferred_size, limit_size, received_size
            , 0 != (command & shrink_in_place));
         ret.first = success ? reuse_ptr : 0;
         return ret;
      }

      *received_size = 0;

      if(limit_size > preferred_size)
         return ret;

      mstate ms = &m_state;
      if(!preaction(ms)){
         if(footers && reuse_ptr){
            mstate m = get_mstate_for(mem2chunk(reuse_ptr));
            if(!ok_magic(m)){
               usage_error_action(m, reuse_ptr);
               goto postaction;      //do not leak the lock
            }
         }
         if(reuse_ptr && (command & (expand_fwd | expand_bwd))){
            void *const r = internal_grow_both_sides
               ( command, reuse_ptr, limit_size
               , preferred_size, received_size, sizeof_object, 1);
            if(r){
               ret.first  = r;
               ret.second = 1;
               goto postaction;
            }
         }

         if(command & allocate_new){
            //The lock is held, so the allocation below must not take it
            //again: both variants are the lockless ones and run inside this
            //one critical section. Never clear the use-lock flag of the heap
            //instead: that would let every OTHER thread skip locking too.
            void *addr;
            if(alignof_object <= malloc_alignment){
               addr = priv_allocate_nolock(preferred_size);
               //Only worth a second try with the smaller size: when the two
               //are equal nothing has changed under the still-held lock
               if(!addr && limit_size != preferred_size)
                  addr = priv_allocate_nolock(limit_size);
            }
            else{
               addr = priv_allocate_aligned_nolock(alignof_object, preferred_size);
               if(!addr && limit_size != preferred_size)
                  addr = priv_allocate_aligned_nolock(alignof_object, limit_size);
            }
            if(addr)
               *received_size = dl_size_impl(addr);

            ret.first  = addr;
            ret.second = 0;
            if(addr)
               goto postaction;
         }

         //Now try to expand both sides with the smallest size that serves
         if(reuse_ptr && (command & (expand_fwd | expand_bwd))){
            void *const r = internal_grow_both_sides
               ( command, reuse_ptr, limit_size
               , preferred_size, received_size, sizeof_object, 0);
            if(r){
               ret.first  = r;
               ret.second = 1;
            }
         }
         postaction:
         postaction(ms);
      }
      return ret;
   }

   //! Takes n_elements blocks of elem_size bytes each and adds them to the
   //! chain. contiguous_elements says how many share one run; pass
   //! default_contiguous to let the heap choose, or all_contiguous to ask
   //! for one run.
   bool multialloc_nodes(size_type n_elements, size_type elem_size,
                         size_type contiguous_elements, memchain *pchain)
   {
      int ret = 0;
      mstate ms = &m_state;
      if(!preaction(ms)){
         ret = internal_node_multialloc
            (n_elements, elem_size, contiguous_elements, pchain);
         postaction(ms);
      }
      return 0 != ret;
   }

   //! Takes n_elements blocks, the i-th of sizes[i]*element_size bytes, and
   //! adds them to the chain.
   bool multialloc_arrays(size_type n_elements, const size_type *sizes,
                          size_type element_size, size_type contiguous_elements,
                          memchain *pchain)
   {
      int ret = 0;
      mstate ms = &m_state;
      if(!preaction(ms)){
         ret = internal_multialloc_arrays
            (n_elements, sizes, element_size, contiguous_elements, pchain);
         postaction(ms);
      }
      return 0 != ret;
   }

   //! Gives back every block of the chain under one lock, merging the ones
   //! that turn out to be neighbours. Blocks it could not take back stay in
   //! the chain.
   void multidealloc(memchain *pchain)
   {  internal_multialloc_free(pchain);  }

   //! Bytes this heap has allocated, chunk overhead included.
   //!
   //! This walks every chunk of every segment, so it costs O(number of chunks).
   //!
   //! For a block small enough to live in a segment the figure is exactly
   //! the sum of the chunks in use. A block large enough for the heap to map
   //! it on its own belongs to no segment, so it is counted by what it costs
   //! the system, which is mmap_foot_pad more than its chunk. The figure
   //! still falls to zero when every such block comes back, because the
   //! mapping goes back with it.
   size_type allocated_memory() const
   {
      size_type alloc_mem = 0;
      basic_dlmalloc *const self = const_cast<basic_dlmalloc *>(this);
      mstate m = &self->m_state;
      if(!preaction(m)){
         if(debug)
            self->do_check_malloc_state();
         if(is_initialized(m)){
            size_type inuse     = 0;
            size_type seg_bytes = 0;
            msegmentptr s = &m->seg;
            while(s != 0){
               seg_bytes += s->size;
               mchunkptr q = align_as_chunk(s->base);
               while(segment_holds(s, q) &&
                     q != m->top && q->head != fencepost_head){
                  if(is_inuse(q) && !this->is_own_bookkeeping(chunk2mem(q)))
                     inuse += chunksize(q);
                  q = next_chunk(q);
               }
               s = s->next;
            }
            //Whatever the heap took from the system and did not put in a
            //segment is a block it mapped on its own
            alloc_mem = (size_type)(inuse + (m->footprint - seg_bytes));
         }
         postaction(m);
      }
      return alloc_mem;
   }

   //! True when everything this heap gave out has come back.
   //!
   //! Walks the heap, because no running total is kept.
   bool all_deallocated() const
   {  return 0 == this->allocated_memory();  }

   private:

   #ifndef BOOST_CONTAINER_DOXYGEN_INVOKED
   //Running each of the two, in the one place that knows how.
   BOOST_CONTAINER_FORCEINLINE static void do_abort()
      {  abort_action()();  }
   BOOST_CONTAINER_FORCEINLINE static void malloc_failure()
      {  malloc_failure_action()();  }

   //What a failed BOOST_CONTAINER_DL_ASSERT does, which is a question of
   //its own: the
   //checks can be compiled in without being fatal.
   BOOST_CONTAINER_FORCEINLINE static void assert_failed(dtl::true_)
      {  do_abort();  }
   BOOST_CONTAINER_FORCEINLINE static void assert_failed(dtl::false_)
      {  }
   BOOST_CONTAINER_FORCEINLINE static void assert_failed()
      {  assert_failed(dtl::bool_<abort_on_assert_failure>());  }

   //The largest value size_type can hold
   static const size_type max_size_t = (~(size_type)0);

   //Flag bits stored in a segment's sflags and in the heap's mflags.
   static const size_type use_noncontiguous_bit = (4U);
   static const size_type extern_bit            = (8U);
   //Zero when use_locks is off
   static const size_type use_lock_bit          = (use_locks ? 2U : 0U);

   #if defined(BOOST_WINDOWS)
   //VirtualAlloc()/VirtualFree() flag words - unsigned long is what those
   //two APIs actually take (flAllocationType, flProtect, dwFreeType), and
   //size_type would just be silently truncated to it on every call anyway.
   static const unsigned long dl_mem_commit     = 0x1000;
   static const unsigned long dl_mem_reserve    = 0x2000;
   static const unsigned long dl_mem_release    = 0x8000;
   static const unsigned long dl_mem_top_down   = 0x100000;
   static const unsigned long dl_page_readwrite = 0x04;
   #else
   //mmap() flag words - int is what ::mmap()'s prot/flags parameters take.
   static const int mmap_prot  = (PROT_READ|PROT_WRITE);
   static const int mmap_flags = (MAP_PRIVATE|BOOST_CONTAINER_DL_MAP_ANONYMOUS);
   #endif

   //True when mem is memory the heap keeps for itself rather than something
   //it handed out. Those chunks are in use, but nobody asked for them, so
   //allocated_memory() must not count them. Two kinds:
   //
   //* a segment record carved out of an older segment, and
   //* the heap object itself, when create() or create_with_base() put it in
   //  a chunk at the front of the first segment.
   //
   //mallinfo() counts both, exactly as the original counts the chunk holding
   //its malloc_state and its segment records. allocated_memory() is
   //Boost.Container's own figure and answers a different question - what did
   //the heap give out - so it leaves both out.
   bool is_own_bookkeeping(const void *mem) const
   {
      if(mem == (const void *)this)
         return true;
      //m_state.seg is the head and lives in this object, not in a chunk.
      //Every record after it sits inside the segment it displaced.
      for(const malloc_segment *s = m_state.seg.next; s != 0; s = s->next)
         if((const void *)s == mem)
            return true;
      return false;
   }

   //////////////////////////////////////////////////////////////////////////
   //                           Platform memory
   //////////////////////////////////////////////////////////////////////////
   //mmap()/VirtualAlloc() return this on failure - not a real address, so
   //nothing else could ever collide with it. Used on every platform, so it
   //sits outside the BOOST_WINDOWS guard below.
   BOOST_CONTAINER_FORCEINLINE static void *mfail()
   {  return (void*)(max_size_t);  }
   BOOST_CONTAINER_FORCEINLINE static char *cmfail()
   {  return (char*)(mfail());  }

   #if defined(BOOST_WINDOWS)

   //win32mmap / win32direct_mmap / win32munmap, unchanged in substance.
   static void *win32mmap(size_type size)
   {
      void *const ptr = VirtualAlloc(0, size, dl_mem_reserve|dl_mem_commit, dl_page_readwrite);
      return (ptr != 0) ? ptr : mfail();
   }

   //MEM_TOP_DOWN for direct maps, to keep them out of the way of the heap
   static void *win32direct_mmap(size_type size)
   {
      void *const ptr = VirtualAlloc(0, size, dl_mem_reserve|dl_mem_commit|dl_mem_top_down,
                                     dl_page_readwrite);
      return (ptr != 0) ? ptr : mfail();
   }

   //VirtualFree releases one whole reservation at a time, and the caller
   //may be releasing part of a run the heap coalesced, so walk the regions.
   static int win32munmap(void *ptr, size_type size)
   {
      ::dl_win_memory_basic_information minfo;
      char *cptr = (char *)ptr;
      while(size){
         if(VirtualQuery(cptr, (::_MEMORY_BASIC_INFORMATION *)&minfo,
                         (dl_win_size_t)sizeof(minfo)) == 0)
            return -1;
         if(minfo.BaseAddress != cptr || minfo.AllocationBase != cptr ||
            minfo.State != dl_mem_commit || minfo.RegionSize > size)
            return -1;
         if(VirtualFree(cptr, 0, dl_mem_release) == 0)
            return -1;
         cptr += minfo.RegionSize;
         size -= minfo.RegionSize;
      }
      return 0;
   }

   //GetSystemInfo without <windows.h>: the caller passes the stand-in above.
   static void dl_get_system_info(::dl_win_system_info *si)
   {  GetSystemInfo((::_SYSTEM_INFO *)si);  }

   #endif   //BOOST_WINDOWS

   // ==== size_type and alignment properties ====
   // ------------------- size_type and alignment properties --------------------

   // The byte and bit size of a size_type
   static const size_type size_t_size = ((sizeof(size_type)));
   static const size_type size_t_bitsize = ((sizeof(size_type) << 3));

   // Some constants coerced to size_type
   // Annoying but necessary to avoid errors on some platforms
   static const size_type size_t_zero = (((size_type)0));
   static const size_type size_t_one = (((size_type)1));
   static const size_type use_mmap_bit = (size_t_one);
   static const size_type size_t_two = (((size_type)2));
   static const size_type size_t_four = (((size_type)4));
   static const size_type two_size_t_sizes = ((size_t_size<<1));
   static const size_type four_size_t_sizes = ((size_t_size<<2));
   static const size_type six_size_t_sizes = ((four_size_t_sizes+two_size_t_sizes));
   static const size_type half_max_size_t = ((max_size_t / 2U));

   // The bit mask value corresponding to malloc_alignment
   static const size_type chunk_align_mask = ((malloc_alignment - size_t_one));

   // True if address a has acceptable alignment
   BOOST_CONTAINER_FORCEINLINE static bool is_aligned(const void *A)
      {  return ((((size_type)((A)) & (chunk_align_mask)) == 0));  }

   // the number of bytes to offset an address to align it
   BOOST_CONTAINER_FORCEINLINE static size_type align_offset(const void *A)
      {  return (((((size_type)(A) & chunk_align_mask) == 0)? 0 :
         ((malloc_alignment - ((size_type)(A) & chunk_align_mask)) & chunk_align_mask)));  }
   BOOST_CONTAINER_FORCEINLINE static size_type align_offset(size_type A)
      {  return (((((size_type)(A) & chunk_align_mask) == 0)? 0 :
         ((malloc_alignment - ((size_type)(A) & chunk_align_mask)) & chunk_align_mask)));  }

   //The struct itself lives in layout_type - see default_dlmalloc_layout.
   typedef typename layout_type::malloc_chunk malloc_chunk;

   typedef malloc_chunk  mchunk;
   typedef malloc_chunk* mchunkptr;
   typedef malloc_chunk* sbinptr;  // The type of bins of chunks
   typedef unsigned int bindex_t;         // Described below
   typedef unsigned int binmap_t;         // Described below
   typedef unsigned int flag_t;           // The type of various bit flag sets

   // ------------------- Chunks sizes and alignments -----------------------

   static const size_type mchunk_size = ((sizeof(mchunk)));

   //One constant with two possible values: a conditional expression says
   //that in one declaration, where two #if arms needed two.
   static const size_type chunk_overhead = (footers ? two_size_t_sizes : size_t_size);
   //allocation_payload is the same figure, spelt without the constants
   //above so that it can be declared with the rest of the interface
   BOOST_CONTAINER_STATIC_ASSERT((allocation_payload == chunk_overhead));

   // MMapped chunks need a second word of overhead ...
   static const size_type mmap_chunk_overhead = ((two_size_t_sizes));
   // ... and additional padding for fake next-chunk at foot
   static const size_type mmap_foot_pad = ((four_size_t_sizes));

   // The smallest size we can malloc is an aligned minimal chunk
   static const size_type min_chunk_size = (((mchunk_size + chunk_align_mask) & ~chunk_align_mask));

   //One place for the two casts the chunk helpers all need, so that each is
   //written once and says why it is safe.
   //
   //bytes_at() drops const: the helpers below take a const pointer because
   //they do not write through it themselves, but every one of them hands out
   //a chunk its caller writes to, so the chunk cannot be const.
   //
   //chunk_at() reaches a type of stricter alignment, going through void* so
   //that it says so deliberately. Every address it is given is one the heap
   //itself aligned - a segment base, a chunk boundary, or a bin slot.
   BOOST_CONTAINER_FORCEINLINE static char *bytes_at(const void *p)
      {  return static_cast<char *>(const_cast<void *>(p));  }
   BOOST_CONTAINER_FORCEINLINE static mchunkptr chunk_at(const void *p)
      {  return static_cast<mchunkptr>(const_cast<void *>(p));  }

   // conversion from malloc headers to user pointers, and back
   BOOST_CONTAINER_FORCEINLINE static void * chunk2mem(const void *p)
      {  return bytes_at(p) + two_size_t_sizes;  }
   BOOST_CONTAINER_FORCEINLINE static mchunkptr mem2chunk(const void *mem)
      {  return chunk_at(bytes_at(mem) - two_size_t_sizes);  }
   BOOST_CONTAINER_FORCEINLINE static mchunkptr mem2chunk(size_type mem)
      {  return chunk_at(reinterpret_cast<char *>(mem) - two_size_t_sizes);  }
   // chunk associated with aligned address A
   BOOST_CONTAINER_FORCEINLINE static mchunkptr align_as_chunk(char *A)
      {  return chunk_at(A + align_offset(chunk2mem(A)));  }

   // Bounds on request (not chunk) sizes.
   static const size_type max_request = (((0 - min_chunk_size) << 2));
   static const size_type min_request = ((min_chunk_size - chunk_overhead - size_t_one));

   // pad request bytes into a usable size
   BOOST_CONTAINER_FORCEINLINE static size_type pad_request(size_type req)
      {  return ((((req) + chunk_overhead + chunk_align_mask) & ~chunk_align_mask));  }

   // pad request, checking for minimum (but not maximum)
   BOOST_CONTAINER_FORCEINLINE static size_type request2size(size_type req)
      {  return ((((req) < min_request)? min_chunk_size : pad_request(req)));  }


   // ------------------ Operations on head and foot fields -----------------

   //
   //The head field of a chunk is or'ed with pinuse_bit when previous
   //adjacent chunk in use, and or'ed with cinuse_bit if this chunk is in
   //use, unless mmapped, in which case both bits are cleared.
   //
   //flag4_bit is not used by this malloc, but might be useful in extensions.
   //

   static const size_type pinuse_bit = ((size_t_one));
   static const size_type cinuse_bit = ((size_t_two));
   static const size_type flag4_bit = ((size_t_four));
   static const size_type inuse_bits = ((pinuse_bit|cinuse_bit));
   static const size_type flag_bits = ((pinuse_bit|cinuse_bit|flag4_bit));

   // Head value for fenceposts
   static const size_type fencepost_head = ((inuse_bits|size_t_size));

   // extraction of fields from head words
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static size_type cinuse(const Chunk *p)
      {  return (((p)->head & cinuse_bit));  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static size_type pinuse(const Chunk *p)
      {  return (((p)->head & pinuse_bit));  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static size_type flag4inuse(const Chunk *p)
      {  return (((p)->head & flag4_bit));  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool is_inuse(const Chunk *p)
      {  return ((((p)->head & inuse_bits) != pinuse_bit));  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool is_mmapped(const Chunk *p)
      {  return ((((p)->head & inuse_bits) == 0));  }

   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static size_type chunksize(const Chunk *p)
      {  return (((p)->head & ~(flag_bits)));  }

   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static void clear_pinuse(Chunk *p)
      {  ((p)->head &= ~pinuse_bit);  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static void set_flag4(Chunk *p)
      {  ((p)->head |= flag4_bit);  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static void clear_flag4(Chunk *p)
      {  ((p)->head &= ~flag4_bit);  }

   // Treat space at ptr +/- offset as a chunk
   BOOST_CONTAINER_FORCEINLINE static mchunkptr chunk_plus_offset(const void *p, size_type s)
      {  return chunk_at(bytes_at(p) + s);  }
   BOOST_CONTAINER_FORCEINLINE static mchunkptr chunk_minus_offset(const void *p, size_type s)
      {  return chunk_at(bytes_at(p) - s);  }

   // Ptr to next or previous physical malloc_chunk.
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static mchunkptr next_chunk(const Chunk *p)
      {  return chunk_at(bytes_at(p) + ((p)->head & ~flag_bits));  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static mchunkptr prev_chunk(const Chunk *p)
      {  return chunk_at(bytes_at(p) - (p)->prev_foot);  }

   // extract next chunk's pinuse bit
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static size_type next_pinuse(const Chunk *p)
      {  return (((next_chunk(p)->head) & pinuse_bit));  }

   // Get/set size at footer
   BOOST_CONTAINER_FORCEINLINE static size_type get_foot(const void *p, size_type s)
      {  return chunk_at(bytes_at(p) + s)->prev_foot;  }
   BOOST_CONTAINER_FORCEINLINE static void set_foot(const void *p, size_type s)
      {  chunk_at(bytes_at(p) + s)->prev_foot = s;  }

   // Set size, pinuse bit, and foot
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static void set_size_and_pinuse_of_free_chunk(Chunk *p, size_type s)
      {  ((p)->head = (s|pinuse_bit), set_foot(p, s));  }

   // Set size, pinuse bit, foot, and clear next pinuse
   template<class ChunkP, class ChunkN>
      BOOST_CONTAINER_FORCEINLINE static void set_free_with_pinuse(ChunkP *p, size_type s, ChunkN *n)
      {  (clear_pinuse(n), set_size_and_pinuse_of_free_chunk(p, s));  }

   // Get the internal overhead associated with chunk p
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static size_type overhead_for(const Chunk *p)
      {  return ((is_mmapped(p)? mmap_chunk_overhead : chunk_overhead));  }

   // Return true if malloced space is not necessarily cleared
   #if BOOST_CONTAINER_DL_MMAP_CLEARS
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool calloc_must_clear(const Chunk *p)
      {  return ((!is_mmapped(p)));  }
   #else // BOOST_CONTAINER_DL_MMAP_CLEARS
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool calloc_must_clear(const Chunk *p)
      {  (void)p;  return ((1));  }
   #endif // BOOST_CONTAINER_DL_MMAP_CLEARS

   //The struct itself lives in layout_type - see default_dlmalloc_layout.
   typedef typename layout_type::malloc_tree_chunk malloc_tree_chunk;

   typedef malloc_tree_chunk  tchunk;
   typedef malloc_tree_chunk* tchunkptr;
   typedef malloc_tree_chunk* tbinptr; // The type of bins of trees

   // A little helper macro for trees
   BOOST_CONTAINER_FORCEINLINE static tchunkptr leftmost_child(tchunkptr t)
      {  return (((t)->child[0] != 0? (t)->child[0] : (t)->child[1]));  }

   //The struct itself lives in layout_type - see default_dlmalloc_layout.
   typedef typename layout_type::malloc_segment malloc_segment;

   BOOST_CONTAINER_FORCEINLINE static flag_t is_mmapped_segment(const malloc_segment *S)
      {  return (((S)->sflags & use_mmap_bit));  }
   BOOST_CONTAINER_FORCEINLINE static flag_t is_extern_segment(const malloc_segment *S)
      {  return (((S)->sflags & extern_bit));  }

   typedef malloc_segment  msegment;
   typedef malloc_segment* msegmentptr;

   // Bin types, widths and sizes
   static const size_type nsmallbins = ((32U));
   static const size_type ntreebins = ((32U));

   //How far apart the small bins sit. The classic spacing is a fixed 8
   //bytes, which is the chunk-size granularity of a 32-bit target and half
   //of a 64-bit one - so on 64 bits every bin at an odd multiple of 8 stands
   //for a size no chunk can have. Half the bins are dead, exact-fit service stops at
   //240-byte chunks, and everything from 256 up pays the best-fit tree walk.
   //
   //wide_smallbins spaces them at the granularity itself instead, whatever
   //malloc_alignment makes that: all 32 bins become usable, and on a 64-bit
   //target chunks up to 496 bytes get an exact O(1) bin rather than the
   //tree. On a 32-bit target the granularity already IS 8, so it selects
   //that same spacing and costs nothing either way.
   static const size_type smallbin_shift =
      (wide_smallbins ? dtl::dlmalloc_log2<malloc_alignment>::value : 3U);
   static const size_type smallbin_width = ((size_t_one << smallbin_shift));

   //Where the tree bins start counting. This follows smallbin_shift, so
   //that the 32 small bins exactly span [0, 1 << treebin_shift) - but note
   //that this is NOT where small stops and large starts. rebased_smallbins
   //moves that boundary and leaves this shift alone: the tree index of a
   //size is still S >> treebin_shift, and the boundary simply lands inside
   //the first tree bin instead of at its start. See min_large_size below,
   //which is the boundary, and the assertion that keeps it in that bin.
   static const size_type treebin_shift =
      (smallbin_shift + dtl::dlmalloc_log2<nsmallbins>::value);

   //Where the small bins hand over to the tree. The plain index is
   //size >> smallbin_shift, so bin i stands for i * smallbin_width
   //and the bins below min_chunk_size stand for sizes no chunk can have:
   //dead, but still initialized, still two pointers each in malloc_state,
   //and still shifted across by the bitmap logic.
   //
   //rebased_smallbins indexes from min_chunk_size instead, so bin 0 means
   //min_chunk_size and every bin becomes reachable; the exact-fit range
   //grows by exactly the bins that were dead, and the boundary moves off
   //the power of two that treebin_shift alone implies. The three index
   //functions further down move with it - all four have to agree, which is
   //what the assertions below check.
   static const size_type min_large_size =
      (rebased_smallbins ? (min_chunk_size + (nsmallbins << smallbin_shift))
                         : (size_t_one << treebin_shift));

   //A bin holds exactly one chunk size, so its spacing has to be the
   //chunk-size granularity - the whole point of the wide spacing, and the
   //one thing a hand-picked malloc_alignment could quietly break.
   BOOST_CONTAINER_STATIC_ASSERT((!wide_smallbins || smallbin_width == malloc_alignment));

   //The small range and the tree range must meet exactly: no size may fall
   //in both, and none between them.
   BOOST_CONTAINER_STATIC_ASSERT((!rebased_smallbins ||
      min_large_size == (min_chunk_size + (nsmallbins << smallbin_shift))));

   //...and that boundary has to land inside the FIRST tree bin, whose range
   //is [1 << treebin_shift, minsize_for_tree_index(1)) - written out here
   //because that function is not a constant expression. Below the bin,
   //large chunks would compute tree index 0 while being smaller than the
   //bin's own minimum; at or above the second bin's minimum, tree bin 0
   //could never fill and compute_tree_index's X == 0 shortcut would become
   //reachable for real chunk sizes.
   BOOST_CONTAINER_STATIC_ASSERT((min_large_size >= (size_t_one << treebin_shift) &&
      min_large_size <  ((size_t_one << treebin_shift) |
                         (size_t_one << (treebin_shift - 1)))));
   static const size_type max_small_size = ((min_large_size - size_t_one));
   static const size_type max_small_request = ((max_small_size - chunk_align_mask - chunk_overhead));

   //The lock type: a spin mutex, as a plain member typedef. A different
   //mutex would be a one-line change here - it only has to be
   //default-constructible and match what the spin_mutex_* free functions
   //below take.
   typedef ::boost::container::dtl::spin_mutex_t mlock_t;

   //The struct itself lives in layout_type - see default_dlmalloc_layout, which
   //has one version with the lock and one without. lock_address() is how the
   //rest of the class asks where the lock is without naming a member that may
   //not exist: the arm that names it is only instantiated when use_locks says
   //there is one, and the other answers with a null pointer, so the code that
   //takes a lock still compiles when there is no lock to take.
   typedef typename layout_type::malloc_state malloc_state;

   typedef malloc_state*    mstate;


   // ==== malloc_params ====
   //The struct itself lives in layout_type - see default_dlmalloc_layout.
   typedef typename layout_type::malloc_params malloc_params;

   BOOST_CONTAINER_FORCEINLINE static mlock_t *lock_address(mstate m, dtl::true_)
      {  return &m->mutex;  }
   BOOST_CONTAINER_FORCEINLINE static mlock_t *lock_address(mstate m, dtl::false_)
      {  (void)m;  return 0;  }
   BOOST_CONTAINER_FORCEINLINE static mlock_t *lock_address(mstate m)
      {  return lock_address(m, dtl::bool_<use_locks>());  }

   // ==== system alloc setup, segment helpers ====
   // -------------------------- system alloc setup -------------------------

   // Operations on mflags

   BOOST_CONTAINER_FORCEINLINE static flag_t use_lock(mstate M)
      {  return (((M)->mflags &   use_lock_bit));  }

   BOOST_CONTAINER_FORCEINLINE static void enable_lock(mstate M)
      {  ((M)->mflags |=  use_lock_bit);  }

   BOOST_CONTAINER_FORCEINLINE static void disable_lock(mstate M, dtl::true_)
      {  ((M)->mflags &= ~use_lock_bit);  }

   BOOST_CONTAINER_FORCEINLINE static void disable_lock(mstate M, dtl::false_)
      {  (void)M;  }

   BOOST_CONTAINER_FORCEINLINE static void disable_lock(mstate M)
      {  disable_lock(M, dtl::bool_<use_locks>());  }

   BOOST_CONTAINER_FORCEINLINE static flag_t use_mmap(mstate M)
      {  return (((M)->mflags &   use_mmap_bit));  }

   BOOST_CONTAINER_FORCEINLINE static void enable_mmap(mstate M)
      {  ((M)->mflags |=  use_mmap_bit);  }

   BOOST_CONTAINER_FORCEINLINE static void disable_mmap(mstate M)
      {  (M)->mflags &= (flag_t)~(flag_t)use_mmap_bit;  }

   BOOST_CONTAINER_FORCEINLINE static flag_t use_noncontiguous(mstate M)
      {  return (((M)->mflags &   use_noncontiguous_bit));  }

   BOOST_CONTAINER_FORCEINLINE static void disable_contiguous(mstate M)
      {  ((M)->mflags |=  use_noncontiguous_bit);  }

   BOOST_CONTAINER_FORCEINLINE static void set_lock(mstate M, int L)
      {  (M)->mflags = (flag_t)((L) ? ((M)->mflags |  (flag_t)use_lock_bit)
                                      : ((M)->mflags & (flag_t)~(flag_t)use_lock_bit));  }

   // page-align a size
   BOOST_CONTAINER_FORCEINLINE size_type page_align(size_type S)
      {  return ((((S) + (m_params.page_size - size_t_one)) & ~(m_params.page_size - size_t_one)));  }

   // granularity-align a size, to a granularity given rather than this
   // heap's - create() has to size a mapping before there is a heap
   BOOST_CONTAINER_FORCEINLINE static size_type granularity_align_to
      (size_type S, size_type gsize)
      {  return ((((S) + (gsize - size_t_one)) & ~(gsize - size_t_one)));  }

   // granularity-align a size
   BOOST_CONTAINER_FORCEINLINE size_type granularity_align(size_type S)
      {  return granularity_align_to(S, m_params.granularity);  }

   // For mmap, use granularity alignment on windows, else page-align
   #if defined(BOOST_WINDOWS)
   BOOST_CONTAINER_FORCEINLINE size_type mmap_align(size_type S)
      {  return (granularity_align(S));  }
   #else
   BOOST_CONTAINER_FORCEINLINE size_type mmap_align(size_type S)
      {  return (page_align(S));  }
   #endif

   // For sys_alloc, enough padding to ensure can malloc request on success
   BOOST_CONTAINER_FORCEINLINE static size_type sys_alloc_padding()
      {  return ((top_foot_size() + malloc_alignment));  }

   BOOST_CONTAINER_FORCEINLINE bool is_page_aligned(size_type S)
      {  return ((((size_type)(S) & (m_params.page_size - size_t_one)) == 0));  }
   BOOST_CONTAINER_FORCEINLINE bool is_granularity_aligned(size_type S)
      {  return ((((size_type)(S) & (m_params.granularity - size_t_one)) == 0));  }

   //  True if segment S holds address A
   BOOST_CONTAINER_FORCEINLINE static bool segment_holds(msegmentptr S, const void *A)
      {
         const char *const a = static_cast<const char *>(A);
         return a >= S->base && a < S->base + S->size;
      }

   // Return segment holding given address
   msegmentptr segment_holding(char* addr)
   {
      mstate m = &m_state;
      msegmentptr sp = &m->seg;
      for (;;) {
         if (addr >= sp->base && addr < sp->base + sp->size)
            return sp;
         if ((sp = sp->next) == 0)
            return 0;
      }
   }

   // Return true if segment contains a segment link
   int has_segment_link(msegmentptr ss)
   {
      mstate m = &m_state;
      msegmentptr sp = &m->seg;
      for (;;) {
         if ((char*)sp >= ss->base && (char*)sp < ss->base + ss->size)
            return 1;
         if ((sp = sp->next) == 0)
            return 0;
      }
   }

   BOOST_CONTAINER_FORCEINLINE static bool should_trim(mstate M, size_type s)
      {  return (((s) > (M)->trim_check));  }

   //
   //top_foot_size() is padding at the end of a segment, including space
   //that may be needed to place segment records and fenceposts when new
   //noncontiguous segments are added.
   //
   BOOST_CONTAINER_FORCEINLINE static size_type top_foot_size()
      {  return ((align_offset(two_size_t_sizes)+pad_request(sizeof(malloc_segment))+min_chunk_size));  }



   // ==== hooks: preaction/postaction and the error actions ====
   // -------------------------------  Hooks --------------------------------

   //
   //preaction should be defined to return 0 on success, and nonzero on
   //failure. If you are not using locking, you can redefine these to do
   //anything you like.
   //

   BOOST_CONTAINER_FORCEINLINE static int preaction(mstate M, dtl::true_)
      {  return (((use_lock(M))? acquire_lock(lock_address(M)) : 0));  }
   BOOST_CONTAINER_FORCEINLINE static int preaction(mstate M, dtl::false_)
      {  (void)M;  return ((0));  }
   BOOST_CONTAINER_FORCEINLINE static int preaction(mstate M)
      {  return preaction(M, dtl::bool_<use_locks>());  }

   BOOST_CONTAINER_FORCEINLINE static void postaction(mstate M, dtl::true_)
      {  { if (use_lock(M)) release_lock(lock_address(M)); };  }
   BOOST_CONTAINER_FORCEINLINE static void postaction(mstate M, dtl::false_)
      {  (void)M;  }
   BOOST_CONTAINER_FORCEINLINE static void postaction(mstate M)
      {  postaction(M, dtl::bool_<use_locks>());  }

   //
   //corruption_error_action is triggered upon detected bad addresses.
   //usage_error_action is triggered on detected bad frees and
   //reallocs. The argument p is an address that might have triggered the
   //fault. It is ignored by the two predefined actions, but might be
   //useful in custom actions that try to help diagnose errors.
   //

   // What proceed_on_error does in place of aborting: forget every block and
   //start again from an empty heap. The second loop is what init_bins()
   //does, applied to the state handed in rather than to this one.
   static void reset_on_error(mstate m)
   {
      bindex_t i;
      m->smallmap = m->treemap = 0;
      m->dvsize = m->topsize = 0;
      m->seg.base = 0;
      m->seg.size = 0;
      m->seg.next = 0;
      m->top = m->dv = 0;
      for (i = 0; i < ntreebins; ++i)
         *treebin_at(m, i) = 0;
      for (i = 0; i < nsmallbins; ++i) {
         sbinptr bin = smallbin_at(m, i);
         bin->fd = bin->bk = bin;
      }
   }

   BOOST_CONTAINER_FORCEINLINE static void corruption_error_action(mstate m, dtl::true_)
      {  reset_on_error(m);  }
   BOOST_CONTAINER_FORCEINLINE static void corruption_error_action(mstate m, dtl::false_)
      {  (void)m;  do_abort();  }
   BOOST_CONTAINER_FORCEINLINE static void corruption_error_action(mstate m)
      {  corruption_error_action(m, dtl::bool_<proceed_on_error>());  }

   BOOST_CONTAINER_FORCEINLINE static void usage_error_action(mstate m, const void *p, dtl::true_)
      {  (void)m;  (void)p;  }
   BOOST_CONTAINER_FORCEINLINE static void usage_error_action(mstate m, const void *p, dtl::false_)
      {  (void)m;  (void)p;  do_abort();  }
   BOOST_CONTAINER_FORCEINLINE static void usage_error_action(mstate m, const void *p)
      {  usage_error_action(m, p, dtl::bool_<proceed_on_error>());  }



   // ==== debugging setup ====
   // -------------------------- Debugging setup ----------------------------

   BOOST_CONTAINER_FORCEINLINE void check_free_chunk(mchunkptr P, dtl::true_)
      {  do_check_free_chunk(P);  }
   BOOST_CONTAINER_FORCEINLINE void check_free_chunk(mchunkptr P, dtl::false_)
      {  (void)P;  }
   BOOST_CONTAINER_FORCEINLINE void check_free_chunk(mchunkptr P)
      {  check_free_chunk(P, dtl::bool_<debug>());  }

   BOOST_CONTAINER_FORCEINLINE void check_inuse_chunk(mchunkptr P, dtl::true_)
      {  do_check_inuse_chunk(P);  }
   BOOST_CONTAINER_FORCEINLINE void check_inuse_chunk(mchunkptr P, dtl::false_)
      {  (void)P;  }
   BOOST_CONTAINER_FORCEINLINE void check_inuse_chunk(mchunkptr P)
      {  check_inuse_chunk(P, dtl::bool_<debug>());  }

   BOOST_CONTAINER_FORCEINLINE void check_top_chunk(mchunkptr P, dtl::true_)
      {  do_check_top_chunk(P);  }
   BOOST_CONTAINER_FORCEINLINE void check_top_chunk(mchunkptr P, dtl::false_)
      {  (void)P;  }
   BOOST_CONTAINER_FORCEINLINE void check_top_chunk(mchunkptr P)
      {  check_top_chunk(P, dtl::bool_<debug>());  }

   BOOST_CONTAINER_FORCEINLINE void check_malloced_chunk(void *P, size_type N, dtl::true_)
      {  do_check_malloced_chunk(P,N);  }
   BOOST_CONTAINER_FORCEINLINE void check_malloced_chunk(void *P, size_type N, dtl::false_)
      {  (void)P;  (void)N;  }
   BOOST_CONTAINER_FORCEINLINE void check_malloced_chunk(void *P, size_type N)
      {  check_malloced_chunk(P, N, dtl::bool_<debug>());  }

   BOOST_CONTAINER_FORCEINLINE void check_mmapped_chunk(mchunkptr P, dtl::true_)
      {  do_check_mmapped_chunk(P);  }
   BOOST_CONTAINER_FORCEINLINE void check_mmapped_chunk(mchunkptr P, dtl::false_)
      {  (void)P;  }
   BOOST_CONTAINER_FORCEINLINE void check_mmapped_chunk(mchunkptr P)
      {  check_mmapped_chunk(P, dtl::bool_<debug>());  }

   BOOST_CONTAINER_FORCEINLINE void check_malloc_state(dtl::true_)
      {  do_check_malloc_state();  }
   BOOST_CONTAINER_FORCEINLINE void check_malloc_state(dtl::false_)
      {  }
   BOOST_CONTAINER_FORCEINLINE void check_malloc_state()
      {  check_malloc_state(dtl::bool_<debug>());  }

   // ==== indexing bins and bin maps ====

   BOOST_CONTAINER_FORCEINLINE static bool is_small(size_type s, dtl::true_)
      {  return ((s) >= min_chunk_size && (s) < min_large_size);  }

   BOOST_CONTAINER_FORCEINLINE static bool is_small(size_type s, dtl::false_)
      {  return ((s) < min_large_size);  }

   BOOST_CONTAINER_FORCEINLINE static bool is_small(size_type s)
      {  return is_small(s, dtl::bool_<rebased_smallbins>());  }

   BOOST_CONTAINER_FORCEINLINE static bindex_t small_index(size_type s, dtl::true_)
      {  return ((bindex_t)(((((s) - min_chunk_size) >> smallbin_shift)) & (nsmallbins - 1)));  }

   BOOST_CONTAINER_FORCEINLINE static bindex_t small_index(size_type s, dtl::false_)
      {  return ((bindex_t)((((s)  >> smallbin_shift)) & (nsmallbins - 1)));  }

   BOOST_CONTAINER_FORCEINLINE static bindex_t small_index(size_type s)
      {  return small_index(s, dtl::bool_<rebased_smallbins>());  }

   BOOST_CONTAINER_FORCEINLINE static size_type small_index2size(size_type i, dtl::true_)
      {  return ((((i) << smallbin_shift) + min_chunk_size));  }

   BOOST_CONTAINER_FORCEINLINE static size_type small_index2size(size_type i, dtl::false_)
      {  return (((i)  << smallbin_shift));  }

   BOOST_CONTAINER_FORCEINLINE static size_type small_index2size(size_type i)
      {  return small_index2size(i, dtl::bool_<rebased_smallbins>());  }

   BOOST_CONTAINER_FORCEINLINE static bindex_t min_small_index()
      {  return ((small_index(min_chunk_size)));  }

   // addressing by index. See above about smallbin repositioning
   BOOST_CONTAINER_FORCEINLINE static sbinptr smallbin_at(mstate M, size_type i)
      {  return chunk_at(&(M)->smallbins[(i) << 1]);  }

   BOOST_CONTAINER_FORCEINLINE static tbinptr * treebin_at(mstate M, size_type i)
      {  return ((&((M)->treebins[i])));  }

   // assign tree index for size S to variable I. Use x86 asm if possible
   #if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
   BOOST_CONTAINER_FORCEINLINE static void compute_tree_index(size_type S, bindex_t &I)
   {
      unsigned int X = (unsigned int)(S >> treebin_shift);
      if (X == 0)
         I = 0;
      else if (X > 0xFFFF)
         I = ntreebins-1;
      else {
         unsigned int K = (unsigned) sizeof(X)*__CHAR_BIT__ - 1 - (unsigned) __builtin_clz(X);
         I =  (bindex_t)((K << 1) + ((S >> (K + (treebin_shift-1)) & 1)));
      }
   }

   #elif defined (__INTEL_COMPILER)
   BOOST_CONTAINER_FORCEINLINE static void compute_tree_index(size_type S, bindex_t &I)
   {
      size_type X = S >> treebin_shift;
      if (X == 0)
         I = 0;
      else if (X > 0xFFFF)
         I = ntreebins-1;
      else {
         unsigned int K = _bit_scan_reverse (X);
         I =  (bindex_t)((K << 1) + ((S >> (K + (treebin_shift-1)) & 1)));
      }
   }

   #elif defined(_MSC_VER) && _MSC_VER>=1300
   BOOST_CONTAINER_FORCEINLINE static void compute_tree_index(size_type S, bindex_t &I)
   {
      size_type X = S >> treebin_shift;
      if (X == 0)
         I = 0;
      else if (X > 0xFFFF)
         I = ntreebins-1;
      else {
         unsigned int K;
         _BitScanReverse((unsigned long *) &K, (unsigned long) X);
         I =  (bindex_t)((K << 1) + ((S >> (K + (treebin_shift-1)) & 1)));
      }
   }

   #else // GNUC
   BOOST_CONTAINER_FORCEINLINE static void compute_tree_index(size_type S, bindex_t &I)
   {
      size_type X = S >> treebin_shift;
      if (X == 0)
         I = 0;
      else if (X > 0xFFFF)
         I = ntreebins-1;
      else {
         unsigned int Y = (unsigned int)X;
         unsigned int N = ((Y - 0x100) >> 16) & 8;
         unsigned int K = (((Y <<= N) - 0x1000) >> 16) & 4;
         N += K;
         N += K = (((Y <<= K) - 0x4000) >> 16) & 2;
         K = 14 - N + ((Y <<= K) >> 15);
         I = (K << 1) + ((S >> (K + (treebin_shift-1)) & 1));
      }
   }
   #endif // GNUC

   // Bit representing maximum resolved size in a treebin at i
   BOOST_CONTAINER_FORCEINLINE static size_type bit_for_tree_index(size_type i)
      {  return ((i == ntreebins-1)? (size_t_bitsize-1) : (((i) >> 1) + treebin_shift - 2));  }

   // Shift placing maximum resolved bit in a treebin at i as sign bit
   BOOST_CONTAINER_FORCEINLINE static size_type leftshift_for_tree_index(size_type i)
      {  return (((i == ntreebins-1)? 0 :
         ((size_t_bitsize-size_t_one) - (((i) >> 1) + treebin_shift - 2))));  }

   // The size of the smallest chunk held in bin with index i
   BOOST_CONTAINER_FORCEINLINE static size_type minsize_for_tree_index(size_type i)
      {  return (((size_t_one << (((i) >> 1) + treebin_shift)) |
         (((size_type)((i) & size_t_one)) << (((i) >> 1) + treebin_shift - 1))));  }


   // ------------------------ Operations on bin maps -----------------------

   // bit corresponding to given index
   BOOST_CONTAINER_FORCEINLINE static binmap_t idx2bit(size_type i)
      {  return (((binmap_t)(1) << (i)));  }

   // Mark/Clear bits with given index
   BOOST_CONTAINER_FORCEINLINE static void mark_smallmap(mstate M, size_type i)
      {  ((M)->smallmap |=  idx2bit(i));  }
   BOOST_CONTAINER_FORCEINLINE static void clear_smallmap(mstate M, size_type i)
      {  ((M)->smallmap &= ~idx2bit(i));  }
   BOOST_CONTAINER_FORCEINLINE static binmap_t smallmap_is_marked(mstate M, size_type i)
      {  return (((M)->smallmap &   idx2bit(i)));  }

   BOOST_CONTAINER_FORCEINLINE static void mark_treemap(mstate M, size_type i)
      {  ((M)->treemap  |=  idx2bit(i));  }
   BOOST_CONTAINER_FORCEINLINE static void clear_treemap(mstate M, size_type i)
      {  ((M)->treemap  &= ~idx2bit(i));  }
   BOOST_CONTAINER_FORCEINLINE static binmap_t treemap_is_marked(mstate M, size_type i)
      {  return (((M)->treemap  &   idx2bit(i)));  }

   // isolate the least set bit of a bitmap
   BOOST_CONTAINER_FORCEINLINE static binmap_t least_bit(binmap_t x)
      {  return (((x) & (0 - (x))));  }

   // mask with all bits to left of least bit of x on
   BOOST_CONTAINER_FORCEINLINE static binmap_t left_bits(binmap_t x)
      {  return ((((x)<<1) | (0 - ((x)<<1))));  }

   // mask with all bits to left of or equal to least bit of x on
   BOOST_CONTAINER_FORCEINLINE static binmap_t same_or_left_bits(binmap_t x)
      {  return (((x) | (0 - (x))));  }

   // index corresponding to given bit. Use x86 asm if possible

   #if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
   BOOST_CONTAINER_FORCEINLINE static void compute_bit2idx(binmap_t X, bindex_t &I)
   {
      unsigned int J;
      J = (unsigned int)__builtin_ctz(X);
      I = (bindex_t)J;
   }
   #elif defined (__INTEL_COMPILER)
   BOOST_CONTAINER_FORCEINLINE static void compute_bit2idx(binmap_t X, bindex_t &I)
   {

      unsigned int J;
      J = _bit_scan_forward (X);
      I = (bindex_t)J;
   }

   #elif defined(_MSC_VER) && _MSC_VER>=1300
   BOOST_CONTAINER_FORCEINLINE static void compute_bit2idx(binmap_t X, bindex_t &I)
   {
      unsigned int J;
      _BitScanForward((unsigned long *) &J, X);
      I = (bindex_t)J;
   }

   #else
   BOOST_CONTAINER_FORCEINLINE static void compute_bit2idx(binmap_t X, bindex_t &I)
   {
      unsigned int Y = X - 1;
      unsigned int K = Y >> (16-4) & 16;
      unsigned int N = K;        Y >>= K;
      N += K = Y >> (8-3) &  8;  Y >>= K;
      N += K = Y >> (4-2) &  4;  Y >>= K;
      N += K = Y >> (2-1) &  2;  Y >>= K;
      N += K = Y >> (1-0) &  1;  Y >>= K;
      I = (bindex_t)(N + Y);
   }
   #endif // GNUC


   // ==== runtime check support ====

   //Check that address a is at least as high as any obtained from the system
   BOOST_CONTAINER_FORCEINLINE static bool ok_address(mstate M, const void *a, dtl::true_)
      {  return static_cast<const char *>(a) >= (M)->least_addr;  }
   BOOST_CONTAINER_FORCEINLINE static bool ok_address(mstate M, const void *a, dtl::false_)
      {  (void)M;  (void)a;  return ((1));  }
   BOOST_CONTAINER_FORCEINLINE static bool ok_address(mstate M, const void *a)
      {  return ok_address(M, a, dtl::bool_<!insecure>());  }

   // Check if address of next chunk n is higher than base chunk p
   BOOST_CONTAINER_FORCEINLINE static bool ok_next(const void *p, const void *n, dtl::true_)
      {  return static_cast<const char *>(p) < static_cast<const char *>(n);  }
   BOOST_CONTAINER_FORCEINLINE static bool ok_next(const void *p, const void *n, dtl::false_)
      {  (void)p;  (void)n;  return ((1));  }
   BOOST_CONTAINER_FORCEINLINE static bool ok_next(const void *p, const void *n)
      {  return ok_next(p, n, dtl::bool_<!insecure>());  }

   // Check if p has inuse status
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool ok_inuse(const Chunk *p, dtl::true_)
      {  return (is_inuse(p));  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool ok_inuse(const Chunk *p, dtl::false_)
      {  (void)p;  return ((1));  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool ok_inuse(const Chunk *p)
      {  return ok_inuse(p, dtl::bool_<!insecure>());  }

   // Check if p has its pinuse bit on
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool ok_pinuse(const Chunk *p, dtl::true_)
      {  return (pinuse(p));  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool ok_pinuse(const Chunk *p, dtl::false_)
      {  (void)p;  return ((1));  }
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE static bool ok_pinuse(const Chunk *p)
      {  return ok_pinuse(p, dtl::bool_<!insecure>());  }

   // Check if (alleged) mstate m has expected magic field. Only the arm that
   //compares reads m_params, so that arm needs the heap; both are members
   //and the one that always says yes simply ignores it.
   BOOST_CONTAINER_FORCEINLINE bool ok_magic(mstate M, dtl::true_) const
      {  return (((M)->magic == m_params.magic));  }
   BOOST_CONTAINER_FORCEINLINE bool ok_magic(mstate M, dtl::false_) const
      {  return (((void)(M), 1));  }
   BOOST_CONTAINER_FORCEINLINE bool ok_magic(mstate M) const
      {  return ok_magic(M, dtl::bool_<footers && !insecure>());  }

   // In gcc, use __builtin_expect to minimize impact of checks
   // In gcc, use __builtin_expect to minimize impact of checks. Which
   //compiler is in use stays a question for the preprocessor - the builtin
   //exists nowhere else - but whether to check at all is a question for
   //insecure, and that one the overloads answer.
   BOOST_CONTAINER_FORCEINLINE static bool rtcheck(bool e, dtl::true_)
      {
         #if defined(__GNUC__) && __GNUC__ >= 3
         return (__builtin_expect(e, 1));
         #else // GNUC
         return ((e));
         #endif // GNUC
      }
   BOOST_CONTAINER_FORCEINLINE static bool rtcheck(bool e, dtl::false_)
      {  (void)e;  return ((1));  }
   BOOST_CONTAINER_FORCEINLINE static bool rtcheck(bool e)
      {  return rtcheck(e, dtl::bool_<!insecure>());  }

   // macros to set up inuse chunks with or without footers

   // Set foot of inuse chunk to be xor of mstate and seed. footers is the
   //only thing the two arms differed by, and it is only this one write:
   //once the empty overload takes its place, the three setters below have
   //a single body each and always call it.
   BOOST_CONTAINER_FORCEINLINE void mark_inuse_foot(mstate M, const void *p, size_type s, dtl::true_) const
      {  chunk_at(bytes_at(p) + s)->prev_foot = (size_type)(M) ^ m_params.magic;  }
   BOOST_CONTAINER_FORCEINLINE void mark_inuse_foot(mstate M, const void *p, size_type s, dtl::false_) const
      {  (void)M;  (void)p;  (void)s;  }
   BOOST_CONTAINER_FORCEINLINE void mark_inuse_foot(mstate M, const void *p, size_type s) const
      {  mark_inuse_foot(M, p, s, dtl::bool_<footers>());  }

   // Which heap a chunk belongs to: with footers, read back out of the foot
   //mark_inuse_foot() wrote; without one, a chunk carries no mark and there
   //is no other heap it could have come from.
   BOOST_CONTAINER_FORCEINLINE mstate get_mstate_for(const malloc_chunk *p, dtl::true_)
      {  return (mstate)(chunk_at(bytes_at(p) + chunksize(p))->prev_foot
                           ^ m_params.magic);  }
   BOOST_CONTAINER_FORCEINLINE mstate get_mstate_for(const malloc_chunk *p, dtl::false_)
      {  (void)p;  return (&m_state);  }
   BOOST_CONTAINER_FORCEINLINE mstate get_mstate_for(const malloc_chunk *p)
      {  return get_mstate_for(p, dtl::bool_<footers>());  }

   // Macros for setting head/foot of non-mmapped chunks

   // Set cinuse bit and pinuse bit of next chunk
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE void set_inuse(mstate M, Chunk *p, size_type s) const
      {  ((p)->head = (((p)->head & pinuse_bit)|s|cinuse_bit),
         chunk_at(bytes_at(p) + s)->head |= pinuse_bit,
         mark_inuse_foot(M,p,s));  }

   // Set cinuse and pinuse of this chunk and pinuse of next chunk
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE void set_inuse_and_pinuse(mstate M, Chunk *p, size_type s) const
      {  ((p)->head = (s|pinuse_bit|cinuse_bit),
         chunk_at(bytes_at(p) + s)->head |= pinuse_bit,
         mark_inuse_foot(M,p,s));  }

   // Set size, cinuse and pinuse bit of this chunk
   template<class Chunk>
      BOOST_CONTAINER_FORCEINLINE void set_size_and_pinuse_of_inuse_chunk(mstate M, Chunk *p, size_type s) const
      {  ((p)->head = (s|pinuse_bit|cinuse_bit),
         mark_inuse_foot(M, p, s));  }


   // ==== debugging support bodies ====

   // Check properties of any chunk, whether free, inuse, mmapped etc
   void do_check_any_chunk(mchunkptr p)
   {
      mstate m = &m_state;
      BOOST_CONTAINER_DL_ASSERT((is_aligned(chunk2mem(p))) || (p->head == fencepost_head));
      BOOST_CONTAINER_DL_ASSERT(ok_address(m, p));
   }

   // Check properties of top chunk
   void do_check_top_chunk(mchunkptr p)
   {
      mstate m = &m_state;
      msegmentptr sp = segment_holding((char*)p);
      size_type  sz = p->head & ~inuse_bits; // third-lowest bit can be set!
      BOOST_CONTAINER_DL_ASSERT(sp != 0);
      BOOST_CONTAINER_DL_ASSERT((is_aligned(chunk2mem(p))) || (p->head == fencepost_head));
      BOOST_CONTAINER_DL_ASSERT(ok_address(m, p));
      BOOST_CONTAINER_DL_ASSERT(sz == m->topsize);
      BOOST_CONTAINER_DL_ASSERT(sz > 0);
      BOOST_CONTAINER_DL_ASSERT(sz == (size_type)((sp->base + sp->size) - bytes_at(p))
                                      - top_foot_size());
      BOOST_CONTAINER_DL_ASSERT(pinuse(p));
      BOOST_CONTAINER_DL_ASSERT(!pinuse(chunk_plus_offset(p, sz)));
   }

   // Check properties of (inuse) mmapped chunks
   void do_check_mmapped_chunk(mchunkptr p)
   {
      mstate m = &m_state;
      size_type  sz = chunksize(p);
      size_type len = (sz + (p->prev_foot) + mmap_foot_pad);
      BOOST_CONTAINER_DL_ASSERT(is_mmapped(p));
      BOOST_CONTAINER_DL_ASSERT(use_mmap(m));
      BOOST_CONTAINER_DL_ASSERT((is_aligned(chunk2mem(p))) || (p->head == fencepost_head));
      BOOST_CONTAINER_DL_ASSERT(ok_address(m, p));
      BOOST_CONTAINER_DL_ASSERT(!is_small(sz));
      BOOST_CONTAINER_DL_ASSERT((len & (m_params.page_size-size_t_one)) == 0);
      BOOST_CONTAINER_DL_ASSERT(chunk_plus_offset(p, sz)->head == fencepost_head);
      BOOST_CONTAINER_DL_ASSERT(chunk_plus_offset(p, sz+size_t_size)->head == 0);
   }

   // Check properties of inuse chunks
   void do_check_inuse_chunk(mchunkptr p)
   {
      mstate m = &m_state;  (void)m;
      do_check_any_chunk(p);
      BOOST_CONTAINER_DL_ASSERT(is_inuse(p));
      BOOST_CONTAINER_DL_ASSERT(next_pinuse(p));
      // If not pinuse and not mmapped, previous chunk has OK offset
      BOOST_CONTAINER_DL_ASSERT(is_mmapped(p) || pinuse(p) || next_chunk(prev_chunk(p)) == p);
      if (is_mmapped(p))
         do_check_mmapped_chunk(p);
   }

   // Check properties of free chunks
   void do_check_free_chunk(mchunkptr p)
   {
      mstate m = &m_state;
      size_type sz = chunksize(p);
      mchunkptr next = chunk_plus_offset(p, sz);
      do_check_any_chunk(p);
      BOOST_CONTAINER_DL_ASSERT(!is_inuse(p));
      BOOST_CONTAINER_DL_ASSERT(!next_pinuse(p));
      BOOST_CONTAINER_DL_ASSERT(!is_mmapped(p));
      if (p != m->dv && p != m->top) {
         if (sz >= min_chunk_size) {
            BOOST_CONTAINER_DL_ASSERT((sz & chunk_align_mask) == 0);
            BOOST_CONTAINER_DL_ASSERT(is_aligned(chunk2mem(p)));
            BOOST_CONTAINER_DL_ASSERT(next->prev_foot == sz);
            BOOST_CONTAINER_DL_ASSERT(pinuse(p));
            BOOST_CONTAINER_DL_ASSERT(next == m->top || is_inuse(next));
            BOOST_CONTAINER_DL_ASSERT(p->fd->bk == p);
            BOOST_CONTAINER_DL_ASSERT(p->bk->fd == p);
         }
         else  // markers are always of size size_t_size
            BOOST_CONTAINER_DL_ASSERT(sz == size_t_size);
      }
   }

   // Check properties of malloced chunks at the point they are malloced
   void do_check_malloced_chunk(void* mem, size_type s)
   {
      mstate m = &m_state;  (void)m;
      if (BOOST_LIKELY(mem != 0)) {
         mchunkptr p = mem2chunk(mem);
         size_type sz = p->head & ~inuse_bits;
         do_check_inuse_chunk(p);
         BOOST_CONTAINER_DL_ASSERT((sz & chunk_align_mask) == 0);
         BOOST_CONTAINER_DL_ASSERT(sz >= min_chunk_size);
         BOOST_CONTAINER_DL_ASSERT(sz >= s);
         // unless mmapped, size is less than min_chunk_size more than request
         BOOST_CONTAINER_DL_ASSERT(is_mmapped(p) || sz < (s + min_chunk_size));
      }
   }

   // Check a tree and its subtrees.
   void do_check_tree(tchunkptr t)
   {
      mstate m = &m_state;  (void)m;
      tchunkptr head = 0;
      tchunkptr u = t;
      bindex_t tindex = t->index;
      size_type tsize = chunksize(t);
      bindex_t idx;
      compute_tree_index(tsize, idx);
      BOOST_CONTAINER_DL_ASSERT(tindex == idx);
      BOOST_CONTAINER_DL_ASSERT(tsize >= min_large_size);
      BOOST_CONTAINER_DL_ASSERT(tsize >= minsize_for_tree_index(idx));
      BOOST_CONTAINER_DL_ASSERT((idx == ntreebins-1) || (tsize < minsize_for_tree_index((idx+1))));

      do { // traverse through chain of same-sized nodes
         do_check_any_chunk(((mchunkptr)u));
         BOOST_CONTAINER_DL_ASSERT(u->index == tindex);
         BOOST_CONTAINER_DL_ASSERT(chunksize(u) == tsize);
         BOOST_CONTAINER_DL_ASSERT(!is_inuse(u));
         BOOST_CONTAINER_DL_ASSERT(!next_pinuse(u));
         BOOST_CONTAINER_DL_ASSERT(u->fd->bk == u);
         BOOST_CONTAINER_DL_ASSERT(u->bk->fd == u);
         if (u->parent == 0) {
            BOOST_CONTAINER_DL_ASSERT(u->child[0] == 0);
            BOOST_CONTAINER_DL_ASSERT(u->child[1] == 0);
         }
         else {
            BOOST_CONTAINER_DL_ASSERT(head == 0); // only one node on chain has parent
            head = u;
            BOOST_CONTAINER_DL_ASSERT(u->parent != u);
            BOOST_CONTAINER_DL_ASSERT(u->parent->child[0] == u ||
               u->parent->child[1] == u ||
               *((tbinptr*)(u->parent)) == u);
            if (u->child[0] != 0) {
               BOOST_CONTAINER_DL_ASSERT(u->child[0]->parent == u);
               BOOST_CONTAINER_DL_ASSERT(u->child[0] != u);
               do_check_tree(u->child[0]);
            }
            if (u->child[1] != 0) {
               BOOST_CONTAINER_DL_ASSERT(u->child[1]->parent == u);
               BOOST_CONTAINER_DL_ASSERT(u->child[1] != u);
               do_check_tree(u->child[1]);
            }
            if (u->child[0] != 0 && u->child[1] != 0) {
               BOOST_CONTAINER_DL_ASSERT(chunksize(u->child[0]) < chunksize(u->child[1]));
            }
         }
         u = u->fd;
      } while (u != t);
      BOOST_CONTAINER_DL_ASSERT(head != 0);
   }

   //  Check all the chunks in a treebin.
   void do_check_treebin(bindex_t i)
   {
      mstate m = &m_state;
      tbinptr* tb = treebin_at(m, i);
      tchunkptr t = *tb;
      int empty = (m->treemap & (1U << i)) == 0;
      if (t == 0)
         BOOST_CONTAINER_DL_ASSERT(empty);
      if (!empty)
         do_check_tree(t);
   }

   //  Check all the chunks in a smallbin.
   void do_check_smallbin(bindex_t i)
   {
      mstate m = &m_state;
      sbinptr b = smallbin_at(m, i);
      mchunkptr p = b->bk;
      unsigned int empty = (m->smallmap & (1U << i)) == 0;
      if (p == b)
         BOOST_CONTAINER_DL_ASSERT(empty);
      if (!empty) {
         for (; p != b; p = p->bk) {
            size_type size = chunksize(p);
            mchunkptr q;
            // each chunk claims to be free
            do_check_free_chunk(p);
            // chunk belongs in bin
            BOOST_CONTAINER_DL_ASSERT(small_index(size) == i);
            BOOST_CONTAINER_DL_ASSERT(p->bk == b || chunksize(p->bk) == chunksize(p));
            // chunk is followed by an inuse chunk
            q = next_chunk(p);
            if (q->head != fencepost_head)
               do_check_inuse_chunk(q);
         }
      }
   }

   // Find x in a bin. Used in other check functions.
   int bin_find(mchunkptr x)
   {
      mstate m = &m_state;
      size_type size = chunksize(x);
      if (is_small(size)) {
         bindex_t sidx = small_index(size);
         sbinptr b = smallbin_at(m, sidx);
         if (smallmap_is_marked(m, sidx)) {
            mchunkptr p = b;
            do {
               if (p == x)
                  return 1;
            } while ((p = p->fd) != b);
         }
      }
      else {
         bindex_t tidx;
         compute_tree_index(size, tidx);
         if (treemap_is_marked(m, tidx)) {
            tchunkptr t = *treebin_at(m, tidx);
            size_type sizebits = size << leftshift_for_tree_index(tidx);
            while (t != 0 && chunksize(t) != size) {
               t = t->child[(sizebits >> (size_t_bitsize-size_t_one)) & 1];
               sizebits <<= 1;
            }
            if (t != 0) {
               tchunkptr u = t;
               do {
                  if (u == (tchunkptr)x)
                     return 1;
               } while ((u = u->fd) != t);
            }
         }
      }
      return 0;
   }

   // Traverse each chunk and check it; return total
   size_type traverse_and_check()
   {
      mstate m = &m_state;
      size_type sum = 0;
      if (is_initialized(m)) {
         msegmentptr s = &m->seg;
         sum += m->topsize + top_foot_size();
         while (s != 0) {
            mchunkptr q = align_as_chunk(s->base);
            mchunkptr lastq = 0;
            BOOST_CONTAINER_DL_ASSERT(pinuse(q));
            while (segment_holds(s, q) &&
               q != m->top && q->head != fencepost_head) {
               sum += chunksize(q);
               if (is_inuse(q)) {
                  BOOST_CONTAINER_DL_ASSERT(!bin_find(q));
                  do_check_inuse_chunk(q);
               }
               else {
                  BOOST_CONTAINER_DL_ASSERT(q == m->dv || bin_find(q));
                  BOOST_CONTAINER_DL_ASSERT(lastq == 0 || is_inuse(lastq)); // Not 2 consecutive free
                  do_check_free_chunk(q);
               }
               lastq = q;
               q = next_chunk(q);
            }
            s = s->next;
         }
      }
      return sum;
   }


   // Check all properties of malloc_state.
   void do_check_malloc_state()
   {
      mstate m = &m_state;
      bindex_t i;
      size_type total;
      // check bins
      for (i = 0; i < nsmallbins; ++i)
         do_check_smallbin(i);
      for (i = 0; i < ntreebins; ++i)
         do_check_treebin(i);

      if (m->dvsize != 0) { // check dv chunk
         do_check_any_chunk(m->dv);
         BOOST_CONTAINER_DL_ASSERT(m->dvsize == chunksize(m->dv));
         BOOST_CONTAINER_DL_ASSERT(m->dvsize >= min_chunk_size);
         BOOST_CONTAINER_DL_ASSERT(bin_find(m->dv) == 0);
      }

      if (m->top != 0) {   // check top chunk
         do_check_top_chunk(m->top);
         //BOOST_CONTAINER_DL_ASSERT(m->topsize == chunksize(m->top)); redundant
         BOOST_CONTAINER_DL_ASSERT(m->topsize > 0);
         BOOST_CONTAINER_DL_ASSERT(bin_find(m->top) == 0);
      }

      total = traverse_and_check();
      BOOST_CONTAINER_DL_ASSERT(total <= m->footprint);
      BOOST_CONTAINER_DL_ASSERT(m->footprint <= m->max_footprint);
   }

   // ==== operations on smallbins and trees ====
   // ----------------------- Operations on smallbins -----------------------

   // Link a free chunk into a smallbin
   BOOST_CONTAINER_FORCEINLINE static void insert_small_chunk(mstate M, mchunkptr P, size_type S)
   {
      bindex_t I  = small_index(S);
      mchunkptr B = smallbin_at(M, I);
      mchunkptr F = B;
      BOOST_CONTAINER_DL_ASSERT(S >= min_chunk_size);
      if (!smallmap_is_marked(M, I))
         mark_smallmap(M, I);
      else if (rtcheck(ok_address(M, B->fd)))
         F = B->fd;
      else {
         corruption_error_action(M);
      }
      B->fd = P;
      F->bk = P;
      P->fd = F;
      P->bk = B;
   }

   // Unlink a chunk from a smallbin
   BOOST_CONTAINER_FORCEINLINE static void unlink_small_chunk(mstate M, mchunkptr P, size_type S)
   {
      mchunkptr F = P->fd;
      mchunkptr B = P->bk;
      bindex_t I = small_index(S);
      BOOST_CONTAINER_DL_ASSERT(P != B);
      BOOST_CONTAINER_DL_ASSERT(P != F);
      BOOST_CONTAINER_DL_ASSERT(chunksize(P) == small_index2size(I));
      if (rtcheck(F == smallbin_at(M,I) || (ok_address(M, F) && F->bk == P))) {
         if (B == F) {
            clear_smallmap(M, I);
         }
         else if (rtcheck(B == smallbin_at(M,I) ||
            (ok_address(M, B) && B->fd == P))) {
            F->bk = B;
            B->fd = F;
         }
         else {
            corruption_error_action(M);
         }
      }
      else {
         corruption_error_action(M);
      }
   }

   // Unlink the first chunk from a smallbin
   BOOST_CONTAINER_FORCEINLINE static void unlink_first_small_chunk(mstate M, mchunkptr B, mchunkptr P, bindex_t I)
   {

      mchunkptr F = P->fd;
      BOOST_CONTAINER_DL_ASSERT(P != B);
      BOOST_CONTAINER_DL_ASSERT(P != F);
      BOOST_CONTAINER_DL_ASSERT(chunksize(P) == small_index2size(I));
      if (B == F) {
         clear_smallmap(M, I);
      }
      else if (rtcheck(ok_address(M, F) && F->bk == P)) {
         F->bk = B;
         B->fd = F;
      }
      else {
         corruption_error_action(M);
      }

   }

   // Replace dv node, binning the old one
   // Used only when dvsize known to be small
   BOOST_CONTAINER_FORCEINLINE static void replace_dv(mstate M, mchunkptr P, size_type S)
   {

      size_type DVS = M->dvsize;
      //"no dv" is spelled dvsize == 0, which is not a chunk size at all.
      //Asking is_small(0) would only work while bin 0 stands for size 0,
      //which it does not once the index is re-based at min_chunk_size, so
      //the intent is spelt out and the check means the same in both layouts.
      BOOST_CONTAINER_DL_ASSERT(DVS == 0 || is_small(DVS));
      if (DVS != 0) {
         mchunkptr DV = M->dv;
         insert_small_chunk(M, DV, DVS);
      }
      M->dvsize = S;
      M->dv = P;

   }

   // ------------------------- Operations on trees -------------------------

   // Insert chunk into tree
   BOOST_CONTAINER_FORCEINLINE static void insert_large_chunk(mstate M, tchunkptr X, size_type S)
   {

      tbinptr* H;
      bindex_t I;
      compute_tree_index(S, I);
      H = treebin_at(M, I);
      X->index = I;
      X->child[0] = X->child[1] = 0;
      if (!treemap_is_marked(M, I)) {
         mark_treemap(M, I);
         *H = X;
         X->parent = (tchunkptr)H;
         X->fd = X->bk = X;
      }
      else {
         tchunkptr T = *H;
         size_type K = S << leftshift_for_tree_index(I);
         for (;;) {
            if (chunksize(T) != S) {
               tchunkptr* C = &(T->child[(K >> (size_t_bitsize-size_t_one)) & 1]);
               K <<= 1;
               if (*C != 0)
                  T = *C;
               else if (rtcheck(ok_address(M, C))) {
                  *C = X;
                  X->parent = T;
                  X->fd = X->bk = X;
                  break;
               }
               else {
                  corruption_error_action(M);
                  break;
               }
            }
            else {
               tchunkptr F = T->fd;
               if (rtcheck(ok_address(M, T) && ok_address(M, F))) {
                  T->fd = F->bk = X;
                  X->fd = F;
                  X->bk = T;
                  X->parent = 0;
                  break;
               }
               else {
                  corruption_error_action(M);
                  break;
               }
            }
         }
      }

   }

   //
   //Unlink steps:
   //
   //1. If x is a chained node, unlink it from its same-sized fd/bk links
   //   and choose its bk node as its replacement.
   //2. If x was the last node of its size, but not a leaf node, it must
   //   be replaced with a leaf node (not merely one with an open left or
   //   right), to make sure that lefts and rights of descendents
   //   correspond properly to bit masks.  We use the rightmost descendent
   //   of x.  We could use any other leaf, but this is easy to locate and
   //   tends to counteract removal of leftmosts elsewhere, and so keeps
   //   paths shorter than minimally guaranteed.  This doesn't loop much
   //   because on average a node in a tree is near the bottom.
   //3. If x is the base of a chain (i.e., has parent links) relink
   //   x's parent and children to x's replacement (or null if none).
   //

   BOOST_CONTAINER_FORCEINLINE static void unlink_large_chunk(mstate M, tchunkptr X)
   {

      tchunkptr XP = X->parent;
      tchunkptr R;
      if (X->bk != X) {
         tchunkptr F = X->fd;
         R = X->bk;
         if (rtcheck(ok_address(M, F) && F->bk == X && R->fd == X)) {
            F->bk = R;
            R->fd = F;
         }
         else {
            corruption_error_action(M);
         }
      }
      else {
         tchunkptr* RP;
         if (((R = *(RP = &(X->child[1]))) != 0) ||
            ((R = *(RP = &(X->child[0]))) != 0)) {
            tchunkptr* CP;
            while ((*(CP = &(R->child[1])) != 0) ||
               (*(CP = &(R->child[0])) != 0)) {
               R = *(RP = CP);
            }
            if (rtcheck(ok_address(M, RP)))
               *RP = 0;
            else {
               corruption_error_action(M);
            }
         }
      }
      if (XP != 0) {
         tbinptr* H = treebin_at(M, X->index);
         if (X == *H) {
            if ((*H = R) == 0)
               clear_treemap(M, X->index);
         }
         else if (rtcheck(ok_address(M, XP))) {
            if (XP->child[0] == X)
               XP->child[0] = R;
            else
               XP->child[1] = R;
         }
         else
            corruption_error_action(M);
         if (R != 0) {
            if (rtcheck(ok_address(M, R))) {
               tchunkptr C0, C1;
               R->parent = XP;
               if ((C0 = X->child[0]) != 0) {
                  if (rtcheck(ok_address(M, C0))) {
                     R->child[0] = C0;
                     C0->parent = R;
                  }
                  else
                     corruption_error_action(M);
               }
               if ((C1 = X->child[1]) != 0) {
                  if (rtcheck(ok_address(M, C1))) {
                     R->child[1] = C1;
                     C1->parent = R;
                  }
                  else
                     corruption_error_action(M);
               }
            }
            else
               corruption_error_action(M);
         }
      }

   }

   // Relays to large vs small bin operations

   BOOST_CONTAINER_FORCEINLINE static void insert_chunk(mstate M, mchunkptr P, size_type S)
   {
      if (is_small(S))
         insert_small_chunk(M, P, S);
      else{
         tchunkptr TP = (tchunkptr)(P);
         insert_large_chunk(M, TP, S);
      }
   }

   BOOST_CONTAINER_FORCEINLINE static void unlink_chunk(mstate M, mchunkptr P, size_type S)
   {
      if (is_small(S))
         unlink_small_chunk(M, P, S);
      else{
         tchunkptr TP = (tchunkptr)(P);
         unlink_large_chunk(M, TP);
      }
   }

   // ==== direct-mmapping chunks ====
   // -----------------------  Direct-mmapping chunks -----------------------

   //
   //Directly mmapped chunks are set up with an offset to the start of
   //the mmapped region stored in the prev_foot field of the chunk. This
   //allows reconstruction of the required argument to MUNMAP when freed,
   //and also allows adjustment of the returned chunk to meet alignment
   //requirements (especially in memalign).
   //

   // Malloc using mmap
   void* mmap_alloc(size_type nb)
   {
      mstate m = &m_state;
      size_type mmsize = mmap_align(nb + six_size_t_sizes + chunk_align_mask);
      if (m->footprint_limit != 0) {
         size_type fp = m->footprint + mmsize;
         if (fp <= m->footprint || fp > m->footprint_limit)
            return 0;
      }
      if (mmsize > nb) {     // Check for wrap around 0
         char* mm = (char*)(call_direct_mmap(mmsize));
         if (mm != cmfail()) {
            size_type offset = align_offset(chunk2mem(mm));
            size_type psize = mmsize - offset - mmap_foot_pad;
            mchunkptr p = chunk_at(mm + offset);
            p->prev_foot = offset;
            p->head = psize;
            mark_inuse_foot(m, p, psize);
            chunk_plus_offset(p, psize)->head = fencepost_head;
            chunk_plus_offset(p, psize+size_t_size)->head = 0;

            if (m->least_addr == 0 || mm < m->least_addr)
               m->least_addr = mm;
            if ((m->footprint += mmsize) > m->max_footprint)
               m->max_footprint = m->footprint;
            BOOST_CONTAINER_DL_ASSERT(is_aligned(chunk2mem(p)));
            check_mmapped_chunk(p);
            return chunk2mem(p);
         }
      }
      return 0;
   }

   // Realloc using mmap
   mchunkptr mmap_resize(mchunkptr oldp, size_type nb, int flags)
   {
      mstate m = &m_state;
      size_type oldsize = chunksize(oldp);
      (void)flags; // placate people compiling -Wunused
      if (is_small(nb)) // Can't shrink mmap regions below small size
         return 0;
      // Keep old chunk if big enough but not too big
      if (oldsize >= nb + size_t_size &&
         (oldsize - nb) <= (m_params.granularity << 1))
         return oldp;
      else {
         size_type offset = oldp->prev_foot;
         size_type oldmmsize = oldsize + offset + mmap_foot_pad;
         //The new mapping has to hold the offset too. Sizing it as if the
         //chunk began at the start of the mapping is only right for a chunk
         //mmap_alloc() made: one that went through internal_memalign() keeps
         //the alignment slack in front of it - prev_foot says how much - and
         //psize below would then come out that much short of nb, so
         //reallocate(allocate_aligned(4096, 300000), 4201) would hand back a
         //4064-byte block and the caller would write past the mapping. Only
         //the mremap path reaches here; without it the caller falls back to
         //allocate, copy and free. test/dlmalloc_memalign_test.cpp pins it
         //down.
         size_type newmmsize = mmap_align(nb + offset + six_size_t_sizes + chunk_align_mask);
         char* cp = (char*)call_mremap((char*)oldp - offset,
            oldmmsize, newmmsize, flags);
         if (cp != cmfail()) {
            mchunkptr newp = chunk_at(cp + offset);
            size_type psize = newmmsize - offset - mmap_foot_pad;
            newp->head = psize;
            mark_inuse_foot(m, newp, psize);
            chunk_plus_offset(newp, psize)->head = fencepost_head;
            chunk_plus_offset(newp, psize+size_t_size)->head = 0;

            if (cp < m->least_addr)
               m->least_addr = cp;
            if ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
               m->max_footprint = m->footprint;
            check_mmapped_chunk(newp);
            return newp;
         }
      }
      return 0;
   }



   //The state and nothing else: the lock, the tuning, the magic and the
   //bins. This is init_user_mstate() without the segment, and it is where
   //every constructor starts.
   void init_state()
   {
      ::std::memset(&m_state, 0, sizeof(m_state));
      init_params();
      (void)initial_lock(lock_address(&m_state));
      m_state.mflags = m_params.default_mflags;
      m_state.release_checks = max_release_check_rate;
      m_state.magic = m_params.magic;
      disable_contiguous(&m_state);
      init_bins();
   }

   //Makes the memory given the heap's first segment, with top at its start.
   //The other half of init_user_mstate(), for the heaps that keep their
   //object outside the memory they manage.
   void attach_segment(char *tbase, size_type tsize, flag_t sflags)
   {
      mchunkptr const t = align_as_chunk(tbase);
      m_state.seg.base   = m_state.least_addr = tbase;
      m_state.seg.size   = m_state.footprint = m_state.max_footprint = tsize;
      m_state.seg.sflags = sflags;
      init_top(t, (size_type)((tbase + tsize) - bytes_at(t)) - top_foot_size());
      check_top_chunk(m_state.top);
   }

   //From init_user_mstate(). The object goes in a chunk of its own at the
   //front of the memory given, and everything after it becomes the first
   //segment. That chunk is marked in use, exactly as the original marks the
   //one holding its malloc_state, so no allocation can ever be handed out
   //over the top of the object - and, as in the original, it is in-use
   //memory as far as mallinfo() and allocated_memory() are concerned.
   static basic_dlmalloc *init_in_place(char *tbase, size_type tsize,
                                        flag_t sflags, bool locked)
   {
      const size_type msize = pad_request(sizeof(basic_dlmalloc));
      mchunkptr const msp = align_as_chunk(tbase);
      //The constructor does to the state what init_user_mstate does: zeroes
      //it, takes the lock, sets the magic, the flags and the bins.
      basic_dlmalloc *const m =
         ::new(chunk2mem(msp), boost_move_new_t()) basic_dlmalloc();
      msp->head = msize | inuse_bits;
      m->m_state.seg.base   = m->m_state.least_addr = tbase;
      m->m_state.seg.size   = m->m_state.footprint = m->m_state.max_footprint = tsize;
      m->m_state.seg.sflags = sflags;
      set_lock(&m->m_state, locked ? 1 : 0);
      mchunkptr const mn = next_chunk(mem2chunk(m));
      m->init_top(mn, (size_type)((tbase + tsize) - bytes_at(mn)) - top_foot_size());
      m->check_top_chunk(m->m_state.top);
      return m;
   }

   // ==== init_top and init_bins ====
   // Initialize top chunk and its size
   void init_top(mchunkptr p, size_type psize)
   {
      mstate m = &m_state;
      // Ensure alignment
      size_type offset = align_offset(chunk2mem(p));
      p = chunk_at(bytes_at(p) + offset);
      psize -= offset;

      m->top = p;
      m->topsize = psize;
      p->head = psize | pinuse_bit;
      // set size of fake trailing chunk holding overhead space only once
      chunk_plus_offset(p, psize)->head = top_foot_size();
      m->trim_check = m_params.trim_threshold; // reset on each update
   }

   // Initialize bins for a new mstate that is otherwise zeroed out
   void init_bins()
   {
      mstate m = &m_state;
      // Establish circular links for smallbins
      bindex_t i;
      for (i = 0; i < nsmallbins; ++i) {
         sbinptr bin = smallbin_at(m,i);
         bin->fd = bin->bk = bin;
      }
   }

   // ==== prepend_alloc and add_segment ====
   void* prepend_alloc(char* newbase, char* oldbase,
      size_type nb)
   {
      mstate m = &m_state;
      mchunkptr p = align_as_chunk(newbase);
      mchunkptr oldfirst = align_as_chunk(oldbase);
      size_type psize = (size_type)(bytes_at(oldfirst) - bytes_at(p));
      mchunkptr q = chunk_plus_offset(p, nb);
      size_type qsize = psize - nb;
      set_size_and_pinuse_of_inuse_chunk(m, p, nb);

      BOOST_CONTAINER_DL_ASSERT((char*)oldfirst > (char*)q);
      BOOST_CONTAINER_DL_ASSERT(pinuse(oldfirst));
      BOOST_CONTAINER_DL_ASSERT(qsize >= min_chunk_size);

      // consolidate remainder with first chunk of old base
      if (oldfirst == m->top) {
         size_type tsize = m->topsize += qsize;
         m->top = q;
         q->head = tsize | pinuse_bit;
         check_top_chunk(q);
      }
      else if (oldfirst == m->dv) {
         size_type dsize = m->dvsize += qsize;
         m->dv = q;
         set_size_and_pinuse_of_free_chunk(q, dsize);
      }
      else {
         if (!is_inuse(oldfirst)) {
            size_type nsize = chunksize(oldfirst);
            unlink_chunk(m, oldfirst, nsize);
            oldfirst = chunk_plus_offset(oldfirst, nsize);
            qsize += nsize;
         }
         set_free_with_pinuse(q, qsize, oldfirst);
         insert_chunk(m, q, qsize);
         check_free_chunk(q);
      }

      check_malloced_chunk(chunk2mem(p), nb);
      return chunk2mem(p);
   }

   // Add a segment to hold a new noncontiguous region
   void add_segment(char* tbase, size_type tsize, flag_t mmapped)
   {
      mstate m = &m_state;
      // Determine locations and sizes of segment, fenceposts, old top
      char* old_top = (char*)m->top;
      msegmentptr oldsp = segment_holding(old_top);
      char* old_end = oldsp->base + oldsp->size;
      size_type ssize = pad_request(sizeof(malloc_segment));
      char* rawsp = old_end - (ssize + four_size_t_sizes + chunk_align_mask);
      size_type offset = align_offset(chunk2mem(rawsp));
      char* asp = rawsp + offset;
      char* csp = (asp < (old_top + min_chunk_size))? old_top : asp;
      mchunkptr sp = chunk_at(csp);
      msegmentptr ss = (msegmentptr)(chunk2mem(sp));
      mchunkptr tnext = chunk_plus_offset(sp, ssize);
      mchunkptr p = tnext;
      int nfences = 0;

      // reset top to new space
      init_top(chunk_at(tbase), tsize - top_foot_size());

      // Set up segment record
      BOOST_CONTAINER_DL_ASSERT(is_aligned(ss));
      set_size_and_pinuse_of_inuse_chunk(m, sp, ssize);
      *ss = m->seg; // Push current record
      m->seg.base = tbase;
      m->seg.size = tsize;
      m->seg.sflags = mmapped;
      m->seg.next = ss;

      // Insert trailing fenceposts
      for (;;) {
         mchunkptr nextp = chunk_plus_offset(p, size_t_size);
         p->head = fencepost_head;
         ++nfences;
         if ((char*)(&(nextp->head)) < old_end)
            p = nextp;
         else
            break;
      }
      BOOST_CONTAINER_DL_ASSERT(nfences >= 2);
      (void) nfences; //Added by iG to silence warning about unused nfences

      // Insert the rest of old top into a bin as an ordinary free chunk
      if (csp != old_top) {
         mchunkptr q = chunk_at(old_top);
         size_type psize = (size_type)(csp - old_top);
         mchunkptr tn = chunk_plus_offset(q, psize);
         set_free_with_pinuse(q, psize, tn);
         insert_chunk(m, q, psize);
      }

      check_top_chunk(m->top);
   }


   // ==== sys_alloc ====
   void* sys_alloc(size_type nb)
   {
      mstate m = &m_state;
      char* tbase = cmfail();
      size_type tsize = 0;
      flag_t mmap_flag = 0;
      size_type asize; // allocation size

      // Directly map large chunks, but only if already initialized
      if (use_mmap(m) && nb >= m_params.mmap_threshold && m->topsize != 0) {
         void* mem = mmap_alloc(nb);
         if (BOOST_LIKELY(mem != 0))
            return mem;
      }

      asize = granularity_align(nb + sys_alloc_padding());
      if (asize <= nb)
         return 0; // wraparound
      if (m->footprint_limit != 0) {
         size_type fp = m->footprint + asize;
         if (fp <= m->footprint || fp > m->footprint_limit)
            return 0;
      }

      if (tbase == cmfail()) {  // Try MMAP
         char* mp = (char*)(call_mmap(asize));
         if (mp != cmfail()) {
            tbase = mp;
            tsize = asize;
            mmap_flag = use_mmap_bit;
         }
      }


      if (tbase != cmfail()) {

         if ((m->footprint += tsize) > m->max_footprint)
            m->max_footprint = m->footprint;

         if (!is_initialized(m)) { // first-time initialization
            if (m->least_addr == 0 || tbase < m->least_addr)
               m->least_addr = tbase;
            m->seg.base = tbase;
            m->seg.size = tsize;
            m->seg.sflags = mmap_flag;
            m->magic = m_params.magic;
            m->release_checks = max_release_check_rate;
            init_bins();
            //The state is a data member, not embedded in a segment
            init_top(chunk_at(tbase), tsize - top_foot_size());
         }

         else {
            // Try to merge with an existing segment
            msegmentptr sp = &m->seg;
            // Only consider most recent segment if traversal suppressed
            while (sp != 0 && tbase != sp->base + sp->size)
               sp = (no_segment_traversal) ? 0 : sp->next;
            if (sp != 0 &&
               !is_extern_segment(sp) &&
               (sp->sflags & use_mmap_bit) == mmap_flag &&
               segment_holds(sp, m->top)) { // append
               sp->size += tsize;
               init_top(m->top, m->topsize + tsize);
            }
            else {
               if (tbase < m->least_addr)
                  m->least_addr = tbase;
               sp = &m->seg;
               while (sp != 0 && sp->base != tbase + tsize)
                  sp = (no_segment_traversal) ? 0 : sp->next;
               if (sp != 0 &&
                  !is_extern_segment(sp) &&
                  (sp->sflags & use_mmap_bit) == mmap_flag) {
                  char* oldbase = sp->base;
                  sp->base = tbase;
                  sp->size += tsize;
                  return prepend_alloc(tbase, oldbase, nb);
               }
               else
                  add_segment(tbase, tsize, mmap_flag);
            }
         }

         if (nb < m->topsize) { // Allocate from new or extended top space
            size_type rsize = m->topsize -= nb;
            mchunkptr p = m->top;
            mchunkptr r = m->top = chunk_plus_offset(p, nb);
            r->head = rsize | pinuse_bit;
            set_size_and_pinuse_of_inuse_chunk(m, p, nb);
            check_top_chunk(m->top);
            check_malloced_chunk(chunk2mem(p), nb);
            return chunk2mem(p);
         }
      }

      malloc_failure();
      return 0;
   }


   // ==== release_unused_segments and sys_trim ====
   size_type release_unused_segments()
   {
      mstate m = &m_state;
      size_type released = 0;
      int nsegs = 0;
      msegmentptr pred = &m->seg;
      msegmentptr sp = pred->next;
      while (sp != 0) {
         char* base = sp->base;
         size_type size = sp->size;
         msegmentptr next = sp->next;
         ++nsegs;
         if (is_mmapped_segment(sp) && !is_extern_segment(sp)) {
            mchunkptr p = align_as_chunk(base);
            size_type psize = chunksize(p);
            // Can unmap if first chunk holds entire segment and not pinned
            if (!is_inuse(p) && (char*)p + psize >= base + size - top_foot_size()) {
               tchunkptr tp = (tchunkptr)p;
               BOOST_CONTAINER_DL_ASSERT(segment_holds(sp, (char*)sp));
               if (p == m->dv) {
                  m->dv = 0;
                  m->dvsize = 0;
               }
               else {
                  unlink_large_chunk(m, tp);
               }
               if (call_munmap(base, size) == 0) {
                  released += size;
                  m->footprint -= size;
                  // unlink obsoleted record
                  sp = pred;
                  sp->next = next;
               }
               else { // back out if cannot unmap
                  insert_large_chunk(m, tp, psize);
               }
            }
         }
         if (no_segment_traversal) // scan only first segment
            break;
         pred = sp;
         sp = next;
      }
      // Reset check counter
      m->release_checks = (((size_type) nsegs > (size_type) max_release_check_rate)?
         (size_type) nsegs : (size_type) max_release_check_rate);
      return released;
   }

   //Releases the whole of the last segment, which the shrink in sys_trim()
   //never can: it keeps one granularity unit of top, because the original's
   //malloc_state lives in its first segment and freeing that unit would
   //free the heap itself.
   //
   //A heap this class default-constructs has its state as a data member,
   //outside every segment, so there is nothing in the segment that has to
   //survive and all of it can go - leaving the heap exactly as the
   //constructor left it, holding nothing. A heap create() built does keep
   //its object in the first segment, and segment_holds() below is what
   //refuses that case, so it behaves as the original does.
   //
   //Only on an explicit trim(0). The automatic trim on deallocation must
   //keep its hysteresis, or a program that allocates and frees one block in
   //a loop would map and unmap a segment every time round.
   size_type release_last_segment()
   {
      mstate m = &m_state;
      msegmentptr sp = &m->seg;
      //One segment only, and it must be the one holding top
      if(sp->next != 0 || m->top == 0 || segment_holding((char *)m->top) != sp)
         return 0;
      //Ours to release, and not the caller's buffer
      if(is_extern_segment(sp) || !is_mmapped_segment(sp))
         return 0;
      //Not if this object lives in it
      if(segment_holds(sp, this))
         return 0;
      //Nothing may be left in the heap: top has to be the only chunk, so
      //the segment begins with it and no bin nor the designated victim
      //holds anything
      if(align_as_chunk(sp->base) != m->top ||
         m->smallmap != 0 || m->treemap != 0 || m->dvsize != 0)
         return 0;

      char *const base = sp->base;
      const size_type size = sp->size;
      if(call_munmap(base, size) != 0)
         return 0;

      m->footprint -= size;
      m->seg.base   = 0;
      m->seg.size   = 0;
      m->seg.sflags = 0;
      m->top        = 0;
      m->topsize    = 0;
      m->trim_check = m_params.trim_threshold;
      return size;
   }

   int sys_trim(size_type pad, bool release_all)
   {
      mstate m = &m_state;
      size_type released = 0;

      if (pad < max_request && is_initialized(m)) {
         pad += top_foot_size(); // ensure enough room for segment overhead

         if (m->topsize > pad) {
            // Shrink top space in granularity-size units, keeping at least one
            size_type unit = m_params.granularity;
            size_type extra = ((m->topsize - pad + (unit - size_t_one)) / unit -
               size_t_one) * unit;
            msegmentptr sp = segment_holding((char*)m->top);

            if (!is_extern_segment(sp)) {
               if (is_mmapped_segment(sp)) {
                  if (sp->size >= extra &&
                     !has_segment_link(sp)) { // can't shrink if pinned
                     size_type newsize = sp->size - extra;
                     (void)newsize; // placate people compiling -Wunused-variable
                     // Prefer mremap, fall back to munmap
                     if ((call_mremap(sp->base, sp->size, newsize, 0) != mfail()) ||
                        (call_munmap(sp->base + newsize, extra) == 0)) {
                        released = extra;
                     }
                  }
               }
            }

            if (released != 0) {
               sp->size -= released;
               m->footprint -= released;
               init_top(m->top, m->topsize - released);
               check_top_chunk(m->top);
            }
         }

         // Unmap any unused mmapped segments
         released += release_unused_segments();

         //...and with nothing asked to be kept, the last one as well. It
         //goes after the loop above, which is what leaves a single segment
         //behind for this to find.
         if (release_all && pad == top_foot_size())
            released += release_last_segment();

         // On failure, disable autotrim to avoid repeated failed future calls
         if (released == 0 && m->topsize > m->trim_check)
            m->trim_check = max_size_t;
      }

      return (released != 0)? 1 : 0;
   }

   // Consolidate and bin a chunk. Differs from exported versions
   //of free mainly in that the chunk need not be marked as inuse.
   //

   // ==== dispose_chunk ====
   void dispose_chunk(mchunkptr p, size_type psize)
   {
      mstate m = &m_state;
      mchunkptr next = chunk_plus_offset(p, psize);
      if (!pinuse(p)) {
         mchunkptr prev;
         size_type prevsize = p->prev_foot;
         if (is_mmapped(p)) {
            psize += prevsize + mmap_foot_pad;
            if (call_munmap((char*)p - prevsize, psize) == 0)
               m->footprint -= psize;
            return;
         }
         prev = chunk_minus_offset(p, prevsize);
         psize += prevsize;
         p = prev;
         if (rtcheck(ok_address(m, prev))) { // consolidate backward
            if (p != m->dv) {
               unlink_chunk(m, p, prevsize);
            }
            else if ((next->head & inuse_bits) == inuse_bits) {
               m->dvsize = psize;
               set_free_with_pinuse(p, psize, next);
               return;
            }
         }
         else {
            corruption_error_action(m);
            return;
         }
      }
      if (rtcheck(ok_address(m, next))) {
         if (!cinuse(next)) {  // consolidate forward
            if (next == m->top) {
               size_type tsize = m->topsize += psize;
               m->top = p;
               p->head = tsize | pinuse_bit;
               if (p == m->dv) {
                  m->dv = 0;
                  m->dvsize = 0;
               }
               return;
            }
            else if (next == m->dv) {
               size_type dsize = m->dvsize += psize;
               m->dv = p;
               set_size_and_pinuse_of_free_chunk(p, dsize);
               return;
            }
            else {
               size_type nsize = chunksize(next);
               psize += nsize;
               unlink_chunk(m, next, nsize);
               set_size_and_pinuse_of_free_chunk(p, psize);
               if (p == m->dv) {
                  m->dvsize = psize;
                  return;
               }
            }
         }
         else {
            set_free_with_pinuse(p, psize, next);
         }
         insert_chunk(m, p, psize);
      }
      else {
         corruption_error_action(m);
      }
   }


   // ==== tmalloc_large and tmalloc_small ====

   // allocate a large request from the best fitting chunk in a treebin
   void* tmalloc_large(size_type nb)
   {
      mstate m = &m_state;
      tchunkptr v = 0;
      size_type rsize = 0 - nb; // Unsigned negation
      tchunkptr t;
      bindex_t idx;
      compute_tree_index(nb, idx);
      if ((t = *treebin_at(m, idx)) != 0) {
         // Traverse tree for this bin looking for node with size == nb
         size_type sizebits = nb << leftshift_for_tree_index(idx);
         tchunkptr rst = 0;  // The deepest untaken right subtree
         for (;;) {
            tchunkptr rt;
            size_type trem = chunksize(t) - nb;
            if (trem < rsize) {
               v = t;
               if ((rsize = trem) == 0)
                  break;
            }
            rt = t->child[1];
            t = t->child[(sizebits >> (size_t_bitsize-size_t_one)) & 1];
            if (rt != 0 && rt != t)
               rst = rt;
            if (t == 0) {
               t = rst; // set t to least subtree holding sizes > nb
               break;
            }
            sizebits <<= 1;
         }
      }
      if (t == 0 && v == 0) { // set t to root of next non-empty treebin
         binmap_t leftbits = left_bits(idx2bit(idx)) & m->treemap;
         if (leftbits != 0) {
            bindex_t i;
            binmap_t leastbit = least_bit(leftbits);
            compute_bit2idx(leastbit, i);
            t = *treebin_at(m, i);
         }
      }

      while (t != 0) { // find smallest of tree or subtree
         size_type trem = chunksize(t) - nb;
         if (trem < rsize) {
            rsize = trem;
            v = t;
         }
         t = leftmost_child(t);
      }

      //  If dv is a better fit, return 0 so malloc will use it
      if (v != 0 && rsize < (size_type)(m->dvsize - nb)) {
         if (rtcheck(ok_address(m, v))) { // split
            mchunkptr r = chunk_plus_offset(v, nb);
            BOOST_CONTAINER_DL_ASSERT(chunksize(v) == rsize + nb);
            if (rtcheck(ok_next(v, r))) {
               unlink_large_chunk(m, v);
               if (rsize < min_chunk_size)
                  set_inuse_and_pinuse(m, v, (rsize + nb));
               else {
                  set_size_and_pinuse_of_inuse_chunk(m, v, nb);
                  set_size_and_pinuse_of_free_chunk(r, rsize);
                  insert_chunk(m, r, rsize);
               }
               return chunk2mem(v);
            }
         }
         corruption_error_action(m);
      }
      return 0;
   }

   // allocate a small request from the best fitting chunk in a treebin
   void* tmalloc_small(size_type nb)
   {
      mstate m = &m_state;
      tchunkptr t, v;
      size_type rsize;
      bindex_t i;
      binmap_t leastbit = least_bit(m->treemap);
      compute_bit2idx(leastbit, i);
      v = t = *treebin_at(m, i);
      rsize = chunksize(t) - nb;

      while ((t = leftmost_child(t)) != 0) {
         size_type trem = chunksize(t) - nb;
         if (trem < rsize) {
            rsize = trem;
            v = t;
         }
      }

      if (rtcheck(ok_address(m, v))) {
         mchunkptr r = chunk_plus_offset(v, nb);
         BOOST_CONTAINER_DL_ASSERT(chunksize(v) == rsize + nb);
         if (rtcheck(ok_next(v, r))) {
            unlink_large_chunk(m, v);
            if (rsize < min_chunk_size)
               set_inuse_and_pinuse(m, v, (rsize + nb));
            else {
               set_size_and_pinuse_of_inuse_chunk(m, v, nb);
               set_size_and_pinuse_of_free_chunk(r, rsize);
               replace_dv(m, r, rsize);
            }
            return chunk2mem(v);
         }
      }

      corruption_error_action(m);
      return 0;
   }

   // ==== try_realloc_chunk, internal_memalign, ialloc, internal_bulk_free ====
   mchunkptr try_realloc_chunk(mchunkptr p, size_type nb,
      int can_move)
   {
      mstate m = &m_state;
      mchunkptr newp = 0;
      size_type oldsize = chunksize(p);
      mchunkptr next = chunk_plus_offset(p, oldsize);
      if (rtcheck(ok_address(m, p) && ok_inuse(p) &&
         ok_next(p, next) && ok_pinuse(next))) {
         if (is_mmapped(p)) {
            newp = mmap_resize(p, nb, can_move);
         }
         else if (oldsize >= nb) {             // already big enough
            size_type rsize = oldsize - nb;
            if (rsize >= min_chunk_size) {      // split off remainder
               mchunkptr r = chunk_plus_offset(p, nb);
               set_inuse(m, p, nb);
               set_inuse(m, r, rsize);
               dispose_chunk(r, rsize);
            }
            newp = p;
         }
         else if (next == m->top) {  // extend into top
            if (oldsize + m->topsize > nb) {
               size_type newsize = oldsize + m->topsize;
               size_type newtopsize = newsize - nb;
               mchunkptr newtop = chunk_plus_offset(p, nb);
               set_inuse(m, p, nb);
               newtop->head = newtopsize |pinuse_bit;
               m->top = newtop;
               m->topsize = newtopsize;
               newp = p;
            }
         }
         else if (next == m->dv) { // extend into dv
            size_type dvs = m->dvsize;
            if (oldsize + dvs >= nb) {
               size_type dsize = oldsize + dvs - nb;
               if (dsize >= min_chunk_size) {
                  mchunkptr r = chunk_plus_offset(p, nb);
                  mchunkptr n = chunk_plus_offset(r, dsize);
                  set_inuse(m, p, nb);
                  set_size_and_pinuse_of_free_chunk(r, dsize);
                  clear_pinuse(n);
                  m->dvsize = dsize;
                  m->dv = r;
               }
               else { // exhaust dv
                  size_type newsize = oldsize + dvs;
                  set_inuse(m, p, newsize);
                  m->dvsize = 0;
                  m->dv = 0;
               }
               newp = p;
            }
         }
         else if (!cinuse(next)) { // extend into next free chunk
            size_type nextsize = chunksize(next);
            if (oldsize + nextsize >= nb) {
               size_type rsize = oldsize + nextsize - nb;
               unlink_chunk(m, next, nextsize);
               if (rsize < min_chunk_size) {
                  size_type newsize = oldsize + nextsize;
                  set_inuse(m, p, newsize);
               }
               else {
                  mchunkptr r = chunk_plus_offset(p, nb);
                  set_inuse(m, p, nb);
                  set_inuse(m, r, rsize);
                  dispose_chunk(r, rsize);
               }
               newp = p;
            }
         }
      }
      else {
         usage_error_action(m, chunk2mem(p));
      }
      return newp;
   }

   //Takes the lock, then runs priv_allocate_aligned_nolock(), which holds the
   //whole implementation.
   //
   //One acquisition covers both the oversized allocation and the realign that
   //follows it. This function once took the lock for each of the two, and
   //needed a repair path to give the block back when the second acquisition
   //failed - which is the leak that path repaired.
   void* internal_memalign(size_type alignment, size_type bytes)
   {
      mstate m = &m_state;
      if (!ok_magic(m)) {
         usage_error_action(m, m);
         return 0;
      }
      if (preaction(m))
         return 0;
      void *const mem = priv_allocate_aligned_nolock(alignment, bytes);
      postaction(m);
      return mem;
   }

   //
   //Common support for independent_X routines, handling
   //  all of the combinations that can result.
   //The opts arg has:
   //  bit 0 set if all elements are same size (using sizes[0])
   //  bit 1 set if elements should be zeroed
   //
   void** ialloc(size_type n_elements,
      size_type* sizes,
      int opts,
      void* chunks[])
   {
      mstate m = &m_state;

      size_type    element_size;   // chunksize of each element, if all same
      size_type    contents_size;  // total size of elements
      size_type    array_size;     // request size of pointer array
      void*     mem;            // malloced aggregate space
      mchunkptr p;              // corresponding chunk
      size_type    remainder_size; // remaining bytes while splitting
      void**    marray;         // either "chunks" or malloced ptr array
      mchunkptr array_chunk;    // chunk for malloced ptr array
      flag_t    was_enabled;    // to disable mmap
      size_type    size;
      size_type    i;

      // compute array length, if needed
      if (chunks != 0) {
         if (n_elements == 0)
            return chunks; // nothing to do
         marray = chunks;
         array_size = 0;
      }
      else {
         // if empty req, must still return chunk representing empty array
         if (n_elements == 0)
            return (void**)priv_allocate(0);
         marray = 0;
         //Every product and sum in this function is checked. Leaving them
         //unchecked reads as "a caller asking for more than the address
         //space holds will be refused by the allocation further down", and
         //that is not what happens: the arithmetic wraps first, so a SMALL
         //block is asked for and granted, and the loop that splits it then
         //carves n_elements pieces of the full size out of it and walks off
         //the end.
         //
         //A request that cannot be represented is a request that cannot be
         //met, and it leaves the same way as any other: null, with the
         //failure action run. Nothing is locked or half-changed yet - the
         //lock and the mmap flag are both further down - so there is nothing
         //to undo.
         if (n_elements > max_size_t / sizeof(void*)) {
            malloc_failure();
            return 0;
         }
         array_size = request2size(n_elements * (sizeof(void*)));
      }

      // compute total element size
      if (opts & 0x1) { // all-same-size
         element_size = request2size(*sizes);
         if (element_size != 0 && n_elements > max_size_t / element_size) {
            malloc_failure();
            return 0;
         }
         contents_size = n_elements * element_size;
      }
      else { // add up all the sizes
         element_size = 0;
         contents_size = 0;
         for (i = 0; i != n_elements; ++i) {
            const size_type each = request2size(sizes[i]);
            if (contents_size > max_size_t - each) {
               malloc_failure();
               return 0;
            }
            contents_size += each;
         }
      }

      if (contents_size > max_size_t - array_size) {
         malloc_failure();
         return 0;
      }
      size = contents_size + array_size;

      //
      //Allocate the aggregate chunk.  First disable direct-mmapping so
      //malloc won't use it, since we would not be able to later
      //free/realloc space internal to a segregated mmap region.
      //
      was_enabled = use_mmap(m);
      disable_mmap(m);
      mem = priv_allocate(size - chunk_overhead);
      if (was_enabled)
         enable_mmap(m);
      if (mem == 0)
         return 0;

      if (preaction(m)) {
         //As in internal_memalign: the block internal_malloc() just returned
         //has not been touched yet (the mmap flag is already restored
         //above), so give it back rather than leak it.
         priv_deallocate(mem);
         return 0;
      }
      p = mem2chunk(mem);
      remainder_size = chunksize(p);

      BOOST_CONTAINER_DL_ASSERT(!is_mmapped(p));

      if (opts & 0x2) {       // optionally clear the elements
         memset((size_type*)mem, 0, remainder_size - size_t_size - array_size);
      }

      // If not provided, allocate the pointer array as final part of chunk
      if (marray == 0) {
         size_type  array_chunk_size;
         array_chunk = chunk_plus_offset(p, contents_size);
         array_chunk_size = remainder_size - contents_size;
         marray = (void**) (chunk2mem(array_chunk));
         set_size_and_pinuse_of_inuse_chunk(m, array_chunk, array_chunk_size);
         remainder_size = contents_size;
      }

      // split out elements
      for (i = 0; ; ++i) {
         marray[i] = chunk2mem(p);
         if (i != n_elements-1) {
            if (element_size != 0)
               size = element_size;
            else
               size = request2size(sizes[i]);
            remainder_size -= size;
            set_size_and_pinuse_of_inuse_chunk(m, p, size);
            p = chunk_plus_offset(p, size);
         }
         else { // the final element absorbs any overallocation slop
            set_size_and_pinuse_of_inuse_chunk(m, p, remainder_size);
            break;
         }
      }

      //The walk itself is worth skipping, not just the checks it makes: with
      //debug off every call in here is an empty function, but the loop
      //around them would still be a loop.
      if (debug) {
         if (marray != chunks) {
            // final element must have exactly exhausted chunk
            if (element_size != 0) {
               BOOST_CONTAINER_DL_ASSERT(remainder_size == element_size);
            }
            else {
               BOOST_CONTAINER_DL_ASSERT(remainder_size == request2size(sizes[i]));
            }
            check_inuse_chunk(mem2chunk(marray));
         }
         for (i = 0; i != n_elements; ++i)
            check_inuse_chunk(mem2chunk(marray[i]));
      }

      postaction(m);
      return marray;
   }

   // Try to free all pointers in the given array.
   //Note: this could be made faster, by delaying consolidation,
   //at the price of disabling some user integrity checks, We
   //still optimize some consolidations by combining adjacent
   //chunks before freeing, which will occur often if allocated
   //with ialloc or the array is sorted.
   //
   size_type internal_bulk_free(void* array[], size_type nelem)
   {
      mstate m = &m_state;
      size_type unfreed = 0;
      if (!preaction(m)) {
         void** a;
         void** fence = &(array[nelem]);
         for (a = array; a != fence; ++a) {
            void* mem = *a;
            if (BOOST_LIKELY(mem != 0)) {
               mchunkptr p = mem2chunk(mem);
               size_type psize = chunksize(p);
               if (footers && get_mstate_for(p) != m) {
                  ++unfreed;
                  continue;
               }
               check_inuse_chunk(p);
               *a = 0;
               if (rtcheck(ok_address(m, p) && ok_inuse(p))) {
                  void ** b = a + 1; // try to merge with next chunk
                  mchunkptr next = next_chunk(p);
                  if (b != fence && *b == chunk2mem(next)) {
                     size_type newsize = chunksize(next) + psize;
                     set_inuse(m, p, newsize);
                     *b = chunk2mem(p);
                  }
                  else
                     dispose_chunk(p, psize);
               }
               else {
                  corruption_error_action(m);
                  break;
               }
            }
         }
         if (should_trim(m, m->topsize))
            sys_trim(0, false);
         postaction(m);
      }
      return unfreed;
   }

   // ==== change_mparam, per instance here ====
   int change_mparam(option_t param_number, size_type value)
   {
      //max_size_t is "no limit". A caller writing -1 for that, the way the
      //parameter codes are written, arrives here as max_size_t already.
      const size_type val = value;
      switch(param_number) {
      case option_trim_threshold:
         m_params.trim_threshold = val;
         return 1;
      case option_granularity:
         if (val >= m_params.page_size && ((val & (val-1)) == 0)) {
            m_params.granularity = val;
            return 1;
         }
         else
            return 0;
      case option_mmap_threshold:
         m_params.mmap_threshold = val;
         return 1;
      default:
         return 0;
      }
   }

   // ==== priv_allocate ====
   //Takes the lock, then runs priv_allocate_nolock(), which holds the whole
   //implementation. The magic is read before the lock: a heap that fails that
   //test must not be locked.
   void* priv_allocate(size_type bytes)
   {
      mstate ms = &m_state;
      if (!ok_magic(ms)) {
         usage_error_action(ms, ms);
         return 0;
      }
      if (preaction(ms))
         return 0;
      void *const mem = priv_allocate_nolock(bytes);
      postaction(ms);
      return mem;
   }


   // ==== priv_deallocate ====
   //Takes the lock, then runs priv_deallocate_nolock(), which holds the whole
   //implementation.
   //
   //The lock is the one of the heap the chunk came from, which with footers
   //need not be this heap - so it is read from the chunk, as the original
   //does, and not from m_state.
   void priv_deallocate(void* mem)
   {
      if (BOOST_LIKELY(mem != 0)) {
         mchunkptr p = mem2chunk(mem);
         mstate fm = get_mstate_for(p);
         if (!ok_magic(fm)) {
            usage_error_action(fm, p);
            return;
         }
         if (!preaction(fm)) {
            priv_deallocate_nolock(mem);
            postaction(fm);
         }
      }
   }


   // ==== priv_allocate_zeroed ====
   void* priv_allocate_zeroed(size_type n_elements, size_type elem_size)
   {
      void* mem;
      size_type req = 0;
      mstate ms = &m_state;
      if (!ok_magic(ms)) {
         usage_error_action(ms,ms);
         return 0;
      }
      if (n_elements != 0) {
         req = n_elements * elem_size;
         if (((n_elements | elem_size) & ~(size_type)0xffff) &&
            (req / n_elements != elem_size))
            req = max_size_t; // force downstream failure on overflow
      }
      mem = priv_allocate(req);
      if (BOOST_LIKELY(mem != 0) && calloc_must_clear(mem2chunk(mem)))
         memset(mem, 0, req);
      return mem;
   }


   // ==== priv_reallocate ====
   void* priv_reallocate(void* oldmem, size_type bytes)
   {
      void* mem = 0;
      if (oldmem == 0) {
         mem = priv_allocate(bytes);
      }
      else if (bytes >= max_request) {
         malloc_failure();
      }
      else if (bytes == 0) {
         priv_deallocate(oldmem);
      }
      else {
         size_type nb = request2size(bytes);
         mchunkptr oldp = mem2chunk(oldmem);
         mstate m = get_mstate_for(oldp);
         if (!ok_magic(m)) {
            usage_error_action(m, oldmem);
            return 0;
         }
         if (!preaction(m)) {
            mchunkptr newp = try_realloc_chunk(oldp, nb, 1);
            postaction(m);
            if (newp != 0) {
               check_inuse_chunk(newp);
               mem = chunk2mem(newp);
            }
            else {
               mem = priv_allocate(bytes);
               if (BOOST_LIKELY(mem != 0)) {
                  size_type oc = chunksize(oldp) - overhead_for(oldp);
                  memcpy(mem, oldmem, (oc < bytes)? oc : bytes);
                  priv_deallocate(oldmem);
               }
            }
         }
      }
      return mem;
   }


   // ==== priv_reallocate_in_place ====
   void* priv_reallocate_in_place(void* oldmem, size_type bytes)
   {
      void* mem = 0;
      if (BOOST_LIKELY(oldmem != 0)) {
         if (bytes >= max_request) {
            malloc_failure();
         }
         else {
            size_type nb = request2size(bytes);
            mchunkptr oldp = mem2chunk(oldmem);
            mstate m = get_mstate_for(oldp);
            if (!ok_magic(m)) {
               usage_error_action(m, oldmem);
               return 0;
            }
            if (!preaction(m)) {
               mchunkptr newp = try_realloc_chunk(oldp, nb, 0);
               postaction(m);
               if (newp == oldp) {
                  check_inuse_chunk(newp);
                  mem = oldmem;
               }
            }
         }
      }
      return mem;
   }


   // ==== priv_allocate_aligned ====
   void* priv_allocate_aligned(size_type alignment, size_type bytes)
   {
      mstate ms = &m_state;
      if (!ok_magic(ms)) {
         usage_error_action(ms,ms);
         return 0;
      }
      if (alignment <= malloc_alignment)
         return priv_allocate(bytes);
      return internal_memalign(alignment, bytes);
   }


   // ==== priv_independent_calloc ====
   void** priv_independent_calloc(size_type n_elements,
      size_type elem_size, void* chunks[])
   {
      size_type sz = elem_size; // serves as 1-element array
      mstate ms = &m_state;
      if (!ok_magic(ms)) {
         usage_error_action(ms,ms);
         return 0;
      }
      return ialloc(n_elements, &sz, 3, chunks);
   }


   // ==== priv_independent_comalloc ====
   void** priv_independent_comalloc(size_type n_elements,
      size_type sizes[], void* chunks[])
   {
      mstate ms = &m_state;
      if (!ok_magic(ms)) {
         usage_error_action(ms,ms);
         return 0;
      }
      return ialloc(n_elements, sizes, 0, chunks);
   }


   //The whole of deallocation, for a caller that already holds the lock.
   //priv_deallocate() is this function with the lock taken around it, and the
   //dead "if (!0)" below is where its preaction() sits - kept in place, and
   //with it the indentation and the postaction label, so that the two stay
   //easy to read against the original mspace_free().
   void priv_deallocate_nolock(void* mem)
   {
      if (BOOST_LIKELY(mem != 0)) {
         mchunkptr p  = mem2chunk(mem);
         mstate fm = get_mstate_for(p);
         if (!ok_magic(fm)) {
            usage_error_action(fm, p);
            return;
         }
         if (!0){//preaction(fm)) {
            check_inuse_chunk(p);
            if (rtcheck(ok_address(fm, p) && ok_inuse(p))) {
               size_type psize = chunksize(p);
               mchunkptr next = chunk_plus_offset(p, psize);
               if (!pinuse(p)) {
                  size_type prevsize = p->prev_foot;
                  if (is_mmapped(p)) {
                     psize += prevsize + mmap_foot_pad;
                     if (call_munmap((char*)p - prevsize, psize) == 0)
                        fm->footprint -= psize;
                     goto postaction;
                  }
                  else {
                     mchunkptr prev = chunk_minus_offset(p, prevsize);
                     psize += prevsize;
                     p = prev;
                     if (rtcheck(ok_address(fm, prev))) { // consolidate backward
                        if (p != fm->dv) {
                           unlink_chunk(fm, p, prevsize);
                        }
                        else if ((next->head & inuse_bits) == inuse_bits) {
                           fm->dvsize = psize;
                           set_free_with_pinuse(p, psize, next);
                           goto postaction;
                        }
                     }
                     else
                        goto erroraction;
                  }
               }

               if (rtcheck(ok_next(p, next) && ok_pinuse(next))) {
                  if (!cinuse(next)) {  // consolidate forward
                     if (next == fm->top) {
                        size_type tsize = fm->topsize += psize;
                        fm->top = p;
                        p->head = tsize | pinuse_bit;
                        if (p == fm->dv) {
                           fm->dv = 0;
                           fm->dvsize = 0;
                        }
                        if (should_trim(fm, tsize))
                           sys_trim(0, false);
                        goto postaction;
                     }
                     else if (next == fm->dv) {
                        size_type dsize = fm->dvsize += psize;
                        fm->dv = p;
                        set_size_and_pinuse_of_free_chunk(p, dsize);
                        goto postaction;
                     }
                     else {
                        size_type nsize = chunksize(next);
                        psize += nsize;
                        unlink_chunk(fm, next, nsize);
                        set_size_and_pinuse_of_free_chunk(p, psize);
                        if (p == fm->dv) {
                           fm->dvsize = psize;
                           goto postaction;
                        }
                     }
                  }
                  else
                     set_free_with_pinuse(p, psize, next);

                  if (is_small(psize)) {
                     insert_small_chunk(fm, p, psize);
                     check_free_chunk(p);
                  }
                  else {
                     tchunkptr tp = (tchunkptr)p;
                     insert_large_chunk(fm, tp, psize);
                     check_free_chunk(p);
                     if (--fm->release_checks == 0)
                        release_unused_segments();
                  }
                  goto postaction;
               }
            }
            erroraction:
            usage_error_action(fm, p);
            postaction:
            ;//postaction(fm);
         }
      }
   }

   //The whole of allocation, for a caller that already holds the lock.
   //priv_allocate() is this function with the lock taken around it, and the
   //dead "if (!0)" below is where its preaction() sits - kept in place, and
   //with it the indentation and the postaction label, so that the two stay
   //easy to read against the original mspace_malloc().
   void* priv_allocate_nolock(size_type bytes)
   {
      mstate ms = &m_state;
      if (!ok_magic(ms)) {
         usage_error_action(ms,ms);
         return 0;
      }
      if (!0){//preaction(ms)) {
         void* mem;
         size_type nb;
         if (bytes <= max_small_request) {
            bindex_t idx;
            binmap_t smallbits;
            nb = (bytes < min_request)? min_chunk_size : pad_request(bytes);
            idx = small_index(nb);
            smallbits = ms->smallmap >> idx;

            if ((smallbits & 0x3U) != 0) { // Remainderless fit to a smallbin.
               mchunkptr b, p;
               idx += ~smallbits & 1;       // Uses next bin if idx empty
               b = smallbin_at(ms, idx);
               p = b->fd;
               BOOST_CONTAINER_DL_ASSERT(chunksize(p) == small_index2size(idx));
               unlink_first_small_chunk(ms, b, p, idx);
               set_inuse_and_pinuse(ms, p, small_index2size(idx));
               mem = chunk2mem(p);
               check_malloced_chunk(mem, nb);
               goto postaction;
            }

            else if (nb > ms->dvsize) {
               if (smallbits != 0) { // Use chunk in next nonempty smallbin
                  mchunkptr b, p, r;
                  size_type rsize;
                  bindex_t i;
                  binmap_t leftbits = (smallbits << idx) & left_bits(idx2bit(idx));
                  binmap_t leastbit = least_bit(leftbits);
                  compute_bit2idx(leastbit, i);
                  b = smallbin_at(ms, i);
                  p = b->fd;
                  BOOST_CONTAINER_DL_ASSERT(chunksize(p) == small_index2size(i));
                  unlink_first_small_chunk(ms, b, p, i);
                  rsize = small_index2size(i) - nb;
                  // Fit here cannot be remainderless if 4byte sizes
                  if (size_t_size != 4 && rsize < min_chunk_size)
                     set_inuse_and_pinuse(ms, p, small_index2size(i));
                  else {
                     set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
                     r = chunk_plus_offset(p, nb);
                     set_size_and_pinuse_of_free_chunk(r, rsize);
                     replace_dv(ms, r, rsize);
                  }
                  mem = chunk2mem(p);
                  check_malloced_chunk(mem, nb);
                  goto postaction;
               }

               else if (ms->treemap != 0 && (mem = tmalloc_small(nb)) != 0) {
                  check_malloced_chunk(mem, nb);
                  goto postaction;
               }
            }
         }
         else if (bytes >= max_request)
            nb = max_size_t; // Too big to allocate. Force failure (in sys alloc)
         else {
            nb = pad_request(bytes);
            if (ms->treemap != 0 && (mem = tmalloc_large(nb)) != 0) {
               check_malloced_chunk(mem, nb);
               goto postaction;
            }
         }

         if (nb <= ms->dvsize) {
            size_type rsize = ms->dvsize - nb;
            mchunkptr p = ms->dv;
            if (rsize >= min_chunk_size) { // split dv
               mchunkptr r = ms->dv = chunk_plus_offset(p, nb);
               ms->dvsize = rsize;
               set_size_and_pinuse_of_free_chunk(r, rsize);
               set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
            }
            else { // exhaust dv
               size_type dvs = ms->dvsize;
               ms->dvsize = 0;
               ms->dv = 0;
               set_inuse_and_pinuse(ms, p, dvs);
            }
            mem = chunk2mem(p);
            check_malloced_chunk(mem, nb);
            goto postaction;
         }

         else if (nb < ms->topsize) { // Split top
            size_type rsize = ms->topsize -= nb;
            mchunkptr p = ms->top;
            mchunkptr r = ms->top = chunk_plus_offset(p, nb);
            r->head = rsize | pinuse_bit;
            set_size_and_pinuse_of_inuse_chunk(ms, p, nb);
            mem = chunk2mem(p);
            check_top_chunk(ms->top);
            check_malloced_chunk(mem, nb);
            goto postaction;
         }

         mem = sys_alloc(nb);

         postaction:
         ;//postaction(ms);
         return mem;
      }

      return 0;
   }

   //The whole of over-aligned allocation, for a caller that already holds the
   //lock. internal_memalign() is this function with the lock taken around it.
   //
   //The oversized block comes from priv_allocate_nolock(), so one critical
   //section covers the allocation and the realign that trims it. That is what
   //makes the operation atomic to another thread, and what leaves nothing for
   //a leak-repair path to repair.
   void* priv_allocate_aligned_nolock(size_type alignment, size_type bytes)
   {
      mstate m = &m_state;
      void* mem = 0;
      if (alignment <  min_chunk_size) // must be at least a minimum chunk size
         alignment = min_chunk_size;
      if ((alignment & (alignment-size_t_one)) != 0) {// Ensure a power of 2
         size_type a = malloc_alignment << 1;
         while (a < alignment) a <<= 1;
         alignment = a;
      }
      if (bytes >= max_request - alignment) {
         if (m != 0)  { // Test isn't needed but avoids compiler warning
            malloc_failure();
         }
      }
      else {
         size_type nb = request2size(bytes);
         size_type req = nb + alignment + min_chunk_size - chunk_overhead;
         mem = priv_allocate_nolock(req);
         if (BOOST_LIKELY(mem != 0)) {
            mchunkptr p = mem2chunk(mem);
            if ((((size_type)(mem)) & (alignment - 1)) != 0) { // misaligned
               //
               //Find an aligned spot inside chunk.  Since we need to give
               //back leading space in a chunk of at least min_chunk_size, if
               //the first calculation places us at a spot with less than
               //min_chunk_size leader, we can move to the next aligned spot.
               //We've allocated enough total room so that this is always
               //possible.
               //
               char* br = (char*)mem2chunk((size_type)(((size_type)((char*)mem + alignment -
                  size_t_one)) &
                  (0 - alignment)));
               char* pos = ((size_type)(br - (char*)(p)) >= min_chunk_size)?
                  br : br+alignment;
               mchunkptr newp = chunk_at(pos);
               size_type leadsize = (size_type)(pos - bytes_at(p));
               size_type newsize = chunksize(p) - leadsize;

               if (is_mmapped(p)) { // For mmapped chunks, just adjust offset
                  newp->prev_foot = p->prev_foot + leadsize;
                  newp->head = newsize;
               }
               else { // Otherwise, give back leader, use the rest
                  set_inuse(m, newp, newsize);
                  set_inuse(m, p, leadsize);
                  dispose_chunk(p, leadsize);
               }
               p = newp;
            }

            // Give back spare room at the end
            if (!is_mmapped(p)) {
               size_type size = chunksize(p);
               if (size > nb + min_chunk_size) {
                  size_type remainder_size = size - nb;
                  mchunkptr remainder = chunk_plus_offset(p, nb);
                  set_inuse(m, p, nb);
                  set_inuse(m, remainder, remainder_size);
                  dispose_chunk(remainder, remainder_size);
               }
            }

            mem = chunk2mem(p);
            BOOST_CONTAINER_DL_ASSERT(chunksize(p) >= nb);
            BOOST_CONTAINER_DL_ASSERT(((size_type)mem & (alignment - 1)) == 0);
            check_inuse_chunk(p);
            //No postaction: the lock is the caller's to release.
         }
      }
      return mem;
   }

   //This function is equal to try_realloc_chunk but handling
   //minimum and desired bytes
   mchunkptr try_realloc_chunk_with_min(mchunkptr p, size_type min_nb, size_type des_nb, int can_move)
   {
      mstate m = &m_state;
      mchunkptr newp = 0;
      size_type oldsize = chunksize(p);
      mchunkptr next = chunk_plus_offset(p, oldsize);
      if (rtcheck(ok_address(m, p) && ok_inuse(p) &&
         ok_next(p, next) && ok_pinuse(next))) {
         if (is_mmapped(p)) {
            newp = mmap_resize(p, des_nb, can_move);
            if(!newp)   //mmap does not return how many bytes we could reallocate, so go the minimum
               newp = mmap_resize(p, min_nb, can_move);
         }
         else if (oldsize >= min_nb) {             // already big enough
            size_type nb = oldsize >= des_nb ? des_nb : oldsize;
            size_type rsize = oldsize - nb;
            if (rsize >= min_chunk_size) {      // split off remainder
               mchunkptr r = chunk_plus_offset(p, nb);
               set_inuse(m, p, nb);
               set_inuse(m, r, rsize);
               dispose_chunk(r, rsize);
            }
            newp = p;
         }
         else if (next == m->top) {  // extend into top
            if (oldsize + m->topsize > min_nb) {
               size_type nb = (oldsize + m->topsize) > des_nb ? des_nb : (oldsize + m->topsize - malloc_alignment);
               size_type newsize = oldsize + m->topsize;
               size_type newtopsize = newsize - nb;
               mchunkptr newtop = chunk_plus_offset(p, nb);
               set_inuse(m, p, nb);
               newtop->head = newtopsize |pinuse_bit;
               m->top = newtop;
               m->topsize = newtopsize;
               newp = p;
            }
         }
         else if (next == m->dv) { // extend into dv
            size_type dvs = m->dvsize;
            if (oldsize + dvs >= min_nb) {
               size_type nb = (oldsize + dvs) >= des_nb ? des_nb : (oldsize + dvs);
               size_type dsize = oldsize + dvs - nb;
               if (dsize >= min_chunk_size) {
                  mchunkptr r = chunk_plus_offset(p, nb);
                  mchunkptr n = chunk_plus_offset(r, dsize);
                  set_inuse(m, p, nb);
                  set_size_and_pinuse_of_free_chunk(r, dsize);
                  clear_pinuse(n);
                  m->dvsize = dsize;
                  m->dv = r;
               }
               else { // exhaust dv
                  size_type newsize = oldsize + dvs;
                  set_inuse(m, p, newsize);
                  m->dvsize = 0;
                  m->dv = 0;
               }
               newp = p;
            }
         }
         else if (!cinuse(next)) { // extend into next free chunk
            size_type nextsize = chunksize(next);
            if (oldsize + nextsize >= min_nb) {
               size_type nb = (oldsize + nextsize) >= des_nb ? des_nb : (oldsize + nextsize);
               size_type rsize = oldsize + nextsize - nb;
               unlink_chunk(m, next, nextsize);
               if (rsize < min_chunk_size) {
                  size_type newsize = oldsize + nextsize;
                  set_inuse(m, p, newsize);
               }
               else {
                  mchunkptr r = chunk_plus_offset(p, nb);
                  set_inuse(m, p, nb);
                  set_inuse(m, r, rsize);
                  dispose_chunk(r, rsize);
               }
               newp = p;
            }
         }
      }
      else {
         usage_error_action(m, chunk2mem(p));
      }
      return newp;
   }

   ///////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////
   //
   //         NEW FUNCTIONS BASED ON DLMALLOC INTERNALS
   //
   ///////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////
   ///////////////////////////////////////////////////////////////

   BOOST_CONTAINER_FORCEINLINE static size_type get_truncated_size(size_type ORIG_SIZE, size_type ROUNDTO)
      {  return (((ORIG_SIZE)/(ROUNDTO)*(ROUNDTO)));  }
   BOOST_CONTAINER_FORCEINLINE static size_type get_rounded_size(size_type ORIG_SIZE, size_type ROUNDTO)
      {  return (((((ORIG_SIZE)-1)/(ROUNDTO)+1)*(ROUNDTO)));  }
   BOOST_CONTAINER_FORCEINLINE static size_type get_truncated_po2_size(size_type ORIG_SIZE, size_type ROUNDTO)
      {  return (((ORIG_SIZE) & (~(ROUNDTO-1))));  }
   BOOST_CONTAINER_FORCEINLINE static size_type get_rounded_po2_size(size_type ORIG_SIZE, size_type ROUNDTO)
      {  return ((((ORIG_SIZE - 1) & (~(ROUNDTO-1))) + ROUNDTO));  }

   // Greatest common divisor and least common multiple
   //gcd is an algorithm that calculates the greatest common divisor of two
   //integers, using Euclid's algorithm.
   //
   //Pre: A > 0 && B > 0
   //Recommended: A > B
   BOOST_CONTAINER_FORCEINLINE static void calculate_gcd(size_type A, size_type B, size_type &out)
   {
      size_type a = A;
      size_type b = B;
      do
      {
         size_type tmp = b;
         b = a % b;
         a = tmp;
      } while (b != 0);

      out = a;
   }

   // lcm is an algorithm that calculates the least common multiple of two
   //integers.
   //
   //Pre: A > 0 && B > 0
   //Recommended: A > B
   BOOST_CONTAINER_FORCEINLINE static void calculate_lcm(size_type A, size_type B, size_type &out)
   {
      calculate_gcd(A, B, out);
      out = (A / out)*B;
   }

   static int calculate_lcm_and_needs_backwards_lcmed
      (size_type backwards_multiple, size_type received_size, size_type size_to_achieve,
      size_type *plcm, size_type *pneeds_backwards_lcmed)
   {
      // Now calculate lcm
      size_type max = backwards_multiple;
      size_type min = malloc_alignment;
      size_type needs_backwards;
      size_type needs_backwards_lcmed;
      size_type lcm;
      size_type current_forward;
      //Swap if necessary
      if(max < min){
         size_type tmp = min;
         min = max;
         max = tmp;
      }
      //Check if it's power of two
      if((backwards_multiple & (backwards_multiple-1)) == 0){
         if(0 != (size_to_achieve & ((backwards_multiple-1)))){
            return 0;
         }

         lcm = max;
         //If we want to use minbytes data to get a buffer between maxbytes
         //and minbytes if maxbytes can't be achieved, calculate the
         //biggest of all possibilities
         current_forward = get_truncated_po2_size(received_size, backwards_multiple);
         needs_backwards = size_to_achieve - current_forward;
         BOOST_CONTAINER_DL_ASSERT((needs_backwards % backwards_multiple) == 0);
         needs_backwards_lcmed = get_rounded_po2_size(needs_backwards, lcm);
         *plcm = lcm;
         *pneeds_backwards_lcmed = needs_backwards_lcmed;
         return 1;
      }
      //Check if it's multiple of alignment
      else if((backwards_multiple & (malloc_alignment - 1u)) == 0){
         lcm = backwards_multiple;
         current_forward = get_truncated_size(received_size, backwards_multiple);
         //No need to round needs_backwards because backwards_multiple == lcm
         needs_backwards_lcmed = needs_backwards = size_to_achieve - current_forward;
         BOOST_CONTAINER_DL_ASSERT((needs_backwards_lcmed & (malloc_alignment - 1u)) == 0);
         *plcm = lcm;
         *pneeds_backwards_lcmed = needs_backwards_lcmed;
         return 1;
      }
      //Check if it's multiple of the half of the alignmment
      else if((backwards_multiple & ((malloc_alignment/2u) - 1u)) == 0){
         lcm = backwards_multiple*2u;
         current_forward = get_truncated_size(received_size, backwards_multiple);
         needs_backwards_lcmed = needs_backwards = size_to_achieve - current_forward;
         if(0 != (needs_backwards_lcmed & (malloc_alignment-1)))
            //while(0 != (needs_backwards_lcmed & (malloc_alignment-1)))
            needs_backwards_lcmed += backwards_multiple;
         BOOST_CONTAINER_DL_ASSERT((needs_backwards_lcmed % lcm) == 0);
         *plcm = lcm;
         *pneeds_backwards_lcmed = needs_backwards_lcmed;
         return 1;
      }
      //Check if it's multiple of the quarter of the alignmment
      else if((backwards_multiple & ((malloc_alignment/4u) - 1u)) == 0){
         size_type remainder;
         lcm = backwards_multiple*4u;
         current_forward = get_truncated_size(received_size, backwards_multiple);
         needs_backwards_lcmed = needs_backwards = size_to_achieve - current_forward;
         //while(0 != (needs_backwards_lcmed & (malloc_alignment-1)))
         //needs_backwards_lcmed += backwards_multiple;
         if(0 != (remainder = ((needs_backwards_lcmed & (malloc_alignment-1))>>(malloc_alignment/8u)))){
            if(backwards_multiple & malloc_alignment/2u){
               needs_backwards_lcmed += (remainder)*backwards_multiple;
            }
            else{
               needs_backwards_lcmed += (4-remainder)*backwards_multiple;
            }
         }
         BOOST_CONTAINER_DL_ASSERT((needs_backwards_lcmed % lcm) == 0);
         *plcm = lcm;
         *pneeds_backwards_lcmed = needs_backwards_lcmed;
         return 1;
      }
      else{
         calculate_lcm(max, min, lcm);
         //If we want to use minbytes data to get a buffer between maxbytes
         //and minbytes if maxbytes can't be achieved, calculate the
         //biggest of all possibilities
         current_forward = get_truncated_size(received_size, backwards_multiple);
         needs_backwards = size_to_achieve - current_forward;
         BOOST_CONTAINER_DL_ASSERT((needs_backwards % backwards_multiple) == 0);
         needs_backwards_lcmed = get_rounded_size(needs_backwards, lcm);
         *plcm = lcm;
         *pneeds_backwards_lcmed = needs_backwards_lcmed;
         return 1;
      }
   }

   // Largest chunk size p could reach by absorbing only the free space that
   //follows it, without moving. It mirrors the cases
   //try_realloc_chunk_with_min() can actually satisfy, so asking that function
   //for exactly this size always succeeds. Returns chunksize(p) when there is
   //nothing to take.
   size_type internal_max_fwd_chunk_size(mchunkptr p)
   {
      mstate m = &m_state;
      const size_type oldsize = chunksize(p);
      mchunkptr next;
      if(is_mmapped(p))
         return oldsize;   // those grow through mmap_resize instead
      next = chunk_plus_offset(p, oldsize);
      if(!rtcheck(ok_next(p, next) && ok_pinuse(next)))
         return oldsize;
      if(next == m->top)
         // top keeps malloc_alignment, exactly as try_realloc_chunk_with_min
         //reserves when it cannot honour the desired size
         return (m->topsize > malloc_alignment) ? (oldsize + m->topsize - malloc_alignment) : oldsize;
      else if(next == m->dv)
         return oldsize + m->dvsize;
      else if(!cinuse(next))
         return oldsize + chunksize(next);
      return oldsize;
   }

   void *internal_grow_both_sides
      (unsigned command
      ,void *oldmem
      ,size_type minbytes
      ,size_type maxbytes
      ,size_type *received_size
      ,size_type backwards_multiple
      ,int only_preferred_backwards)
   {
      mstate m = &m_state;
      mchunkptr oldp = mem2chunk(oldmem);
      size_type oldsize = chunksize(oldp);
      *received_size = oldsize - overhead_for(oldp);
      if(minbytes <= *received_size)
         return oldmem;

      if (rtcheck(ok_address(m, oldp) && ok_inuse(oldp))) {
         if(command & expand_fwd){
            if(try_realloc_chunk_with_min(oldp, request2size(minbytes), request2size(maxbytes), 0)){
               check_inuse_chunk(oldp);
               *received_size = dl_size_impl(oldmem);
               return oldmem;
            }
         }
         else{
            *received_size = dl_size_impl(oldmem);
            if(*received_size >= maxbytes)
               return oldmem;
         }
         //
         //Should we check this?
         //if(backwards_multiple &&
         //   (0 != (minbytes % backwards_multiple) &&
         //    0 != (maxbytes % backwards_multiple)) ){
         //   usage_error_action(m, oldp);
         //   return 0;
         //}
         //
         // We reach here only if forward expansion fails
         if(!(command & expand_bwd) || pinuse(oldp)){
            return 0;
         }
         {
            size_type prevsize = oldp->prev_foot;
            if ((prevsize & use_mmap_bit) != 0){
               //Return failure the previous chunk was mmapped.
               //mremap does not allow expanding to a fixed address (MREMAP_MAYMOVE) without
               //copying (MREMAP_MAYMOVE must be also set).
               return 0;
            }
            else {
               mchunkptr prev = chunk_minus_offset(oldp, prevsize);
               size_type dsize = oldsize + prevsize;
               size_type needs_backwards_lcmed;
               size_type lcm;
               // Forward expansion could not reach minbytes on its own, but the
               //space that *is* free in front still counts towards the total, so
               //the previous chunk only has to cover the remainder. Measure that
               //space without taking it: should the two sides together still fall
               //short, the block has to be left exactly as it was.
               size_type fwd_chunk = oldsize;
               size_type fwd_size  = *received_size;
               if(command & expand_fwd){
                  const size_type max_fwd = internal_max_fwd_chunk_size(oldp);
                  if(max_fwd > oldsize){
                     fwd_chunk = max_fwd;
                     fwd_size  = max_fwd - overhead_for(oldp);
                  }
               }

               // Let's calculate the number of extra bytes of data before the current
               //block's begin. The value is a multiple of backwards_multiple
               //and the alignment
               if(!calculate_lcm_and_needs_backwards_lcmed
                  ( backwards_multiple, fwd_size
                  , only_preferred_backwards ? maxbytes : minbytes
                  , &lcm, &needs_backwards_lcmed)
                  || !rtcheck(ok_address(m, prev))){
                  usage_error_action(m, oldp);
                  return 0;
               }
               // Check if previous block has enough size
               else if(prevsize < needs_backwards_lcmed){
                  // preferred size?
                  return 0;
               }
               // Both halves are available, so now take the forward space. Sized
               //from internal_max_fwd_chunk_size(), so this cannot fail.
               if(fwd_chunk > oldsize){
                  if(!try_realloc_chunk_with_min(oldp, fwd_chunk, fwd_chunk, 0)){
                     BOOST_CONTAINER_DL_ASSERT(0);
                  }
                  check_inuse_chunk(oldp);
                  *received_size = dl_size_impl(oldmem);
                  oldsize = chunksize(oldp);
                  dsize = oldsize + prevsize;
               }
               // We need a minimum size to split the previous one
               if(prevsize >= (needs_backwards_lcmed + min_chunk_size)){
                  mchunkptr r  = chunk_minus_offset(oldp, needs_backwards_lcmed);
                  size_type rsize = oldsize + needs_backwards_lcmed;
                  size_type newprevsize = dsize - rsize;
                  int prev_was_dv = prev == m->dv;

                  BOOST_CONTAINER_DL_ASSERT(newprevsize >= min_chunk_size);

                  if (prev_was_dv) {
                     m->dvsize = newprevsize;
                  }
                  else{// if ((next->head & inuse_bits) == inuse_bits) {
                     unlink_chunk(m, prev, prevsize);
                     insert_chunk(m, prev, newprevsize);
                  }

                  set_size_and_pinuse_of_free_chunk(prev, newprevsize);
                  clear_pinuse(r);
                  set_inuse(m, r, rsize);
                  check_malloced_chunk(chunk2mem(r), rsize);
                  *received_size = chunksize(r) - overhead_for(r);
                  return chunk2mem(r);
               }
               // Check if there is no place to create a new block and
               //the whole new block is multiple of the backwards expansion multiple
               else if(prevsize >= needs_backwards_lcmed && !(prevsize % lcm)) {
                  // Just merge the whole previous block
                  // prevsize is multiple of lcm (and backwards_multiple)
                  *received_size  += prevsize;

                  if (prev != m->dv) {
                     unlink_chunk(m, prev, prevsize);
                  }
                  else{
                     m->dvsize = 0;
                     m->dv     = 0;
                  }
                  set_inuse(m, prev, dsize);
                  check_malloced_chunk(chunk2mem(prev), dsize);
                  return chunk2mem(prev);
               }
               else{
                  // Previous block was big enough but there is no room
                  //to create an empty block and taking the whole block does
                  //not fulfill alignment requirements
                  return 0;
               }
            }
         }
      }
      else{
         usage_error_action(m, oldmem);
         return 0;
      }
   }

   // This is similar to mmap_resize but:
   //* Only to shrink
   //* It takes min and max sizes
   //* Takes additional 'do_commit' argument to obtain the final
   //  size before doing the real shrink operation.
   //
   int internal_mmap_shrink_in_place(mchunkptr oldp, size_type nbmin, size_type nbmax, size_type *received_size, int do_commit)
   {
      mstate m = &m_state;
      size_type oldsize = chunksize(oldp);
      *received_size = oldsize;
      #if BOOST_CONTAINER_DL_MREMAP
      if (is_small(nbmax)) // Can't shrink mmap regions below small size
         return 0;
      {
         size_type effective_min = nbmin > min_large_size ? nbmin : min_large_size;
         // Keep old chunk if big enough but not too big
         if (oldsize >= effective_min + size_t_size &&
            (oldsize - effective_min) <= (m_params.granularity << 1))
            return 0;
         // Now calculate new sizes
         {
            size_type offset = oldp->prev_foot;
            size_type oldmmsize = oldsize + offset + mmap_foot_pad;
            size_type newmmsize = mmap_align(effective_min + six_size_t_sizes + chunk_align_mask);
            *received_size = newmmsize;
            if(!do_commit){
               const int flags = 0; // placate people compiling -Wunused
               char* cp = (char*)call_mremap((char*)oldp - offset,
                  oldmmsize, newmmsize, flags);
               //This must always succeed
               if(!cp){
                  usage_error_action(m, m);
                  return 0;
               }
               {
                  mchunkptr newp = chunk_at(cp + offset);
                  size_type psize = newmmsize - offset - mmap_foot_pad;
                  newp->head = psize;
                  mark_inuse_foot(m, newp, psize);
                  chunk_plus_offset(newp, psize)->head = fencepost_head;
                  chunk_plus_offset(newp, psize+size_t_size)->head = 0;

                  if (cp < m->least_addr)
                     m->least_addr = cp;
                  if ((m->footprint += newmmsize - oldmmsize) > m->max_footprint)
                     m->max_footprint = m->footprint;
                  check_mmapped_chunk(newp);
               }
            }
         }
         return 1;
      }
      #else  //#if BOOST_CONTAINER_DL_MREMAP
      (void)m;
      (void)oldp;
      (void)nbmin;
      (void)nbmax;
      (void)received_size;
      (void)do_commit;
      return 0;
      #endif //#if BOOST_CONTAINER_DL_MREMAP
   }

   int internal_shrink(void* oldmem, size_type minbytes, size_type maxbytes, size_type *received_size, int do_commit)
   {
      mstate m = &m_state;
      *received_size = chunksize(mem2chunk(oldmem)) - overhead_for(mem2chunk(oldmem));
      if (minbytes >= max_request || maxbytes >= max_request) {
         malloc_failure();
         return 0;
      }
      else if(minbytes < min_request){
         minbytes = min_request;
      }
      if (minbytes > maxbytes) {
         return 0;
      }

      {
         mchunkptr oldp = mem2chunk(oldmem);
         size_type oldsize = chunksize(oldp);
         mchunkptr next = chunk_plus_offset(oldp, oldsize);
         void* extra = 0;

         // Try to either shrink or extend into top. Else malloc-copy-free
         if (rtcheck(ok_address(m, oldp) && ok_inuse(oldp) &&
            ok_next(oldp, next) && ok_pinuse(next))) {
            size_type nbmin = request2size(minbytes);
            size_type nbmax = request2size(maxbytes);

            if (nbmin > oldsize){
               // Return error if old size is too small
            }
            else if (is_mmapped(oldp)){
               return internal_mmap_shrink_in_place(oldp, nbmin, nbmax, received_size, do_commit);
            }
            else{ // nbmin <= oldsize /* already big enough*/
               size_type nb = nbmin;
               size_type rsize = oldsize - nb;
               if (rsize >= min_chunk_size) {
                  if(do_commit){
                     mchunkptr remainder = chunk_plus_offset(oldp, nb);
                     set_inuse(m, oldp, nb);
                     set_inuse(m, remainder, rsize);
                     extra = chunk2mem(remainder);
                     priv_deallocate_nolock(extra);
                     check_inuse_chunk(oldp);
                  }
                  *received_size = nb - overhead_for(oldp);
                  return 1;
               }
            }
         }
         else {
            usage_error_action(m, oldmem);
         }
         return 0;
      }
   }

   static const size_type internal_multialloc_default_contiguous_mem = (4096);
   static const size_type sqrt_max_size_t = ((((size_type)-1)>>(sizeof(size_type)*CHAR_BIT/2)));

   // Failure unwind shared by the two multialloc functions below.
   //
   //Two things have to be true afterwards: only the blocks *this call* linked
   //in are returned to the heap (the chain may already carry blocks belonging
   //to the caller), and the chain is left exactly as it was on entry - it must
   //not keep pointing at blocks that are back in the heap, or the caller's
   //next deallocate_many() double-frees them.
   //
   //Each memchain node lives inside the block it describes, so a node's 'next'
   //is read before that block is freed. Freeing block k cannot disturb block
   //k+1's node either: k+1 is still in use, and only free neighbours are
   //ever coalesced.
   void internal_multialloc_rollback
      (memchain *pchain,
      memchain_it entry_last_it, size_type entry_num_mem)
   {
      mstate m = &m_state;
      (void)m;    //every free below is a member
      memchain_it it = entry_last_it;
      it.next();
      while(!memchain::is_end(it)){
         void *addr = it.addr();
         it.next();
         priv_deallocate_nolock(addr);
      }
      // Re-terminate at the entry tail. When the chain arrived empty this is the
      //root node, which restores the pristine MEMCHAIN_INIT state.
      entry_last_it.node_ptr->next_node_ptr = 0;
      pchain->last_node_ptr = entry_last_it.node_ptr;
      pchain->num_mem       = entry_num_mem;
   }

   int internal_node_multialloc
      (size_type n_elements, size_type element_size, size_type contiguous_elements, memchain *pchain)
   {
      mstate m = &m_state;
      void*     mem;            // malloced aggregate space
      mchunkptr p;              // corresponding chunk
      size_type    remainder_size; // remaining bytes while splitting
      flag_t    was_enabled;    // to disable mmap
      size_type    elements_per_segment = 0;
      size_type    element_req_size = request2size(element_size);
      memchain_it prev_last_it = pchain->last();
      size_type prev_num_mem = pchain->size();
      // The count that actually gets multiplied by element_req_size is the
      //per-segment one, and that is what the overflow test below has to use.
      //For ALL_CONTIGUOUS one segment holds everything, so it is n_elements;
      //DEFAULT_CONTIGUOUS derives its count from a fixed byte budget and can
      //never overflow, so nothing needs checking. Testing
      //contiguous_elements itself would reject ALL_CONTIGUOUS outright,
      //the sentinel being (size_type)-1.
      size_type max_seg_elements =
         (contiguous_elements == all_contiguous)     ? n_elements :
      (contiguous_elements == default_contiguous) ? 0u        :
      contiguous_elements;

      //Error if wrong element_size parameter
      if (!element_size ||
         //OR Error if n_elements less than contiguous_elements
         ((contiguous_elements + 1) > (default_contiguous + 1) && n_elements < contiguous_elements) ||
         // OR Error if integer overflow
         (max_seg_elements &&
         sqrt_max_size_t < (element_req_size | max_seg_elements) &&
         (max_size_t / element_req_size) < max_seg_elements)) {
         return 0;
      }
      switch (contiguous_elements) {
      case default_contiguous:
         {
            // Default contiguous, just check that we can store at least one element
            elements_per_segment = internal_multialloc_default_contiguous_mem / element_req_size;
            elements_per_segment += (size_type)(!elements_per_segment);
         }
         break;
      case all_contiguous:
         // All elements should be allocated in a single call
         elements_per_segment = n_elements;
         break;
      default:
         // Allocate in chunks of "contiguous_elements"
         elements_per_segment = contiguous_elements;
      }

      {
         size_type    i;
         size_type next_i;
         //
         //Allocate the aggregate chunk.  First disable direct-mmapping so
         //malloc won't use it, since we would not be able to later
         //free/realloc space internal to a segregated mmap region.
         //
         was_enabled = use_mmap(m);
         disable_mmap(m);
         for (i = 0; i != n_elements; i = next_i)
         {
            size_type accum_size;
            size_type n_elements_left = n_elements - i;
            next_i = i + ((n_elements_left < elements_per_segment) ? n_elements_left : elements_per_segment);
            accum_size = element_req_size * (next_i - i);

            mem = priv_allocate_nolock(accum_size - chunk_overhead);
            if (mem == 0) {
               internal_multialloc_rollback(pchain, prev_last_it, prev_num_mem);
               if (was_enabled)
                  enable_mmap(m);
               return 0;
            }
            p = mem2chunk(mem);
            remainder_size = chunksize(p);

            BOOST_CONTAINER_DL_ASSERT(!is_mmapped(p));
            {  // split out elements
               // Each element's first word IS its memchain node, so the run is
               //linked in place with one store per element and spliced onto the
               //chain with a single INCORPORATE_AFTER, instead of a PUSH_BACK
               //per element (which also rewrites last_node_ptr and num_mem
               //every time). The arrays variant below does the same.
               void *mem_orig = mem;
               memchain_it last_it = pchain->last();
               size_type num_elements = next_i - i;

               size_type num_loops = num_elements - 1;
               remainder_size -= element_req_size * num_loops;
               while (num_loops) {
                  void **mem_prev = ((void**)mem);
                  --num_loops;
                  set_size_and_pinuse_of_inuse_chunk(m, p, element_req_size);
                  p = chunk_plus_offset(p, element_req_size);
                  mem = chunk2mem(p);
                  *mem_prev = mem;
               }
               set_size_and_pinuse_of_inuse_chunk(m, p, remainder_size);
               // mem is the last element; INCORPORATE_AFTER terminates it
               pchain->incorporate_after(last_it, mem_orig, mem, num_elements);
            }
         }
         if (was_enabled)
            enable_mmap(m);
      }
      return 1;
   }

   BOOST_CONTAINER_FORCEINLINE static void boost_alloc_plus_memchain_mem_jump_next(void *THISMEM, void *NEXTMEM)
      {  *((void**)(THISMEM)) = *((void**)((NEXTMEM)));  }

   //This function is based on internal_bulk_free
   //replacing iteration over array[] with memchain.
   //Instead of returning the unallocated nodes, returns a chain of non-deallocated nodes.
   //After forward merging, backwards merging is also tried
   void internal_multialloc_free(memchain *pchain)
   {
      mstate m = &m_state;
      //Blocks that turn out to belong to another heap collect here and go
      //back to the caller. Without footers a block carries no heap mark,
      //there is no other heap to find, and this stays empty.
      memchain ret_chain;
      ret_chain.init();
      if (!preaction(m)) {
         memchain_it a_it = pchain->begin();
         while (!memchain::is_end(a_it)) { // Iterate though all memory holded by the chain
            void* a_mem = a_it.addr();
            mchunkptr a_p = mem2chunk(a_mem);
            size_type psize = chunksize(a_p);
            if (footers && get_mstate_for(a_p) != m) {
               a_it.next();
               ret_chain.push_back(a_mem);
               continue;
            }
            check_inuse_chunk(a_p);
            if (rtcheck(ok_address(m, a_p) && ok_inuse(a_p))) {
               while (1) { // Internal loop to speed up forward and backward merging (avoids some redundant checks)
                  memchain_it b_it = a_it;
                  b_it.next();
                  if (!memchain::is_end(b_it)) {
                     void *b_mem = b_it.addr();
                     mchunkptr b_p = mem2chunk(b_mem);
                     if (b_p == next_chunk(a_p)) { // b chunk is contiguous and next so b's size can be added to a
                        psize += chunksize(b_p);
                        set_inuse(m, a_p, psize);
                        boost_alloc_plus_memchain_mem_jump_next(a_mem, b_mem);
                        continue;
                     }
                     if (rtcheck(ok_address(m, b_p) && ok_inuse(b_p))) {
                        // b chunk is contiguous and previous so a's size can be added to b
                        if (a_p == next_chunk(b_p)) {
                           psize += chunksize(b_p);
                           set_inuse(m, b_p, psize);
                           a_it = b_it;
                           a_p = b_p;
                           a_mem = b_mem;
                           continue;
                        }
                     }
                  }
                  // Normal deallocation starts again in the outer loop
                  a_it = b_it;
                  dispose_chunk(a_p, psize);
                  break;
               }
            }
            else {
               corruption_error_action(m);
               break;
            }
         }
         if (should_trim(m, m->topsize))
            sys_trim(0, false);
         postaction(m);
      }
      if (footers) {
         memchain_it last_pchain = pchain->last();
         pchain->init();
         pchain->incorporate_after(last_pchain
            , ret_chain.first_mem()
            , ret_chain.last_mem()
            , ret_chain.size()
         );
      }
   }

   int internal_multialloc_arrays
      (size_type n_elements, const size_type* sizes, size_type element_size, size_type contiguous_elements, memchain *pchain)
   {
      mstate m = &m_state;
      void*     mem;            // malloced aggregate space
      mchunkptr p;              // corresponding chunk
      size_type    remainder_size; // remaining bytes while splitting
      flag_t    was_enabled;    // to disable mmap
      size_type    size;
      size_type dl_multialloc_segmented_malloc_size;
      size_type max_size;

      // Check overflow
      if(!element_size){
         return 0;
      }
      max_size = max_request/element_size;
      // Different sizes
      switch(contiguous_elements){
      case default_contiguous:
         // Use default contiguous mem
         dl_multialloc_segmented_malloc_size = internal_multialloc_default_contiguous_mem;
         break;
      case all_contiguous:
         dl_multialloc_segmented_malloc_size = max_request + chunk_overhead;
         break;
      default:
         if(max_size < contiguous_elements){
            return 0;
         }
         else{
            // The suggested buffer is just the the element count by the size
            dl_multialloc_segmented_malloc_size = element_size*contiguous_elements;
         }
      }

      {
         size_type    i;
         size_type next_i;
         // Where this call's own blocks start, so the unwind below cannot
         //touch blocks the chain already carried.
         memchain_it entry_last_it = pchain->last();
         size_type entry_num_mem = pchain->size();
         //
         //Allocate the aggregate chunk.  First disable direct-mmapping so
         //malloc won't use it, since we would not be able to later
         //free/realloc space internal to a segregated mmap region.
         //
         was_enabled = use_mmap(m);
         disable_mmap(m);
         for(i = 0, next_i = 0; i != n_elements; i = next_i)
         {
            int error = 0;
            size_type accum_size;
            for(accum_size = 0; next_i != n_elements; ++next_i){
               size_type cur_array_size   = sizes[next_i];
               if(max_size < cur_array_size){
                  error = 1;
                  break;
               }
               else{
                  size_type reqsize = request2size(cur_array_size*element_size);
                  if(((dl_multialloc_segmented_malloc_size - chunk_overhead) - accum_size) < reqsize){
                     if(!accum_size){
                        accum_size += reqsize;
                        ++next_i;
                     }
                     break;
                  }
                  accum_size += reqsize;
               }
            }

            //next_i never passes n_elements: the loop above stops there,
            //and the one place that steps past its own test does so only
            //when there was another element to take.
            BOOST_CONTAINER_DL_ASSERT(next_i <= n_elements);

            mem = error ? 0 : priv_allocate_nolock(accum_size - chunk_overhead);
            if (mem == 0){
               internal_multialloc_rollback(pchain, entry_last_it, entry_num_mem);
               if (was_enabled)
                  enable_mmap(m);
               return 0;
            }
            p = mem2chunk(mem);
            remainder_size = chunksize(p);

            BOOST_CONTAINER_DL_ASSERT(!is_mmapped(p));

            {  // split out elements
               void *mem_orig = mem;
               memchain_it last_it = pchain->last();
               size_type num_elements = next_i-i;

               //The sizes are walked with a pointer rather than indexed.
               //Subscripting lets the optimizer infer how far sizes[i] could
               //possibly reach from the pointer type alone, and then report
               //the loop as able to run past it
               //(-Waggressive-loop-optimizations, naming a trip count of
               //2^61). A pointer that advances in step with i says the same
               //thing and invites no such inference.
               const size_type *psize_i = sizes + i + 1;
               for(++i; i != next_i; ++i, ++psize_i) {
                  void **mem_prev = ((void**)mem);
                  size = request2size(*psize_i * element_size);
                  remainder_size -= size;
                  set_size_and_pinuse_of_inuse_chunk(m, p, size);
                  p = chunk_plus_offset(p, size);
                  mem = chunk2mem(p);
                  *mem_prev = mem;
               }
               set_size_and_pinuse_of_inuse_chunk(m, p, remainder_size);
               pchain->incorporate_after(last_it, mem_orig, mem, num_elements);
            }
         }
         if (was_enabled)
            enable_mmap(m);
      }
      return 1;
   }


   //Is this process known to be running on one thread right now?
   //
   //glibc 2.32 and later keep __libc_single_threaded and clear it before a
   //second thread can run. The flag cannot change while a thread sits in a
   //critical section, so an acquire that skipped the lock is always matched
   //by a release that skips the unlock. Two atomic operations per call become
   //one byte load. Nowhere else is there anything to ask, so the answer is no
   //and every lock is really taken.
   BOOST_CONTAINER_FORCEINLINE static bool single_threaded()
   {
      #if defined(BOOST_CONTAINER_DLMALLOC_GLIBC_IS_SINGLE_THREAD)
      return ::__libc_single_threaded != 0;
      #else
      return false;
      #endif
   }

   BOOST_CONTAINER_FORCEINLINE static int initial_lock(mlock_t *lk, dtl::true_)
   {  return (::boost::container::dtl::spin_mutex_init(lk), 0);  }

   BOOST_CONTAINER_FORCEINLINE static int initial_lock(mlock_t *lk, dtl::false_)
   {  (void)lk;  return 0;  }

   BOOST_CONTAINER_FORCEINLINE static int initial_lock(mlock_t *lk)
   {  return initial_lock(lk, dtl::bool_<use_locks>());  }

   //A spin mutex owns nothing, so this has one body: the arm use_locks would
   //select is the same "return 0" the arm without it needs.
   BOOST_CONTAINER_FORCEINLINE static int destroy_lock(mlock_t *lk)
   {  (void)lk;  return 0;  }

   BOOST_CONTAINER_FORCEINLINE static int acquire_lock(mlock_t *lk, dtl::true_)
   {
      return single_threaded()
         ? 0 : (::boost::container::dtl::spin_mutex_lock(lk), 0);
   }

   BOOST_CONTAINER_FORCEINLINE static int acquire_lock(mlock_t *lk, dtl::false_)
   {  (void)lk;  return 0;  }

   BOOST_CONTAINER_FORCEINLINE static int acquire_lock(mlock_t *lk)
   {  return acquire_lock(lk, dtl::bool_<use_locks>());  }

   BOOST_CONTAINER_FORCEINLINE static void release_lock(mlock_t *lk, dtl::true_)
   {
      if(!single_threaded())
         ::boost::container::dtl::spin_mutex_unlock(lk);
   }

   BOOST_CONTAINER_FORCEINLINE static void release_lock(mlock_t *lk, dtl::false_)
   {  (void)lk;  }

   BOOST_CONTAINER_FORCEINLINE static void release_lock(mlock_t *lk)
   {  release_lock(lk, dtl::bool_<use_locks>());  }

   BOOST_CONTAINER_FORCEINLINE static int try_lock(mlock_t *lk, dtl::true_)
   {
      return single_threaded()
         ? 1 : (::boost::container::dtl::spin_mutex_try_lock(lk) ? 1 : 0);
   }

   BOOST_CONTAINER_FORCEINLINE static int try_lock(mlock_t *lk, dtl::false_)
   {  (void)lk;  return 1;  }

   BOOST_CONTAINER_FORCEINLINE static int try_lock(mlock_t *lk)
   {  return try_lock(lk, dtl::bool_<use_locks>());  }

   //A heap is initialized once it has a top chunk
   BOOST_CONTAINER_FORCEINLINE static bool is_initialized(mstate M)
   {  return M->top != 0;  }

   //Memory comes from mmap alone; a growing process break is never used,
   //because it is one pointer for the whole process and a heap that owns
   //its memory must not move it. The name still has to resolve where a
   //constant condition mentions it, so it always fails.
   BOOST_CONTAINER_FORCEINLINE static void *call_morecore(size_type S)
   {  (void)S;  return mfail();  }

   #if defined(BOOST_WINDOWS)

   BOOST_CONTAINER_FORCEINLINE static void *call_mmap(size_type s)
   {  return win32mmap(s);  }

   BOOST_CONTAINER_FORCEINLINE static void *call_direct_mmap(size_type s)
   {  return win32direct_mmap(s);  }

   BOOST_CONTAINER_FORCEINLINE static int call_munmap(void *a, size_type s)
   {  return win32munmap(a, s);  }

   BOOST_CONTAINER_FORCEINLINE static void *call_mremap
      (void *addr, size_type osz, size_type nsz, int mv)
   {
      //Windows cannot move a reservation, and every caller treats a failure
      //here as "could not resize in place"
      (void)addr;  (void)osz;  (void)nsz;  (void)mv;
      return mfail();
   }

   #else    //BOOST_WINDOWS

   BOOST_CONTAINER_FORCEINLINE static void *call_mmap(size_type s)
   {  return ::mmap(0, s, mmap_prot, mmap_flags, -1, 0);  }

   BOOST_CONTAINER_FORCEINLINE static void *call_direct_mmap(size_type s)
   {  return call_mmap(s);  }

   BOOST_CONTAINER_FORCEINLINE static int call_munmap(void *a, size_type s)
   {  return ::munmap(a, s);  }

   BOOST_CONTAINER_FORCEINLINE static void *call_mremap
      (void *addr, size_type osz, size_type nsz, int mv)
   {
      #if BOOST_CONTAINER_DL_MREMAP
      return ::mremap(addr, osz, nsz, mv);
      #else
      (void)addr;  (void)osz;  (void)nsz;  (void)mv;
      return mfail();
      #endif
   }

   #endif   //BOOST_WINDOWS

   //Usable bytes of a block, which is the chunk less what the chunk costs
   BOOST_CONTAINER_FORCEINLINE static size_type dl_size_impl(const void *p)
   {  return chunksize(mem2chunk(p)) - overhead_for(mem2chunk(p));  }

   //////////////////////////////////////////////////////////////////////////
   //
   //                            The heap state
   //
   //////////////////////////////////////////////////////////////////////////
   //
   //The two members that make this a class rather than a set of functions.
   //m_state is what the bodies above call "m", and m_params holds the
   //tuning that a run can still change - both per instance.
   //
   //malloc_state holds interior pointers to itself (the smallbin bin_at
   //trick), so neither member may be copied or moved once built.
   malloc_state  m_state;
   malloc_params m_params;

   //////////////////////////////////////////////////////////////////////////
   //                    Construction and destruction
   //////////////////////////////////////////////////////////////////////////
   //
   //From init_mparams(). No lock and no lazy path: this runs once, in the
   //constructor, before anything can reach the heap. The magic is therefore
   //always set, which is what lets ensure_initialization() be nothing.
   //What the system reports, and the checks that go with it. Static because
   //create() has to size its mapping before there is an object to ask.
   static void system_sizes(size_type &psize, size_type &gsize)
   {
      #if defined(BOOST_WINDOWS)
      {
         //dwPageSize and dwAllocationGranularity, without <windows.h>
         ::dl_win_system_info si;
         dl_get_system_info(&si);
         psize = si.dwPageSize;
         gsize = ((default_granularity != 0) ? default_granularity
                                             : (size_type)si.dwAllocationGranularity);
      }
      #else
      psize = (size_type)::sysconf(_SC_PAGESIZE);
      gsize = ((default_granularity != 0) ? default_granularity : psize);
      #endif

      //The same sanity checks init_mparams makes. They are about the build,
      //not about the run, so they hold for every instance alike.
      if ((sizeof(size_type) != sizeof(char*)) ||
          (max_size_t < min_chunk_size)  ||
          (sizeof(int) < 4)  ||
          (malloc_alignment < (size_type)8U) ||
          ((malloc_alignment & (malloc_alignment-size_t_one)) != 0) ||
          ((mchunk_size      & (mchunk_size-size_t_one))      != 0) ||
          ((gsize            & (gsize-size_t_one))            != 0) ||
          ((psize            & (psize-size_t_one))            != 0))
         do_abort();
   }

   void init_params()
   {
      size_type psize;
      size_type gsize;
      system_sizes(psize, gsize);

      m_params.granularity    = gsize;
      m_params.page_size      = psize;
      m_params.mmap_threshold = default_mmap_threshold;
      m_params.trim_threshold = default_trim_threshold;
      //Segments are never contiguous with one another
      m_params.default_mflags = use_lock_bit|use_mmap_bit|use_noncontiguous_bit;

      //footers is off, so nothing ever reads this; it is set anyway,
      //because a non-zero magic is what "initialized" means everywhere else
      //in the heap.
      m_params.magic = (size_type)0x58585858U ^ (size_type)(size_type)this;
      m_params.magic |= (size_type)8U;
      m_params.magic &= ~(size_type)7U;
   }
   #endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

};

//! The heap with the default configuration, which is what nearly every
//! caller wants.
typedef basic_dlmalloc<> dlmalloc;

#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

//////////////////////////////////////////////////////////////////////////////
//                     The heap the process shares
//////////////////////////////////////////////////////////////////////////////

//BOOST_SYMBOL_VISIBLE: required for cross-module unification on ELF - the
//visibility of intermodule_globals<>'s storage instantiation is the minimum
//of the visibilities of all its template arguments.
struct BOOST_SYMBOL_VISIBLE dlmalloc_globals_t
{
   dlmalloc heap;
};

//The rendezvous key is derived from this type's name
struct BOOST_SYMBOL_VISIBLE dlmalloc_globals_options
{
   //The heap is deliberately immortal: destructors of user global objects
   //and atexit callbacks may still release memory obtained from it after
   //Boost.Container's own static objects are gone, and ~basic_dlmalloc()
   //unmaps every segment, blocks still handed out included. The OS reclaims
   //everything at process exit.
   static const bool destroy_at_exit = false;

   //pin_constructing_module defaults to false: this is plain state (heap
   //bytes, tuning parameters, lock) with no vtable nor any function pointer
   //into the constructing module's image. The configuration's abort and
   //failure actions are stateless types resolved at compile time in each
   //module, not pointers stored in the object.
};

//Both arguments must be default-visible or every module gets its own heap;
//see BOOST_CONTAINER_INTERMODULE_ASSERT_VISIBLE in intermodule_globals.hpp.
BOOST_CONTAINER_INTERMODULE_ASSERT_VISIBLE(dlmalloc_globals_t,
   "boost::container::dlmalloc_globals_t must be declared BOOST_SYMBOL_VISIBLE");
BOOST_CONTAINER_INTERMODULE_ASSERT_VISIBLE(dlmalloc_globals_options,
   "boost::container::dlmalloc_globals_options must be declared "
   "BOOST_SYMBOL_VISIBLE");

//! The process-wide heap itself: what a stateless allocator allocates from.
BOOST_CONTAINER_FORCEINLINE dlmalloc& dlmalloc_heap()
{
   return ::boost::container::dtl::intermodule_globals
      <dlmalloc_globals_t, dlmalloc_globals_options>().heap;
}

#endif   //#ifndef BOOST_CONTAINER_DOXYGEN_INVOKED

}  //namespace container {
}  //namespace boost {

#include <boost/container/detail/config_end.hpp>

//Every macro this header defined is one of its own BOOST_CONTAINER_DL_
//names, undefined here so including dlmalloc.hpp leaves no trace in the
//including translation unit.

#undef BOOST_CONTAINER_DL_MMAP_CLEARS

#ifdef BOOST_CONTAINER_DL_MAP_ANONYMOUS
#undef BOOST_CONTAINER_DL_MAP_ANONYMOUS
#endif

#ifdef BOOST_CONTAINER_DL_MREMAP
#undef BOOST_CONTAINER_DL_MREMAP
#endif

#ifdef BOOST_CONTAINER_DLMALLOC_GLIBC_IS_SINGLE_THREAD
#undef BOOST_CONTAINER_DLMALLOC_GLIBC_IS_SINGLE_THREAD
#endif

#undef BOOST_CONTAINER_DL_ASSERT

#endif   //BOOST_CONTAINER_DETAIL_DLMALLOC_HPP
