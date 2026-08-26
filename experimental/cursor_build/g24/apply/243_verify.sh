#!/bin/bash
# Baseline verification of the applied patch, against the REAL headers now.
set -u
BR=/mnt/d/Data/LocalGit/boost
G23=$BR/libs/container/experimental/cursor_build/g23/flag
S22=$BR/libs/container/experimental/cursor_build/g22/deep
EX=$BR/libs/container/experimental
O=/tmp/g24ver
rm -rf $O; mkdir -p $O
fails=0

TESTS="set_union set_difference set_intersection set_symmetric_difference merge copy"

echo "=== leaf contract probe (real headers) ==="
for CC in g++-16 clang++-22; do
   if ! $CC -std=c++20 -O2 -Wall -Wextra -I$BR -I$EX \
        $G23/223_sets_contract.cpp -o $O/contract.$CC 2>$O/contract.$CC.err; then
      echo "  $CC: BUILD FAILED"; head -25 $O/contract.$CC.err; fails=1; continue
   fi
   [ -s $O/contract.$CC.err ] && { echo "  $CC: WARNINGS"; head -20 $O/contract.$CC.err; fails=1; }
   printf "  %-12s " $CC
   $O/contract.$CC || { echo "  RUN FAILED"; fails=1; }
done

echo
echo "=== 6 tests x 2 compilers x 4 stds, -Wall -Wextra must be silent ==="
for A in $TESTS; do
   for CC in g++-16 clang++-22; do
      line=""
      for STD in c++03 c++11 c++17 c++20; do
         log=$O/$A.$CC.$STD.log
         if ! $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX \
              $EX/segmented_${A}_test.cpp -o $O/$A.$CC.$STD >$log 2>&1; then
            line="$line BUILD-FAIL($STD)"; fails=1; head -12 $log | sed 's/^/      /'; continue
         fi
         if [ -s $log ]; then
            line="$line WARN($STD)"; fails=1; head -12 $log | sed 's/^/      /'; continue
         fi
         if $O/$A.$CC.$STD >$log.run 2>&1; then line="$line $STD:pass"
         else line="$line $STD:RUNFAIL"; fails=1; tail -12 $log.run | sed 's/^/      /'; fi
      done
      printf "  %-26s %-12s %s\n" $A $CC "$line"
   done
done

echo
echo "=== asan+ubsan -O1 c++20 ==="
for CC in g++-16 clang++-22; do
   for A in $TESTS; do
      if ! $CC -std=c++20 -O1 -g -Wall -Wextra -fsanitize=address,undefined \
           -fno-omit-frame-pointer -I$BR -I$EX \
           $EX/segmented_${A}_test.cpp -o $O/s.$CC.$A >$O/s.$CC.$A.log 2>&1; then
         printf "  %-12s %-26s BUILD FAILED\n" $CC $A; head -12 $O/s.$CC.$A.log; fails=1; continue
      fi
      if $O/s.$CC.$A >$O/s.$CC.$A.run 2>&1; then printf "  %-12s %-26s clean\n" $CC $A
      else printf "  %-12s %-26s SANITIZER FAILURE\n" $CC $A; tail -20 $O/s.$CC.$A.run; fails=1; fi
   done
   if $CC -std=c++20 -O1 -g -Wall -Wextra -fsanitize=address,undefined \
        -fno-omit-frame-pointer -I$BR -I$EX \
        $G23/223_sets_contract.cpp -o $O/s.$CC.contract >$O/s.$CC.contract.log 2>&1 \
      && $O/s.$CC.contract >/dev/null 2>&1; then
      printf "  %-12s %-26s clean\n" $CC 223_sets_contract
   else
      printf "  %-12s %-26s SANITIZER FAILURE\n" $CC 223_sets_contract; fails=1
   fi
done

echo
echo "=== g22 depth survey, all 14 algorithms, depths 1 and 2 ==="
NAMES=(x copy copy_if copy_n transform remove_copy remove_copy_if reverse_copy \
       swap_ranges merge set_union set_difference set_intersection \
       set_symmetric_difference partition_copy)
printf "  %-26s %-8s %-8s\n" algorithm depth1 depth2
for i in $(seq 1 14); do
   res=()
   for d in 1 2; do
      if g++-16 -std=c++20 -O1 -DNDEBUG -DALGO=$i -DDEPTH=$d -I$BR -I$EX \
            -c $S22/202_survey.cpp -o $O/a.o 2>$O/e.$i.$d.log; then res+=("ok")
      else res+=("FAIL"); fails=1; fi
   done
   printf "  %-26s %-8s %-8s\n" "${NAMES[$i]}" "${res[0]}" "${res[1]}"
done

echo
[ $fails -eq 0 ] && echo "243: ALL OK" || echo "243: FAILURES PRESENT"
exit $fails
