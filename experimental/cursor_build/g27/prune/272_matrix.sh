#!/bin/bash
ROOT=/mnt/d/Data/LocalGit/boost
EXP=$ROOT/libs/container/experimental
OUT=$EXP/cursor_build/g27/prune
mkdir -p $OUT/logs
rm -f $OUT/logs/*.txt
RES=$OUT/272_results.txt
: > $RES

echo "helper wc -l: $(wc -l < $EXP/segmented_test_helper.hpp)" >> $RES

WORK=/tmp/g27
mkdir -p $WORK
rm -rf $WORK/*

run_one()
{
   CXX=$1; STD=$2; SRC=$3
   B=$(basename $SRC .cpp)
   TAG="${CXX}_${STD}_${B}"
   LOG=$WORK/$TAG.log
   BIN=$WORK/$TAG.exe
   $CXX -std=$STD -O2 -DNDEBUG -Wall -Wextra \
        -I$ROOT -I$EXP -o $BIN $SRC > $LOG 2>&1
   rc=$?
   if [ $rc -ne 0 ]; then
      echo "COMPILE_FAIL $TAG"
      cp $LOG $OUT/logs/$TAG.txt
      return
   fi
   if [ -s $LOG ]; then
      echo "COMPILE_OUTPUT $TAG"
      cp $LOG $OUT/logs/$TAG.txt
      return
   fi
   $BIN > $WORK/$TAG.runlog 2>&1
   rc=$?
   if [ $rc -ne 0 ]; then
      echo "RUN_FAIL $TAG rc=$rc"
      cp $WORK/$TAG.runlog $OUT/logs/run_$TAG.txt
      return
   fi
   rm -f $BIN
   echo "OK $TAG"
}
export -f run_one
export ROOT EXP OUT WORK

JOBS=$(nproc)
echo "jobs=$JOBS" >> $RES

for CXX in g++-16 clang++-22; do
   for STD in c++03 c++11 c++17 c++20; do
      for SRC in $EXP/segmented_*_test.cpp; do
         echo "$CXX $STD $SRC"
      done
   done
done > $WORK/tasks.txt

echo "total configurations: $(wc -l < $WORK/tasks.txt)" >> $RES

xargs -a $WORK/tasks.txt -P $JOBS -L1 bash -c 'run_one $0 $1 $2' >> $RES 2>&1

echo "===== SUMMARY =====" | tee -a $RES
grep -c "^OK " $RES | sed 's/^/OK: /' | tee -a $RES
for k in COMPILE_FAIL COMPILE_OUTPUT RUN_FAIL; do
   n=$(grep -c "^$k " $RES)
   echo "$k: $n" | tee -a $RES
   grep "^$k " $RES | tee -a $RES
done
