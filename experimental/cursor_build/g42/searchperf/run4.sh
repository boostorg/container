#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
H=$BR/libs/container/experimental/cursor_build/g42/searchperf
for cc in g++-16 clang++-22; do
   echo "================ $cc ================"
   $cc -std=c++17 -O2 -DNDEBUG -I"$BR" -c "$H/asm.cpp" -o "/tmp/a.$cc.o" 2>/dev/null || { echo FAIL; continue; }
   echo "--- symbols (out-of-line means a call per candidate) ---"
   nm -C --size-sort "/tmp/a.$cc.o" | sed -n '1,20p'
   echo "--- srch body ---"
   objdump -d --no-show-raw-insn -C "/tmp/a.$cc.o" \
     | awk '/<srch\(/{f=1} f{print} f&&/^$/{exit}' | sed -n '1,90p'
done
