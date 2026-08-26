@echo off
rem g28 MSVC check against a shadow variant: %1 = arch (x86/x64),
rem %2 = test basename (e.g. segmented_fill_test), %3 = variant (E/T/F/W).
rem Fresh cmd per configuration (vcvarsall overflows env if reused).
setlocal
set VS=C:\Program Files\Microsoft Visual Studio\18\Community
set BOOST=D:\Data\LocalGit\boost
set CONT=%BOOST%\libs\container
set SHADOW=%CONT%\experimental\cursor_build\g28\pattern\shadow_%3
set OUTDIR=%TEMP%\g28_%1_%2_%3
if exist "%OUTDIR%" rd /s /q "%OUTDIR%"
mkdir "%OUTDIR%"
echo ==================== %1  %2  shadow_%3 ====================
call "%VS%\VC\Auxiliary\Build\vcvarsall.bat" %1 >nul 2>&1
pushd "%OUTDIR%"
cl /nologo /EHsc /O2 /DNDEBUG /DBOOST_CONTAINER_G28_EXPECT_SHADOW /W4 /std:c++17 ^
   /I"%SHADOW%" /I"%BOOST%" /I"%CONT%\experimental" ^
   "%CONT%\experimental\%2.cpp" /Fe:t.exe
if errorlevel 1 (echo RESULT: COMPILE FAILED) else (
   t.exe && echo RESULT: PASS || echo RESULT: FAIL
)
popd
