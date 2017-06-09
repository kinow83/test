KDIR := /lib/modules/$(shell uname -r)/build

obj-m += nat_table.o

all:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	rm -fr *.symvers  *.order *.ko  *.mod.c  *.o 
