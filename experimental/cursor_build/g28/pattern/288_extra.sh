#!/bin/bash
# Supplementary static experiments:
#  1. -O3 spot check of the per-variant probe codegen.
#  2. ICF experiment: does --icf=all fold today's (E) per-algorithm walkers?
#  3. Compile time / peak memory per variant (g++ and clang++).
set -u
BR=/mnt/d/Data/LocalGit/boost
G=$BR/libs/container/experimental/cursor_build/g28/pattern
EX=$BR/libs/container/experimental
O=/tmp/g28extra
OUT=$G/out
mkdir -p $O $OUT
PYC=/tmp/281c.py
tr -d '\r' < $G/281_count.py > $PYC

echo "############ 0. corrected .text* totals, -O2, all variants ############"
for CC in g++-16 clang++-22; do
   for V in base T F H W; do
      if [ $V = base ]; then INC="-I$BR"; EXTRA=""; else INC="-I$G/shadow_$V -I$BR"; EXTRA="-DBOOST_CONTAINER_G28_EXPECT_SHADOW"; fi
      obj=$O/probe2.$CC.$V.o
      $CC -std=c++20 -O2 -DNDEBUG $EXTRA $INC -I$EX -c $G/280_probe.cpp -o $obj 2>/dev/null || { echo "$CC $V FAILED"; continue; }
      sz=$(size -A $obj | awk '$1 ~ /^\.text/{s+=$2} END{print s}')
      echo "  $CC $V .text*=$sz"
   done
done

echo
echo "############ 1. -O3 spot check (totals only) ############"
for CC in g++-16 clang++-22; do
   for V in base T F H W; do
      if [ $V = base ]; then INC="-I$BR"; EXTRA=""; else INC="-I$G/shadow_$V -I$BR"; EXTRA="-DBOOST_CONTAINER_G28_EXPECT_SHADOW"; fi
      obj=$O/probe3.$CC.$V.o
      $CC -std=c++20 -O3 -DNDEBUG $EXTRA $INC -I$EX -c $G/280_probe.cpp -o $obj 2>/dev/null || { echo "$CC $V -O3 FAILED"; continue; }
      objdump -d --no-show-raw-insn $obj > $O/l.txt
      printf "%-12s %-5s " $CC $V
      python3 $PYC $O/l.txt probe_ detail_algo | tail -1
   done
done

echo
echo "############ 2. ICF experiment (E shape, lld --icf=all) ############"
for CC in g++-16 clang++-22; do
   $CC -std=c++20 -O2 -DNDEBUG -fPIC -ffunction-sections -I$BR -I$EX -c $G/280_probe.cpp -o $O/icf.$CC.o 2>/dev/null
   for ICF in none all; do
      if clang++-22 -shared -fuse-ld=lld -Wl,--icf=$ICF -Wl,--unresolved-symbols=ignore-all \
           $O/icf.$CC.o -o $O/icf.$CC.$ICF.so 2>$O/icf.err; then
         sz=$(size -A $O/icf.$CC.$ICF.so | awk '$1==".text"{print $2}')
         echo "  $CC objects, --icf=$ICF: .text=$sz"
      else
         echo "  $CC objects, --icf=$ICF: LINK FAILED"; head -3 $O/icf.err | sed 's/^/    /'
      fi
   done
done

echo
echo "############ 3. compile time / peak RSS (c++20 -O2, probe TU) ############"
for CC in g++-16 clang++-22; do
   for V in base T F H W; do
      if [ $V = base ]; then INC="-I$BR"; else INC="-I$G/shadow_$V -I$BR"; fi
      /usr/bin/time -f "  $CC $V: %es wall, %M KB maxRSS" \
         $CC -std=c++20 -O2 -DNDEBUG $INC -I$EX -c $G/280_probe.cpp -o $O/ct.o 2>&1 | tail -1
   done
done

echo
echo "############ 4. compile time / peak RSS on the heaviest affected test (copy) ############"
for CC in g++-16 clang++-22; do
   for V in base F H; do
      if [ $V = base ]; then INC="-I$BR"; else INC="-I$G/shadow_$V -I$BR"; fi
      /usr/bin/time -f "  $CC $V: %es wall, %M KB maxRSS" \
         $CC -std=c++20 -O2 -DNDEBUG $INC -I$EX -c $EX/segmented_copy_test.cpp -o $O/ct2.o 2>&1 | tail -1
   done
done
