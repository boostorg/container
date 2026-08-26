#!/usr/bin/env python3
"""365: build shadow copies of segmented_copy_if.hpp under g31/copyif/<variant>/
so the benchmark can be compiled with -I<variant> ahead of the real include root.
No real header is touched."""
import os, sys, shutil

BR   = "/mnt/d/Data/LocalGit/boost"
REAL = os.path.join(BR, "libs/container/include/boost/container/experimental/segmented_copy_if.hpp")
WD   = os.path.join(BR, "libs/container/experimental/cursor_build/g31/copyif")

SRC = open(REAL).read()

# ---------------------------------------------------------------- p1 ---------
# Drop the GCC "unroll 8" pragma from the fixed-trip-count block loop.  With a
# compile-time trip count of 32 GCC still emits a runtime remainder computation
# plus a six-way cmp/je ladder on every block.
P1_OLD = """      avail -= block_size;
      BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL
      for(Diff chunk = block_size; chunk; ) {"""
P1_NEW = """      avail -= block_size;
      for(Diff chunk = block_size; chunk; ) {"""

# ---------------------------------------------------------------- p2 ---------
# Conformance fix: never re-test an element.  Check the destination once on
# entry and again *after* each write, so a leaf that stops because the
# destination segment filled leaves `first` past the element it just wrote.
P2_OLD = """   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(dst_first == dst_last))
            goto out_path;
         *dst_first = *first;
         ++dst_first;
      }
   }
   out_path:
   return segduo<SrcIter, DstIter>(first, dst_first);"""
P2_NEW = """   //[alg.copy] mandates exactly last - first applications of pred.  Testing an
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
   out_path:
   return segduo<SrcIter, DstIter>(first, dst_first);"""


# ---------------------------------------------------------------- p4 ---------
# Restructured random-access leaf.  Both ranges are random access, so the
# number of iterations that provably cannot overrun the destination is known
# up front: min(source left, destination room).  Running exactly that many
# iterations removes the per-element destination test from the inner loop and
# replaces copy_if_cleanup_blocks' fixed 32-element quantum (which needs room
# for 32 *writes* before it may run at all, and pays a fresh unroll prologue
# every 32 source elements) with one prologue per min(avail, room) elements.
# It also leaves `first` past every element it examined, so nothing is
# re-tested even without the p2 patch.
P4_OLD = """template <std::size_t BlockSize, class RASrcIter, class RADstIter,
          class Pred, class Diff>
BOOST_CONTAINER_FORCEINLINE segtrio<RASrcIter, RADstIter, Diff>
copy_if_cleanup_blocks
   (RASrcIter cur, RADstIter dst_cur, RADstIter dst_last,
    Pred pred, Diff avail)
{
   const Diff block_size = static_cast<Diff>(BlockSize);
   while(avail >= block_size &&
         static_cast<Diff>(dst_last - dst_cur) >= block_size) {
      avail -= block_size;
      BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL
      for(Diff chunk = block_size; chunk; ) {
         --chunk;
         if(pred(*cur)) {
            *dst_cur = *cur;
            ++dst_cur;
         }
         ++cur;
      }
   }
   return segtrio<RASrcIter, RADstIter, Diff>(cur, dst_cur, avail);
}

template <class RASrcIter, class RADstIter, class Pred>
BOOST_CONTAINER_FORCEINLINE typename iterator_enable_if_tag
   <RADstIter, std::random_access_iterator_tag, segduo<RASrcIter, RADstIter> >::type
segmented_copy_if_dst_bounded
   (RASrcIter first, RASrcIter last, RADstIter dst_first, RADstIter dst_last, Pred pred,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &src_tag)
{
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;

   (void)src_tag;
   segtrio<RASrcIter, RADstIter, difference_type> r =
      (copy_if_cleanup_blocks<32>)
         (first, dst_first, dst_last, pred, last - first);

   return (segmented_copy_if_dst_bounded)
      (r.first, last, r.second, dst_last, pred, non_segmented_iterator_tag(), int());
}"""

P4_NEW = """//Both ranges are random access, so the number of iterations that provably
//cannot overrun the destination is known before the loop starts: whichever of
//"source left" and "destination room" is smaller.  Running exactly that many
//iterations takes the destination test out of the inner loop entirely, and
//leaves `first` past every element it examined, so no element is ever tested
//twice and [alg.copy]'s "exactly last - first applications of pred" holds.
template <class RASrcIter, class RADstIter, class Pred>
BOOST_CONTAINER_FORCEINLINE typename iterator_enable_if_tag
   <RADstIter, std::random_access_iterator_tag, segduo<RASrcIter, RADstIter> >::type
segmented_copy_if_dst_bounded
   (RASrcIter first, RASrcIter last, RADstIter dst_first, RADstIter dst_last, Pred pred,
    const non_segmented_iterator_tag &, const std::random_access_iterator_tag &src_tag)
{
   typedef typename iterator_traits<RASrcIter>::difference_type difference_type;

   (void)src_tag;
   difference_type avail = last - first;
   difference_type room  = static_cast<difference_type>(dst_last - dst_first);

   while(avail && room) {
      difference_type run = avail < room ? avail : room;
      avail -= run;
      BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL
      for(; run; --run) {
         if(pred(*first)) {
            *dst_first = *first;
            ++dst_first;
         }
         ++first;
      }
      room = static_cast<difference_type>(dst_last - dst_first);
   }
   return segduo<RASrcIter, RADstIter>(first, dst_first);
}"""

# ---------------------------------------------------------------- p5 ---------
# As p4, but stop the counted loop once the provable run drops below a floor
# and hand the tail to the (p2-shaped) checked leaf, so a destination that is
# nearly full does not pay an unroll prologue per one or two elements.
P5_NEW = P4_NEW.replace(
"""   while(avail && room) {
      difference_type run = avail < room ? avail : room;
      avail -= run;""",
"""   for(;;) {
      difference_type run = avail < room ? avail : room;
      if(run < 8)
         break;
      avail -= run;""").replace(
"""      room = static_cast<difference_type>(dst_last - dst_first);
   }
   return segduo<RASrcIter, RADstIter>(first, dst_first);
}""",
"""      room = static_cast<difference_type>(dst_last - dst_first);
   }
   return (segmented_copy_if_dst_bounded)
      (first, last, dst_first, dst_last, pred, non_segmented_iterator_tag(), int());
}""")


def patch(text, old, new, what):
    if old not in text:
        sys.exit("PATCH FAILED (%s): anchor not found" % what)
    return text.replace(old, new, 1)


VARIANTS = {
    "p0": [],
    "p1": [(P1_OLD, P1_NEW, "p1")],
    "p2": [(P2_OLD, P2_NEW, "p2")],
    "p3": [(P1_OLD, P1_NEW, "p1"), (P2_OLD, P2_NEW, "p2")],
    "p4": [(P2_OLD, P2_NEW, "p2"), (P4_OLD, P4_NEW, "p4")],
    "p5": [(P2_OLD, P2_NEW, "p2"), (P4_OLD, P5_NEW, "p5")],
}

only = sys.argv[1:] or list(VARIANTS)

for name, patches in VARIANTS.items():
    if name not in only:
        continue
    d = os.path.join(WD, name, "boost/container/experimental")
    shutil.rmtree(os.path.join(WD, name), ignore_errors=True)
    os.makedirs(d)
    t = SRC
    for old, new, what in patches:
        t = patch(t, old, new, "%s/%s" % (name, what))
    open(os.path.join(d, "segmented_copy_if.hpp"), "w").write(t)
    print("wrote %s (%d bytes, %d patches)" % (name, len(t), len(patches)))
