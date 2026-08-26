#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
O=/tmp/g21flag
mkdir -p $O

for CXX in g++-16 clang++-22; do
   echo "######################## $CXX ########################"
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -c $S/182_walk_cg.cpp -o $O/w.$CXX.o 2>/dev/null
   objdump -dC --no-show-raw-insn $O/w.$CXX.o > $O/w.$CXX.asm

   # objdump function headers:  "0000... <demangled name>:"
   grep -nE '^[0-9a-f]+ <.*>:$' $O/w.$CXX.asm | while IFS=: read -r ln rest; do
      full=${rest#*<}; full=${full%>:}
      # name = text after the last "detail_algo::" up to the first "(" or "<"
      case "$full" in
         *detail_algo::*) nm2=${full##*detail_algo::}; nm2=${nm2%%[(<]*} ;;
         *) continue ;;
      esac
      body=$(awk -v start="$ln" 'NR>start { if ($0 ~ /^$/) exit; print }' $O/w.$CXX.asm)
      tot=$(printf '%s\n' "$body" | grep -cE '^\s+[0-9a-f]+:')
      c=$(printf '%s\n' "$body" | grep -cE '^\s+[0-9a-f]+:\s+(cmp|test)')
      printf "  %-46s insns=%-5s cmp=%s\n" "$nm2" "$tot" "$c"
   done | sort | uniq -c | sort -rn
done

echo
echo "######## gcc: set_union_until_exhausts (dst walker) full body ########"
ln=$(grep -nE '^[0-9a-f]+ <.*set_union_until_exhausts.*>:$' $O/w.g++-16.asm | head -1 | cut -d: -f1)
awk -v start="$ln" 'NR>start { if ($0 ~ /^$/) exit; print }' $O/w.g++-16.asm | sed -E 's/<boost::container::[^>]*>[^>]*>?//g' | head -80
