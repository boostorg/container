#!/bin/bash
# Depth-1 destinations must generate exactly the same code as before: the new
# overload takes segmented_iterator_tag by value, and at depth 1 the argument
# is non_segmented_iterator_tag, an unrelated type.  Build the depth-1 probe
# against the committed headers and against the working tree and compare.
set -u
BR=/mnt/d/Data/LocalGit/boost
REPO=$BR/libs/container
S=$REPO/experimental/cursor_build/g22/deep
O=/tmp/g22nc
rm -rf $O; mkdir -p $O/base/boost/container

# Shadow tree holding the committed version of the four touched headers.
cp -rL $BR/boost/container/experimental $O/base/boost/container/ 2>/dev/null
for f in segmented_set_union segmented_set_difference segmented_set_intersection \
         segmented_set_symmetric_difference; do
   git -C $REPO show HEAD:include/boost/container/experimental/$f.hpp \
      > $O/base/boost/container/experimental/$f.hpp || { echo "git show failed for $f"; exit 1; }
done
echo "shadow tree built; committed vs working-tree line counts:"
for f in segmented_set_union segmented_set_difference; do
   printf "   %-34s committed=%-6s working=%s\n" $f \
      "$(wc -l < $O/base/boost/container/experimental/$f.hpp)" \
      "$(wc -l < $BR/boost/container/experimental/$f.hpp)"
done

echo
printf "%-28s %-22s %-22s %s\n" algorithm "committed insns" "working insns" verdict
for i in 10 11 12 13; do
   name=$(sed -n "s/.*ALGO == $i\$/&/p" /dev/null; echo "ALGO=$i")
   for mode in base work; do
      if [ $mode = base ]; then INC="-I$O/base -I$BR"; else INC="-I$BR"; fi
      g++-16 -std=c++20 -O2 -DNDEBUG -DALGO=$i -DDEPTH=1 $INC \
         -I$REPO/experimental -c $S/../../g22/deep/202_survey.cpp -o $O/$mode.o 2>$O/$mode.log \
         || { echo "  ALGO=$i $mode BUILD FAIL"; grep -m2 'error:' $O/$mode.log; continue; }
      objdump -d --no-show-raw-insn $O/$mode.o | grep -cE '^\s+[0-9a-f]+:' > $O/$mode.cnt
   done
   b=$(cat $O/base.cnt 2>/dev/null || echo ?)
   w=$(cat $O/work.cnt 2>/dev/null || echo ?)
   if [ "$b" = "$w" ]; then v="identical"; else v="DIFFERS"; fi
   printf "%-28s %-22s %-22s %s\n" "$name" "$b" "$w" "$v"
done
