#!/bin/bash
H=/mnt/d/Data/LocalGit/boost/boost/container/experimental
for f in segmented_copy segmented_copy_if segmented_copy_n segmented_partition_copy \
         segmented_remove_copy segmented_remove_copy_if segmented_reverse_copy \
         segmented_swap_ranges segmented_transform segmented_set_union \
         segmented_set_difference segmented_set_intersection \
         segmented_set_symmetric_difference segmented_merge; do
   echo "=== $f"
   grep -nE "^[A-Za-z_].*[^a-z_]$f *\(" $H/$f.hpp | head -6
done
