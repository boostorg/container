#!/bin/bash
set -u

# Dump out-of-line dst_dispatch bodies for is_odd (2S hit path)
for CXX in g++-16 clang++-22; do
   echo "########## $CXX dst_dispatch is_odd symbols ##########"
   grep -E "^0[0-9a-f]+ <" /tmp/g25_${CXX}.asm | grep "segmented_copy_if_dst_dispatch" | grep "is_odd" \
      | grep "vec_iterator" | grep "deque_iterator"
done

extract() {
   local f=$1 addr=$2 title=$3
   echo "################ $title @ $addr ################"
   awk -v a="^0+$addr <" '
      $0 ~ a {p=1}
      p {print}
      p && /^$/ {exit}
   ' "$f" | sed -E 's/<boost::container::[^>]{20,}>/<...>/g; s/^\/mnt.*\/([A-Za-z0-9_.]+):([0-9]+)$/;; \1:\2/' \
      > /tmp/ext.txt
   echo "instructions: $(grep -cE '^    [0-9a-f]+:' /tmp/ext.txt)"
   echo "line census:"
   grep -oE '[A-Za-z0-9_.]+\.(hpp|h):[0-9]+' /tmp/ext.txt | sed 's|.*/||' | sort | uniq -c | sort -rn | head -15
   echo "--- loop1 / room check ---"
   grep -nE 'cmp +\$0x1c|cmp +\$0x20|sub +\$0x8|segmented_copy_if\.hpp:9[0-9]|segmented_copy_if\.hpp:10[0-9]|segmented_copy_if\.hpp:11[0-9]|segmented_copy_if\.hpp:12[0-9]|segmented_copy_if\.hpp:13[0-9]' /tmp/ext.txt | head -20
   echo "--- first 80 insn ---"
   grep -E '^    [0-9a-f]+:|^;;' /tmp/ext.txt | head -80
   echo
}

# Get addresses
GADDR=$(grep -E "^0[0-9a-f]+ <" /tmp/g25_g++-16.asm | grep "segmented_copy_if_dst_dispatch" | grep "is_odd" | grep "vec_iterator" | grep "deque_iterator" | head -1 | sed -E 's/^0*([0-9a-f]+) .*/\1/')
CADDR=$(grep -E "^0[0-9a-f]+ <" /tmp/g25_clang++-22.asm | grep "segmented_copy_if_dst_dispatch" | grep "is_odd" | grep "vec_iterator" | grep "deque_iterator" | head -1 | sed -E 's/^0*([0-9a-f]+) .*/\1/')
echo "GCC addr=$GADDR CLANG addr=$CADDR"

extract /tmp/g25_g++-16.asm "$GADDR" "GCC dst_dispatch is_odd"
extract /tmp/g25_clang++-22.asm "$CADDR" "CLANG dst_dispatch is_odd"

# Clang std::copy_if inner loop - search in measure_batch around stl_algo
echo "################ CLANG STD inner loop ################"
awk '/measure_batch.*std_copy_if.*vector<MyInt.*deque<MyInt.*is_odd/{p=1; n=0}
     p && /stl_algo\.h:65[78]/ {grab=1}
     grab && /^    [0-9a-f]+:/ {print; c++; if(c>35) exit}
     p && /^0[0-9a-f]+ </ && n++>0 {exit}' /tmp/g25_clang++-22.asm

echo "################ GCC STD inner loop ################"
awk '/measure_batch.*std_copy_if.*vector<MyInt.*deque<MyInt.*is_odd/{p=1; n=0}
     p && /stl_algo\.h:65[78]/ {grab=1}
     grab && /^    [0-9a-f]+:/ {print; c++; if(c>35) exit}
     p && /^0[0-9a-f]+ </ && n++>0 {exit}' /tmp/g25_g++-16.asm
