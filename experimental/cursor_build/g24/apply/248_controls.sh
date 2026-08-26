#!/bin/bash
# Negative controls: is the depth-3 coverage real?
#
# Each mutation is guarded on dst_is_local_seg_t::value, so it changes only the
# self-recursive instantiation of the segmented-destination walker.  For every
# algorithm and every mutation two runs are compared:
#
#   pre   the test with its single test_*_dst_depth3() call removed, i.e. the
#         coverage that existed before this change.  MUST PASS: if it fails the
#         mutation is not isolated to the depth-3 path.
#   full  the test as it now stands.  MUST FAIL: if it passes, the depth-3
#         enumeration compiles a deeper type without ever exercising the new
#         path, and the coverage is nominal rather than real.
#
# The real headers are never modified; the mutated copies are picked up by -I
# ordering.
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g24/apply
EX=$BR/libs/container/experimental
O=/tmp/g24ctl
rm -rf $O; mkdir -p $O

# test-stem : header-stem
PAIRS="set_union:segmented_set_union \
       set_difference:segmented_set_difference \
       set_intersection:segmented_set_intersection \
       set_symmetric_difference:segmented_set_symmetric_difference \
       merge:segmented_merge \
       copy:segmented_copy"

CC=g++-16
bad=0

printf "%-26s %-8s %-22s %-22s %s\n" algorithm mutation "pre (must pass)" "full (must fail)" verdict
printf "%-26s %-8s %-22s %-22s %s\n" -------------------------- -------- ---------------------- ---------------------- -------

for MODE in abort nolast nomid; do
   for P in $PAIRS; do
      T=${P%%:*}
      H=${P##*:}
      SH=$O/$MODE/$T
      rm -rf $SH

      if ! python3 $S/247_mutate.py $H $MODE \
              $SH/boost/container/experimental/$H.hpp >$O/mk.log 2>&1; then
         printf "%-26s %-8s %s\n" $T $MODE "MUTATION FAILED"; cat $O/mk.log; bad=1; continue
      fi

      # "pre": the same source minus the one call that runs depth 3.
      grep -v "test_${T}_dst_depth3();" $EX/segmented_${T}_test.cpp > $O/pre_$T.cpp

      res=()
      for V in pre full; do
         if [ $V = pre ]; then SRCF=$O/pre_$T.cpp; else SRCF=$EX/segmented_${T}_test.cpp; fi
         if ! $CC -std=c++17 -O1 -I$SH -I$BR -I$EX $SRCF -o $O/x.$MODE.$T.$V \
              >$O/b.$MODE.$T.$V.log 2>&1; then
            res+=("BUILD-FAIL"); continue
         fi
         $O/x.$MODE.$T.$V >$O/r.$MODE.$T.$V.log 2>&1
         rc=$?
         if [ $rc -eq 0 ]; then
            res+=("pass")
         elif [ $rc -gt 128 ]; then
            res+=("signal $((rc-128))")
         else
            res+=("fail rc=$rc, $(grep -c 'test:' $O/r.$MODE.$T.$V.log) errors")
         fi
      done

      verdict=BAD
      [ "${res[0]}" = "pass" ] && [ "${res[1]}" != "pass" ] && verdict=ok
      [ $verdict = BAD ] && bad=1
      printf "%-26s %-8s %-22s %-22s %s\n" $T $MODE "${res[0]}" "${res[1]}" $verdict
   done
   echo
done

echo "--- control on the control: unmutated headers, full tests ---"
for P in $PAIRS; do
   T=${P%%:*}
   $CC -std=c++17 -O1 -I$BR -I$EX $EX/segmented_${T}_test.cpp -o $O/clean.$T \
      >$O/clean.$T.log 2>&1 && $O/clean.$T >/dev/null 2>&1 \
      && printf "  %-26s pass\n" $T \
      || { printf "  %-26s UNEXPECTED FAILURE\n" $T; bad=1; }
done

echo
[ $bad -eq 0 ] && echo "248: ALL CONTROLS BEHAVED AS REQUIRED" || echo "248: A CONTROL DID NOT BEHAVE"
exit $bad
