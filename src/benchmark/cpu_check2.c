/*
 * cpu_check.c
 *
 *  Created on: 2018. 10. 28.
 *      Author: kinow
 */

#include "stdlib.h"
#include "unistd.h"
#include "stdio.h"
#include "string.h"
#include "sys/times.h"
#include "sys/vtimes.h"
#include <pthread.h>
#include <stdint.h>
#include <signal.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int onffo = 0;

void sigint(int signo) {
	onffo ^= 1;
}

void *run(void *arg) {
	int sum;
#if 1
	int r1 = 1;
	while (1) {

		srand(time(NULL));
		int r2 = rand() % 500;
		int i;

		for (i = 0; i < r1 * r2; i++) {
			sum += sum + i + r1 - 1 + r2 * 2;
			for (int j=0; j < 600; j++) {

			}
		}
		r1 = r2;
		if (onffo) {
			usleep(r2);
		} else {
			sleep(1);
		}
	}
#else
	int i;
	while (1) {
		for (i=0; i<10000000000; ++i) {
			sum += sum + i - 2 + 3;
		}
		if (onffo) {
			usleep(100);
		} else {
			sleep(1);
		}
	}
#endif
	return NULL;
}


#define THCNT 4
#define CPU_SAMPLING 10

struct cpu_sample {
	clock_t uptime;
	struct tms sample;
	uint32_t burn;
	uint32_t calm;
	uint8_t bypass;
};

struct cpu_sample last_cpu;
static clock_t burn_slice[CPU_SAMPLING] = {0, };
static clock_t calm_slice[CPU_SAMPLING] = {0, };
static int burn_pos = 0;
static int calm_pos = 0;

//static int numProcessors;

void init() {
	memset(&last_cpu, 0, sizeof(last_cpu));
	last_cpu.uptime = times(&last_cpu.sample);;
	//numProcessors = sysconf(_SC_NPROCESSORS_ONLN);
}

double current_cpu() {
	struct cpu_sample now_cpu;
	double percent;
	const clock_t sample_unit = 5 * sysconf(_SC_CLK_TCK);

	now_cpu.uptime = times(&now_cpu.sample);

	if (now_cpu.uptime <= last_cpu.uptime  ||
		now_cpu.sample.tms_stime < last_cpu.sample.tms_stime ||
		now_cpu.sample.tms_utime < last_cpu.sample.tms_utime) {
		//오버플로우 감지
		percent = -1.0;
	}
	/*
	else if (now_cpu.uptime - last_cpu.uptime < sample_unit) {
		percent = -1.0;
	}
	*/
	else {
		percent = (now_cpu.sample.tms_stime - last_cpu.sample.tms_stime)
				 + (now_cpu.sample.tms_utime - last_cpu.sample.tms_utime);
		percent /= (now_cpu.uptime - last_cpu.uptime);
		//percent /= numProcessors;
		percent *= 100;
		last_cpu.sample = now_cpu.sample;
		last_cpu.uptime = now_cpu.uptime;
	}

	return percent;
}

int main(int argc, char **argv)
{
	pthread_t tid[THCNT];
	int i;

	signal(SIGINT, sigint);

	for(i=0; i<THCNT; i++) {
		pthread_create(&tid[i], NULL, run, NULL);
	}
	int ncore = sysconf(_SC_NPROCESSORS_ONLN);
	double limit = atof(argv[1]);
	int check_time_slice = atoi(argv[2]);
	double min_cpu = (limit * ncore) - (limit * 0.2);

	double cur_cpu;
	init();
	clock_t burn_to, burn_from;
	clock_t calm_to, calm_from;

	while (1) {
		cur_cpu = current_cpu();
		if (cur_cpu  > (double)0) {
			if (cur_cpu > min_cpu) {
				last_cpu.burn = MIN(last_cpu.burn+1, CPU_SAMPLING);
				last_cpu.calm = 0;

				burn_slice[burn_pos] = last_cpu.uptime;
				burn_to =   burn_slice[burn_pos] = last_cpu.uptime;
				burn_from = burn_slice[(burn_pos+1+CPU_SAMPLING) % CPU_SAMPLING];
				burn_pos = (burn_pos+1) % CPU_SAMPLING;
			}
			else {
				// need continues calm ...
				last_cpu.calm = MIN(last_cpu.calm+1 ,CPU_SAMPLING);


				calm_slice[calm_pos] = last_cpu.uptime;
				calm_to =   calm_slice[calm_pos] = last_cpu.uptime;
				calm_from = calm_slice[(calm_pos+1+CPU_SAMPLING) % CPU_SAMPLING];
				calm_pos = (calm_pos+1) % CPU_SAMPLING;
			}

			if (last_cpu.burn >= CPU_SAMPLING) {
				if (check_time_slice) {
					printf("[%ld] burn: long: %lu\n", time(NULL), (burn_to-burn_from)/sysconf(_SC_CLK_TCK));
					if ((burn_to-burn_from)/sysconf(_SC_CLK_TCK) < CPU_SAMPLING) {
						last_cpu.bypass = 1;
						last_cpu.burn = last_cpu.calm = 0;
					}
				}
				else {
					last_cpu.bypass = 1;
					last_cpu.burn = last_cpu.calm = 0;
				}
			}
			if (last_cpu.calm >= CPU_SAMPLING) {
				if (check_time_slice) {
					printf("[%ld] calm: long: %lu\n", time(NULL), (calm_to-calm_from)/sysconf(_SC_CLK_TCK));
					if ((calm_to-calm_from)/sysconf(_SC_CLK_TCK) < CPU_SAMPLING) {
						last_cpu.bypass = 0;
						last_cpu.burn = last_cpu.calm = 0;
					}
				}
				else {
					last_cpu.bypass = 0;
					last_cpu.burn = last_cpu.calm = 0;
				}
			}

			printf("[%ld] bypass [%s] [burn:%d, calm:%d] [%lf, %lf]\n",time(NULL),
					last_cpu.bypass ? "ON" : "OFF",
					last_cpu.burn,
					last_cpu.calm,
					cur_cpu, min_cpu);

			/*
			 * up        up up up       up up up up up
			 *     down            down                 down down
			 *
			 */

		}
		sleep(1);
	}


	for(i=0; i<THCNT; i++) {
		pthread_join(tid[i], NULL);
	}
}


