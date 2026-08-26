#!/bin/bash
set -u
O=/tmp/g21flag
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
mkdir -p $O
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -c $S/188_fused.cpp -o $O/fu.$CXX.o 2>/dev/null
   objdump -d --no-show-raw-insn $O/fu.$CXX.o > $O/fu.$CXX.asm
done
dump() {
   local ln
   ln=$(grep -nE "^[0-9a-f]+ <.*$2.*>:\$" $O/fu.$1.asm | head -1 | cut -d: -f1)
   [ -z "$ln" ] && { echo "   (not found: $2)"; return; }
   awk -v start="$ln" 'NR>start { if ($0 ~ /^$/) exit; print }' $O/fu.$1.asm \
     | sed -E 's/<[^>]*\+/<+/; s/<[^>]*>/<sym>/'
}

echo "########## clang partition_copy_false_dispatch (has  first==last || r.fourth ) ##########"
dump clang++-22 partition_copy_false_dispatch

echo
echo "########## gcc cpn (copy_n, walker inlined) ##########"
dump g++-16 '^cpn'
