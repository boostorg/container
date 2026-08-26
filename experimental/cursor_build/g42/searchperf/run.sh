#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
CB=$BR/libs/container/bench
HERE=$BR/libs/container/experimental/cursor_build/g42/searchperf
O=/tmp/g42
mkdir -p $O

for cc in g++-16 clang++-22; do
   echo "================ $cc ================"
   if ! $cc -std=c++17 -O2 -DNDEBUG -I"$BR" -I"$CB" \
        "$HERE/probe.cpp" -o "$O/p.$cc.elf" 2>"$O/$cc.log"; then
      echo BUILD FAIL; sed -n '1,25p' "$O/$cc.log"; continue
   fi
   [ -s "$O/$cc.log" ] && { echo "--- diagnostics ---"; sed -n '1,15p' "$O/$cc.log"; }
   taskset -c 3 nice -n -5 "$O/p.$cc.elf" 2>/dev/null || "$O/p.$cc.elf"
done
