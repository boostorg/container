#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
S=$EX/cursor_build/g32/applyfix
SH=$S/pre
RAW=$S/raw2
O=/tmp/g32bench2
mkdir -p $O $RAW
rm -f $RAW/*.txt $SH/boost/container/experimental/*.hpp

GRPS="25 15 30"
REPS=5

tr -d '\r' < $S/377_mkpre.py > /tmp/mkpre.py
python3 /tmp/mkpre.py || exit 1

# Prove the shadow really is "tree minus the reorder" and nothing else.
echo "--- shadow vs tree, unified diff line counts ---"
for h in segmented_copy_if segmented_remove_copy segmented_remove_copy_if segmented_partition_copy; do
   n=$(diff -u $SH/boost/container/experimental/$h.hpp $BR/boost/container/experimental/$h.hpp | grep -c '^[+-][^+-]')
   echo "   $h.hpp: $n changed lines"
done

if nice -n -5 true 2>/dev/null; then PIN="taskset -c 3 nice -n -5"
else echo "note: nice -n -5 refused by the kernel; taskset -c 3 only"; PIN="taskset -c 3"; fi

for G in $GRPS; do
   for CXX in g++-16 clang++-22; do
      for V in pre post; do
         if [ "$V" = "pre" ]; then INC="-I$SH -I$BR"; else INC="-I$BR"; fi
         if $CXX -std=c++20 -O3 $INC -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$G -falign-functions=64 -falign-loops=64 $EX/bench_segmented_algos.cpp -o $O/b_${G}_${CXX}_${V} 2> $O/bb_${G}_${CXX}_${V}.log
         then
            for r in $(seq 1 $REPS); do
               $PIN $O/b_${G}_${CXX}_${V} > $RAW/r_${G}_${CXX}_${V}_${r}.txt 2>/dev/null
            done
            echo "ok: group $G / $CXX / $V ($REPS runs, $(wc -l < $RAW/r_${G}_${CXX}_${V}_1.txt) lines)"
         else
            echo "BUILD FAILED: group $G / $CXX / $V"; head -25 $O/bb_${G}_${CXX}_${V}.log
         fi
      done
   done
done
echo "DONE-BENCH raw outputs in $RAW"
