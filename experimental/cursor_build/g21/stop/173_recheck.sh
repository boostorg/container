#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
S=$BR/libs/container/experimental/cursor_build/g21/stop
EX=$BR/libs/container/experimental

echo "############ leaf contract ############"
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -Wall -Wextra $S/172_leaf_flag.cpp -o /tmp/lf.elf 2>/dev/null \
      && { echo "-- $CXX --"; /tmp/lf.elf | tail -3; }
done

echo
echo "############ tie shapes (plain + sanitised) ############"
for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O2 -DNDEBUG -I$BR -Wall -Wextra $S/170_tie_probe.cpp -o /tmp/tp.elf 2>/dev/null \
      && { printf "%-12s plain      " "$CXX"; /tmp/tp.elf | tail -1; }
   $CXX -std=c++20 -O1 -g -fsanitize=address,undefined -I$BR $S/170_tie_probe.cpp -o /tmp/tps.elf 2>/dev/null \
      && { printf "%-12s sanitised  " "$CXX"; /tmp/tps.elf 2>&1 | tail -1; }
done

echo
echo "############ tests ############"
for CXX in g++-16 clang++-22; do
   for t in segmented_mismatch_test segmented_equal_test segmented_search_test; do
      $CXX -std=c++20 -O2 -DNDEBUG -I$BR -Wall -Wextra $EX/$t.cpp -o /tmp/$t.elf 2>/tmp/$t.log
      if [ $? -ne 0 ]; then printf "  %-12s %-26s BUILD FAIL\n" "$CXX" "$t"; continue; fi
      /tmp/$t.elf >/dev/null 2>&1
      printf "  %-12s %-26s warnings=%s run_exit=%s\n" "$CXX" "$t" "$(grep -c 'warning:' /tmp/$t.log)" "$?"
   done
done

echo
echo "############ code size ############"
tr -d '\r' < $S/161_tail.sh > /tmp/s161.sh
bash /tmp/s161.sh 2>&1 | head -8
