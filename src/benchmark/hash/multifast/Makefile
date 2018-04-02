all: subdir test_multifast

INCLUDE_DIRECTORY := ahocorasick
LINK_DIRECTORY := ahocorasick/build
LINK_LIBRARY := ahocorasick
LINK_TARGET := $(LINK_DIRECTORY)/lib$(LINK_LIBRARY).a

subdir:
	make -C ahocorasick
	make -C multifast

test_multifast: test_multifast.o $(LINK_TARGET)
	cc -o test_multifast test_multifast.o -L$(LINK_DIRECTORY) -l$(LINK_LIBRARY) -I../ ../libsample.a 

test_multifast.o: test_multifast.c
	cc -o test_multifast.o -c test_multifast.c -I$(INCLUDE_DIRECTORY) -Wall  -I../

clean:
	rm -f test_multifast test_multifast.o
