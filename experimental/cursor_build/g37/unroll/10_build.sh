#!/bin/bash
# Build the group-25/30 benchmark from the g37 snapshot for a set of overlay variants.
set -u
B=/mnt/d/Data/LocalGit/boost/libs/container
W=$B/experimental/cursor_build/g37/unroll
E=$W/exp
OUT=$W/bin
mkdir -p $OUT

GROUP=${GROUP:-25}
FLAGS="-O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$GROUP -falign-functions=64 -falign-loops=64"

VARIANTS=${VARIANTS:-"base p2_copyif p2_all p2_all4 p1_cleanup p2ci_p1 p2_pc p2_rc p2_rci"}

for cc in g++-16 clang++-22; do
   for v in $VARIANTS; do
      if [ "$v" = "base" ]; then INC="-I$W/snap"; else INC="-I$E/$v -I$W/snap"; fi
      bin=$OUT/b_${cc}_${v}_g${GROUP}
      t0=$(date +%s.%N)
      $cc -std=c++20 $FLAGS $INC -I/mnt/d/Data/LocalGit/boost \
          $W/bench_g37.cpp -o $bin 2> $OUT/log_${cc}_${v}_g${GROUP}.txt
      rc=$?
      t1=$(date +%s.%N)
      sz=$(stat -c %s $bin 2>/dev/null || echo NA)
      echo "$cc $v g$GROUP rc=$rc  $(echo "$t1-$t0" | bc)s  size=$sz  warn=$(wc -c < $OUT/log_${cc}_${v}_g${GROUP}.txt)"
   done
done
