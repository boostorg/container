#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
O=/tmp/g21flag
mkdir -p $O

for CXX in g++-16 clang++-22; do
   echo "######################## $CXX ########################"
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -c $S/180_find_cg.cpp -o $O/f.$CXX.o 2>$O/$CXX.log \
      || { echo BUILD FAIL; head -20 $O/$CXX.log; continue; }
   objdump -dC --no-show-raw-insn $O/f.$CXX.o > $O/f.$CXX.asm

   for fn in fnd fndif ppoint sortuntil; do
      # symbol size
      sz=$(nm -SC --size-sort $O/f.$CXX.o | grep -E "^[0-9a-f]+ [0-9a-f]+ [Tt] $fn\(" | awk '{print strtonum("0x"$2)}')
      # body of that function only
      awk -v f="$fn(" '
         index($0, "<"f)>0 && /:$/ {inb=1; next}
         inb && /^$/ {inb=0}
         inb {print}
      ' $O/f.$CXX.asm > $O/$fn.$CXX.body

      tot=$(grep -cE '^\s+[0-9a-f]+:' $O/$fn.$CXX.body)
      cmps=$(grep -cE '^\s+[0-9a-f]+:\s+(cmp|test)' $O/$fn.$CXX.body)
      jmps=$(grep -cE '^\s+[0-9a-f]+:\s+j' $O/$fn.$CXX.body)
      printf "  %-11s size=%-5s insns=%-5s cmp/test=%-4s jcc=%s\n" "$fn" "${sz:-?}" "$tot" "$cmps" "$jmps"
   done
done

echo
echo "######## gcc: fnd full body (walker + inlined leaf) ########"
cat $O/fnd.g++-16.body
