#include <stdio.h>

int size;
#define N 30
#if 0
int map[N][N] = 
#else
int map[6][6] = 
#endif
{

	#if 0
	{1,1,1,1,1,1},
	{0,0,1,0,0,1},
	{1,1,1,0,1,1},
	{1,0,0,0,1,0},
	{1,1,1,0,1,0},
	{0,0,1,1,1,1},
	#else
	{1,1,1,1,1,1},
	{1,0,1,0,0,1},
	{1,1,1,0,1,1},
	{1,0,0,0,1,0},
	{1,1,1,0,1,0},
	{1,1,1,1,1,1},
	#endif
};

int x[N*N];
int y[N*N];
int l[N*N];
int cnt;

void enqueue(int _x, int _y, int _l)
{
	x[cnt] = _x;
	y[cnt] = _y;
	l[cnt] = _l;
	cnt++;
}

void print_xy()
{
	int i;
	for (i=0; i<cnt; i++) {
		printf("(%d, %d)\n", y[i], x[i]);
	}
}

void print_path()
{
	int i, k, pos;
	for (i=0; i<size; i++) {
		for (k=0; k<size; k++) {
			for (pos=0; pos<cnt; pos++) {
				if (i == y[pos] && k == x[pos]) {
					break;
				}
			}
			if (pos < cnt) {
				printf("□");
			} else {
				printf("■");
			}
		}
		printf("\n");
	}
	printf("\n");
}

void BFS(int _x, int _y)
{
	int pos = 0;

	// 시작점의 좌표 정보와 길이를 큐에 삽입한다.
	enqueue(_x, _y, 1);

	while ((pos < cnt) &&
		(x[pos] != size-1 || y[pos] != size-1))
	{
		// visited
		map[ y[pos] ][ x[pos] ] = 0;

		// 위로 갈 수 있다면, 위 지점의 좌료와 길이를 큐에 넣는다.
		if (y[pos] > 0 && map[ y[pos]-1 ][ x[pos] ] == 1)
			enqueue(x[pos], y[pos]-1, l[pos]+1);
		// 아래로 갈 수 있다면,
		if (y[pos] < size-1 && map[ y[pos]+1 ][ x[pos] ] == 1)
			enqueue(x[pos], y[pos]+1, l[pos]+1);
		// 왼쪽으로 갈 수 있다면,
		if (x[pos] > 0 && map [ y[pos] ][ x[pos]-1 ] == 1)
			enqueue(x[pos]-1, y[pos], l[pos]+1);
		// 오른쪽으로 갈 수 있다면,
		if (x[pos] < size-1 && map[ y[pos] ][ x[pos]+1 ] == 1)
			enqueue(x[pos]+1, y[pos], l[pos]+1);
		// 큐 다음 순서의 지점을 방문한다.
		pos++;
	}

	if (pos < cnt) {
		printf("The shotest path: %d\n", l[pos]);
	}
}

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

int main()
{
	int i, k;

#if 0
	printf("size: ");
	scanf("%d", &size);
	for (i=0; i<size; i++) {
		for (k=0; k<size; k++) {
			scanf("%d", &map[i][k]);
		}
	}
#else
	size = 6;
#endif

	print_map();

	BFS(0, 0);

//	print_path();

//	print_xy();

	return 0;
}
