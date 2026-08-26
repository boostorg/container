# Show the "advance to the next segment" blocks of one MSVC walker, so we can see
# whether the walker's post-leaf re-test survived or was jump-threaded away.
param([string]$Asm, [string]$Name, [int]$Ctx = 10)

$lines = Get-Content $Asm
$idx = -1
for ($i = 0; $i -lt $lines.Count; $i++) {
   if ($lines[$i].Contains($Name) -and $lines[$i] -match '\sPROC') { $idx = $i; break }
}
if ($idx -lt 0) { "walker not found: $Name"; exit }

$end = $idx + 1
while ($end -lt $lines.Count -and $lines[$end] -notmatch '\sENDP') { $end++ }
$body = $lines[($idx + 1)..$end] |
   Where-Object { $_ -notmatch '^\s*;' -and $_.Trim() -ne '' } |
   ForEach-Object { $_ -replace '\s*;.*$','' }

"=== $Name : $($body.Count) insn/label lines ==="
$found = $false
for ($i = 0; $i -lt $body.Count; $i++) {
   if ($body[$i] -match '^\s+(add|sub)\s+r\w+,\s*8\s*$') {
      $found = $true
      $lo = [Math]::Max(0, $i - $Ctx); $hi = [Math]::Min($body.Count - 1, $i + 3)
      "--- segment step at line $i ---"
      $body[$lo..$hi]
   }
}
if (-not $found) { "(no 'add rXX,8' segment step found)" }
