/* Hub container.
 * 
 * Copyright 2025-2026 Joaquin M Lopez Munoz.
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_CONTAINER_EXPERIMENTAL_HUB_HPP
#define BOOST_CONTAINER_EXPERIMENTAL_HUB_HPP

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/config/workaround.hpp>
#include <boost/core/allocator_access.hpp>
#include <boost/core/bit.hpp>
#include <boost/core/empty_value.hpp>
#include <boost/core/no_exceptions_support.hpp>
#include <boost/core/pointer_traits.hpp>
#include <boost/throw_exception.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <utility>

#ifndef BOOST_NO_CXX17_HDR_MEMORY_RESOURCE
#include <memory_resource>
#endif

#if defined(BOOST_NO_CXX20_HDR_CONCEPTS) || defined(BOOST_NO_CXX20_HDR_RANGES)
#define BOOST_CONTAINER_HUB_NO_RANGES
#elif BOOST_WORKAROUND(BOOST_CLANG_VERSION, < 170100) && \
      defined(BOOST_LIBSTDCXX_VERSION)
/* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109647
 * https://github.com/llvm/llvm-project/issues/49620
 */
#define BOOST_CONTAINER_HUB_NO_RANGES
#endif

#if !defined(BOOST_CONTAINER_HUB_NO_RANGES)
#include <concepts>
#include <ranges>
#endif

#if !defined(BOOST_CONTAINER_HUB_DISABLE_SSE2)
#if defined(BOOST_CONTAINER_HUB_ENABLE_SSE2)|| \
    defined(__SSE2__) || \
    defined(_M_X64) || (defined(_M_IX86_FP)&&_M_IX86_FP>=2)
#define BOOST_CONTAINER_HUB_SSE2
#endif
#endif

#if defined(BOOST_CONTAINER_HUB_SSE2)
#include <emmintrin.h>
#endif

#ifdef __has_builtin
#define BOOST_CONTAINER_HUB_HAS_BUILTIN(x) __has_builtin(x)
#else
#define BOOST_CONTAINER_HUB_HAS_BUILTIN(x) 0
#endif

#if !defined(NDEBUG)
#define BOOST_CONTAINER_HUB_ASSUME(cond) BOOST_ASSERT(cond)
#elif BOOST_CONTAINER_HUB_HAS_BUILTIN(__builtin_assume)
#define BOOST_CONTAINER_HUB_ASSUME(cond) __builtin_assume(cond)
#elif defined(__GNUC__) || \
      BOOST_CONTAINER_HUB_HAS_BUILTIN(__builtin_unreachable)
#define BOOST_CONTAINER_HUB_ASSUME(cond)           \
  do{                                    \
    if(!(cond)) __builtin_unreachable(); \
  } while(0)
#elif defined(_MSC_VER)
#define BOOST_CONTAINER_HUB_ASSUME(cond) __assume(cond)
#else
#define BOOST_CONTAINER_HUB_ASSUME(cond)          \
  do{                                   \
    static_cast<void>(false && (cond)); \
  } while(0)
#endif

/* We use BOOST_CONTAINER_HUB_PREFETCH[_BLOCK] macros rather than proper
 * functions because of https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109985
 */

#if defined(BOOST_GCC) || defined(BOOST_CLANG)
#define BOOST_CONTAINER_HUB_PREFETCH(p) \
__builtin_prefetch((const char*)boost::to_address(p))
#elif defined(BOOST_CONTAINER_HUB_SSE2)
#define BOOST_CONTAINER_HUB_PREFETCH(p) \
_mm_prefetch((const char*)boost::to_address(p), _MM_HINT_T0)
#else
#define BOOST_CONTAINER_HUB_PREFETCH(p) ((void)(p))
#endif

#define BOOST_CONTAINER_HUB_PREFETCH_BLOCK(pbb, Block) \
do{                                                    \
  auto p0 = &static_cast<Block&>(*(pbb));              \
  BOOST_CONTAINER_HUB_PREFETCH(p0->data());            \
} while(0)

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4714) /* marked as __forceinline not inlined */
#endif

namespace boost {

namespace container {

template<typename T, typename Allocator = std::allocator<T>>
class hub;

template<typename T, typename Allocator, typename Predicate>
typename hub<T, Allocator>::size_type erase_if(hub<T, Allocator>&, Predicate);

#ifndef BOOST_NO_CXX17_HDR_MEMORY_RESOURCE
namespace pmr {

template<typename T>
using hub = boost::container::hub<T, std::pmr::polymorphic_allocator<T>>;

}
#endif

namespace hub_detail {

inline int unchecked_countr_zero(std::uint64_t x)
{
#if defined(BOOST_MSVC) && (defined(_M_X64) || defined(_M_ARM64))
  unsigned long r;
  _BitScanForward64(&r, x);
  return (int)r;
#elif defined(BOOST_GCC) || defined(BOOST_CLANG)
  return (int)__builtin_ctzll(x);
#else
  BOOST_CONTAINER_HUB_ASSUME(x != 0);
  return (int)core::countr_zero(x);
#endif
}

inline int unchecked_countr_one(std::uint64_t x)
{
  return unchecked_countr_zero(~x);
}

inline int unchecked_countl_zero(std::uint64_t x)
{
#if defined(BOOST_MSVC) && (defined(_M_X64) || defined(_M_ARM64))
  unsigned long r;
  _BitScanReverse64(&r, x);
  return (int)(63 - r);
#elif defined(BOOST_GCC) || defined(BOOST_CLANG)
  return (int)__builtin_clzll(x);
#else  
  BOOST_CONTAINER_HUB_ASSUME(x != 0);
  return (int)core::countl_zero(x);
#endif
}

template<typename Pointer, typename T>
using pointer_rebind_t = 
  typename pointer_traits<Pointer>::template rebind<T>;

template<typename VoidPointer>
struct block_base
{
  using pointer = pointer_rebind_t<VoidPointer, block_base>;
  using const_pointer = pointer_rebind_t<VoidPointer, const block_base>;
  using mask_type = std::uint64_t;

  static constexpr int N = 64;
  static constexpr mask_type full = (mask_type)(-1);

  static pointer pointer_to(block_base& x) noexcept
  {
    return pointer_traits<pointer>::pointer_to(x);
  }

  static const_pointer pointer_to(const block_base& x) noexcept
  {
    return pointer_traits<const_pointer>::pointer_to(x);
  }

  BOOST_FORCEINLINE void link_available_before(pointer p) noexcept
  {
    next_available = p;
    prev_available = p->prev_available;
    next_available->prev_available = pointer_to(*this);
    prev_available->next_available = pointer_to(*this);
  }

  BOOST_FORCEINLINE void link_available_after(pointer p) noexcept
  {
    prev_available = p;
    next_available = p->next_available;
    next_available->prev_available = pointer_to(*this);
    prev_available->next_available = pointer_to(*this);
  }

  BOOST_FORCEINLINE void unlink_available() noexcept
  {
    prev_available->next_available = next_available;
    next_available->prev_available = prev_available;
  }

  BOOST_FORCEINLINE void link_before(pointer p) noexcept
  {
    next = p;
    prev = p->prev;
    next->prev = pointer_to(*this);
    prev->next = pointer_to(*this);
  }

  BOOST_FORCEINLINE void link_after(pointer p) noexcept
  {
    prev = p;
    next = p->next;
    next->prev = pointer_to(*this);
    prev->next = pointer_to(*this);
  }

  BOOST_FORCEINLINE void unlink() noexcept
  {
    prev->next = next;
    next->prev = prev;
  }

  pointer   prev_available,
            next_available,
            prev,
            next;
  mask_type mask;
};

template<typename ValuePointer>
struct block: block_base<pointer_rebind_t<ValuePointer, void>>
{
  using super = block_base<pointer_rebind_t<ValuePointer, void>>;

  ValuePointer data() noexcept { return data_; }
  ValuePointer data_;
};

template<typename ValuePointer>
void swap_payload(block<ValuePointer>& x, block<ValuePointer>& y) noexcept
{
  std::swap(x.mask, y.mask);
  std::swap(x.data_, y.data_);
}

template<typename ValuePointer>
struct block_list: block<ValuePointer>
{
  using block = hub_detail::block<ValuePointer>;
  using block_base = typename block::super;
  using block_base_pointer = typename block_base::pointer;
  using const_block_base_pointer = typename block_base::const_pointer;
  using block_pointer = pointer_rebind_t<ValuePointer, block>;
  using block_base::full;
  using block_base::pointer_to;
  using block_base::prev_available;
  using block_base::next_available;
  using block_base::prev;
  using block_base::next;
  using block_base::mask;
  using block::data_;

  static block_pointer 
  static_cast_block_pointer(block_base_pointer pbb) noexcept
  {
    return pointer_traits<block_pointer>::pointer_to(
      static_cast<block&>(*pbb));
  }

  block_list() 
  { 
    reset();
    mask = 1; /* sentinel */
    data_ = nullptr;
  }

  block_list(block_list&& x) noexcept: block_list{}
  {
    if(x.next_available != x.header()) {
      prev_available = x.prev_available;
      next_available = x.next_available;
      next_available->prev_available = header();
      prev_available->next_available = header();
    }
    if(x.prev != x.header()) {
      prev = x.prev;
      next = x.next;
      next->prev = header();
      prev->next = header();
    }
    x.reset();
  }

  block_list& operator=(block_list&& x) noexcept
  {
    reset();
    if(x.next_available != x.header()) {
      prev_available = x.prev_available;
      next_available = x.next_available;
      next_available->prev_available = header();
      prev_available->next_available = header();
    }
    if(x.prev != x.header()) {
      prev = x.prev;
      next = x.next;
      next->prev = header();
      prev->next = header();
    }
    x.reset();
    return *this;
  }

  void reset() noexcept
  {
    prev_available = header();
    next_available = header();
    prev = header();
    next = header();
  }

  block_base_pointer header() noexcept 
  {
    return pointer_to(static_cast<block_base&>(*this)); 
  }

  const_block_base_pointer header() const noexcept 
  {
    return pointer_to(static_cast<const block_base&>(*this)); 
  }

  BOOST_FORCEINLINE void link_at_back(block_pointer pb) noexcept 
  {
    pb->link_before(header());
  }

  BOOST_FORCEINLINE void link_before(
    block_pointer pbx, block_pointer pby) noexcept
  {
    pbx->link_before(pby);
  }

  BOOST_FORCEINLINE static void unlink(block_pointer pb) noexcept
  {
    pb->unlink();
  }

  BOOST_FORCEINLINE void link_available_at_back(block_pointer pb) noexcept 
  {
    pb->link_available_before(header());
  }

  BOOST_FORCEINLINE void link_available_at_front(block_pointer pb) noexcept 
  {
    pb->link_available_after(header());
  }

  BOOST_FORCEINLINE void unlink_available(block_pointer pb) noexcept
  {
    pb->unlink_available();
  }
};

template<typename ValuePointer>
class iterator
{
  using element_type = typename pointer_traits<ValuePointer>::element_type;
  template<typename Value2Pointer>
  using enable_if_consts_to_element_type_t =typename std::enable_if<
    std::is_same<
      const typename pointer_traits<Value2Pointer>::element_type, 
      element_type>::value
  >::type;

public:
  using value_type = typename std::remove_const<element_type>::type;
  using difference_type = 
    typename pointer_traits<ValuePointer>::difference_type;
  using pointer = ValuePointer;
  using reference = element_type&;
  using iterator_category = std::bidirectional_iterator_tag;

  iterator() = default;
  iterator(const iterator&) = default;

  template<
    typename Value2Pointer,
    typename = enable_if_consts_to_element_type_t<Value2Pointer>
  >
  iterator(const iterator<Value2Pointer>& x) noexcept: pbb{x.pbb}, n{x.n} {}
      
  iterator& operator=(const iterator& x) = default;

  template<
    typename Value2Pointer,
    typename = enable_if_consts_to_element_type_t<Value2Pointer>
  >
  iterator& operator=(const iterator<Value2Pointer>& x) noexcept
  {
    pbb = x.pbb;
    n = x.n;
    return *this;
  }

  pointer operator->() const noexcept
  {
    return static_cast<block&>(*pbb).data() + n;
  }

  reference operator*() const noexcept
  {
    return *operator->();
  }

  BOOST_FORCEINLINE iterator& operator++() noexcept
  {
    auto mask = pbb->mask & (full << 1 << n);
    if(BOOST_UNLIKELY(mask == 0)) {
      pbb = pbb->next;
      BOOST_CONTAINER_HUB_PREFETCH_BLOCK(pbb->next, block);
      mask = pbb->mask;
    }
    n = hub_detail::unchecked_countr_zero(mask);
    return *this;
  }

  BOOST_FORCEINLINE iterator operator++(int) noexcept
  {
    iterator tmp(*this);
    this->operator++();
    return tmp;
  }

  BOOST_FORCEINLINE iterator& operator--() noexcept
  {
    auto mask = pbb->mask & (full >> 1 >> (N - 1 - n));
    if(BOOST_UNLIKELY(mask == 0)) {
      pbb = pbb->prev;
      BOOST_CONTAINER_HUB_PREFETCH_BLOCK(pbb->prev, block);
      mask = pbb->mask;
    }
    n = N - 1 - hub_detail::unchecked_countl_zero(mask);
    return *this;
  }

  BOOST_FORCEINLINE iterator operator--(int) noexcept
  {
    iterator tmp(*this);
    this->operator--();
    return tmp;
  }

  friend bool operator==(const iterator& x, const iterator& y) noexcept
  {
    return x.pbb == y.pbb && x.n == y.n;
  }
  
  friend bool operator!=(const iterator& x, const iterator& y) noexcept
  {
    return !(x == y);
  }

private:
  template<typename> friend class iterator;
  template<typename, typename> friend class container::hub;

  template<typename T>
  using pointer_rebind_t = hub_detail::pointer_rebind_t<ValuePointer, T>;
  using block_base = hub_detail::block_base<pointer_rebind_t<void>>;
  using block_base_pointer = pointer_rebind_t<block_base>;
  using const_block_base_pointer = pointer_rebind_t<const block_base>;
  using nonconst_pointer = pointer_rebind_t<value_type>; /* used by Natvis */
  using block = hub_detail::block<nonconst_pointer>;
  using mask_type = typename block_base::mask_type;

  static constexpr int N = block_base::N;
  static constexpr mask_type full = block_base::full;

  iterator(const_block_base_pointer pbb_, int n_) noexcept:
    pbb{const_cast_block_base_pointer(pbb_)}, n{n_} {}

  iterator(const_block_base_pointer pbb_) noexcept:
    pbb{const_cast_block_base_pointer(pbb_)}, 
    n{hub_detail::unchecked_countr_zero(pbb->mask)} 
  {}

  static block_base_pointer
  const_cast_block_base_pointer(const_block_base_pointer pbb_) noexcept
  {
    return block_base::pointer_to(const_cast<block_base&>(*pbb_));
  }

  block_base_pointer pbb = nullptr;
  int                n = 0;
};

template<typename T, std::size_t N>
struct sort_iterator
{
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = T*;
  using reference = T&;
  using iterator_category = std::random_access_iterator_tag;

  sort_iterator(T** pp_, std::size_t index_): pp{pp_}, index{index_} {}

  pointer operator->() const noexcept
  {
    return pp[index / N] + (index % N);
  }

  reference operator*() const noexcept
  {
    return *operator->();
  }

  sort_iterator& operator++() noexcept
  {
    ++index;
    return *this;
  }

  sort_iterator operator++(int) noexcept
  {
    sort_iterator tmp(*this);
    ++index;
    return tmp;
  }

  sort_iterator& operator--() noexcept
  {
    --index;
    return *this;
  }

  sort_iterator operator--(int) noexcept
  {
    sort_iterator tmp(*this);
    --index;
    return tmp;
  }

  friend difference_type
  operator-(const sort_iterator& x, const sort_iterator& y) noexcept
  {
    return (difference_type)(x.index - y.index);
  }

  sort_iterator& operator+=(difference_type n) noexcept
  {
    index += static_cast<std::size_t>(n);
    return *this;
  }
    
  friend sort_iterator
  operator+(const sort_iterator& x, difference_type n) noexcept
  {
    return {x.pp, x.index + static_cast<std::size_t>(n)};
  }

  friend sort_iterator 
  operator+(difference_type n, const sort_iterator& x) noexcept
  {
    return {x.pp, static_cast<std::size_t>(n) + x.index};
  }

  sort_iterator& operator-=(difference_type n) noexcept
  {
    index -= static_cast<std::size_t>(n);
    return *this;
  }
    
  friend sort_iterator 
  operator-(const sort_iterator& x, difference_type n) noexcept
  {
    return {x.pp, x.index - static_cast<std::size_t>(n)};
  }

  reference operator[](difference_type n) const noexcept
  {
    return operator*(*this + n);
  }

  friend bool 
  operator==(const sort_iterator& x, const sort_iterator& y) noexcept
  {
    return x.index == y.index;
  }
  
  friend bool
  operator!=(const sort_iterator& x, const sort_iterator& y) noexcept
  {
    return x.index != y.index;
  }

  friend bool
  operator<(const sort_iterator& x, const sort_iterator& y) noexcept
  {
    return x.index < y.index;
  }

  friend bool
  operator>(const sort_iterator& x, const sort_iterator& y) noexcept
  {
    return x.index > y.index;
  }

  friend bool
  operator<=(const sort_iterator& x, const sort_iterator& y) noexcept
  {
    return x.index <= y.index;
  }

  friend bool
  operator>=(const sort_iterator& x, const sort_iterator& y) noexcept
  {
    return x.index >= y.index;
  }

  T** pp;
  std::size_t index;
};

template<typename T, typename Allocator>
struct buffer
{
  buffer(std::size_t n, Allocator al_): al{al_} 
  {
    data = static_cast<T*>(::operator new[](n * sizeof(T), std::nothrow));
    if(data) capacity = n;
  }

  ~buffer()
  {
    if(data) {
      for(; begin_ != end_; ++begin_) allocator_destroy(al, begin());
      ::operator delete[](data);
    }
  }
  
  T* begin() const noexcept { return data + begin_; }
  T* end() const noexcept { return data + end_; }

  template<typename... Args>
  void emplace_back(Args&&... args)
  {
    BOOST_ASSERT(data && end_ != capacity);
    allocator_construct(al, end(), std::forward<Args>(args)...);
    ++end_;
  }
  
  void erase_front() noexcept
  {
    BOOST_ASSERT(data && begin_ != end_);
    allocator_destroy(al, begin());
    ++begin_;
  }
  
  Allocator   al;
  std::size_t begin_ = 0, end_ = 0;
  std::size_t capacity = 0;
  T*          data = nullptr;
};

template<typename T>
struct nodtor_deleter
{
  using pointer = T*;
  void operator()(pointer p) noexcept { ::operator delete(p); }
};

template<typename T>
struct nodtor_deleter<T[]>
{
  using pointer = T*;
  void operator()(pointer p) noexcept { ::operator delete[](p); }
};

template<typename T>
using nodtor_unique_ptr = std::unique_ptr<T, nodtor_deleter<T>>;

template<typename T>
struct type_identity { using type = T; };

template<typename T>
using type_identity_t = typename type_identity<T>::type;

#if !defined(BOOST_CONTAINER_HUB_NO_RANGES)
template<class R, class T>
concept container_compatible_range =
  std::ranges::input_range<R> &&
  std::convertible_to<std::ranges::range_reference_t<R>, T>;

/* Use own from_range_t only if std::from_range_t does not exist.
 * Technique explained at
 https://bannalia.blogspot.com/2016/09/compile-time-checking-existence-of.html
 */

struct from_range_t{ explicit from_range_t() = default; };
struct from_range_t_hook{};

} /* namespace hub_detail */
} /* namespace container */
} /* namespace boost */

namespace std {

template<> struct hash< ::boost::container::hub_detail::from_range_t_hook>
{
  using from_range_t_type = decltype([] {
    using namespace ::boost::container::hub_detail;
    return from_range_t{};
  }());

  /* make standard happy */
  std::size_t operator()(
    const ::boost::container::hub_detail::from_range_t_hook&) const;
};

}

namespace boost {
namespace container {

/* TODO: this may collide with other same-named entities in different
 * parts of Boost.Container.
 */
using from_range_t = 
  typename std::hash<hub_detail::from_range_t_hook>::from_range_t_type;
inline constexpr from_range_t from_range {};

namespace hub_detail {
#endif

template<typename InputIterator>
using enable_if_is_input_iterator_t =
  typename std::enable_if<
    std::is_convertible<
      typename std::iterator_traits<InputIterator>::iterator_category,
      std::input_iterator_tag
    >::value
  >::type;

template<typename Allocator, typename Ptr, typename = void>
struct allocator_has_destroy: std::false_type {};

template<typename Allocator, typename Ptr>
struct allocator_has_destroy<
  Allocator, Ptr,
  decltype((void)std::declval<Allocator&>().destroy(std::declval<Ptr>()))
>: std::true_type {};

template<typename Allocator>
struct is_std_allocator: std::false_type {};

template<typename T>
struct is_std_allocator<std::allocator<T>>: std::true_type {};

template<typename Allocator>
struct is_std_pmr_polymorphic_allocator: std::false_type {};

#ifndef BOOST_NO_CXX17_HDR_MEMORY_RESOURCE
template<typename T>
struct is_std_pmr_polymorphic_allocator<std::pmr::polymorphic_allocator<T>>:
  std::true_type {};
#endif

struct if_constexpr_void_else{ void operator()() const {} };

template<typename F, typename G = if_constexpr_void_else>
void if_constexpr(std::true_type, F f, G = G{}) { f(); }

template<typename F, typename G>
void if_constexpr(std::false_type, F, G g) { g(); }

template<typename T>
void copy_assign_if(std::true_type, T& x, const T& y) { x = y; }

template<typename T>
void copy_assign_if(std::false_type, T&, const T&) {}

template<typename T>
void move_assign_if(std::true_type, T& x, T& y) { x = std::move(y); }

template<typename T>
void move_assign_if(std::false_type, T&, T&) {}

template<typename T>
void swap_if(std::true_type, T& x, T& y) { using std::swap; swap(x, y); }

template<typename T>
void swap_if(std::false_type, T&, T&) {}

template<typename Allocator>
struct block_typedefs
{
  using pointer = allocator_pointer_t<Allocator>;
  template<typename Q>
  using pointer_rebind_t = hub_detail::pointer_rebind_t<pointer, Q>;

  using block_base = hub_detail::block_base<pointer_rebind_t<void>>;
  using block_base_pointer = pointer_rebind_t<block_base>;
  using const_block_base_pointer = pointer_rebind_t<const block_base>;
  using block = hub_detail::block<pointer>;
  using block_pointer = pointer_rebind_t<block>;
  using block_allocator = allocator_rebind_t<Allocator,block>;
  using block_list = hub_detail::block_list<pointer>;
};

} /* namespace container::hub_detail */

template<typename T, typename Allocator>
class hub: empty_value<
  typename hub_detail::block_typedefs<Allocator>::block_allocator, 0>
{
  static_assert(
    !std::is_const<T>::value && !std::is_volatile<T>::value && 
    !std::is_function<T>::value && !std::is_reference<T>::value && 
    !std::is_void<T>::value,
    "T must be a cv-unqualified object type");
  static_assert(
    std::is_same<T, allocator_value_type_t<Allocator>>::value,
    "Allocator's value_type must be the same type as T");

public:
  using value_type = T;
  using allocator_type = Allocator;
  using pointer = allocator_pointer_t<Allocator>;
  using const_pointer = allocator_const_pointer_t<Allocator>;
  using reference = T&;
  using const_reference = const T&;
  using size_type = allocator_size_type_t<Allocator>;
  using difference_type = allocator_difference_type_t<Allocator>;
  using iterator = hub_detail::iterator<pointer>;
  using const_iterator = hub_detail::iterator<const_pointer>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>; 

  hub() noexcept(noexcept(Allocator())): hub{Allocator()} {}

  explicit hub(const Allocator& al_) noexcept: 
    allocator_base{empty_init, al_} {}

  explicit hub(size_type n, const Allocator& al_ = Allocator()): hub{al_}
  {
    range_insert_impl(size_type(0), n, [&, this] (T* p, size_type) {
      allocator_construct(al(), p);
    });
  }

  hub(size_type n, const T& x, const Allocator& al_ = Allocator()): hub{al_}
  {
    insert(n, x);
  }

  template<
    typename InputIterator, 
    typename = hub_detail::enable_if_is_input_iterator_t<InputIterator>
  >
  hub(
    InputIterator first, InputIterator last,
    const Allocator& al_ = Allocator()): hub{al_}
  {
    insert(first, last);
  }

#if !defined(BOOST_CONTAINER_HUB_NO_RANGES)
  template<hub_detail::container_compatible_range<T> R>
  hub(from_range_t, R&& rg, const Allocator& al_ = Allocator()): hub{al_}
  {
    insert_range(std::forward<R>(rg));
  }
#endif

  hub(const hub& x):
    hub{x, allocator_select_on_container_copy_construction(x.al())} {}

  hub(const hub& x, const hub_detail::type_identity_t<Allocator>& al_):
    hub(x.begin(), x.end(), al_) {}

  hub(hub&& x) noexcept:
    hub{std::move(x), Allocator(std::move(x.al())), std::true_type{}} {}

  hub(hub&& x, const hub_detail::type_identity_t<Allocator>& al_):
    hub{std::move(x), al_, allocator_is_always_equal_t<Allocator>{}} {}

  hub(std::initializer_list<T> il, const Allocator& al_ = Allocator()):
    hub{il.begin(), il.end(), al_} {}

  ~hub() { reset(); }

  hub& operator=(const hub& x)
  {
    using pocca =
      allocator_propagate_on_container_copy_assignment_t<Allocator>;

    if(this != &x) {
      if(al() != x.al() && pocca::value) {
        reset();
        hub_detail::copy_assign_if(pocca{}, al(), x.al());
        insert(x.begin(), x.end());
      }
      else{
        hub_detail::copy_assign_if(pocca{}, al(), x.al());
        assign(x.begin(), x.end());
      }
    }
    return *this;
  }

  hub& operator=(hub&& x)
    noexcept(
      allocator_propagate_on_container_move_assignment_t<Allocator>::value ||
      allocator_is_always_equal_t<Allocator>::value)
  {
    if(this != &x) {
      move_assign(
        x, 
        std::integral_constant<
          bool,
          allocator_propagate_on_container_move_assignment_t<Allocator>::
            value ||
          allocator_is_always_equal_t<Allocator>::value>{});
    }
    return *this;
  }

  hub& operator=(std::initializer_list<T> il)
  {
    assign(il);
    return *this;
  }

  template<
    typename InputIterator,
    typename = hub_detail::enable_if_is_input_iterator_t<InputIterator>
  >
  void assign(InputIterator first, InputIterator last)
  {
    range_assign_impl(
      first, last,
      [this] (T* p, InputIterator it) { allocator_construct(al(), p, *it); },
      [] (T* p, InputIterator it) { *p = *it; });
  }

#if !defined(BOOST_CONTAINER_HUB_NO_RANGES)
  template<hub_detail::container_compatible_range<T> R>
  void assign_range(R&& rg)
  {
    range_assign_impl(
      std::ranges::begin(rg), std::ranges::end(rg),
      [this] (T* p, auto it) { allocator_construct(al(), p, *it); },
      [] (T* p, auto it) { *p = *it; });
  }
#endif

  void assign(size_type n, const T& x)
  {
    range_assign_impl(
      size_type(0), n,
      [&, this] (T* p, size_type) { allocator_construct(al(), p, x); },
      [&] (T* p, size_type) { *p = x; });
  }

  void assign(std::initializer_list<T> il) { assign(il.begin(), il.end()); }

  allocator_type get_allocator() const noexcept { return al(); }

  iterator               begin() noexcept { return ++end(); }
  const_iterator         begin() const noexcept { return ++end(); }
  iterator               end() noexcept { return {blist.header(), 0}; }
  const_iterator         end() const noexcept { return {blist.header(), 0}; }
  reverse_iterator       rbegin() noexcept { return reverse_iterator{end()}; }
  const_reverse_iterator rbegin() const noexcept 
                         { return const_reverse_iterator{end()}; }
  reverse_iterator       rend() noexcept { return reverse_iterator{begin()}; }
  const_reverse_iterator rend() const noexcept 
                         { return const_reverse_iterator{begin()}; }
  const_iterator         cbegin() const noexcept { return begin(); }
  const_iterator         cend() const noexcept { return end(); }
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  const_reverse_iterator crend() const noexcept { return rend(); }

  bool      empty() const noexcept { return size_ == 0; }
  size_type size() const noexcept { return size_; }

  size_type max_size() const noexcept 
  {
    std::size_t
      bs = (std::size_t)allocator_max_size(al()) * sizeof(block),
      vs = (std::size_t)allocator_max_size(Allocator(al())) * sizeof(T);
    return 
      (size_type)((std::min)(bs, vs) / (sizeof(block) + sizeof(T) * N) * N);
  }
  
  size_type capacity() const noexcept { return num_blocks * N; }

  size_type memory() const noexcept // TODO: remove
  { 
    return num_blocks * (sizeof(block) + sizeof(T) * N); 
  }

  void reserve(size_type n)
  {
    if(n > max_size()) {
      BOOST_THROW_EXCEPTION(
        std::length_error("Requested capacity greater than max_size()"));
    }
    while(capacity() < n) (void)create_new_available_block();
  }

  void shrink_to_fit()
  {
    compact();
    trim_capacity();
  }

  void trim_capacity() noexcept { trim_capacity(0); }

  void trim_capacity(size_type n) noexcept
  {
    /* Linear on # available blocks, std::hive is linear on # _reserved_
     * blocks.
     */
    for(auto pbb = blist.header()->next_available;
        capacity() > n && pbb != blist.header(); ) {
      auto pb = static_cast_block_pointer(pbb);
      pbb = pbb-> next_available;
      if(pb->mask == 0) {
         blist.unlink_available(pb);
         delete_block(pb);
         --num_blocks;
      }
    }
  }

  template<typename... Args>
  BOOST_FORCEINLINE iterator emplace(Args&&... args)
  {
    int  n;
    auto pb = retrieve_available_block(n);
    allocator_construct(
      al(), boost::to_address(pb->data() + n), std::forward<Args>(args)...);
    pb->mask |= pb->mask + 1;
    if(BOOST_UNLIKELY(pb->mask + 1 <= 2)) {
      /* pb->mask == 0 (impossible), 1 or full */
      if(pb->mask == 1) blist.link_at_back(pb);
      else /* pb->mask == full */  blist.unlink_available(pb);
    }
    ++size_;
    return {pb, n};
  }

  template<typename... Args>
  BOOST_FORCEINLINE iterator emplace_hint(const_iterator, Args&&... args)
  {
    return emplace(std::forward<Args>(args)...);
  }

  BOOST_FORCEINLINE iterator insert(const T& x) { return emplace(x); }
  BOOST_FORCEINLINE iterator insert(const_iterator, const T& x)
                             { return emplace(x); }
  BOOST_FORCEINLINE iterator insert(T&& x) { return emplace(std::move(x)); }
  BOOST_FORCEINLINE iterator insert(const_iterator, T&& x) 
                             { return emplace(std::move(x)); }

  void insert(std::initializer_list<T> il) { insert(il.begin(), il.end()); }

#if !defined(BOOST_CONTAINER_HUB_NO_RANGES)
  template<hub_detail::container_compatible_range<T> R>
  void insert_range(R&& rg)
  {
    range_insert_impl(
      std::ranges::begin(rg), std::ranges::end(rg),
      [this] (T* p, auto it) { allocator_construct(al(), p, *it); });
  }
#endif

  template<
    typename InputIterator,
    typename = hub_detail::enable_if_is_input_iterator_t<InputIterator>
  >
  void insert(InputIterator first, InputIterator last)
  {
    range_insert_impl(first, last, [this] (T* p, InputIterator it) {
      allocator_construct(al(), p, *it);
    });
  }

  void insert(size_type n, const T& x)
  {
    range_insert_impl(size_type(0), n, [&, this] (T* p, size_type) {
      allocator_construct(al(), p, x);
    });
  }

  BOOST_FORCEINLINE iterator erase(const_iterator pos)
  {
    auto pbb = pos.pbb;
    auto n = pos.n;
    ++pos;
    erase_impl(pbb, n);
    return {pos.pbb, pos.n};
  }

  BOOST_FORCEINLINE void erase_void(const_iterator pos)
  {
    erase_impl(pos.pbb, pos.n);
  }

  iterator erase(const_iterator first, const_iterator last)
  {
    for(auto pbb = first.pbb; first != last; ) {
      first = erase(first);
      if(first.pbb != pbb) break;
    }
    auto pbb = first.pbb;
    if(pbb != last.pbb){
      do {
        auto pb = static_cast_block_pointer(pbb);
        pbb = pb->next;
        BOOST_CONTAINER_HUB_PREFETCH_BLOCK(pbb, block);
        size_ -= destroy_all_in_nonempty_block(pb);
        blist.unlink(pb);
        if(BOOST_UNLIKELY(pb->mask == full)) blist.link_available_at_front(pb);
        pb->mask = 0;
      } while(pbb != last.pbb);
      first = {pbb};
    }
    while(first != last) first = erase(first);
    return {last.pbb, last.n};
  }

  void swap(hub& x)
    noexcept(
      allocator_propagate_on_container_swap_t<Allocator>::value ||
      allocator_is_always_equal_t<Allocator>::value)
  {
    using pocs = allocator_propagate_on_container_swap_t<Allocator>;

    hub_detail::if_constexpr(pocs{}, [&, this]{
      hub_detail::swap_if(pocs{}, al(), x.al());
    },
    [&, this]{ /* else */
      BOOST_ASSERT(al() == x.al());
      (void)this;
    });
    std::swap(blist, x.blist);
    std::swap(num_blocks, x.num_blocks);
    std::swap(size_, x.size_);
  }

  void clear() noexcept { erase(begin(), end()); }

  void splice(hub& x)
  {
    BOOST_ASSERT(this != &x);
    BOOST_ASSERT(al() == x.al());
    for(auto pbb = x.blist.header()->next; pbb != x.blist.header(); ) {
      auto pb = static_cast_block_pointer(pbb);
      pbb = pbb->next;
      if(pb->mask != full) {
        x.blist.unlink_available(pb);
        blist.link_available_at_front(pb);
      }
      x.blist.unlink(pb);
      blist.link_at_back(pb);
      --x.num_blocks;
      ++num_blocks;
      auto s = core::popcount(pb->mask);
      x.size_ -= s;
      size_ += s;
    }
  }

  void splice(hub&& x) { splice(x); }

  template<typename BinaryPredicate = std::equal_to<T>>
  size_type unique(BinaryPredicate pred = BinaryPredicate())
  {
    auto s = size_;
    for(auto first = cbegin(), last = cend(); first != last; ) {
      auto next = std::next(first);
      first = erase(
        next,
        std::find_if_not(next, last, [&] (const T& x) {
          return pred(x, *first);
        }));
    }
    return (size_type)(s - size_);
  }

  template<typename Compare = std::less<T>>
  void sort(Compare comp = Compare())
  {
    /* transfer_sort is usually the fastest, but it consumes the most
     * auxiliary memory when sizeof(T) > sizeof(sort_proxy), so we restrict
     * its usage to the case sizeof(T) <= sizeof(sort_proxy).
     * compact_sort uses the least amount of auxiliary memory (by far), and
     * it's also faster than proxy_sort when the auxiliary memory of the latter
     * exceeds some threshold seemingly related to the size of the L2 cache;
     * we conventionally set the threshold to 2MB for lack of a more precise
     * estimation mechanism.
     * TODO: In 32-bit mode, the threshold policy is not so clear-cut and the
     * cost of moving elements around seems to play a role.
     */
    BOOST_IF_CONSTEXPR(sizeof(T) <= sizeof(sort_proxy)) {
      if(transfer_sort(comp)) return;
    }
    else{
      static constexpr std::size_t memory_threshold = 2 * 1024 * 1024;
      if((std::size_t)size_ * sizeof(sort_proxy) <= memory_threshold) {
        if(proxy_sort(comp)) return;
      }
    }
    compact_sort(comp);
  }

  iterator get_iterator(const_pointer p) noexcept /* noexcept? */
  {   
    std::less<const T*> less;
    for(auto pbb = blist.next; pbb != blist.header(); pbb = pbb-> next) {
      auto pb = static_cast_block_pointer(pbb);
      if(!less(boost::to_address(p), boost::to_address(pb->data())) &&
          less(boost::to_address(p), boost::to_address(pb->data() + N))) {
        return {pb, (int)(p - pb->data())};
      }
    }
    return end(); /* shouldn't assert? */
  }

  const_iterator get_iterator(const_pointer p) const noexcept /* noexcept? */
  {
    return const_cast<hub*>(this)->get_iterator(p);
  }

  template<typename F>
  void visit(iterator first, iterator last, F f)
  {
    visit_while(first, last, [&] (value_type& x) {
      f(x); 
      return true;
    });
  }

  template<typename F>
  void visit(const_iterator first, const_iterator last, F f) const
  {
    visit_while(first, last, [&] (const value_type& x) {
      f(x); 
      return true;
    });
  }

  template<typename F>
  iterator visit_while(iterator first, iterator last, F f)
  {
    for(auto pbb = first.pbb; first != last; ) {
      if(!f(*first)) return first;
      ++first;
      if(first.pbb != pbb) break;
    }
    if(first.pbb != last.pbb) {
      first = visit_while_impl(first.pbb, last.pbb, f);
      if(first.pbb != last.pbb) return first;
    }
    for(; first != last; ++first) if(!f(*first)) return first;
    return first;
  }

  template<typename F>
  const_iterator visit_while(
    const_iterator first, const_iterator last, F f) const
  {
    auto it =const_cast<hub*>(this)->visit_while(
      iterator{first.pbb, first.n}, iterator{last.pbb, last.n},
      [&] (const value_type& x) { return f(x); });
    return {it.pbb, it.n};
  }

  template<typename F>
  void visit_all(F f) 
  {
    visit(begin(), end(), std::ref(f)); 
  }

  template<typename F>
  void visit_all(F f) const
  {
    visit(begin(), end(), std::ref(f)); 
  }

  template<typename F>
  iterator visit_all_while(F f) 
  {
    return visit_while(begin(), end(), std::ref(f)); 
  }

  template<typename F>
  const_iterator visit_all_while(F f) const
  {
    return visit_while(begin(), end(), std::ref(f)); 
  }

private:
  template<typename U, typename A, typename P>
  friend typename hub<U, A>::size_type erase_if(hub<U, A>&, P);

  using block_typedefs = hub_detail::block_typedefs<Allocator>;
  using block_base = typename block_typedefs::block_base;
  using block_base_pointer = typename block_typedefs::block_base_pointer;
  using const_block_base_pointer = 
    typename block_typedefs::const_block_base_pointer;
  using block = typename block_typedefs::block;
  using block_pointer = typename block_typedefs::block_pointer;
  using block_allocator = typename block_typedefs::block_allocator;
  using block_list = typename block_typedefs::block_list;
  using allocator_base = empty_value<block_allocator, 0>;
  using mask_type = typename block_base::mask_type;

  static constexpr int N = block_base::N;
  static constexpr mask_type full = block_base::full;

  block_allocator&       al() noexcept { return allocator_base::get(); }
  const block_allocator& al() const noexcept { return allocator_base::get(); }

  struct reset_on_exit
  {
    ~reset_on_exit() { x.reset(); }

    hub& x;
  };

  hub(
    hub&& x, const Allocator& al_, std::true_type /* equal allocs */) noexcept:
    allocator_base{empty_init, al_}, blist{std::move(x.blist)},
    num_blocks{x.num_blocks}, size_{x.size_}
  {
    x.num_blocks = 0;
    x.size_ = 0;
  }

  hub(
    hub&& x, const Allocator& al_, std::false_type /* maybe unequal allocs */):
    hub{al_}
  {
    if(al() == x.al()) {
      blist = std::move(x.blist);
      num_blocks = x.num_blocks;
      size_ = x.size_;
      x.num_blocks = 0;
      x.size_ = 0;
    }
    else {
      reset_on_exit on_exit{x}; (void)on_exit;
      range_insert_impl(x.begin(), x.end(), [this] (T* p, iterator it) {
        allocator_construct(al(), p, std::move(*it));
      });
    }
  }

  void move_assign(hub& x, std::true_type /* transfer structure */)
  {
    using pocma =
      allocator_propagate_on_container_move_assignment_t<Allocator>;

    reset();
    hub_detail::move_assign_if(pocma{}, al(), x.al());
    blist = std::move(x.blist);
    num_blocks = x.num_blocks;
    size_ = x.size_;
    x.num_blocks = 0;
    x.size_ = 0;
  }

  void move_assign(hub& x, std::false_type /* maybe move data */)
  {
    if(al() == x.al()) {
      move_assign(x, std::true_type{});
    }
    else {
      reset_on_exit on_exit{x}; (void)on_exit;
      range_assign_impl(
        x.begin(), x.end(),
        [this] (T* p, iterator it) 
          { allocator_construct(al(), p, std::move(*it)); },
        [] (T* p, iterator it) 
          { *p = std::move(*it); });
    }
  }

  static block_pointer 
  static_cast_block_pointer(block_base_pointer pbb) noexcept
  {
    return block_list::static_cast_block_pointer(pbb);
  }

  block_pointer create_new_available_block()
  {
    auto pb = allocator_allocate(al(), 1);
    pb->mask = 0;
    BOOST_TRY {
      allocator_rebind_t<Allocator, value_type> val(al());
      pb->data_ = allocator_allocate(val, N);
    }
    BOOST_CATCH(...) {
      allocator_deallocate(al(), pb, 1);
      BOOST_RETHROW;
    }
    BOOST_CATCH_END
    blist.link_available_at_back(pb);
    ++num_blocks;
    return pb;
  }

  void delete_block(block_pointer pb) noexcept
  {
    allocator_rebind_t<Allocator, value_type> val(al());
    allocator_deallocate(val, pb->data(), N);
    allocator_deallocate(al(), pb, 1);
  }

  BOOST_FORCEINLINE block_pointer retrieve_available_block(int& n)
  {
    if(BOOST_LIKELY(blist.next_available != blist.header())){
      auto pb = static_cast_block_pointer(blist.next_available);
      n = hub_detail::unchecked_countr_one(pb->mask);
      return pb;
    }
    else {
      n = 0;
      return create_new_available_block();
    }
  }

  size_type destroy_all_in_nonempty_block(block_pointer pb) noexcept
  {
    BOOST_ASSERT(pb->mask != 0);
    return destroy_all_in_nonempty_block(pb, std::integral_constant<bool,
      std::is_trivially_destructible<T>::value &&
      ( hub_detail::is_std_allocator<block_allocator>::value ||
        hub_detail::is_std_pmr_polymorphic_allocator<block_allocator>::value ||
       !hub_detail::allocator_has_destroy<block_allocator, T*>::value )>{});
  }

  size_type destroy_all_in_nonempty_block(
    block_pointer pb, std::true_type /* trivial destruction */) noexcept
  {
    return (size_type)core::popcount(pb->mask);
  }

  size_type destroy_all_in_nonempty_block(
    block_pointer pb, std::false_type /* use allocator_destroy */) noexcept
  {
    size_type s = 0;
    auto      mask = pb->mask;
    do {
      auto n = hub_detail::unchecked_countr_zero(mask);
      allocator_destroy(al(), boost::to_address(pb->data() + n));
      ++s;
      mask &= mask - 1;
    } while(mask);
    return s;
  }

  size_type destroy_all_in_full_block(block_pointer pb) noexcept
  {
    BOOST_ASSERT(pb->mask == full);
    for(int n = 0; n < N; ++n) {
      allocator_destroy(al(), boost::to_address(pb->data() + n));
    }
    return (size_type)N;
  }

  void reset() noexcept
  {
    for(auto pbb = blist.next_available; pbb != blist.header(); ) {
      auto pb = static_cast_block_pointer(pbb);
      pbb = pb->next_available;
      BOOST_IF_CONSTEXPR(!std::is_trivially_destructible<T>::value) {
        BOOST_CONTAINER_HUB_PREFETCH_BLOCK(pbb, block);
      }
      if(pb->mask != 0) {
        destroy_all_in_nonempty_block(pb);
        blist.unlink(pb);
      }
      delete_block(pb);
    }
    /* full blocks remaining */
    for(auto pbb = blist.next; pbb != blist.header(); ) {
      BOOST_ASSERT(pbb->mask == full);
      auto pb = static_cast_block_pointer(pbb);
      pbb = pb->next;
      BOOST_IF_CONSTEXPR(!std::is_trivially_destructible<T>::value) {
        BOOST_CONTAINER_HUB_PREFETCH_BLOCK(pbb, block);
      }
      destroy_all_in_full_block(pb);
      delete_block(pb);
    }
    blist.reset();
    num_blocks = 0;
    size_ = 0;
  }

  BOOST_FORCEINLINE void erase_impl(block_base_pointer pbb, int n) noexcept
  {
    auto pb = static_cast_block_pointer(pbb);
    allocator_destroy(al(), boost::to_address(pb->data() + n));
    if(BOOST_UNLIKELY(pb->mask == full)) blist.link_available_at_front(pb);
    pb->mask &= ~((mask_type)(1) << n);
    if(BOOST_UNLIKELY(pb->mask == 0)) blist.unlink(pb);
    --size_;
  }

  template<typename Incrementable, typename Sentinel, typename Construct>
  void range_insert_impl(
    Incrementable first, Sentinel last, Construct construct)
  {
    while(first != last) {
      int  n;
      auto pb = retrieve_available_block(n);
      for(; ; ) {
        construct(boost::to_address(pb->data() + n), first++);
        ++size_;
        if(BOOST_UNLIKELY(pb->mask == 0)) blist.link_at_back(pb);
        pb->mask |= pb->mask +1;
        if(pb->mask == full){
          blist.unlink_available(pb);
          break;
        }
        else if(first == last) return;
        n = hub_detail::unchecked_countr_one(pb->mask);
      }
    }
  }

  template<
    typename Incrementable, typename Sentinel, 
    typename Construct, typename Insert
  >
  void range_assign_impl(
    Incrementable first, Sentinel last, Construct construct, Insert insert)
  {
    auto pbb = blist.next;
    int  n = 0;
    if(first != last) {
      /* consume active blocks */
      for(; pbb != blist.header(); pbb = pbb->next, n = 0) {
        auto pb = static_cast_block_pointer(pbb);
        for(mask_type bit = 1; bit; bit <<= 1, ++n) {
          if(pb->mask & bit) { /* full slot */
            insert(boost::to_address(pb->data() + n), first++);
          }
          else { /* empty slot */
            construct(boost::to_address(pb->data() + n), first++);
            ++size_;
            pb->mask |= bit;
            if(pb->mask == full) blist.unlink_available(pb);
          }
          if(first == last) goto exit;
        }
      }
    exit: ;
    }
    if(first != last) {
      /* all active blocks consumed, keep inserting */
      range_insert_impl(first, last, construct);
    }
    else{
      /* erase remaining original elements */
      auto it = (n == 0)? const_iterator{pbb}: ++const_iterator{pbb, n};
      erase(it, cend());
    }
  }

  template<typename Compare>
  bool transfer_sort(Compare comp)
  {
    /* transfer to a buffer, sort and transfer back */
    hub_detail::buffer<T,Allocator> buf(size_, al());
    if(!buf.data) return false;

    visit_all([&] (value_type& x) { buf.emplace_back(std::move(x)); });
    std::sort(buf.begin(), buf.end(), comp);
    visit_all([&] (value_type& x) { 
      x = std::move(*buf.begin());
      buf.erase_front();
    });
    return true;
  }

  struct sort_proxy
  {
    T*        p;
    size_type n;
  };

  template<typename Compare>
  bool proxy_sort(Compare comp)
  {
    /* sort an array of (pointer, index) pairs and relocate according to it */
    if(size_ > 1) {
      hub_detail::nodtor_unique_ptr<sort_proxy[]> p
        {static_cast<sort_proxy*>(
          ::operator new[](size_ * sizeof(sort_proxy), std::nothrow))};
      if(!p) return false;

      size_type i = 0;
      visit_all([&] (value_type& x) {
        p[i] = {std::addressof(x), i};
        ++i;
      });

      std::sort(
        p.get(), p.get() + size_, 
        [&] (const sort_proxy& x, const sort_proxy& y) {
          return comp(const_cast<const T&>(*x.p), const_cast<const T&>(*y.p));
        });

      i = 0;
      for(; i < size_; ++i) {
        if(p[i].n != i) {
          T    x = std::move(*(p[i].p));
          auto j = i;
          do {
            auto k = p[j].n;
            *(p[j].p) = std::move(*p[k].p);
            p[j].n = j;
            j = k;
          } while(p[j].n != i);
          *(p[j].p) = std::move(x);
          p[j].n = j;
        }
      }
    }
    return true;
  }

  template<typename Compare>
  void compact_sort(Compare comp)
  {
    /* compact elements and build an array of pointers to data chunks of N */
    using sort_iterator = hub_detail::sort_iterator<T, static_cast<std::size_t>(N)>;

    if(size_ > 1) {
      std::size_t n = (std::size_t)((size_ + N - 1) / N);
      hub_detail::nodtor_unique_ptr<T*[]> p
        {static_cast<T**>(::operator new[](n * sizeof(T*)))};
      std::size_t i = 0;
      compact([&] (block_pointer pb) { 
        p[i++] = boost::to_address(pb->data()); 
      });
      BOOST_ASSERT(i == n);

      std::sort(
        sort_iterator{p.get(), 0}, sort_iterator{p.get(), size_}, comp);
    }
  }

  void compact() { compact([] (block_pointer) {}); }

  template<typename Track>
  void compact(Track track)
  {
    for(auto pbbx = blist.next; pbbx != blist.header(); ) {
      auto pbx = static_cast_block_pointer(pbbx);
      auto pbby = pbbx->next;
      if(pbx->mask != full) {
        do{
          if(pbby->mask == full) {
            do {
              track(static_cast_block_pointer(pbby));
              pbby = pbby->next;
            } while(pbby->mask == full);
            blist.unlink(pbx);
            blist.link_before(pbx, static_cast_block_pointer(pbby));
          }
          if(pbby == blist.header()) {
            compact(pbx);
            track(pbx);
            return;
          }
          else{
            auto pby = static_cast_block_pointer(pbby);
            compact(pbx,pby);
            if(pby->mask == 0) {
              pbby = pby->next;
              blist.unlink(pby);
            }
          }
        }while(pbx->mask != full);
        blist.unlink_available(pbx);
      }
      track(pbx);
      pbbx = pbby;
    }
  }

  void compact(block_pointer& pbx, block_pointer& pby)
  {
    auto cx = core::popcount(pbx->mask),
         cy = core::popcount(pby->mask);
    if(cx < cy) {
      std::swap(cx, cy);
      swap_payload(*pbx, *pby);
    }
    auto c = (std::min)(N - cx, cy);
    while(c--) {
      auto n = hub_detail::unchecked_countr_one(pbx->mask);
      auto m = N - 1 - hub_detail::unchecked_countl_zero(pby->mask);
      allocator_construct(
        al(), boost::to_address(pbx->data() + n), std::move(pby->data()[m]));
      allocator_destroy(al(), boost::to_address(pby->data() + m));
      pbx->mask |= pbx->mask + 1;
      pby->mask &= ~((mask_type)(1) << m);
    }
  }

  void compact(block_pointer pb)
  {
    for(; ;) {
      auto n = hub_detail::unchecked_countr_one(pb->mask);
      auto m = N - 1 - hub_detail::unchecked_countl_zero(pb->mask);
      if(n > m) return;
      allocator_construct(
        al(), boost::to_address(pb->data() + n), std::move(pb->data()[m]));
      allocator_destroy(al(), boost::to_address(pb->data() + m));
      pb->mask |= pb->mask + 1;
      pb->mask &= ~((mask_type)(1) << m);
    }
  }

  template<typename F>
  iterator visit_while_impl(
    block_base_pointer pbb, block_base_pointer last_pbb, F&& f)
  {
    BOOST_ASSERT(pbb != last_pbb);
    auto           pb = static_cast_block_pointer(pbb);
    auto           mask = pb->mask;
    auto           n = hub_detail::unchecked_countr_zero(mask);
    auto           pd = pb->data();
    do {
      pbb = pb->next;
      auto next_mask = pbb->mask;
      auto next_n = hub_detail::unchecked_countr_zero(next_mask);
      auto next_pd = static_cast_block_pointer(pbb)->data();
      BOOST_CONTAINER_HUB_PREFETCH(next_pd + next_n);
      BOOST_CONTAINER_HUB_PREFETCH(pbb->next);
      for(; ; ) {
        if(!f(pd[n])) return {pb, n};
        mask &= mask - 1;
        if(!mask) break;
        n = hub_detail::unchecked_countr_zero(mask);
      }
      pb = static_cast_block_pointer(pbb);
      mask = next_mask;
      n = next_n;
      pd = next_pd;
    } while(pb != last_pbb);
    return {last_pbb};
  }

  block_list blist;
  size_type  num_blocks = 0;
  size_type  size_ = 0;
};

#if !defined(BOOST_NO_CXX17_DEDUCTION_GUIDES)
template<
  typename InputIterator, 
  typename Allocator = std::allocator<
    typename std::iterator_traits<InputIterator>::value_type>
>
hub(InputIterator, InputIterator, Allocator = Allocator())
  -> hub<
    typename std::iterator_traits<InputIterator>::value_type, Allocator>;

#if !defined(BOOST_CONTAINER_HUB_NO_RANGES)
template<
  std::ranges::input_range R,
  typename Allocator = std::allocator<std::ranges::range_value_t<R>>
>
hub(from_range_t, R&&, Allocator = Allocator())
  -> hub<std::ranges::range_value_t<R>, Allocator>;
#endif
#endif

template<typename T, typename Allocator>
void swap(hub<T, Allocator>& x, hub<T, Allocator>& y)
  noexcept(noexcept(x.swap(y)))
{
  x.swap(y);
}

template<typename T, typename Allocator, typename Predicate>
typename hub<T, Allocator>::size_type
erase_if(hub<T, Allocator>& x, Predicate pred)
{
  using hub_container = hub<T, Allocator>;
  using size_type = typename hub_container::size_type;
  using block = typename hub_container::block;

  auto s = x.size_;
  for(auto pbb = x.blist.next; pbb != x.blist.header(); ) {
    auto pb = x.static_cast_block_pointer(pbb);
    pbb = pb->next;
    BOOST_CONTAINER_HUB_PREFETCH_BLOCK(pbb, block);
    auto mask = pb->mask;
    do {
      auto n = hub_detail::unchecked_countr_zero(mask);
      if(pred(pb->data()[n])) x.erase_impl(pb, n);
      mask &= mask - 1;
    } while(mask);
  }
  return (size_type)(s - x.size_);
}

template<typename T, typename Allocator, typename U = T>
typename hub<T, Allocator>::size_type
erase(hub<T, Allocator>& x, const U& value)
{
  return erase_if(x, [&](const T& v) -> bool { return v == value; });
}

} /* namespace container */

} /* namespace boost */

#if defined(BOOST_MSVC)
#pragma warning(pop) /* C4714 */
#endif

#endif
