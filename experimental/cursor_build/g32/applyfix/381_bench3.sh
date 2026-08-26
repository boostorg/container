#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
S=$EX/cursor_build/g32/applyfix
SH=$S/pre
RAW=$S/raw3
O=/tmp/g32bench3
mkdir -p $O $RAW
rm -f $RAW/*.txt

echo "=== machine state ==="
uptime
echo "top CPU consumers:"
ps -eo pcpu,comm --sort=-pcpu | head -8

tr -d '\r' < $S/377_mkpre.py > /tmp/mkpre.py
python3 /tmp/mkpre.py || exit 1

GRPS="15 25 30"
REPS=5

if nice -n -5 true 2>/dev/null; then PIN="taskset -c 3 nice -n -5"
else echo "note: nice -n -5 refused by the kernel; taskset -c 3 only"; PIN="taskset -c 3"; fi

# Build everything first, then interleave the timed runs pre,post,pre,post,...
# so that any drift in machine state hits both variants equally.
for G in $GRPS; do
   for CXX in g++-16 clang++-22; do
      for V in pre post; do
         if [ "$V" = "pre" ]; then INC="-I$SH -I$BR"; else INC="-I$BR"; fi
         $CXX -std=c++20 -O3 $INC -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$G -falign-functions=64 -falign-loops=64 $EX/bench_segmented_algos.cpp -o $O/b_${G}_${CXX}_${V} 2> $O/bb_${G}_${CXX}_${V}.log \
            || { echo "BUILD FAILED $G $CXX $V"; head -20 $O/bb_${G}_${CXX}_${V}.log; }
      done
   done
done
echo "all builds done"

for G in $GRPS; do
   for CXX in g++-16 clang++-22; do
      for r in $(seq 1 $REPS); do
         for V in pre post; do
            $PIN $O/b_${G}_${CXX}_${V} > $RAW/r_${G}_${CXX}_${V}_${r}.txt 2>/dev/null
         done
      done
      echo "ran group $G / $CXX ($REPS interleaved pre/post pairs)"
   done
done
echo "DONE-BENCH3"
