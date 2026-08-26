#!/bin/bash
set -u
BOOSTROOT=/mnt/d/Data/LocalGit/boost
EXP=$BOOSTROOT/libs/container/experimental
G34=$EXP/cursor_build/g34/unroll
SNAP=$G34/snap
FLAGS="-std=c++20 -O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 -falign-functions=64 -falign-loops=64"
cd $G34
echo "=== raw error ==="
g++-16 $FLAGS -I$SNAP -I$BOOSTROOT bench_segmented_algos.cpp -o /tmp/x.elf 2>&1 | head -20
echo "=== includes in bench ==="
grep -n '#include' bench_segmented_algos.cpp | head -30
