#!/usr/bin/env python3
import re
from pathlib import Path

FUNCS = [
    'copy_generic_ptr', 'copy_both_ra',
    'copy_ra_dst_list_src', 'copy_generic_list_src',
    'copy_ra_src_list_dst', 'copy_generic_list_dst',
    'copy_ra_src_unbounded',
    'copy_if_generic_ptr', 'copy_if_both_ra',
    'copy_if_ra_dst_list_src', 'copy_if_ra_src_unbounded',
    'equal_generic_ptr', 'equal_both_ra',
    'equal_ra_a_list_b', 'equal_list_a_ra_b', 'equal_generic_list_b',
]
SIMD = re.compile(r'\b(movdqu|movaps|vmovdqu|vmovups|movdqa|movups|ymm|xmm0|vmovdqa)\b')

for name in ('gcc', 'clang'):
    text = Path(f'ra_onesided_probe2.{name}.s').read_text(encoding='utf-8', errors='replace')
    print('====', name)
    for fn in FUNCS:
        m = re.search(rf'(?m)^{fn}:.*?(?=^[A-Za-z_][A-Za-z0-9_]*:|\Z)', text, re.S)
        body = m.group(0) if m else ''
        simd = bool(SIMD.search(body))
        # rough: load through ->next indicates list walk
        nexts = body.count('(%')  # weak
        cmps = len(re.findall(r'\bcmpq\b', body))
        print(f'  {fn:28} simd={str(simd):5} cmpq={cmps:3} size={len(body):5}')
