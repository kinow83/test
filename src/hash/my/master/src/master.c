/*
* @file : master.c
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/config.h"
#include "../include/command.h"
#include "../include/time.h"
#include "../include/db.h"

#define LISTENSIZE 100						// 최대 소켓 listen 
#define BACKUP_DIR	"log_backup"	// 백업 디렉토리 경로




int backup_time_check();												// 백업 시간 체크
void *timer_thread_func( void *data );					// 실시간 타이머 쓰레드
void *db_backup_thread_func( void *data );			// DB 백업 쓰레드
void *req_policy_thread_func( void *data );			// 정책 적용 쓰레드
void *req_live_thread_func( void *data );				// 실시간 로그 보기 쓰레드
void *req_net_thread_func( void *data );				// 소속 네트워크 보기 쓰레드
void *req_rev_thread_func( void *data );				// 이 전달 로그 보기 쓰레드
void *req_revtime_thread_func( void *data );		// 이 전달 로그 보기 시 얼마나 시간 요소가 걸리는지 알아내는 쓰레드
int agent_cal(int agent_num);										// 에이전트 번호 계산
char* itoa(unsigned int val);										// integer TO string
int file_size(int agent_num, u_short year, u_short mon);	// 파일의 사이즈를 알아냄

// 커맨드 
COMMAND recv_cmd,	// 수신
			 send_cmd;	// 발신

//에이전트 번호
int agent_num;

int AGENT_SOCK[MAX_AGENT+6]; // 연결된 agent들의 소켓 저장 배열


// 웹에서 요청 응답하는 쓰레드들의 전달 변수
struct thread_info{
	int agent_num;
	int web_sock;	
};

// 이전 달 로그 보기 쓰레드 전달 변수
struct thread_rev{
	int agent_num;
	int web_sock;	
	u_short year;
	u_short mon;
};



int main(int argc,char **argv) {
	
	int master_sock;
	int agent_sock;
	int connect_cnt=0;
	
	struct sockaddr_in master_addr;	
	struct sockaddr_in agnet_addr;	
	int agent_addr_size;
	
	pthread_t timer_thread;	// 타이밍 체크(1초 마다)	
	int status;	
	
	
	// LOG 백업 디렉토리 생성
	if(access(BACKUP_DIR,0) != 0){
		mkdir(BACKUP_DIR);		
	}
			
	//디비 초기화 작업 : 약간의 시간소요가 생길 수 있습니다.	
	if( db_initial(HOST, USER, PASSWORD) != 0){
		puts("[DBMS] DATABASE Initialize : failed");		
	}
	puts("[DBMS] DATABASE Initialize : success");
	
	
	
	// time 쓰레드 시작	
	if(( status = pthread_create( &timer_thread, NULL, &timer_thread_func, (void *)NULL)) != 0 ) {
		printf("timer Thread error : %s\n", strerror(status));
		exit(1);
	}
	
	//////////////////////////////////////////////////////////////////////////
	// command 루틴
	//////////////////////////////////////////////////////////////////////////	
	master_sock = socket(PF_INET, SOCK_STREAM, 0); //TCP SOCKET
	if(master_sock == -1){
		puts("[SOCKET] master_sock error");
		exit(1);
	}
	
	
	memset(&master_addr, 0, sizeof(master_addr));
	master_addr.sin_family = AF_INET;
	master_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	master_addr.sin_port = htons(MASTER_PORT);
	
	
	if(bind(master_sock, (struct sockaddr *)&master_addr, sizeof(master_addr)) == -1){
		puts("bind() error");
		exit(1);
	}	
		
	if(listen(master_sock, LISTENSIZE) == -1){
		puts("listen() error");
		exit(1);
	}
		
	agent_addr_size = sizeof(agnet_addr);
	
	while(1){
		
		// 명령 전달 변수 초기화		
		memset(&recv_cmd, 0, sizeof(recv_cmd));
		memset(&send_cmd, 0, sizeof(send_cmd));
		
		puts("┌───────────────────┐");
		puts("│[COMMAND] WAITING... AGENT CONNECTION │");
		puts("└───────────────────┘");
		
		
		agent_sock = accept(master_sock, (struct sockaddr *)&agnet_addr, &agent_addr_size);
		if(agent_sock != -1){			
		
			if ( read(agent_sock, &recv_cmd, sizeof(recv_cmd)) != 0 ){
				
				switch(recv_cmd.signal){					
					
					
					case START_OK:						
						printf("[START] AGENT[%d] START!! \n",recv_cmd.data);
						break;
						
										
					////////////////////////////////////////////////////////////////////////////												
					//1:HELLO_WITH_ID - 에이전트가 접속이 끊어졌다가 재 접속하는 경우
					////////////////////////////////////////////////////////////////////////////							
					case HELLO_WITH_ID :						
						printf("[COMMAND] SEND : AGENT ------>  RECV : MASTER [HELLO_WITH_ID]\n");
						
						// DB에서 에이전트 번호를 가져온다.
						if ((agent_num = get_agent_num(HOST, USER, PASSWORD)) != -1)
						{
							// 유효한 에이전트 ID 값 확인
							if(agent_num > recv_cmd.data || recv_cmd.data > MAX_AGENT)
							{ 
								//에이전트 번호 갱신
								agent_num = recv_cmd.data;
												
								printf("[CONNECTION] Re_Hello ! AGENT[%d]\n",recv_cmd.data);
								
								// 에이전트에게 시작하라는 응답을 보냄.
								send_cmd.signal = ACK_OK;
								send_cmd.data = 0;
	 							write(agent_sock, &send_cmd, sizeof(send_cmd));
							
								// DB에 agent ip 저장하기
								if (insert_ip(agent_num, inet_ntoa(agnet_addr.sin_addr), HOST, USER, PASSWORD) != 0){			
									puts("[STAUTS] Agent ip insert into database : failed");						
								}
							
								// 연결된 agent socket 유지 //
								// 에이전트 번호를 이용한 에이전트 소켓 저장 배열의 인덱스를 참조
								AGENT_SOCK[agent_cal(agent_num)] = agent_sock;
								printf("[SOCK_SAVE] AGENT_SOCK[%d] = sock%d\n",agent_cal(agent_num), agent_sock);
							
							}
							
							// 잘못된 에이전트 ID 값
							else
							{
								printf("[COMMAND] INVALUED agent %d\n",recv_cmd.data);
								send_cmd.signal = ACK_INVALID;
								send_cmd.data = 0;
								write(agent_sock, &send_cmd, sizeof(send_cmd));
								close(agent_sock);
							}
							
						}
						else
						{
							puts("[DBMS] get_agent_num() error");
						}
						
						break;
					
					
					////////////////////////////////////////////////////////////////////////////							
					//2:HELLO_NO_ID	- 에이전트가 처음 접속하는 경우 - 에이전트 번호를 할당해줌
					////////////////////////////////////////////////////////////////////////////
					case HELLO_NO_ID   :
						printf("[COMMAND] SEND : AGENT ------>  RECV : MASTER [HELLO_NO_ID]\n");
						
						// DB에서 에이전트 번호를 가져온다.
						if ((agent_num = get_agent_num(HOST , USER , PASSWORD)) != -1){						
						
							// 에이전트 번호를 디비에(information.status) 기록
							if ( connect_agent(agent_num, HOST , USER , PASSWORD ) == 0){
								printf("[CONNECTION] Hello ! AGENT[%d]\n",agent_num);
								
								//	최초 연결된 에이전트에게 디비 생성
								if ( create_agent_db(agent_num, tz.year, tz.mon+1, HOST , USER , PASSWORD) != 0){
									puts("[DBMS] create_agent_db() error");								
								}
																
								else{
									// 에이전트 번호를 할당 하여 에이전트에게 알려줌									
									printf("[DBMS] SUCCESS! created agent%d database ...\n",agent_num);							
									send_cmd.data = agent_num;
									send_cmd.signal = ACK_WITH_ID;					
									write(agent_sock, &send_cmd, sizeof(send_cmd));
								}
							}
						}
						
						// DB에 agent ip 갱신 하기
						if (insert_ip(agent_num, inet_ntoa(agnet_addr.sin_addr), HOST , USER , PASSWORD) != 0){
							puts("[STAUTS] Agent ip insert into database : failed");						
						}
					
						// agent socket 유지 //
						// 에이전트 번호를 이용한 에이전트 소켓 저장 배열의 인덱스를 참조
						AGENT_SOCK[agent_cal(agent_num)] = agent_sock;
						printf("[SOCK_SAVE] AGENT_SOCK[%d] = sock%d\n",agent_cal(agent_num), agent_sock);
						
						break;
					
					
					////////////////////////////////////////////////////////////////////////////
					// 웹에서 MASTER에게 보내는 시그널
					////////////////////////////////////////////////////////////////////////////
					default		:
						
						////////////////////////////////////////////////////////////////////////////
						//3:REQ_LIVE - 실시간 로그 보기 요청
						////////////////////////////////////////////////////////////////////////////
						
						// 강제 캐스팅 COMMAND형을 CHAR형 으로
						if(strstr((char *)&recv_cmd, WEB_REQ_LIVE)){
							
							pthread_t req_live_thread;	// 실시간 보기 요청 쓰레드
							struct thread_info info;		// 쓰레드 전달 변수
							
							info.web_sock = agent_sock;	//web socket 저장
														
							info.agent_num = atoi((char *)&recv_cmd + 4);	//에이전트 번호 추출
							printf("What do you want agnet[%d] LIVE\n",info.agent_num);
							
							if(AGENT_SOCK[agent_cal(info.agent_num)] != 0){
							
								// 마스터가 에이전트에게 명령을 전달시 read 할 겨우  COMMAND 루틴이 block 되는걸 방지하기 위해 쓰레드 사용
								if(( status = pthread_create( &req_live_thread, NULL, &req_live_thread_func, (void *)&info)) != 0 ) {
									printf("req_live_Thread error : %s\n", strerror(status));									
								}
							}							
							else{
								printf("[REQ_LIVE] ERROR\n");
							}
														
						}


						////////////////////////////////////////////////////////////////////////////						
						//4:REQ_POLY - 정책을 적용하라는 시그널
						////////////////////////////////////////////////////////////////////////////						
						
						// 강제 캐스팅 COMMAND형을 CHAR형 으로
						else if(strstr((char *)&recv_cmd, WEB_REQ_POLY)){
							pthread_t req_policy_thread;	// 정책 적용 쓰레드		
							struct thread_info info;			// 쓰레드 전달 변수
							
							info.web_sock = agent_sock;	//web socket 저장
							
							info.agent_num = atoi((char *)&recv_cmd + 4);	//에이전트 번호 추출
							printf("What do you want agnet[%d] POLICY APPLY\n",info.agent_num);
							
							if(AGENT_SOCK[agent_cal(info.agent_num)] != 0){
								
								// 마스터가 에이전트에게 명령을 전달시 read 할 겨우  COMMAND 루틴이 block 되는걸 방지하기 위해 쓰레드 사용
								if(( status = pthread_create( &req_policy_thread, NULL, &req_policy_thread_func, (void *)&info)) != 0 ) {
									printf("req_policy_Thread error : %s\n", strerror(status));									
								}
							}							
							else{
								printf("[REQ_POLY] ERROR\n");
							}
						}
						

						////////////////////////////////////////////////////////////////////////////
						// REQ_NET_ZONE - 소속 네트워크 호스트 보기 요청
						////////////////////////////////////////////////////////////////////////////
						else if(strstr((char *)&recv_cmd, WEB_REQ_NET)){
							pthread_t req_net_thread; // 소속 네트워크 호스트 보기 쓰레드
							struct thread_info info;	// 쓰레드 전달 변수
							
							info.web_sock = agent_sock;	//web socket 저장
							
							info.agent_num = atoi((char *)&recv_cmd + 3);	//에이전트 번호 추출
							printf("%s\n",(char *)&recv_cmd);
							printf("What do you want agnet[%d] NET ZONE HOST\n",info.agent_num);
							
							if(AGENT_SOCK[agent_cal(info.agent_num)] != 0){
								
								// 마스터가 에이전트에게 명령을 전달시 read 할 겨우  COMMAND 루틴이 block 되는걸 방지하기 위해 쓰레드 사용
								if(( status = pthread_create( &req_net_thread, NULL, &req_net_thread_func, (void *)&info)) != 0 ) {
									printf("req_net_zone_Thread error : %s\n", strerror(status));									
								}
							}							
							else{
								printf("[REQ_NET] ERROR\n");
							}
						}
						

						////////////////////////////////////////////////////////////////////////////
						// REQ_REV - 이 전달 백업로그 보기 요청
						////////////////////////////////////////////////////////////////////////////
						else if(strstr((char *)&recv_cmd, WEB_REQ_REV)){
							pthread_t req_rev_thread;	// 이전달 로그 보기 요청 쓰레드							
							struct thread_rev info;		// 쓰레드 전달 변수							
							char *p;
							
							puts((char *)&recv_cmd);
							
							// token 처리
							// 에이전트 번호 얻기
							p = strtok((char *)&recv_cmd, "-");
							info.agent_num = atoi(p+3);	//에이전트 번호 추출
							
							// 년도 얻기
							p = strtok(NULL,"-");
							info.year = atoi(p);
							
							// 달 얻기
							p = strtok(NULL,"-");
							info.mon = atoi(p);
							
							// 웹 소켓 얻기
							info.web_sock = agent_sock;
							
							printf("What do you want agnet[%d] REQ_REVIVEW HOST\n",info.agent_num);
							
							// 이전달 로그 보기 쓰레드 시작
							if(( status = pthread_create( &req_rev_thread, NULL, &req_rev_thread_func, (void *)&info)) != 0 ) {
									printf("req_rev_Thread error : %s\n", strerror(status));									
							}							
						}
						
						
						////////////////////////////////////////////////////////////////////////////
						// REQ_REVIEW_TIME - 이전달 백업로그를 가져오는데 걸리는 시간 요청
						//								 - 이전달 백업로그 보기는 상당한 시간이 걸린다.
						//								 - 이전달 로그를 임시 테이블에 복구 하는 작없이므로...
						////////////////////////////////////////////////////////////////////////////
						else if(strstr((char *)&recv_cmd, WEB_REQ_REVTIME)){
							pthread_t req_revtime_thread;
							
							struct thread_rev info;	// 쓰레드 전달 변수							
							char *p;
							
							puts((char *)&recv_cmd);
							
							// token 처리
							// 에이전트 번호 얻기
							p = strtok((char *)&recv_cmd, "-");
							info.agent_num = atoi(p+4);	//에이전트 번호 추출
							
							// 년도 얻기
							p = strtok(NULL,"-");
							info.year = atoi(p);
							
							// 달 얻기
							p = strtok(NULL,"-");
							info.mon = atoi(p);
							
							// 웹 소켓 얻기
							info.web_sock = agent_sock;
							
							// "이전달 백업로그를 가져오는 걸리는 시간" 요청 쓰레드 시작
							printf("What do you want agnet[%d] REQ_REVIEW_TIME HOST\n",info.agent_num);
							
							if(( status = pthread_create( &req_revtime_thread, NULL, &req_revtime_thread_func, (void *)&info)) != 0 ) {
									printf("req_revtime_Thread error : %s\n", strerror(status));									
							}
						}
				}
			}
		}
	}
	
	pthread_join(timer_thread, NULL);
//	pthread_join(alive_check_thread, NULL);
	
}


// 실시간 시간 갱신 쓰레드
void *timer_thread_func( void *data )
{
	
	pthread_t db_backup_thread;	// DB 백업 쓰레드
	int status;	
	
	puts("[TIMER] START");	
	
	while(1){
		
		// 매초 마다 시간 갱신
		sleep(1);
		
		// 매 시간 현재 시간을 구조체에 기록한다 -  타이밍 스케줄러에 꼭 필요
		get_localtime();
		
		
		// DB 백업 시간 체그
		if(backup_time_check()){
			
			// DB 백업 쓰레드 체크
			if(( status = pthread_create( &db_backup_thread, NULL, &db_backup_thread_func, NULL)) != 0 ) {
				printf("Thread error : %s\n", strerror(status));				
			}			
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// 매 5초 마다 에이전트 시스템 상태를 검사한다.
		//
		// information 테이블에 디폴트로 save_time 필드는 '1', back_up 필드는 '-1'로 초기화 됨
		//	[에이전트] 에이전트	--> DB : 매 1초 마다 save_time 필드로 '-1' 또는 '1' 값을 번갈아 가며 기록한다.
		// 	[마스터]		마스터  --> DB : 매 5초 마다 save_time, backup_time를 비교하여 같은 값이면 에이전트가 다운된것임(alive = 0)
		//														 : save_time, backup_time 값이 같지 않으면 (에이전트는 정상, alive = 1) 매 5초 마다 back_up 필드에 save_time 필드를 복사한다.
		//														 : 즉, save_time, backup_time의 '1' 또는 '-1' 값으로 에이전트 상태를 체크
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if(tz.sec==5)
		{
			check_alive(HOST, USER, PASSWORD);
		}			
	}	
}


// DB 백업 쓰레드 - 존재하는 에이전트 데이터베이스와 로그와 트래픽 데이터베이스 백업(한달에 한번)
void *db_backup_thread_func( void *data )
{
	db_backup(HOST, USER, PASSWORD, tz.pre_year, tz.pre_mon+1, tz.year, tz.mon+1, BACKUP_DIR);	
}


// DB 백업 시간 검사
int backup_time_check()
{	
	////////////////////////////////////
	//매달 1일 0시 40분 10초에 백업 진행
	////////////////////////////////////
	if( tz.mday == 1 && tz.hour == 0 && tz.min == backup_min && tz.sec == backup_sec){
		puts("[BACKUP] ALAM!!");
		return 1;
	}	
	return 0;
}




// 에이전트 번호 계산 - 0:표준 입력, 1: 표준 출력, 2: 에러, 3: MASTER SOCKET
// 에이전트 소켓을 4번 이후부터 시작됨
// 에이전트 번호 + 4 값이 에이전트 소켓을 저장하는 배열의 인덱스 값으로
// 특정한 에이전트 소켓 정보는 에이전트 소켓 저장 배열에서 항상 같은 위치를 참조한다.
int agent_cal(int agent_num)
{
	return agent_num + 4;	
}



// 정책 적용 쓰레드
void *req_policy_thread_func( void *info )
{
	struct thread_info data;
	COMMAND s_cmd, r_cmd;
	char signal[10];
	
	data.agent_num = ((struct thread_info *)info)->agent_num;
	data.web_sock = ((struct thread_info *)info)->web_sock;
	
	
	// 에이전트에게 정책을 다시 적용 하라는 시그널 보냄
	s_cmd.signal = REQ_POLY;
	s_cmd.data = 0;
	write(AGENT_SOCK[agent_cal(data.agent_num)] , &s_cmd, sizeof(s_cmd));	
	
	printf("[REQ_POLY] SEND : MASTER ------>  RECV : AGENT[%d]\n", data.agent_num);
	
	
	while(read(AGENT_SOCK[agent_cal(data.agent_num)], &r_cmd, sizeof(r_cmd)) != 0 ){
		
		//정책에 대한 응답 - 정상
		if(r_cmd.signal == RPL_POLY){
			
			// 에이전트에게서 응답이 오면 다시 웹으로 응답을 해준다.
			strcpy(signal, WEB_PRL_POLY);			
			write(data.web_sock , &signal, sizeof(signal));
			printf("[RPL_POLY] SEND : MASTER ->  RECV : WEB\n");			
			break;
		}
		
		//정책에 대한 응답 - 비정상
		else{
			printf("[RPL_POLY] SEND : MASTER ->  RECV : WEB [failed] \n");			
			break;
		}
	}
	close(data.web_sock);
	pthread_exit(0);
}


// 실시간 로그 보기 쓰레드
void *req_live_thread_func( void *info )
{
	struct thread_info data;
	COMMAND s_cmd, r_cmd;
	char signal[10];
	
	data.agent_num = ((struct thread_info *)info)->agent_num;	// 에이전트 번호 추출
	data.web_sock = ((struct thread_info *)info)->web_sock;		// 웹 소켓 추출
	
	
	// 해당 에이전트로 실시간 로그 보기 요청 시그널을 보냄
	s_cmd.signal = REQ_LIVE;
	s_cmd.data = 0;
	write(AGENT_SOCK[agent_cal(data.agent_num)] , &s_cmd, sizeof(s_cmd));
	
	printf("[REQ_LIVE] SEND : MASTER ------>  RECV : AGENT[%d]\n", data.agent_num);
	
	
	while(read(AGENT_SOCK[agent_cal(data.agent_num)], &r_cmd, sizeof(r_cmd)) != 0 ){
		
		//실시간에 대한 응답 - 정상
		if(r_cmd.signal == RPL_LIVE){
			
			// 에이전트에게서 실시간 로그보기 응답이 오면 다시 웹에게 응답한다.
			strcpy(signal, WEB_PRL_LIVE);
			write(data.web_sock , &signal, sizeof(signal));
			printf("[RPL_LIVE] SEND : MASTER ->  RECV : WEB\n");			
			break;		
		}
		
		//실시간에 대한 응답 - 비정상
		else{
			printf("[RPL_LIVE] SEND : MASTER ->  RECV : WEB [failed] \n");			
			break;
		}
	}
	close(data.web_sock);
	pthread_exit(0);
}


// 소속 네트워크 호스트 보기 쓰레드
void *req_net_thread_func( void *info )
{
	
	struct thread_info data;
	COMMAND s_cmd, r_cmd;
	char signal[10];
	
	data.agent_num = ((struct thread_info *)info)->agent_num;	// 에이전트 번호 추출
	data.web_sock = ((struct thread_info *)info)->web_sock;		// 웹 소켓 추출
	
	
	// 에이전트에세 소속 네트워크 호스트 보기 시그널을 보냄
	s_cmd.signal = REQ_NET;
	s_cmd.data = 0;	
	write(AGENT_SOCK[agent_cal(data.agent_num)] , &s_cmd, sizeof(s_cmd));
	
	printf("[REQ_NET] SEND : MASTER ------>  RECV : AGENT[%d]\n", data.agent_num);	
	
	while(read(AGENT_SOCK[agent_cal(data.agent_num)], &r_cmd, sizeof(r_cmd)) != 0 ){
		
		//네트워크에 붙은 호스트 질의에 대한 응답 - 정상
		if(r_cmd.signal == RPL_NET){
			
			// 에이전트에게서 응답이 오면 다시 웹에게 응답을 보냄
			strcpy(signal, WEB_PRL_NET);			
			write(data.web_sock , &signal, sizeof(signal));
			printf("[RPL_NET] SEND : MASTER ->  RECV : WEB\n");			
			break;
		}
		
		//네트워크에 붙은 호스트 질의에 대한 응답 - 비정상
		else{
			printf("[RPL_NET] SEND : MASTER ->  RECV : WEB [failed] \n");			
			break;
		}
	}
	close(data.web_sock);
	pthread_exit(0);
}


// 이전달 로그보기 쓰레드
void *req_rev_thread_func( void *data )
{
	struct thread_rev info;
	info = *(struct thread_rev *)data;	
	
	printf("[REQ_REVIEW] SEND : WEB ------>  RECV : MASTER ### AGNET REVIEW[%d]\n", info.agent_num);
	
	// 압축된 이전 로그 백업 파일을 DB에 저장
	if( review(info.agent_num, BACKUP_DIR, info.year, info.mon, HOST, USER, PASSWORD) != -1)
	{
		
		// 웹에게 응답 - 성공
		char req[] = "ok";
		printf("[RPL_REVIEW] SEND : MASTER ------>  RECV : WEB ### AGNET REVIEW[%d]\n", info.agent_num);
		write(info.web_sock , &req, sizeof(req));
	}
	else
	{
		// 웹에게 응답 - 실패
		char req[] = "fail";
		printf("[RPL_REVIEW] SEND : MASTER ------>  RECV : WEB ### FAIL AGNET REVIEW[%d]\n", info.agent_num);
		write(info.web_sock , &req, sizeof(req));
	}
	
	close(info.web_sock);
	pthread_exit(0);	
}


// 이전달 로그보기 시 소요되는 시간 요청 쓰레드
// - 이전달 로그보기는 많은 시간이 소요됨으로 사용자에게 시간을 알려줌으로
// - 로그 보기의 소요 시간을 알린다.
void *req_revtime_thread_func( void *data )
{
	struct thread_rev info;
	info = *(struct thread_rev *)data;
	int min;
	char ret[32];
	
	// 파일 사이즈를 통하여 예상 시간을 추측한다
	// 이는 일반PC의 벤치 마킹을 통해 얻은 값이므로 시스템에 따라 약간의 오차가 발생할 수 있음.
	if( (min = file_size(info.agent_num, info.year, info.mon)) != -1)
	{
		// 웹에게 응답 - 소요되는 시간을 전송
		strcpy(ret, itoa(min));
		printf("[RPL_REVIEWTIME] SEND : MASTER ------>  RECV : WEB ### AGNET REVIEWTIME[%d]\n", info.agent_num);
		write(info.web_sock , &ret, strlen(ret)+1);
	}
	else
	{
		// 웹에게 응답 - 실패
		strcpy(ret, "no");		
		printf("[RPL_REVIEWTIME] SEND : MASTER ------>  RECV : WEB ### FAIL AGNET REVIEWTIME[%d]\n", info.agent_num);
		write(info.web_sock , &ret, strlen(ret)+1);
	}
	
	write(info.web_sock , &ret, strlen(ret)+1);
	close(info.web_sock);
	pthread_exit(0);	
}



// 파일 사이즈를 통하여 "이전 백업 로그"를 디비에 저장하는 (추측된)시간 구하기
int file_size(int agent_num, u_short year, u_short mon)
{
	FILE *fp;
	char buf[32];
	int file_size;
	int ret;
	
	// 압축된 백업 로그 파일
	sprintf(buf, "%s/agent%d/agent%d-%d-%d.gz", BACKUP_DIR, agent_num, agent_num, year, mon);
	
	puts(buf);
	
	fp = fopen(buf , "r" );	//대상 파일을 읽기 모드로 엽니다.
	if(fp == NULL){
		return -1;
	}
	else{
  	fseek(fp , 0l , SEEK_END );	//파일 포인터를 파일의 마지막으로 이동시킵니다. 
  	file_size =  ftell(fp); 	//결과 해당 파일의 크기를 얻을 수 있습니다. 
  	fclose(fp);
  	  	
  	ret = ceil( file_size / 9786956 );	// 1분당 9786956 bytes를 처리 가능 - 시스템마다 성능이 다를 수 있음
  	printf("이전 로그 복구 작업이 %d분 예상 소요\n",ret);
  	return ret;
  }
  return 0;
}

// integer TO string
char* itoa(unsigned int val)
{
	static char buf[32] = {0};
	int i = 30;
	for( ; val && i ; --i, val /= 10)
		buf[i] = "0123456789"[val % 10];
	return &buf[i+1];
}


