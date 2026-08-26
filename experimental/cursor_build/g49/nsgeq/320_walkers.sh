#!/bin/bash
# Compare emitted walker quality: equal_dispatch<deque,deque> (0x31e0) vs
# mismatch_bounded_dispatch<deque,deque,unreachable> (0x2f60).
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq

body() { # addr
   awk -v s="^0+$1 <" '
      $0 ~ s {p=1}
      p && /^$/ {exit}
      p {print}
   ' bench_gcc.asm
}

for a in 31e0 2f60 2990 25a0; do
   echo "################ 0x$a ################"
   b=$(body "$a")
   n=$(echo "$b" | grep -cE '^    [0-9a-f]+:')
   c=$(echo "$b" | grep -cE 'call   ')
   echo "instructions=$n  calls=$c"
   echo "-- line census:"
   echo "$b" | grep -oE '[a-z_]+\.(hpp|h):[0-9]+' | sort | uniq -c | sort -rn | head -8
   echo "-- inner loops (backward jumps):"
   echo "$b" | grep -E '^    [0-9a-f]+:' | awk '{
      addr = strtonum("0x" substr($1, 1, length($1)-1))
      if($2 ~ /^j/ && match($0, /\+0x[0-9a-f]+>/)) {
         t = substr($0, RSTART+1, RLENGTH-2)
         sub(/0x/, "", t)
      }
   }' 
   echo "$b" | grep -E '^    [0-9a-f]+:\s+(jne|jb|ja|jae|je|jmp)\s' | tail -0
done
