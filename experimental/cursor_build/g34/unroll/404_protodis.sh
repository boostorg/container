#!/bin/bash
set -u
G34=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g34/unroll
PAT="seg_copy_ifIN5boost9container5dequeI5MyIntvNS3_9deque_optILm0ELm128EvLb0EEEEENS3_6vectorIS5_vvEE11is_negativeIS5_ELb1EEE"
for cfg in clang_off clang_proto; do
  line=$(nm --size-sort -S $G34/$cfg.elf | grep measure_batch | grep -F "$PAT" | head -1)
  a=$(echo $line|cut -d' ' -f1); s=$(echo $line|cut -d' ' -f2)
  objdump -d --start-address=$((0x$a)) --stop-address=$((0x$a+0x$s)) --no-show-raw-insn $G34/$cfg.elf \
    | sed -n '/:\t/p' | sed 's/<_Z[^>]*>//g' > /tmp/$cfg.dis
  echo "######## $cfg  (size 0x$s) : smallest repeated loop bodies ########"
  awk 'match($0,/^[ \t]+([0-9a-f]+):/,m){addr=strtonum("0x" m[1])}
       /j(mp|e|ne|s|ns|b|be|a|ae)[ \t]+[0-9a-f]+/{
         if(match($0,/[ \t]([0-9a-f]+)[ \t]*$/,t)){tgt=strtonum("0x" t[1]);
           if(tgt<addr && addr-tgt<400) printf "%d %x %x\n", addr-tgt, tgt, addr }}' /tmp/$cfg.dis \
    | sort -n | awk '!seen[$1]++' | head -4
  echo "--- first hot loop body ---"
  h=$(awk 'match($0,/^[ \t]+([0-9a-f]+):/,m){addr=strtonum("0x" m[1])}
       /j(mp|e|ne|s|ns|b|be|a|ae)[ \t]+[0-9a-f]+/{
         if(match($0,/[ \t]([0-9a-f]+)[ \t]*$/,t)){tgt=strtonum("0x" t[1]);
           if(tgt<addr && addr-tgt<400) printf "%d %x %x\n", addr-tgt, tgt, addr }}' /tmp/$cfg.dis \
    | sort -n -k3 | head -1)
  lo=$(echo $h|cut -d' ' -f2); hi=$(echo $h|cut -d' ' -f3)
  echo "loop $lo .. $hi"
  awk -v L=$((0x$lo)) -v H=$((0x$hi+8)) 'match($0,/^[ \t]+([0-9a-f]+):/,m){a=strtonum("0x" m[1]); if(a>=L&&a<=H) print}' /tmp/$cfg.dis
  echo
done
echo DONE
