#!/bin/bash
set -u
G34=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g34/unroll
OUT=$G34/runs; D=$G34/dis
med() { sort -g | awk '{a[NR]=$1} END{print a[int((NR+1)/2)]}'; }

echo "############ GEOMEAN of median seg_ns / nsg_ns over the 30 group-25 rows ############"
printf '%-14s %12s %12s\n' config geomean_seg_ns geomean_nsg_ns
for cfg in gcc_on gcc_off gcc_proto clang_on clang_off clang_proto; do
  grep -hE '^(copy|remove|swap|transform)' $OUT/$cfg.1.txt | cut -c1-28 | while IFS= read -r lab; do
    key=$(echo "$lab" | sed 's/ *$//')
    s=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $(NF-2)}'; done|med)
    n=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $NF}'; done|med)
    echo "$s $n"
  done > /tmp/gm_$cfg.txt
  awk -v c=$cfg '{ls+=log($1); ln+=log($2); k++} END{printf "%-14s %12.4f %12.4f\n", c, exp(ls/k), exp(ln/k)}' /tmp/gm_$cfg.txt
done

echo
echo "############ copy_if rows only: geomean of seg_ns and nsg_ns ############"
printf '%-14s %12s %12s\n' config geo_seg_copyif geo_nsg_copyif
for cfg in gcc_on gcc_off gcc_proto clang_on clang_off clang_proto; do
  grep -hE '^copy_if' $OUT/$cfg.1.txt | cut -c1-28 | while IFS= read -r lab; do
    key=$(echo "$lab" | sed 's/ *$//')
    s=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $(NF-2)}'; done|med)
    n=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/$cfg.$i.txt|head -1|awk '{print $NF}'; done|med)
    echo "$s $n"
  done > /tmp/gc_$cfg.txt
  awk -v c=$cfg '{ls+=log($1); ln+=log($2); k++} END{printf "%-14s %12.4f %12.4f\n", c, exp(ls/k), exp(ln/k)}' /tmp/gc_$cfg.txt
done

echo
echo "############ run-to-run spread (max/min over 5 runs), seg_ns, clang_off ############"
grep -hE '^(copy_if|copy_n)' $OUT/clang_off.1.txt | cut -c1-28 | while IFS= read -r lab; do
  key=$(echo "$lab" | sed 's/ *$//')
  v=$(for i in 1 2 3 4 5; do grep -F "$key " $OUT/clang_off.$i.txt|head -1|awk '{print $NF}'; done|sort -g)
  lo=$(echo "$v"|head -1); hi=$(echo "$v"|tail -1)
  awk -v l="$key" -v a=$lo -v b=$hi 'BEGIN{printf "  %-26s nsg_ns min=%.3f max=%.3f spread=%.1f%%\n", l,a,b,(b/a-1)*100}'
done

echo
echo "############ clang_proto: copy_if(1S miss) NSG inner loop ############"
a=$(nm --size-sort -S $G34/clang_proto.elf | grep measure_batch | grep -F "seg_copy_ifIN5boost9container5dequeI5MyIntvNS3_9deque_optILm0ELm128EvLb0EEEEENS3_6vectorIS5_vvEE11is_negativeIS5_ELb1EEE" | head -1 | cut -d' ' -f1)
s=$(nm --size-sort -S $G34/clang_proto.elf | grep measure_batch | grep -F "seg_copy_ifIN5boost9container5dequeI5MyIntvNS3_9deque_optILm0ELm128EvLb0EEEEENS3_6vectorIS5_vvEE11is_negativeIS5_ELb1EEE" | head -1 | cut -d' ' -f2)
echo "sym at 0x$a size 0x$s"
objdump -d --start-address=$((0x$a)) --stop-address=$((0x$a+0x$s)) --no-show-raw-insn $G34/clang_proto.elf \
  | sed -n '/:\t/p' | sed 's/<_Z[^>]*>//g' | sed -n '30,120p'
echo DONE
