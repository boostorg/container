#!/bin/bash
# A/B benchmark: working tree ("post") vs the four touched headers at HEAD
# ("pre").  Usage: 407_bench.sh <group> <reps> <cc> [cc...]
set -e
ROOT=/mnt/d/Data/LocalGit/boost
TD=$ROOT/libs/container/experimental
PRE=/tmp/pf39pre
OUT=/tmp/pf39bench
GROUP=$1; shift
REPS=$1; shift
mkdir -p $OUT

FLAGS="-DBENCH_ON -DNDEBUG -O3 -falign-functions=64 -std=c++20 -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$GROUP"

for cc in "$@"; do
  for side in pre post; do
    bin=$OUT/$cc.$side.g$GROUP.elf
    if [ ! -x "$bin" ]; then
      if [ "$side" = pre ]; then INC="-I$PRE -I$ROOT -I$TD"; else INC="-I$ROOT -I$TD"; fi
      echo "building $cc $side group $GROUP" >&2
      $cc $FLAGS $INC "$TD/bench_segmented_algos.cpp" -o "$bin" 2>&1 | head -20
    fi
  done
done

for r in $(seq 1 $REPS); do
  for cc in "$@"; do
    for side in pre post; do
      taskset -c 4 "$OUT/$cc.$side.g$GROUP.elf" > "$OUT/$cc.$side.g$GROUP.r$r.log" 2>&1
    done
  done
done
echo "done group $GROUP reps $REPS" >&2
