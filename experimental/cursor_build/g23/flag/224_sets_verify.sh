#!/bin/bash
# Correctness of the recommended set_* patch, entirely against SHADOW headers:
# the four real headers are owned by another agent and are never written to.
set -u
BR=/mnt/d/Data/LocalGit/boost
G=/tmp/g23
SRC=$BR/libs/container/experimental/cursor_build/g23/flag
EX=$BR/libs/container/experimental
O=/tmp/g23setsv
SH=$O/guard
rm -rf $O; mkdir -p $O

ALGOS="set_union set_difference set_intersection set_symmetric_difference"

echo "--- building shadows (recommended variant) ---"
for A in $ALGOS; do
   python3 $G/220_mkshadow_sets.py $A guard \
      $SH/boost/container/experimental/segmented_$A.hpp || exit 1
done

fails=0

echo
echo "--- leaf contract probe (all four leaves, generic + dual-RA paths) ---"
for CC in g++-16 clang++-22; do
   $CC -std=c++20 -O2 -Wall -Wextra -I$SH -I$BR -I$EX \
       $SRC/223_sets_contract.cpp -o $O/contract.$CC 2>$O/contract.$CC.err
   if [ $? -ne 0 ]; then echo "  $CC: BUILD FAILED"; head -25 $O/contract.$CC.err; fails=1; continue; fi
   [ -s $O/contract.$CC.err ] && { echo "  $CC: WARNINGS"; head -20 $O/contract.$CC.err; fails=1; }
   printf "  %-12s " $CC
   $O/contract.$CC || { echo "  RUN FAILED"; fails=1; }
done

echo
echo "--- the four test files against the shadow headers ---"
for A in $ALGOS; do
   for CC in g++-16 clang++-22; do
      for STD in c++03 c++11 c++17 c++20; do
         log=$O/$A.$CC.$STD.log
         $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra -I$SH -I$BR -I$EX \
             $EX/segmented_${A}_test.cpp -o $O/$A.$CC.$STD >$log 2>&1
         if [ $? -ne 0 ]; then printf "  %-26s %-12s %-6s BUILD FAILED\n" $A $CC $STD; head -12 $log; fails=1; continue; fi
         if [ -s $log ]; then printf "  %-26s %-12s %-6s WARNINGS\n" $A $CC $STD; head -12 $log; fails=1; continue; fi
         if $O/$A.$CC.$STD >$log.run 2>&1; then printf "  %-26s %-12s %-6s pass\n" $A $CC $STD;
         else printf "  %-26s %-12s %-6s RUN FAILED\n" $A $CC $STD; tail -12 $log.run; fails=1; fi
      done
   done
done

echo
echo "--- sanitizers (asan+ubsan, c++20) ---"
for CC in g++-16 clang++-22; do
   for A in $ALGOS; do
      $CC -std=c++20 -O1 -g -Wall -Wextra -fsanitize=address,undefined \
          -fno-omit-frame-pointer -I$SH -I$BR -I$EX \
          $EX/segmented_${A}_test.cpp -o $O/s.$CC.$A >$O/s.$CC.$A.log 2>&1
      if [ $? -ne 0 ]; then printf "  %-12s %-26s BUILD FAILED\n" $CC $A; head -12 $O/s.$CC.$A.log; fails=1; continue; fi
      if $O/s.$CC.$A >$O/s.$CC.$A.run 2>&1; then printf "  %-12s %-26s clean\n" $CC $A;
      else printf "  %-12s %-26s SANITIZER FAILURE\n" $CC $A; tail -20 $O/s.$CC.$A.run; fails=1; fi
   done
   $CC -std=c++20 -O1 -g -Wall -Wextra -fsanitize=address,undefined \
       -fno-omit-frame-pointer -I$SH -I$BR -I$EX \
       $SRC/223_sets_contract.cpp -o $O/s.$CC.contract >$O/s.$CC.contract.log 2>&1 \
      && $O/s.$CC.contract >/dev/null 2>&1 \
      && printf "  %-12s %-26s clean\n" $CC 223_sets_contract \
      || { printf "  %-12s %-26s SANITIZER FAILURE\n" $CC 223_sets_contract; fails=1; }
done

echo
echo "--- depth-1 and depth-2 destination instantiation ---"
for CC in g++-16 clang++-22; do
   for A in 1 2 3 4; do
      for D in 1 2; do
         $CC -std=c++20 -O2 -DNDEBUG -DALGO=$A -DDEPTH=$D -Wall -Wextra \
             -I$SH -I$BR -I$EX -c $SRC/221_sets_probe.cpp -o $O/p.o >$O/p.log 2>&1 \
            || { printf "  %-12s ALGO=%s depth=%s FAILED\n" $CC $A $D; head -8 $O/p.log; fails=1; }
      done
   done
   printf "  %-12s all four algorithms, depths 1 and 2: ok\n" $CC
done

echo
[ $fails -eq 0 ] && echo "224: ALL OK" || echo "224: FAILURES PRESENT"
exit $fails
