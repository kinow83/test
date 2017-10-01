/*
 * dfs.c
 *
 *  Created on: 2017. 9. 2.
 *      Author: root
 */

#include <stdio.h>

int size; //
#define N 30
int map[N][N]; // 정점 인접행렬
int visited[N]; // 방문 배열

void DFS(int v)
{
	int i;

	visited[v] = 1; // 정정점 v를 방문했다고 표시

	// 0 인덱스는 무시하고 진행
	for (i=1; i<=size; i++)
	{
		if (map[v][i] == 1 && !visited[i])
		{
			printf("%d -> %d\n", v, i);

			DFS(i);
		}
	}
}

// 인접행렬
int main()
{
	int start;
	int v1, v2;

	printf("size: ");
	scanf("%d", &size);
	printf("start: ");
	scanf("%d", &start);

	while (1)
	{
		scanf("%d %d", &v1, &v2);
		if (v1 == -1 && v2 == -1) // last
			break;
		map[v1][v2] = 1;
		map[v2][v1] = 1;
	}

	DFS(start);

	return 0;
}

/*
8
1
1 2 1 3 2 4 2 5 4 8 5 8 3 6 3 7 6 8 7 8 -1 -1

 */
