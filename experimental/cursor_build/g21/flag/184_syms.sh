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

   # every out-of-line detail_algo symbol, with size / insn / cmp counts
   nm -SC $O/w.$CXX.o | grep -E '^[0-9a-f]+ [0-9a-f]+ [TtWw] ' | while read -r a sz t rest; do
      short=$(echo "$rest" | sed -E 's/\(.*//; s/.*:://')
      case "$short" in
         *until_exhausts|*_dst_bounded|*_seg2_dispatch|*_scan|*_dst_dispatch|*_bounded|*_dispatch|*_leaf)
            body=$(awk -v pat="$rest" 'index($0,"<"pat">")>0 && /:$/ {inb=1; next} inb && /^$/ {inb=0} inb {print}' $O/w.$CXX.asm)
            tot=$(printf '%s\n' "$body" | grep -cE '^\s+[0-9a-f]+:')
            c=$(printf '%s\n' "$body" | grep -cE '^\s+[0-9a-f]+:\s+(cmp|test)')
            printf "  %-52s size=%-6s insns=%-5s cmp=%s\n" "$short" "$((16#$sz))" "$tot" "$c"
            ;;
      esac
   done | sort -u
done
