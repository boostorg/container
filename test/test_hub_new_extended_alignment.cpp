/* Copyright 2026 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#include <boost/config.hpp>

#if BOOST_CXX_VERSION < 201103L

int main() { return 0; }

#else

#if !defined(__cpp_aligned_new) || __cpp_aligned_new < 201606L
#include <boost/config/pragma_message.hpp>

BOOST_PRAGMA_MESSAGE("Test skipped because aligned new is not available.")

int main()
{
}
#else
#include <boost/config.hpp>
#include <boost/container/hub.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cstdint>
#include <new>

template<typename T>
struct aligned_new_allocator
{
  using value_type = T;

  aligned_new_allocator() = default;
  template<typename U>
  aligned_new_allocator(const aligned_new_allocator<U>&) {}

  T* allocate(std::size_t n)
  {
    return static_cast<T*>(
      ::operator new(n * sizeof(T), std::align_val_t{alignof(T)}));
  }

  void deallocate(T* p, std::size_t)
  { 
    ::operator delete(p, std::align_val_t{alignof(T)});
  }

  bool operator==(const aligned_new_allocator&) const { return true; }
  bool operator!=(const aligned_new_allocator&) const { return false; }
};

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4324) /* structure padded due to alignment specifier */
#endif

struct alignas(__STDCPP_DEFAULT_NEW_ALIGNMENT__ * 2)
new_extended_aligned_object
{
  new_extended_aligned_object(int n_): n{n_} { check_alignment(); }
  new_extended_aligned_object(const new_extended_aligned_object& x):
    n{x.n} { check_alignment(); }

  new_extended_aligned_object& operator=(const new_extended_aligned_object&)
    = default;

  void check_alignment()
  {
    constexpr auto mask =
      static_cast<uintptr_t>(alignof(new_extended_aligned_object) - 1);

    BOOST_TEST_EQ(reinterpret_cast<uintptr_t>(this) & mask, 0);
  }

  bool operator<(const new_extended_aligned_object& x) const 
  { return n < x.n; }

  int n;
};

#if defined(BOOST_MSVC)
#pragma warning(pop) /* C4324 */
#endif

using new_extended_alignment_hub = boost::container::hub<
  new_extended_aligned_object,
  aligned_new_allocator<new_extended_aligned_object>>;

/* The only internal sort function of hub<T> that allocates auxiliary memory
 * for T is transfer_sort: this function, however, is never called when T has
 * new-extended alignment because in this case sizeof(T) exceeds the limit from
 * which other sort functions are selected.
 * For robustness, we test transfer_sort even in this impossible situation by
 * illegally forcing its use through the famous http://www.gotw.ca/gotw/076.htm
 * trick.
 */

namespace boost {
namespace container {

template<> 
template<>
void new_extended_alignment_hub::sort(
  std::less<new_extended_aligned_object> cmp)
{
  transfer_sort(cmp);
}

} /* namespace container */
} /* namespace boost */

int main()
{
  new_extended_alignment_hub h;
  for(int i = 5 ; i >= 0; --i) h.emplace(i);
  h.sort();

  return boost::report_errors();
}
#endif

#endif
