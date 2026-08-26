#!/bin/bash
# Class D (merge/set_*/partition_copy) gate: build+run tests on both compilers
# at c++03 and c++20, -Wall -Wextra silent; then compare .text and per-symbol
# merge walker sizes against the pre-H baseline binaries in bin_before.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
BB=$BR/libs/container/experimental/cursor_build/g29/rollout/bin_before
O=/tmp/g29D
rm -rf $O; mkdir -p $O
fails=0

TESTS="merge set_union set_difference set_intersection set_symmetric_difference partition_copy"

echo "=== class D test matrix ==="
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
      printf "  %-26s %-12s %s\n" $A $CC "$line"
   done
done

echo
echo "=== .text before/after (c++20 -O2 binaries) ==="
for A in $TESTS; do
   for CC in g++-16 clang++-22; do
      b=$(size -A $BB/segmented_${A}_test.$CC 2>/dev/null | awk '$1==".text"{print $2}')
      a=$(size -A $O/$A.$CC.c++20 2>/dev/null | awk '$1==".text"{print $2}')
      printf "  %-26s %-12s before=%-8s after=%-8s delta=%s\n" $A $CC "$b" "$a" $((a-b))
   done
done

echo
echo "=== merge walker symbol sizes before/after (g++-16) ==="
for W in merge_dst_bounded merge_seg2_dispatch merge_scan; do
   b=$(objdump -tC $BB/segmented_merge_test.g++-16 | grep -F "$W" | awk '{s+="0x"$(NF-1)} END{printf "%d", s}')
   a=$(objdump -tC $O/merge.g++-16.c++20 | grep -F "$W" | awk '{s+="0x"$(NF-1)} END{printf "%d", s}')
   printf "  %-22s before=%-7s after=%-7s\n" $W "$b" "$a"
done

echo
[ $fails -eq 0 ] && echo "303: ALL OK" || echo "303: FAILURES PRESENT"
exit $fails
