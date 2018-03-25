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
* include ���� ���� �ٰ�
*/
#include "../include/port.h"
#include "../include/ifconfig.h"
#include "../include/common.h"	//�Ϲ� ���� �ص�
#include "../include/db.h"
#include "../include/command.h"
#include "../include/config.h"
#include "../include/time.h"
#include "../include/system.h"
#include "../include/pcap.h"
#include "../include/facke_page.h"
#include "../include/net_zone.h"





extern struct system_monitor System;	// ������Ʈ �ý��� ����
void *pcap_thread_func( void *data );	// ��Ŷ ĸ�� ������
void *alive_thread_func( void *data );	// �ǽð����� ������Ʈ �ý��� ���� ���� ������
void *timer_thread_func( void *data );	// �ǽð� Ÿ�̸� ������
void *log_thread_func( void *data );		// LOG ���� ������
void sort_func();												// LOG �ߺ��� ���� �ϱ� ���� sort �Լ�
void create_new_file();									// hour �� ����ɶ� ���� LOG ������ ��ü �Ǿ�� �Ѵ�.
void *live_log_thread_func( void *sock );	// �ǽð� �α� ���� ��û ������



int main(int argc, char **argv){
		
	int connect_cnt=0;		
	struct sockaddr_in master_addr;
	
	pthread_t pcap_thread;	// pcap ������
	pthread_t timer_thread;	// timer ������
	pthread_t live_log_thread;		// �ǽð� �α� ��û ������
	pthread_t arp_packet_thread;	// ��Ʈ��ũ ȣ��Ʈ ���� ��û ������
		
	int status;	
	int delaytime=0;
	int refresh = 0;	// MASTER�� ������ ������������ �����ϴ� ���� - ������ �� �̻��� �Ǹ� MASTER�� ������ ������,
	
	// ���� �ð� ����
	get_localtime();
	
	// pipe ����(���� ���μ����� agent ���α׷����� ����� ���� ������)
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
	
	// ��� �������� ������ STATUS�� ���� ǥ�� �ȴ�.
	STATUS = STOP;	// ��� ������ STOP
	FAKE = F_STOP;	// ���� PROCESS STOP
	
	//(���� ���μ����� agent ���� ����� �ѹ��� ȣ��Ǿ�� ��-MASTER ���α׷��� ������ �������� AGENT�� �ٽ� ���� �ϴ� ��찡 �߻��ϹǷ�)
	
	
	// ���� ���� �ҷ�����
	//answer[0]-ID, answer[1]- DEV, answer[2]- master ip, answer[3]- password, answer[4]-URL_PATH
	get_conf();
	
	// ifconfig(��Ʈ��ũ ��ġ ���� ��������) �� ip, mac, broadcast, netmask, mtu ��������
	get_ifconfig(&ifcg, answer[1]);
	strcpy(ifcg.eth, answer[1]);
	
	// ���
	printf("����������< ifconfig >������������\n");
	printf("��DEV : \t%s\n",ifcg.eth);
	printf("��ETHER : \t%s\n",ifcg.mac);
	printf("��IP : \t\t%s\n",ifcg.ip);
	printf("��BROADCAST : \t%s\n",ifcg.broadcast);
	printf("��NETMASK : \t%s\n",ifcg.netmask);
	printf("��MTU : \t%d\n",ifcg.mtu);
	  puts("����������������������������������");	
	
	// LOG ���丮 ����(log ������ ��� ���丮)
	if(access(log_dir,0) != 0){
		mkdir(log_dir);
	}	
	
	// ������Ʈ ��ȣ Ȯ��(������Ʈ�� ������ �ѹ� ����� ���� �ִٸ� ������Ʈ ��ȣ�� ���� ���̴�)
	// moduler = 31
	if(strlen(answer[0]) != 0){
		agent_num = atoi(answer[0]);
		trans_time = agent_num % moduler;	// �ش� ������Ʈ�� LOG ������ ������ �� �ð�
		  
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
	
RESTART:	// master�� �ٿ�� ������Ʈ �ٽ� ���� �õ�
	
	command_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(command_sock == -1){
		puts("AGENT SOCK ERROR");
		exit(-1);
	}
	
	// sock �ʱ�ȭ
	memset(&master_addr, 0, sizeof(master_addr));
	master_addr.sin_family = AF_INET;
	master_addr.sin_addr.s_addr = inet_addr(answer[2]);
	master_addr.sin_port = htons(MASTER_PORT);	
	
	
	printf("����������< Master connection >������������\n");
	printf("�� MASTER IP     %s          ��\n",answer[2]);
	printf("�� MASTER PORT   %d                    ��\n",MASTER_PORT);
	  puts("������������������������������������������");
	
	// �����Ϳ� ������ ��ٸ�
	while( connect(command_sock, (struct sockaddr*) &master_addr, sizeof(master_addr)) == -1 ){
		sleep(5);
		printf("[CONNECT] master connection attemptedcount\n");
	}
	
	// ���� ������ �� ���� ������ �ʱ�ȭ
	memset(&send_cmd, 0, sizeof(send_cmd));
	memset(&recv_cmd, 0, sizeof(recv_cmd));
	
	// ���� Ÿ�� 
	// 1. ID �ִ� ���	// 2. ID ���� ���
	if(agent_num != 0){	// ID ���� �ִٸ� ID�� ��û �Ѵ�.
		send_cmd.signal = HELLO_WITH_ID;
		send_cmd.data = agent_num;
		puts("[COMMAND] SEND : AGENT ----> RECV : MASTER [HELLO_WITH_ID]");
	}
	else{	// ID ���� �����Ͽ� ID�� ��û���� �ʴ´�.
		send_cmd.signal = HELLO_NO_ID;
		puts("[COMMAND] SEND : AGENT ----> RECV : MASTER [HELLO_NO_ID]");
	}
	
	// SEND
	write(command_sock, &send_cmd, sizeof(send_cmd));
	
	while(1){
		
		// MASTER �� ������ ���� ���� ��� ��� �����带 ���� �Ѵ�.
		if(tz.sec == delaytime){		
			refresh++;
			if(refresh >= REFRESH_TIME){	// ���ӵ� refresh�� �߻��Ͽ� 12 �̻��� �Ǹ� MASTER�� ������ �������ٰ� �Ǵ�
				printf("[RECONNECTION] attemp connect:%d\n",refresh);
				refresh = 0;
				STATUS = STOP;	// ��� ������ ���� ALL STOP
				
				close(command_sock);	// MASTER�� ������ ������ ��� ������ �����Ѵ�.
				goto RESTART;					// �ٽ� ���� ���� �õ�
			}
		}
		
		// ��� ����ü �ʱ�ȭ
		memset(&send_cmd, 0, sizeof(send_cmd));
		memset(&recv_cmd, 0, sizeof(recv_cmd));
			 	
		puts("������������������������������������������");
		puts("��[COMMAND] Waiting for COMMAND...      ��");
		puts("������������������������������������������");
		
		
		// MASTER�� ������ �̷���� ����� ���� ����
		if ( read(command_sock, &recv_cmd, sizeof(recv_cmd)) != 0 ){
			
			// ��ɾ� ���� �Ǵ�
			switch(recv_cmd.signal){

				////////////////////////////////////////////////////
				// ACK_WITH_ID - ������Ʈ ��ȣ �Ҵ� ����
				////////////////////////////////////////////////////			
				case ACK_WITH_ID:
					
					// ������Ʈ ��ȣ�� ��� ������Ʈ ��ȣ�� ��û�ϴ�   ���� ��û ��ȣ�� ����				
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [ACK_WITH_ID]");
					
					// MAX_AGENT error : �ִ� 30�� ���� ���� ����
					if(recv_cmd.data > MAX_AGENT){
						printf("[MAX_AGENT] MAX_AGENT is %d\n",MAX_AGENT);
						exit(1);								
					}
					
					// ID ��ȣ�� �ο� ������ ���� ���Ͽ� ID�� �����Ѵ� - ���� ������ �ٽ� ����
					FILE *fp = fopen(CONF_FILE, "w");		
					if(fp == NULL){
						puts("agernt.conf file can not open()");
						exit(1);
					}
		
					// ���� ���� �ٽ� ���� - �Ҵ� ���� ��ȣ�� �߰��ϱ� ���ؼ�..//
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
		
					// ���� �ٽ� ���� �ҷ�����
					get_conf();
		
					// agent ID �� �ο�
					agent_num = atoi(answer[0]);
					
					// LOG ���� �ð� ���
					trans_time = agent_num % moduler;
									  	
					printf("[ALRAM] I am AGENT[%d] \t", agent_num);
					printf("I will send LOG file %dmin\n", trans_time);
					
					// ������ ���� - ����
					STATUS = START;
	
					// pcap ������ ����
					if(( status = pthread_create( &pcap_thread, NULL, &pcap_thread_func, (void*)NULL)) != 0 ) {
						printf("pcap_thread error : %s\n", strerror(status));
					}
						
					// timer ������ ����
					if(( status = pthread_create( &timer_thread, NULL, &timer_thread_func, (void*)NULL)) != 0 ) {
						printf("timer_thread error : %s\n", strerror(status));
					}
					
					// �̷��� ��Ʈ�� IP�� DB�� ����
					if( send_mirror_ip(answer[2], "root", answer[3]) !=0 )
						puts("SEND Mirror ip");
					
					break;
				
					
				////////////////////////////////////////////////////
				// ACK_OK - ������Ʈ�� �����ص� ���ٴ� �ñ׳�
				////////////////////////////////////////////////////		
				case ACK_OK :
					
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [ACK_OK]");
					
					// ������ ���� - ���� 				
					STATUS = START;

					// pcap ������ ����
					if(( status = pthread_create( &pcap_thread, NULL, &pcap_thread_func, (void*)NULL)) != 0 ) {
						printf("pcap_thread error : %s\n", strerror(status));
					}
						
					// timer ������ ����
					if(( status = pthread_create( &timer_thread, NULL, &timer_thread_func, (void*)NULL)) != 0 ) {
						printf("timer_thread error : %s\n", strerror(status));					
					}
					
					// �̷��� ��Ʈ�� IP�� DB�� ����
					if( send_mirror_ip(answer[2], "root", answer[3]) !=0 )
						puts("SEND Mirror ip");
				
					break;
				
					
				////////////////////////////////////////////////////
				// ACK_INVALID - �߸��� ���� / ������Ʈ ���α׷� ���� ��Ų��.
				////////////////////////////////////////////////////
				case ACK_INVALID:
				
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [ACK_INVALID]");
					puts("[ERROR] ./conf/config file syntax error or program error");					
					close(command_sock);
					
					// ���α׷� ����	
					exit(1);
				
					
				//////////////////////////////////////////////////////////////
				// REQPOLY - ��å�� ����Ǿ� �ٽ� �ٿ�ε� �϶�� �ñ׳�
				/////////////////////////////////////////////////////////////								
				case REQ_POLY:
					
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [REQ_POLY]");
					
					ipc_buf = PLY_CHN;	// ��å�� ������� ���� ���μ������� �˸�					
					puts("[PIPE] SEND : LOG -> RECV : FAKE [PLY_CHN]");
					
					write(pipeline[1], &ipc_buf, sizeof(ipc_buf));
					
					if( read(pipeline2[0], &ipc_buf, sizeof(ipc_buf)) != -1)
					{						
						if(ipc_buf == PLY_APP)	// ��å ������ �Ϸ� �Ǿ���.
						{
							puts("[PIPE] SEND : FAKE -> RECV : LOG [PLY_APP]");							
							
							send_cmd.signal = RPL_POLY;	// ��å ���� ����
							send_cmd.data = 0;
							
							write(command_sock, &send_cmd, sizeof(send_cmd));	// ������ ��å�� �Ϸ�Ǿ����� �˸�
							puts("[COMMAND] SEND : AGENT -> RECV : MASTER [PLY_POLY]");							
						}
						else
						{
							puts("[PIPE] Not found Define");
						}
					}
									
					break;


				///////////////////////////////////////////////////
				// REQLIVE - �ǽð� LOG ��û
				///////////////////////////////////////////////////
				case REQ_LIVE:
				
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [REQ_LIVE]");
					
					// �ǽð� �α� ������ ����
					if(( status = pthread_create( &live_log_thread, NULL, &live_log_thread_func, (void*)NULL)) != 0 ) {
						printf("live_log_thread error : %s\n", strerror(status));					
					}
				
					break;


				///////////////////////////////////////////////////
				// REQ_NET_ZONE - �Ҽ� ��Ʈ��ũ ȣ��Ʈ ����
				///////////////////////////////////////////////////
				case REQ_NET:
					
					puts("[COMMAND] SEND : MASTER -> RECV : AGENT [REQ_NET]");
					
					// ping_packet�� ������ ������ ����
					if(( status = pthread_create( &arp_packet_thread, NULL, &ping_packet_thread_func, (void*)NULL)) != 0 ) {
						printf("arp_packet_thread error : %s\n", strerror(status));
					}
					
					pthread_join(arp_packet_thread, NULL);
					
					send_cmd.signal = RPL_NET;	// �Ҽ� ��Ʈ��ũ ȣ��Ʈ ���� ����
					send_cmd.data = 0;
							
					write(command_sock, &send_cmd, sizeof(send_cmd));	// ������ �Ҽ� ��Ʈ��ũ ȣ��Ʈ ���� ����
					puts("[COMMAND] SEND : AGENT -> RECV : MASTER [PLY_NET]");
					break;
				
				default:
					break;
			}
		}
		
		// �����Ϳ� ������ �������� ���
		else{			
			delaytime = tz.sec;
		}
		
		
		////////////////////////////////////////////////////////////////////////
		// FORK - ���� ���μ���
		////////////////////////////////////////////////////////////////////////
		if( FAKE == F_STOP )	// ���� ���μ����� �ѹ��� ���� ��Ų��
		{			
			FAKE = F_START;
			
			pid = fork();
						
			if(pid > 0){
				ppid = pid;				
			}
			
			// ���� ���μ��� ����
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


// �ǽð� �α� ���� ��û ������
void *live_log_thread_func( void *sock )
{
	// pcap ����
	live_pcap_capture(answer[1], pcap_filter, agent_num, answer[2], USER, answer[3], command_sock);	
}


// pcap ������
void *pcap_thread_func( void *data )
{	
	printf("[PCAP START]\n");	
	// pcap ����
	pcap_capture(answer[1], pcap_filter);	
}



// ������Ʈ �ý��� ���¸� �ֱ������� ������
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
	
	// alive_value�� �ֱ������� 1, -1, 1, -1 ...�� �����鼭 ������Ʈ�� ����ִٴ� ��ȣ�� ��� ���
	alive_value *= -1;
	
	// �ý��� ������ ������	
	system_moniter(alive_interval);		
			
	// @ query send system_moniter
	sprintf(query, "UPDATE %s SET save_time=%d, cpu=%d, mem=%d, disk1_name='%s', disk1='%s', disk2_name='%s', disk2='%s' where id=%d",		AGENT_INFORMATION_STATUS, alive_value, System.cpu_stat, System.mem_stat, System.disk_mount[0], System.disk_used[0],System.disk_mount[1], System.disk_used[1], agent_num);

	query_stat = mysql_real_query(alive_connection, query, strlen(query));			
	query_invalied(alive_conn, query_stat);			
	mysql_close(alive_connection);
	return;	
}


// timer ������
void *timer_thread_func( void *data )
{
	
	pthread_t log_thread;
	int status;	
	int start = 1;			// ������ �� ������ �ϳ��� ����	
	FILE *traffic_fd;		// Ʈ���� ���� ��ũ����
	char buf[64];
	
	printf("[TIMER START]\n");	
	
	while(STATUS == START){
		
		sleep(1);	// 1�� ���� �ð� üũ
		get_localtime();
		
		
		// LOG ������ ������. ������ �� �п� log_send_sec(10) �ʰ� �Ǹ�....
		// log_send_sec �� 10�ʷ� ���� Ư���� ������ ����.
		if(trans_time == tz.min && tz.sec == log_send_sec)
		{
			// �α� ���� ������ ����
			if(( status = pthread_create( &log_thread, NULL, &log_thread_func, NULL)) != 0 ) {
				printf("log_Thread error : %s\n", strerror(status));				
			}
		}
		
		// �� �� 1��		
		if( tz.sec == 1 )
		{
			// ������Ʈ �ý��� ���� ����
			alive_func(answer[2], USER, answer[3]);
		}
		
		
		// ����  - ���ο� �α� ���Ϸ� ��ü , Ʈ���� ���� �ʱ�ȭ
		if(tz.sec == 0 && tz.min == 0 && start == 1)
		{		
			// ���ο� ���� ����
			create_new_file();
			
			// Ʈ���� �ʱ�ȭ
			if( (traffic_fd = fopen(TRAFFIC_FILE, "w")) != NULL )		
			{
				// ���Ͽ��� Ʈ���� ��������
				sprintf(buf, "%d-%d-%d\t%d\t%d\t%d\t%d\t%d\t%d", tz.pre_year, tz.pre_mon+1, tz.pre_mday,   tz.pre_hour, traffic.TByte, traffic.GByte, traffic.MByte, traffic.KByte, traffic.Byte);			
				fputs(buf, traffic_fd);
				
				// Ʈ���� ���� �ʱ�ȭ
				memset(&traffic, 0, sizeof(traffic));
				fclose(traffic_fd);			
				printf("[SAVE] %s\n",buf);				
			}
			else
			{
				puts("[TRAFFIC]TRAFFIC_FILE cant not create file");
			}
						
			start = 0;	// �α� ���� ��ü ��ƾ�� �ѹ��� ���� �ǰ� �ϱ� ���ؼ�...
		}
	
		if(tz.sec == 1 && tz.min == 0){		
			start = 1;
		}
	}
	puts("[EXIT] timer_thread_func");
	pthread_exit(0);	
}


// LOG ���� ������
void *log_thread_func( void *data )
{
	char query[200];
	int query_stat;
	MYSQL       *log_connection=NULL, 
							log_conn;
	
	puts("[ LOG START ]");	
	
	// �α� ���� sorting - ���ʿ��� �ߺ� �����Ͱ� �����ϱ� ������ �ߺ��� ���� �Ѵ�
	// ��Ŷ ĸ�Ľ� �ϳ��� ���ῡ ���� ����� ��Ŷ���� �����Ѵ�. �̷��� ��Ŷ ĸ�� �����ʹ� ������ �������.
	sort_func();
	
	
	// LOG�� ��� ����
	mysql_init(&log_conn);	
	log_connection = mysql_real_connect(&log_conn, answer[2], "root", answer[3], NULL, 0, (char *)NULL, 1);	
	if (log_connection == NULL){			
			return;
	}	
	
	sprintf(query, "./%s/AG%02d%02d",  log_dir, tz.pre_mday, tz.pre_hour);
	
	if( access(query, 0) ==0 && agent_num != 0)
	{
	
		//�α� ���� ����
		sprintf(query, "LOAD DATA local INFILE './%s/%s' into table %s%d.AG%02d%02d", log_dir, LOGFILE, AGENT,agent_num, tz.pre_mday, tz.pre_hour);
		query_stat = mysql_real_query(log_connection, query, strlen(query));
		query_invalied(log_conn, query_stat);
		printf("[LOG] SEND FILE : %s\n", query);
					
		// Ʈ���� ����
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


// LOG ���� ���� SORTING
// �α� ���� sorting - ���ʿ��� �ߺ� �����Ͱ� �����ϱ� ������ �ߺ��� ���� �Ѵ�
// ��Ŷ ĸ�Ľ� �ϳ��� ���ῡ ���� ����� ��Ŷ���� �����Ѵ�. �̷��� ��Ŷ ĸ�� �����ʹ� ������ �������.
void sort_func()
{
	
	char system_sort[64];
	
	// �ʵ�(�÷�)�� �̿��� sort ��ɾ�	
	sprintf(system_sort,"sort -u %s/AG%02d%02d | sort -k 7 > %s/%s", log_dir, tz.pre_mday, tz.pre_hour, log_dir,LOGFILE);		
	system(system_sort);
	
	return;
}


// LOG�� DB�� ���۽� ���� ��ü�� ����(���� �ӵ� ���)�ϱ� ������
// �� ��, LOG ������ �������ָ鼭 ���� ������ LOG ���Ͽ� ����ϰ� �ȴ�.
void create_new_file()
{
	if(fd)
	{
		close(fd);
		puts("[PCAP] NEW LOG FILE");
	}
	
	sprintf(save_file, "%s/AG%02d%02d",log_dir, tz.mday, tz.hour);	
	fd = open(save_file, O_RDWR | O_CREAT);		// ��� ���� open
	return;
}
