#!/bin/bash
# Runs each bench binary matching $1 (default all) LAUNCHES times, pinned to
# core 3, elevated when possible.  Logs to out/bench/<name>.runN.txt.
set -u
BR=/mnt/d/Data/LocalGit/boost
G=$BR/libs/container/experimental/cursor_build/g28/pattern
BIN=$G/out/bin
RUNS=$G/out/bench
mkdir -p $RUNS
PAT=${1:-bench.}
LAUNCHES=${LAUNCHES:-5}

if sudo -n true 2>/dev/null; then
   PIN="sudo -n chrt -f 90 taskset -c 3"
else
   PIN="nice -n -5 taskset -c 3"
fi
echo "pin command: $PIN"

for b in $BIN/$PAT*; do
   case $b in *.log) continue;; esac
   name=$(basename $b)
   for i in $(seq 1 $LAUNCHES); do
      $PIN $b > $RUNS/$name.run$i.txt 2>&1
      echo "done $name run$i"
   done
done
