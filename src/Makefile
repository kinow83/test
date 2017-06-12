KDIR := /lib/modules/$(shell uname -r)/build

obj-m += kthread_test.o

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -fr *.symvers  *.order *.ko  *.mod.c  *.o 
