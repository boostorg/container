#!/bin/bash
set -eux
REPO=/mnt/d/Data/LocalGit/boost/libs/container
OUT=$REPO/experimental/cursor_build
INC="-I$REPO/include -I/mnt/d/Data/LocalGit/boost"

cat > /tmp/cn_bench_leaf.cpp <<'EOF'
#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_copy_n.hpp>
#include <boost/container/experimental/wrapped_iterator.hpp>
#include <algorithm>
#include <cstddef>

namespace bc = boost::container;

struct MyInt {
   int int_;
   MyInt(int i = 0) : int_(i) {}
   MyInt(const MyInt& o) : int_(o.int_) {}
   MyInt& operator=(const MyInt& o) { int_ = o.int_; return *this; }
   ~MyInt() { int_ = 0; }
};

typedef typename bc::deque_options< bc::block_size<128> >::type opt_t;
typedef bc::deque<MyInt, void, opt_t> deq_t;
typedef bc::vector<MyInt> vec_t;

__attribute__((noinline)) void std_copy_n_1S(const deq_t& c, vec_t& out, std::ptrdiff_t n)
{
   std::copy_n(c.begin(), n, out.begin());
}

__attribute__((noinline)) void nsg_copy_n_1S(const deq_t& c, vec_t& out, std::ptrdiff_t n)
{
   typedef bc::wrapped_iterator<deq_t::const_iterator> WI;
   typedef bc::wrapped_iterator<vec_t::iterator> WO;
   bc::segmented_copy_n(WI(c.begin()), n, WO(out.begin()));
}

__attribute__((noinline)) void seg_copy_n_1S(const deq_t& c, vec_t& out, std::ptrdiff_t n)
{
   bc::segmented_copy_n(c.begin(), n, out.begin());
}
EOF

g++-16 -std=c++20 -O3 -DNDEBUG $INC -S -o $OUT/cn_bench_leaf.s /tmp/cn_bench_leaf.cpp
g++-16 -std=c++20 -O3 -DNDEBUG $INC -c -o $OUT/cn_bench_leaf.o /tmp/cn_bench_leaf.cpp
echo "---- symbols ----"
nm $OUT/cn_bench_leaf.o | c++filt | grep copy_n

python3 - <<'PY'
from pathlib import Path
text = Path("/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/cn_bench_leaf.s").read_text().splitlines()
# Collect function ranges by .globl / .size
funcs = {}
cur = None
buf = []
for line in text:
    if line.startswith(".globl\t"):
        if cur is not None:
            funcs[cur] = buf
        cur = line.split("\t",1)[1]
        buf = [line]
    elif cur is not None:
        buf.append(line)
        if line.startswith("\t.size\t") and cur in line:
            funcs[cur] = buf
            cur = None
            buf = []
want = ["std_copy_n_1S", "nsg_copy_n_1S", "seg_copy_n_1S"]
for mang, body in funcs.items():
    # keep only our three
    joined = "\n".join(body)
    keep = None
    for w in want:
        if w in mang:  # won't match mangled
            keep = w
    # use c++filt via checking known prefixes from nm
print("parsed", len(funcs), "globals")
# Print bodies for symbols containing copy_n_1S mangling patterns from nm
import subprocess
nm = subprocess.check_output(["nm", "/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/cn_bench_leaf.o"], text=True)
mapping = {}
for line in nm.splitlines():
    parts = line.split()
    if len(parts) >= 3 and parts[1] in "Tt":
        mang = parts[-1]
        dem = subprocess.check_output(["c++filt", mang], text=True).strip()
        for w in want:
            if dem.startswith(w+"(") or dem == w:
                mapping[w] = mang
print(mapping)
for w, mang in mapping.items():
    body = funcs.get(mang)
    if not body:
        # try with leading _Z already
        for k,v in funcs.items():
            if k.endswith(mang) or k == mang:
                body = v
                break
    print("\n========", w, mang, "========")
    if not body:
        print("NOT FOUND")
        continue
    # print only interesting lines
    for line in body:
        if any(x in line for x in (".L", "mov", "add", "cmp", "j", "lea", "ret", "movd", "sub", "test", "xor", "and", "shr", "sal", "dec", "inc")):
            print(line)
PY
