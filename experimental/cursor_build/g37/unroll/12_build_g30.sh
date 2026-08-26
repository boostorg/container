#!/bin/bash
# Group 30 (3-range algorithms, contains partition_copy).  Compilation is kept
# off core 3 so it does not disturb a benchmark pinned there.
set -u
W=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g37/unroll
E=$W/exp
OUT=$W/bin
mkdir -p $OUT
FLAGS="-O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=30 -DBENCH_G37_PARTITION_COPY_ONLY -falign-functions=64 -falign-loops=64"
for cc in g++-16 clang++-22; do
   for v in base p2_pc; do
      if [ "$v" = "base" ]; then INC="-I$W/snap"; else INC="-I$E/$v -I$W/snap"; fi
      taskset -c 16-31 $cc -std=c++20 $FLAGS $INC -I/mnt/d/Data/LocalGit/boost \
         $W/bench_g37.cpp -o $OUT/b_${cc}_${v}_g30 2> $OUT/log_${cc}_${v}_g30.txt
      echo "$cc $v g30 rc=$? warn=$(wc -c < $OUT/log_${cc}_${v}_g30.txt)"
   done
done
