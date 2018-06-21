all: proxy ips

flags =-DDATA_SIZE=1500 -O2 -g
#flags +=-I/home/kinow/git/hiredis

proxy: proxy.c bm_timediff.c
	gcc proxy.c -o proxy bm_timediff.c -lpthread -lhiredis -I. $(flags)

ips: ips.c
	gcc ips.c -o ips -lpthread -lhiredis -I. $(flags)

clean:
	rm -fr proxy ips
