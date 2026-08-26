#!/bin/bash
set -e
OUT=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g30/nsgpart
g++-16 -std=c++20 -O2 "$OUT/355_alignbench.cpp" -o /tmp/ab.elf
# verify emitted loop bytes for one pad
objdump -d --no-show-raw-insn /tmp/ab.elf | awk '/<scan_48/{p=1} p{print} p&&/ret/{exit}' | head -30
echo "== runs =="
nice -n -5 taskset -c 3 /tmp/ab.elf | tee "$OUT/355_result.txt"
