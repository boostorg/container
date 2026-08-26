#!/bin/bash
set -u
E=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd "$E"
fail=0
for t in segmented_mismatch_test segmented_equal_test segmented_search_test segmented_find_end_test; do
   for CXX in g++-16 clang++-22; do
      $CXX -std=c++20 -O2 -I../../.. -I. "$t.cpp" -o "/tmp/$t.elf" 2>/tmp/build_err.txt \
         || { echo "BUILDFAIL $CXX $t"; cat /tmp/build_err.txt | head -20; fail=1; continue; }
      "/tmp/$t.elf" >/dev/null 2>&1 && echo "PASS  $CXX $t" || { echo "FAIL  $CXX $t"; fail=1; }
   done
done
# C++03 sanity on one affected header user
g++-16 -std=c++03 -O2 -I../../.. -I. segmented_equal_test.cpp -o /tmp/eq03.elf 2>/dev/null \
   && /tmp/eq03.elf >/dev/null 2>&1 && echo "PASS  g++-16 c++03 equal" || { echo "FAIL  c++03 equal"; fail=1; }
exit $fail
