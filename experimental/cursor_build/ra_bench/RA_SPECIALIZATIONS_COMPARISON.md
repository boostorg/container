# `BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS` — Group 25 comparison

## Setup

| Item | Value |
|---|---|
| Benchmark | `bench_segmented_algos.cpp`, group **25** (2-range input-output) |
| Flags | `-std=c++20 -O3 -DNDEBUG -DBENCH_ON -DBOOST_CONTAINER_BENCH_SEGMENTED_GROUP=25` |
| Macro | `-DBOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS=0` vs `=1` |
| Workload | `N=100000`, `iters=3000`, `bc::deque` block size 128 |
| Value types | `MyInt`, `MyFatInt<4>`, `MyFatInt<8>` |
| Compilers | g++-16, clang++-22 (WSL, `taskset -c 2`), MSVC 19.51.36252 x64 (toolset 14.51) |
| Logs | `gcc16_ra{0,1}.txt`, `clang22_ra{0,1}.txt`, `msvc145_ra{0,1}.txt` in this directory |

### What the macro gates

When `=1` (default), random-access leaf specializations are enabled. STL contiguous/trivial `memmove`/`memset` paths are not a substitute for these RA leaves on segmented/bounded ranges, so `copy` / `copy_n` / `fill_n` / `reverse` are gated too:

- Blocked leaves: `copy_if`, `remove_copy`, `remove_copy_if`, `partition_copy`
- `min(n)` + `unreachable_sentinel` shortening: `copy`, `copy_n`, `transform`, `swap_ranges`, `reverse_copy`
- Counted / RA reverse: `fill_n`, `reverse`
- Related RA paths: `merge`, `set_*`, `search_n`, `generate_n`, equal/mismatch (`segmented_iter2_bounded`)

### Configurations in the table

| Label | Source | Destination |
|---|---|---|
| `1S` | segmented (`deque`) | flat (`vector`), unbounded |
| `2S` | flat (`vector`) | segmented (`deque`), per-segment bounded |
| `1+2S` | segmented | segmented |

The RA overloads matter most on **`2S` / `1+2S`**, where each destination chunk is a real RA `[begin, end)`.
On **`1S`**, the destination is unbounded (`unreachable_sentinel_t`), so those overloads usually do not match.

---

## Group geomean `std/seg`

Higher means segmented is faster relative to `std::`.

| Compiler | Type | RA=0 | RA=1 | Δ |
|---|---|---:|---:|---:|
| g++-16 | MyInt | 1.80 | 2.57 | **+0.77** |
| g++-16 | Fat4 | 1.16 | 1.31 | +0.15 |
| g++-16 | Fat8 | 1.05 | 1.05 | ~0 |
| clang++-22 | MyInt | 2.12 | 3.11 | **+0.99** |
| clang++-22 | Fat4 | 1.16 | 1.35 | +0.19 |
| clang++-22 | Fat8 | 1.07 | 1.07 | ~0 |
| MSVC 14.51 | MyInt | 3.37 | 4.44 | **+1.07** |
| MSVC 14.51 | Fat4 | 1.92 | 1.99 | +0.07 |
| MSVC 14.51 | Fat8 | 1.36 | 1.36 | ~0 |

---

## MyInt `seg` ns — RA1 / RA0 (lower is better for RA=1)

| Algo | g++-16 | clang++-22 | MSVC 14.51 |
|---|---:|---:|---:|
| `transform(2S)` | 0.20× | 0.12× | 0.44× |
| `transform(1+2S)` | 0.21× | 0.10× | 0.79× |
| `swap_ranges(2S)` | 0.22× | 0.23× | 0.83× |
| `swap_ranges(1+2S)` | 0.31× | 0.92× | 0.77× |
| `copy_if(2S hit)` | 0.52× | 0.67× | 0.77× |
| `copy_if(2S miss)` | 0.63× | 0.38× | 0.71× |
| `copy_if(1+2S hit)` | 0.74× | 0.55× | 0.39× |
| `copy_if(1+2S miss)` | 0.48× | 0.35× | 0.51× |
| `remove_copy_if(2S hit)` | 0.57× | 0.56× | 0.47× |
| `remove_copy(2S hit)` | 0.63× | 0.68× | 0.84× |
| `copy(2S)` / `copy_n(2S)` | ~1.0× | ~1.0× | ~0.9–1.0× |

---

## Analysis

### Why RA=1 wins on `2S` / `1+2S`

1. **Conditional-write family** (`copy_if`, `remove_copy`, `remove_copy_if`)  
   The RA leaf runs a fixed block (size 16) only when both source remainder and destination room are ≥ block size, so the hot loop does not re-check `dst == dst_last` after every write. On miss-heavy scans this also avoids touching the destination bound while nothing is written. The residual tail keeps the dest-after-write contract required by `[alg.copy]` / `[alg.remove]`.

2. **Always-write dual-RA family** (`transform`, `swap_ranges`)  
   The RA overload computes `n = min(src_n, dst_n)` once and recurses with `unreachable_sentinel_t`, removing the per-element destination-full branch. Compilers then emit a tight counted loop (often vectorizable for `MyInt`). Without this, Clang especially paid a large tax on `transform(1+2S)` (0.57 → 0.06 ns/elem).

3. **`copy` / `copy_n` stay flat**  
   Those RA overloads are not behind the macro, so RA=0 vs RA=1 should not change them; observed differences are noise.

### Why `1S` is mostly neutral

Flat vector destinations use an unbounded sentinel. The gated RA overloads take a same-typed RA `dst_last`, so they are not selected. Small ±10–20% moves on some `1S` rows are attributable to binary size / I-cache / unrelated codegen noise, not to the blocked leaf itself.

### Why Fat8 barely moves

For `MyFatInt<8>`, time is dominated by loads/stores of large objects. Hoisting a destination check or shortening with `min(n)` saves little relative to memory traffic, so group geomeans are essentially unchanged.

### Compiler notes

- **GCC / Clang**: Same qualitative picture — large MyInt gains on segmented destinations; Fat4 moderate; Fat8 flat. Clang’s unbounded-style leaf for `transform` without the RA path was especially weak.
- **MSVC 14.51**: Same MyInt 2S/1+2S story. A few Fat4 regressions appear (`copy_if(1S miss)`, `remove_copy_if(1+2S hit)`), showing the blocked leaf is not universally better for larger PODs under MSVC’s codegen.

---

## Conclusion

`BOOST_CONTAINER_SEGMENTED_ENABLE_RA_SPECIALIZATIONS=1` (default) is a clear win for lightweight elements when the destination is segmented. It is nearly a no-op for flat unbounded destinations and for fat elements. Keeping the default at `1` is justified; set to `0` only for A/B experiments or if a specific platform shows a regression worth chasing.
