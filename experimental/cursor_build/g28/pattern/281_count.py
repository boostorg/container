# Per-symbol codegen metrics from `objdump -d --no-show-raw-insn` output.
# Usage: 281_count.py <listing> [substring-filter ...]
# Prints, per matching symbol: instruction count, cmp/test, conditional jumps,
# calls, unconditional jmps, and whether the symbol ends in a tail jmp.
import re, sys, subprocess

listing = open(sys.argv[1]).read().splitlines()
filters = sys.argv[2:]

sym_re = re.compile(r'^[0-9a-f]+ <(.+)>:$')
ins_re = re.compile(r'^\s+[0-9a-f]+:\s+(\S+)(.*)$')

order, ins, cmps, jcc, calls, jmps, lastop = [], {}, {}, {}, {}, {}, {}
cur = None
for line in listing:
   m = sym_re.match(line)
   if m:
      cur = m.group(1)
      if cur not in ins:
         order.append(cur)
         ins[cur] = cmps[cur] = jcc[cur] = calls[cur] = jmps[cur] = 0
         lastop[cur] = ''
      continue
   if cur is None:
      continue
   m = ins_re.match(line)
   if not m:
      continue
   op = m.group(1)
   ins[cur] += 1
   lastop[cur] = op + m.group(2)
   if op.startswith(('cmp', 'test', 'ucomis')):
      cmps[cur] += 1
   elif op == 'jmp' or op == 'jmpq':
      jmps[cur] += 1
   elif op.startswith('j'):
      jcc[cur] += 1
   elif op.startswith('call'):
      calls[cur] += 1

sel = [s for s in order if not filters or any(f in s for f in filters)]
if not sel:
   print("   (no matching symbol)")
   sys.exit(0)

dem = subprocess.run(['c++filt'], input="\n".join(sel),
                     capture_output=True, text=True).stdout.splitlines()
tot = [0]*5
for raw, pretty in zip(sel, dem):
   tot[0] += ins[raw]; tot[1] += cmps[raw]; tot[2] += jcc[raw]
   tot[3] += calls[raw]; tot[4] += jmps[raw]
   tail = 'tail-jmp' if lastop[raw].split()[0] in ('jmp', 'jmpq') and '<' in lastop[raw] else ''
   short = pretty
   for k in ('probe_', 'fill_range', 'count_dispatch', 'find_dispatch',
             'copy_dispatch', 'copy_dst_bounded', 'copy_dst_dispatch',
             'segmented_walk_until', 'segmented_walk'):
      if k in pretty:
         tag = ' [seg]' if 'segmented_iterator_tag' in pretty else ''
         if k == 'probe_':
            short = pretty.split('(')[0]
         else:
            short = k + tag
            for d in ('seg2_vector', 'seg_vector', 'deque'):
               if d in pretty:
                  short += ' <' + d + '>'
                  break
         break
   print("   %-52s insns=%-5d cmp=%-4d jcc=%-4d call=%-3d jmp=%-3d %s" %
         (short[:52], ins[raw], cmps[raw], jcc[raw], calls[raw], jmps[raw], tail))
print("   %-52s insns=%-5d cmp=%-4d jcc=%-4d call=%-3d jmp=%-3d" %
      ("TOTAL(matched)", tot[0], tot[1], tot[2], tot[3], tot[4]))
