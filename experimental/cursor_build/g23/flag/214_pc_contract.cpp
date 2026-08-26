// Leaf-contract probe for the partition_copy flag flip.
//
// The walkers replace `first == last || r.fourth` (old flag: "out_true
// blocked") by `!r.fourth` (new flag: "[f_first, f_last) filled").  That
// substitution is only valid if, for every possible leaf invocation,
//
//    new_flag == !(first == last || old_flag)
//
// with `first` the leaf's returned source position.  old_leaf below is a
// verbatim copy of the pre-change leaf loop, so the two are compared directly
// over every source length, predicate bit pattern and pair of output
// capacities up to 6.  Also checks the tie rule explicitly: a drained source
// must never report the out_false range as resumably full, even when the
// out_false range ran out on the very same element.

#include <iterator>
#include <utility>
#include <cstddef>
#include <cstdio>
#include <cassert>

#include <boost/container/experimental/segmented_iterator_traits.hpp>
#include <boost/container/experimental/segmented_partition_copy.hpp>

namespace bc = boost::container;

struct odd_t { bool operator()(int a) const { return (a & 1) != 0; } };

// Pre-change leaf, kept only as the reference for the equivalence assertion.
template <class SrcIter, class Sent, class TIter, class TSent, class FIter, class FSent, class Pred>
bc::segquartet<SrcIter, TIter, FIter, bool>
old_leaf(SrcIter first, Sent last, TIter t_first, TSent t_last,
         FIter f_first, FSent f_last, Pred pred)
{
   bool true_output_full = false;
   for(; first != last; ++first) {
      if(pred(*first)) {
         if(t_first == t_last) { true_output_full = true; break; }
         *t_first = *first;
         ++t_first;
      }
      else {
         if(f_first == f_last) break;
         *f_first = *first;
         ++f_first;
      }
   }
   return bc::segquartet<SrcIter, TIter, FIter, bool>
      (first, t_first, f_first, true_output_full);
}

int main()
{
   const int NMAX = 6;
   long cases = 0, ties = 0;
   int src[NMAX], tout[NMAX + 1], fout[NMAX + 1], tref[NMAX + 1], fref[NMAX + 1];

   for(int n = 0; n <= NMAX; ++n) {
      for(long mask = 0; mask < (1L << n); ++mask) {
         for(int i = 0; i < n; ++i)
            src[i] = ((mask >> i) & 1) ? (2 * i + 1) : (2 * i);   // odd == "true"
         for(int tcap = 0; tcap <= NMAX; ++tcap) {
            for(int fcap = 0; fcap <= NMAX; ++fcap) {
               for(int k = 0; k <= NMAX; ++k) { tout[k] = fout[k] = -1; tref[k] = fref[k] = -1; }

               const bc::segquartet<const int*, int*, int*, bool> nw =
                  bc::detail_algo::partition_copy_leaf
                     (static_cast<const int*>(src), src + n,
                      tout, tout + tcap, fout, fout + fcap, odd_t(), int());
               const bc::segquartet<const int*, int*, int*, bool> od =
                  old_leaf(static_cast<const int*>(src), src + n,
                           tref, tref + tcap, fref, fref + fcap, odd_t());

               // Same work done either way.
               assert(nw.first  - src  == od.first  - src);
               assert(nw.second - tout == od.second - tref);
               assert(nw.third  - fout == od.third  - fref);
               for(int k = 0; k < NMAX + 1; ++k) {
                  assert(tout[k] == tref[k]);
                  assert(fout[k] == fref[k]);
               }

               const bool src_drained = (nw.first == src + n);
               // The substitution the walkers perform.
               assert(nw.fourth == !(src_drained || od.fourth));
               // New flag true means "advance to the next out_false segment":
               // only legal when this out_false range is exactly full and
               // there is still source left to place.
               if(nw.fourth) {
                  assert(nw.third == fout + fcap);
                  assert(!src_drained);
               }
               // Tie rule: source exhaustion outranks a full out_false range.
               if(src_drained) {
                  assert(!nw.fourth);
                  if(nw.third == fout + fcap && fcap != 0) ++ties;
               }
               ++cases;
            }
         }
      }
   }
   std::printf("214_pc_contract: %ld cases OK (%ld of them source/out_false ties)\n",
               cases, ties);
   return 0;
}
