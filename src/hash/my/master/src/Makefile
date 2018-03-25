master: master.c
	cc master.c  -L/usr/lib/mysql -I/usr/include/mysql -lmysqlclient  -o ../master  -lpthread -D_REENTRANT


clean:
	rm -fr *.o
	rm -fr ../master
	
