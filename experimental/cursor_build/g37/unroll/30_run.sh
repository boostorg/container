#!/bin/bash
# Run a set of benchmark binaries pinned to one core, N launches each.
set -u
W=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g37/unroll
BIN=$W/bin
R=$W/runs
mkdir -p $R

GROUP=${GROUP:-25}
N=${N:-5}
CORE=${CORE:-3}
VARIANTS=${VARIANTS:-"base p2_copyif p2_rc p2_rci p2_all p2ci_p1 p1_cleanup"}

# highest scheduling class we are allowed to take (nice/chrt need CAP_SYS_NICE,
# which is not available here, so this normally falls back to plain pinning)
PRE="taskset -c $CORE"
if chrt -f 50 true 2>&1 | grep -qv . ; then
   PRE="chrt -f 50 taskset -c $CORE"
elif nice -n -20 true 2>&1 | grep -qv . ; then
   PRE="nice -n -20 taskset -c $CORE"
fi
echo "launcher: $PRE"

for cc in g++-16 clang++-22; do
  for v in $VARIANTS; do
    b=$BIN/b_${cc}_${v}_g${GROUP}
    [ -x "$b" ] || { echo "missing $b"; continue; }
    for i in $(seq 1 $N); do
      $PRE $b > $R/${cc}_${v}_g${GROUP}_r$i.txt 2>&1
      echo "ran $cc $v g$GROUP run $i"
    done
  done
done
