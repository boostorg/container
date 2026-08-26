@echo off
rem Single configuration MSVC check: %1 = arch (x86/x64), %2 = test name.
rem Repeated vcvarsall calls in one process overflow the environment, so each
rem configuration must run in a fresh cmd.
setlocal
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set BOOST=D:\Data\LocalGit\boost
set CONT=%BOOST%\libs\container
set OUTDIR=%TEMP%\sf1_%1_%2
if exist "%OUTDIR%" rd /s /q "%OUTDIR%"
mkdir "%OUTDIR%"
echo ==================== %1  %2 ====================
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" %1 >nul 2>&1
pushd "%OUTDIR%"
cl /nologo /EHsc /O2 /DNDEBUG /W4 /std:c++17 ^
   /I"%BOOST%" /I"%CONT%\experimental" ^
   "%CONT%\experimental\%2.cpp" /Fe:t.exe
if errorlevel 1 (echo RESULT: COMPILE FAILED) else (
   t.exe && echo RESULT: PASS || echo RESULT: FAIL
)
popd
