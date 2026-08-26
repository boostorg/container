#!/bin/bash
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
H=$E/cursor_build/g49/nsgeq
cd "$E"
g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   -g bench_segmented_algos.cpp -o /tmp/g49g.elf || { echo BUILDFAIL; exit 1; }

echo "=== reproduce (equal_2r + mismatch_2r rows) ==="
taskset -c 3 /tmp/g49g.elf 2>/dev/null | grep -E "equal_2r|mismatch_2r"

objdump -d --no-show-raw-insn -C -l /tmp/g49g.elf > "$H/bench_gcc.asm"
wc -l "$H/bench_gcc.asm"
grep -n "^0[0-9a-f]* <unsigned long measure_batch" "$H/bench_gcc.asm" | grep -E "_2r" | cut -c1-150
