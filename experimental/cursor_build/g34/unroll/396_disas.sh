#!/bin/bash
set -u
G34=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g34/unroll
D=$G34/dis
mkdir -p $D

dump() { # $1=elf $2=start $3=size $4=name
  objdump -d --start-address=0x$2 --stop-address=$((0x$2+0x$3)) $1 \
    | sed -n '/>:/,$p' > $D/$4.txt
  echo "--- $4 : $(grep -cE '^\s+[0-9a-f]+:' $D/$4.txt) instructions ---"
}

# clang_off
dump $G34/clang_off.elf 1ea40 977 clangoff_copyif_1Smiss_nsg
dump $G34/clang_off.elf 17080 978 clangoff_copyif_1Shit_nsg
dump $G34/clang_off.elf 1cc40 9ac clangoff_copyif_1Smiss_std
dump $G34/clang_off.elf 1d600 1412 clangoff_copyif_1Smiss_seg

echo
echo "==================== loop-back edges per function ===================="
for f in clangoff_copyif_1Smiss_nsg clangoff_copyif_1Shit_nsg clangoff_copyif_1Smiss_std; do
  echo "### $f"
  awk '
    match($0,/^[ \t]+([0-9a-f]+):/,m){ addr=strtonum("0x" m[1]) }
    /j(mp|e|ne|b|be|a|ae|l|le|g|ge|s|ns)[ \t]+[0-9a-f]+/{
      if (match($0,/[ \t]([0-9a-f]+) </,t)) {
        tgt=strtonum("0x" t[1]);
        if (tgt < addr) printf "  back-edge at %x -> %x  (body %d bytes) : %s\n", addr, tgt, addr-tgt, $0
      }
    }' $D/$f.txt
  echo
done
echo DONE
