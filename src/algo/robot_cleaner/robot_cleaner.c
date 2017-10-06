#include <stdio.h>

int map[100][50];
int sx, sy;
int n, m, d;
int rx[] = {1, 0, -1, 0};
int ry[] = {0, -1, 0, 1};
int lx[] = {0, -1, 0, 1};
int ly[] = {-1, 0, 1, 0};

char ls[]={'N','E','S','W'};

int M = 2;

void input()
{
	scanf("%d %d", &n, &m);
	scanf("%d %d %d", &sx, &sy, &d);
	for (int i=0; i<n; i++) {
		for (int k=0; k<m; k++) {
			scanf("%d", &map[i][k]);
		}
	}
}

void print()
{
	printf("s======================\n");
	for (int i=0; i<n; i++) {
		for (int k=0; k<m; k++) {
			printf("%d ", map[i][k]);
		}
		printf("\n");
	}
	printf("e======================\n");
}

int chk(int x, int y, int d)
{
	int cs=0;
	int cd[] = {0,0,0,0};
	// 북
	if (x-1<0 || map[x-1][y]>0)
		cd[0]=1;
	// 동
	if (y+1>=m || map[x][y+1]>0)
		cd[1]=1;
	// 남
	if (x+1>=n || map[x+1][y]>0)
		cd[2]=1;
	// 서
	if (y-1<0 || map[x][y-1]>0)
		cd[3]=1;
	for(int i=0; i<4; i++) 
		cs+=cd[i];

	if (cs==4)
		return 1;
	if (cs==3 && cd[(d+2)%4]==0) {
		printf("%c(%d) -> %c(%d)   %d.%d.%d.%d\n", ls[d],d, ls[(d+2)%4], (d+2)%4, cd[0],cd[1],cd[2],cd[3]);
		return 2;
	}
	return 0;
}

void move(int x, int y, int d)
{
	int ret, dx, dy;
	while (1) {
		ret = chk(x, y, d);
		switch (ret) {
			case 0:
				map[x][y] = 2;
				dx = x + lx[d];
				dy = y + ly[d];
				d = (d+3)%4;
				if (map[dx][dy] == 2) {
					x = dx;
					y = dy;
					printf("only left [%d][%d]:%d(%c) >>>\n", x,y,d,ls[d]);
				} else {
					printf("left [%d][%d]:%d(%c) >>>\n", x,y,d,ls[d]);
				}
				break;
			case 1:
				return;
			case 2:
				x += rx[d];
				y += ry[d];
				printf("back [%d][%d]:%d(%c) >>>\n", x,y,d,ls[d]);
				break;
		}
	}
}

int sum()
{
	int s = 0;
	int i, k;
	for (i=0; i<n; i++) {
		for (k=0; k<m; k++) {
			if (map[i][k] >= 2)
				s++;
		}
	}
	return s;
}

int main()
{
	input();
	print();
	move(sy, sx, d);
	print();
	printf("===>%d\n", sum());

}
