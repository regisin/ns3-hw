#!/bin/bash

cd /home/pregis/workspace/cpp-netbeans/ns-3.26_mobile


time=1030

timeout=300

for speed in 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
do

  # SPLIT

  routing="split"

  for size in 10 100 1000 10000 100000 1000000
  do
    echo "SPLIT$size"
    for appnodes in 1 2 3 4 5 6 7 8 9 10
    do
      fname="${routing}${size}-500x500-25-1-$appnodes-1-$time-$speed.csv"
      echo "run_id,total_bytes_received,throughput_(bps),no_of_received_packets,packet_delivery_ratio,mean_hop_count,mean_delay_(s),l3_drop_packets,total_initial_energy,total_remaining_energy,average_lifetime" > $fname

      for id in 1 2 3 4 5 6 7 10
      do
        timeout $(($appnodes*$timeout)) ./waf --run="split-sim --app=$appnodes --time=$time --routing=$routing --speed=$speed --hist=$size --id=$id" >> $fname 2>&1 &
      done
      wait
    done
  done





  # Others

  routing="olsr"
  echo "OLSR"
  for appnodes in 1 2 3 4 5 6 7 8 9 10
  do
    fname="$routing-500x500-25-1-$appnodes-1-$time-$speed.csv"
    echo "run_id,total_bytes_received,throughput_(bps),no_of_received_packets,packet_delivery_ratio,mean_hop_count,mean_delay_(s),l3_drop_packets,total_initial_energy,total_remaining_energy,average_lifetime" > $fname

    for id in 1 2 3 4 5 6 7 10
    do
      timeout $(($appnodes*$timeout)) ./waf --run="split-sim --app=$appnodes --time=$time --routing=$routing --speed=$speed --id=$id" >> $fname 2>&1 &
    done
    wait
  done


  routing="aodv"
  echo "AODV"
  for appnodes in 1 2 3 4 5 6 7 8 9 10
  do
    fname="$routing-500x500-25-1-$appnodes-1-$time-$speed.csv"
    echo "run_id,total_bytes_received,throughput_(bps),no_of_received_packets,packet_delivery_ratio,mean_hop_count,mean_delay_(s),l3_drop_packets,total_initial_energy,total_remaining_energy,average_lifetime" > $fname

    for id in 1 2 3 4 5 6 7 10
    do
      timeout $(($appnodes*$timeout)) ./waf --run="split-sim --app=$appnodes --time=$time --routing=$routing --speed=$speed --id=$id" >> $fname 2>&1 &
    done
    wait
  done

  routing="dsdv"
  echo "DSDV"
  for appnodes in 1 2 3 4 5 6 7 8 9 10
  do
    fname="$routing-500x500-25-1-$appnodes-1-$time-$speed.csv"
    echo "run_id,total_bytes_received,throughput_(bps),no_of_received_packets,packet_delivery_ratio,mean_hop_count,mean_delay_(s),l3_drop_packets,total_initial_energy,total_remaining_energy,average_lifetime" > $fname

    for id in 1 2 3 4 5 6 7 10
    do
      timeout $(($appnodes*$timeout)) ./waf --run="split-sim --app=$appnodes --time=$time --routing=$routing --speed=$speed --id=$id" >> $fname 2>&1 &
    done
    wait
  done



  routing="etx"
  echo "ETX"
  for appnodes in 1 2 3 4 5 6 7 8 9 10
  do
    fname="$routing-500x500-25-1-$appnodes-1-$time-$speed.csv"
    echo "run_id,total_bytes_received,throughput_(bps),no_of_received_packets,packet_delivery_ratio,mean_hop_count,mean_delay_(s),l3_drop_packets,total_initial_energy,total_remaining_energy,average_lifetime" > $fname

    for id in 1 2 3 4 5 6 7 10
    do
      timeout $(($appnodes*$timeout)) ./waf --run="split-sim --app=$appnodes --time=$time --routing=$routing --speed=$speed --id=$id" >> $fname 2>&1 &
    done
    wait
  done

done