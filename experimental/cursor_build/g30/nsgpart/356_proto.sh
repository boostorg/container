#!/bin/bash
# 356: build bench + partition test with shadow segmented_partition.hpp
# (libstdc++-shaped non-segmented bidirectional fallback), run pinned 5x,
# extract loop layout stats.
set -e
EXP=/mnt/d/Data/LocalGit/boost/libs/container/experimental
OUT=$EXP/cursor_build/g30/nsgpart
SH=$OUT/shadow
cd "$EXP"

FLAGS="-std=c++20 -O3 -I$SH -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=17 -falign-functions=64"

echo "== compiling =="
g++-16     $FLAGS bench_segmented_algos.cpp -o /tmp/g356_gcc.elf   2> "$OUT/356_gcc_compile.log" &
P1=$!
clang++-22 $FLAGS bench_segmented_algos.cpp -o /tmp/g356_clang.elf 2> "$OUT/356_clang_compile.log" &
P2=$!
g++-16     -std=c++20 -O2 -I"$SH" -I../../.. segmented_partition_test.cpp -o /tmp/g356_test_gcc.elf   2> "$OUT/356_test_gcc_compile.log" &
P3=$!
clang++-22 -std=c++20 -O2 -I"$SH" -I../../.. segmented_partition_test.cpp -o /tmp/g356_test_clang.elf 2> "$OUT/356_test_clang_compile.log" &
P4=$!
wait $P1; wait $P2; wait $P3; wait $P4
echo "== compile done =="

echo "== partition test gcc =="
/tmp/g356_test_gcc.elf   && echo "TEST GCC PASS"   || echo "TEST GCC FAIL"
echo "== partition test clang =="
/tmp/g356_test_clang.elf && echo "TEST CLANG PASS" || echo "TEST CLANG FAIL"

cp /tmp/g356_gcc.elf "$OUT/356_gcc.elf"

for i in 1 2 3 4 5; do
   echo "== pinned gcc run $i =="
   nice -n -5 taskset -c 3 /tmp/g356_gcc.elf   > "$OUT/356_gcc_pin$i.txt" 2>&1 || taskset -c 3 /tmp/g356_gcc.elf > "$OUT/356_gcc_pin$i.txt" 2>&1
done
for i in 1 2 3 4 5; do
   echo "== pinned clang run $i =="
   nice -n -5 taskset -c 3 /tmp/g356_clang.elf > "$OUT/356_clang_pin$i.txt" 2>&1 || taskset -c 3 /tmp/g356_clang.elf > "$OUT/356_clang_pin$i.txt" 2>&1
done

for cc in gcc clang; do
   objdump -d --no-show-raw-insn "/tmp/g356_$cc.elf" | c++filt > "/tmp/g356_$cc.dis"
   awk '/^[0-9a-f]+ <.*measure_batch.*partition.*>:$/{p=1} p{print} p&&/^$/{p=0}' \
      "/tmp/g356_$cc.dis" > "$OUT/356_${cc}_partition_funcs.dis"
done

echo "== partition rows pin1 =="
grep -H "partition" "$OUT"/356_gcc_pin1.txt "$OUT"/356_clang_pin1.txt
echo "== done =="
