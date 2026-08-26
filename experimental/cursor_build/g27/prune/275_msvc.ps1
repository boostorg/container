$ErrorActionPreference = 'Continue'
$tests = @(
  'segmented_set_union_test',
  'segmented_set_intersection_test',
  'segmented_set_difference_test',
  'segmented_set_symmetric_difference_test',
  'segmented_copy_test',
  'segmented_merge_test',
  'segmented_partition_copy_test',
  'segmented_find_last_test',
  'segmented_remove_copy_test',
  'segmented_copy_if_test',
  'segmented_fill_test'
)
$script = 'D:\Data\LocalGit\boost\libs\container\experimental\cursor_build\g21\stop\163_msvc_one.cmd'
$log = 'D:\Data\LocalGit\boost\libs\container\experimental\cursor_build\g27\prune\275_msvc_log.txt'
Remove-Item $log -ErrorAction SilentlyContinue
foreach ($arch in @('x86','x64')) {
  foreach ($t in $tests) {
    $out = & cmd /c "`"$script`" $arch $t" 2>&1 | Out-String
    Add-Content $log $out
  }
}
Write-Output "==== RESULT lines ===="
Select-String -Path $log -Pattern 'RESULT:|C1002|error C|warning C|====================' | ForEach-Object { $_.Line }
