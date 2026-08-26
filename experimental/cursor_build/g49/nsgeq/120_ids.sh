#!/bin/bash
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq
grep -n "^0[0-9a-f]* <unsigned long measure_batch" bench_gcc.asm | while IFS= read -r l; do
   line=${l%%:*}
   name=${l#*:}
   case "$name" in
      *mismatch_2r*|*equal_2r*) ;;
      *) continue ;;
   esac
   algo=$(echo "$name" | grep -oE "(std|seg)_(mismatch|equal)_2r")
   # count container kinds in order
   shape=$(echo "$name" | grep -oE "deque<MyInt|vector<MyInt" | head -2 | paste -sd+ | sed 's/deque<MyInt/D/g; s/vector<MyInt/V/g')
   wrap=$(echo "$name" | grep -cE ", true>, noop_reset>\(unsigned long")
   echo "$line  $algo  $shape  wrapped=$wrap"
done
