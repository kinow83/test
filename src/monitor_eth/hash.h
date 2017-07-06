#include <stdint.h>

typedef struct hash_bucket_t {
    void *data;
    struct hash_bucket_t *prev;
    struct hash_bucket_t *next;
} hash_bucket_t;

typedef struct hash_table_t {
    hash_bucket_t   **T;
    uint32_t num_element;
    bool replace;
	uint32_t bucket_size;
    int (*compare)(const void*, const void*);
    uint16_t (*hash)(const void*);
    void (*free_bucket)(void*);
} hash_table_t;

hash_table_t *hash_table_create(int (*compare)(const void*, const void*),
	uint16_t (*hash)(const void*), void (*free_bucket)(void*), bool replace, uint32_t bucket_size);
void hash_table_free(hash_table_t *table);
bool hash_table_insert(hash_table_t *table, void *data);
hash_bucket_t *hash_table_find(hash_table_t *table, const void *data);
void *hash_table_finddata(hash_table_t *table, const void *data);
void hash_table_delete(hash_table_t *table, hash_bucket_t *bucket);
void *hash_table_bucket2data(hash_table_t *table, hash_bucket_t *bucket);

