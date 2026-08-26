#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
O=/tmp/g21deep
mkdir -p $O

for CXX in g++-16 clang++-22; do
   echo "######################## $CXX ########################"
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -I$BR/libs/container/experimental \
        -c $S/195_deep_cg.cpp -o $O/d.$CXX.o 2>$O/d.$CXX.log \
      || { echo BUILD FAIL; grep -m6 'error:' $O/d.$CXX.log; continue; }
   objdump -d --no-show-raw-insn $O/d.$CXX.o > $O/d.$CXX.asm

   # every out-of-line walker, with size / insn / cmp counts
   grep -nE '^[0-9a-f]+ <.*>:$' $O/d.$CXX.asm | while IFS=: read -r ln rest; do
      mang=${rest#*<}; mang=${mang%>:}
      short=$(echo "$mang" | c++filt 2>/dev/null | sed -E 's/.*detail_algo:://; s/[(<].*//')
      [ -z "$short" ] && short="$mang"
      body=$(awk -v s="$ln" 'NR>s { if ($0 ~ /^$/) exit; print }' $O/d.$CXX.asm)
      tot=$(printf '%s\n' "$body" | grep -cE '^\s+[0-9a-f]+:')
      c=$(printf '%s\n' "$body" | grep -cE '^\s+[0-9a-f]+:\s+(cmp|test)')
      printf "  %-40s insns=%-5s cmp=%s\n" "$short" "$tot" "$c"
   done | sort -u
done
