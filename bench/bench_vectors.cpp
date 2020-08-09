//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2007-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning (disable : 4512)
#pragma warning (disable : 4267)
#pragma warning (disable : 4244)
#endif

#include <vector>
#include <deque>
#include <boost/container/vector.hpp>
#include <boost/container/devector.hpp>
#include <boost/container/deque.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/stable_vector.hpp>

#include <memory>    //std::allocator
#include <iostream>  //std::cout, std::endl
#include <cstring>   //std::strcmp
#include <boost/timer/timer.hpp>
#include <typeinfo>

//capacity
#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_FUNCNAME capacity
#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_BEG namespace boost { namespace container { namespace test {
#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_NS_END   }}}
#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MIN 0
#define BOOST_INTRUSIVE_HAS_MEMBER_FUNCTION_CALLABLE_WITH_MAX 0
#include <boost/intrusive/detail/has_member_function_callable_with.hpp>

using boost::timer::cpu_timer;
using boost::timer::cpu_times;
using boost::timer::nanosecond_type;

namespace bc = boost::container;

class MyInt
{
   int int_;

   public:
   explicit MyInt(int i = 0)
      : int_(i)
   {}

   MyInt(const MyInt &other)
      :  int_(other.int_)
   {}

   MyInt & operator=(const MyInt &other)
   {
      int_ = other.int_;
      return *this;
   }

   ~MyInt()
   {
      int_ = 0;
   }
};

template<class C, bool = boost::container::test::
         has_member_function_callable_with_capacity<C>::value>
struct capacity_wrapper
{
   static typename C::size_type get_capacity(const C &c)
   {  return c.capacity(); }

   static void set_reserve(C &c, typename C::size_type cp)
   {  c.reserve(cp); }
};

template<class C>
struct capacity_wrapper<C, false>
{
   static typename C::size_type get_capacity(const C &)
   {  return 0u; }

   static void set_reserve(C &, typename C::size_type )
   { }
};

const std::size_t RangeSize = 5;

struct insert_end_range
{
   std::size_t capacity_multiplier() const
   {  return RangeSize;  }

   template<class C>
   BOOST_CONTAINER_FORCEINLINE void operator()(C &c, int)
   {  c.insert(c.end(), &a[0], &a[0]+RangeSize); }

   const char *name() const
   {  return "insert_end_range"; }

   MyInt a[RangeSize];
};

struct insert_end_repeated
{
   std::size_t capacity_multiplier() const
   {  return RangeSize;  }

   template<class C>
   BOOST_CONTAINER_FORCEINLINE void operator()(C &c, int i)
   {  c.insert(c.end(), RangeSize, MyInt(i)); }

   const char *name() const
   {  return "insert_end_repeated"; }

   MyInt a[RangeSize];
};

struct push_back
{
   std::size_t capacity_multiplier() const
   {  return 1;  }

   template<class C>
   BOOST_CONTAINER_FORCEINLINE void operator()(C &c, int i)
   {  c.push_back(MyInt(i)); }

   const char *name() const
   {  return "push_back"; }
};

struct emplace_back
{
   std::size_t capacity_multiplier() const
   {  return 1;  }

   template<class C>
   BOOST_CONTAINER_FORCEINLINE void operator()(C &c, int i)
   {  c.emplace_back(MyInt(i)); }

   const char *name() const
   {  return "emplace_back"; }
};

struct insert_near_end_repeated
{

   std::size_t capacity_multiplier() const
   {  return RangeSize;  }

   template<class C>
   BOOST_CONTAINER_FORCEINLINE void operator()(C &c, int i)
   {  c.insert(c.size() >= RangeSize ? c.end()-RangeSize : c.begin(), RangeSize, MyInt(i)); }

   const char *name() const
   {  return "insert_near_end_repeated"; }
};

struct insert_near_end_range
{
   std::size_t capacity_multiplier() const
   {  return RangeSize;  }

   template<class C>
   BOOST_CONTAINER_FORCEINLINE void operator()(C &c, int)
   {
      c.insert(c.size() >= RangeSize ? c.end()-RangeSize : c.begin(), &a[0], &a[0]+RangeSize);
   }

   const char *name() const
   {  return "insert_near_end_repeated"; }

   MyInt a[RangeSize];
};

struct insert_near_end
{
   std::size_t capacity_multiplier() const
   {  return 1;  }

   template<class C>
   BOOST_CONTAINER_FORCEINLINE void operator()(C &c, int i)
   {
      typedef typename C::iterator it_t;
      it_t it (c.end());
      it -= static_cast<typename C::size_type>(!c.empty());
      c.insert(it, MyInt(i));
   }

   const char *name() const
   {  return "insert_near_end"; }
};


template<class Container, class Operation>
void vector_test_template(unsigned int num_iterations, unsigned int num_elements, const char *cont_name)
{
   cpu_timer timer;
   timer.resume();


   unsigned int capacity = 0;
   Operation op;
   typedef capacity_wrapper<Container> cpw_t;
   const typename Container::size_type multiplier = op.capacity_multiplier();

   for(unsigned int r = 0; r != num_iterations; ++r){
      Container c;
      cpw_t::set_reserve(c, num_elements);

      for(unsigned e = 0, max = num_elements/multiplier; e != max; ++e){
         op(c, static_cast<int>(e));
      }

      capacity = static_cast<unsigned int>(cpw_t::get_capacity(c));
   }

   timer.stop();


   nanosecond_type nseconds = timer.elapsed().wall;

   std::cout   << cont_name << "->" << op.name() <<" ns: "
               << float(nseconds)/(num_iterations*num_elements)
               << '\t'
               << "Capacity: " << (unsigned int)capacity
               << "\n";
}

template<class Operation>
void test_vectors()
{
   //#define SINGLE_TEST
   #define SIMPLE_IT
   #ifdef SINGLE_TEST
      #ifdef NDEBUG
      std::size_t numit [] = { 100 };
      #else
      std::size_t numit [] = { 20 };
      #endif
      std::size_t numele [] = { 10000 };
   #elif defined SIMPLE_IT
      std::size_t numit [] = { 100 };
      std::size_t numele [] = { 10000 };
   #else
      #ifdef NDEBUG
      unsigned int numit []  = { 1000, 10000, 100000, 1000000 };
      #else
      unsigned int numit []  = { 100, 1000, 10000, 100000 };
      #endif
      unsigned int numele [] = { 10000, 1000,   100,     10       };
   #endif

   for(unsigned int i = 0; i < sizeof(numele)/sizeof(numele[0]); ++i){
      vector_test_template< std::vector<MyInt, std::allocator<MyInt> >, Operation >(numit[i], numele[i]           , "std::vector  ");
      vector_test_template< bc::vector<MyInt, std::allocator<MyInt> >, Operation >(numit[i], numele[i]            , "vector       ");
      vector_test_template< bc::devector<MyInt, std::allocator<MyInt> >, Operation >(numit[i], numele[i]          , "devector     ");
      vector_test_template< bc::small_vector<MyInt, 0, std::allocator<MyInt> >, Operation >(numit[i], numele[i]   , "small_vector ");
      vector_test_template< std::deque<MyInt, std::allocator<MyInt> >, Operation >(numit[i], numele[i]            , "std::deque   ");
      vector_test_template< bc::deque<MyInt, std::allocator<MyInt> >, Operation >(numit[i], numele[i]             , "deque        ");
   }

   std::cout   << "---------------------------------\n---------------------------------\n";
}

int main()
{
   //end
   test_vectors<push_back>();
   test_vectors<emplace_back>();
   test_vectors<insert_end_range>();
   test_vectors<insert_end_repeated>();
   //near end
   test_vectors<insert_near_end>();
   test_vectors<insert_near_end_range>();
   test_vectors<insert_near_end_repeated>();

   return 0;
}
