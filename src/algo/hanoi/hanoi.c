#include <stdio.h>

int count = 0;

void move(int from, int to)
{
	count++;
	//printf("%d => %d\n", from, to);
}

//                      1       2       3
void hanoi(int n, int from, int by, int to)
{
	if (n == 1) {
		move(from, to);
		return;
	}
	// 1에 있는 n-1개를 3을 이용해서 2로 옮긴다.
	hanoi(n-1, from, to, by);
	// 1에 있는 1개를 3으로 옮긴다.
	move(from, to);
	// 2에 있는 n-1개를 1을 이용해서 3으로 옮긴다.
	hanoi(n-1, by, from, to);
}

void main()
{
	hanoi(5, 1, 2, 3);
	printf("count = %d\n", count);
}
