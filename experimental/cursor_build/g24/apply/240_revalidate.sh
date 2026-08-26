#!/bin/bash
# Re-validate the g23 recommended patch against the CURRENT real headers.
#
# 220_mkshadow_sets.py applies the change by exact string anchor and aborts if
# an anchor moved, so regenerating the shadows from today's headers is itself
# the anchor check.  Diffing the fresh shadows against the ones g23 measured
# tells us whether the other agent's edits changed anything the patch touches.
set -u
BR=/mnt/d/Data/LocalGit/boost
G=$BR/libs/container/experimental/cursor_build/g23/flag
O=/tmp/g24rev
NEW=$O/guard
OLD=$G/shadow_guard
rm -rf $O; mkdir -p $O
ALGOS="set_union set_difference set_intersection set_symmetric_difference"

echo "--- regenerating shadows from current real headers (anchor check) ---"
rc=0
for A in $ALGOS; do
   python3 $G/220_mkshadow_sets.py $A guard \
      $NEW/boost/container/experimental/segmented_$A.hpp || rc=1
done
[ $rc -ne 0 ] && { echo "ANCHOR FAILURE"; exit 1; }

echo
echo "--- fresh shadow vs. g23-measured shadow ---"
for A in $ALGOS; do
   P=boost/container/experimental/segmented_$A.hpp
   if diff -q $OLD/$P $NEW/$P >/dev/null; then
      printf "  %-26s identical to measured shadow\n" $A
   else
      printf "  %-26s DIFFERS from measured shadow\n" $A
      diff -u $OLD/$P $NEW/$P
      rc=1
   fi
done

echo
echo "--- current real header vs. fresh shadow (== the patch to apply) ---"
for A in $ALGOS; do
   P=boost/container/experimental/segmented_$A.hpp
   printf "  %-26s " $A
   diff <(tr -d '\r' < $BR/$P) $NEW/$P | grep -c '^[<>]' | tr '\n' ' '
   echo "changed lines"
done

echo
[ $rc -eq 0 ] && echo "240: anchors OK, patch unchanged" || echo "240: REVIEW NEEDED"
exit $rc
