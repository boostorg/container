#!/bin/bash
set -u

# Raw dump of clang dst_dispatch without filtering insn lines
awk '/^0000000000016f20 </{p=1} p{print} p&&/^$/{exit}' /tmp/g25_clang++-22.asm \
   | sed -E 's/<boost::container::[^>]+>/<...>/g' > /tmp/clang_dst.txt
echo "clang instructions: $(grep -cE '^    [0-9a-f]+:' /tmp/clang_dst.txt)"
echo "===== clang dst hot section (first 120 insn lines) ====="
grep -E '^    [0-9a-f]+:|^/mnt|segmented_copy_if|bench_utils|deque_impl' /tmp/clang_dst.txt | head -130

echo
echo "===== GCC unrolled block vs clang: odd-test form ====="
echo "GCC test \$0x1 count: $(grep -c 'test   \$0x1' /tmp/g25_g++-16.asm | head -1)"
# count in the specific function
awk '/^00000000000044a0 </{p=1} p&&/^$/{exit} p' /tmp/g25_g++-16.asm > /tmp/gcc_dst.txt
echo "GCC dst: test\$0x1=$(grep -c 'test   \$0x1' /tmp/gcc_dst.txt)  and\$0x1=$(grep -c 'and.*=\$0x1\|and.*,\$0x1' /tmp/gcc_dst.txt)  cmov=$(grep -c cmov /tmp/gcc_dst.txt)"
echo "CLANG dst: test\$0x1=$(grep -c 'test   \$0x1' /tmp/clang_dst.txt)  and=$(grep -cE 'and ' /tmp/clang_dst.txt)  cmov=$(grep -c cmov /tmp/clang_dst.txt)  jcc=$(grep -cE 'j[a-z]+ ' /tmp/clang_dst.txt)"

echo
echo "===== find clang std_copy_if measure_batch range ====="
grep -n "^0[0-9a-f]* <unsigned long measure_batch" /tmp/g25_clang++-22.asm | grep "std_copy_if" | grep "is_odd" | grep "vector" | head -5

# Print clang std loop around stl_algo by line number 29561
echo "===== CLANG STD around stl_algo ====="
sed -n '29561,30500p' /tmp/g25_clang++-22.asm \
   | awk '/stl_algo\.h:657/{n++} n==1{print} n==1 && /^    [0-9a-f]+:/{c++} n==1 && c>40{exit}' \
   | grep -E '^    |stl_algo|deque_impl|bench_utils' | head -50

echo "===== GCC STD around stl_algo ====="
sed -n '28859,29800p' /tmp/g25_g++-16.asm \
   | awk '/stl_algo\.h:657/{n++} n==1{print} n==1 && /^    [0-9a-f]+:/{c++} n==1 && c>40{exit}' \
   | grep -E '^    |stl_algo|deque_impl|bench_utils' | head -50
