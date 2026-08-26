#!/bin/bash
H=/mnt/d/Data/LocalGit/boost/boost/container/experimental
O=/tmp/g26
mkdir -p $O
for a in set_difference set_union set_intersection set_symmetric_difference; do
   sed -e "s/${a}/set_X/g" -e "s/SET_${a^^}/SET_X/g" $H/segmented_${a}.hpp > $O/$a.norm
done
echo "########## set_difference (user-edited) vs set_intersection ##########"
diff -u $O/set_difference.norm $O/set_intersection.norm | head -80
echo
echo "########## set_difference vs set_union ##########"
diff -u $O/set_difference.norm $O/set_union.norm | head -60
