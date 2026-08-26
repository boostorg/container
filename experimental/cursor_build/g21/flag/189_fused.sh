#!/bin/bash
set -u
O=/tmp/g21flag
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
mkdir -p $O
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -c $S/188_fused.cpp -o $O/fu.$CXX.o 2>$O/fu.$CXX.log \
      || { echo "$CXX BUILD FAIL"; grep -m5 'error:' $O/fu.$CXX.log; continue; }
   objdump -d --no-show-raw-insn $O/fu.$CXX.o > $O/fu.$CXX.asm
done

echo "===================== symbol inventory ====================="
for CXX in g++-16 clang++-22; do
   echo "-- $CXX --"
   nm -SC $O/fu.$CXX.o | grep -E '^[0-9a-f]+ [0-9a-f]+ [TtWw] ' | while read -r a sz t rest; do
      short=$(echo "$rest" | sed -E 's/.*detail_algo:://; s/[(<].*//')
      printf "   %-46s %s bytes\n" "$short" "$((16#$sz))"
   done | sort -u
done

dump() {
   local ln
   ln=$(grep -nE "^[0-9a-f]+ <.*$2.*>:\$" $O/fu.$1.asm | head -1 | cut -d: -f1)
   [ -z "$ln" ] && { echo "   (not found: $2)"; return; }
   awk -v start="$ln" 'NR>start { if ($0 ~ /^$/) exit; print }' $O/fu.$1.asm \
     | sed -E 's/<[^>]*\+/<+/; s/<[^>]*>/<sym>/'
}

echo
echo "===================== gcc partition_copy false walker ====================="
dump g++-16 'partition_copy_false_dispatch'
echo
echo "===================== gcc copy_n dst walker ====================="
dump g++-16 'copy_n_dst_bounded'
