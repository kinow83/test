CC = gcc
SENDER_OBJS = sender.o queue.o sock.o

.SUFFIXES: .c .o

all: sender

sender: $(SENDER_OBJS)
	$(CC) -o $@ $(SENDER_OBJS) -lpthread
