#ifndef _IO_ADAPTER_
#define _IO_ADAPTER_

struct io_link {
	int sock;
};

struct io_linker
{
	int (*init) (io_link *);
	int (*write)(io_link *, void *);
	int (*read) (io_link *, void *);
	void (*destory)(io_link *);
};



#endif
