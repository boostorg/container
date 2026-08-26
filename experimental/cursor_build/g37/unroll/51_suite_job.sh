#!/bin/bash
# One suite job: $1=cc $2=std $3=test  (INC and OUT come from the environment)
cc=$1; std=$2; t=$3
src=$SRCDIR/$t.cpp
[ -f "$src" ] || { echo "SKIP-VANISHED $cc $std $t"; exit 0; }
log=$OUT/${cc}_${std}_${t}.log
$cc -std=$std -O2 -DNDEBUG -Wall -Wextra $INC -I/mnt/d/Data/LocalGit/boost -I$SRCDIR \
    $src -o $OUT/${cc}_${std}_${t} > $log 2>&1
crc=$?
wsz=$(wc -c < $log)
if [ $crc -ne 0 ] || [ "$wsz" != "0" ]; then
   echo "FAIL-COMPILE $cc $std $t rc=$crc bytes=$wsz"
   exit 0
fi
$OUT/${cc}_${std}_${t} > $log.run 2>&1
if [ $? -ne 0 ]; then echo "FAIL-RUN $cc $std $t"; else echo "ok $cc $std $t"; fi
