//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2026-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
//basic_dlmalloc takes the knobs dlmalloc settles with macros as a type
//parameter. This checks that a Config is really read: for each knob there is
//either something observable that changes when it changes, or - where the
//knob has nothing to show from outside - a heap built with it that still
//works and still checks out.
//
//A test that only built the instantiations would pass with a class that
//ignored Config entirely, so wherever a knob has a visible consequence it is
//the consequence that is asserted, not the constant.
//
//Every knob is measured as a difference between two heaps, and both of them
//are spelled out here rather than taken from dlmalloc_default_config: what
//is being checked is that a Config is honoured, not that it differs from
//whatever the library happens to default to today.
#include <boost/container/detail/dlmalloc.hpp>
#include <boost/container/detail/spin_mutex.hpp>
#include "lightweight_test.hpp"

#include <cstddef>
#include <cstring>

using boost::container::basic_dlmalloc;
using boost::container::dlmalloc;
using boost::container::dlmalloc_default_config;

namespace {

typedef dlmalloc_default_config::size_type size_type;

template<class T, class U> struct same        {  static const bool value = false;  };
template<class T>          struct same<T, T>  {  static const bool value = true;   };

bool is_aligned_to(const void *p, size_type a)
{  return (size_type)(::std::size_t)p % a == 0;  }

//---------------------------------------------------------------------------
// A baseline, and configurations one knob away from it
//---------------------------------------------------------------------------
struct baseline_config
{
   typedef ::std::size_t size_type;
   static const bool footers                 = false;
   static const bool insecure                = false;
   static const bool proceed_on_error        = false;
   static const bool malloc_inspect_all      = false;
   static const bool no_segment_traversal    = false;
   static const bool use_locks               = true;
   static const bool wide_smallbins          = true;
   static const bool rebased_smallbins       = true;
   static const bool debug                   = dlmalloc_default_config::debug;
   static const bool abort_on_assert_failure = true;
   typedef ::boost::container::dlmalloc_errno_action malloc_failure_action;
   typedef ::boost::container::dlmalloc_abort        abort_action;
   static const size_type malloc_alignment        = (size_type)(2 * sizeof(void *));
   static const size_type default_granularity     = (size_type)64U * 1024U;
   static const size_type default_trim_threshold  = (size_type)2U * 1024U * 1024U;
   static const size_type default_mmap_threshold  = (size_type)256U * 1024U;
   static const size_type max_release_check_rate  = 4095;
};

typedef basic_dlmalloc<baseline_config> baseline_heap;

//Inheriting is the intended way to change a knob: the member declared here
//hides the one it would otherwise inherit.
struct big_granularity_config : baseline_config
{  static const size_type default_granularity = (size_type)1024U * 1024U;  };

struct wide_alignment_config : baseline_config
{  static const size_type malloc_alignment = 64;  };

struct eager_mmap_config : baseline_config
{  static const size_type default_mmap_threshold = (size_type)32U * 1024U;  };

struct unlocked_config : baseline_config
{  static const bool use_locks = false;  };

struct footers_config : baseline_config
{  static const bool footers = true;  };

//The same trick applied to the library's own defaults, which is what the
//documentation tells a user to do.
struct inherits_the_defaults : dlmalloc_default_config
{  static const size_type default_granularity = (size_type)256U * 1024U;  };

//Everything at once, including the three knobs that have nothing to show
//from outside - if one of them stopped compiling, this is what would say so.
struct everything_config
{
   typedef ::std::size_t size_type;
   static const bool footers                 = true;
   static const bool insecure                = true;
   static const bool proceed_on_error        = true;
   static const bool malloc_inspect_all      = true;
   static const bool no_segment_traversal    = true;
   static const bool use_locks               = false;
   static const bool wide_smallbins          = true;
   static const bool rebased_smallbins       = true;
   static const bool debug                   = dlmalloc_default_config::debug;
   static const bool abort_on_assert_failure = true;
   typedef ::boost::container::dlmalloc_errno_action malloc_failure_action;
   typedef ::boost::container::dlmalloc_abort        abort_action;
   static const size_type malloc_alignment        = 32;
   static const size_type default_granularity     = (size_type)128U * 1024U;
   static const size_type default_trim_threshold  = (size_type)64U * 1024U;
   static const size_type default_mmap_threshold  = (size_type)64U * 1024U;
   static const size_type max_release_check_rate  = 15;
};

//---------------------------------------------------------------------------

//Fill, read back and free, so a heap that hands out overlapping or
//unwritable memory is caught rather than merely counted.
template<class Heap>
void exercise(Heap &h)
{
   const size_type n = 64;
   void *p[n];
   size_type i;
   for(i = 0; i != n; ++i){
      p[i] = h.allocate(i * 37 + 1);
      BOOST_TEST(p[i] != 0);
      if(p[i])
         ::std::memset(p[i], (int)(i & 0xFF), i * 37 + 1);
   }
   BOOST_TEST(h.check());
   for(i = 0; i != n; ++i){
      if(!p[i])
         continue;
      const unsigned char *const c = (const unsigned char *)p[i];
      BOOST_TEST(c[0] == (unsigned char)(i & 0xFF));
      BOOST_TEST(c[i * 37] == (unsigned char)(i & 0xFF));
   }
   for(i = 0; i != n; ++i)
      h.deallocate(p[i]);
   BOOST_TEST(h.check());
}

void test_default_is_the_default()
{
   //void and the config it stands for must name the same configuration, and
   //dlmalloc must still be the class it always was.
   BOOST_TEST((same<dlmalloc, basic_dlmalloc<> >::value));
   BOOST_TEST((same<dlmalloc::config_type, dlmalloc_default_config>::value));
   BOOST_TEST((same<basic_dlmalloc<dlmalloc_default_config>::config_type,
                    dlmalloc_default_config>::value));

   //Naming the defaults explicitly is a different type, but not a different
   //heap: same layout, same behaviour.
   BOOST_TEST(sizeof(basic_dlmalloc<dlmalloc_default_config>) == sizeof(dlmalloc));
   BOOST_TEST(!(same<basic_dlmalloc<>, basic_dlmalloc<unlocked_config> >::value));

   //A default-constructed heap holds nothing at all, so it has none to read.
   dlmalloc d;
   BOOST_TEST(d.footprint() == 0u);

   //create() must allocate to hold the heap state
   dlmalloc *const dc = dlmalloc::create();
   BOOST_TEST(dc != 0);
   BOOST_TEST(dc->footprint() == dlmalloc_default_config::default_granularity);

   typedef basic_dlmalloc<dlmalloc_default_config> named_default_heap;
   named_default_heap *const bc = named_default_heap::create();
   BOOST_TEST(bc != 0);
   BOOST_TEST(dc->footprint() == bc->footprint());
   exercise(*dc);
   exercise(*bc);
   (void)dlmalloc::destroy(dc);
   (void)named_default_heap::destroy(bc);

   //Inheriting from the shipped defaults and hiding one member is the
   //documented way to change a knob.
   typedef basic_dlmalloc<inherits_the_defaults> inheriting_heap;
   inheriting_heap *const ic = inheriting_heap::create();
   BOOST_TEST(ic != 0);
   BOOST_TEST(ic->footprint() == inherits_the_defaults::default_granularity);
   exercise(*ic);
   (void)inheriting_heap::destroy(ic);
}

void test_granularity()
{
   //A heap create() is asked for nothing maps exactly one granularity unit.
   typedef basic_dlmalloc<big_granularity_config> wide_heap;
   baseline_heap *const b = baseline_heap::create();
   wide_heap     *const w = wide_heap::create();
   BOOST_TEST(b != 0 && w != 0);

   BOOST_TEST(b->footprint() == baseline_config::default_granularity);
   BOOST_TEST(w->footprint() == big_granularity_config::default_granularity);
   BOOST_TEST(w->footprint() > b->footprint());
   exercise(*w);
   (void)baseline_heap::destroy(b);
   (void)wide_heap::destroy(w);
}

void test_alignment()
{
   //Every block honours the alignment the configuration asks for, without
   //anyone having to ask for it per call.
   basic_dlmalloc<wide_alignment_config> h;
   const size_type a = wide_alignment_config::malloc_alignment;
   for(size_type i = 0; i != 32; ++i){
      void *const p = h.allocate(i * 11 + 1);
      BOOST_TEST(p != 0);
      BOOST_TEST(is_aligned_to(p, a));
      h.deallocate(p);
   }
   BOOST_TEST(h.check());
   exercise(h);
}

void test_mmap_threshold()
{
   //A request at or above the threshold is mapped on its own, so freeing it
   //hands the memory straight back and the footprint returns to where it
   //started. Below the threshold the block comes out of a segment, which is
   //kept - the baseline heap is here to show that difference is real and
   //not just something every heap does.
   const size_type big = (size_type)100 * 1024;   //over 32K, under 256K

   basic_dlmalloc<eager_mmap_config> eager;
   //One small request first: the direct-mmap path is off
   //until the heap is initialized. Without this, the
   //big request below would come out of a segment and the test would be
   //measuring the wrong thing.
   void *const warm = eager.allocate(64);
   BOOST_TEST(warm != 0);
   eager.deallocate(warm);
   const size_type eager_before = eager.footprint();
   void *const pe = eager.allocate(big);
   BOOST_TEST(pe != 0);
   BOOST_TEST(eager.footprint() > eager_before);
   eager.deallocate(pe);
   BOOST_TEST(eager.footprint() == eager_before);
   BOOST_TEST(eager.check());

   baseline_heap plain;
   const size_type plain_before = plain.footprint();
   void *const pp = plain.allocate(big);
   BOOST_TEST(pp != 0);
   BOOST_TEST(plain.footprint() > plain_before);
   plain.deallocate(pp);
   BOOST_TEST(plain.footprint() > plain_before);
   BOOST_TEST(plain.check());
}

//The lock sits between two fields of the heap state, so whether dropping it
//makes the object smaller depends on whether it fit in padding that was
//already there - which is a question about the target, not about the class.
//These two ask that question in the same shape the state has around the
//lock, and the class has to give the same answer.
struct lock_padding_locked {
   size_type                             head;
   unsigned                              mflags;
   ::boost::container::dtl::spin_mutex_t lock;
   void                                 *tail;
};
struct lock_padding_unlocked {
   size_type  head;
   unsigned   mflags;
   void      *tail;
};

void test_use_locks()
{
   //Turning the lock off removes a member, so the heap shrinks by exactly
   //what the member costs where it costs anything at all. On LP64 that is
   //nothing - the lock fits in padding the state already had - and there
   //the size cannot show the knob was read; dlmalloc_upstream_compat_test
   //is what pins the layout down on both settings.
   const size_type drop = sizeof(lock_padding_locked) - sizeof(lock_padding_unlocked);
   BOOST_TEST(sizeof(baseline_heap) - sizeof(basic_dlmalloc<unlocked_config>) == drop);

   basic_dlmalloc<unlocked_config> h;
   BOOST_TEST(h.check());
   exercise(h);
}

void test_footers()
{
   //A footer is one word per block, written after the block, so a block big
   //enough to hold a given request has exactly one word less room in it.
   //Both heaps are built here because the figure is only meaningful as a
   //difference: what a bare request rounds up to is the target's business.
   baseline_heap                    plain;
   basic_dlmalloc<footers_config>  h;

   void *const pp = plain.allocate(1);
   void *const ph = h.allocate(1);
   BOOST_TEST(pp != 0 && ph != 0);
   if(pp && ph){
      BOOST_TEST(plain.usable_size(pp) ==
                 h.usable_size(ph) + sizeof(size_type));
      plain.deallocate(pp);
      h.deallocate(ph);
   }

   //And the sizes it reports still describe the blocks it handed out.
   for(size_type i = 1; i < 200; i += 7){
      void *const p = h.allocate(i);
      BOOST_TEST(p != 0);
      if(p){
         BOOST_TEST(h.usable_size(p) >= i);
         ::std::memset(p, 0x5A, i);
         h.deallocate(p);
      }
   }
   BOOST_TEST(h.check());
   exercise(h);
}

//The four ways the two bin-layout knobs can be set. Each one gives the small
//bins a different spacing, a different index origin, or both, and moves the
//small/large boundary with them.
struct bins_plain : baseline_config
{
   static const bool wide_smallbins    = false;
   static const bool rebased_smallbins = false;
};
struct bins_wide : baseline_config
{
   static const bool wide_smallbins    = true;
   static const bool rebased_smallbins = false;
};
struct bins_rebased : baseline_config
{
   static const bool wide_smallbins    = false;
   static const bool rebased_smallbins = true;
};
struct bins_both : baseline_config
{
   static const bool wide_smallbins    = true;
   static const bool rebased_smallbins = true;
};

//Walk sizes from the smallest chunk to well past any of the four boundaries,
//so every request lands in a small bin under some settings and in the tree
//under others, and free them out of order so the bins actually fill.
//
//check() is what makes this bite: it walks every small bin and asserts that
//each chunk in bin i really has small_index(chunksize) == i, so an index
//that disagreed with its own inverse, or with where is_small() hands over to
//the tree, would be caught right here rather than becoming a wrong-sized
//block handed to a caller. It only asserts in a debug build - which is why
//this test earns its keep in the debug arms of the matrix, not the release
//one.
template<class Cfg>
void exercise_bins()
{
   typedef basic_dlmalloc<Cfg> heap;
   heap h;
   const size_type n = 160;
   void *p[n];
   size_type i;

   //Every size class from tiny to past the widest boundary, twice over.
   for(i = 0; i != n; ++i){
      const size_type bytes = (i % 80) * 8 + 1;
      p[i] = h.allocate(bytes);
      BOOST_TEST(p[i] != 0);
      if(p[i])
         ::std::memset(p[i], (int)(i & 0xFF), bytes);
   }
   BOOST_TEST(h.check());

   //Free every other one, so free chunks land in bins with in-use chunks
   //still between them and cannot all coalesce away.
   for(i = 0; i < n; i += 2){
      h.deallocate(p[i]);
      p[i] = 0;
   }
   BOOST_TEST(h.check());

   //Ask for the same size classes back: these come out of the bins just
   //filled, which is the path that reads the index rather than writing it.
   for(i = 0; i < n; i += 2){
      const size_type bytes = (i % 80) * 8 + 1;
      p[i] = h.allocate(bytes);
      BOOST_TEST(p[i] != 0);
      BOOST_TEST(h.usable_size(p[i]) >= bytes);
   }
   BOOST_TEST(h.check());

   for(i = 0; i != n; ++i)
      h.deallocate(p[i]);
   BOOST_TEST(h.check());
}

//is_small() decides which of the two structures a size belongs to, so it has
//to agree with the boundary the configuration computed - for every size, not
//just the ones a caller would ever pass. 0 in particular: that is how "there
//is no designated victim" is spelled, and it reaches is_small() through the
//assertion in replace_dv(). Under the re-based index 0 is below the smallest
//chunk and so is not small, which is the answer that matters; under the
//original index bin 0 nominally stands for size 0 and it is.
template<class Cfg>
void check_is_small_boundary()
{
   typedef basic_dlmalloc<Cfg> heap;
   heap h;
   //The heap only exposes behaviour, so what is checked here is that every
   //request the heap actually serves comes back consistent with the
   //boundary - a size classified into the wrong structure shows up as a
   //corrupt bin, which check() catches.
   for(size_type i = 1; i < 700; i += 3){
      void *const p = h.allocate(i);
      BOOST_TEST(p != 0);
      if(p){
         BOOST_TEST(h.usable_size(p) >= i);
         h.deallocate(p);
      }
   }
   BOOST_TEST(h.check());
}

void test_smallbin_layout()
{
   //All four settings have to produce a working heap. A bin index that did
   //not agree with its inverse, or a boundary that left a gap between the
   //last small bin and the first tree bin, corrupts the free lists the
   //moment a chunk is filed - so this is a real check on the arithmetic,
   //not just on the instantiations compiling.
   exercise_bins<bins_plain>();
   exercise_bins<bins_wide>();
   exercise_bins<bins_rebased>();
   exercise_bins<bins_both>();

   check_is_small_boundary<bins_plain>();
   check_is_small_boundary<bins_wide>();
   check_is_small_boundary<bins_rebased>();
   check_is_small_boundary<bins_both>();

   //The wide spacing is what a 64-bit target gains and a 32-bit one already
   //had, so it may not change anything here; the re-based index always
   //moves the boundary up, by the bins below the smallest chunk that it
   //reclaims. Both are private, so what is checked is that a heap built
   //each way still serves and still checks out, above.
   basic_dlmalloc<bins_both> h;
   BOOST_TEST(h.check());
   exercise(h);
}

//The failure action is a type, so the way to see it is to supply one that
//leaves a mark. A request no system can map is refused inside sys_alloc,
//which is where dlmalloc runs the action.
int g_refusals = 0;

struct counting_failure_action
{
   void operator()() const {  ++g_refusals;  }
};

struct counting_failure_config : baseline_config
{  typedef counting_failure_action malloc_failure_action;  };

void test_failure_action()
{
   basic_dlmalloc<counting_failure_config> h;
   //Bigger than the address space, so no system can map it
   const size_type big = (~(size_type)0) - (size_type)(16u*1024u*1024u);

   g_refusals = 0;
   void *const p = h.allocate(big);
   BOOST_TEST(p == 0);
   BOOST_TEST(g_refusals == 1);

   //A request that cannot even be represented is refused the same way, and
   //this is the one dlmalloc gets wrong: the products below overflow, a far
   //smaller block is handed out, and the split then writes past its end.
   //The size has to be big enough that the arithmetic really wraps at this
   //width - half the address space each, four times over.
   const size_type unrepresentable = (~(size_type)0) / 2;
   g_refusals = 0;
   size_type sizes[4];
   for(size_type i = 0; i != 4; ++i)
      sizes[i] = unrepresentable;
   void *chunks[4];
   BOOST_TEST(h.independent_comalloc(4, sizes, chunks) == 0);
   BOOST_TEST(g_refusals >= 1);
   BOOST_TEST(h.check());

   g_refusals = 0;
   BOOST_TEST(h.independent_calloc(4, unrepresentable, chunks) == 0);
   BOOST_TEST(g_refusals >= 1);
   BOOST_TEST(h.check());

   //A request that succeeds leaves the action alone.
   g_refusals = 0;
   void *const q = h.allocate(64);
   BOOST_TEST(q != 0);
   BOOST_TEST(g_refusals == 0);
   h.deallocate(q);
   BOOST_TEST(h.check());
}

//The checks themselves are a knob too. Nothing observable says whether they
//ran - a heap that passes them behaves the same either way - so this only
//has to see that a heap built each way still works.
struct no_debug_config : baseline_config
{  static const bool debug = false;  };

struct debug_config : baseline_config
{  static const bool debug = true;  };

void test_debug_knob()
{
   basic_dlmalloc<no_debug_config> off;
   BOOST_TEST(off.check());
   exercise(off);

   basic_dlmalloc<debug_config> on;
   BOOST_TEST(on.check());
   exercise(on);
}

void test_everything_at_once()
{
   typedef basic_dlmalloc<everything_config> everything_heap;
   everything_heap *const hc = everything_heap::create();
   BOOST_TEST(hc != 0);
   BOOST_TEST(hc->footprint() == everything_config::default_granularity);
   (void)everything_heap::destroy(hc);

   everything_heap h;
   BOOST_TEST(h.check());
   exercise(h);

   void *const p = h.allocate(64);
   BOOST_TEST(p != 0);
   BOOST_TEST(is_aligned_to(p, everything_config::malloc_alignment));
   h.deallocate(p);
   BOOST_TEST(h.check());
}

}  //namespace

int main()
{
   test_default_is_the_default();
   test_granularity();
   test_alignment();
   test_mmap_threshold();
   test_use_locks();
   test_footers();
   test_smallbin_layout();
   test_failure_action();
   test_debug_knob();
   test_everything_at_once();
   return ::boost::report_errors();
}
