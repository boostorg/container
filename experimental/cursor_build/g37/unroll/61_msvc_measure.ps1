# Runs one cl.exe compilation and reports wall time and peak working set.
# Args: <outdir> <logfile> <cl arguments...>
param([Parameter(Mandatory=$true)][string]$OutDir,
      [Parameter(Mandatory=$true)][string]$LogFile,
      [Parameter(ValueFromRemainingArguments=$true)][string[]]$ClArgs)

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = "cl.exe"
$psi.Arguments = ($ClArgs -join ' ')
$psi.WorkingDirectory = $OutDir
$psi.UseShellExecute = $false
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true

# cl.exe spawns a separate compiler-pass process, so the peak has to be taken
# over every cl.exe/c1xx/c2/link alive during the compile, not just the driver.
function Get-TreePeak {
   $m = 0
   foreach ($n in @('cl', 'c1xx', 'c2', 'link')) {
      foreach ($q in (Get-Process -Name $n -ErrorAction SilentlyContinue)) {
         try { if ($q.PeakWorkingSet64 -gt $m) { $m = $q.PeakWorkingSet64 } } catch { }
      }
   }
   return $m
}

$sw = [System.Diagnostics.Stopwatch]::StartNew()
$p = [System.Diagnostics.Process]::Start($psi)
$peak = 0
$outTask = $p.StandardOutput.ReadToEndAsync()
$errTask = $p.StandardError.ReadToEndAsync()
while (-not $p.HasExited) {
   $cur = Get-TreePeak
   if ($cur -gt $peak) { $peak = $cur }
   Start-Sleep -Milliseconds 25
}
$cur = Get-TreePeak
if ($cur -gt $peak) { $peak = $cur }
$p.WaitForExit()
$sw.Stop()
$out = $outTask.Result + $errTask.Result
Set-Content -Path $LogFile -Value $out
$mb = [math]::Round($peak / 1MB, 1)
$sec = [math]::Round($sw.Elapsed.TotalSeconds, 2)
Write-Output ("MSVCSTAT rc={0} sec={1} peakMB={2}" -f $p.ExitCode, $sec, $mb)
