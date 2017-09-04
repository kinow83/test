#!/bin/bash

com_shell=`find ./ -name compile.sh`
cur_pwd=`pwd`

for com in $com_shell; do
	dir=`dirname $com`
	cd $dir
	chmod +x compile.sh
	./compile.sh
	cd $cur_pwd
done
