#include <stdio.h>

int max_cost;
int last_day;
#define NUM_MAX 15
struct sched {
	int day;
	int cost;
};
struct sched S[NUM_MAX];
#define MAX(a, b) (a>b?a:b)

void input() {
	scanf("%d", &last_day);
	int i;
	for (i=0;i<last_day;i++) {
		scanf("%d %d", &S[i].day, &S[i].cost);
	}
	max_cost = -1;
}

void process(int day, int cost)
{
	// day == last_day "==" 비교이유?
	// ex> 첫날 업무량이 3일인 경우 cost 계산도 못해보고 끝... 
	// S[day]는 총 업무day의 다음날이다 index이므로
	// day  1 2 3
	// cost 3 1 1
	if (day == last_day) {
		max_cost = MAX(max_cost, cost);
		return;
	}
	// S[day] 날에 일한다.
	if (day + S[day].day <= last_day)
		process(day + S[day].day, cost + S[day].cost);
	// day+1 날에 일한다.
	if (day + 1 <= last_day)
		process(day + 1, cost);
}

void output() {
	printf("%d\n", max_cost);
}

void main()
{
	input();
	process(0, 0);
	output();
}
