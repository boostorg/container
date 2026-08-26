#!/bin/bash
# Is each header self-contained w.r.t. std::*_iterator_tag being complete?
set -u
BR=/mnt/d/Data/LocalGit/boost
cd /tmp

cat > mm.cpp <<'EOF'
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_mismatch.hpp>
namespace bc = boost::container;
typedef bc::vector<int>::const_iterator it_t;
std::pair<it_t, const int*> f(it_t a, it_t b, const int *p, const int *pe)
{  return bc::segmented_mismatch(a, b, p, pe); }
EOF

cat > se.cpp <<'EOF'
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_search.hpp>
namespace bc = boost::container;
typedef bc::vector<int>::const_iterator it_t;
it_t f(it_t a, it_t b, const int *p, const int *pe)
{  return bc::segmented_search(a, b, p, pe); }
EOF

for f in mm se; do
   printf '%-4s ' "$f"
   if g++-16 -std=c++17 -O1 -DNDEBUG -I"$BR" -c $f.cpp -o /dev/null 2>/tmp/$f.err; then
      echo "OK (self-contained)"
   else
      echo "FAIL: $(grep -m1 'error:' /tmp/$f.err)"
   fi
done
