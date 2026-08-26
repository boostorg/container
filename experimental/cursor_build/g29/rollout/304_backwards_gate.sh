#!/bin/bash
# Backwards family gate: find_last / find_last_if / find_last_if_not /
# reverse_copy (source dispatch converted in this batch) / reverse (untouched,
# regression check).  Build+run both compilers c++03/c++20, then .text
# before/after vs bin_before.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
BB=$BR/libs/container/experimental/cursor_build/g29/rollout/bin_before
O=/tmp/g29E
rm -rf $O; mkdir -p $O
fails=0

TESTS="find_last find_last_if find_last_if_not reverse_copy reverse"

echo "=== backwards family test matrix ==="
for A in $TESTS; do
   for CC in g++-16 clang++-22; do
      line=""
      for STD in c++03 c++20; do
         log=$O/$A.$CC.$STD.log
         if ! $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX \
              $EX/segmented_${A}_test.cpp -o $O/$A.$CC.$STD >$log 2>&1; then
            line="$line BUILD-FAIL($STD)"; fails=1; head -20 $log | sed 's/^/      /'; continue
         fi
         if [ -s $log ]; then
            line="$line WARN($STD)"; fails=1; head -20 $log | sed 's/^/      /'; continue
         fi
         if $O/$A.$CC.$STD >$log.run 2>&1; then line="$line $STD:pass"
         else line="$line $STD:RUNFAIL"; fails=1; tail -15 $log.run | sed 's/^/      /'; fi
      done
      printf "  %-18s %-12s %s\n" $A $CC "$line"
   done
done

echo
echo "=== .text before/after (c++20 -O2 binaries) ==="
for A in $TESTS; do
   for CC in g++-16 clang++-22; do
      b=$(size -A $BB/segmented_${A}_test.$CC 2>/dev/null | awk '$1==".text"{print $2}')
      a=$(size -A $O/$A.$CC.c++20 2>/dev/null | awk '$1==".text"{print $2}')
      printf "  %-18s %-12s before=%-8s after=%-8s delta=%s\n" $A $CC "$b" "$a" $((a-b))
   done
done

echo
[ $fails -eq 0 ] && echo "304: ALL OK" || echo "304: FAILURES PRESENT"
exit $fails
