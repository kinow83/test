/*
* @file: pipe.h
*/

#define PLY_CHN	1	// 정책이 변경되었으니 적용 바람
#define PLY_APP	2	// 정책이 적용완료 되었음

int  pipeline[2];
int  pipeline2[2];

int ipc_buf;		// 파이프를 오고 가는 변수

int ppid, pid;	//부모 프로세스, 자식 프로세스

