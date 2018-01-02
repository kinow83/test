CC = gcc
SENDER_OBJS   = sender.o queue.o sock.o xid.o pbio_data.o
HTTP_OBJS =     http.o   queue.o sock.o xid.o pbio_data.o
CFLAGS = -I userspace-rcu/include -g -O2
TCM = -L /usr/local/lib/ -ltcmalloc #_minimal 

.SUFFIXES: .c .o

all: sender http

sender: $(SENDER_OBJS)
	$(CC) -o $@ $(SENDER_OBJS) -lpthread -lcrypto $(TCM)

http: $(HTTP_OBJS)
	$(CC) -o $@ $(HTTP_OBJS) -lpthread -lcrypto $(TCM)

clean:
	rm -fr *.o
	rm -fr sender
	rm -fr http
