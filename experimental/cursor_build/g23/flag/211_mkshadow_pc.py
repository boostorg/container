# Builds a shadow copy of segmented_partition_copy.hpp in which the leaf's
# bool means "the out_false range filled" (a resumable stop) instead of
# "out_true blocked", so the three walker sites test one boolean instead of
# `first == last || r.fourth`.  Exact string replacements; errors out if any
# anchor moved.
import os, sys

SRC = "/mnt/d/Data/LocalGit/boost/boost/container/experimental/segmented_partition_copy.hpp"
DST = sys.argv[1]

REPL = [
# ---- leaf: flag polarity -------------------------------------------------
("""   bool true_output_full = false;
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(t_first == t_last)) {
            true_output_full = true;
            break;
         }
         *t_first = *first;
         ++t_first;
      }
      else {
         if(BOOST_UNLIKELY(f_first == f_last))
            break;
         *f_first = *first;
         ++f_first;
      }
   }
   return segquartet<SrcIter, TIter, FIter, bool>
      (first, t_first, f_first, true_output_full);
""",
"""   // fourth says whether [f_first, f_last) filled, the only stop a caller
   // walking out_false segments can resume from.  Every exit sets it to a
   // constant, so a caller that would otherwise also re-test source exhaustion
   // just tests the flag.  The source test (the loop condition) precedes the
   // output tests, so a source and an output running out on the same element
   // counts as source exhaustion, which is what those callers require.
   bool false_output_full = false;
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(t_first == t_last))
            break;
         *t_first = *first;
         ++t_first;
      }
      else {
         if(BOOST_UNLIKELY(f_first == f_last)) {
            false_output_full = true;
            break;
         }
         *f_first = *first;
         ++f_first;
      }
   }
   return segquartet<SrcIter, TIter, FIter, bool>
      (first, t_first, f_first, false_output_full);
"""),
# ---- false_bounded segmented walker: first partial segment ---------------
("""         if(BOOST_UNLIKELY(first == last || r.fourth))
            return segquartet<SrcIter, TIter, SegFIter, bool>
               (first, t_first, ftr::compose(fsfirst, r.third), r.fourth);
      }

      for(++fsfirst; fsfirst != fslast; ++fsfirst) {""",
"""         if(BOOST_UNLIKELY(!r.fourth))
            return segquartet<SrcIter, TIter, SegFIter, bool>
               (first, t_first, ftr::compose(fsfirst, r.third), false);
      }

      for(++fsfirst; fsfirst != fslast; ++fsfirst) {"""),
# ---- false_bounded segmented walker: middle segments --------------------
("""         if(BOOST_UNLIKELY(first == last || r.fourth))
            return segquartet<SrcIter, TIter, SegFIter, bool>
               (first, t_first, ftr::compose(fsfirst, r.third), r.fourth);
      }

      fb = ftr::begin(fslast);""",
"""         if(BOOST_UNLIKELY(!r.fourth))
            return segquartet<SrcIter, TIter, SegFIter, bool>
               (first, t_first, ftr::compose(fsfirst, r.third), false);
      }

      fb = ftr::begin(fslast);"""),
# ---- false_dispatch segmented driver ------------------------------------
("""      if(BOOST_UNLIKELY(first == last || r.fourth))
         return segtrio<SrcIter, TIter, SegFIter>(first, t_first, ftr::compose(fs, r.third));""",
"""      if(BOOST_UNLIKELY(!r.fourth))
         return segtrio<SrcIter, TIter, SegFIter>(first, t_first, ftr::compose(fs, r.third));"""),
# ---- comments describing the flag ---------------------------------------
("""// partner.  The recursive result records whether out_true blocked; otherwise a
// non-drained return means this out_false sub-segment filled.""",
"""// partner.  The recursive result says whether the sub-segment filled, the only
// stop that can be resumed by moving on to the next one; anything else (source
// drained, out_true blocked) has to be handed back to the outer stage."""),
("""// worker.  Stops when the source drains or the out_true partner fills;
// otherwise the current out_false segment filled, so advance to the next one.""",
"""// worker.  The current out_false segment filling is the only stop that can be
// resumed here, so the flag alone decides whether to advance to the next
// segment or hand control back to the out_true stage."""),
]

text = open(SRC, newline='').read().replace("\r\n", "\n")
for i, (old, new) in enumerate(REPL):
   old = old.replace("\r\n", "\n")
   new = new.replace("\r\n", "\n")
   if text.count(old) != 1:
      sys.exit("anchor %d matched %d times" % (i, text.count(old)))
   text = text.replace(old, new)

os.makedirs(os.path.dirname(DST), exist_ok=True)
open(DST, "w", newline="\n").write(text)
print("shadow written:", DST)
