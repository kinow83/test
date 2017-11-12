CC = gcc
CFLAGS = -g -Wall -lrt
SEM_OBJS = ipcs_sem.o bm_timediff.o
SOCK_OBJS = ipcs_sock.o bm_timediff.o

all: producer_sem comsumer_sem producer_sock comsumer_sock

producer_sem: producer.o $(SEM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

comsumer_sem: comsumer.o $(SEM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

producer_sock: producer.o $(SOCK_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

comsumer_sock: comsumer.o $(SOCK_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -fr comsumer_sem producer_sem comsumer_sock producer_sock *.o
