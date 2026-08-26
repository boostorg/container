#!/bin/bash
# Every entity segmented_test_helper.hpp defines, and how many of the
# experimental tests name it.  Zero means dead code (or an entity reached only
# through another helper, which the report has to explain).
set -u
EX=/mnt/d/Data/LocalGit/boost/libs/container/experimental
H=$EX/segmented_test_helper.hpp

names=$( { grep -oE '^(class|struct) [a-z_0-9]+'                   $H | awk '{print $2}'
           grep -oE '^(inline )?[A-Za-z_:<>, ]+ [a-z_0-9]+\('      $H | grep -oE '[a-z_0-9]+\($' | tr -d '('
           grep -oE '^(void|bool|int) [a-z_0-9]+\('                $H | awk '{print $2}' | tr -d '('
           grep -oE '^[a-z_0-9]+ [a-z_0-9]+\('                     $H | awk '{print $2}' | tr -d '('
           grep -oE '^\s{3}(void|bool|iterator|std::size_t|reference|pointer) [a-z_0-9]+\(' $H \
              | awk '{print $2}' | tr -d '('
         } | sort -u )

printf "%-32s %s\n" ENTITY "test files naming it"
for n in $names; do
   c=$(grep -lw "$n" $EX/*_test.cpp 2>/dev/null | wc -l)
   printf "%-32s %s%s\n" "$n" "$c" "$( [ "$c" = 0 ] && echo '   <-- UNREFERENCED' )"
done
