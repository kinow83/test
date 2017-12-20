#ifndef _IO_ADAPTER_
#define _IO_ADAPTER_

struct io_link {
	int sock;
};

struct io_linker
{
	int (*init) (io_link *);
	int (*write)(io_link *);
	int (*read) (io_link *);
	void (*destory)(io_link *);
};



#endif
