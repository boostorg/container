// Controlled replay of the two copy_if inner-loop shapes observed in clang_off:
//   shape A = library flat leaf  (store inline, branch OVER the store)
//   shape B = std::copy_if       (store sunk to a cold out-of-line block)
// Both walk a deque-like node array of 128-int (512-byte) blocks.
// Hypothesis: throughput is set by taken branches per element.
//   A: 1 (back-edge) + P(pred false)      -> miss 2, 50% 1.5, hit 1
//   B: 1 (back-edge) + 2*P(pred true)     -> miss 1, 50% 2,   hit 3
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <ctime>

static const int  BLOCK_INTS  = 128;
static const long BLOCK_BYTES = 512;

static double now_s()
{
   timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return double(ts.tv_sec) + 1e-9 * double(ts.tv_nsec);
}

template<int PAD>
static long loopA(int** node, int* cur, int* last, int* dst)
{
   int* dst0 = dst;
   asm volatile(
      ".p2align 6\n\t"
      ".skip %c[pad], 0x90\n\t"
      "1:\n\t"
      "movl (%[cur]), %%r10d\n\t"
      "testl %%r10d, %%r10d\n\t"
      "jns 2f\n\t"
      "movl %%r10d, (%[dst])\n\t"
      "addq $4, %[dst]\n\t"
      "2:\n\t"
      "addq $4, %[cur]\n\t"
      "movq (%[node]), %%r10\n\t"
      "addq %[bb], %%r10\n\t"
      "cmpq %%r10, %[cur]\n\t"
      "je 3f\n\t"
      "cmpq %[last], %[cur]\n\t"
      "jne 1b\n\t"
      "jmp 4f\n\t"
      "3:\n\t"
      "movq 8(%[node]), %[cur]\n\t"
      "addq $8, %[node]\n\t"
      "cmpq %[last], %[cur]\n\t"
      "jne 1b\n\t"
      "4:\n\t"
      : [cur] "+r"(cur), [dst] "+r"(dst), [node] "+r"(node)
      : [last] "r"(last), [bb] "r"(BLOCK_BYTES), [pad] "i"(PAD)
      : "r10", "cc", "memory");
   return long(dst - dst0);
}

template<int PAD>
static long loopB(int** node, int* base, int* cur, int* last, int* dst)
{
   int* dst0 = dst;
   asm volatile(
      ".p2align 6\n\t"
      ".skip %c[pad], 0x90\n\t"
      "1:\n\t"
      "movl (%[cur]), %%r10d\n\t"
      "testl %%r10d, %%r10d\n\t"
      "js 5f\n\t"
      "addq $4, %[cur]\n\t"
      "leaq 512(%[base]), %%r10\n\t"
      "cmpq %%r10, %[cur]\n\t"
      "je 3f\n\t"
      "2:\n\t"
      "cmpq %[last], %[cur]\n\t"
      "jne 1b\n\t"
      "jmp 4f\n\t"
      ".p2align 6\n\t"
      "5:\n\t"
      "movl %%r10d, (%[dst])\n\t"
      "addq $4, %[dst]\n\t"
      "addq $4, %[cur]\n\t"
      "leaq 512(%[base]), %%r10\n\t"
      "cmpq %%r10, %[cur]\n\t"
      "jne 2b\n\t"
      "3:\n\t"
      "movq 8(%[node]), %[base]\n\t"
      "addq $8, %[node]\n\t"
      "movq %[base], %[cur]\n\t"
      "cmpq %[last], %[cur]\n\t"
      "jne 1b\n\t"
      "4:\n\t"
      : [cur] "+r"(cur), [dst] "+r"(dst), [node] "+r"(node), [base] "+r"(base)
      : [last] "r"(last), [pad] "i"(PAD)
      : "r10", "cc", "memory");
   return long(dst - dst0);
}

static const long N = 100000;

struct Data {
   std::vector<int>   buf;
   std::vector<int>   dst;
   std::vector<int*>  node;
   Data() : buf(size_t(N)), dst(size_t(N))
   {
      long nb = (N + BLOCK_INTS - 1) / BLOCK_INTS;
      for (long i = 0; i <= nb; ++i)
         node.push_back(&buf[0] + i * BLOCK_INTS);
   }
   void fill(int pct_neg)
   {
      for (long i = 0; i < N; ++i) {
         bool neg = (pct_neg == 100) || (pct_neg == 50 && (i & 1)) ;
         buf[size_t(i)] = neg ? -(int(i) + 1) : (int(i) + 1);
      }
   }
};

int main(int argc, char** argv)
{
   int reps = (argc > 1) ? atoi(argv[1]) : 2000;
   Data d;

   printf("shape  pct_neg   ns/elem   taken-branch model (per elem)\n");
   for (int p = 0; p <= 100; p += 50) {
      d.fill(p);
      // A
      double best = 1e30; long chk = 0;
      for (int r = 0; r < 3; ++r) {
         double t0 = now_s();
         for (int i = 0; i < reps; ++i)
            chk += loopA<0>(&d.node[0], &d.buf[0], &d.buf[0] + N, &d.dst[0]);
         double el = now_s() - t0;
         double ns = el * 1e9 / (double(reps) * double(N));
         if (ns < best) best = ns;
      }
      printf("A      %3d%%     %7.3f   %.1f   (chk=%ld)\n", p, best,
             1.0 + (100 - p) / 100.0, chk);
      // B
      best = 1e30; chk = 0;
      for (int r = 0; r < 3; ++r) {
         double t0 = now_s();
         for (int i = 0; i < reps; ++i)
            chk += loopB<0>(&d.node[0], &d.buf[0], &d.buf[0], &d.buf[0] + N, &d.dst[0]);
         double el = now_s() - t0;
         double ns = el * 1e9 / (double(reps) * double(N));
         if (ns < best) best = ns;
      }
      printf("B      %3d%%     %7.3f   %.1f   (chk=%ld)\n", p, best,
             1.0 + 2.0 * p / 100.0, chk);
   }

   // Alignment sweep on shape A at 0% (the anomalous case): exclude g30 placement.
   printf("\nalignment sweep, shape A, pct_neg=0 (loop head offset within 64B line)\n");
   d.fill(0);
   long chk = 0;
#define SWEEP(PAD) { double best=1e30; \
   for (int r=0;r<3;++r){ double t0=now_s(); \
     for(int i=0;i<reps;++i) chk += loopA<PAD>(&d.node[0], &d.buf[0], &d.buf[0]+N, &d.dst[0]); \
     double ns=(now_s()-t0)*1e9/(double(reps)*double(N)); if(ns<best)best=ns; } \
   printf("  pad %2d : %7.3f ns/elem\n", PAD, best); }
   SWEEP(0) SWEEP(4) SWEEP(8) SWEEP(12) SWEEP(16) SWEEP(20) SWEEP(24) SWEEP(28)
   SWEEP(32) SWEEP(36) SWEEP(40) SWEEP(44) SWEEP(48) SWEEP(52) SWEEP(56) SWEEP(60)
   printf("(chk=%ld)\n", chk);
   return 0;
}
