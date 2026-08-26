// Inner-loop comparison for the group-20 mixed rows where std/nsg < 1.
// equal(1S): range1 = bc::deque<MyInt>, range2 = bc::vector<MyInt>.

#include <boost/container/deque.hpp>
#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_equal.hpp>
#include <boost/container/experimental/segmented_mismatch.hpp>
#include <boost/container/experimental/wrapped_iterator.hpp>
#include <bench_utils.hpp>
#include <algorithm>

namespace bc = boost::container;
typedef bc::deque<MyInt>            dq_t;
typedef bc::vector<MyInt>           vec_t;
typedef dq_t::const_iterator        dit_t;
typedef vec_t::const_iterator       vit_t;
typedef bc::wrapped_iterator<dit_t> wdit_t;
typedef bc::wrapped_iterator<vit_t> wvit_t;

//--- equal(1S): deque source, vector range2 ---------------------------------
bool eq1s_std(dit_t f, dit_t l, vit_t f2)
{  return std::equal(f, l, f2); }

bool eq1s_nsg(wdit_t f, wdit_t l, wvit_t f2)
{  return bc::segmented_equal(f, l, f2); }

//--- equal(2S): vector source, deque range2 ---------------------------------
bool eq2s_std(vit_t f, vit_t l, dit_t f2)
{  return std::equal(f, l, f2); }

bool eq2s_nsg(wvit_t f, wvit_t l, wdit_t f2)
{  return bc::segmented_equal(f, l, f2); }

//--- mismatch(1S): deque source, vector range2, 3-arg ------------------------
std::pair<dit_t, vit_t> mm1s_std(dit_t f, dit_t l, vit_t f2)
{  return std::mismatch(f, l, f2); }

std::pair<wdit_t, wvit_t> mm1s_nsg(wdit_t f, wdit_t l, wvit_t f2)
{  return bc::segmented_mismatch(f, l, f2); }
