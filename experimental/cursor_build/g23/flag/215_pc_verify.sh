#!/bin/bash
# Correctness of the partition_copy flag flip, measured against the SHADOW
# header (the real header is left untouched until the evidence is in).
set -u
BR=/mnt/d/Data/LocalGit/boost
G=/tmp/g23
SRC=$BR/libs/container/experimental/cursor_build/g23/flag
EX=$BR/libs/container/experimental
O=/tmp/g23pcv
SH=$O/shadow
rm -rf $O; mkdir -p $O

python3 $G/211_mkshadow_pc.py $SH/boost/container/experimental/segmented_partition_copy.hpp || exit 1

fails=0

echo "--- leaf contract probe (shadow leaf vs pre-change leaf) ---"
for CC in g++-16 clang++-22; do
   $CC -std=c++20 -O2 -Wall -Wextra -I$SH -I$BR -I$EX \
       $SRC/214_pc_contract.cpp -o $O/contract.$CC 2>$O/contract.$CC.err
   if [ $? -ne 0 ]; then echo "  $CC: BUILD FAILED"; head -20 $O/contract.$CC.err; fails=1; continue; fi
   [ -s $O/contract.$CC.err ] && { echo "  $CC: WARNINGS"; cat $O/contract.$CC.err; fails=1; }
   printf "  %-12s " $CC
   $O/contract.$CC || { echo "  RUN FAILED"; fails=1; }
done

echo
echo "--- segmented_partition_copy_test.cpp against the shadow header ---"
for CC in g++-16 clang++-22; do
   for STD in c++03 c++11 c++17 c++20; do
      log=$O/t.$CC.$STD.log
      $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra -I$SH -I$BR -I$EX \
          $EX/segmented_partition_copy_test.cpp -o $O/t.$CC.$STD >$log 2>&1
      if [ $? -ne 0 ]; then printf "  %-12s %-6s BUILD FAILED\n" $CC $STD; head -15 $log; fails=1; continue; fi
      if [ -s $log ]; then printf "  %-12s %-6s WARNINGS\n" $CC $STD; head -15 $log; fails=1; continue; fi
      if $O/t.$CC.$STD >$log.run 2>&1; then printf "  %-12s %-6s pass\n" $CC $STD;
      else printf "  %-12s %-6s RUN FAILED\n" $CC $STD; tail -15 $log.run; fails=1; fi
   done
done

echo
echo "--- sanitizers (asan+ubsan, c++20) ---"
for CC in g++-16 clang++-22; do
   for T in $EX/segmented_partition_copy_test.cpp $SRC/214_pc_contract.cpp; do
      b=$(basename $T .cpp)
      $CC -std=c++20 -O1 -g -Wall -Wextra -fsanitize=address,undefined \
          -fno-omit-frame-pointer -I$SH -I$BR -I$EX $T -o $O/s.$CC.$b >$O/s.$CC.$b.log 2>&1
      if [ $? -ne 0 ]; then printf "  %-12s %-34s BUILD FAILED\n" $CC $b; head -15 $O/s.$CC.$b.log; fails=1; continue; fi
      if $O/s.$CC.$b >$O/s.$CC.$b.run 2>&1; then printf "  %-12s %-34s clean\n" $CC $b;
      else printf "  %-12s %-34s SANITIZER FAILURE\n" $CC $b; tail -20 $O/s.$CC.$b.run; fails=1; fi
   done
done

echo
echo "--- depth-2 output end-to-end (survey probe, shadow) ---"
for CC in g++-16 clang++-22; do
   for D in 1 2; do
      $CC -std=c++20 -O2 -DNDEBUG -DDEPTH=$D -Wall -Wextra -I$SH -I$BR -I$EX \
          -c $SRC/210_pc_probe.cpp -o $O/p.o >$O/p.$CC.$D.log 2>&1 \
         && printf "  %-12s depth=%s compiles clean\n" $CC $D \
         || { printf "  %-12s depth=%s FAILED\n" $CC $D; head -15 $O/p.$CC.$D.log; fails=1; }
   done
done

echo
[ $fails -eq 0 ] && echo "215: ALL OK" || echo "215: FAILURES PRESENT"
exit $fails
