#!/bin/bash
set -u
O=/tmp/g21flag
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
mkdir -p $O
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -c $S/182_walk_cg.cpp -o $O/w.$CXX.o 2>/dev/null
   objdump -dC --no-show-raw-insn $O/w.$CXX.o > $O/w.$CXX.asm
done

dump() {  # $1=compiler $2=symbol regex
   local ln
   ln=$(grep -nE "^[0-9a-f]+ <.*$2.*>:\$" $O/w.$1.asm | head -1 | cut -d: -f1)
   [ -z "$ln" ] && { echo "   (not found: $2)"; return; }
   awk -v start="$ln" 'NR>start { if ($0 ~ /^$/) exit; print }' $O/w.$1.asm \
      | sed -E 's/ <boost::container::detail_algo::([a-z_0-9]+)[^>]*>/ ->\1/; s/\[clone[^]]*\]//' \
      | sed -E 's/<boost::container::[^ ]*//'
}

echo "############ gcc set_union_until_exhausts ############"
dump g++-16 set_union_until_exhausts
echo
echo "############ clang set_union_until_exhausts ############"
dump clang++-22 set_union_until_exhausts
