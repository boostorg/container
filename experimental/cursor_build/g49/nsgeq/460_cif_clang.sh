#!/bin/bash
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$E"
H=$E/cursor_build/g49/nsgeq
mkdir -p "$H"

ROWS='copy_if\(2S hit\)|copy_if\(2S miss\)|copy_if\(1S hit\)'

for CXX in g++-16 clang++-22; do
   echo "########## $CXX reproduce ##########"
   $CXX -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 \
      -g bench_segmented_algos.cpp -o /tmp/g25_${CXX}.elf || { echo BUILDFAIL; continue; }
   for i in 1 2 3; do
      taskset -c 3 /tmp/g25_${CXX}.elf 2>/dev/null | grep -E "$ROWS"
      echo ---
   done
   objdump -d --no-show-raw-insn -C -l /tmp/g25_${CXX}.elf > /tmp/g25_${CXX}.asm
done

echo "########## V+D is_odd measure_batch symbols ##########"
for CXX in g++-16 clang++-22; do
   echo "===== $CXX ====="
   grep -n "^0[0-9a-f]* <unsigned long measure_batch" /tmp/g25_${CXX}.asm | grep "copy_if" | while IFS= read -r l; do
      line=${l%%:*}; name=${l#*:}
      echo "$name" | grep -q "is_odd" || continue
      echo "$name" | grep -q "vector<MyInt" || continue
      # first container vector, second deque
      shape=$(echo "$name" | grep -oE "vector<MyInt|deque<MyInt" | head -2 | paste -sd+)
      echo "$shape" | grep -q "vector.*deque\|V" || true
      wrap=$(echo "$name" | grep -c ", true>")
      kind=$(echo "$name" | grep -oE "std_copy_if|seg_copy_if" | head -1)
      # only V then D
      c1=$(echo "$name" | grep -oE "vector<MyInt|deque<MyInt" | head -1)
      c2=$(echo "$name" | grep -oE "vector<MyInt|deque<MyInt" | sed -n '2p')
      [ "$c1" = "vector<MyInt" ] && [ "$c2" = "deque<MyInt" ] || continue
      echo "$line  $kind  wrap=$wrap"
   done
done
