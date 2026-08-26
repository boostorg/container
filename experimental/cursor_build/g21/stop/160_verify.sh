#!/bin/bash
# Verify the applied stop-flag change: tests, standards, warnings, and the
# resulting per-segment code for the mismatch/equal walkers.
set -u
BR=/mnt/d/Data/LocalGit/boost
EX=$BR/libs/container/experimental
OUT=/tmp/g21v
mkdir -p $OUT

TESTS="segmented_mismatch_test segmented_equal_test segmented_search_test"

echo "############ tests, -Wall -Wextra ############"
for CXX in g++-16 clang++-22; do
   for t in $TESTS; do
      SRC=$EX/$t.cpp
      [ -f "$SRC" ] || { printf "%-12s %-26s MISSING\n" "$CXX" "$t"; continue; }
      LOG=$OUT/$CXX.$t.log
      $CXX -std=c++20 -O2 -DNDEBUG -I$BR -Wall -Wextra "$SRC" -o $OUT/$t.$CXX.elf 2>$LOG
      if [ $? -ne 0 ]; then
         printf "%-12s %-26s BUILD FAIL\n" "$CXX" "$t"
         grep -m5 'error:' $LOG
         continue
      fi
      w=$(grep -c 'warning:' $LOG)
      $OUT/$t.$CXX.elf > $OUT/$t.$CXX.run 2>&1
      printf "%-12s %-26s build ok warnings=%-3s run_exit=%s\n" "$CXX" "$t" "$w" "$?"
      [ "$w" != "0" ] && grep -m4 'warning:' $LOG
   done
done

echo
echo "############ standards ############"
for CXX in g++-16 clang++-22; do
   for STD in c++03 c++11 c++17 c++20; do
      LOG=$OUT/std.$CXX.$STD.log
      $CXX -std=$STD -O2 -DNDEBUG -I$BR -Wall -Wextra \
           $EX/segmented_mismatch_test.cpp -o $OUT/std.elf 2>$LOG
      rc=$?
      w=$(grep -c 'warning:' $LOG)
      if [ $rc -eq 0 ]; then
         $OUT/std.elf >/dev/null 2>&1
         printf "  %-12s %-7s ok warnings=%-3s run_exit=%s\n" "$CXX" "$STD" "$w" "$?"
      else
         printf "  %-12s %-7s FAIL\n" "$CXX" "$STD"
         grep -m4 'error:' $LOG
      fi
   done
done

echo
echo "############ per-segment codegen (gcc, deque<int> vs vector) ############"
cat > $OUT/cg.cpp <<'CPPEOF'
#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_mismatch.hpp>
#include <boost/container/experimental/segmented_equal.hpp>
#include <vector>
#include <utility>

namespace bc = boost::container;
typedef bc::deque_options< bc::block_size<128> >::type opt_t;
typedef std::vector<int>::const_iterator                 vit;
typedef bc::deque<int, void, opt_t>::const_iterator       dit;

std::pair<vit, dit> mm3(vit f1, vit l1, dit f2)
{  return bc::segmented_mismatch(f1, l1, f2);  }

std::pair<vit, dit> mm4(vit f1, vit l1, dit f2, dit l2)
{  return bc::segmented_mismatch(f1, l1, f2, l2);  }

bool eq3(vit f1, vit l1, dit f2)
{  return bc::segmented_equal(f1, l1, f2);  }
CPPEOF

for CXX in g++-16 clang++-22; do
   $CXX -std=c++20 -O3 -DNDEBUG -I$BR -c $OUT/cg.cpp -o $OUT/cg.$CXX.o 2>/dev/null || continue
   echo "-- $CXX symbol sizes --"
   nm --print-size -C $OUT/cg.$CXX.o | grep -E ' (mm3|mm4|eq3)\(' | awk '{printf "   %-6s %d bytes\n", $NF, strtonum("0x" $2)}'
   objdump -d --no-show-raw-insn -C $OUT/cg.$CXX.o > $OUT/cg.$CXX.asm
   for s in mm3 mm4 eq3; do
      n=$(awk -v sym="$s(" 'index($0,sym) && /^[0-9a-f]+ </ {inf=1;next} inf && /^$/ {exit} inf && /:/ {c++} END{print c+0}' $OUT/cg.$CXX.asm)
      echo "   $s insns=$n"
   done
done

echo
echo "############ mm3 inner/transition, gcc ############"
awk 'index($0,"mm3(") && /^[0-9a-f]+ </ {inf=1;next} inf && /^$/ {exit} inf {sub(/<[^>]*>/,""); print}' $OUT/cg.g++-16.asm
