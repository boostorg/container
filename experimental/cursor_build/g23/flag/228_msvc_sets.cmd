@echo off
rem The four set_* tests against the recommended shadow headers.  The shadow
rem path is passed unquoted: cmd /c strips the outer quote pair when both the
rem program and a later argument are quoted.
setlocal
set H=%~dp0
for %%A in (x86 x64) do (
   for %%T in (segmented_set_union_test segmented_set_difference_test segmented_set_intersection_test segmented_set_symmetric_difference_test) do (
      call cmd /c "%H%226_msvc_one.cmd" %%A %%T c++17 %H%shadow_guard
   )
)
