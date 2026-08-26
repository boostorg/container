#!/bin/bash
# Shows where the conditional selects sit in the entry-point TUs.
set -u
D=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g44/findend/out/totals
for v in "$@"; do
  echo "=================== $v"
  awk '/^[0-9a-f]+ <.*>:$/{sym=$2} /cmov/{print sym; print "   " $0}' "$D/$v.o.dis" | cut -c1-140
done
