#!/bin/bash
set -euo pipefail
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental
OUT=cursor_build/g20_nsg
mkdir -p "$OUT"
COMMON="-std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20"
run_one() {
  local cxx=$1 tag=$2
  local bin="$OUT/${tag}_g20.elf"
  local log="$OUT/${tag}_g20.txt"
  echo "=== BUILD $tag ==="
  $cxx $COMMON bench_segmented_algos.cpp -o "$bin"
  echo "=== RUN $tag ==="
  taskset -c 2 "$bin" | tee "$log"
  rm -f "$bin"
}
run_one g++-16 gcc16
run_one clang++-22 clang22
echo DONE
