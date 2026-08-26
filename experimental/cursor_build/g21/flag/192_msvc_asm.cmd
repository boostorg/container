@echo off
rem Emit MSVC assembly for the walker probes, to check whether MSVC also
rem jump-threads the walker's post-leaf re-test the way GCC and Clang do.
rem %1 = probe basename (without .cpp), located in the g21/flag dir.
setlocal
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set BOOST=D:\Data\LocalGit\boost
set SRC=%BOOST%\libs\container\experimental\cursor_build\g21\flag
set OUTDIR=%TEMP%\g21flag_%1
if exist "%OUTDIR%" rd /s /q "%OUTDIR%"
mkdir "%OUTDIR%"
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
pushd "%OUTDIR%"
cl /nologo /c /EHsc /O2 /DNDEBUG /std:c++17 /FA /Fa%1.asm ^
   /I"%BOOST%" "%SRC%\%1.cpp" 1>build.log 2>&1
if errorlevel 1 (
   echo COMPILE FAILED
   type build.log
) else (
   echo OK: %OUTDIR%\%1.asm
   for %%F in (%1.asm) do echo asm bytes: %%~zF
)
popd
