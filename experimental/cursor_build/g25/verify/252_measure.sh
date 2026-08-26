#!/bin/bash
# Depth-1 no-regression check: per-symbol instruction / cmp+test / jcc counts of
# the four set_* destination walkers, working tree vs HEAD, for both compilers.
#
# HEAD is overlaid by putting a directory holding only the HEAD copies of the
# four headers ahead of the boost root in the -I list, so everything else
# (segquartet, the SEG_LIKELY macros, the traits) still comes from the tree.
#
# Depth 2 is reported as well: it is the depth the flag exists for, and its
# numbers are the ones expected to move.
set -u
BR=/mnt/d/Data/LocalGit/boost
G23=$BR/libs/container/experimental/cursor_build/g23/flag
G25=$BR/libs/container/experimental/cursor_build/g25/verify
EX=$BR/libs/container/experimental
O=/tmp/g25meas
rm -rf $O; mkdir -p $O

ALGOS=(x set_union set_difference set_intersection set_symmetric_difference)
FILT="until_exhausts dst_bounded run_"

for D in 1 2; do
   echo "############################ DEPTH $D ############################"
   for A in 1 2 3 4; do
      echo
      echo "==== ${ALGOS[$A]} ===="
      for CC in g++-16 clang++-22; do
         for V in head tree; do
            if [ $V = head ]; then INC="-I$G25/head -I$BR"; else INC="-I$BR"; fi
            if ! $CC -std=c++20 -O2 -DNDEBUG -DALGO=$A -DDEPTH=$D $INC -I$EX \
                 -c $G23/221_sets_probe.cpp -o $O/o.o 2>$O/e.log; then
               echo "  $CC $V: COMPILE FAILED"; grep -m3 'error:' $O/e.log | sed 's/^/    /'
               continue
            fi
            objdump -d --no-show-raw-insn $O/o.o > $O/l.txt
            echo "  --- $CC  $V"
            python3 $G23/212_count.py $O/l.txt $FILT | sed 's/^/  /'
         done
      done
   done
done
