CC = gcc
CFLAGS = -W -Wall 
TARGET = queue_benchmark
GLIB_CFLAGS =$(shell pkg-config --cflags glib-2.0)
GLIB_LDFLAGS=$(shell pkg-config --libs glib-2.0)
DPDK_CFLAGS =$(shell pkg-config --cflags libdpdk)
DPDK_LDFGAGS=$(shell pkg-config --libs libdpdk)

$(TARGET) : queue.o ring.o main.o rte_ring.o
	$(CC) $(CFLAGS) -o $(TARGET) queue.o ring.o rte_ring.o main.o $(GLIB_LDFLAGS) -lpthread 

ring.o: ring.c rte_ring.c
	$(CC) -c ring.c rte_ring.c $(CFLAGS) -I.
	
queue.o : queue.c	
	$(CC) -c queue.c $(CFLAGS) $(GLIB_CFLAGS)
	
clean:
	rm -fr *.o $(TARGET)