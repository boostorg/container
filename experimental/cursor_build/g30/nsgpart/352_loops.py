#!/usr/bin/env python3
"""352: extract tight inner loops (short backward branches) from the six
partition measure_batch instantiations in 350_{cc}_partition_funcs.dis."""
import re, sys

OUT = "/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g30/nsgpart"
cc = sys.argv[1] if len(sys.argv) > 1 else "gcc"
MAXBODY = int(sys.argv[2]) if len(sys.argv) > 2 else 30

funcs = []  # (name, [(addr, text)])
cur = None
for line in open(f"{OUT}/350_{cc}_partition_funcs.dis"):
    m = re.match(r"^[0-9a-f]+ <(.*)>:$", line.rstrip())
    if m:
        cur = (m.group(1), [])
        funcs.append(cur)
        continue
    m = re.match(r"^\s+([0-9a-f]+):\t(.*)$", line.rstrip())
    if m and cur is not None:
        cur[1].append((int(m.group(1), 16), m.group(2)))

def shortname(name):
    kind = "std" if "std_partition_batch" in name else "seg/nsg"
    if "seg_partition_batch" in name:
        kind = "nsg" if re.search(r"MyInt>, (true|false)>", name) and ", true>" in name else "seg"
    pred = "is_negative(miss)" if "is_negative" in name else "is_odd(hit)"
    return f"{kind} {pred}"

for name, insns in funcs:
    addr2idx = {a: i for i, (a, _) in enumerate(insns)}
    print(f"\n############ {shortname(name)} : {len(insns)} insns ############")
    seen = set()
    for i, (a, txt) in enumerate(insns):
        m = re.match(r"^(j\w+)\s+([0-9a-f]+) <", txt)
        if not m:
            continue
        tgt = int(m.group(2), 16)
        if tgt >= a or tgt not in addr2idx:
            continue
        j = addr2idx[tgt]
        body = i - j + 1
        if body > MAXBODY:
            continue
        key = (tgt, a)
        if key in seen:
            continue
        seen.add(key)
        print(f"--- loop {tgt:x}..{a:x} ({body} insns) ---")
        for (aa, tt) in insns[j:i+1]:
            print(f"  {aa:6x}: {tt}")
