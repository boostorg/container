#!/bin/bash
# Per measure_batch symbol: does its deque walk use the promoted (lea 0x200)
# block-end form or the unpromoted (load *m_node each element) form?
set -u
cd /mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g49/nsgeq

awk '
   /^0[0-9a-f]+ <unsigned long measure_batch</ {
      if(name != "") printf "%-11s lea(promoted)=%-3d  load+add(unpromoted)=%d\n", tag, lea, lad
      name = $0; lea = 0; lad = 0
      tag = "other"
      if(name ~ /std_equal_2r/)       tag = "std_eq2r"
      else if(name ~ /seg_equal_2r/)  tag = (name ~ /true>\(/) ? "nsg_eq2r" : "seg_eq2r"
      else if(name ~ /std_equal/)     tag = "std_eq"
      else if(name ~ /seg_equal/)     tag = (name ~ /true>\(/) ? "nsg_eq" : "seg_eq"
      else if(name ~ /std_mismatch_2r/) tag = "std_mm2r"
      else if(name ~ /seg_mismatch_2r/) tag = (name ~ /true>\(/) ? "nsg_mm2r" : "seg_mm2r"
      else if(name ~ /std_mismatch/)  tag = "std_mm"
      else if(name ~ /seg_mismatch/)  tag = (name ~ /true>\(/) ? "nsg_mm" : "seg_mm"
      else if(name ~ /std_search/)    tag = "std_search"
      else if(name ~ /seg_search/)    tag = (name ~ /true>\(/) ? "nsg_search" : "seg_search"
   }
   name != "" && /lea    0x200\(/ { lea++ }
   name != "" && /mov    \(%r[a-z0-9]+\),%r[a-z0-9]+$/ { lad++ }
   END { if(name != "") printf "%-11s lea(promoted)=%-3d  load+add(unpromoted)=%d\n", tag, lea, lad }
' bench_clang.asm | grep -v "^other"
