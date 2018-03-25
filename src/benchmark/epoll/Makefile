all: echo_server echo_client

DPDK= /home/kinow/dpdk/dpdk-stable-17.11.1

echo_server: echo_server.cpp echo_server1.cpp echo_server2.cpp Makefile
	gcc echo_server.cpp -o echo_server -lpthread -pg
	gcc echo_server1.cpp -o echo_server1 -lpthread -pg
	gcc echo_server3.cpp pthread_queue.cpp -o echo_server3 -lpthread -pg
	gcc echo_server2.cpp -o echo_server2 \
		$(DPDK)/build/lib/librte_ring.a \
		$(DPDK)/build/lib/librte_eal.a \
		-lpthread -pg \
		-I$(DPDK)/lib \
		-I$(DPDK)/lib/librte_eal/common/include \
		-I$(DPDK)/build/include \
		-lnuma -ldl

echo_client: echo_client.cpp
	gcc echo_client.cpp -o echo_client -lpthread -pg
