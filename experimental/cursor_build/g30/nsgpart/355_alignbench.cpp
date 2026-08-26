// 355: replicate the exact 26-byte backward-scan loop of the partition(miss)
// benchmark (deque iterator operator-- with map-slot memory compare) at
// controlled offsets inside a 64-byte fetch line, on the same fake data
// layout as bc::deque<MyInt, block_size<128>> with 100096 elements.
//
// Loop replicated (from 350_gcc.elf, nsg partition(miss), e.g. 0xea70):
//   1: sub    $0x4,%rcx          ; --cur
//      cmp    %rcx,%rax          ; first == cur ?
//      je     out
//   2: mov    (%rcx),%r8d        ; load *cur
//      test   %r8d,%r8d
//      js     far_out            ; pred hit (rel32 in real code)
//      cmp    %rcx,(%rdi)        ; cur == segment base (map slot deref)
//      jne    1b
//      mov    -0x8(%rdi),%rcx    ; previous segment base
//      sub    $0x8,%rdi
//      add    $0x1fc,%rcx        ; + 127*4 (last element of prev segment)
//      cmp    %rcx,%rax
//      jne    2b
//   out:
#include <cstdio>
#include <cstdlib>
#include <ctime>

enum { SEG_ELEMS = 128, NSEG = 782, TOTAL = SEG_ELEMS * NSEG, REPS = 5000 };

static int*  segs[NSEG];

static double now_ns()
{
   timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return double(ts.tv_sec) * 1e9 + double(ts.tv_nsec);
}

#define DEF_SCAN(PAD)                                                         \
__attribute__((noinline))                                                     \
const int* scan_##PAD(int** map_slot, const int* first, const int* cur)      \
{                                                                             \
   asm volatile(                                                              \
      ".p2align 6\n\t"                                                        \
      ".rept " #PAD "\n\tnop\n\t.endr\n"                                      \
      "1:\n\t"                                                                \
      "sub  $4, %[cur]\n\t"                                                   \
      "cmp  %[cur], %[first]\n\t"                                             \
      "je   3f\n\t"                                                           \
      "2:\n\t"                                                                \
      "mov  (%[cur]), %%r8d\n\t"                                              \
      "test %%r8d, %%r8d\n\t"                                                 \
      "js   4f\n\t"                                                           \
      "cmp  %[cur], (%[map])\n\t"                                             \
      "jne  1b\n\t"                                                           \
      "mov  -8(%[map]), %[cur]\n\t"                                           \
      "sub  $8, %[map]\n\t"                                                   \
      "add  $0x1fc, %[cur]\n\t"                                               \
      "cmp  %[cur], %[first]\n\t"                                             \
      "jne  2b\n"                                                             \
      "3:\n\t"                                                                \
      "jmp  5f\n\t"                                                           \
      ".skip 1024, 0x90\n"                                                    \
      "4:\n\t"                                                                \
      "jmp  3b\n"                                                             \
      "5:\n\t"                                                                \
      : [cur] "+r" (cur), [map] "+r" (map_slot)                               \
      : [first] "r" (first)                                                   \
      : "r8", "memory", "cc");                                                \
   return cur;                                                                \
}

DEF_SCAN(0)  DEF_SCAN(8)  DEF_SCAN(16) DEF_SCAN(24) DEF_SCAN(32)
DEF_SCAN(36) DEF_SCAN(40) DEF_SCAN(44) DEF_SCAN(48) DEF_SCAN(52)
DEF_SCAN(56) DEF_SCAN(60)

typedef const int* (*scan_fn)(int**, const int*, const int*);

int main()
{
   for (int s = 0; s < NSEG; ++s) {
      segs[s] = static_cast<int*>(aligned_alloc(64, SEG_ELEMS * sizeof(int)));
      for (int i = 0; i < SEG_ELEMS; ++i)
         segs[s][i] = s * SEG_ELEMS + i;   // all non-negative: full scan
   }

   struct row { int pad; scan_fn fn; };
   const row rows[] = {
      {0,scan_0},{8,scan_8},{16,scan_16},{24,scan_24},{32,scan_32},
      {36,scan_36},{40,scan_40},{44,scan_44},{48,scan_48},{52,scan_52},
      {56,scan_56},{60,scan_60}
   };

   std::printf("loop bytes = 26; crossing predicted for pad > 38\n");
   for (unsigned r = 0; r < sizeof(rows)/sizeof(rows[0]); ++r) {
      const int* end = 0;
      double best = 1e30;
      for (int trial = 0; trial < 5; ++trial) {
         const double t0 = now_ns();
         for (int rep = 0; rep < REPS; ++rep)
            end = rows[r].fn(&segs[NSEG - 1], segs[0], segs[NSEG - 1] + SEG_ELEMS);
         const double dt = (now_ns() - t0) / (double(REPS) * TOTAL);
         if (dt < best) best = dt;
      }
      std::printf("pad=%2d  %s  ns/elem=%.3f  (scan %s)\n",
                  rows[r].pad, rows[r].pad > 38 ? "CROSS" : "fit  ", best,
                  end == segs[0] ? "ok" : "BAD");
   }
   return 0;
}
