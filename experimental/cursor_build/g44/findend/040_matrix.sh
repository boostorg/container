#!/bin/bash
# 2 compilers x 4 standards for the new test, plus the two neighbours.
set -u
D=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$D" || exit 1
OUT="$D/cursor_build/g44/findend/out/matrix"
rm -rf "$OUT"; mkdir -p "$OUT"

TESTS="segmented_find_end_test segmented_search_test segmented_mismatch_test"
FAIL=0

for cxx in g++-16 clang++-22; do
   for std in c++03 c++11 c++17 c++20; do
      for t in $TESTS; do
         tag="${cxx}_${std}_${t}"
         log="$OUT/$tag.log"
         $cxx -std=$std -Wall -Wextra -O2 -I../../.. "$t.cpp" -o "$OUT/$tag.elf" > "$log" 2>&1
         rc=$?
         warn=$(grep -c 'warning:' "$log")
         if [ $rc -ne 0 ]; then
            echo "BUILD-FAIL $tag (warnings=$warn)"; FAIL=1; head -30 "$log"; continue
         fi
         "$OUT/$tag.elf" > "$log.run" 2>&1
         rrc=$?
         if [ $rrc -ne 0 ]; then
            echo "RUN-FAIL   $tag"; FAIL=1; head -20 "$log.run"; continue
         fi
         if [ "$warn" != "0" ]; then
            echo "WARN       $tag warnings=$warn"; FAIL=1; grep 'warning:' "$log" | head -10
         else
            echo "OK         $tag"
         fi
      done
   done
done

echo "----"
[ $FAIL -eq 0 ] && echo "MATRIX ALL OK" || echo "MATRIX HAS FAILURES"
