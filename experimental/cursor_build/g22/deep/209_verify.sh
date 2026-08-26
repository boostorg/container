#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
O=/tmp/g22v
mkdir -p $O
cd $EX || exit 1

TESTS="segmented_set_union_test segmented_set_difference_test \
segmented_set_intersection_test segmented_set_symmetric_difference_test \
segmented_merge_test segmented_partition_copy_test segmented_copy_test \
segmented_copy_if_test segmented_copy_n_test segmented_transform_test \
segmented_remove_copy_test segmented_remove_copy_if_test \
segmented_reverse_copy_test segmented_swap_ranges_test"

echo "############### standards sweep (all affected + neighbours) ###############"
fails=0
for t in $TESTS; do
   line=$(printf "  %-44s" "$t")
   for CXX in g++-16 clang++-22; do
      for std in c++03 c++11 c++17 c++20; do
         if $CXX -std=$std -O2 -DNDEBUG -I$BR -Wall -Wextra $t.cpp -o $O/x.elf 2>$O/x.log; then
            w=$(grep -c 'warning:' $O/x.log)
            if $O/x.elf >/dev/null 2>&1; then
               [ "$w" -eq 0 ] && line="$line ." || { line="$line W"; fails=$((fails+1)); }
            else
               line="$line F"; fails=$((fails+1))
            fi
         else
            line="$line B"; fails=$((fails+1))
         fi
      done
      line="$line "
   done
   echo "$line"
done
echo "  legend: 8 slots = gcc c++03/11/17/20 then clang c++03/11/17/20"
echo "          . pass+no warnings   W warnings   F run fail   B build fail"
echo "  total problem slots: $fails"

echo
echo "####################### sanitisers (c++20, asan+ubsan) #######################"
for t in segmented_set_union_test segmented_set_difference_test \
         segmented_set_intersection_test segmented_set_symmetric_difference_test \
         segmented_merge_test; do
   for CXX in g++-16 clang++-22; do
      if $CXX -std=c++20 -O1 -g -fsanitize=address,undefined -I$BR $t.cpp -o $O/s.elf 2>$O/s.log; then
         if $O/s.elf >/dev/null 2>&1; then
            printf "  %-12s %-44s clean\n" "$CXX" "$t"
         else
            printf "  %-12s %-44s SANITISER/TEST FAILURE\n" "$CXX" "$t"
         fi
      else
         printf "  %-12s %-44s build fail\n" "$CXX" "$t"
      fi
   done
done
