# Scan the MSVC listing for walkers whose post-leaf re-test survived, i.e. a
# basic block that compares against the SAME bounds the leaf loop just used and
# only then advances the segment.  Reports each walker's size and how many
# compares sit on the "advance to next segment" path.
param([string]$Asm)

$lines = Get-Content $Asm
$procs = $lines | Select-String -Pattern '^\?\??\$?[A-Za-z_].*\sPROC$'

$rows = @()
foreach ($p in $procs) {
   $name = ($p.Line -split '\s+')[0]
   # demangled hint is in the trailing comment of the PROC line
   $hint = if ($p.Line -match ';\s*(.+)$') { $matches[1] } else { $name }
   $short = $hint -replace '.*detail_algo::','' -replace '[(<].*',''
   if ($short -notmatch '_bounded|_dispatch|_scan|until_exhausts|_leaf') { continue }

   $start = $p.LineNumber
   $end = $start
   while ($end -lt $lines.Count -and $lines[$end] -notmatch '\sENDP') { $end++ }
   $body = $lines[($start)..($end)] | Where-Object { $_ -match '^\s+\w' -and $_ -notmatch '^\s*;' }

   $insns = ($body | Where-Object { $_ -match '^\s+(mov|cmp|add|sub|lea|j|test|xor|ret|push|pop|npad|movz|movu|set|inc|dec|imul|shr|sar|and|or)' }).Count
   # blocks that end in a segment advance ("add rXX, 8" then jmp back)
   $adv = 0; $advcmp = 0
   for ($i = 0; $i -lt $body.Count; $i++) {
      if ($body[$i] -match '^\s+add\s+r\w+,\s*8\s*$') {
         $adv++
         # count cmp/test in the 8 instructions before it, within the same block
         for ($k = [Math]::Max(0,$i-8); $k -lt $i; $k++) {
            if ($body[$k] -match '^\s+(cmp|test)\s') { $advcmp++ }
            if ($body[$k] -match '^\$LN|^\$LL') { $advcmp = 0 }
         }
      }
   }
   $rows += [pscustomobject]@{ Walker=$short; Insns=$insns; SegAdvances=$adv; CmpBeforeAdvance=$advcmp }
}
$rows | Sort-Object -Property CmpBeforeAdvance -Descending | Format-Table -AutoSize
