#!/bin/bash
EX=/mnt/d/Data/LocalGit/boost/libs/container/experimental
cd $EX || exit 1
printf "%-46s %6s %6s %6s %6s %6s\n" test lines shape3 _all seg2 shape2
for f in segmented_set_union_test.cpp segmented_set_difference_test.cpp \
         segmented_set_intersection_test.cpp \
         segmented_set_symmetric_difference_test.cpp \
         segmented_merge_test.cpp segmented_partition_copy_test.cpp \
         segmented_copy_test.cpp segmented_copy_if_test.cpp \
         segmented_transform_test.cpp segmented_remove_copy_test.cpp \
         segmented_remove_copy_if_test.cpp segmented_reverse_copy_test.cpp \
         segmented_copy_n_test.cpp segmented_swap_ranges_test.cpp; do
   [ -f "$f" ] || { printf "%-46s MISSING\n" "$f"; continue; }
   printf "%-46s %6s %6s %6s %6s %6s\n" "$f" \
      "$(wc -l < $f)" \
      "$(grep -c 'for_each_shape3' $f)" \
      "$(grep -c 'for_each_shape[0-9]*_all\|_all_cat\|_all_fwd' $f)" \
      "$(grep -c 'seg2' $f)" \
      "$(grep -c 'for_each_shape2' $f)"
done
