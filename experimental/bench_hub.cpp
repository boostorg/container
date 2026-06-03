/* Benchmark of boost::container::hub against plf::hive.
 * 
 * Copyright 2026 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>

#if BOOST_CXX_VERSION < 202002L

int main() { return 0; }

#else

#include <algorithm>
#include <array>
#include <numeric>
#include <iostream>
#include <boost/move/detail/nsec_clock.hpp>
#include "../bench/bench_utils.hpp"

#include <cstddef>
#include <utility>

//Element sizes (in bytes) benchmarked. Each size produces its own table.
//Sizes must be >= sizeof(int); a size that is not a multiple of the int
//alignment is rounded up by the compiler, and the printed sizeof(element)
//reflects the actual (possibly padded) size.
#ifndef ELEMENT_SIZES
//#define ELEMENT_SIZES { 16, 32, 64, 96, 128, 192, 256 }
//#define ELEMENT_SIZES { 16, 64, 128, 256 }
//#define ELEMENT_SIZES { 64, 80 }
#define ELEMENT_SIZES { 64 }
#endif
inline constexpr std::size_t element_sizes[] = ELEMENT_SIZES;
inline constexpr std::size_t element_sizes_count =
   sizeof(element_sizes) / sizeof(element_sizes[0]);

#define NONTRIVIAL_ELEMENT

//Wall-clock measured in nanoseconds via boost::move_detail::nsec_clock().
//measure_start is advanced forward by resume_timing() so that
//(now - measure_start) yields measured (non-paused) time.
boost::move_detail::nanosecond_type measure_start, measure_pause;

template<typename F>
BOOST_NOINLINE double measure(F f)
{
   typedef boost::move_detail::nanosecond_type nsec_t;

   #ifdef NDEBUG
   //static const std::size_t num_trials = 10;
   //static const nsec_t      min_time_per_trial = 150*1000000u; //150 ms
   //static const std::size_t num_trials = 8;
   //static const nsec_t      min_time_per_trial = 100*1000000u; // ms
   static const std::size_t num_trials = 1;
   static const nsec_t      min_time_per_trial = 0u;
   #else
   static const std::size_t num_trials = 1;
   static const nsec_t      min_time_per_trial = 0u;
   #endif

   std::array<double,num_trials> trials;

   for(std::size_t i = 0; i < num_trials; ++i) {
      int             runs = 0;
      nsec_t          t1;
      nsec_t          t2;
      decltype(f())   res;

      measure_start = t1 = boost::move_detail::nsec_clock();
      do{
         clobber();
         res = f();
         escape(&res);
         t2 = boost::move_detail::nsec_clock();
         ++runs;
      }while((t2 - t1) < min_time_per_trial);
      trials[i] = double(t2 - measure_start) / 1.0e9 / runs;
   }
   std::sort(trials.begin(), trials.end());

   const std::size_t ts = trials.size();
   const std::size_t ts_discard = ts/4;
   return std::accumulate(trials.begin() + ts_discard, trials.end(), 0.0)/(ts - ts_discard);
}

BOOST_CONTAINER_FORCEINLINE void pause_timing()
{
   measure_pause = boost::move_detail::nsec_clock();
}

BOOST_CONTAINER_FORCEINLINE void resume_timing()
{
   measure_start += boost::move_detail::nsec_clock() - measure_pause;
}

#include <boost/container/experimental/hub.hpp>
#include <boost/container/experimental/nest.hpp>
#include <boost/core/detail/splitmix64.hpp>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

//#define PLF_HIVE_BENCH
#if defined(PLF_HIVE_BENCH)
#include "plf_hive.h"
#endif

template<std::size_t Size>
struct element_t
{
#if defined(NONTRIVIAL_ELEMENT)
   element_t(int n_) : n{ n_ }
   {
      std::memset(payload, 0, sizeof(payload));
   }

   ~element_t()
   {
      std::memset(payload, 0, sizeof(payload));
   }

   element_t(element_t&& x): n{x.n}
   {
      std::memcpy(payload, x.payload, sizeof(payload));
      std::memset(x.payload, 0, sizeof(payload));
   }

   element_t& operator=(element_t&& x)
   {
      n = x.n;
      std::memcpy(payload, x.payload, sizeof(payload));
      std::memset(x.payload, 0, sizeof(payload));
      return *this;
   }
#else
   element_t(int n_) : n{ n_ }
   {}
#endif

   operator int() const { return n; }

   int n;
   char payload[Size - sizeof(int)];
};

struct urbg
{
   using result_type = boost::uint64_t;

   static constexpr result_type min() { return 0; }
   static constexpr result_type max() { return (result_type)(-1); }

   urbg() = default;
   explicit urbg(result_type seed): rng{seed} {}

   result_type operator()() { return rng(); }

   boost::detail::splitmix64 rng;
};

template<typename Container, typename Iterator>
BOOST_CONTAINER_FORCEINLINE void erase_void(Container& x, Iterator it)
{
   x.erase(it);
}

template<typename... Args, typename Iterator>
BOOST_CONTAINER_FORCEINLINE void erase_void(boost::container::hub<Args...>& x, Iterator it)
{
   x.erase_void(it);
}

template<typename... Args, typename Iterator>
BOOST_CONTAINER_FORCEINLINE void erase_void(boost::container::nest<Args...>& x, Iterator it)
{
   x.erase_void(it);
}

template<typename Container>
Container make(std::size_t n, double erasure_rate)
{
   std::uint64_t erasure_cut =
      (std::uint64_t)(erasure_rate * (double)(std::uint64_t)(-1));

   Container                                 c;
   urbg                                      rng;
   std::vector<typename Container::iterator> iterators;

   iterators.reserve(n);
   for(std::size_t i = 0; i < n; ++i) iterators.push_back(c.insert((int)rng()));
   std::shuffle(iterators.begin(), iterators.end(), rng);
   for(auto it: iterators) {
      if(rng() < erasure_cut) erase_void(c, it);
   }
   return c;
}

template<typename Container>
void fill(Container& c, std::size_t n)
{
   urbg rng;
   if(n > c.size()) {
      n -= c.size();
      while(n--) c.insert((int)rng());
   }
}

/*
static std::size_t min_size_exp = 3,
                   max_size_exp = 7;
static double      min_erasure_rate = 0.0,
                   max_erasure_rate = 0.9,
                   erase_rate_inc = 0.1;
*/

static std::size_t min_size_exp = 3,
                   max_size_exp = 6
;
static double      min_erasure_rate = 0.1,
                   max_erasure_rate = 0.9,
                   erase_rate_inc = 0.4;

struct benchmark_result
{
   std::string                           title;
   std::vector<std::vector<std::string>> data;
   std::vector<std::vector<double>>      ratios;
};

double geomean(const benchmark_result& bench);  // defined further below

template<typename FNum, typename FDen>
benchmark_result benchmark(const char* title, std::size_t element_size, FNum fnum, FDen fden)
{
   static constexpr std::size_t size_limit =
      sizeof(std::size_t) == 4?
#if defined(BOOST_MSVC) && defined(_M_IX86)
                                600ull * 1024ull * 1024ull:
#else
                                800ull * 1024ull * 1024ull:
#endif
                                2048ull * 1024ull * 1024ull;

   benchmark_result res = {title, {}, {}};

   char current_fill = std::cout.fill();
   std::cout << std::setfill('-') << std::setw(41) << "" <<"\n"
             << title << "\n"
             << "sizeof(element): " << element_size << "\n"
             << std::setfill(current_fill);
   std::cout << std::left << std::setw(11) << "" << "container size\n" << std::right
             << std::left << std::setw(11) << "erase rate" << std::right;
   for(std::size_t i = min_size_exp; i <= max_size_exp; ++i)
   {
      std::cout << "1.E" << i << " ";
   }
   std::cout << " mean" << std::endl;

   for(double erasure_rate = min_erasure_rate;
              erasure_rate <= max_erasure_rate;
              erasure_rate += erase_rate_inc) {
      //Print the erase rate in a self-contained format: the row-geomean
      //and final-geomean prints below leave std::cout in std::fixed with a
      //sticky precision, which would otherwise bleed into this column on
      //later rows/tables (e.g. "0.50"/"0.100"). Reset to defaultfloat and
      //round away the += accumulation noise so it always shows e.g. 0.1.
      std::cout << std::left << std::setw(11) << std::defaultfloat << std::setprecision(6)
                << (std::round(erasure_rate * 1000.0) / 1000.0)
                << std::right << std::flush;

      res.data.push_back({});
      res.ratios.push_back({});

      for(std::size_t i = min_size_exp; i <= max_size_exp; ++i) {
         std::ostringstream out;
         std::size_t        n = (std::size_t)std::pow(10.0, (double)i);
         if(n * element_size > size_limit) {
            out << "----";
         }
         else{
            auto tnum = measure([&] { return fnum(n, erasure_rate); });
            auto tden  = measure([&] { return fden(n, erasure_rate); });
            double ratio = tnum / tden;
            out << std::fixed << std::setprecision(2) << ratio;
            res.ratios.back().push_back(ratio);
         }
         std::cout << out.str() << " " << std::flush;
         res.data.back().push_back(out.str());
      }
      {  //Per-row geomean across the container sizes for this erase rate.
         double      row_log_sum = 0.0;
         std::size_t row_count   = 0;
         for(double r: res.ratios.back()) {
            if(r > 0.0) { row_log_sum += std::log(r); ++row_count; }
         }
         double row_geomean = row_count ? std::exp(row_log_sum / (double)row_count) : 0.0;
         std::cout << std::fixed << std::setprecision(2) << row_geomean;
      }
      std::cout << std::endl;
   }
   std::cout << std::left << std::setw(11) << "geomean"
             << std::right << std::fixed << std::setprecision(2)
             << geomean(res) << "\n";
   return res;
}

template<typename Container>
struct create_fill
{
   unsigned int operator()(std::size_t n, double erasure_rate) const
   {
      unsigned int res = 0;
      {
         auto c = make<Container>(n, erasure_rate);
         fill(c, n);
         res = (unsigned int)c.size();
         pause_timing();
      }
      resume_timing();
      return res;
   }
};

template<typename Container>
struct create_fill_and_destroy
{
   unsigned int operator()(std::size_t n, double erasure_rate) const
   {
      auto c = make<Container>(n, erasure_rate);
      fill(c, n);
      return (unsigned int)c.size();
   }
};

//Isolates the cost of make<Container>() alone (the insert + shuffle +
//random-erase build), excluding the subsequent destruction.
template<typename Container>
struct creation
{
   unsigned int operator()(std::size_t n, double erasure_rate) const
   {
      unsigned int res = 0;
      {
         auto c = make<Container>(n, erasure_rate);   // measured
         res = (unsigned int)c.size();
         pause_timing();                              // exclude destruction
      }
      resume_timing();
      return res;
   }
};

//Isolates the cost of fill() alone (re-inserting up to n elements into an
//already-built, partially-erased container), excluding make and destruction.
template<typename Container>
struct filling
{
   unsigned int operator()(std::size_t n, double erasure_rate) const
   {
      unsigned int res = 0;
      {
         pause_timing();
         auto c = make<Container>(n, erasure_rate);   // excluded
         resume_timing();
         fill(c, n);                                  // measured
         res = (unsigned int)c.size();
         pause_timing();                              // exclude destruction
      }
      resume_timing();
      return res;
   }
};

template<typename Container>
struct erasure
{
   unsigned int operator()(std::size_t n, double erasure_rate) const
   {
      unsigned int res = 0;
      {
         pause_timing();
         std::uint64_t erasure_cut =
            (std::uint64_t)(erasure_rate * (double)(std::uint64_t)(-1));

         Container                                 c;
         urbg                                      rng;
         std::vector<typename Container::iterator> iterators;

         iterators.reserve(n);
         for (std::size_t i = 0; i < n; ++i) iterators.push_back(c.insert((int)rng()));
         std::shuffle(iterators.begin(), iterators.end(), rng);
         resume_timing();

         for (auto it : iterators) {
            if (rng() < erasure_cut) erase_void(c, it);
         }
         pause_timing();
         res = (unsigned)c.size();
      }
      return res;
   }
};

//Isolates the cost of the container destructor alone, over a fully built and
//filled container, excluding make and fill.
template<typename Container>
struct destruction
{
   unsigned int operator()(std::size_t n, double erasure_rate) const
   {
      unsigned int res = 0;
      {
         pause_timing();
         auto c = make<Container>(n, erasure_rate);   // excluded
         res = (unsigned int)c.size();
         resume_timing();                             // measure only the dtor below
      }                                               // ~Container() measured here
      return res;
   }
};

template<typename Container>
struct prepare
{
   const Container& get_container(std::size_t n_arg, double erasure_rate_arg)
   {
      if(n_arg != n_ || erasure_rate_arg != erasure_rate_) {
         pause_timing();
         n_ = n_arg;
         erasure_rate_ = erasure_rate_arg;
         c.clear();
         c.shrink_to_fit();
         c = make<Container>(n_, erasure_rate_);
         resume_timing();
      }
      return c;
   }

   std::size_t n_ = 0;
   double      erasure_rate_ = 0.0;
   Container   c;
};

template<typename Container>
struct iteration: prepare<Container>
{
   unsigned int operator()(std::size_t n, double erasure_rate)
   {
      unsigned int res = 0;
      auto& cr = this->get_container(n, erasure_rate);
      for(const auto& x: cr) res += (unsigned int)x;
      return res;
   }
};

template<typename Container>
struct for_each: prepare<Container>
{
   unsigned int operator()(std::size_t n, double erasure_rate)
   {
      unsigned int res = 0;
      auto& cr = this->get_container(n, erasure_rate);
      boost::container::for_each(cr, [&] (const auto& x) { res += (unsigned int)x; });
      return res;
   }
};

#if defined(PLF_HIVE_BENCH)

template<typename Element>
struct for_each <plf::hive<Element>>
   : iteration< plf::hive<Element> >
{};

#endif

template<typename Container>
struct sort
{
   unsigned int operator()(std::size_t n, double erasure_rate) const
   {
      pause_timing();
      auto c = make<Container>(n, erasure_rate);
      resume_timing();
      c.sort();
      return (unsigned int)c.size();
   }
};

using table = std::vector<benchmark_result>;

double geomean(const table& t)
{
   double log_sum = 0.0;
   std::size_t count = 0;
   for(const auto& bench: t) {
      for(const auto& row: bench.ratios) {
         for(double r: row) {
            if(r > 0.0) { log_sum += std::log(r); ++count; }
         }
      }
   }
   return count > 0 ? std::exp(log_sum / (double)count) : 0.0;
}

double geomean(const benchmark_result& bench)
{
   double log_sum = 0.0;
   std::size_t count = 0;
   for(const auto& row: bench.ratios) {
      for(double r: row) {
         if(r > 0.0) { log_sum += std::log(r); ++count; }
      }
   }
   return count > 0 ? std::exp(log_sum / (double)count) : 0.0;
}


//Per-execution (single element size) result summary: the geomean of each
//individual test plus the overall geomean across all tests.
struct run_summary
{
   std::vector<std::pair<std::string, double> > per_test;
   double                                       overall;
};

//Runs the full benchmark suite for a single element size and writes its
//own table file (hub_test_<size>.txt). element_t<Size> is the value type.
template<std::size_t Size>
run_summary run_bench()
{
   using namespace boost::container;
   using element = element_t<Size>;

   #ifdef PLF_HIVE_BENCH
   using num = plf::hive<element>;
   #else
   using num = hub<element>;
   //using num  = nest<element>;
   //using num = nest<element, void, nest_options_t< store_data_in_block<true> > >;
   //using num = nest<element, void, nest_options_t< prefetch<false> > >;
   //using num = nest<element, void, nest_options_t< prefetch<false>, store_data_in_block<true> > >;
   #endif
   using den  = nest<element>;
   //using den  = hub<element>;
   //using den = nest<element, void, nest_options_t< prefetch<false> > >;
   //using den = nest<element, void, nest_options_t< store_data_in_block<true> > >;

   const std::size_t element_size = sizeof(element);

   char current_fill = std::cout.fill();
   std::cout << "\n" << std::setfill('=') << std::setw(41) << "" << "\n"
             << "ELEMENT SIZE: " << element_size << " bytes\n"
             << std::setw(41)  << "\n"
             << std::setfill(current_fill);

   table t;/*
   t.push_back(benchmark(
      "iteration", element_size,
      ::iteration<num>{}, ::iteration<den>{}));
   t.push_back(benchmark(
      "for_each", element_size,
      ::for_each<num>{}, ::for_each<den>{}));
   t.push_back(benchmark(
      "sort", element_size,
      sort<num>{}, sort<den>{}));
   t.push_back(benchmark(
      "create, fill", element_size,
      create_fill<num>{}, create_fill<den>{}));
   t.push_back(benchmark(
      "create, fill, destroy", element_size,
      create_fill_and_destroy<num>{}, create_fill_and_destroy<den>{}));
   t.push_back(benchmark(
      "destroy (dtor)", element_size,
      destruction<num>{}, destruction<den>{}));
   t.push_back(benchmark(
      "creation (make)", element_size,
      creation<num>{}, creation<den>{}));*/
   t.push_back(benchmark(
      "filling", element_size,
      filling<num>{}, filling<den>{}));/*
   t.push_back(benchmark(
      "erasure", element_size,
      ::erasure<num>{}, ::erasure<den>{}));*/

      std::cout << "\n" << std::setfill('-') << std::setw(41) << "" "\n"
             << "Geometric means (num/den time ratio), element size "
             << element_size << "\n";
   std::cout << std::setfill(current_fill);

   run_summary summary;
   for(const auto& bench: t) {
      const double g = geomean(bench);
      summary.per_test.push_back(std::make_pair(bench.title, g));
      std::cout << std::left << std::setw(30) << bench.title
                << std::fixed << std::setprecision(2) << g << "\n";
   }
   summary.overall = geomean(t);
   std::cout << std::left << std::setw(30) << "OVERALL"
             << std::fixed << std::setprecision(2) << summary.overall << "\n";
   return summary;
}

//Geometric mean of a set of (positive) ratios.
inline double geomean_of(const std::vector<double>& v)
{
   double log_sum = 0.0;
   std::size_t count = 0;
   for(double r: v) {
      if(r > 0.0) { log_sum += std::log(r); ++count; }
   }
   return count > 0 ? std::exp(log_sum / (double)count) : 0.0;
}

template<std::size_t... Is>
void run_all(std::index_sequence<Is...>)
{
   //Collect each execution's per-test and overall geomeans, then report
   //the geomean of each test across all executions, followed by the
   //geomean of the per-execution overall geomeans.
   const run_summary summaries[] = { run_bench<element_sizes[Is]>()... };
   const std::size_t num_exec = sizeof...(Is);

   char current_fill = std::cout.fill();
   std::cout << "\n" << std::setfill('=') << std::setw(41) << "" << "\n"
             << "Aggregated geometric means across all " << num_exec
             << " executions (num/den time ratio)\n"
             << std::setfill(current_fill);

   //Per-test geomean across executions (test set is identical per execution).
   const std::size_t num_tests = summaries[0].per_test.size();
   for(std::size_t ti = 0; ti < num_tests; ++ti) {
      std::vector<double> vals;
      for(std::size_t e = 0; e < num_exec; ++e)
         vals.push_back(summaries[e].per_test[ti].second);
      std::cout << std::left << std::setw(30) << summaries[0].per_test[ti].first
                << std::fixed << std::setprecision(2) << geomean_of(vals) << "\n";
   }

   //Geomean of the per-execution overall geomeans.
   std::vector<double> overalls;
   for(std::size_t e = 0; e < num_exec; ++e)
      overalls.push_back(summaries[e].overall);
   std::cout << std::left << std::setw(30) << "GEOMEAN OF GEOMEANS"
             << std::fixed << std::setprecision(2) << geomean_of(overalls) << "\n";
}
   
int main(int argc,char* argv[])
{
   (void)argc;
   (void)argv;

   BOOST_CONTAINER_TRY{
      run_all(std::make_index_sequence<element_sizes_count>{});
   }
   BOOST_CONTAINER_CATCH(const std::exception& e) {
      #ifndef BOOST_NO_EXCEPTIONS
      std::cerr << e.what() << std::endl;
      #endif
   }
   BOOST_CONTAINER_CATCH_END
}

#endif
