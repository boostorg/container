#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
CB=$BR/libs/container/bench
H=$BR/libs/container/experimental/cursor_build/g49/nsgeq
O=/tmp/g49_asm_$$
mkdir -p "$O"

clang++-22 -std=c++20 -O3 -DNDEBUG -I"$BR" -I"$CB" -falign-functions=64 -falign-loops=64 \
   -c "$H/020_asm.cpp" -o "$O/a.o" || { echo BUILDFAIL; exit 1; }

objdump -d --no-show-raw-insn -C "$O/a.o" > "$O/a.asm"

for fn in eq1s_std eq1s_nsg eq2s_std eq2s_nsg mm1s_std mm1s_nsg; do
   echo "=================== $fn ==================="
   awk -v f="<${fn}(" 'index($0,f) && /^0/ {p=1} p {print} p && /^$/ {exit}' "$O/a.asm" \
      | sed -n '1,80p'
   n=$(awk -v f="<${fn}(" 'index($0,f) && /^0/ {p=1; next} p && /^$/ {exit} p {c++} END {print c+0}' "$O/a.asm")
   echo "--- total instructions: $n"
done
