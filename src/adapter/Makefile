all: echo mecho adapter client mclient

FLAGS = -O2 -g #-DDEBUG

mecho: mecho.c sock.c sock.h
	gcc mecho.c sock.c -o mecho -lpthread $(FLAGS)

echo: echo.c sock.c sock.h
	gcc echo.c sock.c -o echo -lpthread $(FLAGS)

client: client.c sock.c sock.h bm_timediff.c bm_timediff.h
	gcc client.c sock.c -o client bm_timediff.c -lpthread $(FLAGS)

mclient: mclient.c sock.c sock.h bm_timediff.c bm_timediff.h
	gcc mclient.c sock.c -o mclient bm_timediff.c -lpthread $(FLAGS)

adapter: sock.c sock.h adapter.c
	gcc adapter.c sock.c -o adapter -lpthread $(FLAGS)


clean:
	rm -fr echo adapter client mecho mclient
