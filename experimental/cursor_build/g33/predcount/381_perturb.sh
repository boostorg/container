#!/bin/bash
# Reintroduces the "test destination before the write" leaf ordering in shadow
# copies of two headers and shows the new count tests fail against them.
ROOT=/mnt/d/Data/LocalGit/boost
TD=$ROOT/libs/container/experimental
HD=$ROOT/libs/container/include/boost/container/experimental
SH=$ROOT/libs/container/experimental/cursor_build/g33/predcount/shadow
rm -rf $SH
mkdir -p $SH/boost/container/experimental

python3 - "$HD" "$SH/boost/container/experimental" <<'PYEOF'
import sys, os
hd, sh = sys.argv[1], sys.argv[2]

def patch(name, good, bad):
    src = open(os.path.join(hd, name)).read()
    if good not in src:
        print("PERTURB-SKIP %s: fixed leaf shape not found" % name)
        return False
    open(os.path.join(sh, name), "w").write(src.replace(good, bad, 1))
    print("PERTURB-OK   %s" % name)
    return True

good_ci = """   if(BOOST_UNLIKELY(dst_first == dst_last))
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
"""
bad_ci = """   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(pred(*first)) {
         if(BOOST_UNLIKELY(dst_first == dst_last))
            goto out_path;
         *dst_first = *first;
         ++dst_first;
      }
   }
"""
patch("segmented_copy_if.hpp", good_ci, bad_ci)

good_rci = """   if(BOOST_UNLIKELY(dst_first == dst_last))
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
"""
bad_rci = """   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(!pred(*first)) {
         if(BOOST_UNLIKELY(dst_first == dst_last))
            goto out_path;
         transfer_op<Move>::apply(*dst_first, *first);
         ++dst_first;
      }
   }
"""
patch("segmented_remove_copy_if.hpp", good_rci, bad_rci)

good_rc = """   if(BOOST_UNLIKELY(dst_first == dst_last))
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
"""
bad_rc = """   BOOST_CONTAINER_SEGMENTED_UNROLL(4)
   for(; first != last; ++first) {
      if(!(*first == value)) {
         if(BOOST_UNLIKELY(dst_first == dst_last))
            goto out_path;
         transfer_op<Move>::apply(*dst_first, *first);
         ++dst_first;
      }
   }
"""
patch("segmented_remove_copy.hpp", good_rc, bad_rc)
PYEOF

FLAGS="-O2 -DNDEBUG -Wall -Wextra -I$SH -I$ROOT -I$TD"
for t in "$@"; do
  echo "=== $t against the perturbed header ==="
  g++-16 -std=c++20 $FLAGS "$TD/$t.cpp" -o "/tmp/$t.bug.elf" 2>&1 | head -20
  if [ -x "/tmp/$t.bug.elf" ]; then
    "/tmp/$t.bug.elf" > "/tmp/$t.bug.out" 2>&1
    echo "exit=$?"
    head -6 "/tmp/$t.bug.out"
    tail -2 "/tmp/$t.bug.out"
  fi
done
