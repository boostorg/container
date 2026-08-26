#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g22/deep
EX=$BR/libs/container/experimental
O=/tmp/g22survey
mkdir -p $O

NAMES=(x copy copy_if copy_n transform remove_copy remove_copy_if reverse_copy \
       swap_ranges merge set_union set_difference set_intersection \
       set_symmetric_difference partition_copy)

printf "%-26s %-10s %-10s\n" "algorithm" "depth1" "depth2"
printf "%-26s %-10s %-10s\n" "--------------------------" "----------" "----------"
for i in $(seq 1 14); do
   res=()
   for d in 1 2; do
      if g++-16 -std=c++20 -O1 -DNDEBUG -DALGO=$i -DDEPTH=$d -I$BR -I$EX \
            -c $S/202_survey.cpp -o $O/a.o 2>$O/e.$i.$d.log; then
         res+=("ok")
      else
         res+=("FAIL")
      fi
   done
   printf "%-26s %-10s %-10s\n" "${NAMES[$i]}" "${res[0]}" "${res[1]}"
done

echo
echo "=================== first error of each depth-2 failure ==================="
for i in $(seq 1 14); do
   if [ -s $O/e.$i.2.log ] && grep -q 'error:' $O/e.$i.2.log; then
      echo "--- ${NAMES[$i]}"
      grep -m1 -A2 'error:' $O/e.$i.2.log | sed 's/^/    /'
   fi
done
