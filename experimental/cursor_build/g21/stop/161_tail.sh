#!/bin/bash
# Show the segment-transition tail of the real segmented_mismatch walker, and
# compare code size against the pre-change headers reconstructed in a shadow tree.
set -u
BR=/mnt/d/Data/LocalGit/boost
OUT=/tmp/g21t
mkdir -p $OUT

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
   echo "== $CXX =="
   for s in mm3 mm4 eq3; do
      b=$(nm --print-size $OUT/cg.$CXX.o | grep -E "_Z3$s" | awk '{printf "%d", strtonum("0x" $2)}')
      printf "   %-4s %s bytes\n" "$s" "${b:-?}"
   done
done

echo
echo "########## mm3 transition tail (gcc) ##########"
objdump -d --no-show-raw-insn -C $OUT/cg.g++-16.o > $OUT/g.asm
awk 'index($0,"mm3(") && /^[0-9a-f]+ </ {inf=1;next} inf && /^$/ {exit} inf {sub(/<[^>]*>/,""); print}' $OUT/g.asm | tail -34

echo
echo "########## count of segment-boundary compares in mm3 ##########"
echo "cmp against last1 / end2 style tests in the tail:"
awk 'index($0,"mm3(") && /^[0-9a-f]+ </ {inf=1;next} inf && /^$/ {exit} inf {print}' $OUT/g.asm \
  | grep -cE '^\s+[0-9a-f]+:\s+cmp\s+%r'
