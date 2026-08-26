#!/bin/bash
EXP=/mnt/d/Data/LocalGit/boost/libs/container/experimental
OUT=/tmp/g36_bench
rm -rf "$OUT"; mkdir -p "$OUT"

build() {
  tag="$1"; shift
  log="$OUT/$tag.log"
  if g++-16 -std=c++20 -O3 -DBENCH_ON -DNDEBUG "$@" \
       -I/mnt/d/Data/LocalGit/boost -I"$EXP" \
       "$EXP/bench_segmented_algos.cpp" -o "$OUT/$tag.exe" > "$log" 2>&1; then
    if [ -s "$log" ]; then echo "OUTPUT $tag"; cat "$log"; else echo "OK $tag"; fi
  else
    echo "FAIL $tag"; cat "$log"
  fi
}

build nomacro
for g in 0 10 15 17 20 25 27 30; do
  build g$g -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=$g
done
