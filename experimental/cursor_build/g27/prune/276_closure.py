import re, glob, os, sys

EXP = '/mnt/d/Data/LocalGit/boost/libs/container/experimental'
H = os.path.join(EXP, 'segmented_test_helper.hpp')

src = open(H).read().split('\n')

# Entity -> (first_line, last_line) inclusive, 0-based, definition body only.
# Derived from the definition sites; a range ends at the line before the next
# entity's leading comment.
ents = [
 'movable_int','seg_vector_iterator','seg_vector','sentinel_wrapper','make_sentinel',
 'seg2_vector_iterator','seg2_vector','seg_split_point','shape_feasible','make_range',
 'make_dest_range','iter_at','seg_value_of','flatten_ints','flatten_all_ints',
 'flatten_n_ints','max_shape_depth','shape_specs','shape_specs_empty',
 'shape_specs_family','shape_all_families','with_shape','for_each_shape_fam_cat',
 'for_each_shape_all_cat','for_each_shape_all','for_each_shape_all_fwd',
 'shape2_bind','shape2_outer','for_each_shape2_fam','for_each_shape2_all',
 'shape3_bind','shape3_outer','for_each_shape3_fam','for_each_shape3_all',
 'for_each_dest_shape_all','filler_intact',
]

dead = ['for_each_shape','for_each_shape_fwd','for_each_shape_cat','for_each_shape2',
        'for_each_shape3','shape_core_families','make_sized_sentinel',
        'sized_sentinel_wrapper']

print('=== leftover mentions of deleted entities in the helper ===')
bad = False
for d in dead:
    for i, l in enumerate(src):
        if re.search(r'\b%s\b' % d, l):
            print('  LEFTOVER %s:%d: %s' % (d, i+1, l.strip()))
            bad = True
if not bad:
    print('  none')

# Strip comments so that references are code references only.
code = []
for l in src:
    s = l.lstrip()
    if s.startswith('//'):
        code.append('')
    else:
        code.append(re.sub(r'//.*$', '', l))

# Which entity does each code line belong to?  Segment the file by definition
# lines: a definition line is one where the entity name is introduced at
# namespace scope.  We approximate with a scan that tracks brace depth 0
# transitions after a 'template<...>' / 'inline' / 'class' / 'struct' header.
owner = [None] * len(code)
cur = None
depth = 0
pending = None
for i, l in enumerate(code):
    if depth == 0:
        for e in ents:
            # a definition introduces the name followed by '(' , '<'+params, or
            # ' {' / newline for class/struct
            if re.search(r'\b(class|struct)\s+%s\b' % e, l) or \
               re.search(r'\b%s\s*\(' % e, l):
                # class member functions are inside depth>0, so at depth 0 this
                # is a definition or a call from another definition's one-liner
                if pending is None:
                    pending = e
                break
    if pending is not None and cur is None:
        cur = pending
    owner[i] = cur
    o = l.count('{') + l.count('(')
    c = l.count('}') + l.count(')')
    depth += l.count('{') - l.count('}')
    if depth <= 0:
        depth = 0
        if l.strip().endswith('}') or l.strip().endswith('};') or \
           l.strip().endswith('}  ') or re.search(r'\}\s*;?\s*$', l.strip()):
            cur = None
            pending = None

# The heuristic above is fragile; use an explicit ownership map instead, built
# from the known definition line of each entity, extending to the next one.
defline = {}
for e in ents:
    for i, l in enumerate(code):
        if re.search(r'\b(class|struct)\s+%s\b' % e, l) or \
           re.search(r'^(inline\s+)?[A-Za-z_][\w:<>,*&\s]*?\b%s\s*\(' % e, l):
            defline.setdefault(e, i)
# entity that owns line i = the entity with the greatest defline <= i
order = sorted(defline.items(), key=lambda kv: kv[1])
def owner_of(i):
    r = None
    for e, d in order:
        if d <= i:
            r = e
        else:
            break
    return r

print()
print('=== ownership map (definition line of each remaining entity) ===')
for e, d in order:
    print('  %-26s line %d' % (e, d + 1))
missing = [e for e in ents if e not in defline]
if missing:
    print('  NOT LOCATED:', missing)

# Roots: entities named in a real consumer (test .cpp files that include the
# helper), code lines only.
consumers = sorted(glob.glob(os.path.join(EXP, 'segmented_*_test.cpp')))
consumers = [f for f in consumers if 'segmented_test_helper' in open(f).read()]
ctext = []
for f in consumers:
    for l in open(f):
        s = l.lstrip()
        if s.startswith('//'):
            continue
        ctext.append(re.sub(r'//.*$', '', l))
ctext = '\n'.join(ctext)

roots = set()
sites = {}
for e in ents:
    n = len(re.findall(r'\b%s\b' % e, ctext))
    sites[e] = n
    if n:
        roots.add(e)

# Edges: entity A -> entity B if a code line owned by A names B (A != B).
edges = {}
for e in ents:
    edges[e] = set()
for i, l in enumerate(code):
    o = owner_of(i)
    if o is None:
        continue
    for e in ents:
        if e == o:
            continue
        if re.search(r'\b%s\b' % e, l):
            edges[o].add(e)
# The traits specialisations at the bottom of the file are required by the
# library algorithms; treat their contents as reachable from the iterators.
marked = set(roots)
frontier = list(roots)
while frontier:
    a = frontier.pop()
    for b in edges[a]:
        if b not in marked:
            marked.add(b)
            frontier.append(b)

print()
print('=== post-prune reachability ===')
print('consumers scanned: %d test files' % len(consumers))
print('%-26s %-8s %s' % ('ENTITY', 'SITES', 'STATUS'))
unreach = []
for e in ents:
    if e in roots:
        st = 'ROOT'
    elif e in marked:
        st = 'reachable'
    else:
        st = '*** UNREACHABLE ***'
        unreach.append(e)
    print('%-26s %-8d %s' % (e, sites[e], st))

print()
if unreach:
    print('UNREACHABLE:', unreach)
else:
    print('every remaining entity is reachable from a real consumer')
