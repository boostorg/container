#!/bin/bash
set -u
G34=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g34/unroll
cd $G34
echo "=== build probe ==="
g++-16 -std=c++20 -O2 -DNDEBUG -falign-functions=64 398_branchprobe.cpp -o 398.elf 2>&1 | head -20 || exit 1
echo "=== verify replayed loop A matches library shape ==="
objdump -d --no-show-raw-insn 398.elf | grep -A 22 "p2align\|<_Z5loopAILi0" | head -5
objdump -d --no-show-raw-insn 398.elf | sed -n '/_Z5loopAILi0EElPPiS0_S0_S0_/,/ret/p' | head -30
echo
echo "=== run pinned ==="
taskset -c 3 ./398.elf 3000
echo DONE
