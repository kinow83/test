
/*
* @ file 	: pcap.h
*	@ brief : 패킷 캡쳐
*/
#include <pcap.h>

#define PROMISCUOUS		1	//무차별 모드
#define NONPROMISCUOUS	0
#define SNAPSIZE		1024	//캡쳐 사이즈


#define log_dir 				"log"					// LOG 디렉토리
#define live_log				log_dir"/LIVE_LOG"			// 실시간 로그 저장 파일
#define LIVE_LOG_FILE		log_dir"/LIVE_LOG_FILE"	// 로그 저장 파일
#define TRAFFIC_FILE		log_dir"/TRAFFIC_FILE"	// 트래픽 저장 파일

#define LIVE_INTERVAL 5	// 실시간 요청은 5초 동안 이루어진다.

#define pcap_filter	" ! arp "	// 패킷 필터 룰

// 트래픽 구조체
typedef struct size{
	int Byte;
	int KByte;
	int MByte;
	int GByte;
	int TByte;
}SIZE;

// 실시간 로그를 DBMS에 전송하기 위한 쓰레드 전달 변수
struct argument{
  	int end_time;	// 실시간 완료 시간
  	int agent_num;// 에이전트 번호
  	char *host;		// 디비 호스트
  	char *user;		// 디비 유저
  	char *password;	// 디비 비밀 번호
  	int sock;	// 소켓 정보
};


char* itoa(unsigned int val);	// 정수를 char로 변환
char * _strcat(char * src, char * dst);	// 향상된 strcat 함수
//char * _strnstr(char *big, char *little, int len);
void pcap_callback(u_char *user, const struct pcap_pkthdr *h, const u_char *packet) ;	// 콜백 함수
void Log_write(char *pbuf, char *url);	// LOG 기록
void Log_live_write(char *pbuf, char *url, const struct pcap_pkthdr *h);	// 파일에 LOG 기록
void cal_traffic();	// 트래픽 계산
void pcap_capture(char *dev, char *filter);	// 패킷 캡쳐
void live_pcap_callback(u_char *user, const struct pcap_pkthdr *h, const u_char *packet);	// 실시간 패킷 캡쳐 콜백 함수
void live_pcap_capture(char *dev, char *filter, int agent_num, char *host, char *user, char *password, int sock);	// 실시간 패킷 캡쳐
void cleanup(void *myarg);	// 쓰레드 종료 시 수행되는 함수 - 실시간 패킷 캡쳐 종료 후 DB에 저장한다(live_log_send 함수 호출)
int live_log_send(char *host, char *user, char *password, int agent_num);	//실시간 패킷 캡쳐 종료 후 DB에 저장한다
void live_sort_func();	// 실시간 LOG의 중복 제거하는 sort함수


SIZE traffic;	// 트래픽

// 로그 저장 변수
struct ether_header *ep;	// ethernet
struct ip  *iph;					// ip
struct tcphdr *tcph;			// tcp
char buf_header[128];			// packet_data
int fd=0;									// LOG 파일 디스크립터
unsigned short ether_type;
char url_host[128];				// url 문자열
char *Host;								// "Host : " 찾는 포인터
char save_file[24];



// 실시간 로그 보기 변수
struct ether_header *live_ep;	// ethernet
struct ip  *live_iph;					// ip
struct tcphdr *live_tcph;			// tcp
char live_buf_header[128];			// packet_data
int live_fd=0;									// LIVE_LOG 파일 디스크립터
unsigned short live_ether_type;
char live_url_host[128];				// url 문자열
char *live_Host;								// "Host : " 찾는 포인터


uint32_t pre_ip_dst;
uint32_t pre_ip_src;
uint16_t pre_port_dst;
uint32_t pre_tcp_dst;
uint32_t pre_udp_dst;


int s;


// 트래픽 계산
// byte ~ t-byte 까지 계산
void cal_traffic(){	
	while(traffic.Byte >= 1024){
		traffic.Byte -= 1024;
		traffic.KByte++;
	}	
	if(traffic.KByte >= 1024){
		traffic.KByte -= 1024;
		traffic.MByte++;		
	}
	if(traffic.MByte >= 1024){
		traffic.MByte -= 1024;
		traffic.GByte++;		
	}
	if(traffic.GByte >= 1024){
		traffic.GByte -= 1024;
		traffic.TByte++;		
	}
	return;	
}

// 실시간 패킷 캡쳐
void live_pcap_capture(char *dev, char *filter, int agent_num, char *host, char *user, char *password, int sock)
{
	
	pcap_t *pd;
	char	ebuf[PCAP_ERRBUF_SIZE]; 	// error buffer
  struct in_addr      netmask, 		// 넷마스크
                      network;		// 네트워크 주소
  struct bpf_program  fcode;  	 	// 패킷필터링 프로그램  
  int status;
  struct argument arg;
  
  pd = pcap_open_live(dev, SNAPSIZE, PROMISCUOUS, 0, ebuf); // 디바이스 열기
  if(pd == NULL) {
  	fprintf(stderr, "pcap_open_live fail: %s", ebuf);
    exit(0);
  }

  // device localnet과, netmask
  pcap_lookupnet(dev, &network.s_addr, &netmask.s_addr, ebuf);

		
	// 필터링 규칙 컴파일
	pcap_compile(pd, &fcode, filter, 0, netmask.s_addr);
  pcap_setfilter(pd, &fcode); // 디스크립터에 필터링 규칙적용

  printf("[LIVE] CAPUTRE START Dev='%s'[net=%s]\n", dev, inet_ntoa(network), inet_ntoa(netmask));

	
	//에이전트  정보 전달
	arg.host = host;
	arg.user = user;
	arg.password = password;
	arg.agent_num = agent_num;
	arg.sock = sock;
	
	//live pcap capure 가 끝 날 시간을 계산
	arg.end_time=tz.t + LIVE_INTERVAL;	
	
	// LIVE_LOG 파일 열기
	live_fd = open(live_log, O_RDWR | O_CREAT);
	
	
	if(pcap_loop(pd, -1, live_pcap_callback, (void *)&arg)<0) {
		fprintf(stderr, "pcap_loop fail: %s\n", pcap_geterr(pd));
        exit(-1);
  }
    
  pcap_close(pd);	// close the packet capture discriptor
}


// 실시간 패킷 캡쳐 콜백 함수
void live_pcap_callback(u_char *arg, const struct pcap_pkthdr *h, const u_char *packet)
{
	char * pbuf = live_buf_header;	//헤더 정보	
	register int i = 0;
	
	
	pthread_cleanup_push(cleanup, arg);
	// 시간이 다 되면 쓰레드 죽기 - 죽은후 디비에 저장
	if(tz.t >= ((struct argument *)arg)->end_time){
		pthread_exit(0);
	}
	pthread_cleanup_pop(0);
	
	
	memset(live_buf_header, 0, sizeof(live_buf_header));
	
	// 이더넷 헤더
	live_ep = (struct ether_header *)packet;
	
	// IP헤더 가져오기 위해 이더넷 헤더 만큼 offset
	packet += sizeof(struct ether_header);
	
	// 이더넷 헤더 타입
  live_ether_type = ntohs(live_ep->ether_type);
  
	
	// @ IP protocol
	live_iph = (struct ip *)packet;
	
	// 이더넷
  if (live_ether_type == ETHERTYPE_IP ){
  	
  	
  	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // @ TCP protocol
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////  	    
    live_tcph = (struct tcphdr *)(packet + live_iph->ip_hl * 4);
    
           
		if (live_iph->ip_p == IPPROTO_TCP){
			
    	// @point : http header 
      packet += sizeof(struct tcphdr) + 20; // 20 tcp header
    	
    	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	// @ HTTP protocol
    	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	if(live_tcph->dest == htons(80)){
    		
    		// url 정보를 기록
    		if((live_Host = strstr(packet+150, "Host: ")) != NULL){
    			
    			int i = 0;
    			live_Host += 6;
    			while( *live_Host != '\r' && *live_Host != 0 )
    				live_url_host[i++] = *live_Host++;    				
    			
    			url_host[i] = 0;
    			
    			// LOG 기록
    			Log_live_write(pbuf, url_host, h);    			
    			write(live_fd, live_buf_header, strlen(live_buf_header));
    		}
    	}    	
    	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	// @ NOT HTTP protocol
    	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	else{
    		Log_live_write(pbuf, "TCP", h);	
    		write(live_fd, live_buf_header, strlen(live_buf_header));
    	}
    }
    
  
  	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// @ UDP protocol
  	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  	else if(iph->ip_p == IPPROTO_UDP){
  		Log_live_write(pbuf, "UDP", h);
    	write(live_fd, live_buf_header, strlen(live_buf_header));
    }
  }
 //printf("%s", live_buf_header);
 // printf("%d %d %d %d	%d	%d\n",traffic.GByte,traffic.MByte,traffic.KByte,traffic.Byte, h->len, s);
	
}




// 패킷 캡쳐
void pcap_capture(char *dev, char *filter)
{
	pcap_t *pd;
	char	ebuf[PCAP_ERRBUF_SIZE]; 	// error buffer
  struct in_addr      netmask, 		// 넷마스크
                      network;		// 네트워크 주소
  struct bpf_program  fcode;  	 	// 패킷필터링 프로그램  
  int status;
  
  

  pd = pcap_open_live(dev, SNAPSIZE, PROMISCUOUS, 0, ebuf); // 디바이스 열기
  if(pd == NULL) {
  	fprintf(stderr, "pcap_open_live fail: %s", ebuf);
    exit(0);
  }

  // device localnet과, netmask
  pcap_lookupnet(dev, &network.s_addr, &netmask.s_addr, ebuf);

		
	// 필터링 규칙 컴파일
	pcap_compile(pd, &fcode, filter, 0, netmask.s_addr);
  pcap_setfilter(pd, &fcode); // 디스크립터에 필터링 규칙적용

  printf("[PCAP] CAPUTRE START Dev='%s'[net=%s]\n", dev, inet_ntoa(network), inet_ntoa(netmask));

	// LOG 파일 열기 - LOG명 : AG0913 - 9일 13일 로그파일
	sprintf(save_file, "%s/AG%02d%02d",log_dir, tz.mday, tz.hour);	
	fd = open(save_file, O_RDWR | O_CREAT);		// 백업 파일 open
	
	
	if(pcap_loop(pd, -1, pcap_callback, NULL)<0) {
		fprintf(stderr, "pcap_loop fail: %s\n", pcap_geterr(pd));
        exit(-1);
  }
    
  pcap_close(pd);	// close the packet capture discriptor
}




/*
 * @ pcap_capture's  callback
 */

void pcap_callback(u_char *user, const struct pcap_pkthdr *h, const u_char *packet) 
{	
	
	char * pbuf = buf_header;	//헤더 정보	
	register int i = 0;	
	
	
	if(STATUS == STOP){
		puts("[EXIT] pcap_capture");
		pthread_exit(0);
	}
	
	
	memset(buf_header, 0, sizeof(buf_header));
	
	// 트래픽 계산
	traffic.Byte += h->len;	
	cal_traffic();
	
	
	// 이더넷 헤더
	ep = (struct ether_header *)packet;
	
	// IP헤더 가져오기 위해 이더넷 헤더 만큼 offset
	packet += sizeof(struct ether_header);
	
	// 이더넷 헤더 타입
  ether_type = ntohs(ep->ether_type);
  
	
	// @ IP protocol
	iph = (struct ip *)packet;
	
  if (ether_type == ETHERTYPE_IP ){
  	
  	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // @ TCP protocol
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////  	
    
    tcph = (struct tcphdr *)(packet + iph->ip_hl * 4);
    
    int x = tcph->psh;        
		if (iph->ip_p == IPPROTO_TCP){
			
    	// @point : http header 
      packet += sizeof(struct tcphdr) + 20; // 20 tcp header      
    	
    	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	// @ HTTP protocol
    	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	
    	// url 정보 기록
    	if(tcph->dest == htons(80)){
    		if((Host = strstr(packet+150, "Host: ")) != NULL){    			
    			
    			int i = 0;
    			Host += 6;
    			while( *Host != '\r' && *Host != 0 )
    				url_host[i++] = *Host++;    				
    			
    			url_host[i] = 0;
    			
    			Log_write(pbuf, url_host);
    			write(fd, buf_header, strlen(buf_header));
    		}
    	}    	
    	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	// @ NOT HTTP protocol
    	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    	else{
    		Log_write(pbuf, "TCP");
    		write(fd, buf_header, strlen(buf_header));
    	}
    }
    
  
  	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// @ UDP protocol
  	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
  	else if(iph->ip_p == IPPROTO_UDP){
  		Log_write(pbuf, "UDP");
    	write(fd, buf_header, strlen(buf_header));  	
    }
  }
	// printf("%s", buf_header);
 	// printf("%d %d %d %d	%d	%d\n",traffic.GByte,traffic.MByte,traffic.KByte,traffic.Byte, h->len, s);
}


// LOG 기록
void Log_write(char *pbuf, char *url)
{
	
	pbuf = (char *)_strcat(pbuf, (char *)inet_ntoa(iph->ip_src));	// 출발지 ip
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, itoa(ntohs(tcph->source)));	// 출발지 port
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, (char *)inet_ntoa(iph->ip_dst));	// 목적지 ip
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, itoa(ntohs(tcph->dest)));		// 목적지 port
	pbuf = (char *)_strcat(pbuf, "\t");	
	pbuf = (char *)_strcat(pbuf, url);												//url
	pbuf = (char *)_strcat(pbuf, "\t");
	
	// 패킷 시간 정보
	*pbuf++ = tz.year / 1000 + '0';
	*pbuf++ = (tz.year % 1000) / 100 + '0';
	*pbuf++ = (tz.year % 100) / 10 + '0';
	*pbuf++ = tz.year% 10 + '0';
	*pbuf++ = '-';
	*pbuf++ = (tz.mon) / 10 + '0';
	*pbuf++ = (tz.mon) % 10 + '0';
	*pbuf++ = '-';
	*pbuf++ = (tz.mday) / 10 + '0';
	*pbuf++ = (tz.mday) % 10 + '0';
	*pbuf++ = '\t';
	*pbuf++ = (tz.hour) / 10 + '0';
	*pbuf++ = (tz.hour) % 10 + '0';
	*pbuf++ = '-';
	*pbuf++ = (tz.min) / 10 + '0';
	*pbuf++ = (tz.min) % 10 + '0';
	*pbuf++ = '-';
	*pbuf++ = (tz.sec) / 10 + '0';
	*pbuf++ = (tz.sec) % 10 + '0';
	*pbuf++ = '\n';
}


// 실시간 로그 기록
void Log_live_write(char *pbuf, char *url, const struct pcap_pkthdr *h)
{
	
	pbuf = (char *)_strcat(pbuf, (char *)inet_ntoa(live_iph->ip_src));
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, itoa(ntohs(live_tcph->source)));	
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, (char *)inet_ntoa(live_iph->ip_dst));	
	pbuf = (char *)_strcat(pbuf, "\t");	
	pbuf = (char *)_strcat(pbuf, itoa(ntohs(live_tcph->dest)));		
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, url);
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, itoa(h->len));	// 패킷 크기
	pbuf = (char *)_strcat(pbuf, "\t");
				
	*pbuf++ = tz.year / 1000 + '0';
	*pbuf++ = (tz.year % 1000) / 100 + '0';
	*pbuf++ = (tz.year % 100) / 10 + '0';
	*pbuf++ = tz.year% 10 + '0';
	*pbuf++ = '-';
	*pbuf++ = (tz.mon) / 10 + '0';
	*pbuf++ = (tz.mon) % 10 + '0';
	*pbuf++ = '-';
	*pbuf++ = (tz.mday) / 10 + '0';
	*pbuf++ = (tz.mday) % 10 + '0';
	*pbuf++ = '\t';
	*pbuf++ = (tz.hour) / 10 + '0';
	*pbuf++ = (tz.hour) % 10 + '0';
	*pbuf++ = '-';
	*pbuf++ = (tz.min) / 10 + '0';
	*pbuf++ = (tz.min) % 10 + '0';
	*pbuf++ = '-';
	*pbuf++ = (tz.sec) / 10 + '0';
	*pbuf++ = (tz.sec) % 10 + '0';
	*pbuf++ = '\n';
	
}



// integer TO char
char* itoa(unsigned int val)
{
	static char buf[32] = {0};
	int i = 30;
	for( ; val && i ; --i, val /= 10)
		buf[i] = "0123456789"[val % 10];
	return &buf[i+1];
}


// 속도 향상된 strcat 함수
char * _strcat(char * src, char * dst)
{
	while(*src++); src--;
	while(*src++ = *dst++);	src--;
	return src;
}


/*  strstr 함수에 검색 길이 조건 추가  */
/*
char * _strnstr(char *big, char *little, int len)
{
	char *p1, *p2;
	while(len){
		while(len > 0 && *big++ != *little) len--;
		if(len <= 0) break;
		p1 = big-1;
		p2 = little;
		while(*p2 && *p1 == *p2) { p1++; p2++; }
		if(!*p2) return big-1;
		len--;
	}
	return NULL;
}

*/


// 실시간 쓰레드 종료 후 호춤됨. - 실시간 로그를 DB에 기록 하게됨
void cleanup(void *arg)
{
	struct argument *info = (struct argument *)arg;	
	COMMAND cmd;
	
	
	live_log_send( info->host, info->user, info->password, info->agent_num );
	
	cmd.signal = RPL_LIVE;
	cmd.data = 0;
	
	// 작업을 모두 마쳤으므로 MASTER 에게 알린다.
	write(info->sock, &cmd, sizeof(cmd));
	printf("cleanup   sock:%d\n",info->sock);
	puts("[COMMAND] SEND : AGENT -> RECV : MASTER [RPL_LIVE]");
	return;
    
}


// 실시간 로그를 DB에 기록
int live_log_send(char *host, char *user, char *password, int agent_num)
{	
	MYSQL       *live_connection=NULL, live_conn;	
	char query[128];
	int query_stat;
	
	
	mysql_init(&live_conn);	
	
	live_connection = mysql_real_connect(&live_conn, host, user, password, NULL, 0, (char *)NULL, 1);
	if (live_connection == NULL){
			return -1;
	}
	
	//live_sort_func();
	
	if( access(live_log, 0) ==0){
		
		sprintf(query, "DELETE FROM %s%d.%s", AGENT,agent_num, LIVE);
		puts(query);
		query_stat = mysql_real_query(live_connection, query, strlen(query));
		
		if( query_invalied(live_conn, query_stat) ){
			puts("[LIVE LOG] query invalid");
		}
		
		
		//LOAD DATA local INFILE
		sprintf(query, "LOAD DATA local INFILE '%s' into table %s%d.%s", live_log, AGENT,agent_num, LIVE);
	 puts(query);
		query_stat = mysql_real_query(live_connection, query, strlen(query));
		
		
		if( query_invalied(live_conn, query_stat) ){
			puts("[LIVE LOG] query invalid");
		}
		else{
			puts("[LIVE LOG] send to DBMS SUCESS");
		}
		
	}else{
		printf("[LIVE LOG] No file : %s\n", live_log);		
	}
	
	mysql_close(live_connection);
	
	unlink(live_log);	
	return 0;
	
}


// 실시간 로그의 중복을 제거 하는 sort 함수 - 선택
void live_sort_func()
{
	char system_sort[64];
	sprintf(system_sort,"sort -u %s | sort -k 8 > %s", live_log, LIVE_LOG_FILE);
	system(system_sort);
	unlink(LIVE_LOG_FILE);
	return;
}


