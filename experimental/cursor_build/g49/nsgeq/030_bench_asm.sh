#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
E=$BR/libs/container/experimental
O=/tmp/g49_bench_$$
mkdir -p "$O"
cd "$E"

clang++-22 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=20 \
   -falign-functions=64 -falign-loops=64 -g bench_segmented_algos.cpp -o "$O/bench.elf" \
   || { echo BUILDFAIL; exit 1; }

echo "=== bench_equal-related symbols ==="
nm -C "$O/bench.elf" | rg "bench_equal|measure_batch" | sed 's/boost::container::/bc::/g; s/bench_ops:://g' | cut -c1-160

echo
echo "=== disassembly saved ==="
objdump -d --no-show-raw-insn -C -l "$O/bench.elf" > "$O/bench.asm"
wc -l "$O/bench.asm"
cp "$O/bench.asm" /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq/bench_clang.asm
