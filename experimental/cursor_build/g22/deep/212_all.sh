#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g22/deep
EX=$BR/libs/container/experimental
O=/tmp/g22all
mkdir -p $O

NAMES=(x all_of any_of none_of count count_if equal fill fill_n find find_if \
       find_if_not find_last find_last_if find_last_if_not for_each generate \
       generate_n is_partitioned is_sorted is_sorted_until mismatch partition \
       partition_point remove remove_if replace replace_if reverse search \
       search_n stable_partition)

printf "%-22s %-8s %-8s\n" algorithm depth1 depth2
printf "%-22s %-8s %-8s\n" "---------------------" "-------" "-------"
bad=0
for i in $(seq 1 31); do
   res=()
   for d in 1 2; do
      if g++-16 -std=c++20 -O1 -DNDEBUG -DALGO=$i -DDEPTH=$d -I$BR -I$EX \
            -c $S/211_all.cpp -o $O/a.o 2>$O/e.$i.$d.log; then
         res+=("ok")
      else
         res+=("FAIL"); bad=$((bad+1))
      fi
   done
   printf "%-22s %-8s %-8s\n" "${NAMES[$i]}" "${res[0]}" "${res[1]}"
done
echo
echo "failures: $bad"
for i in $(seq 1 31); do
   if grep -q 'error:' $O/e.$i.2.log 2>/dev/null; then
      echo "--- ${NAMES[$i]} (depth 2)"
      grep -m1 -A3 'error:' $O/e.$i.2.log | sed 's/^/    /'
   fi
done
