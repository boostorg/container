#!/bin/bash
# 360: reproduce the user's exact command for both compilers, one run each.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
WD=$EX/cursor_build/g31/copyif
cd "$EX" || exit 1

FLAGS="-std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 -falign-functions=64 -falign-loops=64"

echo "=== nproc: $(nproc)  ==="
grep -m1 'model name' /proc/cpuinfo
echo

for CXX in g++-16 clang++-22; do
  echo "===== BUILD $CXX ====="
  t0=$(date +%s)
  $CXX $FLAGS bench_segmented_algos.cpp -o "$WD/a_${CXX}.elf" 2>&1 | tail -20
  t1=$(date +%s)
  echo "build seconds: $((t1-t0))"
  echo "===== RUN $CXX (user's exact command, unpinned) ====="
  t0=$(date +%s)
  "$WD/a_${CXX}.elf" > "$WD/360_run_${CXX}.txt" 2>&1
  t1=$(date +%s)
  echo "run seconds: $((t1-t0))"
  cat "$WD/360_run_${CXX}.txt"
  echo
done
