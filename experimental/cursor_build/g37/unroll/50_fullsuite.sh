#!/bin/bash
# Full segmented test suite: every experimental/segmented_*_test.cpp, both
# compilers, c++03/11/17/20, -O2 -DNDEBUG -Wall -Wextra.  Any compiler output
# at all is a failure.
#
# MODE=live     -> compile the live tests against the live include tree
# MODE=overlay  -> compile the live tests against snapshot + $VARIANT overlay
set -u
B=/mnt/d/Data/LocalGit/boost/libs/container
W=$B/experimental/cursor_build/g37/unroll
MODE=${MODE:-overlay}
VARIANT=${VARIANT:-p2_all}
OUT=$W/bin/suite_$MODE
mkdir -p $OUT

if [ "$MODE" = "live" ]; then
   INC="-I$B/include"
else
   INC="-I$W/exp/$VARIANT -I$W/snap"
fi

KNOWN="segmented_partition_test segmented_is_partitioned_test segmented_search_n_test segmented_search_test"

declare -A BAD
for t in $(ls $B/experimental/segmented_*_test.cpp | xargs -n1 basename | sed 's/\.cpp$//'); do
  for std in c++03 c++11 c++17 c++20; do
    for cc in g++-16 clang++-22; do
      src=$B/experimental/$t.cpp
      [ -f "$src" ] || { echo "SKIP(vanished) $t"; continue; }
      log=$OUT/${cc}_${std}_${t}.log
      taskset -c 16-31 $cc -std=$std -O2 -DNDEBUG -Wall -Wextra $INC \
          -I/mnt/d/Data/LocalGit/boost -I$B/experimental \
          $src -o $OUT/${cc}_${std}_${t} > $log 2>&1
      crc=$?
      wsz=$(wc -c < $log)
      if [ $crc -ne 0 ] || [ "$wsz" != "0" ]; then
         BAD[$t]=1; echo "FAIL-COMPILE $cc $std $t rc=$crc out=$wsz"
         continue
      fi
      taskset -c 16-31 $OUT/${cc}_${std}_${t} > $log.run 2>&1
      if [ $? -ne 0 ]; then BAD[$t]=1; echo "FAIL-RUN $cc $std $t"; fi
    done
  done
done

echo "=== failing tests ==="
FAILSET=$(echo "${!BAD[@]}" | tr ' ' '\n' | sort | tr '\n' ' ')
echo "$FAILSET"
echo "=== expected known-failing set ==="
echo "$(echo $KNOWN | tr ' ' '\n' | sort | tr '\n' ' ')"
if [ "$FAILSET" = "$(echo $KNOWN | tr ' ' '\n' | sort | tr '\n' ' ')" ]; then
   echo "RESULT: failure set is EXACTLY the four known unrelated failures"
else
   echo "RESULT: failure set DIFFERS from the known four"
fi
