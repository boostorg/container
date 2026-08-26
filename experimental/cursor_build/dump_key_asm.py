#!/usr/bin/env python3
import re
from pathlib import Path

KEYS = [
    'copy_generic_ptr', 'copy_both_ra',
    'copy_ra_dst_list_src', 'copy_generic_list_src',
    'copy_ra_src_list_dst', 'copy_generic_list_dst',
    'copy_ra_src_unbounded',
    'equal_generic_ptr', 'equal_both_ra', 'equal_ra_a_list_b', 'equal_generic_list_b',
    'copy_if_both_ra', 'copy_if_ra_dst_list_src',
]

for name in ('gcc', 'clang'):
    text = Path(f'ra_onesided_probe2.{name}.s').read_text(encoding='utf-8', errors='replace')
    print(f'\n########## {name} ##########\n')
    for fn in KEYS:
        m = re.search(rf'(?m)^{fn}:.*?(?=^[A-Za-z_][A-Za-z0-9_]*:|\Z)', text, re.S)
        body = (m.group(0) if m else 'NOT FOUND').splitlines()
        # trim trailing size/.cfi noise after ret sequences somewhat
        print(f'===== {fn} =====')
        print('\n'.join(body[:55]))
        print()
