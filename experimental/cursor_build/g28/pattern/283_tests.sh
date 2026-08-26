#!/bin/bash
# Correctness gate for g28 walker variants: 4 tests x variants x 2 compilers
# x 4 stds with -Wall -Wextra silent, plus asan+ubsan -O1 c++20.
# Known pre-existing warning tolerated: clang++ -std=c++03 on
# segmented_fill_test.cpp line 52 -Wc++11-extensions.
set -u
BR=/mnt/d/Data/LocalGit/boost
G=$BR/libs/container/experimental/cursor_build/g28/pattern
EX=$BR/libs/container/experimental
O=/tmp/g28tests
rm -rf $O; mkdir -p $O
fails=0

TESTS="fill count find copy"
VARIANTS=${VARIANTS:-E T F W}

known_warn_ok() {
   # $1 = log file: true if the only warnings are the known clang c++03 one
   grep 'warning:' "$1" | grep -v 'segmented_fill_test.cpp:52:.*C++11 extension' | grep -q . && return 1
   return 0
}

echo "=== build+run matrix (variant x test x compiler x std) ==="
for V in $VARIANTS; do
   INC="-I$G/shadow_$V -I$BR"
   for A in $TESTS; do
      for CC in g++-16 clang++-22; do
         line=""
         for STD in c++03 c++11 c++17 c++20; do
            log=$O/$V.$A.$CC.$STD.log
            if ! $CC -std=$STD -O2 -DNDEBUG -Wall -Wextra $INC -I$EX \
                 $EX/segmented_${A}_test.cpp -o $O/$V.$A.$CC.$STD >$log 2>&1; then
               line="$line BUILD-FAIL($STD)"; fails=1; head -20 $log | sed 's/^/      /'; continue
            fi
            if [ -s $log ] && ! known_warn_ok $log; then
               line="$line WARN($STD)"; fails=1; head -20 $log | sed 's/^/      /'; continue
            fi
            if $O/$V.$A.$CC.$STD >$log.run 2>&1; then line="$line $STD:pass"
            else line="$line $STD:RUNFAIL"; fails=1; tail -15 $log.run | sed 's/^/      /'; fi
         done
         printf "  %s %-8s %-12s %s\n" $V $A $CC "$line"
      done
   done
done

echo
echo "=== asan+ubsan -O1 c++20 ==="
for V in $VARIANTS; do
   INC="-I$G/shadow_$V -I$BR"
   for CC in g++-16 clang++-22; do
      for A in $TESTS; do
         if ! $CC -std=c++20 -O1 -g -Wall -Wextra -fsanitize=address,undefined \
              -fno-omit-frame-pointer $INC -I$EX \
              $EX/segmented_${A}_test.cpp -o $O/s.$V.$CC.$A >$O/s.$V.$CC.$A.log 2>&1; then
            printf "  %s %-12s %-8s BUILD FAILED\n" $V $CC $A; head -15 $O/s.$V.$CC.$A.log; fails=1; continue
         fi
         if $O/s.$V.$CC.$A >$O/s.$V.$CC.$A.run 2>&1; then printf "  %s %-12s %-8s clean\n" $V $CC $A
         else printf "  %s %-12s %-8s SANITIZER FAILURE\n" $V $CC $A; tail -25 $O/s.$V.$CC.$A.run; fails=1; fi
      done
   done
done

echo
[ $fails -eq 0 ] && echo "283: ALL OK" || echo "283: FAILURES PRESENT"
exit $fails
