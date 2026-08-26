@echo off
rem Compile the equal/mismatch/search tests with MSVC, 32- and 64-bit, after the
rem stop-flag change. Watches for C1002 heap exhaustion, which this family has
rem hit before.
setlocal
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set BOOST=D:\Data\LocalGit\boost
set CONT=%BOOST%\libs\container

for %%A in (x86 x64) do (
   for %%T in (segmented_mismatch_test segmented_equal_test segmented_search_test) do (
      call :one %%A %%T
   )
)
goto :eof

:one
set ARCH=%1
set TST=%2
set OUTDIR=%TEMP%\sf_%ARCH%_%TST%
if exist "%OUTDIR%" rd /s /q "%OUTDIR%"
mkdir "%OUTDIR%"
echo ==================== %ARCH%  %TST% ====================
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" %ARCH% >nul 2>&1
pushd "%OUTDIR%"
cl /nologo /EHsc /O2 /DNDEBUG /W4 /std:c++17 ^
   /I"%BOOST%" /I"%CONT%\experimental" ^
   "%CONT%\experimental\%TST%.cpp" /Fe:t.exe
if errorlevel 1 (echo *** COMPILE FAILED ***) else (
   t.exe && echo test: PASS || echo test: FAIL
)
popd
goto :eof
