#!/bin/bash
# 363b: disassemble the 2S copy_if walkers/leaves (seg) and the nsg inner loops.
set -u
BR=/mnt/d/Data/LocalGit/boost
WD=$BR/libs/container/experimental/cursor_build/g31/copyif
cd "$WD" || exit 1

dump() { # $1=elf $2=mangled-symbol $3=outfile
  objdump -d --no-show-raw-insn --disassemble="$2" "$1" 2>/dev/null \
    | c++filt > "$3"
  printf "%-52s -> %6s insns  %s\n" "$3" "$(grep -cE '^\s+[0-9a-f]+:' "$3")" \
     "$(head -c 0 /dev/null)"
}

echo "===================== g++-16 ====================="
# out-of-line segmented destination walkers (leaf force-inlined into them)
G_NEG=$(nm a_g++-16.elf | grep 'segmented_copy_if_dst_dispatch.*is_negative' | awk '{print $3}')
G_ODD=$(nm a_g++-16.elf | grep 'segmented_copy_if_dst_dispatch.*is_odd'      | awk '{print $3}')
echo "walker(is_negative) = $G_NEG"
echo "walker(is_odd)      = $G_ODD"
dump a_g++-16.elf "$G_NEG" 363_gcc_walker_neg.txt
dump a_g++-16.elf "$G_ODD" 363_gcc_walker_odd.txt

# nsg (Wrap=true) measure_batch for 2S: vector source, deque dest
for P in is_odd is_negative; do
  for W in false true; do
    S=$(nm a_g++-16.elf | grep "measure_batch.*seg_copy_if.*vectorIS3_vvE.*deque.*${P}.*${W}" | awk '{print $3}' | head -1)
    [ -n "$S" ] && dump a_g++-16.elf "$S" "363_gcc_mb_2S_${P}_wrap${W}.txt"
  done
done

echo
echo "===================== clang++-22 ====================="
C_NEG=$(nm a_clang++-22.elf | grep 'segmented_copy_if_dst_dispatch.*is_negative' | awk '{print $3}')
C_ODD=$(nm a_clang++-22.elf | grep 'segmented_copy_if_dst_dispatch.*is_odd'      | awk '{print $3}')
echo "walker(is_negative) = ${C_NEG:-<inlined>}"
echo "walker(is_odd)      = ${C_ODD:-<inlined>}"
[ -n "$C_NEG" ] && dump a_clang++-22.elf "$C_NEG" 363_clang_walker_neg.txt
[ -n "$C_ODD" ] && dump a_clang++-22.elf "$C_ODD" 363_clang_walker_odd.txt

nm -C a_clang++-22.elf | grep -c 'segmented_copy_if' || true
echo "--- clang measure_batch symbols for 2S copy_if ---"
nm a_clang++-22.elf | grep 'measure_batch.*seg_copy_if' | c++filt | grep 'vector<MyInt' | head -8
for P in is_odd is_negative; do
  for W in false true; do
    S=$(nm a_clang++-22.elf | grep "measure_batch.*seg_copy_if.*vectorI5MyInt.*deque.*${P}.*${W}" | awk '{print $3}' | head -1)
    if [ -z "$S" ]; then
      S=$(nm a_clang++-22.elf | grep "measure_batch.*seg_copy_ifIN.*vector.*deque.*${P}.*L${W}" | awk '{print $3}' | head -1)
    fi
    [ -n "$S" ] && dump a_clang++-22.elf "$S" "363_clang_mb_2S_${P}_wrap${W}.txt"
  done
done
