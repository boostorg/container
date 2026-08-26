#!/bin/bash
# Sweep copy_if RA leaf block sizes for 2S hit/miss on GCC and Clang.
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$E"

run_one() {
   local CXX=$1 B1=$2 B2=$3 EXTRA=$4 LABEL=$5
   local DEFS="-DBOOST_CONTAINER_COPY_IF_BLOCK1=$B1 -DBOOST_CONTAINER_COPY_IF_BLOCK2=$B2 $EXTRA"
   $CXX -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 \
      $DEFS bench_segmented_algos.cpp -o /tmp/cif_bs.elf 2>/tmp/cif_bs_err.txt \
      || { echo "BUILDFAIL $LABEL"; cat /tmp/cif_bs_err.txt | head -15; return 1; }
   # 3 runs, print each; also median of seg ns for hit/miss via awk later
   echo "==== $LABEL ===="
   for i in 1 2 3; do
      taskset -c 3 /tmp/cif_bs.elf 2>/dev/null | grep -E "copy_if\(2S "
   done
}

echo "################ g++-16 ################"
for B in 8 16 32; do
   run_one g++-16 $B $B "" "gcc block1=block2=$B"
done
run_one g++-16 32 8 "" "gcc block1=32 block2=8"
run_one g++-16 16 8 "" "gcc block1=16 block2=8"
run_one g++-16 8 8 "-DBOOST_CONTAINER_COPY_IF_NO_MIDDLE" "gcc block=8 NO_MIDDLE"
run_one g++-16 32 32 "-DBOOST_CONTAINER_COPY_IF_NO_MIDDLE" "gcc block=32 NO_MIDDLE"

echo "################ clang++-22 ################"
for B in 8 16 32; do
   run_one clang++-22 $B $B "" "clang block1=block2=$B"
done
run_one clang++-22 32 8 "" "clang block1=32 block2=8"
run_one clang++-22 16 8 "" "clang block1=16 block2=8"
run_one clang++-22 8 8 "-DBOOST_CONTAINER_COPY_IF_NO_MIDDLE" "clang block=8 NO_MIDDLE"
run_one clang++-22 32 32 "-DBOOST_CONTAINER_COPY_IF_NO_MIDDLE" "clang block=32 NO_MIDDLE"
