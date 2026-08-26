#!/bin/bash
# The six tests, now carrying depth-3 destination coverage, over both compilers
# and all four standards, plus how long the new coverage costs.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
O=/tmp/g24t
rm -rf $O; mkdir -p $O
fails=0
TESTS="${TESTS:-set_union set_difference set_intersection set_symmetric_difference merge copy}"
STDS="${STDS:-c++03 c++11 c++17 c++20}"

for A in $TESTS; do
   for CC in g++-16 clang++-22; do
      line=""
      for STD in $STDS; do
         log=$O/$A.$CC.$STD.log
         if ! $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX \
              $EX/segmented_${A}_test.cpp -o $O/$A.$CC.$STD >$log 2>&1; then
            line="$line BUILD-FAIL($STD)"; fails=1; head -25 $log | sed 's/^/      /'; continue
         fi
         if [ -s $log ]; then
            line="$line WARN($STD)"; fails=1; head -20 $log | sed 's/^/      /'; continue
         fi
         t0=$(date +%s%N)
         if $O/$A.$CC.$STD >$log.run 2>&1; then
            t1=$(date +%s%N)
            line="$line $STD:pass($(( (t1-t0)/1000000 ))ms)"
         else
            line="$line $STD:RUNFAIL"; fails=1; tail -15 $log.run | sed 's/^/      /'
         fi
      done
      printf "  %-26s %-12s %s\n" $A $CC "$line"
   done
done
echo
[ $fails -eq 0 ] && echo "246: ALL OK" || echo "246: FAILURES PRESENT"
exit $fails
