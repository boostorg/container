//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#include <boost/container/devector.hpp>
#include <boost/container/allocator.hpp>
#include <boost/container/detail/next_capacity.hpp>
#include "lightweight_test.hpp"
#include <stdexcept>

using namespace boost::container;

template<class Unsigned, class DevectorType>
void test_stored_size_type_impl()
{
   BOOST_TEST(DevectorType().max_size() == typename DevectorType::size_type(Unsigned(-1)));
   #ifndef BOOST_NO_EXCEPTIONS
   DevectorType v;
   typedef typename DevectorType::size_type    size_type;
   typedef typename DevectorType::value_type   value_type;
   size_type const max = Unsigned(-1);
   v.resize(5);
   v.resize(max);
   BOOST_TEST_THROWS(v.resize(max+1),                    std::exception);
   BOOST_TEST_THROWS(v.push_back(value_type(1)),         std::exception);
   BOOST_TEST_THROWS(v.insert(v.begin(), value_type(1)), std::exception);
   BOOST_TEST_THROWS(v.emplace(v.begin(), value_type(1)),std::exception);
   BOOST_TEST_THROWS(v.reserve(max+1),                   std::exception);
   BOOST_TEST_THROWS(DevectorType v2(max+1),             std::exception);
   #endif
}

//All max_size() elements can be inserted: at max_size() the buffer can not be
//reallocated, so the free fraction that devector keeps is used too
template<class Unsigned, class DevectorType>
void test_stored_size_type_fill_to_max()
{
   typedef typename DevectorType::size_type    size_type;
   typedef typename DevectorType::value_type   value_type;
   const size_type max = Unsigned(-1);
   {
      DevectorType v;
      while(v.size() != max)
         v.push_back(value_type(1));
      BOOST_TEST(v.capacity() == max);
      #ifndef BOOST_NO_EXCEPTIONS
      BOOST_TEST_THROWS(v.push_back(value_type(1)), std::exception);
      BOOST_TEST(v.size() == max);
      #endif
   }
   {
      DevectorType v;
      while(v.size() != max)
         v.push_front(value_type(1));
      BOOST_TEST(v.capacity() == max);
   }
   {
      //Insertions in the middle near the limit (inserting all elements in the middle is quadratic)
      DevectorType v;
      v.resize(max - max/16u);
      while(v.size() != max)
         v.insert(v.begin() + static_cast<typename DevectorType::difference_type>(v.size()/2u), value_type(1));
      BOOST_TEST(v.size() == max);
   }
   {
      DevectorType v;
      v.reserve(max);
      BOOST_TEST(v.capacity() >= max);
      DevectorType w(max, reserve_only_tag_t());
      BOOST_TEST(w.capacity() >= max);
      while(w.size() != max)
         w.push_back(value_type(1));
      BOOST_TEST(w.capacity() == max);
   }
}

template<class Unsigned>
void test_stored_size_type()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = devector_options_t< stored_size<Unsigned> >;
   #else
   typedef typename devector_options
      < stored_size<Unsigned> >::type options_t;
   #endif

   //Test first with a typical allocator
   {
      typedef devector<unsigned char, new_allocator<unsigned char>, options_t> devector_t;
      test_stored_size_type_impl<Unsigned, devector_t>();
      test_stored_size_type_fill_to_max<Unsigned, devector_t>();
   }
   //Test with a V2 allocator
   {
      typedef devector<unsigned char, allocator<unsigned char>, options_t> devector_t;
      test_stored_size_type_impl<Unsigned, devector_t>();
      test_stored_size_type_fill_to_max<Unsigned, devector_t>();
   }
   //Test size reduction
   {
      typedef devector<unsigned char, void, options_t> devector_t;
      BOOST_CONTAINER_STATIC_ASSERT( sizeof(Unsigned) >= sizeof(std::size_t) ||
                           sizeof(devector_t) < sizeof(devector<unsigned char>) );
   }
   
}

void test_growth_factor_50()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = devector_options_t< growth_factor<growth_factor_50> >;
   #else
   typedef devector_options
      < growth_factor<growth_factor_50> >::type options_t;
   #endif

   devector<int, new_allocator<int>, options_t> v(5u, 0);
   std::size_t old_capacity = v.size() + v.back_free_capacity() + v.front_free_capacity();
   v.push_back(0);
   std::size_t new_capacity = v.size() + v.back_free_capacity() + v.front_free_capacity();
   BOOST_TEST(new_capacity == old_capacity + old_capacity/2);
}

void test_growth_factor_60()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = devector_options_t< growth_factor<growth_factor_60> >;
   #else
   typedef devector_options
      < growth_factor<growth_factor_60> >::type options_t;
   #endif

   devector<int, new_allocator<int>, options_t> v(5u, 0);

   std::size_t old_capacity = v.size()+v.back_free_capacity()+v.front_free_capacity();
   v.push_back(0);
   std::size_t new_capacity = v.size() + v.back_free_capacity() + v.front_free_capacity();
   BOOST_TEST(new_capacity == old_capacity + 3*old_capacity/5);
}

void test_growth_factor_100()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = devector_options_t< growth_factor<growth_factor_100> >;
   #else
   typedef devector_options
      < growth_factor<growth_factor_100> >::type options_t;
   #endif

   devector<int, new_allocator<int>, options_t> v(5,0);
   std::size_t old_capacity = v.size() + v.back_free_capacity() + v.front_free_capacity();
   v.push_back(0);
   std::size_t new_capacity = v.size() + v.back_free_capacity() + v.front_free_capacity();
   BOOST_TEST(new_capacity == 2*old_capacity);
}


void  test_stored_reloc_on_66()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = devector_options_t< relocate_on_66 >;
   #else
   typedef devector_options
      < relocate_on_66 >::type options_t;
   #endif
   const std::size_t buffer_size = 32u;
   const std::size_t initial_side = buffer_size/2u;

   devector<int, new_allocator<int>, options_t> v(initial_side, initial_side, reserve_only_tag_t());

   const int* buffer = v.data();
   const int* buffer_start = v.data() - initial_side;
   std::size_t old_cp = v.capacity();
   for ( int i = 0u; i != (int)initial_side; ++i) {
      v.push_back(i);
   }
   BOOST_TEST(v.back_free_capacity() == 0);
   BOOST_TEST(buffer == v.data());
   v.push_back(0);
   BOOST_TEST(v.size() == initial_side + 1u);
   //Relocation -> 9 elements on the left, 8 elements on the right
   BOOST_TEST(v.front_free_capacity() == (buffer_size - v.size())/2u);
   BOOST_TEST(v.data() == buffer_start + v.front_free_capacity());
   BOOST_TEST(v.capacity() == old_cp);

   //Reach the back limit again
   for (int i = 0u, max = (int)v.back_free_capacity(); i != max; ++i) {
      v.push_back(-i);
   }
   BOOST_TEST(v.back_free_capacity() == 0);

   //New insertion should reallocate
   v.push_back(-1);
   BOOST_TEST(v.back_free_capacity() > initial_side/2u);
   BOOST_TEST(v.capacity() > old_cp);
}

void  test_stored_reloc_on_90()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = devector_options_t< relocate_on_90 >;
   #else
   typedef devector_options< relocate_on_90 >::type options_t;
   #endif

   const std::size_t buffer_size = 32u;
   const std::size_t initial_side = buffer_size/2u;

   devector<int, new_allocator<int>, options_t> v(initial_side, initial_side, reserve_only_tag_t());

   const int* buffer = v.data();
   const int* buffer_start = v.data() - initial_side;
   std::size_t old_cp = v.capacity();
   for ( int i = 0u; i != (int)initial_side; ++i) {
      v.push_back(i);
   }
   BOOST_TEST(v.back_free_capacity() == 0);
   BOOST_TEST(buffer == v.data());
   v.push_back(0);
   BOOST_TEST(v.size() == initial_side + 1u);
   //Relocation -> 9 elements on the left, 8 elements on the right
   BOOST_TEST(v.front_free_capacity() == (buffer_size - v.size())/2u);
   BOOST_TEST(v.data() == buffer_start + v.front_free_capacity());
   BOOST_TEST(v.capacity() == old_cp);

   //Reach the back limit again
   for (int i = 0u, max = (int)v.back_free_capacity(); i != max; ++i) {
      v.push_back(-i);
   }
   BOOST_TEST(v.back_free_capacity() == 0);

   //New insertion should relocate again
   v.push_back(-1);
   BOOST_TEST(v.capacity() == old_cp);
   BOOST_TEST(v.front_free_capacity() == (buffer_size - v.size()) / 2u);
   BOOST_TEST(v.data() == buffer_start + v.front_free_capacity());

   //Reach the back limit again
   for (int i = 0u, max = (int)v.back_free_capacity(); i != max; ++i) {
      v.push_back(-i);
   }
   BOOST_TEST(v.back_free_capacity() == 0);

   //Last insertion should reallocate
   v.push_back(-1);
   BOOST_TEST(v.capacity() > old_cp);
}

#ifndef BOOST_NO_EXCEPTIONS

//The constructor from int throws when throw_on_construction is true
struct throwing_int
{
   static bool throw_on_construction;
   int value;

   throwing_int(int v)
      : value(v)
   {
      if (throw_on_construction)
         throw std::runtime_error("throwing_int");
   }
};

bool throwing_int::throw_on_construction = false;

//With relocate_never an insertion at a full end reallocates, and the reallocation constructs
//the new element before it moves the old ones: if the construction throws, nothing changes
template<class Devector, bool Back>
void test_reloc_never_strong_guarantee()
{
   const std::size_t side = 8u;
   Devector v(side, side, reserve_only_tag_t());
   for (int i = 0; i != (int)side; ++i) {
      if (Back) v.emplace_back(i);
      else      v.emplace_front(i);
   }
   BOOST_TEST((Back ? v.back_free_capacity() : v.front_free_capacity()) == 0u);
   BOOST_TEST((Back ? v.front_free_capacity() : v.back_free_capacity()) == side);
   const throwing_int *const old_data = v.data();
   const std::size_t old_capacity = v.capacity();

   throwing_int::throw_on_construction = true;
   BOOST_TEST_THROWS(Back ? (void)v.emplace_back(-1) : (void)v.emplace_front(-1), std::runtime_error);
   throwing_int::throw_on_construction = false;

   BOOST_TEST(v.size() == side);
   BOOST_TEST(v.data() == old_data);
   BOOST_TEST(v.capacity() == old_capacity);
   for (std::size_t i = 0; i != side; ++i) {
      BOOST_TEST(v[i].value == (Back ? int(i) : int(side - 1u - i)));
   }
}

#endif   //BOOST_NO_EXCEPTIONS

void  test_stored_reloc_never()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = devector_options_t< relocate_never >;
   #else
   typedef devector_options< relocate_never >::type options_t;
   #endif
   typedef devector<int, new_allocator<int>, options_t> devector_t;

   const std::size_t buffer_size = 32u;
   const std::size_t initial_side = buffer_size/2u;

   //All the buffer is usable
   devector_t v(initial_side, initial_side, reserve_only_tag_t());
   BOOST_TEST(v.capacity() == buffer_size);
   BOOST_TEST(v.front_free_capacity() == initial_side);
   BOOST_TEST(v.back_free_capacity() == initial_side);
   {
      devector_t r;
      r.reserve(10u);
      BOOST_TEST(r.capacity() == 10u);
   }

   //The back is full: the next insertion reallocates although the front is free
   const std::size_t old_cp = v.capacity();
   for (int i = 0; i != (int)initial_side; ++i) {
      v.push_back(i);
   }
   BOOST_TEST(v.back_free_capacity() == 0u);
   BOOST_TEST(v.front_free_capacity() == initial_side);
   v.push_back(-1);
   BOOST_TEST(v.capacity() > old_cp);
   BOOST_TEST(v.size() == initial_side + 1u);
   for (int i = 0; i != (int)initial_side; ++i) {
      BOOST_TEST(v[std::size_t(i)] == i);
   }
   BOOST_TEST(v.back() == -1);

   //The same at the front
   {
      devector_t f(initial_side, initial_side, reserve_only_tag_t());
      for (int i = 0; i != (int)initial_side; ++i) {
         f.push_front(i);
      }
      BOOST_TEST(f.front_free_capacity() == 0u);
      f.push_front(-1);
      BOOST_TEST(f.capacity() > buffer_size);
      BOOST_TEST(f.size() == initial_side + 1u);
      BOOST_TEST(f.front() == -1);
   }

   //Never relocate, also when a buffer of max_size() elements can not be reallocated
   #ifndef BOOST_NO_EXCEPTIONS
   {
      #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
      using small_options_t = devector_options_t< relocate_never, stored_size<unsigned char> >;
      #else
      typedef devector_options< relocate_never, stored_size<unsigned char> >::type small_options_t;
      #endif
      devector<int, new_allocator<int>, small_options_t> s;
      s.reserve(s.max_size());
      BOOST_TEST(s.capacity() == s.max_size());
      const int* const s_buffer = s.data() - s.front_free_capacity();
      while (s.back_free_capacity() != 0u) {
         s.push_back(1);
      }
      const std::size_t s_size = s.size();
      const std::size_t s_front_free = s.front_free_capacity();
      BOOST_TEST(s_front_free != 0u);
      BOOST_TEST_THROWS(s.push_back(2), std::exception);
      BOOST_TEST(s.capacity() == s.max_size());
      BOOST_TEST(s.size() == s_size);
      BOOST_TEST(s.front_free_capacity() == s_front_free);
      BOOST_TEST(s.data() - s.front_free_capacity() == s_buffer);
   }
   #endif   //BOOST_NO_EXCEPTIONS

   //Strong exception guarantee at both ends
   #ifndef BOOST_NO_EXCEPTIONS
   test_reloc_never_strong_guarantee<devector<throwing_int, new_allocator<throwing_int>, options_t>, true>();
   test_reloc_never_strong_guarantee<devector<throwing_int, new_allocator<throwing_int>, options_t>, false>();
   #endif
}

int main()
{
   test_growth_factor_50();
   test_growth_factor_60();
   test_growth_factor_100();
   test_stored_size_type<unsigned char>();
   test_stored_size_type<unsigned short>();
   test_stored_reloc_on_66();
   test_stored_reloc_on_90();
   test_stored_reloc_never();
   return ::boost::report_errors();
}
