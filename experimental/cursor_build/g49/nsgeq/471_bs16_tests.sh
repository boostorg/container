#!/bin/bash
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental
fail=0
for t in segmented_copy_if_test segmented_remove_copy_if_test segmented_remove_copy_test segmented_partition_copy_test; do
   g++-16 -std=c++20 -O2 -I../../.. -I. "$t.cpp" -o "/tmp/$t.elf" 2>/tmp/e.txt \
      && "/tmp/$t.elf" >/dev/null && echo "PASS $t" \
      || { echo "FAIL $t"; head -20 /tmp/e.txt; fail=1; }
done
exit $fail
