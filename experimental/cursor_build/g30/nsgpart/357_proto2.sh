#!/bin/bash
# 357: prototype v2 (manually 2x-unrolled fallback scans) via shadow2 header.
# Also compute medians/layout for 350 (baseline), 356 (v1), 357 (v2).
set -e
EXP=/mnt/d/Data/LocalGit/boost/libs/container/experimental
OUT=$EXP/cursor_build/g30/nsgpart
SH=$OUT/shadow2
cd "$EXP"

FLAGS="-std=c++20 -O3 -I$SH -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=17 -falign-functions=64"

echo "== compiling =="
g++-16     $FLAGS bench_segmented_algos.cpp -o /tmp/g357_gcc.elf   2> "$OUT/357_gcc_compile.log" &
P1=$!
clang++-22 $FLAGS bench_segmented_algos.cpp -o /tmp/g357_clang.elf 2> "$OUT/357_clang_compile.log" &
P2=$!
g++-16     -std=c++20 -O2 -I"$SH" -I../../.. segmented_partition_test.cpp -o /tmp/g357_test_gcc.elf   2> "$OUT/357_test_gcc_compile.log" &
P3=$!
clang++-22 -std=c++20 -O2 -I"$SH" -I../../.. segmented_partition_test.cpp -o /tmp/g357_test_clang.elf 2> "$OUT/357_test_clang_compile.log" &
P4=$!
wait $P1; wait $P2; wait $P3; wait $P4
echo "== compile done =="

echo "== partition test gcc ==";   /tmp/g357_test_gcc.elf   && echo "TEST GCC PASS"   || echo "TEST GCC FAIL"
echo "== partition test clang =="; /tmp/g357_test_clang.elf && echo "TEST CLANG PASS" || echo "TEST CLANG FAIL"

cp /tmp/g357_gcc.elf "$OUT/357_gcc.elf"

for i in 1 2 3 4 5; do
   nice -n -5 taskset -c 3 /tmp/g357_gcc.elf   > "$OUT/357_gcc_pin$i.txt"   2>&1 || true
done
for i in 1 2 3 4 5; do
   nice -n -5 taskset -c 3 /tmp/g357_clang.elf > "$OUT/357_clang_pin$i.txt" 2>&1 || true
done

for cc in gcc clang; do
   objdump -d --no-show-raw-insn "/tmp/g357_$cc.elf" | c++filt > "/tmp/g357_$cc.dis"
   awk '/^[0-9a-f]+ <.*measure_batch.*partition.*>:$/{p=1} p{print} p&&/^$/{p=0}' \
      "/tmp/g357_$cc.dis" > "$OUT/357_${cc}_partition_funcs.dis"
done

tr -d '\r' < "$OUT/358_medians.py" > /tmp/med.py
tr -d '\r' < "$OUT/359_layout.py"  > /tmp/lay.py
for p in 350 356 357; do python3 /tmp/med.py $p; done
echo "== layout gcc: baseline / v1 / v2 =="
python3 /tmp/lay.py 350_gcc_partition_funcs.dis
echo "--"
python3 /tmp/lay.py 356_gcc_partition_funcs.dis
echo "--"
python3 /tmp/lay.py 357_gcc_partition_funcs.dis
echo "== done =="
