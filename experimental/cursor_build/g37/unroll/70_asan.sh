#!/bin/bash
# ASan + UBSan on the four primary tests, baseline and unrolled overlay.
set -u
W=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g37/unroll
T=$W/tests
OUT=$W/bin/san
mkdir -p $OUT
TESTS="segmented_copy_if_test segmented_remove_copy_test segmented_remove_copy_if_test segmented_partition_copy_test"
VARIANTS=${VARIANTS:-"base p2_all4"}
export UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1
export ASAN_OPTIONS=detect_stack_use_after_return=1:strict_string_checks=1:detect_leaks=1

for cc in g++-16 clang++-22; do
 for v in $VARIANTS; do
  if [ "$v" = "base" ]; then INC="-I$W/snap"; else INC="-I$W/exp/$v -I$W/snap"; fi
  for t in $TESTS; do
    log=$OUT/${cc}_${v}_${t}.log
    taskset -c 16-31 $cc -std=c++20 -O1 -g -fno-omit-frame-pointer \
       -fsanitize=address,undefined -fno-sanitize-recover=all \
       -Wall -Wextra $INC -I/mnt/d/Data/LocalGit/boost -I$T \
       $T/$t.cpp -o $OUT/${cc}_${v}_${t} > $log 2>&1
    if [ $? -ne 0 ] || [ "$(wc -c < $log)" != "0" ]; then
       echo "SAN-COMPILE-FAIL $cc $v $t"; head -20 $log; continue
    fi
    taskset -c 16-31 $OUT/${cc}_${v}_${t} > $log.run 2>&1
    rc=$?
    if [ $rc -ne 0 ] || grep -qE "runtime error|ERROR: (Address|Leak)Sanitizer" $log.run; then
       echo "SAN-FAIL $cc $v $t rc=$rc"; tail -30 $log.run
    else
       echo "san-ok $cc $v $t"
    fi
  done
 done
done
