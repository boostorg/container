#!/bin/bash
set -u
BOOSTROOT=/mnt/d/Data/LocalGit/boost
EXP=$BOOSTROOT/libs/container/experimental
G34=$EXP/cursor_build/g34/unroll
SNAP=$G34/snap

echo "=== toolchain ==="
g++-16 --version | head -1
clang++-22 --version | head -1
nproc
echo "=== include layout ==="
ls -ld $BOOSTROOT/boost/container/experimental 2>&1 | head -2
ls -l $BOOSTROOT/boost/container/experimental/segmented_copy_if.hpp 2>&1 | head -2
echo "=== snapshot sanity ==="
ls $SNAP/boost/container/experimental/segmented_copy_if.hpp
grep -n "define BOOST_CONTAINER_SEGMENTED_DISABLE_PRAGMA_UNROLL" $SNAP/boost/container/experimental/segmented_iterator_traits.hpp | head -5
echo "=== bench copy sanity ==="
ls -l $G34/bench_segmented_algos.cpp
md5sum $G34/bench_segmented_algos.cpp $EXP/bench_segmented_algos.cpp
