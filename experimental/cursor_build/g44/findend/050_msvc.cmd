@echo off
rem MSVC x86 + x64 compile+run of the new find_end test and its neighbours.
setlocal
set ONE=D:\Data\LocalGit\boost\libs\container\experimental\cursor_build\g21\stop\163_msvc_one.cmd
for %%a in (x86 x64) do (
  for %%t in (segmented_find_end_test segmented_search_test segmented_mismatch_test) do (
    cmd /c "%ONE%" %%a %%t
  )
)
