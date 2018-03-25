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

#define LISTENSIZE 100						// �ִ� ���� listen 
#define BACKUP_DIR	"log_backup"	// ��� ���丮 ���




int backup_time_check();												// ��� �ð� üũ
void *timer_thread_func( void *data );					// �ǽð� Ÿ�̸� ������
void *db_backup_thread_func( void *data );			// DB ��� ������
void *req_policy_thread_func( void *data );			// ��å ���� ������
void *req_live_thread_func( void *data );				// �ǽð� �α� ���� ������
void *req_net_thread_func( void *data );				// �Ҽ� ��Ʈ��ũ ���� ������
void *req_rev_thread_func( void *data );				// �� ���� �α� ���� ������
void *req_revtime_thread_func( void *data );		// �� ���� �α� ���� �� �󸶳� �ð� ��Ұ� �ɸ����� �˾Ƴ��� ������
int agent_cal(int agent_num);										// ������Ʈ ��ȣ ���
char* itoa(unsigned int val);										// integer TO string
int file_size(int agent_num, u_short year, u_short mon);	// ������ ����� �˾Ƴ�

// Ŀ�ǵ� 
COMMAND recv_cmd,	// ����
			 send_cmd;	// �߽�

//������Ʈ ��ȣ
int agent_num;

int AGENT_SOCK[MAX_AGENT+6]; // ����� agent���� ���� ���� �迭


// ������ ��û �����ϴ� ��������� ���� ����
struct thread_info{
	int agent_num;
	int web_sock;	
};

// ���� �� �α� ���� ������ ���� ����
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
	
	pthread_t timer_thread;	// Ÿ�̹� üũ(1�� ����)	
	int status;	
	
	
	// LOG ��� ���丮 ����
	if(access(BACKUP_DIR,0) != 0){
		mkdir(BACKUP_DIR);		
	}
			
	//��� �ʱ�ȭ �۾� : �ణ�� �ð��ҿ䰡 ���� �� �ֽ��ϴ�.	
	if( db_initial(HOST, USER, PASSWORD) != 0){
		puts("[DBMS] DATABASE Initialize : failed");		
	}
	puts("[DBMS] DATABASE Initialize : success");
	
	
	
	// time ������ ����	
	if(( status = pthread_create( &timer_thread, NULL, &timer_thread_func, (void *)NULL)) != 0 ) {
		printf("timer Thread error : %s\n", strerror(status));
		exit(1);
	}
	
	//////////////////////////////////////////////////////////////////////////
	// command ��ƾ
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
		
		// ��� ���� ���� �ʱ�ȭ		
		memset(&recv_cmd, 0, sizeof(recv_cmd));
		memset(&send_cmd, 0, sizeof(send_cmd));
		
		puts("������������������������������������������");
		puts("��[COMMAND] WAITING... AGENT CONNECTION ��");
		puts("������������������������������������������");
		
		
		agent_sock = accept(master_sock, (struct sockaddr *)&agnet_addr, &agent_addr_size);
		if(agent_sock != -1){			
		
			if ( read(agent_sock, &recv_cmd, sizeof(recv_cmd)) != 0 ){
				
				switch(recv_cmd.signal){					
					
					
					case START_OK:						
						printf("[START] AGENT[%d] START!! \n",recv_cmd.data);
						break;
						
										
					////////////////////////////////////////////////////////////////////////////												
					//1:HELLO_WITH_ID - ������Ʈ�� ������ �������ٰ� �� �����ϴ� ���
					////////////////////////////////////////////////////////////////////////////							
					case HELLO_WITH_ID :						
						printf("[COMMAND] SEND : AGENT ------>  RECV : MASTER [HELLO_WITH_ID]\n");
						
						// DB���� ������Ʈ ��ȣ�� �����´�.
						if ((agent_num = get_agent_num(HOST, USER, PASSWORD)) != -1)
						{
							// ��ȿ�� ������Ʈ ID �� Ȯ��
							if(agent_num > recv_cmd.data || recv_cmd.data > MAX_AGENT)
							{ 
								//������Ʈ ��ȣ ����
								agent_num = recv_cmd.data;
												
								printf("[CONNECTION] Re_Hello ! AGENT[%d]\n",recv_cmd.data);
								
								// ������Ʈ���� �����϶�� ������ ����.
								send_cmd.signal = ACK_OK;
								send_cmd.data = 0;
	 							write(agent_sock, &send_cmd, sizeof(send_cmd));
							
								// DB�� agent ip �����ϱ�
								if (insert_ip(agent_num, inet_ntoa(agnet_addr.sin_addr), HOST, USER, PASSWORD) != 0){			
									puts("[STAUTS] Agent ip insert into database : failed");						
								}
							
								// ����� agent socket ���� //
								// ������Ʈ ��ȣ�� �̿��� ������Ʈ ���� ���� �迭�� �ε����� ����
								AGENT_SOCK[agent_cal(agent_num)] = agent_sock;
								printf("[SOCK_SAVE] AGENT_SOCK[%d] = sock%d\n",agent_cal(agent_num), agent_sock);
							
							}
							
							// �߸��� ������Ʈ ID ��
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
					//2:HELLO_NO_ID	- ������Ʈ�� ó�� �����ϴ� ��� - ������Ʈ ��ȣ�� �Ҵ�����
					////////////////////////////////////////////////////////////////////////////
					case HELLO_NO_ID   :
						printf("[COMMAND] SEND : AGENT ------>  RECV : MASTER [HELLO_NO_ID]\n");
						
						// DB���� ������Ʈ ��ȣ�� �����´�.
						if ((agent_num = get_agent_num(HOST , USER , PASSWORD)) != -1){						
						
							// ������Ʈ ��ȣ�� ���(information.status) ���
							if ( connect_agent(agent_num, HOST , USER , PASSWORD ) == 0){
								printf("[CONNECTION] Hello ! AGENT[%d]\n",agent_num);
								
								//	���� ����� ������Ʈ���� ��� ����
								if ( create_agent_db(agent_num, tz.year, tz.mon+1, HOST , USER , PASSWORD) != 0){
									puts("[DBMS] create_agent_db() error");								
								}
																
								else{
									// ������Ʈ ��ȣ�� �Ҵ� �Ͽ� ������Ʈ���� �˷���									
									printf("[DBMS] SUCCESS! created agent%d database ...\n",agent_num);							
									send_cmd.data = agent_num;
									send_cmd.signal = ACK_WITH_ID;					
									write(agent_sock, &send_cmd, sizeof(send_cmd));
								}
							}
						}
						
						// DB�� agent ip ���� �ϱ�
						if (insert_ip(agent_num, inet_ntoa(agnet_addr.sin_addr), HOST , USER , PASSWORD) != 0){
							puts("[STAUTS] Agent ip insert into database : failed");						
						}
					
						// agent socket ���� //
						// ������Ʈ ��ȣ�� �̿��� ������Ʈ ���� ���� �迭�� �ε����� ����
						AGENT_SOCK[agent_cal(agent_num)] = agent_sock;
						printf("[SOCK_SAVE] AGENT_SOCK[%d] = sock%d\n",agent_cal(agent_num), agent_sock);
						
						break;
					
					
					////////////////////////////////////////////////////////////////////////////
					// ������ MASTER���� ������ �ñ׳�
					////////////////////////////////////////////////////////////////////////////
					default		:
						
						////////////////////////////////////////////////////////////////////////////
						//3:REQ_LIVE - �ǽð� �α� ���� ��û
						////////////////////////////////////////////////////////////////////////////
						
						// ���� ĳ���� COMMAND���� CHAR�� ����
						if(strstr((char *)&recv_cmd, WEB_REQ_LIVE)){
							
							pthread_t req_live_thread;	// �ǽð� ���� ��û ������
							struct thread_info info;		// ������ ���� ����
							
							info.web_sock = agent_sock;	//web socket ����
														
							info.agent_num = atoi((char *)&recv_cmd + 4);	//������Ʈ ��ȣ ����
							printf("What do you want agnet[%d] LIVE\n",info.agent_num);
							
							if(AGENT_SOCK[agent_cal(info.agent_num)] != 0){
							
								// �����Ͱ� ������Ʈ���� ����� ���޽� read �� �ܿ�  COMMAND ��ƾ�� block �Ǵ°� �����ϱ� ���� ������ ���
								if(( status = pthread_create( &req_live_thread, NULL, &req_live_thread_func, (void *)&info)) != 0 ) {
									printf("req_live_Thread error : %s\n", strerror(status));									
								}
							}							
							else{
								printf("[REQ_LIVE] ERROR\n");
							}
														
						}


						////////////////////////////////////////////////////////////////////////////						
						//4:REQ_POLY - ��å�� �����϶�� �ñ׳�
						////////////////////////////////////////////////////////////////////////////						
						
						// ���� ĳ���� COMMAND���� CHAR�� ����
						else if(strstr((char *)&recv_cmd, WEB_REQ_POLY)){
							pthread_t req_policy_thread;	// ��å ���� ������		
							struct thread_info info;			// ������ ���� ����
							
							info.web_sock = agent_sock;	//web socket ����
							
							info.agent_num = atoi((char *)&recv_cmd + 4);	//������Ʈ ��ȣ ����
							printf("What do you want agnet[%d] POLICY APPLY\n",info.agent_num);
							
							if(AGENT_SOCK[agent_cal(info.agent_num)] != 0){
								
								// �����Ͱ� ������Ʈ���� ����� ���޽� read �� �ܿ�  COMMAND ��ƾ�� block �Ǵ°� �����ϱ� ���� ������ ���
								if(( status = pthread_create( &req_policy_thread, NULL, &req_policy_thread_func, (void *)&info)) != 0 ) {
									printf("req_policy_Thread error : %s\n", strerror(status));									
								}
							}							
							else{
								printf("[REQ_POLY] ERROR\n");
							}
						}
						

						////////////////////////////////////////////////////////////////////////////
						// REQ_NET_ZONE - �Ҽ� ��Ʈ��ũ ȣ��Ʈ ���� ��û
						////////////////////////////////////////////////////////////////////////////
						else if(strstr((char *)&recv_cmd, WEB_REQ_NET)){
							pthread_t req_net_thread; // �Ҽ� ��Ʈ��ũ ȣ��Ʈ ���� ������
							struct thread_info info;	// ������ ���� ����
							
							info.web_sock = agent_sock;	//web socket ����
							
							info.agent_num = atoi((char *)&recv_cmd + 3);	//������Ʈ ��ȣ ����
							printf("%s\n",(char *)&recv_cmd);
							printf("What do you want agnet[%d] NET ZONE HOST\n",info.agent_num);
							
							if(AGENT_SOCK[agent_cal(info.agent_num)] != 0){
								
								// �����Ͱ� ������Ʈ���� ����� ���޽� read �� �ܿ�  COMMAND ��ƾ�� block �Ǵ°� �����ϱ� ���� ������ ���
								if(( status = pthread_create( &req_net_thread, NULL, &req_net_thread_func, (void *)&info)) != 0 ) {
									printf("req_net_zone_Thread error : %s\n", strerror(status));									
								}
							}							
							else{
								printf("[REQ_NET] ERROR\n");
							}
						}
						

						////////////////////////////////////////////////////////////////////////////
						// REQ_REV - �� ���� ����α� ���� ��û
						////////////////////////////////////////////////////////////////////////////
						else if(strstr((char *)&recv_cmd, WEB_REQ_REV)){
							pthread_t req_rev_thread;	// ������ �α� ���� ��û ������							
							struct thread_rev info;		// ������ ���� ����							
							char *p;
							
							puts((char *)&recv_cmd);
							
							// token ó��
							// ������Ʈ ��ȣ ���
							p = strtok((char *)&recv_cmd, "-");
							info.agent_num = atoi(p+3);	//������Ʈ ��ȣ ����
							
							// �⵵ ���
							p = strtok(NULL,"-");
							info.year = atoi(p);
							
							// �� ���
							p = strtok(NULL,"-");
							info.mon = atoi(p);
							
							// �� ���� ���
							info.web_sock = agent_sock;
							
							printf("What do you want agnet[%d] REQ_REVIVEW HOST\n",info.agent_num);
							
							// ������ �α� ���� ������ ����
							if(( status = pthread_create( &req_rev_thread, NULL, &req_rev_thread_func, (void *)&info)) != 0 ) {
									printf("req_rev_Thread error : %s\n", strerror(status));									
							}							
						}
						
						
						////////////////////////////////////////////////////////////////////////////
						// REQ_REVIEW_TIME - ������ ����α׸� �������µ� �ɸ��� �ð� ��û
						//								 - ������ ����α� ����� ����� �ð��� �ɸ���.
						//								 - ������ �α׸� �ӽ� ���̺� ���� �ϴ� �۾��̹Ƿ�...
						////////////////////////////////////////////////////////////////////////////
						else if(strstr((char *)&recv_cmd, WEB_REQ_REVTIME)){
							pthread_t req_revtime_thread;
							
							struct thread_rev info;	// ������ ���� ����							
							char *p;
							
							puts((char *)&recv_cmd);
							
							// token ó��
							// ������Ʈ ��ȣ ���
							p = strtok((char *)&recv_cmd, "-");
							info.agent_num = atoi(p+4);	//������Ʈ ��ȣ ����
							
							// �⵵ ���
							p = strtok(NULL,"-");
							info.year = atoi(p);
							
							// �� ���
							p = strtok(NULL,"-");
							info.mon = atoi(p);
							
							// �� ���� ���
							info.web_sock = agent_sock;
							
							// "������ ����α׸� �������� �ɸ��� �ð�" ��û ������ ����
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


// �ǽð� �ð� ���� ������
void *timer_thread_func( void *data )
{
	
	pthread_t db_backup_thread;	// DB ��� ������
	int status;	
	
	puts("[TIMER] START");	
	
	while(1){
		
		// ���� ���� �ð� ����
		sleep(1);
		
		// �� �ð� ���� �ð��� ����ü�� ����Ѵ� -  Ÿ�̹� �����ٷ��� �� �ʿ�
		get_localtime();
		
		
		// DB ��� �ð� ü��
		if(backup_time_check()){
			
			// DB ��� ������ üũ
			if(( status = pthread_create( &db_backup_thread, NULL, &db_backup_thread_func, NULL)) != 0 ) {
				printf("Thread error : %s\n", strerror(status));				
			}			
		}
		
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// �� 5�� ���� ������Ʈ �ý��� ���¸� �˻��Ѵ�.
		//
		// information ���̺� ����Ʈ�� save_time �ʵ�� '1', back_up �ʵ�� '-1'�� �ʱ�ȭ ��
		//	[������Ʈ] ������Ʈ	--> DB : �� 1�� ���� save_time �ʵ�� '-1' �Ǵ� '1' ���� ������ ���� ����Ѵ�.
		// 	[������]		������  --> DB : �� 5�� ���� save_time, backup_time�� ���Ͽ� ���� ���̸� ������Ʈ�� �ٿ�Ȱ���(alive = 0)
		//														 : save_time, backup_time ���� ���� ������ (������Ʈ�� ����, alive = 1) �� 5�� ���� back_up �ʵ忡 save_time �ʵ带 �����Ѵ�.
		//														 : ��, save_time, backup_time�� '1' �Ǵ� '-1' ������ ������Ʈ ���¸� üũ
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if(tz.sec==5)
		{
			check_alive(HOST, USER, PASSWORD);
		}			
	}	
}


// DB ��� ������ - �����ϴ� ������Ʈ �����ͺ��̽��� �α׿� Ʈ���� �����ͺ��̽� ���(�Ѵ޿� �ѹ�)
void *db_backup_thread_func( void *data )
{
	db_backup(HOST, USER, PASSWORD, tz.pre_year, tz.pre_mon+1, tz.year, tz.mon+1, BACKUP_DIR);	
}


// DB ��� �ð� �˻�
int backup_time_check()
{	
	////////////////////////////////////
	//�Ŵ� 1�� 0�� 40�� 10�ʿ� ��� ����
	////////////////////////////////////
	if( tz.mday == 1 && tz.hour == 0 && tz.min == backup_min && tz.sec == backup_sec){
		puts("[BACKUP] ALAM!!");
		return 1;
	}	
	return 0;
}




// ������Ʈ ��ȣ ��� - 0:ǥ�� �Է�, 1: ǥ�� ���, 2: ����, 3: MASTER SOCKET
// ������Ʈ ������ 4�� ���ĺ��� ���۵�
// ������Ʈ ��ȣ + 4 ���� ������Ʈ ������ �����ϴ� �迭�� �ε��� ������
// Ư���� ������Ʈ ���� ������ ������Ʈ ���� ���� �迭���� �׻� ���� ��ġ�� �����Ѵ�.
int agent_cal(int agent_num)
{
	return agent_num + 4;	
}



// ��å ���� ������
void *req_policy_thread_func( void *info )
{
	struct thread_info data;
	COMMAND s_cmd, r_cmd;
	char signal[10];
	
	data.agent_num = ((struct thread_info *)info)->agent_num;
	data.web_sock = ((struct thread_info *)info)->web_sock;
	
	
	// ������Ʈ���� ��å�� �ٽ� ���� �϶�� �ñ׳� ����
	s_cmd.signal = REQ_POLY;
	s_cmd.data = 0;
	write(AGENT_SOCK[agent_cal(data.agent_num)] , &s_cmd, sizeof(s_cmd));	
	
	printf("[REQ_POLY] SEND : MASTER ------>  RECV : AGENT[%d]\n", data.agent_num);
	
	
	while(read(AGENT_SOCK[agent_cal(data.agent_num)], &r_cmd, sizeof(r_cmd)) != 0 ){
		
		//��å�� ���� ���� - ����
		if(r_cmd.signal == RPL_POLY){
			
			// ������Ʈ���Լ� ������ ���� �ٽ� ������ ������ ���ش�.
			strcpy(signal, WEB_PRL_POLY);			
			write(data.web_sock , &signal, sizeof(signal));
			printf("[RPL_POLY] SEND : MASTER ->  RECV : WEB\n");			
			break;
		}
		
		//��å�� ���� ���� - ������
		else{
			printf("[RPL_POLY] SEND : MASTER ->  RECV : WEB [failed] \n");			
			break;
		}
	}
	close(data.web_sock);
	pthread_exit(0);
}


// �ǽð� �α� ���� ������
void *req_live_thread_func( void *info )
{
	struct thread_info data;
	COMMAND s_cmd, r_cmd;
	char signal[10];
	
	data.agent_num = ((struct thread_info *)info)->agent_num;	// ������Ʈ ��ȣ ����
	data.web_sock = ((struct thread_info *)info)->web_sock;		// �� ���� ����
	
	
	// �ش� ������Ʈ�� �ǽð� �α� ���� ��û �ñ׳��� ����
	s_cmd.signal = REQ_LIVE;
	s_cmd.data = 0;
	write(AGENT_SOCK[agent_cal(data.agent_num)] , &s_cmd, sizeof(s_cmd));
	
	printf("[REQ_LIVE] SEND : MASTER ------>  RECV : AGENT[%d]\n", data.agent_num);
	
	
	while(read(AGENT_SOCK[agent_cal(data.agent_num)], &r_cmd, sizeof(r_cmd)) != 0 ){
		
		//�ǽð��� ���� ���� - ����
		if(r_cmd.signal == RPL_LIVE){
			
			// ������Ʈ���Լ� �ǽð� �α׺��� ������ ���� �ٽ� ������ �����Ѵ�.
			strcpy(signal, WEB_PRL_LIVE);
			write(data.web_sock , &signal, sizeof(signal));
			printf("[RPL_LIVE] SEND : MASTER ->  RECV : WEB\n");			
			break;		
		}
		
		//�ǽð��� ���� ���� - ������
		else{
			printf("[RPL_LIVE] SEND : MASTER ->  RECV : WEB [failed] \n");			
			break;
		}
	}
	close(data.web_sock);
	pthread_exit(0);
}


// �Ҽ� ��Ʈ��ũ ȣ��Ʈ ���� ������
void *req_net_thread_func( void *info )
{
	
	struct thread_info data;
	COMMAND s_cmd, r_cmd;
	char signal[10];
	
	data.agent_num = ((struct thread_info *)info)->agent_num;	// ������Ʈ ��ȣ ����
	data.web_sock = ((struct thread_info *)info)->web_sock;		// �� ���� ����
	
	
	// ������Ʈ���� �Ҽ� ��Ʈ��ũ ȣ��Ʈ ���� �ñ׳��� ����
	s_cmd.signal = REQ_NET;
	s_cmd.data = 0;	
	write(AGENT_SOCK[agent_cal(data.agent_num)] , &s_cmd, sizeof(s_cmd));
	
	printf("[REQ_NET] SEND : MASTER ------>  RECV : AGENT[%d]\n", data.agent_num);	
	
	while(read(AGENT_SOCK[agent_cal(data.agent_num)], &r_cmd, sizeof(r_cmd)) != 0 ){
		
		//��Ʈ��ũ�� ���� ȣ��Ʈ ���ǿ� ���� ���� - ����
		if(r_cmd.signal == RPL_NET){
			
			// ������Ʈ���Լ� ������ ���� �ٽ� ������ ������ ����
			strcpy(signal, WEB_PRL_NET);			
			write(data.web_sock , &signal, sizeof(signal));
			printf("[RPL_NET] SEND : MASTER ->  RECV : WEB\n");			
			break;
		}
		
		//��Ʈ��ũ�� ���� ȣ��Ʈ ���ǿ� ���� ���� - ������
		else{
			printf("[RPL_NET] SEND : MASTER ->  RECV : WEB [failed] \n");			
			break;
		}
	}
	close(data.web_sock);
	pthread_exit(0);
}


// ������ �α׺��� ������
void *req_rev_thread_func( void *data )
{
	struct thread_rev info;
	info = *(struct thread_rev *)data;	
	
	printf("[REQ_REVIEW] SEND : WEB ------>  RECV : MASTER ### AGNET REVIEW[%d]\n", info.agent_num);
	
	// ����� ���� �α� ��� ������ DB�� ����
	if( review(info.agent_num, BACKUP_DIR, info.year, info.mon, HOST, USER, PASSWORD) != -1)
	{
		
		// ������ ���� - ����
		char req[] = "ok";
		printf("[RPL_REVIEW] SEND : MASTER ------>  RECV : WEB ### AGNET REVIEW[%d]\n", info.agent_num);
		write(info.web_sock , &req, sizeof(req));
	}
	else
	{
		// ������ ���� - ����
		char req[] = "fail";
		printf("[RPL_REVIEW] SEND : MASTER ------>  RECV : WEB ### FAIL AGNET REVIEW[%d]\n", info.agent_num);
		write(info.web_sock , &req, sizeof(req));
	}
	
	close(info.web_sock);
	pthread_exit(0);	
}


// ������ �α׺��� �� �ҿ�Ǵ� �ð� ��û ������
// - ������ �α׺���� ���� �ð��� �ҿ������ ����ڿ��� �ð��� �˷�������
// - �α� ������ �ҿ� �ð��� �˸���.
void *req_revtime_thread_func( void *data )
{
	struct thread_rev info;
	info = *(struct thread_rev *)data;
	int min;
	char ret[32];
	
	// ���� ����� ���Ͽ� ���� �ð��� �����Ѵ�
	// �̴� �Ϲ�PC�� ��ġ ��ŷ�� ���� ���� ���̹Ƿ� �ý��ۿ� ���� �ణ�� ������ �߻��� �� ����.
	if( (min = file_size(info.agent_num, info.year, info.mon)) != -1)
	{
		// ������ ���� - �ҿ�Ǵ� �ð��� ����
		strcpy(ret, itoa(min));
		printf("[RPL_REVIEWTIME] SEND : MASTER ------>  RECV : WEB ### AGNET REVIEWTIME[%d]\n", info.agent_num);
		write(info.web_sock , &ret, strlen(ret)+1);
	}
	else
	{
		// ������ ���� - ����
		strcpy(ret, "no");		
		printf("[RPL_REVIEWTIME] SEND : MASTER ------>  RECV : WEB ### FAIL AGNET REVIEWTIME[%d]\n", info.agent_num);
		write(info.web_sock , &ret, strlen(ret)+1);
	}
	
	write(info.web_sock , &ret, strlen(ret)+1);
	close(info.web_sock);
	pthread_exit(0);	
}



// ���� ����� ���Ͽ� "���� ��� �α�"�� ��� �����ϴ� (������)�ð� ���ϱ�
int file_size(int agent_num, u_short year, u_short mon)
{
	FILE *fp;
	char buf[32];
	int file_size;
	int ret;
	
	// ����� ��� �α� ����
	sprintf(buf, "%s/agent%d/agent%d-%d-%d.gz", BACKUP_DIR, agent_num, agent_num, year, mon);
	
	puts(buf);
	
	fp = fopen(buf , "r" );	//��� ������ �б� ���� ���ϴ�.
	if(fp == NULL){
		return -1;
	}
	else{
  	fseek(fp , 0l , SEEK_END );	//���� �����͸� ������ ���������� �̵���ŵ�ϴ�. 
  	file_size =  ftell(fp); 	//��� �ش� ������ ũ�⸦ ���� �� �ֽ��ϴ�. 
  	fclose(fp);
  	  	
  	ret = ceil( file_size / 9786956 );	// 1�д� 9786956 bytes�� ó�� ���� - �ý��۸��� ������ �ٸ� �� ����
  	printf("���� �α� ���� �۾��� %d�� ���� �ҿ�\n",ret);
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


