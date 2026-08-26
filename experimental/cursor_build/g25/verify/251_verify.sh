#!/bin/bash
# Full correctness matrix for the unverified working tree.
set -u
BR=/mnt/d/Data/LocalGit/boost
G25=$BR/libs/container/experimental/cursor_build/g25/verify
S22=$BR/libs/container/experimental/cursor_build/g22/deep
EX=$BR/libs/container/experimental
O=/tmp/g25ver
rm -rf $O; mkdir -p $O
fails=0

TESTS="set_union set_difference set_intersection set_symmetric_difference \
       merge copy partition_copy partition"

echo "=== 250 leaf-contract probe, 2 compilers x 4 stds, -Wall -Wextra silent ==="
for CC in g++-16 clang++-22; do
   for STD in c++03 c++11 c++17 c++20; do
      log=$O/c.$CC.$STD.log
      if ! $CC -std=$STD -O2 -Wall -Wextra -I$BR -I$EX \
           $G25/250_contract.cpp -o $O/c.$CC.$STD >$log 2>&1; then
         echo "  $CC $STD: BUILD FAILED"; head -25 $log | sed 's/^/      /'; fails=1; continue
      fi
      if [ -s $log ]; then
         echo "  $CC $STD: WARNINGS"; head -25 $log | sed 's/^/      /'; fails=1; continue
      fi
      printf "  %-12s %-7s " $CC $STD
      $O/c.$CC.$STD || { echo "  PROBE FAILED"; fails=1; }
   done
done

echo
echo "=== 8 tests x 2 compilers x 4 stds, -Wall -Wextra must be silent ==="
for A in $TESTS; do
   for CC in g++-16 clang++-22; do
      line=""
      for STD in c++03 c++11 c++17 c++20; do
         log=$O/$A.$CC.$STD.log
         if ! $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra -I$BR -I$EX \
              $EX/segmented_${A}_test.cpp -o $O/$A.$CC.$STD >$log 2>&1; then
            line="$line BUILD-FAIL($STD)"; fails=1; head -15 $log | sed 's/^/      /'; continue
         fi
         if [ -s $log ]; then
            line="$line WARN($STD)"; fails=1; head -15 $log | sed 's/^/      /'; continue
         fi
         if $O/$A.$CC.$STD >$log.run 2>&1; then line="$line $STD:pass"
         else line="$line $STD:RUNFAIL"; fails=1; tail -15 $log.run | sed 's/^/      /'; fi
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
         printf "  %-12s %-26s BUILD FAILED\n" $CC $A; head -15 $O/s.$CC.$A.log; fails=1; continue
      fi
      if $O/s.$CC.$A >$O/s.$CC.$A.run 2>&1; then printf "  %-12s %-26s clean\n" $CC $A
      else printf "  %-12s %-26s SANITIZER FAILURE\n" $CC $A; tail -25 $O/s.$CC.$A.run; fails=1; fi
   done
   if $CC -std=c++20 -O1 -g -Wall -Wextra -fsanitize=address,undefined \
        -fno-omit-frame-pointer -I$BR -I$EX \
        $G25/250_contract.cpp -o $O/s.$CC.contract >$O/s.$CC.contract.log 2>&1 \
      && $O/s.$CC.contract >/dev/null 2>&1; then
      printf "  %-12s %-26s clean\n" $CC 250_contract
   else
      printf "  %-12s %-26s SANITIZER FAILURE\n" $CC 250_contract
      tail -25 $O/s.$CC.contract.log; fails=1
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
for i in $(seq 1 14); do
   for d in 1 2; do
      if [ -s $O/e.$i.$d.log ] && grep -q 'error:' $O/e.$i.$d.log; then
         echo "  --- ${NAMES[$i]} depth=$d"; grep -m1 -A3 'error:' $O/e.$i.$d.log | sed 's/^/      /'
      fi
   done
done

echo
[ $fails -eq 0 ] && echo "251: ALL OK" || echo "251: FAILURES PRESENT"
exit $fails
