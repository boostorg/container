#!/bin/bash
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$E"
fail=0
for t in segmented_copy_if_test segmented_remove_copy_if_test segmented_remove_copy_test segmented_partition_copy_test; do
   for CXX in g++-16 clang++-22; do
      $CXX -std=c++20 -O2 -I../../.. -I. "$t.cpp" -o "/tmp/$t.elf" 2>/tmp/err.txt \
         || { echo "BUILDFAIL $CXX $t"; head -30 /tmp/err.txt; fail=1; continue; }
      "/tmp/$t.elf" >/dev/null && echo "PASS $CXX $t" || { echo "FAIL $CXX $t"; fail=1; }
   done
done
exit $fail
