#!/bin/bash
set -u
W=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g37/unroll
P=$W/40_parse.py
tr -d '\r' < $P > /tmp/g37parse.py
R=$W/runs
{
echo "###################### copy_if leaf: base -> p2_copyif ######################"
ROWS='^copy_if' python3 /tmp/g37parse.py 25 base p2_copyif
echo
echo "###################### remove_copy leaf: base -> p2_rc ######################"
ROWS='^remove_copy\(' python3 /tmp/g37parse.py 25 base p2_rc
echo
echo "#################### remove_copy_if leaf: base -> p2_rci ####################"
ROWS='^remove_copy_if' python3 /tmp/g37parse.py 25 base p2_rci
echo
echo "################## all three leaves: base -> p2_all (full group) ############"
python3 /tmp/g37parse.py 25 base p2_all
} > $W/runs/REPORT.txt 2>&1
echo "written $W/runs/REPORT.txt ($(wc -l < $W/runs/REPORT.txt) lines)"
