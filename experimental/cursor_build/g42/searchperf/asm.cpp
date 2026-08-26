#include <boost/container/vector.hpp>
#include <boost/container/experimental/segmented_search.hpp>

namespace bc = boost::container;

typedef bc::vector<int>::const_iterator it_t;

// Non-segmented haystack, random-access needle: the shape the refactor changed.
it_t srch(it_t f, it_t l, const int *p, const int *pe)
{  return bc::segmented_search(f, l, p, pe); }
