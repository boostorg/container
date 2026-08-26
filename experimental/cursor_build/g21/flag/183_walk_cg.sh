#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/flag
O=/tmp/g21flag
mkdir -p $O
FNS="cpy cpyif xform mrg suni sdif sint ssym pcpy"

for CXX in g++-16 clang++-22; do
   echo "######################## $CXX ########################"
   /usr/bin/time -f "  build: %es  %MkB" $CXX -std=c++20 -O2 -DNDEBUG -I$BR \
      -c $S/182_walk_cg.cpp -o $O/w.$CXX.o 2>$O/w$CXX.log
   grep -E '^  build:' $O/w$CXX.log
   grep -qE 'error:' $O/w$CXX.log && { echo BUILD FAIL; grep -m5 'error:' $O/w$CXX.log; continue; }
   objdump -dC --no-show-raw-insn $O/w.$CXX.o > $O/w.$CXX.asm
   printf "  %-8s %8s %8s %8s %8s\n" fn size insns cmp jcc
   for fn in $FNS; do
      sz=$(nm -SC $O/w.$CXX.o | grep -E "^[0-9a-f]+ [0-9a-f]+ [Tt] $fn\(" | awk '{print strtonum("0x"$2)}')
      awk -v f="$fn(" 'index($0,"<"f)>0 && /:$/ {inb=1; next} inb && /^$/ {inb=0} inb {print}' \
         $O/w.$CXX.asm > $O/$fn.$CXX.body
      tot=$(grep -cE '^\s+[0-9a-f]+:' $O/$fn.$CXX.body)
      c=$(grep -cE '^\s+[0-9a-f]+:\s+(cmp|test)' $O/$fn.$CXX.body)
      j=$(grep -cE '^\s+[0-9a-f]+:\s+j' $O/$fn.$CXX.body)
      printf "  %-8s %8s %8s %8s %8s\n" "$fn" "${sz:-oOL}" "$tot" "$c" "$j"
   done
done

echo
echo "############ gcc: set_union inner region (dst transition) ############"
# the dst walker transition is where  first1==last1 || first2==last2  would sit:
# show the densest cmp cluster
grep -nE '^\s+[0-9a-f]+:\s+(cmp|test|j)' $O/suni.g++-16.body | head -70
