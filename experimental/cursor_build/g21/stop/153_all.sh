#!/bin/bash
# One-shot: build, then dump the segment-transition code of both walkers.
set -u

BOOST_ROOT=/mnt/d/Data/LocalGit/boost
PROBE=$BOOST_ROOT/libs/container/experimental/cursor_build/g21/stop/150_stopflag_probe.cpp
OUT=/tmp/g21stop
mkdir -p $OUT

dump() {  # $1=asm  $2=symbol
   awk -v sym="$2(" '
      index($0, sym) && /^[0-9a-f]+ </ {inf=1; next}
      inf && /^$/ {exit}
      inf {sub(/<[^>]*\+/, "<+"); sub(/<[a-zA-Z_][^>]*>/, "<L>"); print}' "$1"
}

for CXX in g++-16 clang++-22; do
   BIN=$OUT/$CXX.elf
   $CXX -std=c++20 -O3 -DNDEBUG -I$BOOST_ROOT "$PROBE" -o "$BIN" 2>/dev/null || { echo "$CXX build failed"; continue; }
   objdump -d --no-show-raw-insn -C "$BIN" > $OUT/$CXX.asm

   for s in base_b_int var_b_int base_a_int var_a_int; do
      n=$(dump $OUT/$CXX.asm $s | grep -c ':')
      echo "############## $CXX  $s   ($n insns) ##############"
      dump $OUT/$CXX.asm $s
      echo
   done
done
