

// Copyright (c) <2009>, Ivan Novick
// All rights reserved (BSD License)
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Ivan Novick nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL IVAN NOVICK BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Compare the wall clock time of different concurrency mechanisms, under different levels of conention
// mutex, spinlock, reead-write locks, and atomic memory accesses are compared

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <features.h>

// compile command line
// gcc pthread_concurrency.c -D_GNU_SOURCE  -o pthread_concurrency -lpthread -lrt

struct timespec diff_timespec(struct timespec start, struct timespec end);
long long nanosec_elapsed(struct timespec diff);

// change this to make the run time short or longer
#define ITERS_PER_TEST 500000

// global variable will be incremented from many threads
unsigned long counter = 0;

pthread_mutex_t mutex;
pthread_rwlock_t rwlock;
pthread_spinlock_t spinlock;
typedef void* thread_func_t(void*);

void* atomic(void* arg){

	int i;
	for (i = 0 ; i < ITERS_PER_TEST; ++i) {
		__sync_fetch_and_add( &counter, 1 );
	}
}

void* mutexfunc(void* arg){

	int i;
	for (i = 0 ; i < ITERS_PER_TEST; ++i) {
		pthread_mutex_lock(&mutex);
		counter++;
		pthread_mutex_unlock(&mutex);
	}
}

void* spin(void* arg){

	int i;
	for (i = 0 ; i < ITERS_PER_TEST; ++i) {
		pthread_spin_lock(&spinlock);
		counter++;
		pthread_spin_unlock(&spinlock);
	}
}

void* read_lock(void* arg){

	int i;
	for (i = 0 ; i < ITERS_PER_TEST; ++i) {
		pthread_rwlock_rdlock(&rwlock);
		counter++;
		pthread_rwlock_unlock(&rwlock);
	}
}

void* write_lock(void* arg){

	int i;
	for (i = 0 ; i < ITERS_PER_TEST; ++i) {
		pthread_rwlock_wrlock(&rwlock);
		counter++;
		pthread_rwlock_unlock(&rwlock);
	}
}

void do_test(thread_func_t func, unsigned short threads, const char* name){

	int i;
	struct timespec start;
	struct timespec end;
	struct timespec diff;

	pthread_t thread_array[threads];
	counter = 0;

	clock_gettime(CLOCK_MONOTONIC, &start);

	for (i = 0; i < threads; ++i){
		if (pthread_create( &thread_array[i], NULL, func, NULL)){
			printf("error creating threads, exiting");
			exit(1);
		}
	}

	for (i = 0; i < threads; i++)
		pthread_join( thread_array[i], NULL );

	clock_gettime(CLOCK_MONOTONIC, &end);
	diff = diff_timespec(start, end);
	printf("%14s %2d threads took %16lld nanoseconds, global counter = %lu\n", name, threads, nanosec_elapsed(diff), counter);
}

void do_all_tests(int threads){
	printf("*************************************\n");
	fflush(stdout);
	do_test(&atomic, threads, "atomic");
	do_test(&mutexfunc, threads, "mutex");
	do_test(&spin, threads, "spin");
	do_test(&read_lock, threads, "read_lock");
	do_test(&write_lock, threads, "write_lock");
	printf("*************************************\n");
	fflush(stdout);
}

int main(int argc, char** argv)
{
	pthread_mutex_init(&mutex, NULL);
	pthread_rwlock_init(&rwlock, NULL);
	pthread_spin_init(&spinlock, 0);

	do_all_tests(1);
	do_all_tests(8);
	do_all_tests(32); // spin lock will take forever if the number of threads is much higher

	pthread_mutex_destroy(&mutex);
	pthread_rwlock_destroy(&rwlock);
	pthread_spin_destroy(&spinlock);

	return 0;
}

struct timespec diff_timespec(struct timespec start, struct timespec end)
{
	struct timespec result;

	if (end.tv_nsec < start.tv_nsec){ // peform carry like in normal subtraction
		//                123456789
		result.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;        
		result.tv_sec = end.tv_sec - 1 - start.tv_sec;
	}
	else{
		result.tv_nsec = end.tv_nsec - start.tv_nsec;        
		result.tv_sec = end.tv_sec - start.tv_sec;
	}

	return result;
}

long long nanosec_elapsed(struct timespec diff){
	//                                123456789 
	return ((long long)diff.tv_sec * 1000000000) + diff.tv_nsec;
}

// gcc -O2 pthread_concurrency.c -D_GNU_SOURCE  -o pthread_concurrency -lpthread -lrt

