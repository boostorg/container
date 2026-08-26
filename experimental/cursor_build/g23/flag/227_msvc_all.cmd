@echo off
rem Every cl.exe invocation in a fresh process; each 226 call is its own cmd.
setlocal
set H=%~dp0
set SH=%H%shadow_guard

for %%A in (x86 x64) do (
   for %%S in (c++17 c++20) do (
      call cmd /c "%H%226_msvc_one.cmd" %%A segmented_partition_copy_test %%S
      call cmd /c "%H%226_msvc_one.cmd" %%A segmented_partition_test %%S
   )
)

for %%A in (x86 x64) do (
   for %%T in (segmented_set_union_test segmented_set_difference_test segmented_set_intersection_test segmented_set_symmetric_difference_test) do (
      call cmd /c "%H%226_msvc_one.cmd" %%A %%T c++17 "%SH%"
   )
)
