#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
H=$BR/libs/container/experimental/cursor_build/g49/nsgeq
O=/tmp/g49sf_$$
mkdir -p "$O"

for CXX in g++-16 clang++-22; do
   echo "########## $CXX ##########"
   $CXX -std=c++20 -O3 -DNDEBUG -I"$BR" -c "$H/420_stopflag.cpp" -o "$O/a.o" || { echo BUILDFAIL; continue; }
   objdump -d --no-show-raw-insn -C "$O/a.o" > "$O/a.asm"
   for fn in leaf_cur leaf_head; do
      echo "--- $fn ---"
      awk -v f="<${fn}(" 'index($0,f) && /^0/ {p=1} p {print} p && /^$/ {exit}' "$O/a.asm" \
         | grep -vE '^$|^/' | head -40
      echo
   done
done
