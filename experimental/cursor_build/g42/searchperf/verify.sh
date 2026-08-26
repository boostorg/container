#!/bin/bash
set -u
BR=/mnt/d/Data/LocalGit/boost
H=$BR/libs/container/experimental/cursor_build/g42/searchperf
E=$BR/libs/container/experimental

echo "=== self-containment (vector only, no deque to drag in <iterator>) ==="
cd /tmp
cat > se.cpp <<'EOF'
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_search.hpp>
namespace bc = boost::container;
typedef bc::vector<int>::const_iterator it_t;
it_t f(it_t a, it_t b, const int *p, const int *pe)
{  return bc::segmented_search(a, b, p, pe); }
EOF
for cc in g++-16 clang++-22; do
   printf '  %-12s ' "$cc"
   $cc -std=c++17 -O1 -DNDEBUG -I"$BR" -c se.cpp -o /dev/null 2>/tmp/se.err \
      && echo OK || { echo FAIL; grep -m1 'error:' /tmp/se.err; }
done

echo
echo "=== segmented_search_test matrix ==="
for cc in g++-16 clang++-22; do
   for std in c++03 c++11 c++17 c++20; do
      printf '  %-12s %-6s ' "$cc" "$std"
      if $cc -std=$std -O2 -DNDEBUG -I"$BR" -I"$E" \
           "$E/segmented_search_test.cpp" -o /tmp/st.elf 2>/tmp/st.log; then
         if /tmp/st.elf >/dev/null 2>&1; then echo "PASS"; else echo "RUNFAIL"; fi
      else
         echo "BUILDFAIL"; grep -m1 'error:' /tmp/st.log
      fi
   done
done
