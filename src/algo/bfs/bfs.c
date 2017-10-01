/*
1-2
1-3
2-4
2-5
3-4
4-5
4-6
5-6
*/

#include <stdio.h>
#define N 30
int size; // 입력되는 정점의 최대값
int rear, front; // 큐 위치 포인터
int map[N][N]; // 인접(정점의)행렬
int queue[N]; // 방문할 정점의 큐
int visited[N]; // 방문한 배열

void BFS(int v)
{
	int i;

	visited[v] = 1; // 시작 정점 v를 방문했다고 표시
	printf("[%d] start\n", v);

	queue[rear++] = v; // 큐에 방문할 v를 삽입하고 rear 1 증가

	// front가 rear과 같거나 크면 종료 (큐가 비었다면)
	while (front < rear)
	{
		// 큐에 방문할 v를 꺼냄 
		v = queue[front++];
		for (i=1; i<=size; i++)
		{
			// (인접행렬) 정점 v와i가 만나고 정점 i가 방문하지 않았다면 방문
			if (map[v][i] == 1 && !visited[i])
			{
				visited[i] = 1; // 정점 i를 방문했다고 표시
				printf("%d -> %d\n", v, i); // 정점 v에서 i로 이동
				queue[rear++] = i; // 큐에 다음에 방문할 정점 i를 삽입
			}
		}
	}
}

void print_map()
{
	int i, k;
	for (i=1; i<=size; i++) {
		for (k=1; k<=size; k++) {
			printf("[%d][%d] = %d\n", i, k, map[i][k]);
		}
	}
}

int main()
{
	int start;
	int v1, v2;

	printf("size: ");
	scanf("%d", &size);
	printf("start: ");
	scanf("%d", &start);

	while (1) {
		scanf("%d%d", &v1, &v2);
		if (v1 == -1 && v2 == -1)
			break;
		map[v1][v2] = 1;
//		map[v2][v1] = 1;
	}

	print_map();

	BFS(start);

	return 0;
}

/*
size: 6
start: 1
1 2 1 3 2 4 2 5 3 4 3 6 4 5 4 6 5 6 -1 -1
*/
