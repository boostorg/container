@echo off
rem MSVC x64 and x86 build+run of segmented_search_test: watch for C1002.
set HERE=d:\Data\LocalGit\boost\libs\container\experimental\cursor_build\g45\ownsearch
set ROOT=d:\Data\LocalGit\boost
set EXP=%ROOT%\libs\container\experimental
set VC=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build
pushd "%HERE%"

for %%A in (64 32) do (
  echo === MSVC x%%A
  setlocal
  if "%%A"=="64" (call "%VC%\vcvars64.bat" >nul) else (call "%VC%\vcvars32.bat" >nul)
  for %%S in (14 17 20) do (
    cl /nologo /O2 /DNDEBUG /EHsc /W4 /std:c++%%S ^
       /I"%ROOT%" /I"%EXP%" ^
       "%EXP%\segmented_search_test.cpp" /Fe:ms%%A_%%S.exe /Fo:ms%%A_%%S.obj >ms%%A_%%S.log 2>&1
    if errorlevel 1 (
       echo   c++%%S BUILD-FAIL
       findstr /C:"C1002" /C:"error" ms%%A_%%S.log
    ) else (
       ms%%A_%%S.exe >nul 2>&1
       if errorlevel 1 (echo   c++%%S RUN-FAIL) else (echo   c++%%S PASS)
       findstr /C:"C1002" /C:"warning" ms%%A_%%S.log
    )
  )
  endlocal
)
popd
