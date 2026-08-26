#!/bin/bash
# 368: the nsg (wrapped-iterator) inner loops for the 2S case, both predicates.
# The destination there is a wrapped deque_iterator, not a flat pointer, so its
# operator++ carries a per-write block-boundary test.
set -u
BR=/mnt/d/Data/LocalGit/boost
WD=$BR/libs/container/experimental/cursor_build/g31/copyif
cd "$WD" || exit 1

dump() { # elf addr size out
  local end
  end=$(python3 -c "print(hex(int('$2',16)+int('$3',16)))")
  objdump -d --no-show-raw-insn --start-address="$2" --stop-address="$end" "$1" \
    | sed -n 's/^ *\([0-9a-f]*\):\t/\1: /p' | sed 's/<[^>]*>//g; s/ *$//' > "$4"
  printf "%-34s %4d insns\n" "$4" "$(wc -l < "$4")"
}

ELF=365_bench_p0_g++-16.elf
for A in 00000000000107c0:is_negative 00000000000128c0:is_odd; do
  ADDR=0x${A%%:*}; NAME=${A##*:}
  SZ=0x$(nm -S "$ELF" | grep -i "^${A%%:*} " | awk '{print $2}')
  dump "$ELF" "$ADDR" "$SZ" "368_g_nsg2S_${NAME}.asm"
done

echo
echo "===== g++ nsg 2S / is_odd : inner loop (search for the deque ++ boundary test) ====="
head -70 368_g_nsg2S_is_odd.asm
echo
echo "===== g++ nsg 2S / is_negative : inner loop ====="
head -45 368_g_nsg2S_is_negative.asm
