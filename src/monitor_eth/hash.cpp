#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

hash_table_t *hash_table_create(
	int (*compare)(const void*, const void*),
	uint16_t (*hash)(const void*), 
	void (*free_bucket)(void*), 
	bool replace,
	uint32_t bucket_size)
{
    hash_table_t *table = NULL;

    table = (hash_table_t*)malloc(sizeof(hash_table_t));

    table->T = (hash_bucket_t**)malloc(sizeof(hash_bucket_t*) * bucket_size);
    memset(table->T, 0, sizeof(hash_bucket_t*) * bucket_size);

    table->num_element = 0;
	table->bucket_size = bucket_size;
    table->compare = compare;
    table->hash = hash;
    table->replace = replace;
    table->free_bucket = free_bucket;
    return table;
}
void hash_table_free(hash_table_t *table)
{
    hash_bucket_t *cur = NULL;
    hash_bucket_t *tmp = NULL;
    int i;

    if (!table) return;

    for(i = 0; i < table->bucket_size; i++) {
        cur = table->T[i];
        while(cur) {
            if(cur->data) {
                if(table->free_bucket)
                    table->free_bucket(cur->data);
            }
            tmp = cur;
            cur = tmp->next;
            free(tmp);
        }
    }
    free(table->T);
    free(table);
    return;
}

bool hash_table_insert(hash_table_t *table, void *data)
{
    hash_bucket_t *cur = NULL;
    hash_bucket_t *newbee = NULL;
    uint16_t index;
    int result;

    index = table->hash(data) % table->bucket_size;

    cur = table->T[index];

    while(cur) {
        if(cur->data) {
            result = table->compare(data, cur->data);
            if(result == 0) {
                if(table->replace == false) {
                    return false;
                }

                if(table->free_bucket) table->free_bucket(cur->data);
                cur->data = data;
                return true;
            }
        }
        cur = cur->next;
    }

    newbee = (hash_bucket_t*)malloc(sizeof(hash_bucket_t));
    memset(newbee, 0, sizeof(hash_bucket_t));

    newbee->data = data;

    newbee->next = table->T[index];
    table->T[index] = newbee;
    if(newbee->next)
        newbee->next->prev = newbee;

    table->num_element++;
    return true;
}

hash_bucket_t *hash_table_find(hash_table_t *table, const void *data)
{
    hash_bucket_t *cur = NULL;
    uint16_t index;
    int result;

    if (data == NULL) return NULL;

    index = table->hash(data) % table->bucket_size;

    cur = table->T[index];

    while(cur) {
        result = table->compare(data, cur->data);
        if(!result) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

void *hash_table_finddata(hash_table_t *table, const void *data)
{
    hash_bucket_t *bucket = NULL;

    bucket = hash_table_find(table, data);
    if (!bucket) return NULL;
    return bucket->data;
}

void hash_table_delete(hash_table_t *table, hash_bucket_t *bucket)
{
    hash_bucket_t *cur = NULL;
    uint16_t index;
    int result;

    if (bucket == NULL) return;

    index = table->hash(bucket->data) % table->bucket_size;

    cur = table->T[index];

    if(!cur) return;

    while(cur) {
        result = table->compare(cur->data, bucket->data);
        if(!result) {
            if(table->free_bucket)
                table->free_bucket(cur->data);

            if(cur == table->T[index])
                table->T[index] = cur->next;
            else
                cur->prev->next = cur->next;

            if(cur->next)
                cur->next->prev = cur->prev;

            free(cur);
            table->num_element--;
            return;
        }
        cur = cur->next;
    }
    return;
}

void *hash_table_bucket2data(hash_table_t *table, hash_bucket_t *bucket)
{
    if(!table) return NULL;
    if(!bucket) return NULL;
    return bucket->data;
}

