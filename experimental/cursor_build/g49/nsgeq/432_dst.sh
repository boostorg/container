#!/bin/bash
set -u
echo "################ out-of-line segmented_copy_if_dst_dispatch @0x31c0 ################"
awk '/^0+31c0 </{p=1} p{print} p && /^$/ {exit}' /tmp/g25.asm \
   | sed -E 's/<boost::container::[^>]*>/<...>/g; s/^\/mnt.*\/([A-Za-z0-9_.]+):([0-9]+)$/;; \1:\2/' \
   | grep -vE '^(boost::|segduo|unsigned|is_negative)' | head -120

echo
echo "########## instruction count / line census ##########"
awk '/^0+31c0 </{p=1} p && /^$/{exit} p' /tmp/g25.asm > /tmp/g25_dst.asm
echo -n "instructions: "; grep -cE '^    [0-9a-f]+:' /tmp/g25_dst.asm
echo "line census:"
grep -oE '[A-Za-z0-9_.]+\.(hpp|h):[0-9]+' /tmp/g25_dst.asm | sed 's|.*/||' | sort | uniq -c | sort -rn | head -15
echo -n "internal calls: "; grep -cE 'call ' /tmp/g25_dst.asm || true
grep -E 'call ' /tmp/g25_dst.asm | sed -E 's/.*call\s+//; s/<boost::container::detail_algo::/<da::/; s/\(.*//' | head -6
