# Per-symbol instruction / compare counts from `objdump -d --no-show-raw-insn`.
# Usage: 212_count.py <objdump-listing> [substring-filter ...]
import re, sys, subprocess

listing = open(sys.argv[1]).read().splitlines()
filters = sys.argv[2:]

sym_re = re.compile(r'^[0-9a-f]+ <(.+)>:$')
ins_re = re.compile(r'^\s+[0-9a-f]+:\s+(\S+)')

order, ins, cmps, jmps = [], {}, {}, {}
cur = None
for line in listing:
   m = sym_re.match(line)
   if m:
      cur = m.group(1)
      if cur not in ins:
         order.append(cur)
         ins[cur] = cmps[cur] = jmps[cur] = 0
      continue
   if cur is None:
      continue
   m = ins_re.match(line)
   if not m:
      continue
   op = m.group(1)
   ins[cur] += 1
   if op.startswith('cmp') or op.startswith('test') or op.startswith('ucomis'):
      cmps[cur] += 1
   if op.startswith('j'):
      jmps[cur] += 1

sel = [s for s in order if not filters or any(f in s for f in filters)]
if not sel:
   print("   (no matching symbol)")
   sys.exit(0)

dem = subprocess.run(['c++filt'] + ['--'] , input="\n".join(sel),
                     capture_output=True, text=True).stdout.splitlines()
tot_i = tot_c = tot_j = 0
for raw, pretty in zip(sel, dem):
   tot_i += ins[raw]; tot_c += cmps[raw]; tot_j += jmps[raw]
   short = pretty
   for k in ('partition_copy_false_bounded', 'partition_copy_false_dispatch',
             'partition_copy_true_bounded', 'partition_copy_true_dispatch',
             'set_union_until_exhausts', 'set_union_dst_bounded',
             'set_difference_until_exhausts', 'set_difference_dst_bounded',
             'set_intersection_until_exhausts', 'set_intersection_dst_bounded',
             'set_symmetric_difference_until_exhausts',
             'set_symmetric_difference_dst_bounded',
             'merge_until_exhausts', 'merge_dst_bounded', 'run_'):
      if k in pretty:
         short = k + (' [seg]' if 'segmented_iterator_tag' in pretty else '')
         break
   print("   %-42s insns=%-5d cmp/test=%-4d jcc=%-4d" %
         (short[:42], ins[raw], cmps[raw], jmps[raw]))
print("   %-42s insns=%-5d cmp/test=%-4d jcc=%-4d" % ("TOTAL", tot_i, tot_c, tot_j))
