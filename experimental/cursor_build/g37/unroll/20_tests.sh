#!/bin/bash
# Correctness gate: the four predicate-count tests, snapshot headers + overlay variant.
set -u
B=/mnt/d/Data/LocalGit/boost/libs/container
W=$B/experimental/cursor_build/g37/unroll
E=$W/exp
T=$W/tests
OUT=$W/bin/t
mkdir -p $OUT

TESTS="segmented_copy_if_test segmented_remove_copy_test segmented_remove_copy_if_test segmented_partition_copy_test"
VARIANTS=${VARIANTS:-"base p2_copyif p2_rc p2_rci p2_all p2_all4 p2_pc"}
STDS=${STDS:-"c++20"}

fail=0
for cc in g++-16 clang++-22; do
 for v in $VARIANTS; do
  if [ "$v" = "base" ]; then INC="-I$W/snap"; else INC="-I$E/$v -I$W/snap"; fi
  for std in $STDS; do
   for t in $TESTS; do
     log=$OUT/${cc}_${v}_${std}_${t}.log
     $cc -std=$std -O2 -DNDEBUG -Wall -Wextra $INC -I/mnt/d/Data/LocalGit/boost \
        -I$T $T/$t.cpp -o $OUT/${cc}_${v}_${std}_${t} > $log 2>&1
     crc=$?
     wsz=$(wc -c < $log)
     if [ $crc -ne 0 ] || [ "$wsz" != "0" ]; then
        echo "COMPILE-FAIL $cc $v $std $t rc=$crc warnbytes=$wsz"; head -20 $log; fail=1; continue
     fi
     $OUT/${cc}_${v}_${std}_${t} > $log.run 2>&1
     rrc=$?
     if [ $rrc -ne 0 ]; then
        echo "RUN-FAIL $cc $v $std $t rc=$rrc"; tail -20 $log.run; fail=1
     else
        echo "ok $cc $v $std $t"
     fi
   done
  done
 done
done
echo "OVERALL fail=$fail"
