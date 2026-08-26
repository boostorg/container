#!/bin/bash
H=/mnt/d/Data/LocalGit/boost/boost/container/experimental
for a in set_union set_difference set_intersection set_symmetric_difference merge; do
   echo "=================== $a ==================="
   echo "--- until_exhausts segmented-dst walker (stop condition + shape) ---"
   awk -v n="${a}_until_exhausts" '
      $0 ~ n && $0 ~ /segmented_iterator_tag/ { found=1 }
      /segmented_iterator_tag &,/ { inseg=1 }
      { buf[NR]=$0 }
      END{}' $H/${a}.hpp >/dev/null
   grep -nE "segmented_iterator_tag" $H/${a}.hpp | head -8
   echo "--- the SEG_UNLIKELY stop tests in this header ---"
   grep -nE "SEG_UNLIKELY\(|BOOST_UNLIKELY\(" $H/${a}.hpp | head -14
done
