#!/bin/bash
# Experiment 1: GCC baseline vs -falign-loops=64 (placement check, seg column).
# Experiment 2: FORCEINLINE on seg-iter2 leaf walker (already edited in header).
# The header edit is active in ALL builds here; the git stash build gives the
# pre-edit baseline.
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
R=/mnt/d/Data/LocalGit/boost/libs/container
cd "$E"

ROWS="equal\(1\+2S|mismatch\(1\+2S|mismatch_2r\(1\+2S|equal\(1S hit|search\("

echo "########## A: pre-edit baseline (git stash), default align ##########"
cd "$R" && git stash -q && cd "$E"
/usr/bin/time -f "compile: %es %MKB" g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   bench_segmented_algos.cpp -o /tmp/g49xa.elf 2>&1 | tail -1
taskset -c 3 /tmp/g49xa.elf 2>/dev/null | grep -E "$ROWS"

echo "########## B: pre-edit baseline + -falign-loops=64 ##########"
g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   -falign-loops=64 bench_segmented_algos.cpp -o /tmp/g49xb.elf
taskset -c 3 /tmp/g49xb.elf 2>/dev/null | grep -E "$ROWS"

cd "$R" && git stash pop -q && cd "$E"
echo "########## C: FORCEINLINE seg-iter2 walker, default align ##########"
/usr/bin/time -f "compile: %es %MKB" g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   bench_segmented_algos.cpp -o /tmp/g49xc.elf 2>&1 | tail -1
taskset -c 3 /tmp/g49xc.elf 2>/dev/null | grep -E "$ROWS"

echo "########## D: FORCEINLINE + -falign-loops=64 ##########"
g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   -falign-loops=64 bench_segmented_algos.cpp -o /tmp/g49xd.elf
taskset -c 3 /tmp/g49xd.elf 2>/dev/null | grep -E "$ROWS"

echo "########## E: clang + FORCEINLINE, user flags ##########"
clang++-22 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   -falign-functions=64 -falign-loops=64 bench_segmented_algos.cpp -o /tmp/g49xe.elf
taskset -c 3 /tmp/g49xe.elf 2>/dev/null | grep -E "$ROWS"
