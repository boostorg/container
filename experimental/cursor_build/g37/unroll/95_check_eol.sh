#!/bin/bash
L=/mnt/d/Data/LocalGit/boost/libs/container/include/boost/container/experimental
for f in segmented_copy_if.hpp segmented_remove_copy_if.hpp segmented_remove_copy.hpp segmented_partition_copy.hpp; do
   tot=$(wc -l < $L/$f)
   crlf=$(grep -c $'\r$' $L/$f)
   printf '%-34s lines=%-5s crlf=%-5s lf_only=%s\n' "$f" "$tot" "$crlf" "$((tot - crlf))"
done
echo "--- segmented_iterator_traits.hpp line 396 (must be the user's pragma switch) ---"
sed -n '394,398p' $L/segmented_iterator_traits.hpp | cat -A | sed 's/\$$//'
echo "--- git diff of segmented_iterator_traits.hpp (must be only line 396) ---"
cd /mnt/d/Data/LocalGit/boost/libs/container && git diff -- include/boost/container/experimental/segmented_iterator_traits.hpp
