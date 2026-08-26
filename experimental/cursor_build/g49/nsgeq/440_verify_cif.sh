#!/bin/bash
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$E"

g++-16 -std=c++20 -O2 -I../../.. -I. segmented_copy_if_test.cpp -o /tmp/cif_test.elf \
   && /tmp/cif_test.elf && echo PASS_test || { echo FAIL_test; exit 1; }

g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 \
   bench_segmented_algos.cpp -o /tmp/g25b.elf || { echo BUILDFAIL; exit 1; }

echo "########## copy_if rows (3 runs) ##########"
for i in 1 2 3; do
   taskset -c 3 /tmp/g25b.elf 2>/dev/null | grep -E "copy_if\("
   echo ---
done

# Confirm no per-block dest cmp in out-of-line walker miss path
objdump -d --no-show-raw-insn -C /tmp/g25b.elf > /tmp/g25b.asm
echo "########## miss dst_dispatch hot cmp census ##########"
awk '/segmented_copy_if_dst_dispatch.*is_negative.*false>/{p=1} p&&/^$/{exit} p' /tmp/g25b.asm \
   | grep -E 'cmp +\$0x7c|sub +\$0x20|test |jns |cmp ' | head -20
nm -C /tmp/g25b.elf | grep -c copy_if_cleanup_blocks || echo "cleanup_blocks symbols: 0"
