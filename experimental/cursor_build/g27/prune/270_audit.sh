#!/bin/bash
# Reachability audit for segmented_test_helper.hpp
ROOT=/mnt/d/Data/LocalGit/boost/libs/container
EXP=$ROOT/experimental
H=$EXP/segmented_test_helper.hpp

echo "=== .cpp files directly in experimental/ ==="
ls $EXP/*.cpp 2>/dev/null | xargs -n1 basename
echo "=== count of segmented_*_test.cpp ==="
ls $EXP/segmented_*_test.cpp | wc -l
echo "=== bench dir? ==="
ls -d $ROOT/bench 2>/dev/null || echo "no bench dir"
echo "=== any file outside experimental/ including the helper ==="
grep -rl "segmented_test_helper" $ROOT --include=*.cpp --include=*.hpp --include=*.jam --include=*.txt 2>/dev/null | grep -v "/experimental/" || echo "none"
echo "=== files under experimental (non-cursor_build) including the helper ==="
grep -rl "segmented_test_helper" $EXP --include=*.cpp --include=*.hpp 2>/dev/null | grep -v cursor_build

echo
echo "=== helper line count ==="
wc -l $H

ENTS="movable_int seg_vector_iterator seg_vector sentinel_wrapper make_sentinel sized_sentinel_wrapper make_sized_sentinel seg2_vector_iterator seg2_vector seg_split_point shape_feasible make_range make_dest_range iter_at seg_value_of flatten_ints flatten_all_ints flatten_n_ints max_shape_depth shape_specs shape_specs_empty shape_specs_family shape_core_families shape_all_families with_shape for_each_shape_fam_cat for_each_shape_cat for_each_shape for_each_shape_fwd for_each_shape_all_cat for_each_shape_all for_each_shape_all_fwd shape2_bind shape2_outer for_each_shape2_fam for_each_shape2 for_each_shape2_all shape3_bind shape3_outer for_each_shape3_fam for_each_shape3 for_each_shape3_all for_each_dest_shape_all filler_intact"

echo
echo "=== call sites in experimental/*.cpp (word-boundary, excl. cursor_build) ==="
printf "%-28s %s\n" "ENTITY" "SITES"
for e in $ENTS; do
  n=$(grep -ho "\b$e\b" $EXP/*.cpp 2>/dev/null | wc -l)
  printf "%-28s %s\n" "$e" "$n"
done

echo
echo "=== references inside the helper itself, with line numbers (code+comment) ==="
for e in $ENTS; do
  echo "--- $e ---"
  grep -n "\b$e\b" $H
done
