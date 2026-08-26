#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
CB=$BR/libs/container/bench
H=$BR/libs/container/experimental/cursor_build/g49/nsgeq
O=/tmp/g49_pr_$$
mkdir -p "$O"

clang++-22 -std=c++20 -O3 -DNDEBUG -I"$BR" -I"$CB" -falign-functions=64 -falign-loops=64 \
   "$H/040_promote.cpp" -o "$O/p.elf" || { echo BUILDFAIL; exit 1; }

echo "=== promotion check: block-end computation per variant ==="
objdump -d --no-show-raw-insn -C "$O/p.elf" > "$O/p.asm"
for fn in "bool eq_shape_std" "bool eq_shape_leaf"; do
   echo "--- $fn (out-of-line) ---"
   awk -v f="$fn" 'index($0,f) && /^0/ {p=1} p {print} p && /^$/ {exit}' "$O/p.asm" \
      | grep -E "lea .*0x200|mov +\(%r..\),%r.. *$|add .*%r" | head -6
done
echo "--- main (inlined std::equal + segmented_equal): 0x200 usage ---"
awk '/<main>:/ {p=1} p {print} p && /^ret/ {exit}' "$O/p.asm" | grep -cE "lea +0x200" || true
awk '/<main>:/ {p=1} p {print} p && /^$/ {exit}' "$O/p.asm" | grep -E "lea +0x200|mov +\$0x200" | head -8

echo
echo "=== timings (3 rounds) ==="
for i in 1 2 3; do taskset -c 3 "$O/p.elf"; echo; done
