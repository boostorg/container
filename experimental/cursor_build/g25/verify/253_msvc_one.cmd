@echo off
rem Single MSVC configuration: %1 = arch (x86/x64), %2 = source path relative to
rem the container library root, without extension.  Repeated vcvarsall calls in
rem one process overflow the environment, so each configuration needs a fresh
rem cmd.
setlocal
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set BOOST=D:\Data\LocalGit\boost
set CONT=%BOOST%\libs\container
for %%F in ("%CONT%\%2.cpp") do set BASE=%%~nF
set OUTDIR=%TEMP%\g25_%1_%BASE%
if exist "%OUTDIR%" rd /s /q "%OUTDIR%"
mkdir "%OUTDIR%"
echo ==================== %1  %BASE% ====================
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" %1 >nul 2>&1
pushd "%OUTDIR%"
cl /nologo /EHsc /O2 /DNDEBUG /W4 /std:c++17 ^
   /I"%BOOST%" /I"%CONT%\experimental" ^
   "%CONT%\%2.cpp" /Fe:t.exe
if errorlevel 1 (echo RESULT: COMPILE FAILED) else (
   t.exe && echo RESULT: PASS || echo RESULT: FAIL
)
popd
