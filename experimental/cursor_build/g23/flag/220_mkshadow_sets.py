# Builds shadow copies of the four segmented_set_*.hpp headers.
#
#   220_mkshadow_sets.py <algo> <before|after> <outfile>
#
# "before" is the state the other agent is producing: a segmented-destination
# *_dst_bounded overload mirroring merge's, returning segtrio, with the walkers
# re-testing `first1 == last1 || first2 == last2`.  set_union already has that
# overload in the working tree, so for set_union "before" is a verbatim copy.
#
# "after" adds the leaf flag ("a source ran out") as in merge_dst_bounded and
# replaces both walker re-tests by a test of the flag.
#
# The real headers are never modified; probes pick a shadow up via -I ordering.
import os, sys

NAME, MODE, DST = sys.argv[1], sys.argv[2], sys.argv[3]   # NAME = set_union, ...
SRC = ("/mnt/d/Data/LocalGit/boost/boost/container/experimental/segmented_%s.hpp"
       % NAME)

text = open(SRC, newline='').read().replace("\r\n", "\n")
repl = []

def sub(old, new):
   repl.append((old, new))

# ---------------------------------------------------------------------------
# The segmented-destination walker, identical in shape for all four (it only
# threads the two sources and recurses on the destination's local structure).
# ---------------------------------------------------------------------------

SEG_HEAD = """//////////////////////////////////////////////////////////////////////////////
// Destination segmented in its own right: the bounded range spans destination
// segments, so walk them and recurse.  A destination segmented N levels deep
// resolves to this overload N-1 times before reaching a leaf above.  Stopping
// on source exhaustion rather than on "the segment did not fill" matters for
// the same reason it does in until_exhausts: when the output ends exactly on a
// segment boundary both hold at once.
//////////////////////////////////////////////////////////////////////////////

"""

def seg_overload(flag):
   res = ("segquartet<Iter1, Iter2, SegDstIter, bool>" if flag
          else "segtrio<Iter1, Iter2, SegDstIter>")
   loc = ("segquartet<Iter1, Iter2, dst_local_iterator, bool>" if flag
          else "segtrio<Iter1, Iter2, dst_local_iterator>")
   test = ("r.fourth" if flag else "first1 == last1 || first2 == last2")
   early = ("            return result_t(first1, first2, dst_traits::compose(sfirst, r.third), true);"
            if flag else
            "            return result_t(first1, first2, dst_traits::compose(sfirst, r.third));")
   final = ("   return result_t(r.first, r.second, dst_traits::compose(sfirst, r.third), r.fourth);"
            if flag else
            "   return result_t(r.first, r.second, dst_traits::compose(sfirst, r.third));")
   return SEG_HEAD + """template <class Iter1, class Sent1, class Iter2, class Sent2, class SegDstIter,
          class Comp, class SrcCat>
%(res)s %(name)s_dst_bounded
   (Iter1 first1, Sent1 last1, Iter2 first2, Sent2 last2,
    SegDstIter dst_first, SegDstIter dst_last, Comp comp,
    segmented_iterator_tag, SrcCat)
{
   typedef segmented_iterator_traits<SegDstIter>  dst_traits;
   typedef typename dst_traits::local_iterator    dst_local_iterator;
   typedef typename dst_traits::segment_iterator  dst_segment_iterator;
   typedef typename segmented_iterator_traits<dst_local_iterator>::is_segmented_iterator dst_is_local_seg_t;
   typedef %(loc)s  local_result_t;
   typedef %(res)s          result_t;

   dst_segment_iterator       sfirst = dst_traits::segment(dst_first);
   const dst_segment_iterator slast  = dst_traits::segment(dst_last);

   dst_local_iterator db = dst_traits::local(dst_first);

   if(BOOST_LIKELY(sfirst != slast)) {
      {
         const local_result_t r = (%(name)s_dst_bounded)
            ( first1, last1, first2, last2, db
            , dst_traits::end(sfirst), comp, dst_is_local_seg_t(), SrcCat());
         first1 = r.first;
         first2 = r.second;
         if(BOOST_UNLIKELY(%(test)s))
%(early)s
      }

      for(++sfirst; sfirst != slast; ++sfirst) {
         const local_result_t r = (%(name)s_dst_bounded)
            ( first1, last1, first2, last2, dst_traits::begin(sfirst)
            , dst_traits::end(sfirst), comp, dst_is_local_seg_t(), SrcCat());
         first1 = r.first;
         first2 = r.second;
         if(BOOST_UNLIKELY(%(test)s))
%(early)s
      }

      db = dst_traits::begin(slast);
   }
   const local_result_t r = (%(name)s_dst_bounded)
      ( first1, last1, first2, last2, db
      , dst_traits::local(dst_last), comp, dst_is_local_seg_t(), SrcCat());
%(final)s
}

""" % dict(res=res, loc=loc, name=NAME, test=test, early=early, final=final)

# before : verbatim copy of the real header (segtrio, walkers re-test).
# split  : merge-style leaf, every exit edge carrying a constant flag.
# post   : leaf keeps its single-exit loop and computes the flag once at that
#          exit, so it costs the two compares the caller would have done and
#          saves them at every enclosing level.
# guard  : set_union only.  The destination test moves from the loop condition
#          into the body, which separates the exit edges without changing the
#          dynamic compare count and folds away completely under
#          unreachable_sentinel_t.  Same shape as the other three leaves.
FLAG = (MODE in ("split", "post", "guard"))

# ---------------------------------------------------------------------------
# 1. Provide (or replace) the segmented-destination overload.
# ---------------------------------------------------------------------------
UNTIL_HEAD = ("//////////////////////////////////////////////////////////////"
              "////////////////\n// %s_until_exhausts" % NAME)

start = text.index(SEG_HEAD)
end   = text.index(UNTIL_HEAD)
if text[start:end] != seg_overload(False):
   sys.exit("%s: existing segmented-destination overload is not the expected "
            "no-flag shape" % NAME)
if FLAG:
   text = text[:start] + seg_overload(True) + text[end:]

# ---------------------------------------------------------------------------
# 2. The leaf, the dual-RA forwarder and until_exhausts.
# ---------------------------------------------------------------------------
if FLAG:
   sub("typename algo_enable_if_c<!DstTag::value, segtrio<Iter1, Iter2, DstIter> >::type\n%s_dst_bounded" % NAME,
       "typename algo_enable_if_c<!DstTag::value, segquartet<Iter1, Iter2, DstIter, bool> >::type\n%s_dst_bounded" % NAME)
   sub("   , segtrio<RAIter1, RAIter2, DstIter> >::type\n%s_dst_bounded" % NAME,
       "   , segquartet<RAIter1, RAIter2, DstIter, bool> >::type\n%s_dst_bounded" % NAME)

   FLAG_DOC = """   // fourth says whether a source ran out, the only stop a caller walking
   // destination segments cannot resume from.  Every exit sets it to a
   // constant, so a caller that would otherwise re-compare both source ends
   // just tests the flag; the compiler folds that away where it can already
   // see this body, and cannot where the caller recursed into a segmented
   // destination instead.  The source test precedes the dst test at each exit,
   // so a source and the destination running out on the same element count as
   // source exhaustion, which is what those callers require.
"""

   if NAME == "set_union" and MODE == "guard":
      sub("""   while(first1 != last1 && first2 != last2 && dst_first != dst_last) {
      if      (comp(*first1, *first2)) { *dst_first = *first1;  ++first1;  }
      else if (comp(*first2, *first1)) { *dst_first = *first2; ++first2; }
      else                             { *dst_first = *first1;  ++first1; ++first2; }
      ++dst_first;
   }
   return segtrio<Iter1, Iter2, DstIter>(first1, first2, dst_first);""",
FLAG_DOC + """   bool src_done = true;
   while(first1 != last1 && first2 != last2) {
      if(BOOST_UNLIKELY(dst_first == dst_last)) {
         src_done = false;
         break;
      }
      if      (comp(*first1, *first2)) { *dst_first = *first1;  ++first1;  }
      else if (comp(*first2, *first1)) { *dst_first = *first2; ++first2; }
      else                             { *dst_first = *first1;  ++first1; ++first2; }
      ++dst_first;
   }
   return segquartet<Iter1, Iter2, DstIter, bool>(first1, first2, dst_first, src_done);""")
   elif NAME == "set_union" and MODE == "post":
      sub("""   while(first1 != last1 && first2 != last2 && dst_first != dst_last) {
      if      (comp(*first1, *first2)) { *dst_first = *first1;  ++first1;  }
      else if (comp(*first2, *first1)) { *dst_first = *first2; ++first2; }
      else                             { *dst_first = *first1;  ++first1; ++first2; }
      ++dst_first;
   }
   return segtrio<Iter1, Iter2, DstIter>(first1, first2, dst_first);""",
FLAG_DOC + """   while(first1 != last1 && first2 != last2 && dst_first != dst_last) {
      if      (comp(*first1, *first2)) { *dst_first = *first1;  ++first1;  }
      else if (comp(*first2, *first1)) { *dst_first = *first2; ++first2; }
      else                             { *dst_first = *first1;  ++first1; ++first2; }
      ++dst_first;
   }
   return segquartet<Iter1, Iter2, DstIter, bool>
      (first1, first2, dst_first, first1 == last1 || first2 == last2);""")
   elif NAME == "set_union":
      # The loop-top compound test funnels all three stops into one edge and
      # cannot carry per-exit constants, so it is split the way merge's was.
      sub("""   while(first1 != last1 && first2 != last2 && dst_first != dst_last) {
      if      (comp(*first1, *first2)) { *dst_first = *first1;  ++first1;  }
      else if (comp(*first2, *first1)) { *dst_first = *first2; ++first2; }
      else                             { *dst_first = *first1;  ++first1; ++first2; }
      ++dst_first;
   }
   return segtrio<Iter1, Iter2, DstIter>(first1, first2, dst_first);""",
FLAG_DOC + """   bool src_done = true;
   if(first1 != last1 && first2 != last2) {
      if(dst_first != dst_last) {
         while(true) {
            if      (comp(*first1, *first2)) {
               *dst_first = *first1;  ++first1;
               ++dst_first;
               if(first1 == last1)
                  break;
            }
            else if (comp(*first2, *first1)) {
               *dst_first = *first2; ++first2;
               ++dst_first;
               if(first2 == last2)
                  break;
            }
            else {
               *dst_first = *first1;  ++first1; ++first2;
               ++dst_first;
               if(first1 == last1 || first2 == last2)
                  break;
            }
            if(dst_first == dst_last) {
               src_done = false;
               break;
            }
         }
      }
      else {
         src_done = false;
      }
   }
   return segquartet<Iter1, Iter2, DstIter, bool>(first1, first2, dst_first, src_done);""")
   else:
      # These leaves already have exactly two (or three) exit edges: the loop
      # condition (a source ran out) and the write guard (destination full).
      # Only the constant needs writing down.
      sub("""   // The destination-full test guards the write, not the whole iteration: an
   // element that produces no output still has to be consumed, so stopping
   // with room left in the sources but none in the destination would leave
   // the segmented walker unable to tell a full segment from an exhausted
   // destination.  With unreachable_sentinel_t the test folds away as before.
   while(first1 != last1 && first2 != last2) {""",
FLAG_DOC + """   //
   // The destination-full test guards the write, not the whole iteration: an
   // element that produces no output still has to be consumed, so stopping
   // with room left in the sources but none in the destination would leave
   // the segmented walker unable to tell a full segment from an exhausted
   // destination.  With unreachable_sentinel_t the test folds away as before.
   bool src_done = true;
   while(first1 != last1 && first2 != last2) {""")
      # set_symmetric_difference has two destination-full exits, at different
      # indentation levels; both must carry the constant.
      EXPECTED = {"set_difference": 1, "set_intersection": 1,
                  "set_symmetric_difference": 2}
      done = 0
      for ind in (9, 12):
         p = " " * ind
         old_break = ("%sif(BOOST_UNLIKELY(dst_first == dst_last))\n"
                      "%sbreak;" % (p, p + "   "))
         new_break = ("%sif(BOOST_UNLIKELY(dst_first == dst_last)) {\n"
                      "%ssrc_done = false;\n"
                      "%sbreak;\n"
                      "%s}" % (p, p + "   ", p + "   ", p))
         n = text.count(old_break)
         done += n
         if n:
            text = text.replace(old_break, new_break)
      if done != EXPECTED[NAME]:
         sys.exit("%s: flagged %d destination-full exits, expected %d"
                  % (NAME, done, EXPECTED[NAME]))
      sub("""   }
   return segtrio<Iter1, Iter2, DstIter>(first1, first2, dst_first);
}""",
"""   }
   return segquartet<Iter1, Iter2, DstIter, bool>(first1, first2, dst_first, src_done);
}""")

   # until_exhausts, non-segmented destination: the flag is a constant here.
   sub("""    const Tag &, const Cat &src1_cat)
{
   return (%s_dst_bounded)
      (first1, last1, first2, last2, result, unreachable_sentinel_t(),
       comp, non_segmented_iterator_tag(), src1_cat);
}""" % NAME,
"""    const Tag &, const Cat &src1_cat)
{
   //An unbounded destination can only stop on source exhaustion, so the leaf's
   //flag is a constant here and drops out.
   const segquartet<Iter1, Iter2, DstIter, bool> r = (%s_dst_bounded)
      (first1, last1, first2, last2, result, unreachable_sentinel_t(),
       comp, non_segmented_iterator_tag(), src1_cat);
   return segtrio<Iter1, Iter2, DstIter>(r.first, r.second, r.third);
}""" % NAME)

   # until_exhausts, segmented destination: consume the flag.
   sub("   typedef segtrio<Iter1, Iter2, dst_local_iterator>  bounded_t;",
       "   typedef segquartet<Iter1, Iter2, dst_local_iterator, bool>  bounded_t;")
   sub("""      // Stop on source exhaustion, not on "the segment did not fill": when
      // the output ends exactly on a segment boundary both hold at once, and
      // stepping dst_seg then walks off the end of the destination.  compose()
      // normalises a local iterator sitting on the segment end, the same way
      // segmented_copy_dst_dispatch relies on.
      if(BOOST_UNLIKELY(first1 == last1 || first2 == last2)) {""",
"""      // Stop on source exhaustion, not on "the segment did not fill": when
      // the output ends exactly on a segment boundary both hold at once, and
      // stepping dst_seg then walks off the end of the destination.  compose()
      // normalises a local iterator sitting on the segment end, the same way
      // segmented_copy_dst_dispatch relies on.  fourth already answers that
      // question, and gives source exhaustion priority on such a tie.
      if(BOOST_UNLIKELY(r.fourth)) {""")

for i, (old, new) in enumerate(repl):
   if text.count(old) != 1:
      sys.exit("%s/%s: anchor %d matched %d times" % (NAME, MODE, i, text.count(old)))
   text = text.replace(old, new)

os.makedirs(os.path.dirname(DST), exist_ok=True)
open(DST, "w", newline="\n").write(text)
print("   %-30s %-7s ok" % (NAME, MODE))
