/*
* @ file : system.h
*	@brief : 시스템 정보를 가져온다
*/


// 시스템 정보 구조체 - cpu, memory, disk
struct system_monitor{
	
	char disk_used[2][5];		// 사용 가능한 디스크 크기
	char disk_mount[2][50];	// 마운트된 디스크 정보	
	char mem_stat;					// 메모리 정보	
	char cpu_stat;					// cpu 정보
};

struct system_monitor System;


