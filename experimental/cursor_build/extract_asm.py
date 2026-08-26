#!/usr/bin/env python3
import re
import sys
from pathlib import Path

FUNCS = [
    'copy_generic_bounded', 'copy_both_ra', 'copy_ra_dst_only',
    'copy_ra_src_only_bounded', 'copy_ra_src_unbounded', 'copy_ra_src_counted',
    'copy_if_generic', 'copy_if_both_ra', 'copy_if_ra_dst_only', 'copy_if_ra_src_unbounded',
    'equal_generic', 'equal_both_ra', 'equal_ra_src_only', 'equal_ra_dst_only',
]

def extract(path: Path):
    text = path.read_text(encoding='utf-8', errors='replace').splitlines()
    # GNU: name:   / LLVM: name:  (maybe with .globl)
    labels = {}
    for i, line in enumerate(text):
        m = re.match(r'^([A-Za-z_][A-Za-z0-9_$]*):\s*(?:#.*)?$', line.strip())
        if m:
            labels[m.group(1)] = i

    out = []
    for fn in FUNCS:
        # try mangled / unmangled / with leading _
        idx = None
        for key in (fn, '_' + fn):
            if key in labels:
                idx = labels[key]
                break
        # C++ may mangle; scan for substring
        if idx is None:
            for k, v in labels.items():
                if fn in k and not k.startswith('.'):
                    idx = v
                    break
        if idx is None:
            out.append(f'===== {fn}: NOT FOUND =====\n')
            continue
        # take until next global label that is not a local .L
        chunk = [text[idx]]
        for line in text[idx+1:]:
            if re.match(r'^[A-Za-z_][A-Za-z0-9_$]*:$', line.strip()) and not line.strip().startswith('.L'):
                # stop at next function, but allow .cfi etc
                name = line.strip()[:-1]
                if name in labels and not name.startswith('.') and name != fn:
                    break
            chunk.append(line)
            if len(chunk) > 120:
                break
        # classify
        body = '\n'.join(chunk)
        simd = any(x in body for x in ('movdqu', 'movaps', 'vmovdqu', 'vmovups', 'movdqa', 'ymm', 'xmm', 'movups'))
        out.append(f'===== {fn} (simd={simd}, lines={len(chunk)}) =====\n')
        out.append('\n'.join(chunk[:90]) + '\n\n')
    return ''.join(out)

def main():
    for asm in sys.argv[1:]:
        p = Path(asm)
        print(f'\n########## {p.name} ##########\n')
        print(extract(p))

if __name__ == '__main__':
    main()
