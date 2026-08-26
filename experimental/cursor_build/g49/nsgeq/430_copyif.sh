#!/bin/bash
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$E"

echo "########## reproduce group 25 copy_if rows ##########"
g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 \
   -g bench_segmented_algos.cpp -o /tmp/g25.elf || { echo BUILDFAIL; exit 1; }

for i in 1 2 3; do
   echo "--- run $i ---"
   taskset -c 3 /tmp/g25.elf 2>/dev/null | grep -E "copy_if"
done

objdump -d --no-show-raw-insn -C -l /tmp/g25.elf > /tmp/g25.asm
echo "asm lines: $(wc -l </tmp/g25.asm)"

# Identify measure_batch symbols for copy_if 2S
echo
echo "########## copy_if measure_batch symbols ##########"
grep -n "^0[0-9a-f]* <unsigned long measure_batch" /tmp/g25.asm | grep copy_if | while IFS= read -r l; do
   line=${l%%:*}
   name=${l#*:}
   wrap=$(echo "$name" | grep -c ", true>")
   # container order in template args
   shape=$(echo "$name" | grep -oE "vector<MyInt|deque<MyInt" | head -2 | paste -sd+ | sed 's/vector<MyInt/V/g; s/deque<MyInt/D/g')
   pred=$(echo "$name" | grep -oE "is_odd|is_negative" | head -1)
   kind=$(echo "$name" | grep -oE "std_copy_if|seg_copy_if" | head -1)
   echo "$line  $kind  $shape  pred=$pred  wrap=$wrap"
done
