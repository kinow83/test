/*
* @ file : system.c system.h
*	brief  : 에이전트 시스템 정보 가져오기
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "../include/system.h"

//#define KERNERL_2_6
//#define KERNERL_2_4

#define BUFSIZE	256

#define DISK_CMD	"df -h"						// 디스크 정보 보기
#define MEM_CMD	"cat /proc/meminfo"	// 메모리 정보 보기
#define CPU_CMD	"cat /proc/stat"		// CPU 정보 보기

#define keyword		"cpu"
#define keyword1	"MemTotal"
#define keyword2	"MemFree"

char buf[BUFSIZE];
char garbage[48];






/* get_mem
*/
char total[12]; // 전체 메모리 크기
char freed[12];	// 잔여 메모리 크기
unsigned long tot; 		// 전체 메모리 크기
unsigned long freem; 	// 잔여 메모리 크기


/* get_cpu
*/
double n_total, n_id;
double p_total, p_id;

// 커널 2.4와 커널 2.6의 CPU 정보 보기 형식이 다르다.
#ifdef KERNEL_2_4
char tok[5][12];

#else	//KERNERL_2_6
char tok[8][12];
#endif




/***************************
 *  디스크 정보 얻기       * 
 *!!!!!!!!!!!!!!!!!!!!!!!!!*
 * 파티션 2개 까지 출력가능*
 *!!!!!!!!!!!!!!!!!!!!!!!!!*
 ***************************/
void get_disk(){

	FILE *fp;
	int i=0;	
	char tmp[50];
	int long_string_df=0;
	
	fp = popen(DISK_CMD, "r");
	if(!fp){
					puts("can not found "DISK_CMD);
					return;
	}
	
	fgets(buf, BUFSIZE-1, fp);
	
	for(i=0; i<2; i++){
		fgets(buf, BUFSIZE-1, fp);			// 불필요한 정보 : garbage
		if(long_string_df == 0){
			sscanf(buf,"%s %s %s %s %s %s", tmp, garbage, garbage, garbage, System.disk_used[i], System.disk_mount[i]);
		}
		else{
			sscanf(buf,"%s %s %s %s %s",garbage, garbage, garbage, System.disk_used[i], System.disk_mount[i]);
			long_string_df = 0;
		}
		
		if(strlen(tmp) > 10){
						i--;
						long_string_df = 1;
						tmp[0]='\0';
		}
		else{
		//if(!System.disk_mount[i]){
		//	strcpy(System.disk_used[i], "nul");
		//}
		System.disk_used[i][strlen(System.disk_used[i])] = 0;
		System.disk_mount[i][strlen(System.disk_mount[i])] = 0;
		}
	}
	pclose(fp);
}



/*********************
 * 메모리 정보 얻기 *
 *********************/
void get_mem(){

	FILE *fp;
	fp = popen(MEM_CMD, "r");
	if(!fp){
					puts("can not found "MEM_CMD);
					return;
	}
					
	while(fgets(buf, BUFSIZE-1, fp)){
		
		//MemTotal
		// 불필요한 정보 : garbage
		if(strstr(buf, keyword1)){
			sscanf(buf,"%s %s", garbage, total);
		}
		
		//MemFree		
		if(strstr(buf, keyword2)){
			sscanf(buf,"%s %s", garbage, freed);
			break;
		}
	}
	pclose(fp);
	tot = atoi(total);
	freem = atoi(freed);
	
	System.mem_stat = ( (tot - freem) * 100  / tot );	
}





/*********************
 *프로세스 정보 얻기 *
 *********************/
#ifdef KERNEL_2_4
void get_cpu(int interval){

	FILE *fp;
	
	// first cycle routine
	fp = popen(CPU_CMD, "r");
	if(!fp){
					puts("can not found "CPU_CMD);
					return;
	}
					
	while(fgets(buf, BUFSIZE-1, fp)){
					if(strstr(buf, keyword)){									
							sscanf(buf,"%s %s %s %s %s", tok[0], tok[1], tok[2], tok[3], tok[4]);
							break;
					}
	}
	
	
	p_total = atoi(tok[0])+atoi(tok[1])+atoi(tok[2])+atoi(tok[3])+atoi(tok[4]);
	p_id = atoi(tok[4]);
	
	pclose(fp);
	
	// CPU 사용 비율을 보기 위해서는 딜레이가 필요하다
	sleep(interval);	
	
	// second cycle routine	
	fp = popen(CPU_CMD, "r");
	if(!fp){
					puts("can not found "CPU_CMD);
					return;
	}	
	
	while(fgets(buf, BUFSIZE-1, fp)){
					if(strstr(buf, keyword)){									
							sscanf(buf,"%s %s %s %s %s", tok[0], tok[1], tok[2], tok[3], tok[4]);
							break;
					}
	}
	

	n_total = atoi(tok[0])+atoi(tok[1])+atoi(tok[2])+atoi(tok[3])+atoi(tok[4]);
	n_id = atoi(tok[4]);
	
	
	System.cpu_stat  = ( 1 - ( (n_id - p_id) / (n_total - p_total)) )* 100;
	
	pclose(fp);	
	
}

#else	//KERNERL_2_6
void get_cpu(int interval){

	FILE *fp;
	
	// first cycle routine
	fp = popen(CPU_CMD, "r");
	if(!fp){
					puts("can not found "CPU_CMD);
					return;
	}
					
	while(fgets(buf, BUFSIZE-1, fp)){
					if(strstr(buf, keyword)){
						sscanf(buf,"%s %s %s %s %s %s %s %s", tok[0], tok[1], tok[2], tok[3], tok[4], tok[5], tok[6], tok[7]);
						break;
					}
	}
		
	p_total = atoi(tok[0])+atoi(tok[1])+atoi(tok[2])+atoi(tok[3])+atoi(tok[4])+atoi(tok[5])+atoi(tok[6])+atoi(tok[7]);
	p_id = atoi(tok[4]);
	
	pclose(fp);
	
	// CPU 사용 비율을 보기 위해서는 딜레이가 필요하다
	sleep(interval);
	
	// second cycle routine	
	fp = popen(CPU_CMD, "r");
	if(!fp){
					puts("can not found "CPU_CMD);
					return;
	}	
	
	while(fgets(buf, BUFSIZE-1, fp)){
					if(strstr(buf, keyword)){						
							sscanf(buf,"%s %s %s %s %s %s %s %s", tok[0], tok[1], tok[2], tok[3], tok[4], tok[5], tok[6], tok[7]);
							break;
					}
	}
	

	n_total = atoi(tok[0])+atoi(tok[1])+atoi(tok[2])+atoi(tok[3])+atoi(tok[4])+atoi(tok[5])+atoi(tok[6])+atoi(tok[7]);
	n_id = atoi(tok[4]);
	
	
	System.cpu_stat = ( 1 - ( (n_id - p_id) / (n_total - p_total)) )* 100;
	pclose(fp);
	
}
#endif


// 모든 시스템 정보 가져오기
void system_moniter(int interval){
	get_mem();
	get_disk();
	get_cpu( interval );
}


