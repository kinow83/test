/*
* @ file : system.h
*	@brief : �ý��� ������ �����´�
*/


// �ý��� ���� ����ü - cpu, memory, disk
struct system_monitor{
	
	char disk_used[2][5];		// ��� ������ ��ũ ũ��
	char disk_mount[2][50];	// ����Ʈ�� ��ũ ����	
	char mem_stat;					// �޸� ����	
	char cpu_stat;					// cpu ����
};

struct system_monitor System;


