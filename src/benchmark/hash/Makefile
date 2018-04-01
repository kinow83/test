
SUBDIRS=hsearch myhash unorederd_map trie 

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

