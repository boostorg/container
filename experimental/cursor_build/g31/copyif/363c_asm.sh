#!/bin/bash
# 363c: address-sliced disassembly of the 2S copy_if seg walkers and the
# nsg measure_batch bodies.
set -u
BR=/mnt/d/Data/LocalGit/boost
WD=$BR/libs/container/experimental/cursor_build/g31/copyif
cd "$WD" || exit 1

slice() { # $1=elf $2=mangled $3=out
  local elf="$1" sym="$2" out="$3"
  local line addr size
  line=$(nm -S "$elf" | grep -F -m1 -- "$sym") || return 1
  [ -z "$line" ] && { echo "MISS $sym"; return 1; }
  addr=0x$(echo "$line" | awk '{print $1}')
  size=0x$(echo "$line" | awk '{print $2}')
  python3 -c "print(hex(int('$addr',16)+int('$size',16)))" > /tmp/end.$$
  objdump -d --no-show-raw-insn --start-address="$addr" \
     --stop-address="$(cat /tmp/end.$$)" "$elf" | c++filt > "$out"
  printf "  %-40s %6d insns  (size %s)\n" "$out" \
     "$(grep -cE '^\s+[0-9a-f]+:' "$out")" "$size"
}

for CXX in g++-16 clang++-22; do
  ELF="a_${CXX}.elf"
  TAG=$(echo "$CXX" | sed 's/[+]//g;s/-.*//')
  echo "===================== $CXX ====================="
  # 2S seg walkers: source is vec_iterator<MyInt*,true>
  for P in 11is_negative 6is_odd; do
    S=$(nm "$ELF" | grep 'segmented_copy_if_dst_dispatchINS0_12vec_iterator' | grep "$P" | awk '{print $NF}' | head -1)
    N=$(echo "$P" | sed 's/^[0-9]*//')
    echo "seg walker 2S / $N:"
    slice "$ELF" "$S" "363_${TAG}_seg2S_${N}.txt"
  done
  # 2S nsg: measure_batch< seg_copy_if< vector, deque, PRED, true > >
  for P in 11is_negative 6is_odd; do
    N=$(echo "$P" | sed 's/^[0-9]*//')
    for W in Lb0E Lb1E; do
      S=$(nm "$ELF" | grep '13measure_batchIN' | grep '11seg_copy_ifIN' | \
          grep 'container6vectorI5MyIntvvENS2_5dequeIS5_' | grep "$P" | grep "${W}EE10noop_reset" | awk '{print $NF}' | head -1)
      if [ -z "$S" ]; then
        S=$(nm "$ELF" | grep 'measure_batch' | grep 'seg_copy_if' | \
            grep '6vectorI5MyIntvvENS._5dequeIS5_' | grep "$P" | grep "$W" | awk '{print $NF}' | head -1)
      fi
      L=$( [ "$W" = "Lb1E" ] && echo nsg || echo segdup )
      if [ -n "$S" ]; then
        echo "measure_batch 2S / $N / $L:"
        slice "$ELF" "$S" "363_${TAG}_mb2S_${N}_${L}.txt"
      else
        echo "measure_batch 2S / $N / $L: SYMBOL NOT FOUND"
      fi
    done
  done
  echo
done
ls -la 363_*.txt
