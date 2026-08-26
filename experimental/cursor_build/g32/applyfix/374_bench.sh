#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
CT=$BR/libs/container
S=$EX/cursor_build/g32/applyfix
SH=$S/pre
RAW=$S/raw
O=/tmp/g32bench
mkdir -p $O $RAW

GRPS="25 15 30"
REPS="5"
echo "GRPS=[$GRPS] REPS=[$REPS]"

# Shadow copies of the pre-fix headers, taken from HEAD.
mkdir -p $SH/boost/container/experimental
for h in segmented_copy_if segmented_remove_copy segmented_remove_copy_if segmented_partition_copy; do
   git -C $CT show HEAD:include/boost/container/experimental/$h.hpp \
      > $SH/boost/container/experimental/$h.hpp || exit 1
done
echo "shadow (pre-fix) headers regenerated; sanity check:"
echo "   pre  copy_if entry-test comment lines: $(grep -c 'alg.copy. mandates' $SH/boost/container/experimental/segmented_copy_if.hpp)"
echo "   post copy_if entry-test comment lines: $(grep -c 'alg.copy. mandates' $BR/boost/container/experimental/segmented_copy_if.hpp)"

if nice -n -5 true 2>/dev/null; then
   PIN="taskset -c 3 nice -n -5"
else
   echo "note: nice -n -5 refused by the kernel; taskset -c 3 only"
   PIN="taskset -c 3"
fi

for G in $GRPS; do
   for CXX in g++-16 clang++-22; do
      for V in pre post; do
         if [ "$V" = "pre" ]; then INC="-I$SH -I$BR"; else INC="-I$BR"; fi
         if $CXX -std=c++20 -O3 $INC -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$G -falign-functions=64 -falign-loops=64 $EX/bench_segmented_algos.cpp -o $O/b_${G}_${CXX}_${V} 2> $O/bb_${G}_${CXX}_${V}.log
         then
            for r in $(seq 1 $REPS); do
               $PIN $O/b_${G}_${CXX}_${V} > $RAW/r_${G}_${CXX}_${V}_${r}.txt 2>/dev/null
            done
            echo "ok: group $G / $CXX / $V  ($REPS runs)"
         else
            echo "BUILD FAILED: group $G / $CXX / $V"; head -20 $O/bb_${G}_${CXX}_${V}.log
         fi
      done
   done
done
echo "raw run outputs are in $RAW"
