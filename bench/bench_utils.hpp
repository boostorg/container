//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2025-2026. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_CONTAINER_BENCH_UTILS_HPP
#define BOOST_CONTAINER_BENCH_UTILS_HPP

#include <boost/move/detail/nsec_clock.hpp>
#include <boost/container/detail/workaround.hpp>

volatile int bench_utils_sink = 0;

//#define BOOST_CONTAINER_BENCH_UTILS_NO_BARRIERS

#ifdef BOOST_CONTAINER_BENCH_UTILS_NO_BARRIERS
   #define BOOST_CONTAINER_BENCH_CLOBBER()   ((void)0)
   #define BOOST_CONTAINER_BENCH_ESCAPE(p)   ((void)(p))
#else
   #if defined(_MSC_VER)
      #define BOOST_CONTAINER_BENCH_CLOBBER()   _ReadWriteBarrier()
      #define BOOST_CONTAINER_BENCH_ESCAPE(p)   (bench_utils_sink = *static_cast<int*>(p))
   #elif defined(__GNUC__)
      #define BOOST_CONTAINER_BENCH_CLOBBER()   asm volatile("" : : : "memory")
      //#define BOOST_CONTAINER_BENCH_ESCAPE(p)   asm volatile("" : : "g"(p) : "memory")
      #define BOOST_CONTAINER_BENCH_ESCAPE(p)   asm volatile("" : "+r,m"(p) : : "memory")

   #else
      #define BOOST_CONTAINER_BENCH_CLOBBER()   ((void)0)
      #define BOOST_CONTAINER_BENCH_ESCAPE(p)   ((void)(p))
   #endif
#endif

#define BOOST_CONTAINER_BENCH_UTILS_USE_MEDIAN_CLOCK

#ifdef BOOST_CONTAINER_BENCH_UTILS_USE_MEDIAN_CLOCK

class cpu_timer
{
   typedef boost::move_detail::cpu_times cpu_times;
   typedef boost::move_detail::nanosecond_type nanosecond_type;

   nanosecond_type start_ns_;
   bool running_;
   mutable std::vector<nanosecond_type> samples_;

   BOOST_CONTAINER_FORCEINLINE static nanosecond_type now_ns()
   {
      cpu_times t;
      boost::move_detail::get_cpu_times(t);
      return t.wall;
   }

   nanosecond_type robust_median() const
   {
      if(samples_.empty())
         return 0u;

      std::sort(samples_.begin(), samples_.end());

      std::size_t trim = samples_.size() / 20u; // Drop 5% low and high values.
      if(trim * 2u >= samples_.size())
         trim = 0u;

      const std::size_t begin = trim;
      const std::size_t end = samples_.size() - trim;
      const std::size_t count = end - begin;
      const std::size_t mid = begin + count / 2u;

      if((count % 2u) != 0u)
         return samples_[mid];

      return static_cast<nanosecond_type>((samples_[mid - 1u] + samples_[mid]) / 2u);
   }

   public:
   cpu_timer(std::size_t reserve = 0)
      : start_ns_(0u), running_(false), samples_()
   {  samples_.reserve(reserve); }

   BOOST_CONTAINER_FORCEINLINE bool is_stopped() const
   {  return !running_;  }

   cpu_times elapsed() const
   {
      cpu_times t;
      t.wall = robust_median() * static_cast<nanosecond_type>(samples_.size());
      return t;
   }

   void start()
   {
      samples_.clear();
      start_ns_ = now_ns();
      running_ = true;
   }

   void stop()
   {
      if(!running_)
         return;
      const nanosecond_type now = now_ns();
      samples_.push_back(now - start_ns_);
      running_ = false;
   }

   void resume()
   {
      if(running_)
         return;
      start_ns_ = now_ns();
      running_ = true;
   }
};

#else

typedef boost::move_detail::cpu_timer cpu_timer;

#endif


BOOST_CONTAINER_FORCEINLINE void clobber()       { BOOST_CONTAINER_BENCH_CLOBBER(); }
BOOST_CONTAINER_FORCEINLINE void escape(void* p) { BOOST_CONTAINER_BENCH_ESCAPE(p); }

#endif   //BOOST_CONTAINER_BENCH_UTILS_HPP
