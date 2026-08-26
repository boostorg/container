#!/bin/bash
# Reference count of the helper's public API, in the working tree and at HEAD,
# so an entity that only became unreferenced now can be told from one that was
# already unreferenced before.
set -u
EX=/mnt/d/Data/LocalGit/boost/libs/container/experimental
HEADDIR=/tmp/g25head
rm -rf $HEADDIR; mkdir -p $HEADDIR

API="make_range make_dest_range iter_at flatten_ints flatten_n_ints flatten_all_ints
     filler_intact seg_vector seg2_vector movable_int sentinel_wrapper
     sized_sentinel_wrapper make_sentinel make_sized_sentinel
     for_each_shape for_each_shape_cat for_each_shape_fwd
     for_each_shape_all for_each_shape_all_cat for_each_shape_all_fwd
     for_each_shape2 for_each_shape2_all for_each_shape3 for_each_shape3_all
     for_each_dest_shape_all
     max_shape_depth shape_specs shape_specs_empty shape_specs_family
     shape_core_families shape_all_families shape_feasible with_shape
     seg_split_point seg_value_of total_size"

printf "%-26s %-6s %-6s\n" ENTITY tree HEAD
for n in $API; do
   t=$(grep -lw "$n" $EX/*_test.cpp 2>/dev/null | wc -l)
   printf "%-26s %-6s" "$n" "$t"
   h=$(cd /mnt/d/Data/LocalGit/boost/libs/container && \
       git grep -lw "$n" HEAD -- 'experimental/*_test.cpp' 2>/dev/null | wc -l)
   printf " %-6s%s\n" "$h" "$( [ "$t" = 0 ] && [ "$h" != 0 ] && echo '   <-- became unreferenced' )"
done
