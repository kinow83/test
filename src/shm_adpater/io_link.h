#ifndef _IO_ADAPTER_
#define _IO_ADAPTER_


typedef struct io_link {
	int sock;
} io_link;

typedef struct io_data {
	uint8_t unused;
	uint8_t data[1500];
	size_t datalen;
} io_data;

typedef struct io_index {
	uint16_t index;
//	struct sockaddr_in from;
	uint32_t xid;
} __attribute__((packed)) io_index;


typedef struct io_linker
{
	int (*init)        (io_link *);
	int (*write_data)  (io_link *, uint8_t *, size_t);
	int (*read_data  ) (io_link *, uint8_t *);
	int (*write_index) (io_link *, io_index *);
	int (*read_index)  (io_link *, io_index *);
	void (*destroy)    (io_link *);
} io_linker;



#endif
