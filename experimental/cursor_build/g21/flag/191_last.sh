#!/bin/bash
set -u
O=/tmp/g21flag
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
mkdir -p $O
g++-16 -std=c++20 -O2 -DNDEBUG -I$BR -c $S/188_fused.cpp -o $O/fu.o 2>/dev/null
objdump -d --no-show-raw-insn $O/fu.o > $O/fu.asm

echo "########## gcc cpn : segmented_copy_n, walker inlined ##########"
ln=$(grep -nE '^[0-9a-f]+ <cpn>:$' $O/fu.asm | cut -d: -f1)
awk -v s="$ln" 'NR>s { if ($0 ~ /^$/) exit; print }' $O/fu.asm \
  | sed -E 's/<[^>]*\+/<+/; s/<[^>]*>/<sym>/' | tail -40
