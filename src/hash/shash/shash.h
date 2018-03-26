#ifndef _HASHED_EAP_SESSION
#define	_HASHED_EAP_SESSION

#include <stdint.h>

#define	NUMBER_OF_BUCKETS	65535

typedef struct hash_bucket_t {
	void *Data;
	struct hash_bucket_t *prev;
	struct hash_bucket_t *next;
} hash_bucket_t;

typedef struct hash_table_t {
#ifndef	NDEBUG
	uint32_t	magic;
#endif
	hash_bucket_t	**T;
	int num_element;
	int (*Compare)(const void*, const void*);
	uint16_t (*Hash)(const void*);
	int replace_flag;
	void (*freeBucket)(void*);
} hash_table_t;

hash_table_t *hash_table_create(int (*Compare)(const void*, const void*),
		uint16_t (*Hash)(const void*),
		void (*freebucket)(void*), int replace_flag);
void hash_table_free(hash_table_t *table);
int hash_table_insert(hash_table_t *table, void *Data);
hash_bucket_t *hash_table_find(hash_table_t *table, const void *Data);
void hash_table_delete(hash_table_t *table, hash_bucket_t *bucket);
void *hash_table_bucket2data(hash_table_t *table, hash_bucket_t *bucket);
void *hash_table_finddata(hash_table_t *table, const void *Data);

#endif	/* _HASHED_EAP_SESSION */
