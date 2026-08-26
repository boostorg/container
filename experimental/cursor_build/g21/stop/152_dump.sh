#!/bin/bash
# Dump the segment-transition tails of walker B and walker A, both compilers.
set -u
OUT=/tmp/g21stop

dump() {  # $1=asm file  $2=symbol
   awk -v sym="$2(" '
      index($0, sym) && /^[0-9a-f]+ </ {inf=1; next}
      inf && /^$/ {exit}
      inf {sub(/<[^>]*>/, "<L>"); print}' "$1"
}

for CXX in g++-16 clang++-22; do
   for s in base_b_int var_b_int; do
      echo "=============== $CXX  $s ==============="
      dump $OUT/$CXX.asm $s
      echo
   done
done

for CXX in g++-16; do
   for s in base_a_int var_a_int; do
      echo "=============== $CXX  $s ==============="
      dump $OUT/$CXX.asm $s
      echo
   done
done
