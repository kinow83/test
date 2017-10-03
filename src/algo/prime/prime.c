#include <stdio.h>

#define MAX_SIZE 102400

int main()
{
	int arr[MAX_SIZE];
	int size = 90000;
	int i, k;

	for (i=2; i<=size; i++) {
		arr[i] = i;
	}

	for (i=2; i<=size; i++) {
		for (k=2; k<=size; k++) {
			if (i != k && 
				arr[k] % i == 0) {
				arr[k] = 0;
			}
		}
	}

	for (i=2; i<=size; i++) {
		if (arr[i] != 0) {
			printf("%d ", arr[i]);
		}
	}
	printf("\n");
}
