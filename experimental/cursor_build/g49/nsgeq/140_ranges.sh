#!/bin/bash
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq

pr() { # lo hi title
   echo "===== $3 ====="
   awk -v lo="$1" -v hi="$2" '
      /^    [0-9a-f]+:/ {
         split($0, p, ":")
         gsub(/ /, "", p[1])
         a = strtonum("0x" p[1])
         if(a >= strtonum(lo) && a <= strtonum(hi)) print
      }' bench_gcc.asm | sed -E 's/<unsigned long measure_batch[^>]*\+/<mb+/'
}

pr 0x52c0 0x5335 "nsg counted leaf loop (mismatch_2r D+D)"
pr 0x6aa0 0x6b10 "std::mismatch loop (D+D)"
