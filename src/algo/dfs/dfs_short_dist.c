/*
 * dfs_short_dist.c
 *
 *  Created on: 2017. 9. 2.
 *      Author: root
 */

#include <stdio.h>

int size;
int min;

#define N 30
#ifdef INPUT_DATA
int map[N][N];
#else
int map[5][5] = {
	{1,1,1,1,1},
	{0,0,0,0,1},
	{1,1,1,1,1},
	{1,0,1,0,0},
	{1,1,1,1,1}
};
#endif

void print_map()
{
	int i, k;
	for (i=0; i<size; i++) {
		for (k=0; k<size; k++) {
			if (map[i][k] == 1) {
				printf("□");
			} else {
				printf("■");
			}
		}
		printf("\n");
	}
	printf("\n");
}

void DFS(int x, int y, int l)
{
	if (x == size-1 && y == size-1) {
		if (min > l) min = l;
		return;
	}
	// visited
	map[y][x] = 0;

	// 위로 갈 수 있다면,
	if (y > 0 && map[y-1][x] == 1)
		DFS(x, y-1, l+1);
	// 아래로 갈 수 있다면,
	if (y < size-1 && map[y+1][x] == 1)
		DFS(x, y+1, l+1);
	// 왼쪽으로 갈 수 있다면,
	if (x > 0 && map[y][x-1] == 1)
		DFS(x-1, y, l+1);
	// 오른쪽으로 갈 수 있다면,
	if (x < size-1 && map[y][x+1] == 1)
		DFS(x+1, y, l+1);

	//map[y][x] = 1;
}

int main()
{
	int i, k;

#ifdef INPUT_DATA
	printf("size: ");
	scanf("%d", &size);
	min = size * size;
	for (i=0; i<size; i++) {
		for (k=0; k<size; k++) {
			scanf("%d", &map[i][k]);
		}
	}
#else
	size = 5;
	min = size * size;
#endif

	print_map();

	DFS(0, 0, 1);

	printf("The shortest: %d\n", min);

	return 0;
}
