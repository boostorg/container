#!/bin/bash
set -u
D=/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g21/stop
tr -d '\r' < $D/173_recheck.sh > /tmp/a.sh
tr -d '\r' < $D/174_tests.sh   > /tmp/b.sh
bash /tmp/a.sh 2>&1 | grep -v run_exit
echo '--- tests ---'
bash /tmp/b.sh 2>&1
