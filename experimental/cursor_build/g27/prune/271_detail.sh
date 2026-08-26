#!/bin/bash
EXP=/mnt/d/Data/LocalGit/boost/libs/container/experimental
H=$EXP/segmented_test_helper.hpp

for e in make_dest_range shape_feasible seg_value_of make_range for_each_shape_all_fwd for_each_shape3_all flatten_ints for_each_dest_shape_all; do
  echo "===== $e ====="
  grep -n "\b$e\b" $EXP/*.cpp | sed "s|$EXP/||"
done

echo "===== make_sentinel : per-file counts ====="
for f in $EXP/*.cpp; do
  n=$(grep -c "\bmake_sentinel\b" $f)
  [ "$n" != "0" ] && printf "%-50s %s\n" "$(basename $f)" "$n"
done

echo "===== candidates: dead names, any occurrence anywhere in experimental (incl cursor_build) ====="
for e in for_each_shape for_each_shape2 for_each_shape3 for_each_shape_fwd for_each_shape_cat shape_core_families make_sized_sentinel sized_sentinel_wrapper; do
  echo "--- $e ---"
  grep -rn "\b$e\b" $EXP --include=*.cpp --include=*.hpp | sed "s|$EXP/||" | grep -v "^segmented_test_helper.hpp" || echo "  (none outside helper)"
done

echo "===== sentinel_wrapper refs in helper ====="
grep -n "\bsentinel_wrapper\b" $H
echo "===== sized_sentinel_wrapper refs in helper ====="
grep -n "\bsized_sentinel_wrapper\b" $H
echo "===== jamfile mentions of bench/probe cpp using helper ====="
grep -rn "segmented_test_helper" $EXP/Jamfile* 2>/dev/null || echo none
ls $EXP/Jamfile* 2>/dev/null || echo "no jamfile"
