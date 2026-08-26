#!/bin/bash
# Who actually needs <iterator>?  HEAD's search, the new search without the
# include, the new search with it, and segmented_find_if on its own.
set -u
BR=/mnt/d/Data/LocalGit/boost
SH=/tmp/sc_shadow/boost/container/experimental
mkdir -p "$SH"
cd "$BR/libs/container"
git show HEAD:include/boost/container/experimental/segmented_search.hpp > "$SH/segmented_search.hpp"

cd /tmp
cat > se.cpp <<'EOF'
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_search.hpp>
namespace bc = boost::container;
typedef bc::vector<int>::const_iterator it_t;
it_t f(it_t a, it_t b, const int *p, const int *pe)
{  return bc::segmented_search(a, b, p, pe); }
EOF

cat > fi.cpp <<'EOF'
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_find_if.hpp>
namespace bc = boost::container;
struct is_zero { bool operator()(int v) const { return !v; } };
typedef bc::vector<int>::const_iterator it_t;
it_t f(it_t a, it_t b) {  return bc::segmented_find_if(a, b, is_zero()); }
EOF

try() {  # name, source, extra include path
   printf '  %-34s ' "$1"
   for cc in g++-16 clang++-22; do
      if $cc -std=c++17 -O1 -DNDEBUG $3 -I"$BR" -c "$2" -o /dev/null 2>/tmp/sc.err; then
         printf '%s=OK ' "$cc"
      else
         printf '%s=FAIL ' "$cc"
      fi
   done
   echo
}

try "HEAD segmented_search"        se.cpp "-I/tmp/sc_shadow"
try "segmented_find_if alone"      fi.cpp ""
try "new segmented_search (as-is)" se.cpp ""
