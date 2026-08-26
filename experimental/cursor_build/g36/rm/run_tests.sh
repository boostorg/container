#!/bin/bash
# usage: run_tests.sh <compiler> <tag>
CXX="$1"
TAG="$2"
EXP=/mnt/d/Data/LocalGit/boost/libs/container/experimental
OUT=/tmp/g36_$TAG
rm -rf "$OUT"; mkdir -p "$OUT"

one() {
  f="$1"
  b=$(basename "$f" .cpp)
  log="$OUT/$b.log"
  if ! $CXX -std=c++20 -O2 -DNDEBUG -Wall -Wextra \
       -I/mnt/d/Data/LocalGit/boost -I"$EXP" \
       "$f" -o "$OUT/$b.exe" > "$log" 2>&1; then
    echo "COMPILE_FAIL $b"
    return
  fi
  if [ -s "$log" ]; then
    echo "WARNINGS $b"
    return
  fi
  if "$OUT/$b.exe" > "$OUT/$b.run" 2>&1; then
    echo "PASS $b"
  else
    echo "RUN_FAIL $b"
  fi
}
export -f one
export CXX OUT EXP

ls $EXP/segmented_*_test.cpp | xargs -P 8 -I{} bash -c 'one "$@"' _ {} | sort > "$OUT/summary.txt"
cat "$OUT/summary.txt"
echo "---- counts ----"
awk '{print $1}' "$OUT/summary.txt" | sort | uniq -c
