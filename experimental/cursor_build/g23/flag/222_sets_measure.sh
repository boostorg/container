#!/bin/bash
# Question 2: instruction / compare counts of the segmented-destination walkers
# of the four set_* algorithms, with and without the leaf flag.
set -u
BR=/mnt/d/Data/LocalGit/boost
G=/tmp/g23
SRC=$BR/libs/container/experimental/cursor_build/g23/flag
EX=$BR/libs/container/experimental
O=/tmp/g23sets
rm -rf $O; mkdir -p $O

ALGOS=(x set_union set_difference set_intersection set_symmetric_difference)

VARIANTS="before split post guard"

echo "--- building shadows ---"
for V in $VARIANTS; do
   for A in 1 2 3 4; do
      python3 $G/220_mkshadow_sets.py ${ALGOS[$A]} $V \
         $O/$V/boost/container/experimental/segmented_${ALGOS[$A]}.hpp || exit 1
   done
done

FILT="until_exhausts dst_bounded"

for A in 1 2 3 4; do
   echo
   echo "############ ${ALGOS[$A]} ############"
   for CC in g++-16 clang++-22; do
      for D in 1 2; do
         for V in $VARIANTS; do
            $CC -std=c++20 -O2 -DNDEBUG -DALGO=$A -DDEPTH=$D -I$O/$V -I$BR -I$EX \
                -c $SRC/221_sets_probe.cpp -o $O/o.o 2>$O/e.log
            if [ $? -ne 0 ]; then echo "=== $CC depth=$D $V : COMPILE FAILED"; \
               grep -m3 'error:' $O/e.log | sed 's/^/    /'; continue; fi
            objdump -d --no-show-raw-insn $O/o.o > $O/l.txt
            echo "=== $CC  depth=$D  $V"
            python3 $G/212_count.py $O/l.txt $FILT
            printf "   %-42s " "(whole object)"
            python3 $G/212_count.py $O/l.txt | tail -1 | sed 's/^ *TOTAL *//'
         done
      done
   done
done
