#!/bin/bash
set -u
G34=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g34/unroll
D=$G34/dis; mkdir -p $D

for cfg in gcc_off gcc_on; do
echo "############################ $cfg ############################"
for pat in "seg_copy_ifIN5boost9container6vectorI5MyIntvvEENS3_5dequeIS5_vNS3_9deque_optILm0ELm128EvLb0EEEEE11is_negativeIS5_ELb0EEE"; do
  line=$(nm --size-sort -S $G34/$cfg.elf | grep "measure_batch" | grep -F "$pat" | head -1)
  echo "sym: $line"
  a=$(echo $line | cut -d' ' -f1); s=$(echo $line | cut -d' ' -f2)
  st=$((0x$a)); en=$((0x$a + 0x$s))
  objdump -d --start-address=$st --stop-address=$en --no-show-raw-insn $G34/$cfg.elf \
    | sed -n '/:\t/p' | sed 's/<_Z[^>]*>//g' > $D/${cfg}_copyif2Smiss_seg.txt
  echo "instructions: $(wc -l < $D/${cfg}_copyif2Smiss_seg.txt)"
  echo "--- back-edges (loop bodies) ---"
  awk 'match($0,/^[ \t]+([0-9a-f]+):/,m){addr=strtonum("0x" m[1])}
       /j(mp|e|ne|b|be|a|ae|l|le|g|ge|s|ns)[ \t]+[0-9a-f]+/{
         if(match($0,/[ \t]([0-9a-f]+)$/,t)||match($0,/[ \t]([0-9a-f]+)[ \t]/,t)){
           tgt=strtonum("0x" t[1]); if(tgt<addr && addr-tgt<600)
             printf "  %x -> %x (%d bytes)\n", addr, tgt, addr-tgt }}' \
     $D/${cfg}_copyif2Smiss_seg.txt | sort -u -k3 | head -20
done
echo
done

echo "########## first Duff-copy body, gcc_off (32-elem block loop region) ##########"
sed -n '1,140p' $D/gcc_off_copyif2Smiss_seg.txt
echo
echo "########## first Duff-copy body, gcc_on ##########"
sed -n '1,170p' $D/gcc_on_copyif2Smiss_seg.txt
echo DONE
