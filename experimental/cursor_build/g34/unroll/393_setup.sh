#!/bin/bash
set -eu
BOOSTROOT=/mnt/d/Data/LocalGit/boost
EXP=$BOOSTROOT/libs/container/experimental
G34=$EXP/cursor_build/g34/unroll
SNAP=$G34/snap

echo "=== quoted includes in bench ==="
grep -n '#include "' $G34/bench_segmented_algos.cpp

mkdir -p $G34/exp
cp $G34/bench_segmented_algos.cpp $G34/exp/
cp $G34/segmented_test_helper.hpp $G34/exp/ 2>/dev/null || true
rm -rf $G34/bench
cp -r $BOOSTROOT/libs/container/bench $G34/bench
echo "=== isolated tree ==="
ls $G34/exp $G34/bench | head -20

FLAGS="-std=c++20 -O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 -falign-functions=64 -falign-loops=64"
cd $G34/exp
echo "=== snap build g++ ==="
g++-16 $FLAGS -I$SNAP -I$BOOSTROOT bench_segmented_algos.cpp -o $G34/gcc_snap.elf 2>&1 | head -10
echo "=== snap build clang ==="
clang++-22 $FLAGS -I$SNAP -I$BOOSTROOT bench_segmented_algos.cpp -o $G34/clang_snap.elf 2>&1 | head -10
echo "=== sizes ==="
ls -l $G34/*.elf
echo "=== verify snap headers actually used (g++ -H count) ==="
g++-16 $FLAGS -I$SNAP -I$BOOSTROOT -H -fsyntax-only bench_segmented_algos.cpp 2>&1 | grep -c "$SNAP/boost/container/experimental"
g++-16 $FLAGS -I$SNAP -I$BOOSTROOT -H -fsyntax-only bench_segmented_algos.cpp 2>&1 | grep -c "$BOOSTROOT/boost/container/experimental"
echo DONE
