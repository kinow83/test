#!/bin/bash

echo "" > /tmp/my.result

do_sql() {
	type=$1
	count=$2
	mysql test -e "delete from test"
	
	RES=`{ time ./my_async $type $count; } 2>&1| grep real` >> /tmp/my.result
	echo "$type $count: $RES" >> /tmp/my.result
}

do_sql 2
do_sql 3 2
do_sql 3 3
do_sql 3 4
do_sql 3 5
do_sql 3 6
do_sql 3 7
do_sql 3 8

do_sql 2
do_sql 3 2
do_sql 3 3
do_sql 3 4
do_sql 3 5
do_sql 3 6
do_sql 3 7
do_sql 3 8

do_sql 2
do_sql 3 2
do_sql 3 3
do_sql 3 4
do_sql 3 5
do_sql 3 6
do_sql 3 7
do_sql 3 8

do_sql 2
do_sql 3 2
do_sql 3 3
do_sql 3 4
do_sql 3 5
do_sql 3 6
do_sql 3 7
do_sql 3 8

