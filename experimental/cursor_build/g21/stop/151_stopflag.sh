#!/bin/bash
# Build the stop-flag probe, check correctness, compare symbol sizes and
# instruction counts, then time it.
set -u

BOOST_ROOT=/mnt/d/Data/LocalGit/boost
PROBE=$BOOST_ROOT/libs/container/experimental/cursor_build/g21/stop/150_stopflag_probe.cpp
OUT=/tmp/g21stop
mkdir -p $OUT

SYMS="base_a_int var_a_int base_b_int var_b_int base_a_fat var_a_fat base_b_fat var_b_fat"

for CXX in g++-16 clang++-22; do
   echo "==================================================================="
   echo "== $CXX"
   echo "==================================================================="

   BIN=$OUT/$CXX.elf
   $CXX -std=c++20 -O3 -DNDEBUG -I$BOOST_ROOT -Wall -Wextra \
        "$PROBE" -o "$BIN" 2> $OUT/$CXX.log
   rc=$?
   nwarn=$(grep -c 'warning:' $OUT/$CXX.log)
   echo "build rc=$rc warnings=$nwarn"
   if [ $rc -ne 0 ]; then
      head -40 $OUT/$CXX.log
      continue
   fi
   [ "$nwarn" != "0" ] && grep 'warning:' $OUT/$CXX.log | head -10

   echo
   echo "--- symbol sizes (bytes) ---"
   for s in $SYMS; do
      sz=$(nm --print-size --size-sort -C "$BIN" 2>/dev/null | grep " $s(" | awk '{print strtonum("0x" $2)}' | head -1)
      printf "  %-12s %s\n" "$s" "${sz:-?}"
   done

   echo
   echo "--- instruction counts ---"
   objdump -d --no-show-raw-insn -C "$BIN" > $OUT/$CXX.asm 2>/dev/null
   for s in $SYMS; do
      n=$(awk -v sym="$s(" '
         index($0, sym) && /^[0-9a-f]+ </ {inf=1; next}
         inf && /^$/ {inf=0}
         inf && /^  *[0-9a-f]+:/ {c++}
         END {print c+0}' $OUT/$CXX.asm)
      printf "  %-12s %s\n" "$s" "$n"
   done

   echo
   echo "--- run ---"
   taskset -c 4 "$BIN"
done

echo
echo "==================================================================="
echo "== walker B disassembly, gcc (segment transition path)"
echo "==================================================================="
for s in base_b_int var_b_int; do
   echo "----- $s -----"
   awk -v sym="$s(" '
      index($0, sym) && /^[0-9a-f]+ </ {inf=1; print; next}
      inf && /^$/ {exit}
      inf {print}' $OUT/g++-16.asm
done
