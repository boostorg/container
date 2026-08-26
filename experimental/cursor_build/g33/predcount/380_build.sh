#!/bin/bash
# Build+run matrix: g++-16 / clang++-22 x c++03/11/17/20, -O2 -DNDEBUG -Wall -Wextra.
# Zero compiler output is required.  Usage: 380_build.sh <test_basename> ...
ROOT=/mnt/d/Data/LocalGit/boost
TD=$ROOT/libs/container/experimental
OUT=/tmp/pc
rm -rf $OUT
mkdir -p $OUT
FLAGS="-O2 -DNDEBUG -Wall -Wextra -I$ROOT -I$TD"

JOBS=$(nproc)
CMDS=$OUT/cmds.txt
: > $CMDS
for t in "$@"; do
  for cc in g++-16 clang++-22; do
    for std in c++03 c++11 c++17 c++20; do
      echo "$t|$cc|$std" >> $CMDS
    done
  done
done

run_one()
{
  IFS='|' read -r t cc std <<< "$1"
  ROOT=/mnt/d/Data/LocalGit/boost
  TD=$ROOT/libs/container/experimental
  OUT=/tmp/pc
  FLAGS="-O2 -DNDEBUG -Wall -Wextra -I$ROOT -I$TD"
  tag="$t.$cc.$std"
  $cc -std=$std $FLAGS "$TD/$t.cpp" -o "$OUT/$tag.elf" > "$OUT/$tag.cc.log" 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "BUILD-FAIL $tag" > "$OUT/$tag.res"
    return
  fi
  if [ -s "$OUT/$tag.cc.log" ]; then
    echo "WARNINGS   $tag" > "$OUT/$tag.res"
    return
  fi
  "$OUT/$tag.elf" > "$OUT/$tag.run.log" 2>&1
  rc=$?
  if [ $rc -ne 0 ]; then
    echo "RUN-FAIL   $tag" > "$OUT/$tag.res"
  else
    echo "OK         $tag" > "$OUT/$tag.res"
  fi
}
export -f run_one

xargs -a $CMDS -P $JOBS -I{} bash -c 'run_one "$@"' _ {}

cat $OUT/*.res | sort > $OUT/summary.txt
echo "=== FAILURES ==="
grep -v '^OK' $OUT/summary.txt
echo "=== TOTALS ==="
awk '{print $1}' $OUT/summary.txt | sort | uniq -c
echo "=== COMPILER OUTPUT ==="
for f in $OUT/*.cc.log; do
  if [ -s "$f" ]; then echo "== $f"; head -40 "$f"; fi
done
echo "=== TEST OUTPUT (failures only) ==="
for r in $OUT/*.res; do
  if ! grep -q '^OK' "$r"; then
    f="${r%.res}.run.log"
    if [ -s "$f" ]; then echo "== $f"; head -40 "$f"; fi
  fi
done
