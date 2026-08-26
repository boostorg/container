#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
O=/tmp/g29smoke
mkdir -p $O
cd $EX || exit 1

# One representative test per converted walker class, g++ and clang, c++20 only.
TESTS="segmented_fill_test segmented_count_test segmented_find_test \
       segmented_find_last_test segmented_copy_test segmented_transform_test \
       segmented_partition_test segmented_search_n_test segmented_swap_ranges_test \
       segmented_merge_test segmented_set_union_test segmented_partition_point_test"

fail=0
for t in $TESTS; do
   [ -f $t.cpp ] || { printf "  %-38s MISSING\n" "$t"; continue; }
   for CXX in g++-16 clang++-22; do
      if $CXX -std=c++20 -O2 -DNDEBUG -I$BR -Wall -Wextra $t.cpp -o $O/$t.$CXX.elf 2>$O/$t.$CXX.log; then
         if $O/$t.$CXX.elf >/dev/null 2>&1; then
            w=$(grep -c 'warning:' $O/$t.$CXX.log)
            printf "  %-38s %-12s PASS warnings=%s\n" "$t" "$CXX" "$w"
         else
            printf "  %-38s %-12s RUN FAIL\n" "$t" "$CXX"; fail=1
         fi
      else
         printf "  %-38s %-12s BUILD FAIL\n" "$t" "$CXX"; fail=1
         grep -m2 'error:' $O/$t.$CXX.log | sed 's/^/      /'
      fi
   done
done
echo
[ $fail -eq 0 ] && echo "SMOKE: ALL OK" || echo "SMOKE: FAILURES PRESENT"
