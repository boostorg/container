#!/bin/bash
# Apply the g23 recommended patch to the four REAL headers.
#
# The generator is the applier: it matches by exact string anchor and aborts
# rather than mis-applying, and 240 already proved the anchors are intact and
# that its output is byte-identical to the shadow g23 measured.  Running it
# with DST = the real header therefore applies exactly the measured change.
#
# Before writing, the pre-patch headers are persisted under pre/ so the
# depth-1 regression comparison can overlay them via -I ordering.  A second
# run of this script aborts in the generator (the segmented overload no longer
# has the no-flag shape), which is the intended behaviour.
set -eu
BR=/mnt/d/Data/LocalGit/boost
G=$BR/libs/container/experimental/cursor_build/g23/flag
A24=$BR/libs/container/experimental/cursor_build/g24/apply
PRE=$A24/pre/boost/container/experimental
HEADPRE=$A24/head/boost/container/experimental
ALGOS="set_union set_difference set_intersection set_symmetric_difference"

mkdir -p $PRE $HEADPRE
cd $BR/libs/container

echo "--- persisting pre-patch baselines ---"
for A in $ALGOS; do
   P=include/boost/container/experimental/segmented_$A.hpp
   cp $BR/boost/container/experimental/segmented_$A.hpp $PRE/segmented_$A.hpp
   git show HEAD:./$P > $HEADPRE/segmented_$A.hpp
   printf "  %-26s worktree=%s HEAD=%s lines\n" $A \
      "$(wc -l < $PRE/segmented_$A.hpp)" "$(wc -l < $HEADPRE/segmented_$A.hpp)"
done

echo
echo "--- applying ---"
for A in $ALGOS; do
   python3 $G/220_mkshadow_sets.py $A guard \
      $BR/boost/container/experimental/segmented_$A.hpp
done

echo
echo "--- applied diff vs pre-patch worktree ---"
for A in $ALGOS; do
   printf "  %-26s " $A
   diff $PRE/segmented_$A.hpp \
        $BR/boost/container/experimental/segmented_$A.hpp | grep -c '^[<>]' \
      | tr -d '\n'
   echo " changed lines"
done

echo
echo "--- byte-identical to the measured shadow? ---"
for A in $ALGOS; do
   printf "  %-26s " $A
   if cmp -s $G/shadow_guard/boost/container/experimental/segmented_$A.hpp \
             $BR/boost/container/experimental/segmented_$A.hpp; then
      echo yes
   else
      echo "NO"; exit 1
   fi
done

echo
echo "--- CR check (these four are LF in the repo) ---"
for A in $ALGOS; do
   printf "  %-26s CR=%s\n" $A \
      "$(tr -dc '\r' < $BR/boost/container/experimental/segmented_$A.hpp | wc -c)"
done
echo "242: applied"
