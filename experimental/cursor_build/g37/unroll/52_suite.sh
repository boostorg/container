#!/bin/bash
# Full segmented test suite, run in parallel on cores 16-31.
#   MODE=live     -> live experimental tests against the live include tree
#   MODE=overlay  -> live experimental tests against snapshot + $VARIANT overlay
set -u
B=/mnt/d/Data/LocalGit/boost/libs/container
W=$B/experimental/cursor_build/g37/unroll
export MODE=${MODE:-live}
export VARIANT=${VARIANT:-p2_final}
export SRCDIR=$B/experimental
export OUT=$W/bin/suite_${MODE}_${VARIANT}
mkdir -p $OUT
if [ "$MODE" = "live" ]; then export INC="-I$B/include"
else export INC="-I$W/exp/$VARIANT -I$W/snap"; fi

tr -d '\r' < $W/51_suite_job.sh > $W/bin/job.sh
chmod +x $W/bin/job.sh

TESTS=$(ls $SRCDIR/segmented_*_test.cpp | xargs -n1 basename | sed 's/\.cpp$//')
JOBS=$OUT/jobs.txt
: > $JOBS
for t in $TESTS; do
  for std in c++03 c++11 c++17 c++20; do
    for cc in g++-16 clang++-22; do echo "$cc $std $t" >> $JOBS; done
  done
done
echo "jobs: $(wc -l < $JOBS)  mode=$MODE variant=$VARIANT inc=$INC"

taskset -c 16-31 xargs -a $JOBS -P 14 -L 1 bash $W/bin/job.sh > $OUT/results.txt 2>&1

echo "--- non-ok results ---"
grep -v '^ok ' $OUT/results.txt || echo "(none)"
echo "--- failing test names ---"
grep -E '^FAIL' $OUT/results.txt | awk '{print $4}' | sort -u > $OUT/failset.txt
cat $OUT/failset.txt
echo "--- expected known-unrelated set ---"
printf '%s\n' segmented_is_partitioned_test segmented_partition_test segmented_search_n_test segmented_search_test > $OUT/known.txt
cat $OUT/known.txt
if diff -q $OUT/failset.txt $OUT/known.txt > /dev/null; then
   echo "RESULT: failure set is EXACTLY the four known unrelated failures"
else
   echo "RESULT: failure set DIFFERS"
   diff $OUT/known.txt $OUT/failset.txt
fi
echo "ok count: $(grep -c '^ok ' $OUT/results.txt) / $(wc -l < $JOBS)"
