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
// Compares Boost.Container's bundled dlmalloc against the platform's
// malloc/free.
//
// Design (same shape as bench_vector_common.hpp):
//  - Auto-scaling measurement: each timing repeats the workload until a minimum
//    wall-clock budget elapses, runs several trials and discards the slowest.
//  - Dead-store-elimination barriers: clobber()/escape() (bench_utils.hpp) wrap
//    the measured region so the optimizer cannot delete the work.
//  - Speedup output: every allocator is timed and reported as
//    time(baseline)/time(allocator) against the first one registered, so a
//    figure above 1 means that allocator is that many times faster than the
//    baseline. A per-column geometric mean closes the table.
//
// Both allocators are warmed up before being measured, so the numbers describe
// the steady-state cost of the allocation algorithm rather than which of the
// two happened to pay for growing its heap first.
//
// The last two operations are the ones where dlmalloc offers something
// malloc's interface cannot express, benchmarked as the same *job* rather than
// the same call:
//  - "grow": enlarge a live buffer keeping its contents. malloc uses realloc;
//    dlmalloc issues one allocation_command with ALLOCATE_NEW|EXPAND_FWD, the
//    call boost::container::allocator itself makes, and only copies when the
//    command reports it could not expand in place.
//  - "multi_node": obtain (and later release) many equally sized nodes at once.
//    malloc loops; dlmalloc issues one dlmalloc_multialloc_nodes() and one
//    dlmalloc_multidealloc().
//
// Reading the large-size rows. From 4 KiB up, most of dlmalloc's advantage over
// glibc is not faster allocation logic but memory retention: glibc consolidates
// and hands those blocks back to the OS between iterations, so the next one
// re-faults the pages, while dlmalloc keeps its arena. Re-running the multi_node
// workload after mallopt(M_TRIM_THRESHOLD/M_MMAP_THRESHOLD, huge) collapses
// glibc's cost from ~1300-2000 ns/node to ~14-16, i.e. from a 118-146x gap down
// to 1.2x, while dlmalloc's own figure does not move (11.1 -> 11.3 ns). The
// retention is a real, measurable win for a program that allocates in waves,
// but it is a policy difference and it is tunable, so do not read those rows as
// "dlmalloc allocates 50x faster". The small-size rows, where the working set
// stays hot, are the allocator-to-allocator comparison.
//
// Defaults follow the convention of the other benchmarks here: a short run so
// the bench suite stays quick, and a full sweep under -DLONG_BENCH.
//
//////////////////////////////////////////////////////////////////////////////

#include <boost/container/detail/config_begin.hpp>
#include <boost/container/detail/dlmalloc.hpp>
#include <boost/container/detail/workaround.hpp>
#include <boost/container/string.hpp>
#include <boost/container/vector.hpp>

#include "bench_utils.hpp"   //cpu_timer, clobber(), escape()

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdlib>   //std::malloc, std::free, std::realloc
#include <cstring>   //std::memcpy, std::memset
#include <iomanip>
#include <iostream>
#include <sstream>

namespace bc = boost::container;

typedef boost::move_detail::nanosecond_type nsec_t;

///////////////////////////////////////////////////////////////////////////////
// Benchmark configuration knobs.
///////////////////////////////////////////////////////////////////////////////
struct bench_defaults
{
#if defined(LONG_BENCH)
   static const std::size_t num_trials        = 5;
   static const std::size_t num_sizes         = 11;
   //60 ms per trial
   static const nsec_t      min_time_per_trial = nsec_t(60) * 1000000;
   static const std::size_t batch_elements     = 4096;
   static const std::size_t churn_reps         = 4096;
   static const std::size_t working_set_bytes  = 1u << 20;   //1 MiB
#else
   static const std::size_t num_trials        = 1;
   static const std::size_t num_sizes         = 4;
   static const nsec_t      min_time_per_trial = 0;
   static const std::size_t batch_elements     = 256;
   static const std::size_t churn_reps         = 256;
   static const std::size_t working_set_bytes  = 1u << 16;   //64 KiB
#endif
};

//Block sizes swept by every operation, ascending. Kept as a plain array so the
//sweep is also valid in C++03.
//
//The steps double, so the sweep straddles every boundary these allocators
//change behaviour at rather than jumping over it: dlmalloc's smallbin/treebin
//split (256 bytes, or 512 with BOOST_CONTAINER_DLMALLOC_WIDE_SMALLBINS),
//glibc's tcache limit (1032), and the mmap thresholds of both (128 KiB and
//256 KiB, above this range). A size on each side of a boundary is what makes
//a row attributable to one mechanism.
inline std::size_t bench_size(std::size_t i)
{
   static const std::size_t sizes[] =
      { 16u, 32u, 64u, 128u, 256u, 512u, 1024u, 2048u, 4096u, 8192u, 16384u };
   //A table entry must exist for every size the sweep asks for; a second,
   //hand-written index list would silently go stale whenever this table is
   //edited, so both the check and the short run's sampling are derived from
   //the table itself.
   const std::size_t entries = sizeof(sizes)/sizeof(sizes[0]);
   typedef int bench_size_table_must_cover_the_sweep
      [(bench_defaults::num_sizes <= sizeof(sizes)/sizeof(sizes[0])) ? 1 : -1];
   //referenced so the check is not an unused local typedef
   (void)sizeof(bench_size_table_must_cover_the_sweep);
#if defined(LONG_BENCH)
   (void)entries;
   return sizes[i];
#else
   //The short run spreads its few points across the whole table instead of
   //truncating it, so it still spans the entire range.
   {
      const std::size_t last = entries - 1u;
      const std::size_t span = bench_defaults::num_sizes > 1u
                             ? bench_defaults::num_sizes - 1u : 1u;
      return sizes[(i * last) / span];
   }
#endif
}

//Length of the x1.5 growth chain for a starting size. dlmalloc serves anything
//above 256 KiB with mmap, and glibc above 128 KiB, so a chain that crosses
//either threshold stops comparing the two allocators and starts comparing
//mmap/munmap against mremap - which does not reproduce between runs. Shorten
//the chain instead, so the largest sizes stay inside both heaps: 8 steps up to
//4 KiB, 4 at 16 KiB and a single grow at 64 KiB. Fewer steps means the outer
//loop runs proportionally more times, and both allocators always perform the
//very same sequence, so the columns stay comparable.
inline std::size_t grow_steps(std::size_t sz)
{
   const std::size_t cap = 120u * 1024u;   //below glibc's 128 KiB mmap threshold
   std::size_t steps = 8u;
   while(steps > 1u){
      //final size after "steps" multiplications by 3/2
      std::size_t final_sz = sz;
      for(std::size_t s = 0; s != steps; ++s)
         final_sz = final_sz + final_sz / 2u + 1u;
      if(final_sz <= cap)
         break;
      --steps;
   }
   return steps;
}

//Number of live blocks a batch operation uses for a given block size. The bytes
//touched per run are held roughly constant across the sweep: letting the batch
//grow with the block size makes the largest rows measure the OS paging a bigger
//working set in and out, which swamps the allocator and does not reproduce
//between runs.
inline std::size_t blocks_for(std::size_t sz)
{
   std::size_t n = bench_defaults::working_set_bytes / sz;
   if(n < 8u)
      n = 8u;
   if(n > bench_defaults::batch_elements)
      n = bench_defaults::batch_elements;
   return n;
}

///////////////////////////////////////////////////////////////////////////////
// Deterministic PRNG, so every allocator sees the very same request stream.
///////////////////////////////////////////////////////////////////////////////
class rng
{
   unsigned x_;

   public:
   explicit rng(unsigned seed = 123456789u) : x_(seed ? seed : 1u) {}

   unsigned next()
   {
      //xorshift32
      x_ ^= x_ << 13;
      x_ ^= x_ >> 17;
      x_ ^= x_ << 5;
      return x_;
   }

   std::size_t below(std::size_t n)
   {  return n ? std::size_t(next() % n) : 0u;  }
};

///////////////////////////////////////////////////////////////////////////////
// Allocator policies. Everything is static so the calls inline the same way
// they would in real code.
///////////////////////////////////////////////////////////////////////////////
struct std_malloc_policy
{
   static const char *name() {  return "malloc";  }

   static void *alloc(std::size_t n)
   {  return std::malloc(n);  }

   static void dealloc(void *p)
   {  std::free(p);  }

   //Enlarge keeping contents.
   static void *grow(void *p, std::size_t /*oldsz*/, std::size_t newsz)
   {  return std::realloc(p, newsz);  }

   //Obtain n_elements blocks of elem_size, storing them in out.
   static void alloc_many(std::size_t n_elements, std::size_t elem_size, void **out)
   {
      for(std::size_t i = 0; i != n_elements; ++i)
         out[i] = std::malloc(elem_size);
   }

   static void dealloc_many(std::size_t n_elements, void **out)
   {
      for(std::size_t i = 0; i != n_elements; ++i)
         std::free(out[i]);
   }
};

struct dlmalloc_policy
{
   static const char *name() {  return "dlmalloc";  }

   static void *alloc(std::size_t n)
   {  return bc::dlmalloc_malloc(n);  }

   static void dealloc(void *p)
   {  bc::dlmalloc_free(p);  }

   //dlmalloc has no realloc. The intended way to enlarge a live block is a
   //single allocation_command asking for expansion in place OR a new block,
   //which is what boost::container::allocator issues: one entry into the
   //allocator (one lock acquisition) instead of a grow attempt followed by a
   //separate malloc. ret.second tells which happened - when the block was
   //reused the contents are already in place and nothing is copied.
   static void *grow(void *p, std::size_t oldsz, std::size_t newsz)
   {
      std::size_t received = 0;
      bc::dlmalloc_command_ret_t r = bc::dlmalloc_allocation_command
         ( allocation_type(BOOST_CONTAINER_ALLOCATE_NEW | BOOST_CONTAINER_EXPAND_FWD)
         , 1u, 1u, newsz, newsz, &received, p);
      if(!r.first)
         return 0;
      if(!r.second){          //a fresh block: move the contents across
         std::memcpy(r.first, p, oldsz);
         bc::dlmalloc_free(p);
      }
      return r.first;
   }

   static void alloc_many(std::size_t n_elements, std::size_t elem_size, void **out)
   {
      boost_cont_memchain chain;
      BOOST_CONTAINER_MEMCHAIN_INIT(&chain);
      if(!bc::dlmalloc_multialloc_nodes
            (n_elements, elem_size, BOOST_CONTAINER_DL_MULTIALLOC_DEFAULT_CONTIGUOUS, &chain)){
         for(std::size_t i = 0; i != n_elements; ++i)
            out[i] = 0;
         return;
      }
      boost_cont_memchain_it it = BOOST_CONTAINER_MEMCHAIN_BEGIN_IT(&chain);
      for(std::size_t i = 0; i != n_elements; ++i){
         out[i] = BOOST_CONTAINER_MEMIT_ADDR(it);
         BOOST_CONTAINER_MEMIT_NEXT(it);
      }
   }

   static void dealloc_many(std::size_t n_elements, void **out)
   {
      if(!n_elements || !out[0]){
         return;
      }
      boost_cont_memchain chain;
      BOOST_CONTAINER_MEMCHAIN_INIT(&chain);
      for(std::size_t i = n_elements; i--;)
         BOOST_CONTAINER_MEMCHAIN_PUSH_FRONT(&chain, out[i]);
      bc::dlmalloc_multidealloc(&chain);
   }
};

///////////////////////////////////////////////////////////////////////////////
// Operations. Each one returns a value derived from the work so escape() can
// keep it alive, and touches every block it obtains so the allocation is not
// merely a bookkeeping exercise.
///////////////////////////////////////////////////////////////////////////////
BOOST_CONTAINER_FORCEINLINE unsigned touch(void *p, std::size_t n)
{
   if(!p) return 0u;
   std::memset(p, 1, n < 64u ? n : 64u);
   escape(p);
   return unsigned(*static_cast<unsigned char*>(p));
}

//Allocate and immediately release: the pure hot-path cost of one malloc/free
//pair over a heap that stays in the same shape.
template<class P>
unsigned op_churn(std::size_t sz, std::size_t reps, void **, rng &)
{
   unsigned acc = 0;
   for(std::size_t i = 0; i != reps; ++i){
      void *p = P::alloc(sz);
      acc += touch(p, sz);
      P::dealloc(p);
   }
   return acc;
}

//Allocate a whole batch, then release it youngest-first.
template<class P>
unsigned op_batch_lifo(std::size_t sz, std::size_t n, void **buf, rng &)
{
   unsigned acc = 0;
   for(std::size_t i = 0; i != n; ++i){
      buf[i] = P::alloc(sz);
      acc += touch(buf[i], sz);
   }
   for(std::size_t i = n; i--;)
      P::dealloc(buf[i]);
   return acc;
}

//Same batch, released oldest-first: the order that fragments most allocators.
template<class P>
unsigned op_batch_fifo(std::size_t sz, std::size_t n, void **buf, rng &)
{
   unsigned acc = 0;
   for(std::size_t i = 0; i != n; ++i){
      buf[i] = P::alloc(sz);
      acc += touch(buf[i], sz);
   }
   for(std::size_t i = 0; i != n; ++i)
      P::dealloc(buf[i]);
   return acc;
}

//Steady state: a working set of live blocks where a random slot is freed and
//re-allocated with a random size. This is the shape real programs produce.
template<class P>
unsigned op_random_mix(std::size_t sz, std::size_t n, void **buf, rng &r)
{
   const std::size_t live = n / 4u ? n / 4u : 1u;
   unsigned acc = 0;
   for(std::size_t i = 0; i != live; ++i){
      buf[i] = P::alloc(sz);
      acc += touch(buf[i], sz);
   }
   for(std::size_t i = 0; i != n; ++i){
      const std::size_t slot = r.below(live);
      P::dealloc(buf[slot]);
      //Sizes wander around sz so the free lists actually get exercised.
      const std::size_t nsz = sz / 2u + r.below(sz);
      buf[slot] = P::alloc(nsz ? nsz : 1u);
      acc += touch(buf[slot], nsz);
   }
   for(std::size_t i = 0; i != live; ++i)
      P::dealloc(buf[i]);
   return acc;
}

//Vector-like growth: enlarge one live buffer repeatedly, keeping contents.
template<class P>
unsigned op_grow(std::size_t sz, std::size_t n, void **, rng &)
{
   unsigned acc = 0;
   const std::size_t steps = grow_steps(sz);
   for(std::size_t i = 0; i != n / steps + 1u; ++i){
      std::size_t cur = sz;
      void *p = P::alloc(cur);
      acc += touch(p, cur);
      for(std::size_t s = 0; s != steps; ++s){
         const std::size_t next = cur + cur / 2u + 1u;
         void *q = P::grow(p, cur, next);
         if(!q) break;
         p   = q;
         cur = next;
         acc += touch(p, cur);
      }
      P::dealloc(p);
   }
   return acc;
}

//Growth again, but with other live blocks allocated between the steps so the
//buffer usually cannot simply eat the space above it. op_grow is the best case
//for expansion in place; this is the case real programs hit far more often.
template<class P>
unsigned op_grow_frag(std::size_t sz, std::size_t n, void **buf, rng &)
{
   unsigned acc = 0;
   const std::size_t steps = grow_steps(sz);
   //Keep the interleaved blocks inside the same working-set budget as the
   //batch operations, so this row does not turn into a paging benchmark
   //at the largest sizes.
   const std::size_t live  = blocks_for(sz) < 32u ? blocks_for(sz) : 32u;
   for(std::size_t i = 0; i != live; ++i)
      buf[i] = P::alloc(sz);

   for(std::size_t i = 0; i != n / steps + 1u; ++i){
      std::size_t cur = sz;
      void *p = P::alloc(cur);
      acc += touch(p, cur);
      for(std::size_t s = 0; s != steps; ++s){
         //Put a fresh block right after the current one, then release an older
         //one: the neighbourhood keeps changing, as it does under real load.
         const std::size_t slot = (i*steps + s) % live;
         P::dealloc(buf[slot]);
         buf[slot] = P::alloc(sz);
         acc += touch(buf[slot], sz);

         const std::size_t next = cur + cur / 2u + 1u;
         void *q = P::grow(p, cur, next);
         if(!q) break;
         p   = q;
         cur = next;
         acc += touch(p, cur);
      }
      P::dealloc(p);
   }
   for(std::size_t i = 0; i != live; ++i)
      P::dealloc(buf[i]);
   return acc;
}

//Many equally sized nodes at once, then released at once.
template<class P>
unsigned op_multi_node(std::size_t sz, std::size_t n, void **buf, rng &)
{
   unsigned acc = 0;
   P::alloc_many(n, sz, buf);
   for(std::size_t i = 0; i != n; ++i)
      acc += touch(buf[i], sz);
   P::dealloc_many(n, buf);
   return acc;
}

///////////////////////////////////////////////////////////////////////////////
// Measurement, mirroring bench_vector_common.hpp: repeat until the trial budget
// is met, run several trials, drop the slowest third.
///////////////////////////////////////////////////////////////////////////////
template<class F>
BOOST_NOINLINE double measure(F f, std::size_t num_trials, nsec_t min_time_per_trial)
{
   if(!num_trials) num_trials = 1;
   bc::vector<double> trials(num_trials);
   for(std::size_t i = 0; i != num_trials; ++i){
      std::size_t runs = 0;
      const nsec_t t1 = boost::move_detail::nsec_clock();
      nsec_t t2;
      do{
         clobber();
         unsigned res = f();
         escape(&res);
         t2 = boost::move_detail::nsec_clock();
         ++runs;
      } while((t2 - t1) < min_time_per_trial);
      trials[i] = double(t2 - t1) / 1.0e9 / double(runs);
   }
   std::sort(trials.begin(), trials.end());
   const std::size_t ts         = trials.size();
   const std::size_t ts_discard = ts / 3u;   //drop the slowest
   double sum = 0.0;
   for(std::size_t i = ts_discard; i != ts; ++i)
      sum += trials[i];
   return sum / double(ts - ts_discard);
}

//Binds one (operation, policy) pair to the arguments measure() needs.
template<class P>
struct invoker
{
   typedef unsigned (*fn_t)(std::size_t, std::size_t, void **, rng &);

   fn_t        fn;
   std::size_t sz;
   std::size_t n;
   void      **buf;

   invoker(fn_t f, std::size_t s, std::size_t n_, void **b)
      : fn(f), sz(s), n(n_), buf(b)
   {}

   unsigned operator()() const
   {
      rng r(987654321u);      //identical request stream for every allocator
      return fn(sz, n, buf, r);
   }
};

///////////////////////////////////////////////////////////////////////////////
// Report: one row per (operation, size), one column per allocator, printed as
// the speedup over the first registered allocator: time(baseline)/time(column),
// so values above 1 mean the column is faster.
///////////////////////////////////////////////////////////////////////////////
inline bc::string bench_fmt2(double v)
{
   std::ostringstream o;
   o << std::fixed << std::setprecision(2) << v;
   return bc::string(o.str().c_str());
}

class report
{
   struct row
   {
      bc::string          op;
      std::size_t         size;
      bc::vector<double>  sec;
   };

   bc::vector<bc::string> cols_;
   bc::vector<row>        rows_;

   static const int op_w   = 14;
   static const int size_w = 11;
   static const int col_w  = 12;

   //Geometric mean of the speedups of column c over the rows [first, last).
   double column_geomean_range(std::size_t c, std::size_t first, std::size_t last) const
   {
      double acc = 0.0;
      std::size_t cnt = 0;
      for(std::size_t i = first; i != last; ++i){
         const double base = rows_[i].sec[0];
         const double t    = rows_[i].sec[c];
         if(base > 0.0 && t > 0.0){
            acc += std::log(base / t);
            ++cnt;
         }
      }
      return cnt ? std::exp(acc / double(cnt)) : 0.0;
   }

   //Same, but over every row measured at one block size (across operations).
   double column_geomean_size(std::size_t c, std::size_t size) const
   {
      double acc = 0.0;
      std::size_t cnt = 0;
      for(std::size_t i = 0; i != rows_.size(); ++i){
         if(rows_[i].size != size)
            continue;
         const double base = rows_[i].sec[0];
         const double t    = rows_[i].sec[c];
         if(base > 0.0 && t > 0.0){
            acc += std::log(base / t);
            ++cnt;
         }
      }
      return cnt ? std::exp(acc / double(cnt)) : 0.0;
   }

   double column_geomean(std::size_t c) const
   {  return column_geomean_range(c, 0u, rows_.size());  }

   //Geomean over every row of one operation (across the size sweep).
   double column_geomean_op(std::size_t c, const bc::string &op) const
   {
      double acc = 0.0;
      std::size_t cnt = 0;
      for(std::size_t i = 0; i != rows_.size(); ++i){
         if(rows_[i].op != op)
            continue;
         const double base = rows_[i].sec[0];
         const double t    = rows_[i].sec[c];
         if(base > 0.0 && t > 0.0){
            acc += std::log(base / t);
            ++cnt;
         }
      }
      return cnt ? std::exp(acc / double(cnt)) : 0.0;
   }

   //Operation names in the order they were first measured.
   void collect_ops(bc::vector<bc::string> &out) const
   {
      for(std::size_t i = 0; i != rows_.size(); ++i){
         bool seen = false;
         for(std::size_t k = 0; k != out.size() && !seen; ++k)
            seen = (out[k] == rows_[i].op);
         if(!seen)
            out.push_back(rows_[i].op);
      }
   }

   //Block sizes in the order they were first measured.
   void collect_sizes(bc::vector<std::size_t> &out) const
   {
      for(std::size_t i = 0; i != rows_.size(); ++i){
         bool seen = false;
         for(std::size_t k = 0; k != out.size() && !seen; ++k)
            seen = (out[k] == rows_[i].size);
         if(!seen)
            out.push_back(rows_[i].size);
      }
   }

   void print_summary_line(const char *label, const char *size_txt
                          , bc::vector<double> &vals) const
   {
      std::cout << std::left << std::setw(op_w) << label
                << std::right << std::setw(size_w) << size_txt;
      for(std::size_t c = 0; c != vals.size(); ++c)
         std::cout << std::setw(col_w) << bench_fmt2(vals[c]).c_str();
      std::cout << "\n";
   }

   public:
   void add_column(const char *name)
   {  cols_.push_back(bc::string(name));  }

   void add_row(const char *op, std::size_t size, const bc::vector<double> &sec)
   {
      row r;
      r.op   = bc::string(op);
      r.size = size;
      r.sec  = sec;
      rows_.push_back(r);
   }

   void print() const
   {
      if(cols_.empty() || rows_.empty())
         return;

      const int printed = int(cols_.size()) - 1;
      const int line_w  = op_w + size_w + col_w * printed;

      std::cout << "\n" << bc::string(45, '=') << "\n"
                << "dlmalloc vs the platform allocator\n"
                << "speedup over '" << cols_[0]
                << "': >1 means the column is faster\n"
                << bc::string(45, '=') << "\n";

      std::cout << std::left << std::setw(op_w) << "operation"
                << std::right << std::setw(size_w) << "size";
      for(std::size_t c = 1; c != cols_.size(); ++c)
         std::cout << std::setw(col_w) << cols_[c].c_str();
      std::cout << "\n";

      //Data rows, grouped per operation. The per-operation geomeans are
      //collected in their own summary block below, next to the per-size ones.
      for(std::size_t i = 0; i != rows_.size(); ++i){
         const bool last_of_group =
            (i + 1u == rows_.size()) || (rows_[i + 1u].op != rows_[i].op);

         std::ostringstream se;
         se << rows_[i].size;
         std::cout << std::left << std::setw(op_w) << rows_[i].op.c_str()
                   << std::right << std::setw(size_w) << se.str();
         const double base = rows_[i].sec[0];
         for(std::size_t c = 1; c != cols_.size(); ++c){
            const double t = rows_[i].sec[c];
            const double speedup = (base > 0.0 && t > 0.0) ? base / t : 0.0;
            std::cout << std::setw(col_w) << bench_fmt2(speedup).c_str();
         }
         std::cout << "\n";

         if(last_of_group && i + 1u != rows_.size())
            std::cout << "\n";
      }

      //Per-operation geomean: one line per operation, across every size.
      std::cout << bc::string(bc::string::size_type(line_w), '-') << "\n";
      bc::vector<bc::string> ops;
      collect_ops(ops);
      for(std::size_t o = 0; o != ops.size(); ++o){
         bc::vector<double> vals;
         for(std::size_t c = 1; c != cols_.size(); ++c)
            vals.push_back(column_geomean_op(c, ops[o]));
         print_summary_line(o ? "" : "geomean/op", ops[o].c_str(), vals);
      }

      //Per-size geomean: one line per block size, across every operation.
      std::cout << bc::string(bc::string::size_type(line_w), '-') << "\n";
      bc::vector<std::size_t> sizes;
      collect_sizes(sizes);
      for(std::size_t s = 0; s != sizes.size(); ++s){
         std::ostringstream se;
         se << sizes[s];
         bc::vector<double> vals;
         for(std::size_t c = 1; c != cols_.size(); ++c)
            vals.push_back(column_geomean_size(c, sizes[s]));
         print_summary_line(s ? "" : "geomean/size", se.str().c_str(), vals);
      }

      std::cout << bc::string(bc::string::size_type(line_w), '-') << "\n";
      bc::vector<double> all;
      for(std::size_t c = 1; c != cols_.size(); ++c)
         all.push_back(column_geomean(c));
      print_summary_line("geomean (all)", "", all);
   }
};

///////////////////////////////////////////////////////////////////////////////
// Driver
///////////////////////////////////////////////////////////////////////////////
struct op_desc
{
   const char *name;
   unsigned  (*malloc_fn)(std::size_t, std::size_t, void **, rng &);
   unsigned  (*dlmalloc_fn)(std::size_t, std::size_t, void **, rng &);
   bool        churn;   //true: the "n" argument means repetitions, not blocks
};

//Give each allocator a heap that has already grown, so the measurement is not
//dominated by whichever one first pays for fresh pages from the OS.
template<class P>
void warm_up(std::size_t max_blocks, void **buf)
{
   for(std::size_t s = 0; s != bench_defaults::num_sizes; ++s){
      const std::size_t sz = bench_size(s);
      for(std::size_t i = 0; i != max_blocks; ++i)
         buf[i] = P::alloc(sz);
      for(std::size_t i = max_blocks; i--;)
         P::dealloc(buf[i]);
   }
}

int main()
{
   const std::size_t n   = bench_defaults::batch_elements;
   const std::size_t rep = bench_defaults::churn_reps;

   bc::vector<void *> buf(n + 1u);

   warm_up<std_malloc_policy>(n, &buf[0]);
   warm_up<dlmalloc_policy>(n, &buf[0]);

   static const op_desc ops[] =
   { { "churn",      &op_churn<std_malloc_policy>,      &op_churn<dlmalloc_policy>,      true  }
   , { "batch_lifo", &op_batch_lifo<std_malloc_policy>, &op_batch_lifo<dlmalloc_policy>, false }
   , { "batch_fifo", &op_batch_fifo<std_malloc_policy>, &op_batch_fifo<dlmalloc_policy>, false }
   , { "random_mix", &op_random_mix<std_malloc_policy>, &op_random_mix<dlmalloc_policy>, false }
   , { "grow",       &op_grow<std_malloc_policy>,       &op_grow<dlmalloc_policy>,       false }
   , { "grow_frag",  &op_grow_frag<std_malloc_policy>,  &op_grow_frag<dlmalloc_policy>,  false }
   , { "multi_node", &op_multi_node<std_malloc_policy>, &op_multi_node<dlmalloc_policy>, false }
   };
   const std::size_t num_ops = sizeof(ops)/sizeof(ops[0]);

   report rep_out;
   rep_out.add_column(std_malloc_policy::name());     //baseline / denominator
   rep_out.add_column(dlmalloc_policy::name());

   std::cout << "Boost.Container dlmalloc benchmark"
#if defined(LONG_BENCH)
             << " (LONG_BENCH)"
#else
             << " (short run; rebuild with -DLONG_BENCH for the full sweep)"
#endif
             << std::endl;

   for(std::size_t o = 0; o != num_ops; ++o){
      for(std::size_t s = 0; s != bench_defaults::num_sizes; ++s){
         const std::size_t sz    = bench_size(s);
         const std::size_t count = ops[o].churn ? rep : blocks_for(sz);

         bc::vector<double> sec(2u);
         sec[0] = measure(invoker<std_malloc_policy>
                     (ops[o].malloc_fn, sz, count, &buf[0])
                   , bench_defaults::num_trials, bench_defaults::min_time_per_trial);
         sec[1] = measure(invoker<dlmalloc_policy>
                     (ops[o].dlmalloc_fn, sz, count, &buf[0])
                   , bench_defaults::num_trials, bench_defaults::min_time_per_trial);
         rep_out.add_row(ops[o].name, sz, sec);
      }
   }

   rep_out.print();

   //A benchmark that leaked would look faster than it is: make sure every
   //dlmalloc block taken above went back to the heap.
   if(!bc::dlmalloc_all_deallocated()){
      std::cout << "\nWARNING: the dlmalloc heap still holds "
                << bc::dlmalloc_in_use_memory()
                << " bytes; these timings are not comparable." << std::endl;
      return 1;
   }
   return 0;
}

#include <boost/container/detail/config_end.hpp>
