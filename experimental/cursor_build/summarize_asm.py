#!/usr/bin/env python3
import re
from pathlib import Path

FUNCS = [
    'copy_generic_bounded', 'copy_both_ra', 'copy_ra_dst_only',
    'copy_ra_src_only_bounded', 'copy_ra_src_unbounded', 'copy_ra_src_counted',
    'copy_if_generic', 'copy_if_both_ra', 'copy_if_ra_dst_only', 'copy_if_ra_src_unbounded',
    'equal_generic', 'equal_both_ra', 'equal_ra_src_only', 'equal_ra_dst_only',
]
SIMD = re.compile(r'\b(movdqu|movaps|vmovdqu|vmovups|movdqa|movups|ymm|xmm0)\b')

for name in ('gcc', 'clang'):
    text = Path(f'ra_onesided_probe.{name}.s').read_text(encoding='utf-8', errors='replace')
    print('====', name)
    for fn in FUNCS:
        m = re.search(rf'(?m)^{fn}:.*?(?=^[A-Za-z_][A-Za-z0-9_]*:|\Z)', text, re.S)
        body = m.group(0) if m else ''
        simd = bool(SIMD.search(body))
        cmps = len(re.findall(r'\bcmpq\b', body))
        print(f'  {fn:28} simd={str(simd):5} cmpq={cmps:3} chars={len(body):5}')
