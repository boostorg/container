#!/usr/bin/env python3
"""359: 64-byte-line crossing stats for the tight loops in a partition_funcs.dis file."""
import re, sys

OUT = "/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g30/nsgpart"
fn = sys.argv[1]

funcs, cur = [], None
for line in open(f"{OUT}/{fn}"):
    m = re.match(r"^[0-9a-f]+ <(.*)>:$", line.rstrip())
    if m:
        cur = (m.group(1), [])
        funcs.append(cur)
        continue
    m = re.match(r"^\s+([0-9a-f]+):\t(.*)$", line.rstrip())
    if m and cur is not None:
        cur[1].append((int(m.group(1), 16), m.group(2)))

def shortname(name):
    if "std_partition_batch" in name:
        kind = "std"
    else:
        kind = "nsg" if ", true>, bench_ops::batch_reset" in name else "seg"
    pred = "miss" if "is_negative" in name else "hit"
    return f"{kind} {pred}"

for name, insns in funcs:
    addr2idx = {a: i for i, (a, _) in enumerate(insns)}
    fit = cross = 0
    sizes = []
    for i, (a, txt) in enumerate(insns):
        m = re.match(r"^(j\w+)\s+([0-9a-f]+)", txt)
        if not m:
            continue
        tgt = int(m.group(2), 16)
        if tgt >= a or tgt not in addr2idx:
            continue
        body = i - addr2idx[tgt] + 1
        if body > 20:
            continue
        next_a = insns[i+1][0] if i+1 < len(insns) else a + 8
        nbytes = next_a - tgt
        if (tgt // 64) != ((next_a - 1) // 64):
            cross += 1
        else:
            fit += 1
        sizes.append(nbytes)
    print(f"{shortname(name):10s} loops<=20insn: fit={fit} cross64={cross} sizes={sorted(set(sizes))}")
