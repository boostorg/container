#!/bin/bash
set -u
# Dump hot loops for copy_if(2S miss): V+D is_negative
# seg@9162 wrap0, nsg@22264 wrap1, std@23535

pr_loop() {
   local start=$1 title=$2
   echo "################ $title (line $start) ################"
   # Find first copy_if / cleanup_blocks / is_negative loop body
   sed -n "${start},$((start+800))p" /tmp/g25.asm \
      | sed -E 's/<unsigned long measure_batch[^>]*\+/<mb+/; s/^\/mnt.*\/([A-Za-z0-9_.]+):([0-9]+)$/;; \1:\2/' \
      | awk '
         /segmented_copy_if\.hpp:|stl_algo\.h:|copy_if_cleanup|is_negative|bench_utils\.hpp:168/ {
            if(!seen[$0]++) hits++
         }
         /^    [0-9a-f]+:/ { if(hits>=1){ print; n++; if(n>55) exit } }
      '
}

pr_loop 9162  "SEG  V+D miss"
pr_loop 22264 "NSG  V+D miss"
pr_loop 23535 "STD  V+D miss"

echo
echo "########## source refs in each body ##########"
for pair in "9162:SEG" "22264:NSG" "23535:STD"; do
   start=${pair%%:*}; name=${pair##*:}
   echo "--- $name ---"
   sed -n "${start},$((start+900))p" /tmp/g25.asm \
      | grep -oE '[A-Za-z0-9_./]+\.(hpp|h):[0-9]+' \
      | sed 's|.*/||' | sort | uniq -c | sort -rn | head -12
done

echo
echo "########## calls inside each measure_batch ##########"
for pair in "9162:SEG" "22264:NSG" "23535:STD"; do
   start=${pair%%:*}; name=${pair##*:}
   echo -n "$name calls: "
   awk -v s="$start" '
      NR==s {p=1}
      p && NR>s && /^0[0-9a-f]+ </ {exit}
      p && /call /
   ' /tmp/g25.asm | wc -l
   awk -v s="$start" '
      NR==s {p=1}
      p && NR>s && /^0[0-9a-f]+ </ {exit}
      p && /call /
   ' /tmp/g25.asm | sed -E 's/.*call\s+//; s/<.*detail_algo::/<detail_algo::/; s/<unsigned long measure_batch/<mb/;' | head -8
done
