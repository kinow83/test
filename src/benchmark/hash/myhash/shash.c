/**
  @file		eap_table.c
  @brief
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shash.h"

/**
  @brief	creating hash table

  allocating hash table, buckets\n
  seting up Compare function, freeBucket, replace_flag

  @return	hash table pointer
  */
hash_table_t *hash_table_create(int (*Compare)(const void *, const void*),
		uint16_t (*Hash)(const void *), void (*freeBucket)(void *),
		int replace_flag)
{
	hash_table_t *table = NULL;

	table = (hash_table_t*)malloc(sizeof(hash_table_t));

	table->T = (hash_bucket_t**)malloc(sizeof(hash_bucket_t*) * NUMBER_OF_BUCKETS);
	memset(table->T, 0, sizeof(hash_bucket_t*) * NUMBER_OF_BUCKETS);
	
	table->num_element = 0;
	table->Compare = Compare;
	table->Hash = Hash;
	table->replace_flag = replace_flag;
	table->freeBucket = freeBucket;

	return table;
}

/**
  @brief	freeing hash table

  freeing hash buckets, then table
  */
void hash_table_free(hash_table_t *table)
{
	hash_bucket_t	*cur;
	hash_bucket_t	*tmp;
	int i;

	if (!table) return;

	for(i = 0; i < NUMBER_OF_BUCKETS; i++) {
		cur = table->T[i];
		while(cur) {
			if(cur->Data) {
				if(table->freeBucket)
					table->freeBucket(cur->Data);
			}
			tmp = cur;
			cur = tmp->next;
			free(tmp);
		}
	}

#ifndef NDEBUG
	table->magic = 0;
#endif
	free(table->T);
	free(table);

	return;
}

#if 0
/**
  @brief	making hash key

  using the first 2 bytes from the state field
  */
static uint16_t	hashing(const void *Data)
{
	uint16_t ret;

	memcpy(&ret, ((EAP_HANDLER*)Data)->state, 2);

	return ret;
}
#endif

/**
  @brief	inserting eap data into hash table

  It will insert a eap data into hash table if it doesn't exist in the table.
  Then increase num_element.

  @return	if seccess then return 1, else 0
  */
int hash_table_insert(hash_table_t *table, void *Data)
{
	hash_bucket_t	*cur;
	hash_bucket_t	*newbee;
	uint16_t		index;
	int				result;

	index = table->Hash(Data);

	cur = table->T[index];

#if 0
if (cur)
{
FILE *fp = fopen("/root/ppp", "a");
fprintf(fp, "hash insert dup entry %x\n", index);
fclose(fp);
}
#endif

	while(cur) {
		if(cur->Data) {
			result = table->Compare(Data, cur->Data);
			if(result == 0) {
				if(table->replace_flag == 0) {
					return 0;
				}

				if(table->freeBucket) table->freeBucket(cur->Data);
				cur->Data = Data;
				return 1;
			}
		}
		cur = cur->next;
	}

	newbee = (hash_bucket_t*)malloc(sizeof(hash_bucket_t));
	memset(newbee, 0, sizeof(hash_bucket_t));

	newbee->Data = Data;

	newbee->next = table->T[index];
	table->T[index] = newbee;
	if(newbee->next)
		newbee->next->prev = newbee;

	table->num_element++;

	return 1;
}

/**
  @brief	finding eap data from hash table
  @return	bucket pointer including eap data pointer
  */
hash_bucket_t *hash_table_find(hash_table_t *table, const void *Data)
{
	hash_bucket_t	*cur;
	uint16_t		index;
	int				result;

	if (Data == NULL) return NULL;

	index = table->Hash(Data);

	cur = table->T[index];

	while(cur) {
		result = table->Compare(Data, cur->Data);
		if(!result) {
			return cur;
		}
		cur = cur->next;
	}

	return NULL;
}

void *hash_table_finddata(hash_table_t *table, const void *Data)
{
	hash_bucket_t *bucket;

	bucket = hash_table_find(table, Data);
	if (!bucket) return NULL;

	return bucket->Data;
}

/**
  @brief	getting eap data pointer from bucket
  @return	if hash table is existing then eap data pointer, else NULL
  */
void *hash_table_bucket2data(hash_table_t *table, hash_bucket_t *bucket)
{
	if(!table) return NULL;
	if(!bucket) return NULL;

	return bucket->Data;
}

/**
  @brief	deleting bucket from hash table

  It will delete a bucket from hash table if it exists in the table.
  Then decrease num_element.

  */
void hash_table_delete(hash_table_t *table, hash_bucket_t *bucket)
{
	hash_bucket_t	*cur;
	uint16_t		index;
	int				result;

	if (bucket == NULL) return;

	index = table->Hash(bucket->Data);

	cur = table->T[index];

	if(!cur) return ;

	while(cur) {
		result = table->Compare(cur->Data, bucket->Data);
		if(!result) {
			if(table->freeBucket)
				table->freeBucket(cur->Data);

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

