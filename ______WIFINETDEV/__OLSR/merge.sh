#!/bin/bash

#for appnodes in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
#do
#  cd "/home/pregis/workspace/cpp-netbeans/ns-3.26/_individual/power_split_500x500_25_1_${appnodes}_1_400.000000"
#  python merge.py
#  cd ..
#done

for appnodes in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
do
  cd "/home/pregis/workspace/cpp-netbeans/ns-3.26/__OLSR/_overall/olsr_500x500_25_1_${appnodes}_1_400.000000"
  python merge.py
  cd ..
done

#cd "/home/pregis/workspace/cpp-netbeans/ns-3.26/_individual"
#python merge_merge_p.py

cd "/home/pregis/workspace/cpp-netbeans/ns-3.26/__OLSR/_overall"
python merge_merge_o.py
