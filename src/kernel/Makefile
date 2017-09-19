obj-m = hello2.o
obj-m = hello_param.o
obj-m = chardev.o
obj-m = syscall.o
obj-m = procfs1.o
obj-m = procfs2.o
obj-m = interupt.o
obj-m = tasklet_test.o
obj-m = work_queue.o
obj-m = netfilter_test.o
obj-m = protocol_hook.o
obj-m = kernel_ioctl.o


KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
