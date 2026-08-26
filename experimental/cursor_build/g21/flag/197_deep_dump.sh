#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
O=/tmp/g21deep
mkdir -p $O
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -I$BR/libs/container/experimental \
        -c $S/195_deep_cg.cpp -o $O/d.$CXX.o 2>/dev/null
   objdump -d --no-show-raw-insn $O/d.$CXX.o > $O/d.$CXX.asm
done

echo "############ clang: merge_dst_bounded (recursive segmented dst walker) ############"
ln=$(grep -nE '^[0-9a-f]+ <.*merge_dst_bounded.*>:$' $O/d.clang++-22.asm | head -1 | cut -d: -f1)
awk -v s="$ln" 'NR>s { if ($0 ~ /^$/) exit; print }' $O/d.clang++-22.asm \
  | sed -E 's/<[^>]*\+/<+/; s/<[^>]*>/<CALL>/' \
  | grep -nE 'call|cmp|test|^\s+[0-9a-f]+:\s+j|ret'
