#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>

void show_csw(struct rusage *usage)
{
	getrusage(RUSAGE_SELF, usage);
	printf("voluntary context switches:   %ld\n", usage->ru_nvcsw);
	printf("involuntary context switches: %ld\n", usage->ru_nivcsw);
}

int main() {
	struct rusage usage;
	int i, j, k = 0;

	for (i = 0; i < 5; i++) {
		for (j = 0; j < 100000000; j++) {
			k += j+i;
		}
		usleep(1);
		show_csw(&usage);
	}
	return 0;
}
