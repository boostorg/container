#!/bin/bash
# Question 1: does replacing `first == last || r.fourth` by a single flag test
# in segmented_partition_copy.hpp change the emitted branch structure?
set -u
BR=/mnt/d/Data/LocalGit/boost
G=/tmp/g23
SRC=$BR/libs/container/experimental/cursor_build/g23/flag
EX=$BR/libs/container/experimental
O=/tmp/g23pc
SH=$O/shadow
rm -rf $O; mkdir -p $O

python3 $G/211_mkshadow_pc.py $SH/boost/container/experimental/segmented_partition_copy.hpp || exit 1

FILT="partition_copy_false_bounded partition_copy_false_dispatch partition_copy_true"

for CC in g++-16 clang++-22; do
   for D in 1 2; do
      for V in base flag; do
         if [ $V = base ]; then INC="-I$BR"; else INC="-I$SH -I$BR"; fi
         $CC -std=c++20 -O2 -DNDEBUG -DDEPTH=$D $INC -I$EX \
             -c $SRC/210_pc_probe.cpp -o $O/$CC.$D.$V.o 2>$O/$CC.$D.$V.err
         if [ $? -ne 0 ]; then echo "=== $CC depth$D $V : COMPILE FAILED"; \
            head -20 $O/$CC.$D.$V.err; continue; fi
         objdump -d --no-show-raw-insn $O/$CC.$D.$V.o > $O/$CC.$D.$V.txt
         echo "=== $CC  depth=$D  $V"
         python3 $G/212_count.py $O/$CC.$D.$V.txt $FILT
      done
   done
done

echo
echo "=== whole-object totals (all symbols) ==="
for CC in g++-16 clang++-22; do
   for D in 1 2; do
      for V in base flag; do
         [ -f $O/$CC.$D.$V.txt ] || continue
         printf "%-12s depth=%s %-5s " $CC $D $V
         python3 $G/212_count.py $O/$CC.$D.$V.txt | tail -1
      done
   done
done
