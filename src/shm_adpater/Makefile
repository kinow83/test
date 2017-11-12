CC = gcc
SENDER_OBJS   = sender.o queue.o sock.o io_link.o
RECEIVER_OBJS = receiver.o queue.o sock.o io_link.o

.SUFFIXES: .c .o

all: sender receiver

sender: $(SENDER_OBJS)
	$(CC) -o $@ $(SENDER_OBJS) -lpthread

receiver: $(RECEIVER_OBJS)
	$(CC) -o $@ $(RECEIVER_OBJS) -lpthread
