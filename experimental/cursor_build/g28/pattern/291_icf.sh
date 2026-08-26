#!/bin/bash
# ICF-only rerun (section 2 of 288_extra.sh), plus a variant comparison:
# does --icf=all reclaim F-level size from the E shape?
set -u
BR=/mnt/d/Data/LocalGit/boost
G=$BR/libs/container/experimental/cursor_build/g28/pattern
EX=$BR/libs/container/experimental
O=/tmp/g28icf
mkdir -p $O

for CC in g++-16 clang++-22; do
   for V in base H F; do
      if [ $V = base ]; then INC="-I$BR"; else INC="-I$G/shadow_$V -I$BR"; fi
      $CC -std=c++20 -O2 -DNDEBUG -fPIC -ffunction-sections $INC -I$EX \
         -c $G/280_probe.cpp -o $O/icf.$CC.$V.o 2>/dev/null || { echo "$CC $V compile FAILED"; continue; }
      for ICF in none all; do
         if clang++-22 -shared -fuse-ld=lld -Wl,--icf=$ICF -Wl,--unresolved-symbols=ignore-all \
              $O/icf.$CC.$V.o -o $O/icf.so 2>$O/e.txt; then
            sz=$(size -A $O/icf.so | awk '$1 ~ /^\.text/{s+=$2} END{print s}')
            echo "  $CC $V --icf=$ICF: .text*=$sz"
         else
            echo "  $CC $V --icf=$ICF: LINK FAILED"; head -3 $O/e.txt | sed 's/^/    /'
         fi
      done
   done
done
