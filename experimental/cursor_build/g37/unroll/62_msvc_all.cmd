@echo off
rem Full MSVC 18 x64 matrix for the g37 unroll: base vs p2_final vs p2_all4,
rem the four affected tests, c++17 and c++20.  Each cl.exe runs in a fresh cmd.
setlocal
set W=D:\Data\LocalGit\boost\libs\container\experimental\cursor_build\g37\unroll
for %%V in (base p2_final p2_all4) do (
  for %%S in (c++17 c++20) do (
    for %%T in (segmented_copy_if_test segmented_remove_copy_if_test segmented_remove_copy_test segmented_partition_copy_test) do (
      cmd /c "%W%\60_msvc_one.cmd" x64 %%T %%V %%S
    )
  )
)
