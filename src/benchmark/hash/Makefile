SUBDIRS=hsearch myhash unordered_map unordered_map2

LIB_NAME = libsample.a
LIB_SRCS = libsample.c
LIB_OBJS = $(LIB_SRCS:%.c=%.o)

all: $(LIB_NAME) subdirs

$(LIB_NAME): $(LIB_OBJS)
	ar rcv $@ $(LIB_OBJS)

subdirs:
	@for dir in $(SUBDIRS); do \
		make -C $$dir all; \
		if [ $$? != 0 ]; then exit 1; fi; \
	done

clean:
	rm -fr *.a *.o
	@for dir in $(SUBDIRS); do \
		make -C $$dir clean; \
		if [ $$? != 0 ]; then exit 1; fi; \
	done


test:
	@base64 /dev/urandom | head -c 4048576 > 1MB.txt
	@for dir in $(SUBDIRS); do \
		./$$dir/test_$$dir 1MB.txt; \
	done
	./google/sparsehash-master/test_google 1MB.txt
	@rm -fr 1MB.txt

taskset:
	@base64 /dev/urandom | head -c 4048576 > 1MB.txt
	@for dir in $(SUBDIRS); do \
		taskset 0x1 ./$$dir/test_$$dir 1MB.txt; \
	done
	taskset 0x1 ./google/sparsehash-master/test_google 1MB.txt
	@rm -fr 1MB.txt
