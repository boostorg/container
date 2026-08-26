@echo off
rem MSVC 18 x86 and x64 over the eight tests plus the leaf-contract probe.
rem Every cl.exe gets a fresh cmd, so vcvarsall never runs twice in a process.
setlocal
set HERE=%~dp0
set STOP=D:\Data\LocalGit\boost\libs\container\experimental\cursor_build\g21\stop
for %%A in (x86 x64) do (
   for %%T in (set_union set_difference set_intersection set_symmetric_difference ^
               merge copy partition_copy partition) do (
      call "%STOP%\163_msvc_one.cmd" %%A segmented_%%T_test
   )
   call "%HERE%253_msvc_one.cmd" %%A experimental\cursor_build\g25\verify\250_contract
)
