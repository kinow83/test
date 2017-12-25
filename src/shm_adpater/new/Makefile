CC = gcc
SENDER_OBJS   = sender.o queue.o sock.o xid.o pbio_data.o
HTTP_OBJS =     http.o   queue.o sock.o xid.o pbio_data.o

.SUFFIXES: .c .o

all: sender http

sender: $(SENDER_OBJS)
	$(CC) -o $@ $(SENDER_OBJS) -lpthread -lcrypto

http: $(HTTP_OBJS)
	$(CC) -o $@ $(HTTP_OBJS) -lpthread -lcrypto
