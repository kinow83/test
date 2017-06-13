/*
 * pthread_rwlock.cpp
 *
 *  Created on: 2017. 6. 13.
 *      Author: root
 */

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <map>


typedef struct std_info {
	int cnt1;
	int cnt2;
} ST_STDINFO;

pthread_rwlock_t g_rwlock;

typedef std::map<int, ST_STDINFO> map_stdinfo_t;
typedef map_stdinfo_t::iterator map_stdinfo_it;
map_stdinfo_t g_stdinfo;

void add_stdinfo(int cnt) {
	for (int i=0; i<cnt; i++) {
		ST_STDINFO std;
		std.cnt1 = rand() % 500000;
		std.cnt2 = rand() % 500000;
		g_stdinfo.insert(std::make_pair(i, std));
	}
}

void* read_stdinfo(void *arg) {
	int tmp;
	printf("start read_stdinfo\n");
	while (1) {
		pthread_rwlock_rdlock(&g_rwlock);
		{
			//printf("%ld read lock\n", pthread_self());
			map_stdinfo_it it = g_stdinfo.begin();
			while (it != g_stdinfo.end()) {
				if (it->second.cnt1 > it->second.cnt2) {
					tmp = it->second.cnt2;
				}
				it++;
			}
		}
		pthread_rwlock_unlock(&g_rwlock);
		sleep(4);
		//printf("%ld read unlock\n", pthread_self());
	}
	return NULL;
}

void* write_stdinfo(void *arg) {
	printf("start write_stdinfo\n");
	while (1) {
		pthread_rwlock_wrlock(&g_rwlock);
		{
			printf("%ld write lock\n", pthread_self());
			g_stdinfo.clear();
			add_stdinfo(rand() % 1000000);
		}
		pthread_rwlock_unlock(&g_rwlock);
		printf("%ld write unlock\n", pthread_self());

		sleep(4);
	}
	return NULL;
}

#define PEXIT(ret, x) ({ if (ret) { perror(x); exit(1); } })

int main()
{
	add_stdinfo(1000000);

	int ret;

	ret = pthread_rwlock_init(&g_rwlock, NULL);
	PEXIT(ret, "pthread_rwlock_init");

	pthread_t read_th[3];
	pthread_t write_th[1];

	for (int i=0; i<sizeof(read_th)/sizeof(pthread_t); i++) {
		ret = pthread_create(&read_th[i], NULL, read_stdinfo, NULL);
		PEXIT(ret, "pthread_create for read");
	}

	for (int i=0; i<sizeof(write_th)/sizeof(pthread_t); i++) {
		ret = pthread_create(&write_th[i], NULL, write_stdinfo, NULL);
		PEXIT(ret, "pthread_create for write");
	}

	sleep(60);
	pthread_rwlock_destroy(&g_rwlock);
	return 0;
}
