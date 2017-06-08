/*
 * crypto_lock_test.c
 *
 *  Created on: 2017. 6. 8.
 *      Author: root
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <openssl/crypto.h>
#include <openssl/opensslconf.h>


#if OPENSSL_VERSION_NUMBER < 0x10100004L
	typedef int            TEST_RWLOCK;
	#define CRYPTO_THREAD_lock_new() CRYPTO_get_new_dynlockid()
	#define CRYPTO_THREAD_read_lock(type) \
    if(type) CRYPTO_lock(CRYPTO_LOCK|CRYPTO_READ,type,__FILE__,__LINE__)
	#define CRYPTO_THREAD_read_unlock(type) \
    if(type) CRYPTO_lock(CRYPTO_UNLOCK|CRYPTO_READ,type,__FILE__,__LINE__)
	#define CRYPTO_THREAD_write_lock(type) \
    if(type) CRYPTO_lock(CRYPTO_LOCK|CRYPTO_WRITE,type,__FILE__,__LINE__)
	#define CRYPTO_THREAD_write_unlock(type) \
    if(type) CRYPTO_lock(CRYPTO_UNLOCK|CRYPTO_WRITE,type,__FILE__,__LINE__)
	#define CRYPTO_atomic_add(addr,amount,result,type) \
    *result = type ? CRYPTO_add(addr,amount,type) : (*addr+=amount)

#else
	typedef CRYPTO_RWLOCK *TEST_RWLOCK;
	#define CRYPTO_THREAD_read_unlock(type)  CRYPTO_THREAD_unlock(type)
	#define CRYPTO_THREAD_write_unlock(type) CRYPTO_THREAD_unlock(type)
#endif


#define LOCKABLE

static TEST_RWLOCK lock = 0;
#define max_threads 10
static long values[max_threads] = {0, };

static void myinit(void) {
	lock = CRYPTO_THREAD_lock_new();
}

static void mylock(void) {
	CRYPTO_w_lock(lock);
}

static void myunlock(void) {
	CRYPTO_w_unlock(lock);
}

static long global_count = 0;
void* serialized(void *arg) {
	int i = *(int *)arg;
#ifdef LOCKABLE
	mylock();
#endif
	{
		values[i] = ++global_count;
		printf("[%d] global_count=%ld (%ld) (%ld)\n",
				i, global_count, values[i], pthread_self());
	}
#ifdef LOCKABLE
	myunlock();
#endif
	free(arg);
	return NULL;
}

int main()
{
	int i, j;
	pthread_t pid[max_threads];
	int status[max_threads];

	ERR_load_CRYPTO_strings();

#ifdef LOCKABLE
	myinit();
#endif
	printf("%d\n", lock);
	printf("%d\n", CRYPTO_get_new_dynlockid());
	printf("%s\n", ERR_lib_error_string(0));

	for (i=0; i<max_threads; i++) {
		int *arg = (int *)malloc(sizeof(int));
		*arg = i;
		pthread_create(&pid[i], NULL, serialized, arg);
	}


	for (i=0; i<max_threads; i++) {
		pthread_join(pid[i], (void**)&status[i]);
	}

	int expect = 0;
	int result = 0;
	for (i=0; i<max_threads; i++) {
		expect += i+1;
		result += values[i];
	}

	if ( expect == result ) {
		printf("OK\n");
	} else {
		printf("CRASH %d != %d\n", expect, result);
	}



	return 0;

}
