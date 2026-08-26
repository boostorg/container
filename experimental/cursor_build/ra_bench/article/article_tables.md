# Generated tables

## Benchmark 1 (RA=0): compiler geomeans by shape

T = `MyInt`, shape = `1S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.55 | 1.61 | 1.04 |
| Clang 22 | 3.07 | 2.97 | 0.97 |
| MSVC 2026 | 5.12 | 5.15 | 1.00 |

T = `MyFatInt<4>`, shape = `1S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 0.96 | 0.97 | 1.01 |
| Clang 22 | 1.38 | 1.23 | 0.90 |
| MSVC 2026 | 2.82 | 2.80 | 0.99 |

T = `MyFatInt<8>`, shape = `1S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.03 | 1.03 | 1.00 |
| Clang 22 | 1.06 | 1.03 | 0.96 |
| MSVC 2026 | 1.81 | 1.75 | 0.97 |

T = `MyInt`, shape = `2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.05 | 1.02 | 0.98 |
| Clang 22 | 0.77 | 0.83 | 1.08 |
| MSVC 2026 | 1.86 | 1.88 | 1.01 |

T = `MyFatInt<4>`, shape = `2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.04 | 1.04 | 1.00 |
| Clang 22 | 1.03 | 1.04 | 1.01 |
| MSVC 2026 | 1.42 | 1.35 | 0.96 |

T = `MyFatInt<8>`, shape = `2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.02 | 1.02 | 1.00 |
| Clang 22 | 1.01 | 1.02 | 1.00 |
| MSVC 2026 | 1.23 | 1.19 | 0.97 |

T = `MyInt`, shape = `1+2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.36 | 1.36 | 1.00 |
| Clang 22 | 1.97 | 2.04 | 1.03 |
| MSVC 2026 | 3.76 | 3.64 | 0.97 |

T = `MyFatInt<4>`, shape = `1+2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.30 | 1.36 | 1.04 |
| Clang 22 | 1.27 | 1.25 | 0.99 |
| MSVC 2026 | 2.44 | 1.94 | 0.79 |

T = `MyFatInt<8>`, shape = `1+2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.08 | 1.09 | 1.01 |
| Clang 22 | 1.16 | 1.15 | 0.99 |
| MSVC 2026 | 1.75 | 1.25 | 0.71 |

## Benchmark 1 (RA=0): per-algorithm by shape, cross-compiler geomean

T = `MyInt`, shape = `1S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 4.56 | 4.70 | 1.03 |
| `copy_if(hit)` | 1.90 | 1.88 | 0.99 |
| `copy_if(miss)` | 2.94 | 2.24 | 0.76 |
| `copy_n` | 2.92 | 2.84 | 0.97 |
| `remove_copy(hit)` | 2.32 | 2.74 | 1.18 |
| `remove_copy(miss)` | 2.28 | 2.85 | 1.25 |
| `remove_copy_if(hit)` | 1.88 | 1.85 | 0.98 |
| `remove_copy_if(miss)` | 2.31 | 2.31 | 1.00 |
| `swap_ranges` | 4.64 | 4.47 | 0.96 |
| `transform` | 5.32 | 5.14 | 0.97 |
| **geomean** | **2.90** | **2.91** | **1.00** |

T = `MyFatInt<4>`, shape = `1S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.47 | 1.51 | 1.03 |
| `copy_if(hit)` | 1.85 | 1.79 | 0.97 |
| `copy_if(miss)` | 2.41 | 2.08 | 0.86 |
| `copy_n` | 1.31 | 1.31 | 1.00 |
| `remove_copy(hit)` | 1.43 | 1.38 | 0.97 |
| `remove_copy(miss)` | 1.43 | 1.40 | 0.98 |
| `remove_copy_if(hit)` | 1.47 | 1.39 | 0.95 |
| `remove_copy_if(miss)` | 1.50 | 1.41 | 0.94 |
| `swap_ranges` | 1.32 | 1.25 | 0.95 |
| `transform` | 1.60 | 1.63 | 1.02 |
| **geomean** | **1.55** | **1.50** | **0.97** |

T = `MyFatInt<8>`, shape = `1S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.17 | 1.17 | 1.00 |
| `copy_if(hit)` | 1.40 | 1.41 | 1.01 |
| `copy_if(miss)` | 1.77 | 1.59 | 0.90 |
| `copy_n` | 1.17 | 1.19 | 1.02 |
| `remove_copy(hit)` | 1.19 | 1.18 | 1.00 |
| `remove_copy(miss)` | 1.18 | 1.18 | 1.00 |
| `remove_copy_if(hit)` | 1.37 | 1.35 | 0.99 |
| `remove_copy_if(miss)` | 1.18 | 1.16 | 0.99 |
| `swap_ranges` | 1.11 | 1.01 | 0.90 |
| `transform` | 1.15 | 1.15 | 1.00 |
| **geomean** | **1.26** | **1.23** | **0.98** |

T = `MyInt`, shape = `2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.63 | 1.64 | 1.00 |
| `copy_if(hit)` | 0.93 | 0.92 | 0.99 |
| `copy_if(miss)` | 0.77 | 0.68 | 0.87 |
| `copy_n` | 1.43 | 1.59 | 1.12 |
| `remove_copy(hit)` | 0.96 | 0.95 | 1.00 |
| `remove_copy(miss)` | 1.02 | 1.01 | 0.99 |
| `remove_copy_if(hit)` | 0.97 | 1.00 | 1.03 |
| `remove_copy_if(miss)` | 1.04 | 1.28 | 1.23 |
| `swap_ranges` | 1.49 | 1.60 | 1.07 |
| `transform` | 1.56 | 1.47 | 0.94 |
| **geomean** | **1.14** | **1.17** | **1.02** |

T = `MyFatInt<4>`, shape = `2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.41 | 1.42 | 1.01 |
| `copy_if(hit)` | 1.02 | 0.98 | 0.97 |
| `copy_if(miss)` | 1.05 | 1.08 | 1.02 |
| `copy_n` | 1.17 | 1.30 | 1.11 |
| `remove_copy(hit)` | 1.16 | 1.18 | 1.02 |
| `remove_copy(miss)` | 1.04 | 1.07 | 1.03 |
| `remove_copy_if(hit)` | 1.12 | 1.07 | 0.96 |
| `remove_copy_if(miss)` | 1.22 | 1.16 | 0.96 |
| `swap_ranges` | 1.24 | 1.08 | 0.87 |
| `transform` | 1.09 | 1.08 | 0.99 |
| **geomean** | **1.15** | **1.14** | **0.99** |

T = `MyFatInt<8>`, shape = `2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.17 | 1.17 | 1.00 |
| `copy_if(hit)` | 1.07 | 1.07 | 1.00 |
| `copy_if(miss)` | 1.01 | 1.02 | 1.01 |
| `copy_n` | 1.17 | 1.18 | 1.02 |
| `remove_copy(hit)` | 1.05 | 1.05 | 1.01 |
| `remove_copy(miss)` | 1.06 | 1.06 | 1.00 |
| `remove_copy_if(hit)` | 1.08 | 1.07 | 0.99 |
| `remove_copy_if(miss)` | 1.07 | 1.08 | 1.00 |
| `swap_ranges` | 1.11 | 0.99 | 0.90 |
| `transform` | 1.05 | 1.05 | 1.00 |
| **geomean** | **1.08** | **1.07** | **0.99** |

T = `MyInt`, shape = `1+2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 3.54 | 3.54 | 1.00 |
| `copy_if(hit)` | 2.09 | 1.99 | 0.95 |
| `copy_if(miss)` | 2.07 | 2.01 | 0.97 |
| `copy_n` | 3.42 | 3.69 | 1.08 |
| `remove_copy(hit)` | 1.54 | 1.52 | 0.98 |
| `remove_copy(miss)` | 1.93 | 1.89 | 0.98 |
| `remove_copy_if(hit)` | 1.43 | 1.46 | 1.02 |
| `remove_copy_if(miss)` | 1.90 | 1.88 | 0.99 |
| `swap_ranges` | 3.20 | 3.30 | 1.03 |
| `transform` | 1.62 | 1.63 | 1.00 |
| **geomean** | **2.16** | **2.16** | **1.00** |

T = `MyFatInt<4>`, shape = `1+2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.47 | 1.73 | 1.18 |
| `copy_if(hit)` | 1.69 | 1.63 | 0.97 |
| `copy_if(miss)` | 2.47 | 1.61 | 0.65 |
| `copy_n` | 1.37 | 1.60 | 1.17 |
| `remove_copy(hit)` | 1.72 | 1.67 | 0.97 |
| `remove_copy(miss)` | 1.71 | 1.63 | 0.95 |
| `remove_copy_if(hit)` | 1.57 | 1.59 | 1.01 |
| `remove_copy_if(miss)` | 1.48 | 1.06 | 0.72 |
| `swap_ranges` | 1.23 | 1.14 | 0.93 |
| `transform` | 1.50 | 1.39 | 0.93 |
| **geomean** | **1.59** | **1.49** | **0.93** |

T = `MyFatInt<8>`, shape = `1+2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.19 | 1.08 | 0.90 |
| `copy_if(hit)` | 1.46 | 1.40 | 0.96 |
| `copy_if(miss)` | 1.96 | 1.31 | 0.67 |
| `copy_n` | 1.18 | 1.05 | 0.89 |
| `remove_copy(hit)` | 1.22 | 1.11 | 0.91 |
| `remove_copy(miss)` | 1.24 | 1.13 | 0.91 |
| `remove_copy_if(hit)` | 1.36 | 1.32 | 0.97 |
| `remove_copy_if(miss)` | 1.23 | 1.11 | 0.90 |
| `swap_ranges` | 1.16 | 1.06 | 0.90 |
| `transform` | 1.15 | 1.11 | 0.96 |
| **geomean** | **1.30** | **1.16** | **0.89** |

## Benchmark 2 (RA=1): compiler geomeans by shape

T = `MyInt`, shape = `1S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.92 | 1.87 | 0.97 |
| Clang 22 | 3.01 | 2.95 | 0.98 |
| MSVC 2026 | 5.29 | 5.27 | 1.00 |

T = `MyFatInt<4>`, shape = `1S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 0.99 | 1.05 | 1.06 |
| Clang 22 | 1.41 | 1.18 | 0.84 |
| MSVC 2026 | 2.65 | 2.64 | 0.99 |

T = `MyFatInt<8>`, shape = `1S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.01 | 1.02 | 1.01 |
| Clang 22 | 1.09 | 1.04 | 0.95 |
| MSVC 2026 | 1.79 | 1.74 | 0.97 |

T = `MyInt`, shape = `2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 2.78 | 2.81 | 1.01 |
| Clang 22 | 2.56 | 2.61 | 1.02 |
| MSVC 2026 | 2.59 | 2.59 | 1.00 |

T = `MyFatInt<4>`, shape = `2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.30 | 1.32 | 1.02 |
| Clang 22 | 1.24 | 1.30 | 1.05 |
| MSVC 2026 | 1.55 | 1.47 | 0.95 |

T = `MyFatInt<8>`, shape = `2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.03 | 1.03 | 1.00 |
| Clang 22 | 1.03 | 1.03 | 1.00 |
| MSVC 2026 | 1.19 | 1.14 | 0.96 |

T = `MyInt`, shape = `1+2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 3.24 | 3.24 | 1.00 |
| Clang 22 | 4.02 | 3.89 | 0.97 |
| MSVC 2026 | 6.67 | 6.45 | 0.97 |

T = `MyFatInt<4>`, shape = `1+2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.53 | 1.58 | 1.03 |
| Clang 22 | 1.56 | 1.62 | 1.04 |
| MSVC 2026 | 2.59 | 2.06 | 0.79 |

T = `MyFatInt<8>`, shape = `1+2S`:

| Compiler | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| GCC 16 | 1.09 | 1.09 | 1.00 |
| Clang 22 | 1.16 | 1.16 | 1.00 |
| MSVC 2026 | 1.81 | 1.28 | 0.71 |

## Benchmark 2 (RA=1): per-algorithm by shape, cross-compiler geomean

T = `MyInt`, shape = `1S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 4.89 | 4.81 | 0.98 |
| `copy_if(hit)` | 2.08 | 2.08 | 1.00 |
| `copy_if(miss)` | 2.61 | 2.14 | 0.82 |
| `copy_n` | 5.25 | 5.10 | 0.97 |
| `remove_copy(hit)` | 2.48 | 2.55 | 1.02 |
| `remove_copy(miss)` | 2.45 | 2.65 | 1.08 |
| `remove_copy_if(hit)` | 1.90 | 1.89 | 1.00 |
| `remove_copy_if(miss)` | 2.19 | 2.30 | 1.05 |
| `swap_ranges` | 4.68 | 4.67 | 1.00 |
| `transform` | 5.37 | 5.04 | 0.94 |
| **geomean** | **3.13** | **3.07** | **0.98** |

T = `MyFatInt<4>`, shape = `1S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.49 | 1.44 | 0.97 |
| `copy_if(hit)` | 1.55 | 1.53 | 0.99 |
| `copy_if(miss)` | 2.46 | 1.95 | 0.79 |
| `copy_n` | 1.44 | 1.65 | 1.14 |
| `remove_copy(hit)` | 1.42 | 1.38 | 0.97 |
| `remove_copy(miss)` | 1.46 | 1.41 | 0.97 |
| `remove_copy_if(hit)` | 1.57 | 1.52 | 0.97 |
| `remove_copy_if(miss)` | 1.49 | 1.30 | 0.87 |
| `swap_ranges` | 1.31 | 1.24 | 0.95 |
| `transform` | 1.49 | 1.50 | 1.01 |
| **geomean** | **1.54** | **1.48** | **0.96** |

T = `MyFatInt<8>`, shape = `1S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.17 | 1.17 | 1.00 |
| `copy_if(hit)` | 1.43 | 1.39 | 0.97 |
| `copy_if(miss)` | 1.76 | 1.51 | 0.86 |
| `copy_n` | 1.17 | 1.20 | 1.02 |
| `remove_copy(hit)` | 1.18 | 1.19 | 1.01 |
| `remove_copy(miss)` | 1.17 | 1.18 | 1.01 |
| `remove_copy_if(hit)` | 1.35 | 1.35 | 1.00 |
| `remove_copy_if(miss)` | 1.19 | 1.18 | 0.99 |
| `swap_ranges` | 1.12 | 1.00 | 0.89 |
| `transform` | 1.15 | 1.15 | 1.00 |
| **geomean** | **1.26** | **1.22** | **0.97** |

T = `MyInt`, shape = `2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 4.99 | 4.99 | 1.00 |
| `copy_if(hit)` | 1.64 | 1.65 | 1.01 |
| `copy_if(miss)` | 1.44 | 1.54 | 1.08 |
| `copy_n` | 5.10 | 5.76 | 1.13 |
| `remove_copy(hit)` | 1.78 | 1.72 | 0.97 |
| `remove_copy(miss)` | 1.62 | 1.60 | 0.99 |
| `remove_copy_if(hit)` | 1.86 | 1.90 | 1.02 |
| `remove_copy_if(miss)` | 1.87 | 1.68 | 0.90 |
| `swap_ranges` | 4.67 | 5.02 | 1.08 |
| `transform` | 5.88 | 5.66 | 0.96 |
| **geomean** | **2.64** | **2.67** | **1.01** |

T = `MyFatInt<4>`, shape = `2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.41 | 1.41 | 1.00 |
| `copy_if(hit)` | 1.31 | 1.40 | 1.06 |
| `copy_if(miss)` | 1.76 | 1.54 | 0.87 |
| `copy_n` | 1.46 | 1.60 | 1.10 |
| `remove_copy(hit)` | 1.26 | 1.34 | 1.06 |
| `remove_copy(miss)` | 1.28 | 1.34 | 1.05 |
| `remove_copy_if(hit)` | 1.27 | 1.36 | 1.07 |
| `remove_copy_if(miss)` | 1.24 | 1.23 | 0.99 |
| `swap_ranges` | 1.29 | 1.10 | 0.86 |
| `transform` | 1.39 | 1.39 | 1.00 |
| **geomean** | **1.36** | **1.36** | **1.00** |

T = `MyFatInt<8>`, shape = `2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.17 | 1.18 | 1.01 |
| `copy_if(hit)` | 1.07 | 1.06 | 1.00 |
| `copy_if(miss)` | 0.89 | 0.86 | 0.96 |
| `copy_n` | 1.18 | 1.18 | 1.00 |
| `remove_copy(hit)` | 1.07 | 1.07 | 1.00 |
| `remove_copy(miss)` | 1.07 | 1.07 | 1.00 |
| `remove_copy_if(hit)` | 1.10 | 1.09 | 1.00 |
| `remove_copy_if(miss)` | 1.07 | 1.07 | 1.00 |
| `swap_ranges` | 1.09 | 0.99 | 0.90 |
| `transform` | 1.13 | 1.13 | 1.00 |
| **geomean** | **1.08** | **1.07** | **0.99** |

T = `MyInt`, shape = `1+2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 6.44 | 6.15 | 0.96 |
| `copy_if(hit)` | 4.34 | 4.35 | 1.00 |
| `copy_if(miss)` | 4.50 | 4.36 | 0.97 |
| `copy_n` | 6.69 | 7.06 | 1.06 |
| `remove_copy(hit)` | 3.04 | 3.15 | 1.04 |
| `remove_copy(miss)` | 3.02 | 3.00 | 1.00 |
| `remove_copy_if(hit)` | 3.38 | 3.16 | 0.93 |
| `remove_copy_if(miss)` | 3.03 | 2.99 | 0.99 |
| `swap_ranges` | 5.48 | 4.82 | 0.88 |
| `transform` | 6.72 | 6.54 | 0.97 |
| **geomean** | **4.43** | **4.33** | **0.98** |

T = `MyFatInt<4>`, shape = `1+2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.58 | 1.64 | 1.03 |
| `copy_if(hit)` | 2.39 | 2.16 | 0.90 |
| `copy_if(miss)` | 3.51 | 2.66 | 0.76 |
| `copy_n` | 1.60 | 1.93 | 1.20 |
| `remove_copy(hit)` | 1.72 | 1.66 | 0.96 |
| `remove_copy(miss)` | 1.70 | 1.62 | 0.95 |
| `remove_copy_if(hit)` | 1.74 | 1.74 | 1.00 |
| `remove_copy_if(miss)` | 1.61 | 1.42 | 0.87 |
| `swap_ranges` | 1.36 | 1.29 | 0.95 |
| `transform` | 1.81 | 1.63 | 0.90 |
| **geomean** | **1.83** | **1.74** | **0.95** |

T = `MyFatInt<8>`, shape = `1+2S` (geomean of the three compilers):

| Algorithm | `nsg/seg` | `std/seg` | `std/nsg` |
| --- | --- | --- | --- |
| `copy` | 1.20 | 1.09 | 0.91 |
| `copy_if(hit)` | 1.50 | 1.46 | 0.97 |
| `copy_if(miss)` | 2.00 | 1.35 | 0.67 |
| `copy_n` | 1.22 | 1.06 | 0.87 |
| `remove_copy(hit)` | 1.21 | 1.09 | 0.90 |
| `remove_copy(miss)` | 1.19 | 1.08 | 0.91 |
| `remove_copy_if(hit)` | 1.40 | 1.37 | 0.98 |
| `remove_copy_if(miss)` | 1.24 | 1.11 | 0.90 |
| `swap_ranges` | 1.15 | 1.03 | 0.89 |
| `transform` | 1.23 | 1.17 | 0.95 |
| **geomean** | **1.32** | **1.17** | **0.89** |

## Annex tables (B1 = first benchmark, B2 = second benchmark)

### GCC 16

T = `MyInt`:

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 2.58 | 2.78 | 1.08 | 3.32 | 3.17 | 0.95 |
| `copy(2S)` | 1.11 | 1.11 | 1.00 | 4.33 | 4.34 | 1.00 |
| `copy(1+2S)` | 1.11 | 1.26 | 1.14 | 5.59 | 5.32 | 0.95 |
| `copy_if(1S hit)` | 1.00 | 1.01 | 1.01 | 1.15 | 1.08 | 0.94 |
| `copy_if(2S hit)` | 1.20 | 1.16 | 0.97 | 2.60 | 2.74 | 1.06 |
| `copy_if(1+2S hit)` | 2.02 | 1.75 | 0.87 | 3.01 | 3.06 | 1.02 |
| `copy_if(1S miss)` | 1.12 | 1.10 | 0.99 | 0.89 | 0.92 | 1.03 |
| `copy_if(2S miss)` | 0.78 | 0.68 | 0.87 | 1.68 | 1.81 | 1.08 |
| `copy_if(1+2S miss)` | 1.55 | 1.39 | 0.89 | 2.20 | 2.05 | 0.93 |
| `copy_n(1S)` | 0.92 | 0.88 | 0.95 | 4.51 | 4.32 | 0.96 |
| `copy_n(2S)` | 0.94 | 0.85 | 0.91 | 4.46 | 4.14 | 0.93 |
| `copy_n(1+2S)` | 1.38 | 1.67 | 1.21 | 6.30 | 7.06 | 1.12 |
| `remove_copy(1S hit)` | 1.22 | 1.51 | 1.24 | 1.72 | 1.31 | 0.76 |
| `remove_copy(2S hit)` | 1.40 | 1.36 | 0.97 | 2.24 | 2.18 | 0.97 |
| `remove_copy(1+2S hit)` | 1.04 | 1.06 | 1.02 | 2.03 | 2.24 | 1.10 |
| `remove_copy(1S miss)` | 1.19 | 1.53 | 1.29 | 1.47 | 1.24 | 0.84 |
| `remove_copy(2S miss)` | 1.19 | 1.17 | 0.98 | 1.81 | 1.85 | 1.02 |
| `remove_copy(1+2S miss)` | 1.52 | 1.49 | 0.98 | 1.98 | 1.94 | 0.98 |
| `remove_copy_if(1S hit)` | 1.01 | 0.98 | 0.96 | 0.97 | 1.14 | 1.18 |
| `remove_copy_if(2S hit)` | 1.11 | 1.13 | 1.01 | 1.93 | 1.99 | 1.03 |
| `remove_copy_if(1+2S hit)` | 1.33 | 1.26 | 0.95 | 2.15 | 2.04 | 0.95 |
| `remove_copy_if(1S miss)` | 1.53 | 1.53 | 1.00 | 1.28 | 1.52 | 1.19 |
| `remove_copy_if(2S miss)` | 1.21 | 1.35 | 1.12 | 2.07 | 2.01 | 0.97 |
| `remove_copy_if(1+2S miss)` | 1.32 | 1.34 | 1.02 | 2.17 | 2.10 | 0.97 |
| `swap_ranges(1S)` | 3.47 | 3.49 | 1.01 | 3.46 | 3.97 | 1.15 |
| `swap_ranges(2S)` | 0.75 | 0.75 | 1.00 | 3.96 | 3.99 | 1.01 |
| `swap_ranges(1+2S)` | 1.16 | 1.07 | 0.92 | 4.28 | 4.25 | 0.99 |
| `transform(1S)` | 3.98 | 3.55 | 0.89 | 4.04 | 3.40 | 0.84 |
| `transform(2S)` | 0.95 | 0.92 | 0.97 | 5.17 | 5.28 | 1.02 |
| `transform(1+2S)` | 1.36 | 1.50 | 1.10 | 6.84 | 6.82 | 1.00 |
| **geomean** | **1.30** | **1.31** | **1.01** | **2.59** | **2.57** | **0.99** |

T = `MyFatInt<4>`:

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 1.02 | 1.01 | 0.99 | 1.00 | 1.01 | 1.01 |
| `copy(2S)` | 0.94 | 0.95 | 1.01 | 0.90 | 0.90 | 1.00 |
| `copy(1+2S)` | 1.28 | 1.52 | 1.19 | 1.21 | 1.35 | 1.11 |
| `copy_if(1S hit)` | 0.97 | 0.99 | 1.02 | 0.91 | 1.07 | 1.17 |
| `copy_if(2S hit)` | 1.10 | 1.04 | 0.95 | 1.44 | 1.47 | 1.02 |
| `copy_if(1+2S hit)` | 1.43 | 1.39 | 0.97 | 1.70 | 1.82 | 1.07 |
| `copy_if(1S miss)` | 1.04 | 1.13 | 1.09 | 1.04 | 1.03 | 0.99 |
| `copy_if(2S miss)` | 1.06 | 1.01 | 0.95 | 1.90 | 1.90 | 1.00 |
| `copy_if(1+2S miss)` | 1.61 | 1.53 | 0.95 | 2.73 | 2.61 | 0.96 |
| `copy_n(1S)` | 0.88 | 0.87 | 1.00 | 1.02 | 1.51 | 1.49 |
| `copy_n(2S)` | 0.67 | 0.69 | 1.03 | 1.02 | 1.03 | 1.01 |
| `copy_n(1+2S)` | 1.21 | 1.55 | 1.29 | 1.25 | 1.66 | 1.33 |
| `remove_copy(1S hit)` | 0.79 | 0.86 | 1.09 | 0.77 | 0.86 | 1.12 |
| `remove_copy(2S hit)` | 1.30 | 1.42 | 1.09 | 1.39 | 1.52 | 1.09 |
| `remove_copy(1+2S hit)` | 1.41 | 1.37 | 0.97 | 1.38 | 1.33 | 0.96 |
| `remove_copy(1S miss)` | 0.83 | 0.92 | 1.11 | 0.86 | 0.92 | 1.07 |
| `remove_copy(2S miss)` | 0.99 | 1.08 | 1.09 | 1.40 | 1.51 | 1.08 |
| `remove_copy(1+2S miss)` | 1.50 | 1.40 | 0.93 | 1.45 | 1.33 | 0.92 |
| `remove_copy_if(1S hit)` | 0.96 | 0.82 | 0.86 | 1.14 | 1.01 | 0.89 |
| `remove_copy_if(2S hit)` | 1.03 | 1.12 | 1.08 | 1.39 | 1.51 | 1.09 |
| `remove_copy_if(1+2S hit)` | 1.07 | 1.14 | 1.07 | 1.47 | 1.53 | 1.04 |
| `remove_copy_if(1S miss)` | 0.90 | 0.83 | 0.93 | 0.90 | 0.85 | 0.94 |
| `remove_copy_if(2S miss)` | 1.68 | 1.47 | 0.88 | 1.65 | 1.48 | 0.90 |
| `remove_copy_if(1+2S miss)` | 1.11 | 1.15 | 1.04 | 1.77 | 1.74 | 0.98 |
| `swap_ranges(1S)` | 1.17 | 1.19 | 1.02 | 1.15 | 1.16 | 1.01 |
| `swap_ranges(2S)` | 1.05 | 1.04 | 0.99 | 1.13 | 1.12 | 0.99 |
| `swap_ranges(1+2S)` | 1.14 | 1.11 | 0.98 | 1.31 | 1.28 | 0.98 |
| `transform(1S)` | 1.15 | 1.19 | 1.04 | 1.17 | 1.20 | 1.02 |
| `transform(2S)` | 0.88 | 0.85 | 0.97 | 1.12 | 1.12 | 1.00 |
| `transform(1+2S)` | 1.40 | 1.50 | 1.07 | 1.46 | 1.47 | 1.01 |
| **geomean** | **1.09** | **1.11** | **1.02** | **1.25** | **1.30** | **1.04** |

T = `MyFatInt<8>`:

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 1.01 | 1.00 | 0.99 | 1.01 | 1.01 | 1.00 |
| `copy(2S)` | 0.98 | 0.99 | 1.01 | 0.99 | 1.01 | 1.02 |
| `copy(1+2S)` | 1.04 | 1.08 | 1.04 | 1.05 | 1.09 | 1.03 |
| `copy_if(1S hit)` | 0.94 | 0.95 | 1.01 | 0.93 | 0.93 | 1.00 |
| `copy_if(2S hit)` | 1.07 | 1.06 | 0.99 | 1.06 | 1.05 | 0.99 |
| `copy_if(1+2S hit)` | 1.08 | 1.10 | 1.02 | 1.16 | 1.13 | 0.97 |
| `copy_if(1S miss)` | 1.25 | 1.32 | 1.05 | 1.15 | 1.16 | 1.01 |
| `copy_if(2S miss)` | 1.00 | 1.01 | 1.01 | 1.01 | 1.01 | 0.99 |
| `copy_if(1+2S miss)` | 1.31 | 1.29 | 0.99 | 1.25 | 1.31 | 1.05 |
| `copy_n(1S)` | 1.00 | 0.99 | 0.99 | 1.02 | 1.02 | 1.00 |
| `copy_n(2S)` | 0.98 | 0.99 | 1.01 | 1.03 | 1.02 | 0.99 |
| `copy_n(1+2S)` | 1.02 | 1.02 | 1.00 | 1.07 | 1.01 | 0.95 |
| `remove_copy(1S hit)` | 1.02 | 1.01 | 0.99 | 0.97 | 1.00 | 1.03 |
| `remove_copy(2S hit)` | 0.99 | 1.00 | 1.01 | 1.03 | 1.03 | 1.00 |
| `remove_copy(1+2S hit)` | 1.07 | 1.08 | 1.01 | 1.03 | 1.02 | 1.00 |
| `remove_copy(1S miss)` | 1.02 | 1.02 | 1.00 | 0.96 | 0.99 | 1.02 |
| `remove_copy(2S miss)` | 1.01 | 1.02 | 1.01 | 1.02 | 1.03 | 1.01 |
| `remove_copy(1+2S miss)` | 1.12 | 1.14 | 1.02 | 1.01 | 1.01 | 1.00 |
| `remove_copy_if(1S hit)` | 1.06 | 1.08 | 1.02 | 1.07 | 1.10 | 1.03 |
| `remove_copy_if(2S hit)` | 1.12 | 1.08 | 0.97 | 1.16 | 1.13 | 0.98 |
| `remove_copy_if(1+2S hit)` | 1.01 | 1.01 | 1.00 | 1.11 | 1.10 | 1.00 |
| `remove_copy_if(1S miss)` | 0.99 | 0.99 | 1.00 | 1.00 | 1.00 | 0.99 |
| `remove_copy_if(2S miss)` | 1.08 | 1.09 | 1.01 | 1.08 | 1.09 | 1.01 |
| `remove_copy_if(1+2S miss)` | 1.07 | 1.07 | 1.00 | 1.12 | 1.10 | 0.98 |
| `swap_ranges(1S)` | 0.99 | 0.99 | 1.00 | 0.98 | 0.98 | 0.99 |
| `swap_ranges(2S)` | 0.96 | 0.97 | 1.01 | 0.93 | 0.94 | 1.01 |
| `swap_ranges(1+2S)` | 1.04 | 1.07 | 1.03 | 1.00 | 1.02 | 1.02 |
| `transform(1S)` | 1.04 | 1.03 | 0.99 | 1.03 | 1.03 | 1.00 |
| `transform(2S)` | 1.00 | 1.00 | 1.01 | 1.02 | 1.02 | 1.00 |
| `transform(1+2S)` | 1.05 | 1.08 | 1.03 | 1.09 | 1.10 | 1.00 |
| **geomean** | **1.04** | **1.05** | **1.01** | **1.04** | **1.05** | **1.00** |

### Clang 22

T = `MyInt`:

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 5.77 | 5.85 | 1.01 | 5.94 | 5.92 | 1.00 |
| `copy(2S)` | 0.67 | 0.67 | 1.00 | 4.42 | 4.42 | 1.00 |
| `copy(1+2S)` | 6.85 | 6.58 | 0.96 | 6.99 | 6.99 | 1.00 |
| `copy_if(1S hit)` | 1.67 | 1.59 | 0.96 | 1.65 | 1.74 | 1.05 |
| `copy_if(2S hit)` | 0.80 | 0.80 | 0.99 | 1.28 | 1.23 | 0.96 |
| `copy_if(1+2S hit)` | 1.49 | 1.49 | 1.00 | 2.77 | 2.74 | 0.99 |
| `copy_if(1S miss)` | 5.29 | 2.38 | 0.45 | 4.58 | 2.45 | 0.53 |
| `copy_if(2S miss)` | 0.87 | 0.69 | 0.79 | 1.88 | 2.01 | 1.07 |
| `copy_if(1+2S miss)` | 2.00 | 2.05 | 1.03 | 5.82 | 5.70 | 0.98 |
| `copy_n(1S)` | 5.36 | 5.16 | 0.96 | 5.45 | 5.21 | 0.96 |
| `copy_n(2S)` | 0.59 | 0.91 | 1.55 | 4.99 | 7.79 | 1.56 |
| `copy_n(1+2S)` | 6.26 | 6.52 | 1.04 | 7.78 | 8.15 | 1.05 |
| `remove_copy(1S hit)` | 1.73 | 2.31 | 1.34 | 1.64 | 2.32 | 1.41 |
| `remove_copy(2S hit)` | 0.83 | 0.79 | 0.95 | 1.46 | 1.40 | 0.96 |
| `remove_copy(1+2S hit)` | 1.04 | 1.01 | 0.97 | 2.12 | 2.14 | 1.01 |
| `remove_copy(1S miss)` | 1.53 | 2.32 | 1.52 | 1.53 | 2.30 | 1.51 |
| `remove_copy(2S miss)` | 0.70 | 0.69 | 0.98 | 1.56 | 1.53 | 0.98 |
| `remove_copy(1+2S miss)` | 0.99 | 0.95 | 0.97 | 2.06 | 2.07 | 1.01 |
| `remove_copy_if(1S hit)` | 1.83 | 1.82 | 0.99 | 1.78 | 1.56 | 0.88 |
| `remove_copy_if(2S hit)` | 0.78 | 0.81 | 1.04 | 1.44 | 1.45 | 1.01 |
| `remove_copy_if(1+2S hit)` | 1.05 | 1.23 | 1.17 | 2.66 | 2.47 | 0.93 |
| `remove_copy_if(1S miss)` | 1.59 | 1.54 | 0.97 | 1.59 | 1.55 | 0.98 |
| `remove_copy_if(2S miss)` | 0.79 | 1.33 | 1.68 | 2.39 | 1.82 | 0.76 |
| `remove_copy_if(1+2S miss)` | 1.18 | 1.13 | 0.96 | 2.19 | 2.17 | 0.99 |
| `swap_ranges(1S)` | 4.93 | 4.32 | 0.88 | 5.01 | 4.33 | 0.87 |
| `swap_ranges(2S)` | 0.90 | 1.11 | 1.22 | 4.37 | 5.38 | 1.23 |
| `swap_ranges(1+2S)` | 6.22 | 7.99 | 1.28 | 6.72 | 5.00 | 0.74 |
| `transform(1S)` | 7.07 | 7.17 | 1.01 | 7.01 | 6.89 | 0.98 |
| `transform(2S)` | 0.80 | 0.70 | 0.87 | 6.61 | 5.76 | 0.87 |
| `transform(1+2S)` | 0.88 | 0.88 | 1.00 | 7.43 | 7.44 | 1.00 |
| **geomean** | **1.67** | **1.71** | **1.03** | **3.14** | **3.11** | **0.99** |

T = `MyFatInt<4>`:

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 1.06 | 1.17 | 1.10 | 1.15 | 1.04 | 0.90 |
| `copy(2S)` | 0.95 | 0.96 | 1.02 | 0.99 | 0.99 | 1.00 |
| `copy(1+2S)` | 0.90 | 1.34 | 1.49 | 1.13 | 1.20 | 1.06 |
| `copy_if(1S hit)` | 1.63 | 1.45 | 0.89 | 1.68 | 1.37 | 0.82 |
| `copy_if(2S hit)` | 1.03 | 1.04 | 1.01 | 1.29 | 1.61 | 1.24 |
| `copy_if(1+2S hit)` | 1.25 | 1.26 | 1.01 | 2.35 | 1.75 | 0.74 |
| `copy_if(1S miss)` | 3.17 | 1.87 | 0.59 | 3.57 | 1.79 | 0.50 |
| `copy_if(2S miss)` | 1.00 | 1.00 | 1.00 | 2.71 | 1.80 | 0.66 |
| `copy_if(1+2S miss)` | 2.36 | 1.86 | 0.79 | 3.38 | 4.18 | 1.24 |
| `copy_n(1S)` | 0.98 | 0.98 | 0.99 | 0.99 | 1.00 | 1.00 |
| `copy_n(2S)` | 0.98 | 1.28 | 1.31 | 1.00 | 1.32 | 1.32 |
| `copy_n(1+2S)` | 0.88 | 1.10 | 1.25 | 1.16 | 1.53 | 1.31 |
| `remove_copy(1S hit)` | 1.33 | 1.10 | 0.83 | 1.35 | 1.10 | 0.82 |
| `remove_copy(2S hit)` | 1.12 | 1.11 | 0.99 | 1.17 | 1.29 | 1.10 |
| `remove_copy(1+2S hit)` | 1.43 | 1.42 | 0.99 | 1.44 | 1.43 | 0.99 |
| `remove_copy(1S miss)` | 1.28 | 1.08 | 0.85 | 1.30 | 1.10 | 0.85 |
| `remove_copy(2S miss)` | 1.12 | 1.10 | 0.99 | 1.23 | 1.31 | 1.06 |
| `remove_copy(1+2S miss)` | 1.35 | 1.33 | 0.99 | 1.37 | 1.36 | 0.99 |
| `remove_copy_if(1S hit)` | 1.23 | 1.26 | 1.03 | 1.28 | 1.33 | 1.04 |
| `remove_copy_if(2S hit)` | 1.23 | 1.04 | 0.85 | 1.23 | 1.45 | 1.18 |
| `remove_copy_if(1+2S hit)` | 1.25 | 1.31 | 1.04 | 1.57 | 1.62 | 1.04 |
| `remove_copy_if(1S miss)` | 1.34 | 1.19 | 0.89 | 1.52 | 1.07 | 0.71 |
| `remove_copy_if(2S miss)` | 1.12 | 1.12 | 1.00 | 1.22 | 1.34 | 1.09 |
| `remove_copy_if(1+2S miss)` | 1.71 | 1.25 | 0.73 | 1.32 | 1.83 | 1.38 |
| `swap_ranges(1S)` | 1.18 | 1.04 | 0.88 | 1.19 | 1.04 | 0.88 |
| `swap_ranges(2S)` | 1.08 | 1.10 | 1.01 | 1.11 | 1.09 | 0.99 |
| `swap_ranges(1+2S)` | 0.99 | 0.99 | 1.00 | 1.15 | 1.23 | 1.07 |
| `transform(1S)` | 1.40 | 1.42 | 1.02 | 1.10 | 1.10 | 1.00 |
| `transform(2S)` | 0.73 | 0.74 | 1.01 | 1.03 | 1.02 | 0.99 |
| `transform(1+2S)` | 1.11 | 0.87 | 0.78 | 1.69 | 1.30 | 0.77 |
| **geomean** | **1.21** | **1.17** | **0.96** | **1.39** | **1.35** | **0.97** |

T = `MyFatInt<8>`:

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 0.98 | 1.00 | 1.02 | 0.98 | 0.99 | 1.00 |
| `copy(2S)` | 0.99 | 0.98 | 1.00 | 0.99 | 1.00 | 1.01 |
| `copy(1+2S)` | 1.03 | 1.03 | 1.00 | 1.04 | 1.05 | 1.01 |
| `copy_if(1S hit)` | 1.17 | 1.18 | 1.01 | 1.25 | 1.15 | 0.92 |
| `copy_if(2S hit)` | 1.03 | 1.05 | 1.02 | 1.07 | 1.06 | 1.00 |
| `copy_if(1+2S hit)` | 1.31 | 1.20 | 0.91 | 1.25 | 1.25 | 1.00 |
| `copy_if(1S miss)` | 1.16 | 0.80 | 0.69 | 1.53 | 0.96 | 0.63 |
| `copy_if(2S miss)` | 1.00 | 1.00 | 1.00 | 1.07 | 1.02 | 0.95 |
| `copy_if(1+2S miss)` | 1.65 | 1.67 | 1.01 | 1.65 | 1.65 | 1.01 |
| `copy_n(1S)` | 1.00 | 1.06 | 1.06 | 0.98 | 1.05 | 1.07 |
| `copy_n(2S)` | 1.00 | 1.03 | 1.04 | 0.98 | 0.99 | 1.01 |
| `copy_n(1+2S)` | 1.05 | 1.04 | 0.99 | 1.05 | 1.04 | 0.99 |
| `remove_copy(1S hit)` | 1.06 | 1.05 | 1.00 | 1.07 | 1.07 | 0.99 |
| `remove_copy(2S hit)` | 1.05 | 1.06 | 1.01 | 1.08 | 1.08 | 1.01 |
| `remove_copy(1+2S hit)` | 1.12 | 1.12 | 1.00 | 1.13 | 1.12 | 0.99 |
| `remove_copy(1S miss)` | 1.05 | 1.05 | 1.00 | 1.07 | 1.07 | 1.00 |
| `remove_copy(2S miss)` | 1.06 | 1.06 | 1.00 | 1.08 | 1.08 | 1.00 |
| `remove_copy(1+2S miss)` | 1.11 | 1.11 | 1.00 | 1.11 | 1.11 | 0.99 |
| `remove_copy_if(1S hit)` | 1.17 | 1.11 | 0.95 | 1.10 | 1.07 | 0.98 |
| `remove_copy_if(2S hit)` | 1.07 | 1.06 | 0.99 | 1.06 | 1.06 | 1.00 |
| `remove_copy_if(1+2S hit)` | 1.18 | 1.18 | 1.00 | 1.20 | 1.20 | 1.00 |
| `remove_copy_if(1S miss)` | 1.04 | 1.01 | 0.97 | 1.03 | 1.00 | 0.97 |
| `remove_copy_if(2S miss)` | 1.05 | 1.05 | 1.00 | 1.05 | 1.04 | 0.99 |
| `remove_copy_if(1+2S miss)` | 1.11 | 1.11 | 1.00 | 1.11 | 1.11 | 1.01 |
| `swap_ranges(1S)` | 0.99 | 0.99 | 1.00 | 0.99 | 0.99 | 1.00 |
| `swap_ranges(2S)` | 0.98 | 0.97 | 1.00 | 0.98 | 0.99 | 1.01 |
| `swap_ranges(1+2S)` | 1.02 | 1.04 | 1.01 | 1.03 | 1.03 | 1.00 |
| `transform(1S)` | 1.05 | 1.05 | 1.00 | 1.02 | 1.02 | 1.00 |
| `transform(2S)` | 0.91 | 0.91 | 0.99 | 0.99 | 1.00 | 1.01 |
| `transform(1+2S)` | 1.13 | 1.13 | 1.00 | 1.15 | 1.14 | 1.00 |
| **geomean** | **1.08** | **1.06** | **0.99** | **1.09** | **1.07** | **0.98** |

### MSVC 2026

T = `MyInt`:

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 6.37 | 6.38 | 1.00 | 5.94 | 5.94 | 1.00 |
| `copy(2S)` | 5.87 | 5.88 | 1.00 | 6.49 | 6.49 | 1.00 |
| `copy(1+2S)` | 5.83 | 5.37 | 0.92 | 6.82 | 6.25 | 0.92 |
| `copy_if(1S hit)` | 4.11 | 4.13 | 1.00 | 4.76 | 4.77 | 1.00 |
| `copy_if(2S hit)` | 0.84 | 0.84 | 1.01 | 1.33 | 1.33 | 1.00 |
| `copy_if(1+2S hit)` | 3.02 | 3.02 | 1.00 | 9.83 | 9.84 | 1.00 |
| `copy_if(1S miss)` | 4.30 | 4.31 | 1.00 | 4.34 | 4.35 | 1.00 |
| `copy_if(2S miss)` | 0.68 | 0.66 | 0.97 | 0.94 | 1.01 | 1.08 |
| `copy_if(1+2S miss)` | 2.87 | 2.87 | 1.00 | 7.10 | 7.10 | 1.00 |
| `copy_n(1S)` | 5.04 | 5.04 | 1.00 | 5.89 | 5.89 | 1.00 |
| `copy_n(2S)` | 5.22 | 5.19 | 0.99 | 5.96 | 5.94 | 1.00 |
| `copy_n(1+2S)` | 4.63 | 4.63 | 1.00 | 6.10 | 6.11 | 1.00 |
| `remove_copy(1S hit)` | 5.92 | 5.92 | 1.00 | 5.44 | 5.44 | 1.00 |
| `remove_copy(2S hit)` | 0.76 | 0.81 | 1.07 | 1.72 | 1.68 | 0.98 |
| `remove_copy(1+2S hit)` | 3.40 | 3.27 | 0.96 | 6.55 | 6.54 | 1.00 |
| `remove_copy(1S miss)` | 6.50 | 6.51 | 1.00 | 6.50 | 6.50 | 1.00 |
| `remove_copy(2S miss)` | 1.27 | 1.29 | 1.01 | 1.50 | 1.46 | 0.97 |
| `remove_copy(1+2S miss)` | 4.77 | 4.77 | 1.00 | 6.75 | 6.75 | 1.00 |
| `remove_copy_if(1S hit)` | 3.57 | 3.57 | 1.00 | 3.99 | 3.82 | 0.96 |
| `remove_copy_if(2S hit)` | 1.05 | 1.10 | 1.05 | 2.31 | 2.36 | 1.02 |
| `remove_copy_if(1+2S hit)` | 2.08 | 1.99 | 0.95 | 6.77 | 6.25 | 0.92 |
| `remove_copy_if(1S miss)` | 5.09 | 5.22 | 1.03 | 5.19 | 5.19 | 1.00 |
| `remove_copy_if(2S miss)` | 1.18 | 1.16 | 0.98 | 1.32 | 1.30 | 0.98 |
| `remove_copy_if(1+2S miss)` | 4.41 | 4.41 | 1.00 | 5.87 | 5.87 | 1.00 |
| `swap_ranges(1S)` | 5.85 | 5.93 | 1.01 | 5.91 | 5.92 | 1.00 |
| `swap_ranges(2S)` | 4.89 | 4.89 | 1.00 | 5.89 | 5.89 | 1.00 |
| `swap_ranges(1+2S)` | 4.56 | 4.19 | 0.92 | 5.73 | 5.26 | 0.92 |
| `transform(1S)` | 5.34 | 5.34 | 1.00 | 5.48 | 5.47 | 1.00 |
| `transform(2S)` | 4.97 | 4.98 | 1.00 | 5.95 | 5.95 | 1.00 |
| `transform(1+2S)` | 3.58 | 3.29 | 0.92 | 5.98 | 5.51 | 0.92 |
| **geomean** | **3.30** | **3.28** | **0.99** | **4.50** | **4.45** | **0.99** |

T = `MyFatInt<4>`:

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 2.91 | 2.91 | 1.00 | 2.86 | 2.86 | 1.00 |
| `copy(2S)` | 3.15 | 3.15 | 1.00 | 3.15 | 3.15 | 1.00 |
| `copy(1+2S)` | 2.73 | 2.54 | 0.93 | 2.91 | 2.71 | 0.93 |
| `copy_if(1S hit)` | 3.98 | 3.98 | 1.00 | 2.45 | 2.45 | 1.00 |
| `copy_if(2S hit)` | 0.94 | 0.88 | 0.94 | 1.22 | 1.15 | 0.94 |
| `copy_if(1+2S hit)` | 2.68 | 2.48 | 0.93 | 3.43 | 3.18 | 0.93 |
| `copy_if(1S miss)` | 4.26 | 4.26 | 1.00 | 4.02 | 4.02 | 1.00 |
| `copy_if(2S miss)` | 1.10 | 1.23 | 1.12 | 1.06 | 1.06 | 1.00 |
| `copy_if(1+2S miss)` | 3.95 | 1.48 | 0.37 | 4.67 | 1.72 | 0.37 |
| `copy_n(1S)` | 2.63 | 2.63 | 1.00 | 2.93 | 2.95 | 1.00 |
| `copy_n(2S)` | 2.47 | 2.47 | 1.00 | 3.04 | 3.04 | 1.00 |
| `copy_n(1+2S)` | 2.40 | 2.40 | 1.00 | 2.84 | 2.85 | 1.00 |
| `remove_copy(1S hit)` | 2.79 | 2.79 | 1.00 | 2.75 | 2.75 | 1.00 |
| `remove_copy(2S hit)` | 1.07 | 1.05 | 0.99 | 1.22 | 1.22 | 0.99 |
| `remove_copy(1+2S hit)` | 2.54 | 2.38 | 0.94 | 2.55 | 2.39 | 0.94 |
| `remove_copy(1S miss)` | 2.78 | 2.78 | 1.00 | 2.78 | 2.78 | 1.00 |
| `remove_copy(2S miss)` | 1.02 | 1.03 | 1.01 | 1.21 | 1.21 | 1.00 |
| `remove_copy(1+2S miss)` | 2.47 | 2.31 | 0.93 | 2.49 | 2.33 | 0.94 |
| `remove_copy_if(1S hit)` | 2.70 | 2.61 | 0.97 | 2.65 | 2.64 | 0.99 |
| `remove_copy_if(2S hit)` | 1.12 | 1.06 | 0.95 | 1.20 | 1.16 | 0.96 |
| `remove_copy_if(1+2S hit)` | 2.87 | 2.67 | 0.93 | 2.28 | 2.12 | 0.93 |
| `remove_copy_if(1S miss)` | 2.82 | 2.82 | 1.00 | 2.43 | 2.43 | 1.00 |
| `remove_copy_if(2S miss)` | 0.96 | 0.95 | 0.99 | 0.94 | 0.93 | 1.00 |
| `remove_copy_if(1+2S miss)` | 1.70 | 0.84 | 0.49 | 1.80 | 0.89 | 0.49 |
| `swap_ranges(1S)` | 1.66 | 1.59 | 0.95 | 1.65 | 1.57 | 0.95 |
| `swap_ranges(2S)` | 1.68 | 1.09 | 0.65 | 1.70 | 1.10 | 0.65 |
| `swap_ranges(1+2S)` | 1.64 | 1.35 | 0.82 | 1.66 | 1.36 | 0.82 |
| `transform(1S)` | 2.54 | 2.55 | 1.00 | 2.55 | 2.55 | 1.00 |
| `transform(2S)` | 2.03 | 2.03 | 1.00 | 2.33 | 2.33 | 1.00 |
| `transform(1+2S)` | 2.19 | 2.07 | 0.95 | 2.40 | 2.27 | 0.95 |
| **geomean** | **2.14** | **1.95** | **0.91** | **2.20** | **2.00** | **0.91** |

T = `MyFatInt<8>`:

| Algorithm | `nsg/seg` B1 | `std/seg` B1 | `std/nsg` B1 | `nsg/seg` B2 | `std/seg` B2 | `std/nsg` B2 |
| --- | --- | --- | --- | --- | --- | --- |
| `copy(1S)` | 1.60 | 1.60 | 1.00 | 1.60 | 1.59 | 1.00 |
| `copy(2S)` | 1.65 | 1.65 | 1.00 | 1.64 | 1.63 | 1.00 |
| `copy(1+2S)` | 1.59 | 1.14 | 0.71 | 1.58 | 1.13 | 0.72 |
| `copy_if(1S hit)` | 2.49 | 2.49 | 1.00 | 2.50 | 2.50 | 1.00 |
| `copy_if(2S hit)` | 1.10 | 1.10 | 1.00 | 1.08 | 1.08 | 1.00 |
| `copy_if(1+2S hit)` | 2.22 | 2.08 | 0.94 | 2.34 | 2.19 | 0.94 |
| `copy_if(1S miss)` | 3.84 | 3.84 | 1.00 | 3.09 | 3.09 | 1.00 |
| `copy_if(2S miss)` | 1.03 | 1.04 | 1.01 | 0.66 | 0.61 | 0.93 |
| `copy_if(1+2S miss)` | 3.48 | 1.04 | 0.30 | 3.87 | 1.13 | 0.29 |
| `copy_n(1S)` | 1.60 | 1.60 | 1.00 | 1.61 | 1.61 | 1.00 |
| `copy_n(2S)` | 1.63 | 1.63 | 1.00 | 1.62 | 1.62 | 1.00 |
| `copy_n(1+2S)` | 1.53 | 1.09 | 0.71 | 1.61 | 1.13 | 0.70 |
| `remove_copy(1S hit)` | 1.54 | 1.54 | 1.00 | 1.57 | 1.57 | 1.00 |
| `remove_copy(2S hit)` | 1.11 | 1.10 | 1.00 | 1.10 | 1.10 | 1.00 |
| `remove_copy(1+2S hit)` | 1.53 | 1.14 | 0.74 | 1.53 | 1.12 | 0.73 |
| `remove_copy(1S miss)` | 1.54 | 1.54 | 1.00 | 1.57 | 1.57 | 1.00 |
| `remove_copy(2S miss)` | 1.10 | 1.10 | 1.00 | 1.10 | 1.10 | 1.00 |
| `remove_copy(1+2S miss)` | 1.52 | 1.15 | 0.75 | 1.50 | 1.12 | 0.75 |
| `remove_copy_if(1S hit)` | 2.06 | 2.06 | 1.00 | 2.07 | 2.08 | 1.00 |
| `remove_copy_if(2S hit)` | 1.05 | 1.06 | 1.01 | 1.08 | 1.09 | 1.01 |
| `remove_copy_if(1+2S hit)` | 2.11 | 1.94 | 0.92 | 2.07 | 1.93 | 0.93 |
| `remove_copy_if(1S miss)` | 1.58 | 1.58 | 1.00 | 1.63 | 1.63 | 1.00 |
| `remove_copy_if(2S miss)` | 1.09 | 1.09 | 1.00 | 1.09 | 1.09 | 1.00 |
| `remove_copy_if(1+2S miss)` | 1.55 | 1.14 | 0.73 | 1.55 | 1.12 | 0.73 |
| `swap_ranges(1S)` | 1.39 | 1.04 | 0.74 | 1.44 | 1.04 | 0.72 |
| `swap_ranges(2S)` | 1.45 | 1.04 | 0.72 | 1.43 | 1.03 | 0.72 |
| `swap_ranges(1+2S)` | 1.49 | 1.06 | 0.71 | 1.49 | 1.04 | 0.70 |
| `transform(1S)` | 1.41 | 1.41 | 1.00 | 1.44 | 1.45 | 1.00 |
| `transform(2S)` | 1.28 | 1.27 | 1.00 | 1.43 | 1.42 | 1.00 |
| `transform(1+2S)` | 1.29 | 1.12 | 0.87 | 1.47 | 1.28 | 0.87 |
| **geomean** | **1.57** | **1.38** | **0.87** | **1.57** | **1.36** | **0.87** |
