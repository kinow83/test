#include <iostream>
#include <stdio.h>
using namespace std;

int ARR[100];
int N;
int COUNT;

void input() {
	cin >> N;
	for (int i=0; i<N; i++)
		ARR[i] = i;
}

void RR(int start, int end) {
	// ------>
	int last = ARR[end];
	for (int i=end; i>start; i--)
		ARR[i] = ARR[i-1];
	ARR[start] = last;
}

void LR(int start, int end) {
	// <------
	int first = ARR[start];
	for (int i=start; i<end; i++)
		ARR[i] = ARR[i+1];
	ARR[end] = first;
}

void print_arr() {
	for (int i=0; i<N; i++) {
		printf("%d ", ARR[i]);
		COUNT++;
	}
	printf("\n");
}

void perm_order(int idx) {
	if (idx == N) {
		print_arr();
		return;
	}

	for (int i=idx; i<N; i++) {
		RR(idx, i);
		perm_order(idx+1);
		LR(idx, i);
	}
}

int main() {
	input();
	perm_order(0);
	return 0;
}
