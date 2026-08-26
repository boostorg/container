#!/bin/bash
set -euo pipefail
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental
OUT=cursor_build/ra_bench
mkdir -p "$OUT"
COMMON="-std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25"
run_one() {
  local cxx=$1 ra=$2 tag=$3
  local bin="$OUT/${tag}_ra${ra}.elf"
  local log="$OUT/${tag}_ra${ra}.txt"
  echo "=== BUILD $tag RA=$ra ==="
  $cxx $COMMON -DBOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS=$ra \
    bench_segmented_algos.cpp -o "$bin"
  echo "=== RUN $tag RA=$ra ==="
  taskset -c 2 "$bin" | tee "$log"
  rm -f "$bin"
}
for ra in 0 1; do
  run_one g++-16 "$ra" gcc16
  run_one clang++-22 "$ra" clang22
done
echo DONE
