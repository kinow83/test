
all: test_myhash

test_myhash: test_myhash.c shash.c
	gcc test_myhash.c -o test_myhash -I.. ../libsample.a shash.c

clean:
	rm -fr test_myhash
