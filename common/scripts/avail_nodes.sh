#!/bin/bash

IFS=' '
avail_comp=( $(sinfo | grep idle | awk '{ print $6 }' | grep comp | cut -d "[" -f2 | cut -d "]" -f1 | tr ',' ' ' | tr '-' ' ' | sed 's/"//g') )
avail_stor=( $(sinfo | grep idle | awk '{ print $6 }' | grep stor | cut -d "[" -f2 | cut -d "]" -f1 | tr ',' ' ' | tr '-' ' ' | sed 's/"//g') )

iter=$((${#avail_comp[@]} / 2))
count_comp=0
for i in $(seq 0 $(($iter - 1))); do
  count_comp=$((${count_comp} + avail_comp[$((2 * $i + 1))] - avail_comp[$((2 * $i))]))
done

iter=$((${#avail_stor[@]} / 2))
count_stor=0
for i in $(seq 0 $(($iter - 1))); do
  count_stor=$((${count_stor} + avail_comp[$((2 * $i + 1))] - avail_comp[$((2 * $i))]))
done

echo "There are ${count_comp} compute nodes available"
echo "There are ${count_stor} storage nodes available"
