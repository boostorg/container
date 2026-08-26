#!/bin/bash
# 367: clean per-loop disassembly of the 2S copy_if paths, p0 vs p2, both
# compilers, with the symbol noise stripped from branch targets.
set -u
BR=/mnt/d/Data/LocalGit/boost
WD=$BR/libs/container/experimental/cursor_build/g31/copyif
cd "$WD" || exit 1

clean() { sed 's/<[^>]*>//g; s/ *$//' ; }

slice() { # elf mangled out
  local elf="$1" sym="$2" out="$3" line addr size end
  line=$(nm -S "$elf" | grep -F -m1 -- "$sym")
  [ -z "$line" ] && { echo "  MISS: $sym"; return 1; }
  addr=0x$(echo "$line" | awk '{print $1}')
  size=0x$(echo "$line" | awk '{print $2}')
  end=$(python3 -c "print(hex(int('$addr',16)+int('$size',16)))")
  objdump -d --no-show-raw-insn --start-address="$addr" --stop-address="$end" "$elf" \
    | sed -n 's/^ *\([0-9a-f]*\):\t/\1: /p' | clean > "$out"
  printf "  %-38s %4d insns\n" "$out" "$(wc -l < "$out")"
}

for V in p0 p2; do
 for CXX in g++-16 clang++-22; do
  T=$(echo "$CXX" | sed 's/[+]//g;s/-.*//')
  ELF="365_bench_${V}_${CXX}.elf"
  [ -f "$ELF" ] || { echo "no $ELF"; continue; }
  echo "=== $V / $CXX ==="
  for P in 11is_negative 6is_odd; do
    N=$(echo "$P" | sed 's/^[0-9]*//')
    S=$(nm "$ELF" | grep 'segmented_copy_if_dst_dispatchINS0_12vec_iterator' | grep "$P" | awk '{print $NF}' | head -1)
    [ -n "$S" ] && slice "$ELF" "$S" "367_${T}_${V}_2S_${N}.asm"
  done
 done
done

echo
echo "############ g++-16 p0, 2S/is_negative: the copy_if_cleanup_blocks<32> loop ############"
sed -n '1,80p' 367_g_p0_2S_is_negative.asm

echo
echo "############ g++-16 p2, 2S/is_odd: the per-boundary sequence ############"
grep -n . 367_g_p2_2S_is_odd.asm | tail -60

echo
echo "############ static instruction counts ############"
for f in 367_*.asm; do printf "%-40s %4d\n" "$f" "$(wc -l < "$f")"; done
