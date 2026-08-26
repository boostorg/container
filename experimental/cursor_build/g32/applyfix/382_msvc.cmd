@echo off
rem MSVC 18 x64 compile+run for the tests affected by the leaf reorder.
setlocal
set H=D:\Data\LocalGit\boost\libs\container\experimental\cursor_build\g21\stop\163_msvc_one.cmd
for %%T in (segmented_copy_if_test segmented_remove_copy_test segmented_remove_copy_if_test segmented_remove_test segmented_remove_if_test segmented_partition_copy_test segmented_copy_test) do (
   cmd /c "%H%" x64 %%T
)
echo DONE-MSVC
