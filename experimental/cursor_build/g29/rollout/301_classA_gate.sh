#!/bin/bash
# Class A (full-scan singles) gate: build+run affected tests on both compilers
# at c++03 and c++20, -Wall -Wextra silent; then per-symbol statics +
# leaf-specialization fingerprint for fill/count via the g28 probe TU.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
G28=$BR/libs/container/experimental/cursor_build/g28/pattern
O=/tmp/g29A
rm -rf $O; mkdir -p $O
fails=0

TESTS="fill count count_if for_each generate replace replace_if partition stable_partition"

known_warn_ok() {
   grep 'warning:' "$1" | grep -v 'segmented_fill_test.cpp:52:.*C++11 extension' | grep -q . && return 1
   return 0
}

echo "=== class A test matrix ==="
for A in $TESTS; do
   for CC in g++-16 clang++-22; do
      line=""
      for STD in c++03 c++20; do
         log=$O/$A.$CC.$STD.log
         if ! $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX \
              $EX/segmented_${A}_test.cpp -o $O/$A.$CC.$STD >$log 2>&1; then
            line="$line BUILD-FAIL($STD)"; fails=1; head -20 $log | sed 's/^/      /'; continue
         fi
         if [ -s $log ] && ! known_warn_ok $log; then
            line="$line WARN($STD)"; fails=1; head -20 $log | sed 's/^/      /'; continue
         fi
         if $O/$A.$CC.$STD >$log.run 2>&1; then line="$line $STD:pass"
         else line="$line $STD:RUNFAIL"; fails=1; tail -15 $log.run | sed 's/^/      /'; fi
      done
      printf "  %-18s %-12s %s\n" $A $CC "$line"
   done
done

echo
echo "=== per-symbol statics (probe TU, real headers) ==="
for CC in g++-16 clang++-22; do
   $CC -std=c++20 -O2 -DNDEBUG -I$BR -I$EX -c $G28/280_probe.cpp -o $O/probe.$CC.o 2>$O/probe.$CC.log \
      || { echo "PROBE BUILD FAIL $CC"; fails=1; continue; }
   objdump -dC $O/probe.$CC.o > $O/lst.$CC.txt
   for P in "probe_fill_deq" "probe_count_deq"; do
      body=$(awk "/<$P/{f=1} f&&/^\$/{exit} f" $O/lst.$CC.txt)
      simd=$(echo "$body" | grep -cE 'paddd|vpaddd|pcmpeq|vpcmpeq|movdqu|vmovdq')
      insns=$(echo "$body" | grep -cE '^\s+[0-9a-f]+:')
      cbound=$(echo "$body" | grep -c '0x400')
      printf "  %-12s %-18s insns=%-5s simd=%-4s const-bound=%s\n" $CC $P "$insns" "$simd" "$cbound"
   done
   # out-of-line walkers (fill/count dispatch), if any
   objdump -tC $O/probe.$CC.o | grep -E 'segmented_(fill|count)' | awk '{printf "  %s ool: %s %s\n", "'$CC'", $(NF-1), $NF}'
done

echo
[ $fails -eq 0 ] && echo "301: ALL OK" || echo "301: FAILURES PRESENT"
exit $fails
