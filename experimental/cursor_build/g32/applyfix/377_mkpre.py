# Builds the "before" shadow headers: the CURRENT working-tree headers with
# only the conditional-write leaf reorder reverted.  Nothing else differs, so a
# benchmark of shadow-vs-tree isolates exactly this change (and is unaffected by
# the macro rename and by any other uncommitted work in the tree).
import os, sys

SRC = "/mnt/d/Data/LocalGit/boost/boost/container/experimental"
DST = "/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g32/applyfix/pre/boost/container/experimental"

EDITS = {}

EDITS["segmented_copy_if.hpp"] = ("""   //[alg.copy] mandates exactly last - first applications of pred.  Testing an
   //element, discovering the destination segment is full and returning makes the
   //enclosing destination walker call this leaf again on the same element, which
   //re-applies pred.  Checking the destination once on entry and again after each
   //write removes that: when the destination fills, `first` has already moved
   //past the element that was written, so the next call resumes on an untested
   //element.  With an unreachable_sentinel_t destination both checks fold away,
   //so the flat path is unchanged.
   if(BOOST_UNLIKELY(dst_first == dst_last))
      return segduo<SrcIter, DstIter>(first, dst_first);

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         *dst_first = *first;
         ++dst_first;
         if(BOOST_UNLIKELY(dst_first == dst_last)) {
            ++first;
            goto out_path;
         }
      }
   }
""", """   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(dst_first == dst_last))
            goto out_path;
         *dst_first = *first;
         ++dst_first;
      }
   }
""")

EDITS["segmented_remove_copy.hpp"] = ("""   //[alg.remove] mandates exactly last - first comparisons.  Comparing an
   //element, discovering the destination segment is full and returning makes the
   //enclosing destination walker call this leaf again on the same element, which
   //compares it a second time.  Checking the destination once on entry and again
   //after each write removes that: when the destination fills, `first` has
   //already moved past the element that was written, so the next call resumes on
   //an uncompared element.  With an unreachable_sentinel_t destination both
   //checks fold away, so the flat path is unchanged.
   if(BOOST_UNLIKELY(dst_first == dst_last))
      return segduo<SrcIter, DstIter>(first, dst_first);

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(!(*first == value)) {
         transfer_op<Move>::apply(*dst_first, *first);
         ++dst_first;
         if(BOOST_UNLIKELY(dst_first == dst_last)) {
            ++first;
            goto out_path;
         }
      }
   }
""", """   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(!(*first == value)) {
         if(BOOST_UNLIKELY(dst_first == dst_last))
            goto out_path;
         transfer_op<Move>::apply(*dst_first, *first);
         ++dst_first;
      }
   }
""")

EDITS["segmented_remove_copy_if.hpp"] = ("""   //[alg.remove] mandates exactly last - first applications of pred.  Testing an
   //element, discovering the destination segment is full and returning makes the
   //enclosing destination walker call this leaf again on the same element, which
   //re-applies pred.  Checking the destination once on entry and again after each
   //write removes that: when the destination fills, `first` has already moved
   //past the element that was written, so the next call resumes on an untested
   //element.  With an unreachable_sentinel_t destination both checks fold away,
   //so the flat path is unchanged.
   if(BOOST_UNLIKELY(dst_first == dst_last))
      return segduo<SrcIter, DstIter>(first, dst_first);

   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(!pred(*first)) {
         transfer_op<Move>::apply(*dst_first, *first);
         ++dst_first;
         if(BOOST_UNLIKELY(dst_first == dst_last)) {
            ++first;
            goto out_path;
         }
      }
   }
""", """   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(!pred(*first)) {
         if(BOOST_UNLIKELY(dst_first == dst_last))
            goto out_path;
         transfer_op<Move>::apply(*dst_first, *first);
         ++dst_first;
      }
   }
""")

EDITS["segmented_partition_copy.hpp"] = ("""   //
   // [alg.partitions] mandates exactly last - first applications of pred.
   // Testing an element, discovering that the output it belongs to is full and
   // returning makes the enclosing walker call this leaf again on the same
   // element, which re-applies pred.  Each output is therefore tested once on
   // entry and again after each write to it: when one fills, `first` has already
   // moved past the element that was written, so the next call resumes on an
   // untested element, and the loop body can take for granted that both outputs
   // have room.  With unreachable_sentinel_t outputs the tests fold away, so the
   // flat path is unchanged.
   if(BOOST_UNLIKELY(t_first == t_last))
      return segquartet<SrcIter, TIter, FIter, bool>(first, t_first, f_first, false);
   if(BOOST_UNLIKELY(f_first == f_last))
      return segquartet<SrcIter, TIter, FIter, bool>(first, t_first, f_first, first != last);

   bool false_output_full = false;
   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         *t_first = *first;
         ++t_first;
         if(BOOST_UNLIKELY(t_first == t_last)) {
            ++first;
            break;
         }
      }
      else {
         *f_first = *first;
         ++f_first;
         if(BOOST_UNLIKELY(f_first == f_last)) {
            ++first;
            false_output_full = first != last;
            break;
         }
      }
   }
""", """   bool false_output_full = false;
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
""")

os.makedirs(DST, exist_ok=True)
rc = 0
for name, (new, old) in EDITS.items():
    text = open(os.path.join(SRC, name)).read()
    if text.count(new) != 1:
        print(f"FAIL {name}: post-fix leaf found {text.count(new)} times, expected 1")
        rc = 1
        continue
    open(os.path.join(DST, name), "w").write(text.replace(new, old))
    print(f"ok   {name}: leaf reverted ({len(new.splitlines())} lines -> {len(old.splitlines())})")
sys.exit(rc)
