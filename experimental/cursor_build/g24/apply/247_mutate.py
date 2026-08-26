# Negative controls for the depth-3 destination coverage.
#
#   247_mutate.py <header-stem> <abort|nolast|nomid> <outfile>
#
# Every mutation is applied to a SHADOW copy and is guarded on
# dst_is_local_seg_t::value, the compile-time answer to "is the destination's
# local iterator itself segmented".  That is exactly the condition under which
# the segmented_iterator_tag overload of *_dst_bounded recurses into ITSELF,
# so a guarded mutation changes only the self-recursive instantiation and
# leaves the depth-1 and depth-2 instantiations byte-for-byte alone.
#
# Hence:
#   the pre-existing tests (destination depth <= 2) must still pass, and
#   the new depth-3 tests must fail,
# which is what distinguishes real coverage from a test that merely compiles a
# deeper type without ever running the new path.
#
#   abort  : die on entry to any invocation that is about to recurse into
#            itself.  Proves the self-recursive invocation is reached at all.
#   nolast : drop the restart at the beginning of the last destination segment.
#   nomid  : drop the restart at the beginning of each intermediate segment.
import os, sys

STEM, MODE, DST = sys.argv[1], sys.argv[2], sys.argv[3]
SRC = "/mnt/d/Data/LocalGit/boost/boost/container/experimental/%s.hpp" % STEM

text = open(SRC, newline='').read().replace("\r\n", "\n")

# Unique to the segmented-destination *_dst_bounded overload: until_exhausts
# and the dispatchers name their local iterator differently.
ENTRY = "   dst_local_iterator db = dst_traits::local(dst_first);"
LAST  = "      db = dst_traits::begin(slast);"
MID   = "dst_traits::begin(sfirst)"


def once(old, new):
   global text
   if text.count(old) != 1:
      sys.exit("%s/%s: anchor %r matched %d times"
               % (STEM, MODE, old, text.count(old)))
   text = text.replace(old, new)


if MODE == "abort":
   once(ENTRY, ENTRY + "\n   if(dst_is_local_seg_t::value) std::abort();")
   once("#include <cstddef>", "#include <cstddef>\n#include <cstdlib>")
elif MODE == "nolast":
   once(LAST, "      if(!dst_is_local_seg_t::value) db = dst_traits::begin(slast);")
elif MODE == "nomid":
   # Only the occurrence inside the intermediate-segment loop of the
   # segmented-destination overload; that loop is the only place the walker
   # restarts at a segment it has not written to yet.
   if text.count(MID) != 1:
      sys.exit("%s/%s: %r matched %d times" % (STEM, MODE, MID, text.count(MID)))
   text = text.replace(MID, "(dst_is_local_seg_t::value ? db : %s)" % MID)
else:
   sys.exit("unknown mode " + MODE)

os.makedirs(os.path.dirname(DST), exist_ok=True)
open(DST, "w", newline="\n").write(text)
print("   %-40s %-7s ok" % (STEM, MODE))
