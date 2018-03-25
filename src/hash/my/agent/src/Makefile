agent: agent.c system.o  bistree.o bitree.o table.o obj.o
	cc agent.c system.o bistree.o bitree.o table.o obj.o -o ../agent -D_REENTRANT -L/usr/lib/mysql -I/usr/include/mysql -lmysqlclient -lpthread -lpcap -D_KERNERL_2_6

system.o : system.c
	cc system.c -c

bitree.o : bitree.c
	cc bitree.c -c

bistree.o : bistree.c
	cc bistree.c -c

table.o : table.c
	cc table.c -c

obj.o : obj.c
	cc obj.c -c

clean:
	rm -fr *.o
	rm -fr ../agent
