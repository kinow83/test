/*
* @file: net_zone.h
* @brief: 소속 네트워크에 모든 호스트를 ping으로 알아낸다.
*/

#include <netdb.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>


// 255의 문자열로 DB에서의 기본 값은 *********.... (*가 255개의 문자열)이루어져 있다.
// 소속된 네트워크 호스트 보기를 요청하여 에이전트가 ping 보내어 응답이 있는 호스는를 확인하면
// 응답을 보낸 호스의 IP 뒷자리가 255 사이즈의 배열의 인덱스가 된다.
// 해당 인덱스에 * 대신 '1'을 기록 함으로써 
// DB에서는 '1'로 표시된 인덱스 번호가 소속된 네트워크에 살아 있는 호스트의 IP 뒷자리가 된다.
char net_zone[255];		// C CLASS 범위의 소속 네트워크를 저장하는 변수




int seqnum;            // ping 메시지 일련번호
char recvbuf[4096];   // 수신버퍼
char sendbuf[4096];   // 송신버퍼
int rawsock;              // Raw 소켓 번호
int notrecv = 0;           // ping 응답을 받지 못한 회수
struct sockaddr_in sendaddr, recvaddr;
int death;

int send_ping();                                    // ping request
void errquit(char *msg) { perror(msg); exit(0); }
int ping();																					// ping 패킷 생성
unsigned short in_cksum(unsigned short *addr, int len);		// 체크섬 계산
int send_netzone(char *host, char *user, char *password);	// DBMS로 소속 네트워크 모든 호스트 전송

// 소속 네트워크 보기 - ping 패킷 생성 
void *ping_packet_thread_func( void *data )
{
	ping();
}

// select 함수를 사용하여 non-block으로 패킷을 생성
int ping() {
	
    int recvsize,	addrlen=sizeof(struct sockaddr);
    fd_set readset;
    struct timeval tv;
    int ret;
		u_int32_t host, ip, mask, tmp ;
		u_char *p;
 		int i, flag;
 		unsigned char index;
 		
 		
    addrlen = sizeof(struct sockaddr);
    bzero(&recvaddr, sizeof(struct sockaddr));
    bzero(&sendaddr, sizeof(struct sockaddr));
    
    p = (char *)&recvaddr.sin_addr;

    memset(&net_zone, '*', sizeof(net_zone));
    
    // raw 소켓 생성
    rawsock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(rawsock<0) errquit("socket fail");
    	
    flag = fcntl( rawsock, F_GETFL, 0 );    	
		fcntl( rawsock, F_SETFL, flag | O_NONBLOCK );
    	
    sendaddr.sin_family = AF_INET;
		sendaddr.sin_port = htons(0);

		host = inet_addr(ifcg.ip);
		mask = inet_addr(ifcg.netmask);

		ip = host & mask;
		
		tmp = ntohl(ip);
		tmp++;
		ip = htonl(tmp);
		
		// C CLASS 범위로 ping 패킷을 뿌린다.
    for(i=0; i<255; i++){
    		usleep(100);
        FD_ZERO(&readset);
        FD_SET(rawsock, &readset);
        tv.tv_sec =  0; // 1초 타이머
        tv.tv_usec = 0;

				sendaddr.sin_addr.s_addr = ip;
				tmp = ntohl(ip);
				tmp++;
				ip = htonl(tmp);

        send_ping();    // ping을 보냄

        ret = select(rawsock+1, &readset, NULL,NULL,&tv); // 타이머 0
        
        if ( recvfrom(rawsock, recvbuf, sizeof(recvbuf),0, (struct sockaddr*)&recvaddr, &addrlen) > 0){
        	index = *(unsigned char *)(p + 3);
        	if(index < 255)
        	{
        		net_zone[index] = '1';        		
        	}        	
        }
    }
    
    FD_CLR(rawsock, &readset);
    close(rawsock);
    
    send_netzone(answer[2], USER, answer[3]);
    
    
     return 0;
}


// ping Request 보내기
int send_ping() {
    struct icmphdr *icmp;
    int len, sendsize;
    icmp = (struct icmphdr *) sendbuf;
    bzero((char *)icmp, sizeof(struct icmp));
    icmp->code = 0 ;
    icmp->type  = ICMP_ECHO; // ICMP_ECHO = 8
    icmp->un.echo.sequence = seqnum;// Ping 메시지 일련번호
    icmp->un.echo.id = getpid();    // pid 를 ID로 설정
    icmp->checksum = 0;        // checksum 계산전 반드시 zero
    icmp->checksum=in_cksum((unsigned short *)icmp, sizeof(struct icmp));
    len = sizeof(struct icmphdr);  // 8 byte
    sendsize = sendto(rawsock, sendbuf, len, 0,
        (struct sockaddr*)&sendaddr, sizeof(struct sockaddr));

    //prn_icmp(icmp,sendsize); // ICMP 헤더 출력
    return sendsize;
}

// checksum 구하기
unsigned short in_cksum(unsigned short *addr, int len)
 {
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while(nleft>1) {
        sum += *w++; nleft -= 2;
    }

    if( nleft==-1) {
        *(unsigned char *) (&answer) = *(unsigned char *)w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum>>16);
    answer = ~sum;
    return (answer);
}	


// send net_zone to DBMS
int send_netzone(char *host, char *user, char *password)
{
	
	MYSQL	*mirror_ip_connection=NULL, mirror_ip_conn;
	char query[512];
	int query_stat;
	
	mysql_init(&mirror_ip_conn);	
	mirror_ip_connection = mysql_real_connect(&mirror_ip_conn, host, user, password, NULL, 0, (char *)NULL, 1);	
	if (mirror_ip_connection == NULL){			
			return -1;
	}
	
	sprintf(query, "UPDATE %s SET net_zone ='%s' WHERE id=%d", AGENT_INFORMATION_STATUS, net_zone, agent_num);
	
	query_stat = mysql_real_query(mirror_ip_connection, query, strlen(query));
	if( query_invalied(mirror_ip_conn, query_stat) ){
			return -1;
	}	
	
	mysql_close(mirror_ip_connection);
	
	return 0;
}


// 미러링 포트에 할당된 IP를 전송
int send_mirror_ip(char *host, char *user, char *password)
{
	
	MYSQL	*mirror_ip_connection=NULL, mirror_ip_conn;
	char query[512];
	int query_stat;
	
	mysql_init(&mirror_ip_conn);	
	mirror_ip_connection = mysql_real_connect(&mirror_ip_conn, host, user, password, NULL, 0, (char *)NULL, 1);	
	if (mirror_ip_connection == NULL){			
			return -1;
	}
	
	
	sprintf(query, "UPDATE %s SET  mirror_ip='%s' WHERE id=%d", AGENT_INFORMATION_STATUS, ifcg.ip, agent_num);
	puts(query);
	query_stat = mysql_real_query(mirror_ip_connection, query, strlen(query));
	if( query_invalied(mirror_ip_conn, query_stat) ){
			return -1;
	}
	mysql_close(mirror_ip_connection);	
	return 0;
}









/* 참고 */
//////////////////////////////////////////////////////////////
// ARP 로 네트워크 소속된 host 알아보기
//////////////////////////////////////////////////////////////

/*
#define ETH_HW_ADDR_LEN 6  
#define IP_ADDR_LEN 4    
#define ARP_FRAME_TYPE 0x0806  
#define ETHER_HW_TYPE 1         
#define IP_PROTO_TYPE 0x0800
#define OP_ARP_REQUEST 1
#define OP_ARP_REPLY 0



struct arp_packet {
   u_char 	dst_hw[ETH_HW_ADDR_LEN];
   u_char 	src_hw[ETH_HW_ADDR_LEN];
   u_short 	frame_type;
   u_short 	hw_type;

   u_short 	prot_type;
   u_char 	hw_addr_size;
   u_char 	prot_addr_size;
   u_short 	op;
   u_char 	send_hw[ETH_HW_ADDR_LEN];
   u_int 		send_ip;
   u_char 	recv_hw[ETH_HW_ADDR_LEN];
   u_int 		recv_ip;
   u_char 	padding[8];
};



char net_zone[255];
void * netzone_thread_func( void *data );
int send_netzone(char *host, char *user, char *password);

void *ping_packet_thread_func( void *data )
{
	
	pthread_t netzone_thread;
	int status;
	struct arp_packet pkt, rkt;
	struct ether_addr * ether;
	u_char broad_mac[ETH_HW_ADDR_LEN] = {0xfF, 0xfF, 0xfF, 0xfF, 0xfF, 0xfF};
	u_char nul_mac[ETH_HW_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	unsigned long mask, loop_ip;
	struct sockaddr sa, da;	
	int sock;
	int da_len = sizeof(da);
	
	
	u_char *recv_ipp;
	



	// raw 소켓 생성
	sock=socket(AF_INET,SOCK_RAW, htons(0x0003));
	if(sock<0){
		perror("socket");		
	}

	sa.sa_family = AF_INET;

	// net_zone 초기화
	memset(net_zone, '*', sizeof(net_zone));
	net_zone[sizeof(net_zone)-1] = 0;


	//옵션 설정
	pkt.frame_type		= htons(ARP_FRAME_TYPE);
	pkt.hw_type			= htons(ETHER_HW_TYPE);
	pkt.prot_type		= htons(IP_PROTO_TYPE);
	pkt.hw_addr_size	= ETH_HW_ADDR_LEN;
	pkt.prot_addr_size	= IP_ADDR_LEN;
	pkt.op				= htons(OP_ARP_REQUEST);


	// 출발지 맥주소 
	ether = ether_aton(ifcg.mac);
	memcpy( &pkt.src_hw  , ether->ether_addr_octet, sizeof(u_char)* ETH_HW_ADDR_LEN );
	memcpy( &pkt.send_hw , ether->ether_addr_octet, sizeof(u_char)* ETH_HW_ADDR_LEN );
	
	// 목적지 맥주소
	memcpy( &pkt.dst_hw , &broad_mac, sizeof(u_char)* ETH_HW_ADDR_LEN );
	memcpy( &pkt.recv_hw ,&nul_mac,		sizeof(u_char)* ETH_HW_ADDR_LEN );

	// 출발지 ip
	pkt.send_ip = inet_addr(ifcg.ip);
	

	// 넷마스크
	mask = inet_addr(ifcg.netmask);

	//목적지 ip
	loop_ip = pkt.send_ip & mask;
	
	recv_ipp = (u_char *)&pkt.recv_ip-2;

	//padding
	bzero(pkt.padding,8);
	strcpy(sa.sa_data,answer[1]);
	
	

	if(( status = pthread_create( &netzone_thread, NULL, &netzone_thread_func, (void *)NULL)) != 0 ) {
		printf("timer Thread error : %s\n", strerror(status));
		return;
	}	
	


	int i;	
	
	for(i =0; i<254*2; i++){

		loop_ip		= ntohl(loop_ip);
		loop_ip++;
		loop_ip   = htonl(loop_ip);
		
		*((u_int *)recv_ipp) = loop_ip;
		
		puts("send");

		if(sendto(sock, &pkt, sizeof(pkt), 0, &sa, sizeof(sa)) < 0){
			perror("sendto");
			close(sock);
		}
		
		
		usleep(1);		
	}
	pthread_exit(0);
	
}




void * netzone_thread_func( void *data )
{
	int sock,i; 
	struct sockaddr myaddr, from; 
  u_int32_t fromlen; 
  struct sockaddr da;
  struct arp_packet pkt;
  int da_len = sizeof(da);
  struct in_addr	addr;
  
  
  sock = socket(AF_INET, SOCK_PACKET, htons(0x0003)); 
  
  memset(&pkt, 0, sizeof(pkt));
  da.sa_family = AF_INET;
  strcpy(da.sa_data,answer[1]);
  
  bind(sock, &da, sizeof(da));
  
  while(1){
  	if( recvfrom(sock, &pkt, sizeof(pkt), 0, &da, &da_len) > 0){
  		if(pkt.op == htons(OP_ARP_REPLY)){
  			addr.s_addr = pkt.recv_ip;
				puts(inet_ntoa(addr));
			}
		}
	}
 }


int send_netzone(char *host, char *user, char *password)
{
	
	MYSQL	*mirror_ip_connection=NULL, mirror_ip_conn;
	char query[1024];
	int query_stat;
puts("111111");
	mysql_init(&mirror_ip_conn);	
	mirror_ip_connection = mysql_real_connect(&mirror_ip_conn, host, user, password, NULL, 0, (char *)NULL, 1);	
	if (mirror_ip_connection == NULL){			
			return -1;
	}
	
	sprintf(query, "UPDATE %s SET net_zone ='%s' WHERE id=%d", AGENT_INFORMATION_STATUS, net_zone, agent_num);
	puts(query);
	query_stat = mysql_real_query(mirror_ip_connection, query, strlen(query));
	if( query_invalied(log_conn, query_stat) ){
			return;
	}
	
	
	mysql_close(mirror_ip_connection);
	
	return 0;
}

*/







