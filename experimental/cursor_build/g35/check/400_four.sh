#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
O=/tmp/g35
mkdir -p $O
cd $EX || exit 1

for t in segmented_partition_test segmented_is_partitioned_test \
         segmented_search_n_test segmented_search_test; do
   for CXX in g++-16 clang++-22; do
      if $CXX -std=c++20 -O2 -DNDEBUG -I$BR -I$EX -Wall -Wextra $t.cpp -o $O/$t.$CXX.elf 2>$O/$t.$CXX.log; then
         $O/$t.$CXX.elf > $O/$t.$CXX.out 2>&1
         rc=$?
         # count how many reported errors mention a count/application bound
         errs=$(grep -c -iE 'error|fail' $O/$t.$CXX.out)
         printf "  %-34s %-12s exit=%-3s reported_errors=%s\n" "$t" "$CXX" "$rc" "$errs"
      else
         printf "  %-34s %-12s BUILD FAIL\n" "$t" "$CXX"
         grep -m2 'error:' $O/$t.$CXX.log | sed 's/^/      /'
      fi
   done
done

echo
echo "=== sample of the count assertions that fail (gcc, partition + search) ==="
grep -iE 'calls|count|applic' $O/segmented_partition_test.g++-16.out 2>/dev/null | head -6
grep -iE 'calls|count|comparis' $O/segmented_search_test.g++-16.out 2>/dev/null | head -6

echo
echo "=== controls: two algorithms expected to PASS their exact-count tests ==="
for t in segmented_copy_if_test segmented_partition_copy_test; do
   g++-16 -std=c++20 -O2 -DNDEBUG -I$BR -I$EX -Wall -Wextra $t.cpp -o $O/$t.elf 2>/dev/null \
      && { $O/$t.elf >/dev/null 2>&1; printf "  %-34s exit=%s\n" "$t" "$?"; } \
      || printf "  %-34s BUILD FAIL\n" "$t"
done
