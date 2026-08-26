#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
P=$BR/libs/container/experimental/cursor_build/g21/stop/156_flagb_probe.cpp
OUT=/tmp/g21b
mkdir -p $OUT

SYMS="a_base_int a_flaga_int a_flagb_int b_base_int b_flaga_int b_flagb_int a_base_fat a_flaga_fat a_flagb_fat b_base_fat b_flaga_fat b_flagb_fat"

dump() {
   awk -v sym="$2(" '
      index($0, sym) && /^[0-9a-f]+ </ {inf=1; next}
      inf && /^$/ {exit}
      inf {print}' "$1"
}

for CXX in g++-16 clang++-22; do
   echo "==================== $CXX ===================="
   BIN=$OUT/$CXX.elf
   $CXX -std=c++20 -O3 -DNDEBUG -I$BR -Wall -Wextra "$P" -o "$BIN" 2>$OUT/$CXX.log
   if [ $? -ne 0 ]; then head -30 $OUT/$CXX.log; continue; fi
   echo "warnings=$(grep -c 'warning:' $OUT/$CXX.log)"
   "$BIN"
   objdump -d --no-show-raw-insn -C "$BIN" > $OUT/$CXX.asm

   echo
   printf "%-14s %8s %8s\n" symbol insns bytes
   for s in $SYMS; do
      n=$(dump $OUT/$CXX.asm $s | grep -c ':')
      b=$(nm --print-size -C "$BIN" | grep " $s(" | awk '{printf "%d", strtonum("0x" $2)}')
      printf "%-14s %8s %8s\n" "$s" "$n" "${b:-?}"
   done
   echo
done

echo "########## gcc walker B, flagb transition ##########"
dump $OUT/g++-16.asm b_flagb_int | sed 's/<[^>]*>//g'
