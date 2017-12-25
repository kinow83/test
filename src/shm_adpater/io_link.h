#ifndef _IO_ADAPTER_
#define _IO_ADAPTER_

typedef struct iohello_t {
	uint16_t port;
	uint32_t xid;
} __attribute__((packed)) iohello_t;

typedef struct ioindex_t {
	uint16_t index;
	uint32_t xid;
} __attribute__((packed)) ioindex_t;


/* [ offerer ] ****************************************************/
typedef struct io_offerer_ctx {
	int listen_sock;
	int accept_sock;
} io_offerer_ctx;

/* [ disposer ] ****************************************************/
typedef struct io_disposer_ctx {
	int connect_sock;
} io_disposer_ctx;

typedef struct iolinker {
	int (*init) (void *);
	int (*put)  (void *, uint8_t *, size_t,   ioindex_t *);
	int (*pop)  (void *, uint8_t *, size_t *, ioindex_t *);
	int (*mark) (void *, ioindex_t *);
	int (*hello)(void *, 
	int (*send) (void *, ioindex_t *);
	int (*recv) (void *, ioindex_t *);
	void (*destroy) (void *);
} iolinker;

#endif
