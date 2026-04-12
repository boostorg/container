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
#include <chrono>
#include <numeric>
#include <iostream>
#include "../bench/bench_utils.hpp"

#ifndef ELEMENT_SIZE
#define ELEMENT_SIZE 64
//#define ELEMENT_SIZE 16
#endif
#define NONTRIVIAL_ELEMENT

std::chrono::high_resolution_clock::time_point measure_start, measure_pause;

template<typename F>
double measure(F f)
{
   using namespace std::chrono;

   //static const int              num_trials = 10;
   //static const milliseconds     min_time_per_trial(200);

   static const std::size_t      num_trials = 5;
   static const milliseconds     min_time_per_trial(100);

   std::array<double,num_trials> trials;

   for(std::size_t i = 0; i < num_trials; ++i) {
      int                               runs = 0;
      high_resolution_clock::time_point t2;
      decltype(f())                     res;

      measure_start = high_resolution_clock::now();
      do{
         clobber();
         res = f();
         escape(&res);
         t2 = high_resolution_clock::now();
         ++runs;
      }while((t2 - measure_start) < min_time_per_trial);
      trials[i] =
         duration_cast<duration<double>>(t2 - measure_start).count() / runs;
   }
   std::sort(trials.begin(), trials.end());

   return std::accumulate(trials.begin() + 2, trials.end() - 2, 0.0)/(trials.size() - 4);
}

BOOST_CONTAINER_FORCEINLINE void pause_timing()
{
   measure_pause = std::chrono::high_resolution_clock::now();
}

BOOST_CONTAINER_FORCEINLINE void resume_timing()
{
   measure_start += std::chrono::high_resolution_clock::now() - measure_pause;
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

struct element
{
   element(int n_) : n{ n_ }
   {}

#if defined(NONTRIVIAL_ELEMENT)
   ~element()
   {
      std::memset(payload, 0, sizeof(payload));
   }

   element(element&& x): n{x.n}
   {
      std::memcpy(payload, x.payload, sizeof(payload));
      std::memset(x.payload, 0, sizeof(payload));
   }

   element& operator=(element&& x)
   {
      n = x.n;
      std::memcpy(payload, x.payload, sizeof(payload));
      std::memset(x.payload, 0, sizeof(payload));
      return *this;
   }
#endif

   operator int() const { return n; }

   int n;
   char payload[ELEMENT_SIZE - sizeof(int)];
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
void erase_void(Container& x, Iterator it)
{
   x.erase(it);
}

template<typename... Args, typename Iterator>
void erase_void(boost::container::hub<Args...>& x, Iterator it)
{
   x.erase_void(it);
}

template<typename... Args, typename Iterator>
void erase_void(boost::container::nest<Args...>& x, Iterator it)
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
                   max_size_exp = 5;
static double      min_erasure_rate = 0.1,
                   max_erasure_rate = 0.9,
                   erase_rate_inc = 0.4;

struct benchmark_result
{
   std::string                           title;
   std::vector<std::vector<std::string>> data;
   std::vector<std::vector<double>>      ratios;
};

template<typename FNum, typename FDen>
benchmark_result benchmark(const char* title, FNum fnum, FDen fden)
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

   std::cout << std::string(41, '-') << "\n"
             << title << "\n"
             << "sizeof(element): " << sizeof(element) << "\n";
   std::cout << std::left << std::setw(11) << "" << "container size\n" << std::right
             << std::left << std::setw(11) << "erase rate" << std::right;
   for(std::size_t i = min_size_exp; i <= max_size_exp; ++i)
   {
      std::cout << "1.E" << i << " ";
   }
   std::cout << std::endl;

   for(double erasure_rate = min_erasure_rate;
              erasure_rate <= max_erasure_rate;
              erasure_rate += erase_rate_inc) {
      std::cout << std::left << std::setw(11) << erasure_rate << std::right << std::flush;

      res.data.push_back({});
      res.ratios.push_back({});

      for(std::size_t i = min_size_exp; i <= max_size_exp; ++i) {
         std::ostringstream out;
         std::size_t        n = (std::size_t)std::pow(10.0, (double)i);
         if(n * sizeof(element) > size_limit) {
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
      std::cout << std::endl;
   }
   return res;
}

template<typename Container>
struct create
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
struct create_and_destroy
{
   unsigned int operator()(std::size_t n, double erasure_rate) const
   {
      auto c = make<Container>(n, erasure_rate);
      fill(c, n);
      return (unsigned int)c.size();
   }
};

template<typename Container>
struct prepare
{
   const Container& get_container(std::size_t n_, double erasure_rate_)
   {
      if(n_ != n || erasure_rate_ != erasure_rate) {
         pause_timing();
         n = n_;
         erasure_rate = erasure_rate_;
         c.clear();
         c.shrink_to_fit();
         c = make<Container>(n, erasure_rate);
         resume_timing();
      }
      return c;
   }

   std::size_t n = 0;
   double      erasure_rate = 0.0;
   Container   c;
};

template<typename Container>
struct for_each: prepare<Container>
{
   unsigned int operator()(std::size_t n, double erasure_rate)
   {
      unsigned int res = 0;
      auto& c = this->get_container(n, erasure_rate);
      for(const auto& x: c) res += (unsigned int)x;
      return res;
   }
};

template<typename Container>
struct visit_all: prepare<Container>
{
   unsigned int operator()(std::size_t n, double erasure_rate)
   {
      unsigned int res = 0;
      auto& c = this->get_container(n, erasure_rate);
      c.visit_all([&] (const auto& x) { res += (unsigned int)x; });
      return res;
   }
};

#if defined(PLF_HIVE_BENCH)

template<typename Element>
struct visit_all <plf::hive<Element>>
   : for_each< plf::hive<Element> >
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

void write_table(const table& t, const char* filename)
{
   static std::size_t first_column_width = 11;
   static std::size_t data_column_width = (max_size_exp + 1 - min_size_exp) * 5;
   std::size_t        num_data_columns = t.size();
   std::size_t        table_width = first_column_width + 2 + num_data_columns * (data_column_width + 2) + 1;

   std::ofstream fout(filename);
   std::ostream &out = fout;
   //(void)filename;
   //std::ostream &out = std::cout;

   auto data_horizontal_line =
      std::string(first_column_width + 2, ' ') + std::string(table_width - first_column_width - 2, '-');
   auto table_horizontal_line = std::string(table_width, '-');

   out << std::left;

   out << data_horizontal_line << "\n";

   out << "  " << std::setw(static_cast<int>(first_column_width)) << " ";
   out << std::setw(static_cast<int>(table_width - first_column_width - 3))
       << std::string("| sizeof(element): ") + std::to_string(ELEMENT_SIZE) << "|\n";

   out << data_horizontal_line << "\n";

   out << "  " << std::setw(static_cast<int>(first_column_width)) << " ";
   for(const benchmark_result& res: t) {
      out << "| " << std::setw(static_cast<int>(data_column_width)) << res.title;
   }
   out << "|\n";

   out << data_horizontal_line << "\n";

   out << "  " << std::setw(static_cast<int>(first_column_width)) << " " ;
   for(std::size_t i = 0; i < num_data_columns; ++i) {
      out << "| " << std::setw(static_cast<int>(data_column_width)) << "container size";
   }
   out << "|\n";

   out << table_horizontal_line << "\n";

   out << "| " << std::setw(static_cast<int>(first_column_width)) << "erase rate";
   for(std::size_t i = 0; i < num_data_columns; ++i) {
      out << "| ";
      for(auto j = min_size_exp; j <= max_size_exp; ++j) {
         out << "1.E" << j << " ";
      }
   }
   out << "|\n";

   out << table_horizontal_line << "\n";

   std::size_t row = 0;
   for(double erasure_rate = min_erasure_rate;
       erasure_rate <= max_erasure_rate;
       erasure_rate += erase_rate_inc, ++row) {
      out << "| " << std::setw(static_cast<int>(first_column_width)) << erasure_rate;
      for(const benchmark_result& res: t) {
         out << "| ";
         for(const auto& x: res.data[row]) {
            out << x << " ";
         }
      }
      out << "|\n";
   }

   out << table_horizontal_line;
}

int main(int argc,char* argv[])
{
   (void)argc;
   (void)argv;

   using namespace boost::container;
   try{
      #ifdef PLF_HIVE_BENCH
      using num = plf::hive<element>;
      #else
      using num = hub<element>;
      //using num = nest<element, void, nest_options_t< store_data_in_block<true> > >;
      //using num = nest<element, void, nest_options_t< prefetch<false> > >;
      //using num = nest<element, void, nest_options_t< prefetch<false>, store_data_in_block<true> > >;
      #endif
      using den  = nest<element>;

      table t;

      t.push_back(benchmark(
         "creat, ins, erase, ins",
         create<num>{}, create<den>{}));
      t.push_back(benchmark(
         "creat, ins, erase, ins, destroy",
         create_and_destroy<num>{}, create_and_destroy<den>{}));
      t.push_back(benchmark(
         "for_each",
         for_each<num>{}, for_each<den>{}));
      t.push_back(benchmark(
         "visit_all",
         visit_all<num>{}, visit_all<den>{}));
      t.push_back(benchmark(
         "sort",
         sort<num>{}, sort<den>{}));

      const char* filename = "hub_test.txt";
      write_table(t, filename);

      std::cout << "\n" << std::string(41, '-') << "\n"
                << "Geometric means (num/den time ratio)\n";
      for(const auto& bench: t) {
         std::cout << std::left << std::setw(30) << bench.title
                   << std::fixed << std::setprecision(3) << geomean(bench) << "\n";
      }
      std::cout << std::left << std::setw(30) << "OVERALL"
                << std::fixed << std::setprecision(3) << geomean(t) << "\n";
   }
   catch(const std::exception& e) {
      std::cerr << e.what() << std::endl;
   }
}

#endif
