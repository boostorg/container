# MSVC 18 x64 /EHsc /O2 /DNDEBUG /W4 /std:c++17, one fresh cl.exe per test.
# Reports wall time and peak working set of the cl.exe process.
$Tests = $args

$VS    = 'C:\Program Files\Microsoft Visual Studio\18\Community'
$BOOST = 'D:\Data\LocalGit\boost'
$CONT  = "$BOOST\libs\container"
$Root  = Join-Path $env:TEMP 'pf39msvc'
if (Test-Path $Root) { Remove-Item -Recurse -Force $Root }
New-Item -ItemType Directory -Path $Root | Out-Null

foreach ($t in $Tests) {
   $out = Join-Path $Root $t
   New-Item -ItemType Directory -Path $out | Out-Null
   $bat = Join-Path $out 'go.cmd'
   $cmd = @"
@echo off
call "$VS\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
cd /d "$out"
cl /nologo /EHsc /O2 /DNDEBUG /W4 /std:c++17 /I"$BOOST" /I"$CONT\experimental" "$CONT\experimental\$t.cpp" /Fe:t.exe
if errorlevel 1 exit 1
exit 0
"@
   Set-Content -Path $bat -Value $cmd -Encoding ASCII

   $sw = [Diagnostics.Stopwatch]::StartNew()
   $p = Start-Process -FilePath 'cmd.exe' -ArgumentList '/c', "`"$bat`"" -NoNewWindow -PassThru `
        -RedirectStandardOutput (Join-Path $out 'cc.log') -RedirectStandardError (Join-Path $out 'cc.err')
   $peak = 0
   while (-not $p.HasExited) {
      foreach ($c in (Get-Process -Name cl -ErrorAction SilentlyContinue)) {
         try { if ($c.PeakWorkingSet64 -gt $peak) { $peak = $c.PeakWorkingSet64 } } catch {}
      }
      Start-Sleep -Milliseconds 100
   }
   $sw.Stop()

   $exe   = Join-Path $out 't.exe'
   $built = Test-Path $exe
   $log   = ((Get-Content (Join-Path $out 'cc.log') -Raw) + (Get-Content (Join-Path $out 'cc.err') -Raw))
   $noise = @($log -split "`r?`n" | Where-Object { $_.Trim() -ne '' -and $_.Trim() -ne "$t.cpp" })
   $status = if (-not $built) { 'COMPILE-FAILED' } elseif ($noise.Count -gt 0) { 'WARNINGS' } else { 'CLEAN' }

   $runres = 'skipped'
   if ($built) {
      $r = Start-Process -FilePath $exe -NoNewWindow -PassThru -Wait `
           -RedirectStandardOutput (Join-Path $out 'run.log') -RedirectStandardError (Join-Path $out 'run.err')
      $runres = if ($r.ExitCode -eq 0) { 'PASS' } else { "FAIL($($r.ExitCode))" }
   }

   '{0,-34} {1,-14} run={2,-9} cc={3,6:N1}s peakWS={4,6:N0}MB' -f `
      $t, $status, $runres, $sw.Elapsed.TotalSeconds, ($peak / 1MB)
   if ($noise.Count -gt 0) { $noise | Select-Object -First 20 | ForEach-Object { "    | $_" } }
   if ($runres -ne 'PASS' -and $built) {
      Get-Content (Join-Path $out 'run.log') -TotalCount 20 | ForEach-Object { "    > $_" }
   }
}
