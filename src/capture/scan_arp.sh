#!/bin/bash

i=0

while [ $i -le 255 ]; do
	./capture -i enp3s0 -m s -d "0.0.0.0, 00:00:00:00:00:00, 10.10.200.$i, 00:00:00:00:00:00"
	i=`expr $i + 1`
done
