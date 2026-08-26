# Group 25 with `BOOST_CONTAINER_SEGMENTED_DISABLE_PRAGMA_UNROLL` — outlier investigation

Work directory: `experimental/cursor_build/g34/unroll/`
Scripts `390`–`404`, run outputs in `runs/`, disassembly in `dis/`.

---

## 0. Snapshot state (what was measured)

The very first action was copying `include/boost/container/` to
`experimental/cursor_build/g34/unroll/snap/boost/container/`, and
`experimental/bench_segmented_algos.cpp` + `segmented_test_helper.hpp` into the g34 area.
Everything below was built with `-I<snap> -I/mnt/d/Data/LocalGit/boost`, verified by
`g++ -H`: **48 headers came from the snapshot, 0 from the live tree**.

At snapshot time (`git log -1` = `2d8d74e Use BOOST_[UN]LIKELY instead of own macro`):

```
git diff --stat -- include/
 include/boost/container/experimental/segmented_iterator_traits.hpp | 2 +-
 1 file changed, 1 insertion(+), 1 deletion(-)
```

The only uncommitted header change is the pragma switch itself. **The conditional-write leaf
fix is already committed in `HEAD`** (commit `6a1a7d6 Fix standard mandatory predicate calls
on segmented outputs`), and is present in all three leaves — verified by inspection of the
snapshot copies:

| leaf | entry `dst_first == dst_last` guard | post-write guard + `++first; goto out_path` |
|---|---|---|
| `segmented_copy_if.hpp` | line 60 | lines 68–71 |
| `segmented_remove_copy.hpp` | line 49 | lines 57–59 |
| `segmented_remove_copy_if.hpp` | line 62 | lines 70–72 |

So the leaf-fix variable did **not** move under me; only the unroll variable did. The
on/off matrix below is therefore a clean single-variable comparison (unroll on/off × fix
always present).

An md5 comparison of all 48 experimental headers across
`libs/container/include/…`, the staged boost root `/mnt/d/.../boost/container/…` and my
snapshot reported **0 mismatches**, and the snapshot-built binaries were byte-size identical
to the exact-command binaries. The exact-command output in §1 is therefore equivalent to a
snapshot build.

## Mechanism confirmed (`segmented_iterator_traits.hpp` 396–422)

```
396: #define BOOST_CONTAINER_SEGMENTED_DISABLE_PRAGMA_UNROLL     <- user's change (was commented)
403: #if !defined(DISABLE) && !defined(ENABLE)
404:    #if defined(BOOST_CLANG)
406:       #define BOOST_CONTAINER_SEGMENTED_DISABLE_PRAGMA_UNROLL
407:    #else
408:       #define BOOST_CONTAINER_SEGMENTED_ENABLE_PRAGMA_UNROLL
417: #elif defined(DISABLE)
418:    #define BOOST_CONTAINER_SEGMENTED_UNROLL(N)          <- empty
419:    #define BOOST_CONTAINER_SEGMENTED_AUTO_UNROLL         <- empty
```

**Critical consequence, which frames everything below: the default block at 403–410 already
selected `DISABLE` for Clang.** Forcing `DISABLE` at line 396 therefore changes **GCC only**.
Clang's codegen is bit-identical before and after the user's edit. Any "clang got worse"
reading of the numbers would be wrong — clang was always in the no-pragma configuration.

---

## 1. The user's exact command, verbatim output

Run from `libs/container/experimental`, `-I../../..`, single unpinned run.

### `g++-16 -std=c++20 -O3 -I../../.. -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25 -falign-functions=64 -falign-loops=64`

```
=== Segmented algorithm benchmark [5MyInt] ===
Elements: 100000   Iterations: 5000

--- bc::deque<5MyInt> ---

===== Group 25: 2-range input-output algorithms [5MyInt] =====
< algo >                     < nsg/seg > < std/seg > < std/nsg > < seg ns > < std ns > < nsg ns >
copy(1S)                            2.33        2.33        1.00      0.107      0.250      0.249
copy(2S)                            5.47        5.43        0.99      0.050      0.271      0.273
copy(1+2S)                          5.74        5.94        1.04      0.063      0.375      0.362
copy_if(1S hit)                     1.51        1.51        1.00      0.297      0.449      0.448
copy_if(2S hit)                     1.24        1.34        1.08      0.364      0.488      0.451
copy_if(1+2S hit)                   1.29        1.28        1.00      0.396      0.508      0.509
copy_if(1S miss)                    1.36        1.39        1.02      0.360      0.501      0.490
copy_if(2S miss)                    0.92        0.92        1.00      0.253      0.231      0.231
copy_if(1+2S miss)                  1.05        1.18        1.12      0.305      0.360      0.321
copy_n(1S)                          5.57        5.26        0.94      0.051      0.269      0.285
copy_n(2S)                          5.48        5.09        0.93      0.050      0.256      0.276
copy_n(1+2S)                        6.25        7.79        1.25      0.063      0.491      0.394
remove_copy(1S hit)                 1.82        1.81        0.99      0.256      0.463      0.465
remove_copy(2S hit)                 1.73        1.70        0.99      0.259      0.442      0.449
remove_copy(1+2S hit)               1.70        1.73        1.02      0.275      0.476      0.468
remove_copy(1S miss)                1.84        1.86        1.01      0.249      0.463      0.457
remove_copy(2S miss)                1.34        1.36        1.01      0.259      0.351      0.348
remove_copy(1+2S miss)              1.53        1.59        1.04      0.270      0.429      0.412
remove_copy_if(1S hit)              1.12        1.49        1.34      0.428      0.639      0.478
remove_copy_if(2S hit)              1.27        1.22        0.95      0.383      0.465      0.488
remove_copy_if(1+2S hit)            1.10        1.20        1.09      0.436      0.524      0.479
remove_copy_if(1S miss)             1.76        1.77        1.01      0.247      0.437      0.434
remove_copy_if(2S miss)             1.30        1.32        1.02      0.257      0.341      0.333
remove_copy_if(1+2S miss)           1.66        1.67        1.01      0.263      0.440      0.437
swap_ranges(1S)                     3.43        3.13        0.91      0.101      0.317      0.347
swap_ranges(2S)                     3.91        4.56        1.17      0.084      0.381      0.327
swap_ranges(1+2S)                   3.97        4.09        1.03      0.099      0.405      0.393
transform(1S)                       3.54        3.36        0.95      0.104      0.350      0.368
transform(2S)                       5.98        5.97        1.00      0.053      0.318      0.318
transform(1+2S)                     6.59        6.61        1.00      0.064      0.422      0.421
-------------------------------------------------------------------------------------------------
group geomean                       2.25        2.32        1.03       [5MyInt]
```

### `clang++-22` (same flags)

```
===== Group 25: 2-range input-output algorithms [5MyInt] =====
< algo >                     < nsg/seg > < std/seg > < std/nsg > < seg ns > < std ns > < nsg ns >
copy(1S)                            5.66        5.66        1.00      0.048      0.270      0.270
copy(2S)                            4.99        4.99        1.00      0.046      0.232      0.231
copy(1+2S)                          7.15        7.35        1.03      0.058      0.424      0.413
copy_if(1S hit)                     1.72        1.67        0.97      0.247      0.413      0.423
copy_if(2S hit)                     0.99        0.99        1.00      0.230      0.227      0.227
copy_if(1+2S hit)                   2.71        2.73        1.01      0.194      0.532      0.527
copy_if(1S miss)                    5.05        2.53        0.50      0.110      0.279      0.556
copy_if(2S miss)                    2.29        2.29        1.00      0.082      0.188      0.188
copy_if(1+2S miss)                  5.97        5.83        0.98      0.089      0.521      0.533
copy_n(1S)                          6.88        5.28        0.77      0.049      0.261      0.340
copy_n(2S)                          7.31        8.97        1.23      0.047      0.417      0.340
copy_n(1+2S)                        8.32        7.83        0.94      0.056      0.439      0.467
remove_copy(1S hit)                 1.68        2.25        1.34      0.242      0.545      0.407
remove_copy(2S hit)                 1.35        1.31        0.97      0.225      0.294      0.304
remove_copy(1+2S hit)               1.92        1.93        1.01      0.238      0.459      0.456
remove_copy(1S miss)                1.57        2.20        1.40      0.243      0.533      0.381
remove_copy(2S miss)                1.56        1.56        1.00      0.224      0.350      0.351
remove_copy(1+2S miss)              1.82        1.84        1.01      0.226      0.417      0.412
remove_copy_if(1S hit)              1.60        1.73        1.08      0.321      0.555      0.515
remove_copy_if(2S hit)              1.35        1.32        0.98      0.290      0.383      0.392
remove_copy_if(1+2S hit)            1.91        1.73        0.90      0.312      0.539      0.596
remove_copy_if(1S miss)             1.62        1.56        0.96      0.233      0.365      0.378
remove_copy_if(2S miss)             1.83        1.81        0.99      0.194      0.351      0.356
remove_copy_if(1+2S miss)           2.12        3.46        1.63      0.195      0.675      0.415
swap_ranges(1S)                     4.37        4.36        1.00      0.087      0.378      0.378
swap_ranges(2S)                     5.36        5.36        1.00      0.086      0.463      0.463
swap_ranges(1+2S)                   7.21        7.16        0.99      0.100      0.715      0.721
transform(1S)                       6.55        6.55        1.00      0.049      0.322      0.322
transform(2S)                       7.20        7.23        1.00      0.047      0.342      0.341
transform(1+2S)                     7.20        7.97        1.11      0.059      0.473      0.427
-------------------------------------------------------------------------------------------------
group geomean                       3.14        3.17        1.01       [5MyInt]
```

## Column semantics — verified in source, not assumed

`print_ratio` (bench line 668–684) receives `(std_ns, seg_ns, nsg_ns)` and prints
`nsg/seg`, `std/seg`, `std/nsg`, then `seg ns`, `std ns`, `nsg ns`.
`nsg_seg_ratio = nsg_ns / seg_ns`, so **`nsg/seg < 1` means the segmented path was slower**.
`bench_copy_if` (line 1617) builds three functors: `std_copy_if` (= `bench_detail::copy_if`),
`seg_copy_if<…,false>` (seg) and `seg_copy_if<…,true>` (nsg, via
`iter_w<true>::wrap` → `bc::wrapped_iterator`). Confirmed.

---

## 2. Pinned medians (5 runs, `taskset -c 3`), snapshot builds

Noise floor measured on this machine, from the 5-run min/max spread of the `nsg ns` column
for the copy rows: **0.3 % – 7.5 %, typically ≤ 4 %**. I therefore call a row an **outlier
only at > 10 % deviation**, comfortably clear of placement noise.

**Outlier criteria** (both are ratios that ought to sit at ≈1.00 by construction):
* **O1 — `nsg/seg < 0.90`**: the segmented path is >10 % slower than the library's *own*
  flat path on the same data. Structural regression in the segmented walker/leaf.
* **O2 — `std/nsg < 0.90`**: the library's flat path is >10 % slower than `std::` on
  *bit-identical iterators*. Pure codegen defect in the leaf, no algorithmic excuse.

### `gcc_off` — the shipping configuration after the user's edit

```
< algo >                       <nsg/seg>   <std/seg>   <std/nsg>   <seg ns>   <std ns>   <nsg ns>
copy(1S)                            2.34        2.34        1.00      0.107      0.250      0.250
copy(2S)                            5.21        5.21        1.00      0.052      0.271      0.271
copy(1+2S)                          5.97        5.95        1.00      0.063      0.375      0.376
copy_if(1S hit)                     1.49        1.49        1.00      0.306      0.457      0.457
copy_if(2S hit)                     1.41        1.45        1.03      0.330      0.479      0.464
copy_if(1+2S hit)                   1.29        1.27        0.98      0.396      0.501      0.510
copy_if(1S miss)                    1.34        1.36        1.01      0.373      0.506      0.500
copy_if(2S miss)                    0.90        0.90        1.00      0.257      0.231      0.231   <-- O1
copy_if(1+2S miss)                  1.04        1.17        1.12      0.309      0.360      0.322
copy_n(1S)                          5.50        5.19        0.94      0.052      0.270      0.286
copy_n(2S)                          5.39        5.02        0.93      0.051      0.256      0.275
copy_n(1+2S)                        6.40        7.79        1.22      0.063      0.491      0.403
remove_copy(1S hit)                 1.82        1.82        1.00      0.256      0.467      0.467
remove_copy(2S hit)                 1.73        1.72        0.99      0.260      0.446      0.449
remove_copy(1+2S hit)               1.77        1.79        1.01      0.273      0.488      0.483
remove_copy(1S miss)                1.87        1.86        1.00      0.249      0.464      0.466
remove_copy(2S miss)                1.34        1.36        1.01      0.259      0.352      0.348
remove_copy(1+2S miss)              1.61        1.66        1.03      0.268      0.446      0.432
remove_copy_if(1S hit)              1.15        1.55        1.35      0.428      0.663      0.492
remove_copy_if(2S hit)              1.27        1.25        0.98      0.382      0.477      0.485
remove_copy_if(1+2S hit)            1.10        1.19        1.08      0.445      0.528      0.490
remove_copy_if(1S miss)             1.76        1.77        1.00      0.248      0.439      0.437
remove_copy_if(2S miss)             1.31        1.33        1.02      0.257      0.343      0.337
remove_copy_if(1+2S miss)           1.66        1.73        1.04      0.263      0.454      0.436
swap_ranges(1S)                     3.25        3.25        1.00      0.102      0.332      0.332
swap_ranges(2S)                     4.04        4.43        1.10      0.084      0.372      0.339
swap_ranges(1+2S)                   4.02        4.05        1.01      0.099      0.401      0.398
transform(1S)                       3.44        3.27        0.95      0.106      0.347      0.365
transform(2S)                       5.89        5.89        1.00      0.054      0.318      0.318
transform(1+2S)                     6.73        6.75        1.00      0.063      0.425      0.424
```

**GCC outliers: exactly one — `copy_if(2S miss)`, `nsg/seg = 0.90`** (seg 0.257 vs nsg 0.231,
seg 11 % slower). `copy_n(1S/2S)` at `std/nsg` 0.94/0.93 and `transform(1S)` at 0.95 are
inside the ±7.5 % noise band and are **not** claimed as outliers.

### `clang_off` — the shipping configuration, unchanged by the user's edit

```
< algo >                       <nsg/seg>   <std/seg>   <std/nsg>   <seg ns>   <std ns>   <nsg ns>
copy(1S)                            5.40        5.40        1.00      0.050      0.270      0.270
copy(2S)                            4.81        4.81        1.00      0.048      0.231      0.231
copy(1+2S)                          7.05        7.15        1.01      0.059      0.422      0.416
copy_if(1S hit)                     1.70        1.66        0.98      0.248      0.411      0.421
copy_if(2S hit)                     1.00        1.00        1.00      0.229      0.229      0.228
copy_if(1+2S hit)                   2.80        2.79        1.00      0.191      0.533      0.535
copy_if(1S miss)                    5.15        2.54        0.49      0.110      0.279      0.566   <-- O2, headline
copy_if(2S miss)                    2.33        2.29        0.98      0.082      0.188      0.191
copy_if(1+2S miss)                  6.08        6.06        1.00      0.089      0.539      0.541
copy_n(1S)                          6.69        5.12        0.77      0.051      0.261      0.341   <-- O2
copy_n(2S)                          7.23        8.85        1.22      0.047      0.416      0.340
copy_n(1+2S)                        8.19        7.74        0.94      0.057      0.441      0.467
remove_copy(1S hit)                 1.71        2.30        1.34      0.241      0.554      0.412
remove_copy(2S hit)                 1.35        1.35        1.00      0.222      0.300      0.300
remove_copy(1+2S hit)               1.94        1.95        1.00      0.237      0.461      0.460
remove_copy(1S miss)                1.58        2.22        1.40      0.243      0.540      0.385
remove_copy(2S miss)                1.58        1.58        1.00      0.222      0.350      0.350
remove_copy(1+2S miss)              1.85        1.84        0.99      0.225      0.414      0.417
remove_copy_if(1S hit)              1.62        1.76        1.09      0.319      0.563      0.517
remove_copy_if(2S hit)              1.33        1.29        0.98      0.295      0.382      0.391
remove_copy_if(1+2S hit)            1.92        1.73        0.90      0.311      0.539      0.596   <-- borderline O2
remove_copy_if(1S miss)             1.62        1.57        0.97      0.233      0.366      0.377
remove_copy_if(2S miss)             1.85        1.84        0.99      0.192      0.353      0.355
remove_copy_if(1+2S miss)           2.15        3.44        1.60      0.196      0.675      0.422
swap_ranges(1S)                     4.34        4.40        1.01      0.087      0.383      0.378
swap_ranges(2S)                     5.40        5.40        1.00      0.086      0.464      0.464
swap_ranges(1+2S)                   7.20        7.16        0.99      0.099      0.709      0.713
transform(1S)                       6.46        6.44        1.00      0.050      0.322      0.323
transform(2S)                       6.82        6.84        1.00      0.050      0.342      0.341
transform(1+2S)                     7.23        8.07        1.12      0.060      0.484      0.434
```

**Clang outliers:**
1. **`copy_if(1S miss)` `std/nsg = 0.49`** — nsg 0.566 vs std 0.279 ns/elem, **2.03× slower**.
   This is the row the user flagged. Reproducible: 5-run spread only 2.5 % (0.557–0.571).
2. **`copy_n(1S)` `std/nsg = 0.77`** — nsg 0.341 vs std 0.261, 1.31× slower.
3. `remove_copy_if(1+2S hit)` `std/nsg = 0.90` — borderline, at the threshold.

Note the structural oddity in (1): the library's flat `copy_if` on a *miss* predicate
(0.566) is **slower than on a hit predicate** (0.421), despite performing zero stores.

---

## 3. Cause analysis, per compiler

### 3.1 Clang — `copy_if(1S miss)` nsg (the user's anomaly)

**Specialisation.** `measure_batch<seg_copy_if<deque<MyInt,…,deque_opt<0,128>>, vector<MyInt>, is_negative<MyInt>, true>, noop_reset>` at `0x1ea40`, size `0x977`.
Control: the `is_odd` (hit) twin at `0x17080` size `0x978`, and `std_copy_if` at `0x1cc40`.
The destination is a *wrapped* `vector` iterator ⇒ non-segmented ⇒
`segmented_copy_if_dst_dispatch` passes `unreachable_sentinel_t`, so the
`RADstIter`/`RADstIter` overload cannot match and the **generic leaf**
(`segmented_copy_if_dst_bounded`, `!DstTag::value`, `segmented_copy_if.hpp:46–76`) runs, with
both `dst_first == dst_last` guards folded away. `copy_if_cleanup_blocks<32>` is **not**
involved in this row.

**Library inner loop (`0x1ec80`, 8 Duff copies at 0xC0 stride, all 64-byte aligned):**

```
1ec80: mov    (%rdx),%r10d          ; *first
1ec83: test   %r10d,%r10d
1ec86: jns    1ec8f                 ; <=== TAKEN on every element in the miss case
1ec88: mov    %r10d,(%r9)           ; *dst_first = *first   (inline, never executed)
1ec8b: add    $0x4,%r9
1ec8f: add    $0x4,%rdx             ; ++first
1ec93: mov    (%rdi),%r10           ; reload deque block base from the node array
1ec96: add    %r8,%r10
1ec99: cmp    %r10,%rdx
1ec9c: je     1eca5                 ; block exhausted?
1ec9e: cmp    %rcx,%rdx
1eca1: jne    1ec80                 ; <=== TAKEN back-edge
```

**`std::copy_if` inner loop (`0x1ce80`):**

```
1ce80: mov    (%rdi),%r10d
1ce83: test   %r10d,%r10d
1ce86: js     1cec0                 ; NOT taken in the miss case — store is OUT OF LINE
1ce88: add    $0x4,%rdi
1ce8c: lea    0x200(%rdx),%r10      ; block base kept in a register, no reload
1ce93: cmp    %r10,%rdi
1ce96: je     1ced7
1ce98: cmp    %rcx,%rdi
1ce9b: jne    1ce80                 ; only TAKEN branch
...
1cec0: mov    %r10d,(%r9)           ; cold store block
```

Two differences: the library keeps the store **inline and branches over it**, and it
**reloads the block base every iteration**; clang sank `std::copy_if`'s store into a cold
block, which also freed the base to stay in a register.

The dominant term is the first one. **Taken branches per element** in the miss case are
**2** for the library loop (`jns` + back-edge) and **1** for `std::copy_if` (back-edge only).
Ratio 2 : 1, measured ratio 0.566 : 0.279 = **2.03 : 1**.

The same model explains why *miss is slower than hit*: with `is_odd`, the `jns`/`je` is taken
only ~50 % of the time, giving 1.5 taken branches/elem, and 1.5/2.0 = 0.75 vs measured
0.421/0.566 = 0.744.

**Controlled proof (`398_branchprobe.cpp`, `-O2`, `taskset -c 3`).** Both loop shapes were
replayed in inline asm, instruction-for-instruction, over a real 128-int-block node array,
with the predicate hit-rate swept. Shape A = library (branch **over** the store),
shape B = `std` (store sunk out of line). Their taken-branch counts move in *opposite*
directions with hit-rate, so the design is crossed:

```
shape  pct_neg   ns/elem   predicted taken-branches/elem
A        0%       1.130    2.0
B        0%       0.627    1.0
A       50%       0.756    1.5
B       50%       0.807    2.0
A      100%       0.745    1.0
B      100%       1.105    3.0
```

A gets **faster** as the hit rate rises (1.130 → 0.756 → 0.745) while B gets **slower**
(0.627 → 0.807 → 1.105), with identical data, identical memory traffic and identical work.
A/B at 0 % = **1.80×**, against 2.03× in the real benchmark. Taken-branch pressure is
confirmed as the dominant term. (The mapping is not a strict 1 taken-branch/cycle — B@100 %
predicts 3.0 but lands below A@0 %'s 2.0 — so I claim the *ordering and direction*, not an
exact cycle formula.)

**Placement excluded (g30 methodology).** Sweeping the loop head across all 16 four-byte
offsets in a 64-byte line, shape A at 0 %:

```
pad  0: 1.147   pad 16: 1.135   pad 32: 1.184   pad 48: 1.183
pad  4: 1.168   pad 20: 1.100   pad 36: 1.180   pad 52: 0.936
pad  8: 1.148   pad 24: 1.099   pad 40: 1.165   pad 56: 0.936
pad 12: 1.141   pad 28: 1.122   pad 44: 1.182   pad 60: 1.173
```

Total spread 0.936–1.184 = **1.26×**, versus the 1.80–2.03× effect being explained. Both real
loops are 64-byte aligned at the head and ≤50 bytes long (single fetch line). Placement is a
real but second-order effect here and cannot account for the anomaly.

**Did disabling the pragmas cause this?** No — clang was already in the no-pragma
configuration via the `BOOST_CLANG` carve-out at line 404–406. The anomaly pre-dates the
user's edit. But the pragma *would* have masked it: rebuilding clang with
`ENABLE_PRAGMA_UNROLL` drops this row from **0.566 → 0.312 ns** (1.81× better), because
`#pragma unroll 4` amortises both the back-edge and the block-base reload.

`copy_n(1S)` (`std/nsg` 0.77) is the same family: the library's flat `copy_n` leaf keeps a
per-element branch that `std::copy_n` does not. Not separately micro-benchmarked — see
not-verified list.

### 3.2 GCC — `copy_if(2S miss)` seg, and the g31 dead Duff ladder

**Specialisation.** GCC out-lines the walker: `measure_batch<seg_copy_if<vector, deque,
is_negative, false>>.isra.0` calls
`segmented_copy_if_dst_dispatch<vec_iterator<MyInt*,true>, …, deque_iterator<MyInt*,false,0u,128u,…>, is_negative<MyInt>, random_access_iterator_tag>.isra.0`.
Source is a flat `vector`, destination is a real segmented `bc::deque`, so this row *does*
go through `copy_if_cleanup_blocks<32>`.

**With pragmas ON (`gcc_on`, = the g31 configuration), callee at `0x3f00`, size `0x3ce`
(974 bytes), 274 instructions.** The g31 finding reproduces exactly:

```
3f90: mov %rdi,%rax        3fa9: and $0x7,%eax        <- unroll-remainder computation
3f93: sub $0x20,%r9        3fac: je  4080
3f97: mov %r8,%rcx         3fb2: cmp $0x1,%rax ; je 4037   \
3f9a: sub %r8,%rax         3fb8: cmp $0x2,%rax ; je 4027    |
3f9d: sub $0x4,%rax        3fbe: cmp $0x3,%rax ; je 4017    |  six-way Duff ladder,
3fa1: shr $0x2,%rax        3fc4: cmp $0x4,%rax ; je 4007    |  provably dead
                           3fca: cmp $0x5,%rax ; je 3ff7    |
                           3fd0: cmp $0x6,%rax ; je 3fe7   /
```

`rax = (rdi − r8 − 4)/4 + 1` with `rdi = r8 + 0x80` ⇒ `rax = 32`, and `32 & 7 == 0`, so the
`je 4080` at `0x3fac` always fires and all six `cmp`/`je` pairs are unreachable. Together
with the `0x7c` destination-space guard this is **24 prologue instructions per 32 elements**
— exactly g31's count. The real work is an 8-way unrolled body at `0x4080`–`0x40f2`.

**With pragmas OFF (`gcc_off`), callee at `0x3500`, size `0x17a` (378 bytes), 97
instructions.**

```
3580: mov %r9,%rax ; sub %rcx,%rax ; cmp $0x7c,%rax ; jle 35e3   <- dest-space guard
358c: sub $0x20,%r8
3590: mov %rdi,%rax
35c0: mov (%rax),%edx        \
35c2: test %edx,%edx          |
35c4: jns 35cc                |  plain scalar loop, NOT unrolled:
35c6: mov %edx,(%rcx)         |  2 taken branches/element in the miss case
35c8: add $0x4,%rcx           |
35cc: add $0x4,%rax           |
35d0: cmp %rax,%rsi           |
35d3: jne 35c0               /
35d5: sub $-0x80,%rdi ; sub $-0x80,%rsi ; cmp $0x1f,%r8 ; jg 3580
```

**The dead six-way Duff ladder is gone.** The prologue dropped from 24 instructions per 32
elements to 6, and the whole callee shrank **2.6×** (974 → 378 bytes). g31's cause 1 is
eliminated as a code-size and prologue problem.

**But the row still got slower**, because the pragma was also providing the 8-way body:
per element the unrolled body costs `8×(mov,test,jns-taken)` + `lea,cmp,jne` = **1.125 taken
branches/elem**, while the un-unrolled loop costs **2.0**. Measured `seg`: 0.175 (on) →
0.257 (off) = 1.47× worse; model predicts 1.78×. Same mechanism as the clang anomaly:
branch-over-store dominates once the unroll is removed.

Both compilers therefore hit **one root cause** — the leaves branch over the store on the
hot path — which explicit unrolling was hiding.

---

## 4. Was disabling the pragmas net positive?

Geomean of the 5-run median `seg ns` and `nsg ns` over all 30 group-25 rows:

| config | geomean `seg ns` | geomean `nsg ns` |
|---|---|---|
| `gcc_on` (previous GCC default) | **0.1358** | 0.3556 |
| `gcc_off` (**after user's edit**) | 0.1700 (+25.2 %) | 0.3855 (+8.4 %) |
| `gcc_proto` (§5) | 0.1514 | **0.3494** |
| `clang_on` | **0.1215** | **0.3195** |
| `clang_off` (default before *and* after) | 0.1246 (+2.6 %) | 0.3902 (+22.1 %) |
| `clang_proto` (§5) | 0.1233 | 0.3653 |

`copy_if` rows only:

| config | geomean `seg ns` | geomean `nsg ns` |
|---|---|---|
| `gcc_on` | **0.1691** | 0.3189 |
| `gcc_off` | 0.3253 (**+92.4 %**) | 0.3988 |
| `gcc_proto` | 0.1834 | **0.2546** |
| `clang_on` | **0.1291** | 0.2570 |
| `clang_off` | 0.1434 (+11.1 %) | 0.3798 |
| `clang_proto` | 0.1347 | 0.2792 |

### Per-row before/after, GCC, `seg ns` (the column the library controls)

| row | `gcc_on` (≈g31) | `gcc_off` (now) | change |
|---|---|---|---|
| copy_if(1S hit) | 0.136 | 0.306 | **2.25× worse** |
| copy_if(2S hit) | 0.152 | 0.330 | **2.17× worse** |
| copy_if(1+2S hit) | 0.169 | 0.396 | **2.34× worse** |
| copy_if(1S miss) | 0.221 | 0.373 | 1.69× worse |
| copy_if(2S miss) | 0.175 | 0.257 | 1.47× worse |
| copy_if(1+2S miss) | 0.173 | 0.309 | 1.79× worse |
| remove_copy_if(1S hit) | 0.310 | 0.428 | 1.38× worse |
| remove_copy_if(2S hit) | 0.246 | 0.382 | 1.55× worse |
| remove_copy_if(1+2S hit) | 0.268 | 0.445 | 1.66× worse |
| remove_copy(1S hit/miss) | 0.233 | 0.256 / 0.249 | ~1.08× worse |
| remove_copy(2S hit/miss) | 0.225 | 0.260 / 0.259 | ~1.15× worse |
| swap_ranges(1S) | 0.084 | 0.102 | 1.21× worse |
| copy(1+2S) | 0.057 | 0.063 | 1.11× worse |
| transform(1+2S) | 0.066 | 0.063 | 1.05× **better** |
| copy(1S), copy_n(*), transform(1S/2S) | — | — | unchanged (≤2 %) |

**Verdict: net negative, decisively, and GCC-only.**
* **GCC: net negative.** `seg` geomean +25 %, `copy_if` `seg` geomean +92 %, with only one
  row (transform(1+2S), +5 %) improving. The one genuine benefit — the dead Duff ladder and a
  2.6× smaller callee — did not pay for the loss of the unrolled body.
* **Clang: no change whatsoever.** The `BOOST_CLANG` carve-out at line 404–406 already
  selected `DISABLE`, so `clang_off` is both the before and the after state. Separately, the
  `clang_on` column shows clang's *existing* default is leaving 22 % on the `nsg` geomean and
  11 % on the `copy_if` `seg` geomean — a pre-existing issue, not something this edit caused.

---

## 5. Prototype: source-level unroll, no pragmas

Applied to `snap_proto/boost/container/experimental/segmented_copy_if.hpp` only
(script `401_proto.sh`; the real header is untouched). C++03-clean: no `auto`, lambdas,
range-`for`, `nullptr` or variadics. Recursion on both input and output sides is untouched —
only the two innermost leaf loop bodies change, and the walkers are not force-inlined.

**P2 — generic (non-segmented destination) leaf, 4× manual unroll.** Preserves the
conditional-write fix exactly: the `dst_first == dst_last` test still happens after every
single write, and `++first; goto out_path` still guarantees the standard's
`last - first` predicate-application count.

```cpp
   #define BOOST_CONTAINER_SEG_COPY_IF_STEP()          \
      if(pred(*first)) {                               \
         *dst_first = *first;                          \
         ++dst_first;                                  \
         if(BOOST_UNLIKELY(dst_first == dst_last)) {   \
            ++first;                                   \
            goto out_path;                             \
         }                                             \
      }                                                \
      ++first;

   while(first != last) {
      BOOST_CONTAINER_SEG_COPY_IF_STEP()
      if(first == last) break;
      BOOST_CONTAINER_SEG_COPY_IF_STEP()
      if(first == last) break;
      BOOST_CONTAINER_SEG_COPY_IF_STEP()
      if(first == last) break;
      BOOST_CONTAINER_SEG_COPY_IF_STEP()
   }
   #undef BOOST_CONTAINER_SEG_COPY_IF_STEP
```

**P1 — `copy_if_cleanup_blocks`, 4× manual unroll** of the fixed-size block loop
(`BlockSize` = 32, with a remainder loop so any block size stays correct).

### Why it works — codegen

Clang's prototype loop for the anomalous row (`clang_proto`, `0x20b00`–`0x20ba4`):

```
20b4f: cmp %rcx,%rsi ; je 20c8b
20b58: mov (%rsi),%r10d ; test %r10d,%r10d
20b5e: js  20c00              <- NOT taken in the miss case: store sunk OUT OF LINE
20b64: add $0x4,%rsi ; lea 0x200(%r9),%r10 ; cmp %r10,%rsi ; je 20c1b
20b78: cmp %rcx,%rsi ; je 20c8b
20b81: mov (%rsi),%r10d ; test %r10d,%r10d
20b87: js  20c40              <- NOT taken
20b8d: add $0x4,%rsi ; add $0x200,%r9 ; cmp %r9,%rsi ; je 20c5b
20ba1: cmp %rcx,%rsi
20ba4: jne 20b00              <- only taken branch, once per 4 elements
```

The manual unroll made clang **sink the stores out of line** — the loop adopts exactly the
`std::copy_if` shape (shape B) that the microbenchmark showed is optimal at low hit rates.
Taken branches/elem in the miss case drop from **2.0 → 0.25**, and the block base is no
longer reloaded per element.

### Measured effect (5-run medians, snapshot builds)

| row / column | `clang_off` | `clang_proto` | |
|---|---|---|---|
| **copy_if(1S miss) `nsg`** | **0.566** | **0.240** | **2.36× faster**; `std/nsg` 0.49 → 1.15 |
| copy_if(1S hit) `nsg` | 0.421 | 0.342 | 1.23× faster |
| copy_if(1+2S hit) `nsg` | 0.535 | 0.308 | 1.74× faster |
| copy_if(1+2S miss) `nsg` | 0.541 | 0.306 | 1.77× faster |
| copy_if(2S miss) `nsg` | 0.191 | 0.232 | 1.21× **slower** |
| copy_if(2S hit) `nsg` | 0.228 | 0.264 | 1.16× **slower** |
| clang `nsg` geomean (30 rows) | 0.3902 | 0.3653 | 6.4 % better |
| clang `seg` geomean (30 rows) | 0.1246 | 0.1233 | flat |

| row / column | `gcc_off` | `gcc_proto` | `gcc_on` (reference) |
|---|---|---|---|
| copy_if(1S hit) `seg` | 0.306 | 0.224 | 0.136 |
| copy_if(2S hit) `seg` | 0.330 | **0.152** | 0.152 |
| copy_if(1+2S hit) `seg` | 0.396 | **0.160** | 0.169 |
| copy_if(1S miss) `seg` | 0.373 | 0.250 | 0.221 |
| copy_if(2S miss) `seg` | 0.257 | **0.159** | 0.175 |
| copy_if(1+2S miss) `seg` | 0.309 | 0.176 | 0.173 |
| copy_if(2S miss) `nsg/seg` | 0.90 (O1) | 0.97 | 0.66 |
| gcc `copy_if` `seg` geomean | 0.3253 | **0.1834** | 0.1691 |
| gcc `seg` geomean (30 rows) | 0.1700 | **0.1514** | 0.1358 |
| gcc `nsg` geomean (30 rows) | 0.3855 | **0.3494** | 0.3556 |

The prototype recovers **44 %** of the GCC `copy_if` `seg` loss without any pragma, matches
or beats `gcc_on` on three of the six `copy_if` rows, and beats `gcc_on` outright on the
`nsg` geomean. GCC's `copy_if(2S miss)` O1 outlier moves 0.90 → 0.97, inside noise.

### Correctness gate

`segmented_copy_if_test.cpp` (copied into the g34 area, md5 `9fdab3b918f1fdeb859996ee176b04e4`),
`-std=c++20 -O2 -DNDEBUG`:

```
  g++-16     / snap       : test exit=0
  g++-16     / snap_proto : test exit=0
  clang++-22 / snap       : test exit=0
  clang++-22 / snap_proto : test exit=0
```

---

## 6. Ranked recommendations

1. **Revert `BOOST_CONTAINER_SEGMENTED_DISABLE_PRAGMA_UNROLL` at line 396** (back to
   commented out), restoring the per-compiler default block. This is the single largest
   effect measured: GCC `copy_if` `seg` geomean 0.3253 → 0.1691 (**1.92× faster**), overall
   GCC `seg` geomean 0.1700 → 0.1358 (**+25 %**). It costs nothing on clang, whose behaviour
   the flag does not change. Do this first, independently of everything else.

2. **Adopt the P2 source-level 4× unroll in the generic `copy_if` leaf** (§5). This is the
   real fix and matches the standing no-pragma policy. It resolves the clang
   `copy_if(1S miss)` `nsg` outlier (2.36× faster, `std/nsg` 0.49 → 1.15), improves the
   `nsg` geomean on both compilers, and — uniquely — beats the pragma build on GCC's `nsg`
   geomean. Caveat to weigh: clang's `copy_if(2S)` `nsg` rows regress 16–21 %
   (0.191 → 0.232, 0.228 → 0.264); those are the pointer-source cases where the extra
   `first == last` tests are not amortised. Net across the 30 rows is still positive.
   If (1) is also applied, P2's `AUTO_UNROLL`/`UNROLL(4)` interaction should be re-measured —
   I measured P2 only against the pragma-disabled snapshot.

3. **Apply the same 4× shape to `remove_copy` and `remove_copy_if`.** Their leaves are
   structurally identical (`segmented_remove_copy.hpp:52`, `segmented_remove_copy_if.hpp:65`,
   same guard + `goto out_path`), and they show the same GCC regression pattern
   (remove_copy_if(1+2S hit) `seg` 0.268 → 0.445). Not prototyped or measured — see
   not-verified list.

4. **Investigate `copy_n(1S)` on clang** (`std/nsg` 0.77, nsg 0.341 vs std 0.261).
   Same family of defect, not analysed here.

5. **Do not pursue branchless conditional-store** (`*dst = *cur; dst += pred(*cur);`), the
   obvious way to remove the branch entirely. It writes past the returned output iterator,
   which contradicts both `[alg.copy]` and the conditional-write fix just landed in
   commit `6a1a7d6`. Recorded so it is not re-proposed.

I am **not** recommending re-enabling a pragma anywhere beyond item (1), which is a revert
rather than a new pragma. Item (2) achieves most of the same benefit at source level.

---

## 7. Element-type coverage

`main()` (bench lines 2553–2560):

```cpp
int main()
{
   //run_benchmarks<int>();
   run_benchmarks<MyInt>();
   //run_benchmarks<MyFatInt<4> >();
   //run_benchmarks<MyFatInt<8> >();
   return 0;
}
```

**Only `MyInt` (a 4-byte `int` wrapper) is measured.** The `MyFatInt<4>` and `MyFatInt<8>`
rows are commented out, as is plain `int`. Every number in this report is `MyInt`-only.
This matters for the conclusions: the taken-branch model is a front-end effect and the
element is small enough that the loops are not memory-bound. With `MyFatInt<8>` (32 bytes)
the copy itself would cost more and the relative weight of the branch-over-store would fall,
so the size of these effects — though not, I expect, their direction — would change. I have
**not** measured any fat-element configuration.

---

## 8. Not verified / explicitly out of scope

* **No PMU counters.** WSL1 exposes no `perf` hardware events, so branch-mispredict,
  taken-branch and front-end-stall counts are **not** measured anywhere in this report. All
  front-end attribution comes from the controlled asm microbenchmark in §3.1 plus instruction
  counting from disassembly. The taken-branch model is supported by direction, ordering and
  the crossed A/B design; it is **not** established as an exact cycle-accurate formula
  (shape B at 100 % predicts 3.0 taken branches/elem but measures faster than shape A at
  2.0).
* `copy_n(1S)` (clang `std/nsg` 0.77) and `remove_copy_if(1+2S hit)` (0.90) are **reported as
  outliers but not root-caused**. No disassembly was taken for them.
* Recommendation (3) — applying the P2 shape to `remove_copy`/`remove_copy_if` — is
  **not prototyped, not built, not measured, not tested**. It is an inference from source
  similarity only.
* The prototype was measured **only against the pragma-disabled snapshot**. I did not build
  P2 combined with `ENABLE_PRAGMA_UNROLL`, so the interaction in recommendation (1)+(2) is
  unmeasured.
* Only `segmented_copy_if_test.cpp` was run against the prototype. The rest of the
  `segmented_*_test.cpp` suite was **not** run — the prototype only touches
  `segmented_copy_if.hpp`, but that is an argument, not a test result.
* The `gcc_on`/`clang_on` columns are **my own fresh builds** from the snapshot with
  `ENABLE_PRAGMA_UNROLL`; they are not the literal g31 numbers. `gcc_on` copy_if(2S miss)
  (`seg` 0.175, `nsg` 0.116, ratio 0.66) is close to but not identical with g31's reported
  0.159/0.115 — the conditional-write fix landed in between, so the two are not expected to
  match exactly and I did not attempt to reproduce the g31 build.
* The alignment sweep in §3.1 covers the **microbenchmark** loop, not the real benchmark's
  loops. I did not perform a controlled-offset sweep inside `bench_segmented_algos.cpp`.
* Single-socket, single-core (`taskset -c 3`), no `sudo`, so no frequency pinning, no
  C-state or turbo control. Run-to-run spread was 0.3–7.5 %; claims below 10 % are not made.
* `MyFatInt` rows: not measured (see §7).
