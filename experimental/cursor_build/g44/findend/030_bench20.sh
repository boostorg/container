#!/bin/bash
set -u
D=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$D" || exit 1
OUT="$D/cursor_build/g44/findend/out"
mkdir -p "$OUT"
g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON \
   -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 -falign-functions=64 \
   bench_segmented_algos.cpp -o "$OUT/bench20.elf" 2>&1 | head -60
[ -x "$OUT/bench20.elf" ] || { echo "BUILD FAIL"; exit 1; }
taskset -c 3 "$OUT/bench20.elf"
