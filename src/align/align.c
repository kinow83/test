#include <stdio.h>


typedef struct st1 {
	int a;
	char b;
} st1;

typedef struct st2 {
	int a;
	char b;
} __attribute__((packed)) st2;

int main()
{
	int arr[10];
	printf("arr[0] = %p \n", &arr[0]);
	printf("arr[1] = %p \n", &arr[1]);
	printf("arr[2] = %p \n", &arr[2]);
	printf("arr[3] = %p \n", &arr[3]);

	char c[3];
	printf("c[0] = %p \n", &c[0]);
	printf("c[1] = %p \n", &c[1]);

	st1 s1[3];
	printf("s1[0] = %p \n", &s1[0]);
	printf("s1[1] = %p \n", &s1[1]);
	printf("s1[2] = %p \n", &s1[2]);

	st2 s2[3];
	printf("s2[0] = %p \n", &s2[0]);
	printf("s2[1] = %p \n", &s2[1]);
	printf("s2[2] = %p \n", &s2[2]);
}
