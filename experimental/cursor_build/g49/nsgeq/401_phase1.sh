#!/bin/bash
# Phase 1 (header edit OFF): GCC baseline, default align and -falign-loops=64.
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$E"
ROWS="equal\(1\+2S|mismatch\(1\+2S|mismatch_2r\(1\+2S|equal\(1S hit|search\("

echo "########## A: baseline, default align ##########"
/usr/bin/time -f "compile: %es  peak %MKB" g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   bench_segmented_algos.cpp -o /tmp/g49xa.elf 2>&1 | tail -1
taskset -c 3 /tmp/g49xa.elf 2>/dev/null | grep -E "$ROWS"

echo "########## B: baseline, -falign-loops=64 ##########"
g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   -falign-loops=64 bench_segmented_algos.cpp -o /tmp/g49xb.elf
taskset -c 3 /tmp/g49xb.elf 2>/dev/null | grep -E "$ROWS"
