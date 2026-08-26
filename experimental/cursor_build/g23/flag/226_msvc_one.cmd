@echo off
rem Single configuration MSVC check.
rem   %1 = arch (x86/x64), %2 = test basename, %3 = /std: value,
rem   %4 = optional extra include dir searched before the boost root.
rem Repeated vcvarsall calls in one process overflow the environment, so each
rem configuration must run in a fresh cmd.
setlocal
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set BOOST=D:\Data\LocalGit\boost
set CONT=%BOOST%\libs\container
set EXTRA=
if not "%~4"=="" set EXTRA=/I"%~4"
set OUTDIR=%TEMP%\g23_%1_%2_%3
if exist "%OUTDIR%" rd /s /q "%OUTDIR%"
mkdir "%OUTDIR%"
echo ==================== %1  %3  %2 ====================
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" %1 >nul 2>&1
pushd "%OUTDIR%"
cl /nologo /EHsc /O2 /DNDEBUG /W4 /std:%3 ^
   %EXTRA% /I"%BOOST%" /I"%CONT%\experimental" ^
   "%CONT%\experimental\%2.cpp" /Fe:t.exe
if errorlevel 1 (echo RESULT: COMPILE FAILED) else (
   t.exe && echo RESULT: PASS || echo RESULT: FAIL
)
popd
