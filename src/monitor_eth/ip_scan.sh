#!/bin/bash
network=192.168.10
int1=101
int2=199
for ip in $(seq $int1 $int2); do
    ping -c 1 $network.$ip -I eth3 -i 1 #| grep "bytes from" &
done
