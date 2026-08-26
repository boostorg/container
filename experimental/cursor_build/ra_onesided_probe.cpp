// Assembly / codegen probe: one-sided vs both-sided random-access leaves.
// Not part of the library — exploration only. Compile with -O3 -S.
#include <cstddef>
#include <iterator>

struct FwdInt {
   int* p;
   using iterator_category = std::forward_iterator_tag;
   using value_type = int;
   using difference_type = std::ptrdiff_t;
   using pointer = int*;
   using reference = int&;
   int& operator*() const { return *p; }
   FwdInt& operator++() { ++p; return *this; }
   FwdInt operator++(int) { FwdInt t = *this; ++p; return t; }
   friend bool operator==(FwdInt a, FwdInt b) { return a.p == b.p; }
   friend bool operator!=(FwdInt a, FwdInt b) { return a.p != b.p; }
};

struct BiInt {
   int* p;
   using iterator_category = std::bidirectional_iterator_tag;
   using value_type = int;
   using difference_type = std::ptrdiff_t;
   using pointer = int*;
   using reference = int&;
   int& operator*() const { return *p; }
   BiInt& operator++() { ++p; return *this; }
   BiInt operator++(int) { BiInt t = *this; ++p; return t; }
   BiInt& operator--() { --p; return *this; }
   BiInt operator--(int) { BiInt t = *this; --p; return t; }
   friend bool operator==(BiInt a, BiInt b) { return a.p == b.p; }
   friend bool operator!=(BiInt a, BiInt b) { return a.p != b.p; }
};

// ---- copy leaves ----------------------------------------------------------
extern "C" {

// Generic bounded leaf (current fallback): two iterator exit conditions.
__attribute__((noinline))
int* copy_generic_bounded(int* first, int* last, int* d, int* dlast)
{
   for (; first != last; ++first) {
      if (d == dlast) break;
      *d = *first;
      ++d;
   }
   return d;
}

// Both-RA: min then single-exit counted loop (current RA specialization).
__attribute__((noinline))
int* copy_both_ra(int* first, int* last, int* d, int* dlast)
{
   const std::ptrdiff_t n = (last - first) < (dlast - d) ? (last - first) : (dlast - d);
   int* const fend = first + n;
   for (; first != fend; ++first, ++d)
      *d = *first;
   return d;
}

// RA destination only, forward source: countdown on destination capacity.
__attribute__((noinline))
int* copy_ra_dst_only(FwdInt first, FwdInt last, int* d, int* dlast)
{
   std::ptrdiff_t n = dlast - d;
   for (; n && first != last; --n, ++first, ++d)
      *d = *first;
   return d;
}

// RA source only, bidirectional/bounded destination (cannot O(1) size dest).
__attribute__((noinline))
int* copy_ra_src_only_bounded(int* first, int* last, BiInt d, BiInt dlast)
{
   std::ptrdiff_t n = last - first;
   for (; n && d != dlast; --n, ++first, ++d)
      *d = *first;
   return d.p;
}

// RA source + unbounded destination (unreachable sentinel folded away).
__attribute__((noinline))
int* copy_ra_src_unbounded(int* first, int* last, int* d)
{
   for (; first != last; ++first, ++d)
      *d = *first;
   return d;
}

// Same but expressed as counted loop (what a one-sided RA-src leaf would do).
__attribute__((noinline))
int* copy_ra_src_counted(int* first, int* last, int* d)
{
   for (std::ptrdiff_t n = last - first; n; --n, ++first, ++d)
      *d = *first;
   return d;
}

// ---- copy_if leaves -------------------------------------------------------

inline bool is_odd(int x) { return (x & 1) != 0; }

__attribute__((noinline))
int* copy_if_generic(int* first, int* last, int* d, int* dlast)
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

// Both-RA block amortize (current).
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

// RA-dst only: can amortize destination room, but must still check first!=last
// every element (no O(1) source length). Block of B only when dst has B room;
// source exhaustion checked inside the block.
__attribute__((noinline))
int* copy_if_ra_dst_only(FwdInt first, FwdInt last, int* d, int* dlast)
{
   const std::ptrdiff_t B = 16;
   if (d == dlast) return d;
   while ((dlast - d) >= B && first != last) {
      // Process up to B source elements or until source ends.
      for (std::ptrdiff_t c = B; c && first != last; --c, ++first) {
         if (is_odd(*first)) { *d = *first; ++d; }
      }
      // If we exited early because source ended, stop.
      // If we wrote fewer than B, destination still has room — continue only
      // if source remains. The inner loop already stopped on source end.
      if (first == last) return d;
      // Destination may have filled partially; re-check outer while condition.
   }
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

// RA-src only + unbounded dest: counted miss-friendly scan, no dest check.
__attribute__((noinline))
int* copy_if_ra_src_unbounded(int* first, int* last, int* d)
{
   for (std::ptrdiff_t n = last - first; n; --n, ++first) {
      if (is_odd(*first)) { *d = *first; ++d; }
   }
   return d;
}

// ---- equal / iter2 leaves -------------------------------------------------

__attribute__((noinline))
bool equal_generic(int* a, int* aend, int* b, int* bend)
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
   return a == aend; // true iff lengths equal and all matched (stop semantics)
}

// RA range1 only: countdown on known length; still compare b != bend.
__attribute__((noinline))
bool equal_ra_src_only(int* a, int* aend, FwdInt b, FwdInt bend)
{
   for (std::ptrdiff_t n = aend - a; n; --n, ++a, ++b) {
      if (b == bend) return false;
      if (*a != *b) return false;
   }
   return true;
}

// RA range2 only.
__attribute__((noinline))
bool equal_ra_dst_only(FwdInt a, FwdInt aend, int* b, int* bend)
{
   for (std::ptrdiff_t n = bend - b; n && a != aend; --n, ++a, ++b)
      if (*a != *b) return false;
   return a == aend;
}

// ---- merge-style room check (needs all RA for block) ----------------------

// With only dest RA: can ask (dlast-d) >= B, but still need both sources alive.
__attribute__((noinline))
int* merge_ra_dst_block(BiInt a, BiInt aend, BiInt b, BiInt bend, int* d, int* dlast)
{
   const std::ptrdiff_t B = 32;
   while ((dlast - d) >= B && a != aend && b != bend) {
      for (std::ptrdiff_t c = B; c; --c) {
         if (*b < *a) { *d = *b; ++b; }
         else         { *d = *a; ++a; }
         ++d;
         // Sources may end mid-block — unsafe without RA lengths.
         // This version is intentionally WRONG if a/b end mid-block;
         // kept only to show why one-sided RA is insufficient for merge_blocks.
         (void)aend; (void)bend;
      }
   }
   return d;
}

} // extern "C"

int main() { return 0; }
