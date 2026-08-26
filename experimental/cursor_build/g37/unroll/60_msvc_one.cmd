@echo off
rem Single configuration MSVC check for the g37 unroll.
rem %1 = arch (x64), %2 = test name, %3 = variant (base|p2_all|p2_all4), %4 = std (c++17/c++20)
rem Repeated vcvarsall calls in one process overflow the environment, so each
rem configuration must run in a fresh cmd.
setlocal
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set BOOST=D:\Data\LocalGit\boost
set CONT=%BOOST%\libs\container
set W=%CONT%\experimental\cursor_build\g37\unroll
set OUTDIR=%TEMP%\g37_%1_%2_%3_%4
if exist "%OUTDIR%" rd /s /q "%OUTDIR%"
mkdir "%OUTDIR%"
echo ==================== %1 %2 %3 %4 ====================
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" %1 >nul 2>&1
if "%3"=="base" (set INC=/I"%W%\snap") else (set INC=/I"%W%\exp\%3" /I"%W%\snap")
powershell -NoProfile -ExecutionPolicy Bypass -File "%W%\61_msvc_measure.ps1" ^
  "%OUTDIR%" "%OUTDIR%\cl.log" ^
  /nologo /EHsc /O2 /DNDEBUG /W4 /std:%4 /Bt+ ^
  %INC% /I"%BOOST%" /I"%W%\tests" ^
  "%W%\tests\%2.cpp" /Fe:t.exe
findstr /C:"error" /C:"warning" /C:"C1002" /C:"heap" "%OUTDIR%\cl.log"
findstr /C:"time(" "%OUTDIR%\cl.log"
if exist "%OUTDIR%\t.exe" (
   for %%A in ("%OUTDIR%\t.exe") do echo EXESIZE=%%~zA
   pushd "%OUTDIR%"
   t.exe && echo RESULT: PASS || echo RESULT: FAIL
   popd
) else (
   echo RESULT: COMPILE FAILED
)
