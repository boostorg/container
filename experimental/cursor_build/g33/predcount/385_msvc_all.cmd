@echo off
rem MSVC x64 compile+run for every test file carrying a new count test.
rem One fresh cl.exe process per test, as 163_msvc_one.cmd requires.
setlocal
set ONE=D:\Data\LocalGit\boost\libs\container\experimental\cursor_build\g21\stop\163_msvc_one.cmd
for %%t in (
  segmented_all_of_test
  segmented_any_of_test
  segmented_copy_if_test
  segmented_count_if_test
  segmented_count_test
  segmented_equal_test
  segmented_fill_n_test
  segmented_fill_test
  segmented_find_if_not_test
  segmented_find_if_test
  segmented_find_last_if_not_test
  segmented_find_last_if_test
  segmented_find_last_test
  segmented_find_test
  segmented_for_each_test
  segmented_is_partitioned_test
  segmented_is_sorted_test
  segmented_is_sorted_until_test
  segmented_merge_test
  segmented_mismatch_test
  segmented_none_of_test
  segmented_partition_copy_test
  segmented_partition_point_test
  segmented_partition_test
  segmented_remove_copy_if_test
  segmented_remove_copy_test
  segmented_remove_if_test
  segmented_remove_test
  segmented_replace_if_test
  segmented_replace_test
  segmented_search_n_test
  segmented_search_test
  segmented_set_difference_test
  segmented_set_intersection_test
  segmented_set_symmetric_difference_test
  segmented_set_union_test
  segmented_stable_partition_test
  segmented_swap_ranges_test
  segmented_transform_test
) do (
  cmd /c "%ONE%" x64 %%t
)
