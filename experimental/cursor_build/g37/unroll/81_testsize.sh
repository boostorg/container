#!/bin/bash
# Per-translation-unit code size: the four affected tests, -O2 -DNDEBUG,
# base vs p2_final.  This is the realistic per-user figure, unlike the
# benchmark binary which instantiates the algorithms 12 times over.
set -u
W=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g37/unroll
T=$W/tests
O=$W/bin/sz
mkdir -p $O
printf '%-34s %-11s %10s %10s %8s   %10s %10s %8s\n' test compiler 'text base' 'text new' 'd%' 'file base' 'file new' 'd%'
for t in segmented_copy_if_test segmented_remove_copy_if_test segmented_remove_copy_test segmented_partition_copy_test; do
 for cc in g++-16 clang++-22; do
  for v in base p2_final; do
    if [ "$v" = base ]; then INC="-I$W/snap"; else INC="-I$W/exp/$v -I$W/snap"; fi
    $cc -std=c++20 -O2 -DNDEBUG $INC -I/mnt/d/Data/LocalGit/boost -I$T $T/$t.cpp -o $O/${cc}_${v}_$t 2>/dev/null
  done
  tb=$(size -A $O/${cc}_base_$t     | awk '$1==".text"{print $2}')
  ta=$(size -A $O/${cc}_p2_final_$t | awk '$1==".text"{print $2}')
  fb=$(stat -c %s $O/${cc}_base_$t)
  fa=$(stat -c %s $O/${cc}_p2_final_$t)
  printf '%-34s %-11s %10d %10d %+7.1f   %10d %10d %+7.1f\n' $t $cc $tb $ta \
     $(echo "scale=4; 100*($ta/$tb-1)" | bc) $fb $fa $(echo "scale=4; 100*($fa/$fb-1)" | bc)
 done
done
