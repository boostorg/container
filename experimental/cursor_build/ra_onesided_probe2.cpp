// Honest one-sided RA probe: non-RA side is a linked-list forward iterator
// so the compiler cannot recover O(1) distance via pointer subtraction.
#include <cstddef>
#include <iterator>

struct Node { int value; Node* next; };

struct ListIt {
   Node* n;
   using iterator_category = std::forward_iterator_tag;
   using value_type = int;
   using difference_type = std::ptrdiff_t;
   using pointer = int*;
   using reference = int&;
   int& operator*() const { return n->value; }
   ListIt& operator++() { n = n->next; return *this; }
   ListIt operator++(int) { ListIt t = *this; n = n->next; return t; }
   friend bool operator==(ListIt a, ListIt b) { return a.n == b.n; }
   friend bool operator!=(ListIt a, ListIt b) { return a.n != b.n; }
};

extern "C" {

// Generic: RA pointers both sides, two-exit (baseline for both-RA containers
// when specialization is off).
__attribute__((noinline))
int* copy_generic_ptr(int* first, int* last, int* d, int* dlast)
{
   for (; first != last; ++first) {
      if (d == dlast) break;
      *d = *first;
      ++d;
   }
   return d;
}

__attribute__((noinline))
int* copy_both_ra(int* first, int* last, int* d, int* dlast)
{
   const std::ptrdiff_t n =
      (last - first) < (dlast - d) ? (last - first) : (dlast - d);
   int* const fend = first + n;
   for (; first != fend; ++first, ++d)
      *d = *first;
   return d;
}

// RA dst only: list source. Countdown on destination capacity.
__attribute__((noinline))
int* copy_ra_dst_list_src(ListIt first, ListIt last, int* d, int* dlast)
{
   for (std::ptrdiff_t n = dlast - d; n && first != last; --n, ++first, ++d)
      *d = *first;
   return d;
}

// Same shape as generic but with list source (no RA on either conceptually for
// the source exit; dest still RA compared as iterators).
__attribute__((noinline))
int* copy_generic_list_src(ListIt first, ListIt last, int* d, int* dlast)
{
   for (; first != last; ++first) {
      if (d == dlast) break;
      *d = *first;
      ++d;
   }
   return d;
}

// RA src only: list destination (write through list nodes — unusual but models
// non-RA bounded output).
__attribute__((noinline))
ListIt copy_ra_src_list_dst(int* first, int* last, ListIt d, ListIt dlast)
{
   for (std::ptrdiff_t n = last - first; n && d != dlast; --n, ++first, ++d)
      *d = *first;
   return d;
}

__attribute__((noinline))
ListIt copy_generic_list_dst(int* first, int* last, ListIt d, ListIt dlast)
{
   for (; first != last; ++first) {
      if (d == dlast) break;
      *d = *first;
      ++d;
   }
   return d;
}

// RA src + unbounded dest (sentinel folded): single-exit; vectorises today
// without needing dest to be RA.
__attribute__((noinline))
int* copy_ra_src_unbounded(int* first, int* last, int* d)
{
   for (; first != last; ++first, ++d)
      *d = *first;
   return d;
}

// ---- copy_if --------------------------------------------------------------

inline bool is_odd(int x) { return (x & 1) != 0; }

__attribute__((noinline))
int* copy_if_generic_ptr(int* first, int* last, int* d, int* dlast)
{
   if (d == dlast) return d;
   for (; first != last; ++first) {
      if (is_odd(*first)) {
         *d = *first;
         ++d;
         if (d == dlast) { ++first; break; }
      }
   }
   return d;
}

__attribute__((noinline))
int* copy_if_both_ra(int* first, int* last, int* d, int* dlast)
{
   const std::ptrdiff_t B = 16;
   std::ptrdiff_t n = last - first;
   if (d == dlast) return d;
   while (n >= B && (dlast - d) >= B) {
      n -= B;
      for (std::ptrdiff_t c = B; c; --c, ++first) {
         if (is_odd(*first)) { *d = *first; ++d; }
      }
   }
   if (d == dlast) return d;
   for (; n; --n, ++first) {
      if (is_odd(*first)) {
         *d = *first;
         ++d;
         if (d == dlast) { ++first; break; }
      }
   }
   return d;
}

// RA-dst only with list source: amortize dest room with block of B, but each
// element still checks first != last (list walk). Safety: never process a
// fixed B without knowing source length — so block is "up to B" with early exit.
__attribute__((noinline))
int* copy_if_ra_dst_list_src(ListIt first, ListIt last, int* d, int* dlast)
{
   const std::ptrdiff_t B = 16;
   if (d == dlast) return d;
   while ((dlast - d) >= B && first != last) {
      for (std::ptrdiff_t c = B; c && first != last; --c, ++first) {
         if (is_odd(*first)) { *d = *first; ++d; }
      }
   }
   if (d == dlast || first == last) return d;
   for (; first != last; ++first) {
      if (is_odd(*first)) {
         *d = *first;
         ++d;
         if (d == dlast) { ++first; break; }
      }
   }
   return d;
}

// RA-src only, unbounded dest (flat output / sentinel).
__attribute__((noinline))
int* copy_if_ra_src_unbounded(int* first, int* last, int* d)
{
   for (std::ptrdiff_t n = last - first; n; --n, ++first)
      if (is_odd(*first)) { *d = *first; ++d; }
   return d;
}

// ---- equal / mismatch style ----------------------------------------------

__attribute__((noinline))
bool equal_generic_ptr(int* a, int* aend, int* b, int* bend)
{
   for (; a != aend; ++a) {
      if (b == bend) return false;
      if (*a != *b) return false;
      ++b;
   }
   return true;
}

__attribute__((noinline))
bool equal_both_ra(int* a, int* aend, int* b, int* bend)
{
   std::ptrdiff_t n = (aend - a) < (bend - b) ? (aend - a) : (bend - b);
   int* const a2 = a + n;
   for (; a != a2; ++a, ++b)
      if (*a != *b) return false;
   return a == aend;
}

__attribute__((noinline))
bool equal_ra_a_list_b(int* a, int* aend, ListIt b, ListIt bend)
{
   for (std::ptrdiff_t n = aend - a; n; --n, ++a, ++b) {
      if (b == bend) return false;
      if (*a != *b) return false;
   }
   return true;
}

__attribute__((noinline))
bool equal_list_a_ra_b(ListIt a, ListIt aend, int* b, int* bend)
{
   for (std::ptrdiff_t n = bend - b; n && a != aend; --n, ++a, ++b)
      if (*a != *b) return false;
   return a == aend;
}

__attribute__((noinline))
bool equal_generic_list_b(int* a, int* aend, ListIt b, ListIt bend)
{
   for (; a != aend; ++a) {
      if (b == bend) return false;
      if (*a != *b) return false;
      ++b;
   }
   return true;
}

} // extern "C"

int main() { return 0; }
