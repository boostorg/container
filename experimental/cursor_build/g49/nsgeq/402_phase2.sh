#!/bin/bash
# Phase 2 (FORCEINLINE ON in seg-iter2 walker): GCC default align + clang.
# Also verify the per-segment out-of-line call at 0x25a0-style is gone.
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$E"
ROWS="equal\(1\+2S|mismatch\(1\+2S|mismatch_2r\(1\+2S|equal\(1S hit|search\("

echo "########## C: FORCEINLINE, default align ##########"
/usr/bin/time -f "compile: %es  peak %MKB" g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   bench_segmented_algos.cpp -o /tmp/g49xc.elf 2>&1 | tail -1
taskset -c 3 /tmp/g49xc.elf 2>/dev/null | grep -E "$ROWS"

echo "########## out-of-line leaf walkers remaining (GCC) ##########"
nm -C /tmp/g49xc.elf | grep -c "segmented_iter2_bounded" || true

echo "########## D: FORCEINLINE + -falign-loops=64 ##########"
g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   -falign-loops=64 bench_segmented_algos.cpp -o /tmp/g49xd.elf
taskset -c 3 /tmp/g49xd.elf 2>/dev/null | grep -E "$ROWS"

echo "########## E: clang + FORCEINLINE, user flags ##########"
/usr/bin/time -f "compile: %es  peak %MKB" clang++-22 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   -falign-functions=64 -falign-loops=64 bench_segmented_algos.cpp -o /tmp/g49xe.elf 2>&1 | tail -1
taskset -c 3 /tmp/g49xe.elf 2>/dev/null | grep -E "$ROWS"
