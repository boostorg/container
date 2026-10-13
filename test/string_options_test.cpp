//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Ion Gaztanaga 2004-2013. Distributed under the Boost
// Software License, Version 1.0. (See accompanying file
// LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/container for documentation.
//
//////////////////////////////////////////////////////////////////////////////
#include <boost/container/string.hpp>
#include <boost/container/allocator.hpp>
#include <boost/container/detail/next_capacity.hpp>
#include "lightweight_test.hpp"
#include <climits>

using namespace boost::container;

template<class Unsigned, class StringType>
void test_stored_size_type_impl()
{
   typedef typename StringType::size_type    size_type;
   typedef typename StringType::value_type   value_type;
   //We need a bit to store short/long info, minus the null terminator
   size_type const max = Unsigned(-1)/2u - 1u;
   BOOST_TEST(StringType().max_size() == max);
   {  //reserve allocates the requested size, not the size plus the requested size
      StringType s(max/2u, value_type('a'));
      s.shrink_to_fit();
      s.reserve(max);
      BOOST_TEST(s.capacity() == max);
   }
   {  //assignment of a bigger string near the limit
      StringType a(max/2u, value_type('a')), b(max, value_type('b'));
      a = b;
      BOOST_TEST(a.size() == max);
   }
   #ifndef BOOST_NO_EXCEPTIONS
   StringType s;
   s.resize(5);
   s.resize(max);
   BOOST_TEST_THROWS(s.resize(max+1),                    std::exception);
   BOOST_TEST_THROWS(s.push_back(value_type(1)),         std::exception);
   BOOST_TEST_THROWS(s.insert(s.begin(), value_type(1)), std::exception);
   BOOST_TEST_THROWS(s.reserve(max+1),                   std::exception);
   BOOST_TEST_THROWS(s.append(size_type(1), value_type(1)), std::exception);
   //Constructors and operator+ beyond the limit must throw, not truncate the size
   BOOST_TEST_THROWS(StringType(max+1, default_init),    std::exception);
   BOOST_TEST_THROWS(s + s,                              std::exception);
   #endif
}

template<class Unsigned>
void test_stored_size_type()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = string_options_t< stored_size<Unsigned> >;
   #else
   typedef typename string_options
      < stored_size<Unsigned> >::type options_t;
   #endif

   basic_string<char, std::char_traits<char>, void> default_string_t;
   //Test first with a typical allocator
   {
      typedef basic_string<char, std::char_traits<char>, void, options_t> string_t;
      BOOST_CONTAINER_STATIC_ASSERT(sizeof(string_t) < sizeof(default_string_t));
      test_stored_size_type_impl<Unsigned, string_t>();
   }
   //Test with a V2 allocator: it can receive more memory than requested
   {
      typedef basic_string<char, std::char_traits<char>, allocator<char>, options_t> string_t;
      BOOST_CONTAINER_STATIC_ASSERT(sizeof(string_t) < sizeof(default_string_t));
      test_stored_size_type_impl<Unsigned, string_t>();
   }
   {
      //Test initial capacity
      typedef basic_string<char, std::char_traits<char>, void, options_t> string_t;
      const std::size_t theoretical_long_repr_t = ((2 * sizeof(Unsigned) - 1u) / sizeof(void*) + 1u) * sizeof(void*) + sizeof(void*);
      BOOST_CONTAINER_STATIC_ASSERT(sizeof(string_t)  == theoretical_long_repr_t);
      const std::size_t theoretical_sso_cap = theoretical_long_repr_t - 2u;   //-2 --> 1 for short header, 1 for null terminator
      string_t s;
      BOOST_TEST(s.capacity() == theoretical_sso_cap);
   }
}

void test_growth_factor_50()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = string_options_t< growth_factor<growth_factor_50> >;
   #else
   typedef string_options
      < growth_factor<growth_factor_50> >::type options_t;
   #endif

   basic_string<char, std::char_traits<char>, void, options_t> s;

   s.resize(5);
   s.resize(s.capacity());
   std::size_t old_capacity = s.capacity();
   s.push_back(0);
   std::size_t new_capacity = s.capacity();
   std::size_t old_storage = old_capacity+1u;
   std::size_t new_storage = new_capacity + 1u;
   BOOST_TEST(new_storage == (old_storage + old_storage/2));
}

void test_growth_factor_60()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = string_options_t< growth_factor<growth_factor_60> >;
   #else
   typedef string_options
      < growth_factor<growth_factor_60> >::type options_t;
   #endif

   basic_string<char, std::char_traits<char>, void, options_t> s;

   s.resize(5);
   s.resize(s.capacity());
   std::size_t old_capacity = s.capacity();
   s.push_back(0);
   std::size_t new_capacity = s.capacity();
   std::size_t old_storage = old_capacity+1u;
   std::size_t new_storage = new_capacity + 1u;
   BOOST_TEST(new_storage == (old_storage + 3*old_storage/5));
}

void test_growth_factor_100()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = string_options_t< growth_factor<growth_factor_100> >;
   #else
   typedef string_options
      < growth_factor<growth_factor_100> >::type options_t;
   #endif

   basic_string<char, std::char_traits<char>, void, options_t> s;

   s.resize(5);
   s.resize(s.capacity());
   std::size_t old_capacity = s.capacity();
   s.push_back(0);
   std::size_t new_capacity = s.capacity();
   std::size_t old_storage = old_capacity+1u;
   std::size_t new_storage = new_capacity + 1u;
   BOOST_TEST(new_storage == 2u*old_storage);
}

//The biggest internal buffer that stored_size allows (inline_chars<127> with
//stored_size<unsigned char> does not compile): the internal buffer is full and at the limit
void test_inline_chars_at_stored_size_limit()
{
   #if !defined(BOOST_NO_CXX11_TEMPLATE_ALIASES)
   using options_t = string_options_t< inline_chars<126>, stored_size<unsigned char> >;
   #else
   typedef string_options< inline_chars<126>, stored_size<unsigned char> >::type options_t;
   #endif
   typedef basic_string<char, std::char_traits<char>, void, options_t> string_t;
   string_t s(126u, 'a');
   BOOST_TEST(s.capacity() == 126u);
   BOOST_TEST(s.max_size() == 126u);
   #ifndef BOOST_NO_EXCEPTIONS
   BOOST_TEST_THROWS(s.push_back('b'), std::exception);
   BOOST_TEST(s.size() == 126u);
   #endif
}

//A length of npos does not fit
void test_npos_length()
{
   #ifndef BOOST_NO_EXCEPTIONS
   BOOST_TEST_THROWS(string(string::npos, default_init), std::exception);
   BOOST_TEST_THROWS(string(string::npos, 'a'),          std::exception);
   #endif
}

//An allocator with a max_size smaller than the internal buffer
template<class T>
class small_max_size_allocator
{
   public:
   typedef T value_type;

   template<class U>
   struct rebind
   {  typedef small_max_size_allocator<U> other;   };

   small_max_size_allocator()
   {}

   template<class U>
   small_max_size_allocator(const small_max_size_allocator<U> &)
   {}

   T* allocate(std::size_t n)
   {
      if(n > this->max_size())
         throw_bad_alloc();
      return static_cast<T*>(::operator new(n*sizeof(T)));
   }

   void deallocate(T *p, std::size_t)
   {  ::operator delete(p);  }

   std::size_t max_size() const
   {  return 4u;  }

   friend bool operator==(const small_max_size_allocator &, const small_max_size_allocator &)
   {  return true;  }

   friend bool operator!=(const small_max_size_allocator &, const small_max_size_allocator &)
   {  return false;  }
};

//The internal buffer is bigger than the allocator max_size: the internal buffer
//is always available and growth beyond it throws length_error
void test_allocator_max_size_smaller_than_internal_buffer()
{
   typedef basic_string<char, std::char_traits<char>, small_max_size_allocator<char> > string_t;
   string_t s;
   const std::size_t cap = s.capacity();
   BOOST_TEST(cap > 3u);
   BOOST_TEST(s.max_size() == cap);
   s.assign(cap, 'a');
   BOOST_TEST(s.size() == cap);
   BOOST_TEST(s.capacity() == cap);
   #ifndef BOOST_NO_EXCEPTIONS
   BOOST_TEST_THROWS(s.push_back('b'), length_error_t);
   BOOST_TEST_THROWS(s.append(1u, 'b'), length_error_t);
   BOOST_TEST_THROWS(s.insert(s.begin(), 'b'), length_error_t);
   BOOST_TEST_THROWS(s.resize(cap + 1u), length_error_t);
   BOOST_TEST_THROWS(s.reserve(cap + 1u), length_error_t);
   BOOST_TEST(s.size() == cap);
   #endif
}

//replace can make the size exactly max_size(), but not bigger
void test_replace_up_to_max_size()
{
   typedef basic_string<char, std::char_traits<char>, small_max_size_allocator<char> > string_t;
   const std::size_t max = string_t().max_size();
   const string_t two(2u, 'b');
   {
      string_t s(max - 1u, 'a');
      s.replace(0u, 1u, two);
      BOOST_TEST(s.size() == max);
   }
   {
      string_t s(max - 1u, 'a');
      s.replace(0u, 1u, "bb", 2u);
      BOOST_TEST(s.size() == max);
   }
   {
      string_t s(max - 1u, 'a');
      s.replace(0u, 1u, 2u, 'b');
      BOOST_TEST(s.size() == max);
   }
   #ifndef BOOST_NO_EXCEPTIONS
   {
      string_t s(max, 'a');
      BOOST_TEST_THROWS(s.replace(0u, 1u, two), length_error_t);
      BOOST_TEST_THROWS(s.replace(0u, 1u, "bb", 2u), length_error_t);
      BOOST_TEST_THROWS(s.replace(0u, 1u, 2u, 'b'), length_error_t);
      BOOST_TEST(s == string_t(max, 'a'));
   }
   #endif
}

//n default-initialized characters, just above the internal buffer capacity
void test_default_init_above_internal_capacity()
{
   const std::size_t cap = string().capacity();
   for(std::size_t n = cap - 1u; n != cap + 2u; ++n){
      string s(n, default_init);
      BOOST_TEST(s.size() == n);
      BOOST_TEST(s.capacity() >= n);
      BOOST_TEST(s.c_str()[n] == '\0');
   }
}

int main()
{
   test_allocator_max_size_smaller_than_internal_buffer();
   test_default_init_above_internal_capacity();
   test_replace_up_to_max_size();
   test_inline_chars_at_stored_size_limit();
   test_npos_length();
   test_growth_factor_50();
   test_growth_factor_60();
   test_growth_factor_100();
   test_stored_size_type<unsigned char>();
   test_stored_size_type<unsigned short>();

   return ::boost::report_errors();
}
