#!/bin/bash
set -u
O=/tmp/g21flag
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
mkdir -p $O
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -c $S/182_walk_cg.cpp -o $O/w.$CXX.o 2>/dev/null
   objdump -d --no-show-raw-insn $O/w.$CXX.o > $O/m.$CXX.asm   # keep mangled: short
done

dump() {  # $1=compiler  $2=mangled substring  $3=tail lines
   local ln
   ln=$(grep -nE "^[0-9a-f]+ <.*$2.*>:\$" $O/m.$1.asm | head -1 | cut -d: -f1)
   [ -z "$ln" ] && { echo "   (not found: $2)"; return; }
   awk -v start="$ln" 'NR>start { if ($0 ~ /^$/) exit; print }' $O/m.$1.asm \
     | sed -E 's/<[^>]*\+/<+/; s/<[^>]*>/<sym>/' | tail -n "$3"
}

echo "=== gcc set_union_until_exhausts : TAIL (generic leaf exit + walker decision) ==="
dump g++-16 set_union_until_exhausts 62
