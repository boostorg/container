#!/bin/bash
set -u
G34=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g34/unroll
D=$G34/dis; mkdir -p $D

for pair in "gcc_off 3500" "gcc_on 3f00"; do
  set -- $pair; cfg=$1; a=$2
  echo "############ $cfg : outlined segmented_copy_if callee @$a ############"
  nm -C --size-sort -S $G34/$cfg.elf | grep -i "^0*$a " | head -3
  sz=$(nm --size-sort -S $G34/$cfg.elf | awk -v A=$a 'tolower($1)==sprintf("%016x",strtonum("0x" A)){print $2}' | head -1)
  echo "size=0x$sz"
  st=$((0x$a)); en=$((0x$a + 0x${sz:-400}))
  objdump -d --start-address=$st --stop-address=$en --no-show-raw-insn $G34/$cfg.elf \
    | sed -n '/:\t/p' | sed 's/<[^>]*>//g' > $D/${cfg}_callee.txt
  echo "instructions: $(wc -l < $D/${cfg}_callee.txt)"
  cat $D/${cfg}_callee.txt
  echo
done
echo DONE
