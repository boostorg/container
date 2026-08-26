// Leaf-contract probe for the four set_* destination leaves with the flag.
//
// The walkers replace `first1 == last1 || first2 == last2` by `r.fourth`, so
// the flag must satisfy, for every possible leaf invocation,
//
//    r.fourth == (r.first == last1 || r.second == last2)
//
// including the tie where the destination runs out on the very same element
// that exhausts a source: there the flag must say "a source ran out", because
// a destination walker that instead stepped to the next segment would walk off
// the end of the destination.  (segmented_mismatch got exactly this kind of
// tie rule wrong once.)
//
// Also checks that the reworked leaves do the same work as the pre-change
// ones: set_union's destination-full test moves out of the loop condition, so
// its loop is compared against a verbatim copy of the original.
//
// Both the generic leaf (SrcCat = int) and the dual-RA blocked fast path
// (SrcCat = random_access_iterator_tag) are exercised, since the fast path
// forwards the flag from the generic leaf it tails into.

#include <iterator>
#include <utility>
#include <cstddef>
#include <cstdio>
#include <cassert>

#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_set_union.hpp>
#include <boost/container/experimental/segmented_set_difference.hpp>
#include <boost/container/experimental/segmented_set_intersection.hpp>
#include <boost/container/experimental/segmented_set_symmetric_difference.hpp>

namespace bc = boost::container;
namespace da = boost::container::detail_algo;

struct less_t { bool operator()(int a, int b) const { return a < b; } };

typedef bc::segtrio<const int*, const int*, int*> ref_t;

// ---- verbatim copies of the pre-change leaves, as work references ----------

ref_t ref_union(const int* f1, const int* l1, const int* f2, const int* l2,
                int* d, const int* dl, less_t comp)
{
   while(f1 != l1 && f2 != l2 && d != dl) {
      if      (comp(*f1, *f2)) { *d = *f1; ++f1; }
      else if (comp(*f2, *f1)) { *d = *f2; ++f2; }
      else                     { *d = *f1; ++f1; ++f2; }
      ++d;
   }
   return ref_t(f1, f2, d);
}

ref_t ref_difference(const int* f1, const int* l1, const int* f2, const int* l2,
                     int* d, const int* dl, less_t comp)
{
   while(f1 != l1 && f2 != l2) {
      if (comp(*f1, *f2)) {
         if(d == dl) break;
         *d = *f1; ++f1; ++d;
      }
      else {
         if (!comp(*f2, *f1)) ++f1;
         ++f2;
      }
   }
   return ref_t(f1, f2, d);
}

ref_t ref_intersection(const int* f1, const int* l1, const int* f2, const int* l2,
                       int* d, const int* dl, less_t comp)
{
   while(f1 != l1 && f2 != l2) {
      if      (comp(*f1, *f2)) { ++f1; }
      else if (comp(*f2, *f1)) { ++f2; }
      else {
         if(d == dl) break;
         *d = *f1; ++f1; ++f2; ++d;
      }
   }
   return ref_t(f1, f2, d);
}

ref_t ref_symmetric(const int* f1, const int* l1, const int* f2, const int* l2,
                    int* d, const int* dl, less_t comp)
{
   while(f1 != l1 && f2 != l2) {
      if (comp(*f1, *f2)) {
         if(d == dl) break;
         *d = *f1; ++f1; ++d;
      }
      else {
         if (comp(*f2, *f1)) {
            if(d == dl) break;
            *d = *f2; ++d;
         }
         else {
            ++f1;
         }
         ++f2;
      }
   }
   return ref_t(f1, f2, d);
}

// ---- enumeration of every non-decreasing sequence over {0,1,2} up to len 4 --

const int  ALPHA = 3;
const int  LMAX  = 4;
const int  DMAX  = 6;

int seqs[64][LMAX];
int seq_len[64];
int nseq = 0;

void gen(int len, int start, int* buf, int at)
{
   if(at == len) {
      for(int i = 0; i < len; ++i) seqs[nseq][i] = buf[i];
      seq_len[nseq] = len;
      ++nseq;
      return;
   }
   for(int v = start; v < ALPHA; ++v) { buf[at] = v; gen(len, v, buf, at + 1); }
}

long cases = 0, ties = 0, flags_set = 0;

#define CHECK_LEAF(NAME, REF, SRCCAT)                                          \
{                                                                              \
   int out[DMAX + 2], rout[DMAX + 2];                                          \
   for(int k = 0; k < DMAX + 2; ++k) { out[k] = rout[k] = -7; }                \
   const bc::segquartet<const int*, const int*, int*, bool> r =                \
      (da::NAME##_dst_bounded)(a, a + na, b, b + nb, out, out + cap,           \
                               less_t(), bc::non_segmented_iterator_tag(),     \
                               SRCCAT);                                        \
   const ref_t rr = REF(a, a + na, b, b + nb, rout, rout + cap, less_t());     \
   assert(r.first  - a   == rr.first  - a);                                    \
   assert(r.second - b   == rr.second - b);                                    \
   assert(r.third  - out == rr.third  - rout);                                 \
   for(int k = 0; k < DMAX + 2; ++k) assert(out[k] == rout[k]);                \
   const bool compound = (r.first == a + na) || (r.second == b + nb);          \
   assert(r.fourth == compound);                                               \
   if(r.fourth) ++flags_set;                                                   \
   if(compound && r.third == out + cap && cap != 0) ++ties;                    \
   ++cases;                                                                    \
}

int main()
{
   int buf[LMAX];
   for(int len = 0; len <= LMAX; ++len) gen(len, 0, buf, 0);

   for(int i = 0; i < nseq; ++i) {
      const int* a  = seqs[i];
      const int  na = seq_len[i];
      for(int j = 0; j < nseq; ++j) {
         const int* b  = seqs[j];
         const int  nb = seq_len[j];
         for(int cap = 0; cap <= DMAX; ++cap) {
            CHECK_LEAF(set_union,                ref_union,      int())
            CHECK_LEAF(set_difference,           ref_difference, int())
            CHECK_LEAF(set_intersection,         ref_intersection, int())
            CHECK_LEAF(set_symmetric_difference, ref_symmetric,  int())
            CHECK_LEAF(set_union,                ref_union,      std::random_access_iterator_tag())
            CHECK_LEAF(set_difference,           ref_difference, std::random_access_iterator_tag())
            CHECK_LEAF(set_intersection,         ref_intersection, std::random_access_iterator_tag())
            CHECK_LEAF(set_symmetric_difference, ref_symmetric,  std::random_access_iterator_tag())
         }
      }
   }
   std::printf("223_sets_contract: %ld leaf calls OK (%ld flag-set, %ld source/dst ties)\n",
               cases, flags_set, ties);
   return 0;
}
