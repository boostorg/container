#!/usr/bin/env python3
"""353: alignment stats of tight loops + extraction of larger outer loops,
with jump-target symbol names stripped for readability."""
import re, sys

OUT = "/mnt/d/Data/LocalGit/boost/libs/container/experimental/cursor_build/g30/nsgpart"
cc = sys.argv[1] if len(sys.argv) > 1 else "gcc"

funcs = []
cur = None
for line in open(f"{OUT}/350_{cc}_partition_funcs.dis"):
    m = re.match(r"^[0-9a-f]+ <(.*)>:$", line.rstrip())
    if m:
        cur = (m.group(1), [])
        funcs.append(cur)
        continue
    m = re.match(r"^\s+([0-9a-f]+):\t(.*)$", line.rstrip())
    if m and cur is not None:
        txt = re.sub(r"<[^>]*\[clone \.isra\.0\](\+0x[0-9a-f]+)?>", "<F>", m.group(2))
        cur[1].append((int(m.group(1), 16), txt))

def shortname(name):
    if "std_partition_batch" in name:
        kind = "std"
    else:
        kind = "nsg" if ", true>, bench_ops::batch_reset" in name else "seg"
    pred = "miss/is_negative" if "is_negative" in name else "hit/is_odd"
    return f"{kind} {pred}"

MAXBODY = 90
for name, insns in funcs:
    addr2idx = {a: i for i, (a, _) in enumerate(insns)}
    end_addr = insns[-1][0]
    sn = shortname(name)
    print(f"\n############ {sn} : {len(insns)} insns ############")
    loops = {}
    for i, (a, txt) in enumerate(insns):
        m = re.match(r"^(j\w+)\s+([0-9a-f]+)", txt)
        if not m:
            continue
        tgt = int(m.group(2), 16)
        if tgt >= a or tgt not in addr2idx:
            continue
        body = i - addr2idx[tgt] + 1
        if body > MAXBODY:
            continue
        # keep smallest loop per backedge target = innermost; also record largest
        loops.setdefault(tgt, []).append((body, a, i))
    # summary of alignment for small loops
    print("-- inner-loop alignment summary (loops <= 12 insns) --")
    for tgt in sorted(loops):
        for body, a, i in loops[tgt]:
            if body > 12:
                continue
            j = addr2idx[tgt]
            next_a = insns[i+1][0] if i+1 < len(insns) else a+8
            nbytes = next_a - tgt
            cross = (tgt // 64) != ((next_a - 1) // 64)
            print(f"  loop @{tgt:6x} bytes={nbytes:3d} mod32={tgt%32:2d} mod64={tgt%64:2d} cross64={'YES' if cross else 'no '} body={body}")
    # print largest loop (outer loop) once, for the first two occurrences
    print("-- largest loop bodies (outer loops), first occurrence --")
    biggest = sorted(((b, t, a, i) for t, v in loops.items() for b, a, i in v), reverse=True)
    shown = 0
    for b, t, a, i in biggest:
        if b < 15 or shown >= 1:
            continue
        shown += 1
        j = addr2idx[t]
        print(f"  --- outer loop {t:x}..{a:x} ({b} insns) ---")
        for (aa, tt) in insns[j:i+1]:
            print(f"    {aa:6x}: {tt}")
