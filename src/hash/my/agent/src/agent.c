/*
*	@ file  : agent.c
* @ brief : 
* @ include : ../include
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/types.h>
#include <pthread.h>
#include <mysql.h>

#define LOCALHOST	"localhost"
#define HOST			"127.0.0.1"
#define USER			"root"

/*
* include 순서 지켜 줄것
*/
#include "../include/port.h"
#include "../include/ifconfig.h"
#include "../include/common.h"	//일반 설정 해데
#include "../include/db.h"
#include "../include/command.h"
#include "../include/config.h"
#include "../include/time.h"
#include "../include/system.h"
#include "../include/pcap.h"
#include "../include/facke_page.h"
#include "../include/net_zone.h"





extern struct system_monitor System;	// 에이전트 시스템 정보
void *pcap_thread_func( void *data );	// 패킷 캡쳐 쓰레드
void *alive_thread_func( void *data );	// 실시간으로 에이전트 시스템 정보 전달 쓰레드
void *timer_thread_func( void *data );	// 실시간 타이머 쓰레드
void *log_thread_func( void *data );		// LOG 전송 쓰레드
void sort_func();												// LOG 중복을 제거 하기 위한 sort 함수
void create_new_file();									// hour 가 변경될때 마다 LOG 파일은 교체 되어야 한다.
void *live_log_thread_func( void *sock );	// 실시간 로그 보기 요청 쓰레드



int main(int argc, char **argv){
		
	int connect_cnt=0;		
	struct sockaddr_in master_addr;
	
	pthread_t pcap_thread;	// pcap 쓰레드
	pthread_t timer_thread;	// timer 쓰레드
	pthread_t live_log_thread;		// 실시간 로그 요청 쓰레드
	pthread_t arp_packet_thread;	// 네트워크 호스트 보기 요청 쓰레드
		
	int status;	
	int delaytime=0;
	int refresh = 0;	// MASTER와 접속이 끊어졌는지를 감지하는 변수 - 정해진 값 이상이 되면 MASTER와 접속이 끊어짐,
	
	// 현재 시간 갱신
	get_localtime();
	
	// pipe 개설(차단 프로세스와 agent 프로그램간에 통신을 위한 파이프)
	if( pipe(pipeline) == -1)
	{
		puts("pipe() error");
		exit(1);
	}
	if( pipe(pipeline2) == -1)
	{
		puts("pipe() error");
		exit(1);
	}
	
	// 모든 쓰레드의 동작이 STATUS로 상태 표시 된다.
	STATUS = STOP;	// 모든 쓰레드 STOP
	FAKE = F_STOP;	// 차단 PROCESS STOP
	
	//(차단 프로세스는 agent 정상 실행시 한번만 호출되어야 함-MASTER 프로그램과 연결이 끊어진후 AGENT가 다시 연결 하는 경우가 발생하므로)
	
	
	// 설정 파일 불러오기
	//answer[0]-ID, answer[1]- DEV, answer[2]- master ip, answer[3]- password, answer[4]-URL_PATH
	get_conf();
	
	// ifconfig(네트워크 장치 설정 가져오기) 의 ip, mac, broadcast, netmask, mtu 가져오기
	get_ifconfig(&ifcg, answer[1]);
	strcpy(ifcg.eth, answer[1]);
	
	// 출력
	printf("┌────< ifconfig >─────┐\n");
	printf("│DEV : \t%s\n",ifcg.eth);
	printf("│ETHER : \t%s\n",ifcg.mac);
	printf("│IP : \t\t%s\n",ifcg.ip);
	printf("│BROADCAST : \t%s\n",ifcg.broadcast);
	printf("│NETMASK : \t%s\n",ifcg.netmask);
	printf("│MTU : \t%d\n",ifcg.mtu);
	  puts("└───────────────┘");	
	
	// LOG 디렉토리 생성(log 파일이 담길 디렉토리)
	if(access(log_dir,0) != 0){
		mkdir(log_dir);
	}	
	
	// 에이전트 번호 확인(에이전트가 기존에 한번 연결된 적이 있다면 에이전트 번호가 있을 것이다)
	// moduler = 31
	if(strlen(answer[0]) != 0){
		agent_num = atoi(answer[0]);
		trans_time = agent_num % moduler;	// 해당 에이전트가 LOG 파일을 보내야 할 시간
		  
		printf("I am AGENT[%d]\t", agent_num);
		printf("I will send LOG file %dmin\n", trans_time);		  
		
	}
	else{
		agent_num = 0;
		printf("I yet haven't agent ID\n");
	}
	
	/////////////////////////////////////////////////////////////
	// command rutine
	/////////////////////////////////////////////////////////////
	
RESTART:	// master가 다운시 에이전트 다시 접속 시도
	
	command_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(command_sock == -1){
		puts("AGENT SOCK ERROR");
		exit(-1);
	}
	
	// sock 초기화
	memset(&master_addr, 0, sizeof(master_addr));
	master_addr.sin_family = AF_INET;
	master_addr.sin_addr.s_addr = inet_addr(answer[2]);
	master_addr.sin_port = htons(MASTER_PORT);	
	
	
	printf("┌────< Master connection >─────┐\n");
	printf("│ MASTER IP     %s          │\n",answer[2]);
	printf("│ MASTER PORT   %d                    │\n",MASTER_PORT);
	  puts("└───────────────────┘");
	
	// 마스터와 접속을 기다림
	while( connect(command_sock, (struct sockaddr*) &master_addr, sizeof(master_addr)) == -1 ){
		sleep(5);
		printf("[CONNECT] master connection attemptedcount\n");
	}
	
	// 보낼 데이터 와 받을 데이터 초기화
	memset(&send_cmd, 0, sizeof(send_cmd));
	memset(&recv_cmd, 0, sizeof(recv_cmd));
	
	// 전송 타입 
	// 1. ID 있는 경우	// 2. ID 없는 경우
	if(agent_num != 0){	// ID 값이 있다면 ID를 요청 한다.
		send_cmd.signal = HELLO_WITH_ID;
		send_cmd.data = agent_num;
		puts("[COMMAND] SEND : AGENT ----> RECV : MASTER [HELLO_WITH_ID]");
	}
	else{	// ID 값이 존재하여 ID를 요청하지 않는다.
		send_cmd.signal = HELLO_NO_ID;
		puts("[COMMAND] SEND : AGENT ----> RECV : MASTER [HELLO_NO_ID]");
	}
	
	// SEND
	write(command_sock, &send_cmd, sizeof(send_cmd));
	
	while(1){
		
		// MASTER 가 접속이 끊어 졌을 경우 모든 쓰레드를 종료 한다.
		if(tz.sec == delaytime){		
			refresh++;
			if(refresh >= REFRESH_TIME){	// 연속된 refresh가 발생하여 12 이상이 되면 MASTER와 연결이 끊어졌다고 판단
				printf("[RECONNECTION] attemp connect:%d\n",refresh);
				refresh = 0;
				STATUS = STOP;	// 모든 쓰레드 상태 ALL STOP
				
				close(command_sock);	// MASTER와 연결이 끊어져 명령 소켓을 종료한다.
				goto RESTART;					// 다시 연결 접속 시도
			}
		}
		
		// 명령 구조체 초기화
		memset(&send_cmd, 0, sizeof(send_cmd));
		memset(&recv_cmd, 0, sizeof(recv_cmd));
			 	
		puts("┌───────────────────┐");
		puts("│[COMMAND] Waiting for COMMAND...      │");
		puts("└───────────────────┘");
		
		
		// MASTER와 연결이 이루어져 명령을 전달 받음
		if ( read(command_sock, &recv_cmd, sizeof(recv_cmd)) != 0 ){
			
			// 명령어 종류 판단
			switch(recv_cmd.signal){

				////////////////////////////////////////////////////
				// ACK_WITH_ID - 에이전트 번호 할당 받음
				////////////////////////////////////////////////////			
				case ACK_WITH_ID:
					
					// 에이전트 번호가 없어서 에이전트 번호를 요청하는   연결 요청 신호를 보냄				
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [ACK_WITH_ID]");
					
					// MAX_AGENT error : 최대 30대 까지 연결 가능
					if(recv_cmd.data > MAX_AGENT){
						printf("[MAX_AGENT] MAX_AGENT is %d\n",MAX_AGENT);
						exit(1);								
					}
					
					// ID 번호를 부여 받으면 설정 파일에 ID를 기입한다 - 파일 내용을 다시 쓴다
					FILE *fp = fopen(CONF_FILE, "w");		
					if(fp == NULL){
						puts("agernt.conf file can not open()");
						exit(1);
					}
		
					// 설정 파일 다시 쓰기 - 할당 받은 번호를 추가하기 위해서..//
					char buffer[CONFIG_BUFSIZE];
		
					sprintf(buffer,"ID=%d\n", recv_cmd.data);
					fputs(buffer, fp);
					sprintf(buffer,"DEV=%s\n",answer[1]);
					fputs(buffer, fp);					
					sprintf(buffer,"MASTER=%s\n",answer[2]);
					fputs(buffer, fp);
					sprintf(buffer,"PASSWORD=%s\n",answer[3]);
					fputs(buffer, fp);					
					sprintf(buffer,"URL_PATH=%s\n",answer[4]);
					fputs(buffer, fp);
					fclose(fp);
		
					// 설정 다시 파일 불러오기
					get_conf();
		
					// agent ID 재 부여
					agent_num = atoi(answer[0]);
					
					// LOG 전송 시간 계산
					trans_time = agent_num % moduler;
									  	
					printf("[ALRAM] I am AGENT[%d] \t", agent_num);
					printf("I will send LOG file %dmin\n", trans_time);
					
					// 쓰레드 상태 - 시작
					STATUS = START;
	
					// pcap 쓰레드 시작
					if(( status = pthread_create( &pcap_thread, NULL, &pcap_thread_func, (void*)NULL)) != 0 ) {
						printf("pcap_thread error : %s\n", strerror(status));
					}
						
					// timer 쓰레드 시작
					if(( status = pthread_create( &timer_thread, NULL, &timer_thread_func, (void*)NULL)) != 0 ) {
						printf("timer_thread error : %s\n", strerror(status));
					}
					
					// 미러링 포트의 IP를 DB에 전송
					if( send_mirror_ip(answer[2], "root", answer[3]) !=0 )
						puts("SEND Mirror ip");
					
					break;
				
					
				////////////////////////////////////////////////////
				// ACK_OK - 에이전트가 시작해도 좋다는 시그널
				////////////////////////////////////////////////////		
				case ACK_OK :
					
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [ACK_OK]");
					
					// 쓰레드 상태 - 시작 				
					STATUS = START;

					// pcap 쓰레드 시작
					if(( status = pthread_create( &pcap_thread, NULL, &pcap_thread_func, (void*)NULL)) != 0 ) {
						printf("pcap_thread error : %s\n", strerror(status));
					}
						
					// timer 쓰레드 시작
					if(( status = pthread_create( &timer_thread, NULL, &timer_thread_func, (void*)NULL)) != 0 ) {
						printf("timer_thread error : %s\n", strerror(status));					
					}
					
					// 미러링 포트의 IP를 DB에 전송
					if( send_mirror_ip(answer[2], "root", answer[3]) !=0 )
						puts("SEND Mirror ip");
				
					break;
				
					
				////////////////////////////////////////////////////
				// ACK_INVALID - 잘못된 연결 / 에이전트 프로그램 종료 시킨다.
				////////////////////////////////////////////////////
				case ACK_INVALID:
				
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [ACK_INVALID]");
					puts("[ERROR] ./conf/config file syntax error or program error");					
					close(command_sock);
					
					// 프로그램 종료	
					exit(1);
				
					
				//////////////////////////////////////////////////////////////
				// REQPOLY - 정책이 변경되어 다시 다운로드 하라는 시그널
				/////////////////////////////////////////////////////////////								
				case REQ_POLY:
					
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [REQ_POLY]");
					
					ipc_buf = PLY_CHN;	// 정책이 변경됨을 차단 프로세스에게 알림					
					puts("[PIPE] SEND : LOG -> RECV : FAKE [PLY_CHN]");
					
					write(pipeline[1], &ipc_buf, sizeof(ipc_buf));
					
					if( read(pipeline2[0], &ipc_buf, sizeof(ipc_buf)) != -1)
					{						
						if(ipc_buf == PLY_APP)	// 정책 적용이 완료 되었음.
						{
							puts("[PIPE] SEND : FAKE -> RECV : LOG [PLY_APP]");							
							
							send_cmd.signal = RPL_POLY;	// 정책 적용 응답
							send_cmd.data = 0;
							
							write(command_sock, &send_cmd, sizeof(send_cmd));	// 웹으로 정책이 완료되었음을 알림
							puts("[COMMAND] SEND : AGENT -> RECV : MASTER [PLY_POLY]");							
						}
						else
						{
							puts("[PIPE] Not found Define");
						}
					}
									
					break;


				///////////////////////////////////////////////////
				// REQLIVE - 실시간 LOG 요청
				///////////////////////////////////////////////////
				case REQ_LIVE:
				
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [REQ_LIVE]");
					
					// 실시간 로그 쓰레드 시작
					if(( status = pthread_create( &live_log_thread, NULL, &live_log_thread_func, (void*)NULL)) != 0 ) {
						printf("live_log_thread error : %s\n", strerror(status));					
					}
				
					break;


				///////////////////////////////////////////////////
				// REQ_NET_ZONE - 소속 네트워크 호스트 질의
				///////////////////////////////////////////////////
				case REQ_NET:
					
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [REQ_NET]");
					
					// ping_packet을 보내는 쓰레드 시작
					if(( status = pthread_create( &arp_packet_thread, NULL, &ping_packet_thread_func, (void*)NULL)) != 0 ) {
						printf("arp_packet_thread error : %s\n", strerror(status));
					}
					
					pthread_join(arp_packet_thread, NULL);
					
					send_cmd.signal = RPL_NET;	// 소속 네트워크 호스트 보기 응답
					send_cmd.data = 0;
							
					write(command_sock, &send_cmd, sizeof(send_cmd));	// 웹으로 소속 네트워크 호스트 보기 응답
					puts("[COMMAND] SEND : AGENT -> RECV : MASTER [PLY_NET]");
					break;
				
				default:
					break;
			}
		}
		
		// 마스터와 접속이 끊어졌을 경우
		else{			
			delaytime = tz.sec;
		}
		
		
		////////////////////////////////////////////////////////////////////////
		// FORK - 차단 프로세스
		////////////////////////////////////////////////////////////////////////
		if( FAKE == F_STOP )	// 차단 프로세스를 한번만 실행 시킨다
		{			
			FAKE = F_START;
			
			pid = fork();
						
			if(pid > 0){
				ppid = pid;				
			}
			
			// 차단 프로세스 생성
			else if(pid == 0){
				puts("[FAKE] Fack page PROCESS START");
				fake_page_process(answer[5]);
			}
			
			else if(pid == -1){
				puts("[FAKE] Fack page PROCESS END");
				exit(0);
			}
		}		
	}
	
	//pthread_join(alive_thread, NULL);
	pthread_join(pcap_thread, NULL);
	pthread_join(timer_thread, NULL);
}


// 실시간 로그 보기 요청 쓰레드
void *live_log_thread_func( void *sock )
{
	// pcap 실행
	live_pcap_capture(answer[1], pcap_filter, agent_num, answer[2], USER, answer[3], command_sock);	
}


// pcap 쓰레드
void *pcap_thread_func( void *data )
{	
	printf("[PCAP START]\n");	
	// pcap 실행
	pcap_capture(answer[1], pcap_filter);	
}



// 에이전트 시스템 상태를 주기적으로 보낸다
void *alive_func(char *host, char *user, char *password)
{
	char query[400];
	int query_stat;	
	char cpu, mem;
	MYSQL       *alive_connection=NULL,
							alive_conn;
	
	mysql_init(&alive_conn);
	
	alive_connection = mysql_real_connect(&alive_conn, host, user, password, NULL, 0, (char *)NULL, 0);	
	if (alive_connection == NULL){			
			return;
	}
	
	// alive_value를 주기적으로 1, -1, 1, -1 ...을 보내면서 에이전트가 살아있다는 신호를 디비에 기록
	alive_value *= -1;
	
	// 시스템 정보를 가져옴	
	system_moniter(alive_interval);		
			
	// @ query send system_moniter
	sprintf(query, "UPDATE %s SET save_time=%d, cpu=%d, mem=%d, disk1_name='%s', disk1='%s', disk2_name='%s', disk2='%s' where id=%d",		AGENT_INFORMATION_STATUS, alive_value, System.cpu_stat, System.mem_stat, System.disk_mount[0], System.disk_used[0],System.disk_mount[1], System.disk_used[1], agent_num);

	query_stat = mysql_real_query(alive_connection, query, strlen(query));			
	query_invalied(alive_conn, query_stat);			
	mysql_close(alive_connection);
	return;	
}


// timer 쓰레드
void *timer_thread_func( void *data )
{
	
	pthread_t log_thread;
	int status;	
	int start = 1;			// 파일을 한 순간에 하나만 생성	
	FILE *traffic_fd;		// 트래픽 파일 디스크립터
	char buf[64];
	
	printf("[TIMER START]\n");	
	
	while(STATUS == START){
		
		sleep(1);	// 1분 마다 시각 체크
		get_localtime();
		
		
		// LOG 파일을 보낸다. 보내야 할 분에 log_send_sec(10) 초가 되면....
		// log_send_sec 를 10초로 정한 특별한 이유는 없다.
		if(trans_time == tz.min && tz.sec == log_send_sec)
		{
			// 로그 전송 쓰레드 시작
			if(( status = pthread_create( &log_thread, NULL, &log_thread_func, NULL)) != 0 ) {
				printf("log_Thread error : %s\n", strerror(status));				
			}
		}
		
		// 매 분 1초		
		if( tz.sec == 1 )
		{
			// 에이전트 시스템 상태 전송
			alive_func(answer[2], USER, answer[3]);
		}
		
		
		// 정시  - 새로운 로그 파일로 대체 , 트래픽 변수 초기화
		if(tz.sec == 0 && tz.min == 0 && start == 1)
		{		
			// 새로운 파일 생성
			create_new_file();
			
			// 트래픽 초기화
			if( (traffic_fd = fopen(TRAFFIC_FILE, "w")) != NULL )		
			{
				// 파일에서 트래픽 가져오기
				sprintf(buf, "%d-%d-%d\t%d\t%d\t%d\t%d\t%d\t%d", tz.pre_year, tz.pre_mon+1, tz.pre_mday,   tz.pre_hour, traffic.TByte, traffic.GByte, traffic.MByte, traffic.KByte, traffic.Byte);			
				fputs(buf, traffic_fd);
				
				// 트래픽 변수 초기화
				memset(&traffic, 0, sizeof(traffic));
				fclose(traffic_fd);			
				printf("[SAVE] %s\n",buf);				
			}
			else
			{
				puts("[TRAFFIC]TRAFFIC_FILE cant not create file");
			}
						
			start = 0;	// 로그 파일 대체 루틴이 한번만 실행 되게 하기 위해서...
		}
	
		if(tz.sec == 1 && tz.min == 0){		
			start = 1;
		}
	}
	puts("[EXIT] timer_thread_func");
	pthread_exit(0);	
}


// LOG 전송 쓰레드
void *log_thread_func( void *data )
{
	char query[200];
	int query_stat;
	MYSQL       *log_connection=NULL, 
							log_conn;
	
	puts("[ LOG START ]");	
	
	// 로그 파일 sorting - 불필요한 중복 데이터가 존재하기 때문에 중복을 제거 한다
	// 패킷 캡쳐시 하나의 연결에 대한 비슷한 패킷들이 존재한다. 이러한 패킷 캡쳐 데이터는 지워도 상관없음.
	sort_func();
	
	
	// LOG를 디비에 전송
	mysql_init(&log_conn);	
	log_connection = mysql_real_connect(&log_conn, answer[2], "root", answer[3], NULL, 0, (char *)NULL, 1);	
	if (log_connection == NULL){			
			return;
	}	
	
	sprintf(query, "./%s/AG%02d%02d",  log_dir, tz.pre_mday, tz.pre_hour);
	
	if( access(query, 0) ==0 && agent_num != 0)
	{
	
		//로그 파일 전송
		sprintf(query, "LOAD DATA local INFILE './%s/%s' into table %s%d.AG%02d%02d", log_dir, LOGFILE, AGENT,agent_num, tz.pre_mday, tz.pre_hour);
		query_stat = mysql_real_query(log_connection, query, strlen(query));
		query_invalied(log_conn, query_stat);
		printf("[LOG] SEND FILE : %s\n", query);
					
		// 트래픽 전송
		sprintf(query, "LOAD DATA local INFILE './%s' into table %s%d", TRAFFIC_FILE, STATICS_AGENT, agent_num);
		query_stat = mysql_real_query(log_connection, query, strlen(query));		
		query_invalied(log_conn, query_stat);					
		printf("[LOG] SEND FILE : %s\n", query);
			
		puts("[LOG] PRE LOG FILE DELETE");
			
		sprintf(query, "%s/AG%02d%02d", log_dir, tz.pre_mday, tz.pre_hour);
		unlink(query);
			
		sprintf(query, "%s/%s", log_dir, LOGFILE);
		unlink(query);
			
		puts("[FISHED] LOG SEND TO DBMS");			
	}
	
	mysql_close(log_connection);
	pthread_exit(0);	
}


// LOG 파일 내용 SORTING
// 로그 파일 sorting - 불필요한 중복 데이터가 존재하기 때문에 중복을 제거 한다
// 패킷 캡쳐시 하나의 연결에 대한 비슷한 패킷들이 존재한다. 이러한 패킷 캡쳐 데이터는 지워도 상관없음.
void sort_func()
{
	
	char system_sort[64];
	
	// 필드(컬럼)을 이용한 sort 명령어	
	sprintf(system_sort,"sort -u %s/AG%02d%02d | sort -k 7 > %s/%s", log_dir, tz.pre_mday, tz.pre_hour, log_dir,LOGFILE);		
	system(system_sort);
	
	return;
}


// LOG를 DB에 전송시 파일 자체로 전송(전송 속도 향상)하기 때문에
// 매 시, LOG 파일을 생성해주면서 새로 생성된 LOG 파일에 기록하게 된다.
void create_new_file()
{
	if(fd)
	{
		close(fd);
		puts("[PCAP] NEW LOG FILE");
	}
	
	sprintf(save_file, "%s/AG%02d%02d",log_dir, tz.mday, tz.hour);	
	fd = open(save_file, O_RDWR | O_CREAT);		// 백업 파일 open
	return;
}
