all: proxy ips

flags =-DDATA_SIZE=1500 -O2 -g
flags +=-I/home/kinow/git/hiredis

proxy: proxy.c bm_timediff.c proxy2.c
	gcc proxy.c  -o proxy  bm_timediff.c -lpthread -lhiredis -I. $(flags)
	gcc proxy2.c -o proxy2 bm_timediff.c -lpthread -lhiredis -I. $(flags)

ips: ips.c ips2.c
	gcc ips.c  -o ips  -lpthread -lhiredis -I. $(flags)
	gcc ips2.c -o ips2 -lpthread -lhiredis -I. $(flags)

clean:
	rm -fr proxy ips
	rm -fr proxy2 ips2
