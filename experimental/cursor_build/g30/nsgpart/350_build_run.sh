#!/bin/bash
# 350: build bench_segmented_algos.cpp (group 17) with g++-16 and clang++-22,
# run user's exact command once, then 5 pinned runs each, extract partition disasm.
set -e
EXP=/mnt/d/Data/LocalGit/boost/libs/container/experimental
OUT=$EXP/cursor_build/g30/nsgpart
mkdir -p "$OUT"
cd "$EXP"

FLAGS="-std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=17 -falign-functions=64"

echo "== compiling gcc =="
g++-16     $FLAGS bench_segmented_algos.cpp -o /tmp/g350_gcc.elf   2> "$OUT/350_gcc_compile.log" &
PID_G=$!
echo "== compiling clang =="
clang++-22 $FLAGS bench_segmented_algos.cpp -o /tmp/g350_clang.elf 2> "$OUT/350_clang_compile.log" &
PID_C=$!
wait $PID_G; wait $PID_C
echo "== compile done =="
cp /tmp/g350_gcc.elf   "$OUT/350_gcc.elf"
cp /tmp/g350_clang.elf "$OUT/350_clang.elf"

# --- user's exact command (unpinned) ---
echo "== exact run gcc =="
/tmp/g350_gcc.elf   > "$OUT/350_gcc_exact.txt"   2>&1
echo "== exact run clang =="
/tmp/g350_clang.elf > "$OUT/350_clang_exact.txt" 2>&1

# --- pinned runs ---
if sudo -n true 2>/dev/null; then
   PIN="sudo -n chrt -f 90 taskset -c 3"
else
   PIN="nice -n -5 taskset -c 3"
fi
echo "== pinning with: $PIN =="
for i in 1 2 3 4 5; do
   echo "== pinned gcc run $i =="
   $PIN /tmp/g350_gcc.elf   > "$OUT/350_gcc_pin$i.txt"   2>&1
done
for i in 1 2 3 4 5; do
   echo "== pinned clang run $i =="
   $PIN /tmp/g350_clang.elf > "$OUT/350_clang_pin$i.txt" 2>&1
done

# --- disassembly: extract measure_batch instantiations mentioning partition ---
for cc in gcc clang; do
   objdump -d --no-show-raw-insn "/tmp/g350_$cc.elf" | c++filt > "/tmp/g350_$cc.dis"
   awk '/^[0-9a-f]+ <.*measure_batch.*partition.*>:$/{p=1} p{print} p&&/^$/{p=0}' \
      "/tmp/g350_$cc.dis" > "$OUT/350_${cc}_partition_funcs.dis"
   # symbol inventory of anything partition-related that survived as a symbol
   objdump -t "/tmp/g350_$cc.elf" | c++filt | grep -i partition | sort > "$OUT/350_${cc}_partition_syms.txt" || true
   cp "/tmp/g350_$cc.dis" "$OUT/350_${cc}_full.dis"
done

echo "== partition rows, exact runs =="
grep -H "partition" "$OUT"/350_*_exact.txt
echo "== done =="
