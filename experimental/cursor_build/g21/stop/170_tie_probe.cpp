// The RA leaf now derives its stop flag from  src_n < iter2_n.  That predicate
// is right for computing the min but wrong as "the source was exhausted":
// when the two lengths are equal the loop consumes the whole source, yet the
// flag says false, so the walker treats it as "iter2 segment ran out" and
// advances to a segment that may not exist.
//
// The tie is easy to reach: a source whose remaining length is exactly the
// remaining length of the current iter2 segment.

#include <boost/container/deque.hpp>
#include <boost/container/options.hpp>
#include <boost/container/experimental/segmented_mismatch.hpp>
#include <boost/container/experimental/segmented_equal.hpp>

#include <vector>
#include <utility>
#include <algorithm>
#include <cstdio>
#include <cstddef>

namespace bc = boost::container;

static const std::size_t BS = 128;
typedef bc::deque_options< bc::block_size<BS> >::type opt_t;
typedef bc::deque<int, void, opt_t>                   dq_t;

static int failures = 0;

//Reference: lock-step walk over the common prefix.
static std::size_t ref_pos(const std::vector<int> &v, const dq_t &d)
{
   const std::size_t n = v.size() < d.size() ? v.size() : d.size();
   std::size_t k = 0;
   while(k != n && v[k] == d[k])
      ++k;
   return k;
}

static void one(std::size_t vn, std::size_t dn, long mp, const char *what)
{
   std::vector<int> v;
   dq_t             d;
   for(std::size_t i = 0; i != vn; ++i)
      v.push_back(int(i));
   for(std::size_t i = 0; i != dn; ++i)
      d.push_back(int(i));
   if(mp >= 0 && std::size_t(mp) < dn)
      d[std::size_t(mp)] = -1;

   const std::size_t want = ref_pos(v, d);

   //3-argument form: iter2 unbounded, walks iter2 segments (walker B).
   const std::pair<std::vector<int>::const_iterator, dq_t::const_iterator> r3 =
      bc::segmented_mismatch(v.begin(), v.end(), d.begin());
   const std::size_t got1 = std::size_t(r3.first - v.begin());
   const std::size_t got2 = std::size_t(r3.second - d.begin());

   //4-argument form: both ends bounded.
   const std::pair<std::vector<int>::const_iterator, dq_t::const_iterator> r4 =
      bc::segmented_mismatch(v.begin(), v.end(), d.begin(), d.end());
   const std::size_t got1b = std::size_t(r4.first - v.begin());
   const std::size_t got2b = std::size_t(r4.second - d.begin());

   const bool eq_want = (want == vn) && (vn <= dn);
   const bool eq_got  = bc::segmented_equal(v.begin(), v.end(), d.begin());

   const bool ok = got1 == want && got2 == want &&
                   got1b == want && got2b == want &&
                   eq_got == eq_want;
   if(!ok) {
      std::printf("FAIL %-22s vn=%-5zu dn=%-5zu mism=%-5ld want=%zu"
                  "  3arg=(%zu,%zu) 4arg=(%zu,%zu) equal=%d/%d\n",
                  what, vn, dn, mp, want, got1, got2, got1b, got2b,
                  int(eq_got), int(eq_want));
      ++failures;
   }
}

int main()
{
   std::printf("deque block size = %zu\n\n", BS);

   //The source ends exactly where the first iter2 segment ends.
   one(BS,     BS,     -1, "tie, no next block");
   one(BS,     2*BS,   -1, "tie, next block exists");
   one(BS,     BS + 7, -1, "tie, short next block");

   //The source ends exactly where a later segment ends.
   one(2*BS,   2*BS,   -1, "tie at 2nd block end");
   one(3*BS,   3*BS,   -1, "tie at 3rd block end");
   one(2*BS,   5*BS,   -1, "tie mid-deque");

   //The shape the benchmark uses: 100000 elements, block 128.
   one(100000, 100000, -1, "bench shape, all equal");
   one(100000, 100000, 50000, "bench shape, mismatch");

   //Ties combined with a mismatch inside the tying segment.
   one(BS,     BS,     0,      "tie + mism at 0");
   one(BS,     BS,     long(BS/2), "tie + mism mid");
   one(BS,     BS,     long(BS-1), "tie + mism last");
   one(2*BS,   2*BS,   long(BS), "tie + mism at boundary");

   //Non-tie controls.
   one(BS - 1, BS,     -1, "source shorter");
   one(BS + 1, 2*BS,   -1, "source longer than 1 block");
   one(0,      BS,     -1, "empty source");

   std::printf("\n%s (%d failures)\n", failures ? "FAILURES PRESENT" : "all OK", failures);
   return failures != 0;
}
