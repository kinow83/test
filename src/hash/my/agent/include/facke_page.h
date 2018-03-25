
/* fake paaket for access deny
 * date : 2008 . 8. 4
 * id   : kinow
 * doc  : 1. 3 way-handshake 세션 단계
 *        2. HOST는 "GET" 메소드의 페이지를 요청하는 PSH와 ACK 플래그가 포함된 패킷을 웹서버로 보낼 것이다. (브라우저를 통해...)
 *        3. 이 때 AGENT는 웹서버를 대신해 GET 요청에 대한 ACK 패킷을 HOST에게 보낸 후, 곧 바로
 *        4. 웹서버를 대신한 차단 페이지를 보낸다.
 *        5. 뒤 늦게 도착한 실제 웹서버의 페이지는 유효 하지 않게 된다.

 */

#include "http_filter.h"
#include "pipe.h"

#include "table.h"
#include "obj.h"

#define snapsize 1024          				// capture Length
#define http_header_size snapsize-40  // Snap Length
#define fake_rule		" ! arp "					// 캡쳐 시 arp 제외
#define TOTAL_NETWORK	"0.0.0.0"				// 전체 네트워크


int ssock;	// raw socket 전역 변수

char URL_PATH[] 			= "policy/url";
char KEYWORD_PATH[]		= "policy/keyword";
char IP_PATH[] 				= "policy/ip";

char URL_OBJ_PATH[]				= "policy/url_obj";
char KEYWORD_OBJ_PATH[] 	= "policy/keyword_obj";
char IP_OBJ_PATH[] 				= "policy/ip_obj";

char URL_BASE_PATH[] 			= "policy/url_base";
char KEYWORD_BASE_PATH[]	= "policy/keyword_base";
char IP_BASE_PATH[] 			= "policy/ip_base";

enum {O_START, O_STOP} OPERATION;

static char html[2048];


typedef struct pseudo_header{  /* rfc 793 tcp pseudo-header */
	unsigned long saddr, daddr;
	char mbz;
	char ptcl;
	unsigned short tcpl;
} ph_t;

ph_t ph;

// url 차단 쓰레드, ip 차단 쓰레드, keyword 차단 쓰레드로 각 각 복사된다.
// 각 쓰레드는 복사된 데이터로 차단 정책을 검색하게 된다.
struct packet_data{
	char check;
	int type;
	char data[http_header_size];	
	u_int32_t saddr;
	u_int32_t daddr;	
	u_int16_t source;
	u_int16_t dest;	
	u_int32_t ack_seq;
	u_int32_t seq;	
	u_int32_t packet_len;
};

char http_header[http_header_size];	// 웹페이지

/**************************
*			차단 프로세스				*
***************************/
void *fake_page_process(char *path);



static struct packet_data 	ip_data;			//ip 차단 구조체
static struct packet_data  keyword_data;	//keyword 차단 구조체
static struct packet_data  capture_data;	//url 차단 구조체
static struct packet_data  referer_data;	//referer url 구조체

extern OBJ	url_obj[OBJ_SIZE];	// url network object
extern OBJ	ip_obj[OBJ_SIZE];		// ip network object
extern OBJ	key_obj[OBJ_SIZE];	// keyword network object

extern BiTree  url_base;       // base("0.0.0.0") url network AVL TREE
extern BiTree  ip_base;        // base("0.0.0.0") ip network AVL TREE
extern BiTree  key_base;       // base("0.0.0.0") keyword network AVL TREE

extern int OBJ_create(OBJ *o, char *filepath);
extern int BASE_create(BiTree *b, char *filepath);
extern int URLToTable(char *filepath);
extern int IPToTable(char *filepath);
extern int KEYToTable(char *filepath);

// unsigned long 변수를 unsigned char 변수로 변환
unsigned char ntochar(unsigned long p);
// 차단 프로세스를 위한 패킷 캡쳐
void f_callback(  u_char *user, const struct pcap_pkthdr *h, const u_char *packet);
// ip check sum
unsigned short ip_sum (u_short *addr, int len);

// 패킷을 조합 함수
void send_tcp_segment(struct iphdr *ih, struct tcphdr *th, char *data, int dlen);
// 포트를 포함한 패킷 조립
void assemble_port(u_int32_t my_ip, u_int32_t their_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size);
// 응답 패킷 조립
void assemble_ack(u_int32_t my_ip, u_int32_t their_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size);
// 데이터가 포함한 패킷 조립
void assemble_psh(u_int32_t my_ip, u_int32_t their_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size);
// 리셋 패킷 조립
void assemble_rst(u_int32_t s_ip, u_int32_t d_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size);

// 웹 페이지를 파일에서 읽어와 메모리에 올림
int get_webpage(char *buf, char *path);
// 정책을 메모리에 LOAD
int policy_load(void);
// MASTER로 부터 정책을 가져옴
int get_policy(int agent_num);
// 정책 적용 쓰레드
void *apply_thread_func(void *data);
// 그룹 오브젝트를 셋팅
int group_set(int num);



// url 차단 쓰레드(referer 필드)
void *referer_filter_thread_func(void *data)
{	
	struct packet_data send_data;
	unsigned long no; // url's key
	unsigned long *p_no = &no;
	u_char host;
	char buf_url[http_header_size];
	
	while(1){
		if(OPERATION == O_START && referer_data.check == 1)
		{
			char *p2;
			
			memcpy(&send_data, &referer_data, sizeof(referer_data));
			
			if((p2 = strstr(referer_data.data+20, "Referer: ")) != NULL){
				register int i=0;
				p2 += 9;
					
				while(*p2 != '\r' && *p2 != 0){
					buf_url[i++] = *p2++;
				}
				
				buf_url[i] = 0;
				
				// url 해쉬 테이블에 있는지 찾음
				if( no = URL_serach(buf_url, url_node) ) // find url, return key
				{
								
					// 베이스 테이블에 존재
					if( bistree_lookup(&url_base, (void **)&p_no) != -1) //exist in base
					{
						puts("URL베이스 테이블에 존재");
						host = ntochar(capture_data.saddr); // octec 4th
									
						// 그룹 오브젝트 배열에 source ip에 해당하는 인덱스에 정책이 설정되었는지 검색
						if( url_obj_group[ host ] == 1 ) //apply group
						{
							// url 네트워크 오브젝트 검색 - 트리 검색
							if (bistree_lookup( &(url_obj[host].tree), (void **)&p_no ) != -1) // exist in object
							{ 	
								//DENY
								puts("URL베이스 테이블에 있고 그룹에 존재하고 호스트에 있고 차단됨");
								assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
			    			assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
							
							// 오브젝트에서 차단 안됨
							}
							else
							{
								puts("URL베이스 테이블에 있고 그룹에 존재하고 호스트에 없고 차단 안됨");
								break;
							}
						}
						// 오브젝트 그룹에 존재하지 않음.
						else	// empty group
						{	
							// DENY
							puts("URL베이스 테이블에 있고 그룹에 없어 차단됨");
							assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
			    		assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
						}
					}
								
					// 베이스 테이블에 존재 안함
					else // Didn't exist base
					{									
						host = ntochar(capture_data.saddr); // octec 4th
									
						// 그룹 오브젝트 배열에 source ip에 해당하는 인덱스에 정책이 설정되었는지 검색
						if( url_obj_group[ host ] == 1 ) //apply group
						{
							// url 네트워크 오브젝트 검색 - 트리 검색
							if (bistree_lookup( &(url_obj[host].tree), (void **)&p_no ) != -1) // exist in object
							{	
								puts("URL베이스 테이블에 없고 그룹에 있고 호스트에 있어 차단됨");
								assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
			    			assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
							}
							else
							{
								puts("URL베이스 테이블에 없고 그룹에 있고 호스트에 없어  차단 안됨");
								break;
							}
						}
									
						// 오브젝트 그룹에 존재하지 않음
						else // empty group
						{
							puts("URL베이스 테이블에 없고 그룹에 없어 차단 안됨");
							break;
						}
					}
				}
				referer_data.check = 0;
			}				
		}
		else
		{
				usleep(0);
		}
	}
}



// ip 차단 쓰레드
void *ip_filter_thread_func(void *data)
{	
	struct packet_data send_data;
	unsigned long no; // url's key
	unsigned long *p_no = &no;
	u_char host;
	
	while(1){
IP_OUT:
		if(OPERATION == O_START && ip_data.check == 1){
			
			memcpy(&send_data, &ip_data, sizeof(ip_data));			
			
			// ip 테이블에 있는지 찾음
			if( no = IP_serach(send_data.daddr, ip_node) ) // find ip, return key
			{
				puts("IP 테이블에 있는지 찾음");
				
				// 베이스 테이블에 존재 여부
				//exist in base
				if( bistree_lookup(&ip_base, (void **)&p_no) != -1)
				{
					// octec 4th
					host = ntochar(send_data.saddr);
					
					// 그룹 오브젝트 배열에 source ip에 해당하는 인덱스에 정책이 설정되었는지 검색
					if( ip_obj_group[ host ] == 1 )
					{
						// ip 네트워크 오브젝트 검색 - 트리 검색
						if (bistree_lookup( &(ip_obj[host].tree), (void **)&p_no ) != -1) // exist in object
						{ 
							//DENY
							puts("IP 베이스에 있고 그룹에 있고 호스트에 있어서 차단 됨");
							assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
			    		assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);			    		
						}
						else
						{
							puts("IP 베이스에 있고  그룹에 있고 호스트에 없어서 차단 안됨");
							goto IP_OUT;
						}
					}
					
					// empty group
					else
					{	
						// DENY
						puts("IP 베이스에 있고 그룹없어서 차단 됨");						
						assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
    				assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);    				
					}
				}
				
				// Didn't exist base
				else 
				{
					// octec 4th
					host = ntochar(send_data.saddr);
					
					// 그룹 오브젝트 배열에 source ip에 해당하는 인덱스에 정책이 설정되었는지 검색
					if( ip_obj_group[ host ] == 1 )
					{
						// ip 네트워크 오브젝트 검색 - 트리 검색
						if (bistree_lookup( &(ip_obj[host].tree), (void **)&p_no ) != -1) // exist in object
						{	
							//DENY
							puts("IP 베이스에 없고 그룹에 있고 호스트에 있어서 차단 됨");
							assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
	    				assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);	    				
						}
						else
						{
							puts("IP 베이스에 없고 그룹에 있고 호스트에 없어서 차단 안됨");
							goto IP_OUT;
						}
					}
					
					// empty group
					else
					{
						puts("IP 베이스에 없고 그룹없어서 차단 안됨");
						goto IP_OUT;
					}
				}
			}
			ip_data.check = 0;			
		}	
		else{
			usleep(0);
		}
	}	
}



// 키워드 차단 쓰레드
void *keyword_filter_thread_func(void *data)
{
	
	struct packet_data send_data;
	register int i;
	unsigned long no; // url's key
	unsigned long *p_no = &no;
	u_char host;
	
	
	while(1){
KEY_OUT:		
		if(keyword_data.check == 1 && OPERATION == O_START){
			
			memcpy(&send_data, &keyword_data, sizeof(keyword_data));			
			
			switch(send_data.type){
			
			case IS_GET :
				
				// 키워드 테이블에 키워드와 일치하는 문구 찾음
				// exist in key table
				if ( no = KEY_serach(send_data.data, key_node) )
				{					
					// 베이스 테이블에 존재 여부
					//exist in base
					if( bistree_lookup(&key_base, (void **)&p_no) != -1)
					{						
						host = ntochar(send_data.saddr); // octec 4th
						
						// 그룹 오브젝트 배열에 source ip에 해당하는 인덱스에 정책이 설정되었는지 검색
						if( key_obj_group[ host ] == 1 ) //apply group
						{
							// 키워드 네트워크 오브젝트 검색 - 트리 검색
							if (bistree_lookup( &(key_obj[host].tree), (void **)&p_no ) != -1) // exist in object
							{ 	
								//DENY
								puts("KEY베이스에 있고 그룹에 있고 호스트에 있어서 차단");
								assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
  				  		assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
    						assemble_rst(send_data.saddr, send_data.daddr, send_data.source, send_data.dest, send_data.seq+1, send_data.ack_seq, 0);
							}
							// 호스트에 존재 하지 않음
							else
							{
								puts("KEY베이스에 있고 그룹에 있고 호스트에 없어서 차단 안됨");
								goto KEY_OUT;
							}
						}
						// 오브젝트 그룹 테이블에 존재 하지 않음
						else	// empty group
						{	
							// DENY							
							puts("KEY베이스에 있고 그룹에 없어서 차단");
							assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
	  				  assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
							assemble_rst(send_data.saddr, send_data.daddr, send_data.source, send_data.dest, send_data.seq+1, send_data.ack_seq, 0);
						}
					}
					
					// 베이스 테이블에 존재 하지 않음
					else // Didn't exist base
					{
						// octec 4th
						host = ntochar(send_data.saddr);
					
						// 그룹 오브젝트 배열에 source ip에 해당하는 인덱스에 정책이 설정되었는지 검색
						if( key_obj_group[ host ] == 1 ) //apply group
						{
							// 키워드 네트워크 오브젝트 검색 - 트리 검색
							// exist in object
							if (bistree_lookup( &(key_obj[host].tree), (void **)&p_no ) != -1)
							{	
								//DENY								
								puts("KEY베이스에 없고 그룹에 있고 호스트에 있어서 차단");
								assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
		  					assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
								assemble_rst(send_data.saddr, send_data.daddr, send_data.source, send_data.dest, send_data.seq+1, send_data.ack_seq, 0);
							}
							// 호스트에 존재 하지 않음
							else
							{
								puts("KEY베이스에 없고 그룹에 있고 호스트 없어서 차단 안됨");
								goto KEY_OUT;
							}
						}
						
						// 오브젝트 그룹 테이블에 존재 하지 않음
						else // empty group
						{
							puts("KEY베이스에 없고 그룹에 없어서 차단 안됨");
							goto KEY_OUT;
						}
					}
				}
				break;
				
			case IS_POST :
				break;
				
			default: break;
				
			}
			keyword_data.check = 0;
		}				
		else{
			usleep(0);
		}
	}
}


/*
 * Check Sum 
 */ 
unsigned short ip_sum (u_short *addr, int len){
	register int nleft = len;
	register u_short *w = addr;
	register int sum = 0;
	u_short answer = 0;
	
	while (nleft > 1){
		  sum += *w++;
		  nleft -= 2;
	}

	if (nleft == 1){
		  *(u_char *) (&answer) = *(u_char *) w;
		  sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xffff);   /* add hi 16 to low 16 */
	sum += (sum >> 16);           /* add carry */
	answer = ~sum;                /* truncate to 16 bits */
	return (answer);
}



/* 
 * packet assembly 
 */ 
void send_tcp_segment(struct iphdr *ih, struct tcphdr *th, char *data, int dlen)
{
	char buf[1500];
	
	struct sockaddr_in sin;

	ssock=socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
  if(ssock<0) {
		perror("socket (raw)");
		return;
  }
	
	ph.saddr=ih->saddr;
	ph.daddr=ih->daddr;
	ph.mbz=0;
	ph.ptcl=IPPROTO_TCP;
	ph.tcpl=htons(sizeof(*th)+dlen);
	
	memcpy(buf, &ph, sizeof(ph));
	memcpy(buf+sizeof(ph), th, sizeof(*th));
	memcpy(buf+sizeof(ph)+sizeof(*th), data, dlen);
	memset(buf+sizeof(ph)+sizeof(*th)+dlen, 0, 4);
	th->check=ip_sum((u_short *)buf, (sizeof(ph)+sizeof(*th)+dlen+1)&~1);
	
	memcpy(buf, ih, 4*ih->ihl);
	memcpy(buf+4*ih->ihl, th, sizeof(*th));
	memcpy(buf+4*ih->ihl+sizeof(*th), data, dlen);
	memset(buf+4*ih->ihl+sizeof(*th)+dlen, 0, 4);
	
	ih->check=ip_sum((u_short *)buf, (4*ih->ihl + sizeof(*th)+ dlen + 1) & ~1);
	memcpy(buf, ih, 4*ih->ihl);


	/* make socket information */
	sin.sin_family=AF_INET;
	sin.sin_port=th->dest;
	sin.sin_addr.s_addr=ih->daddr;

	/* messege send  */
	if(sendto(ssock, buf, 4*ih->ihl + sizeof(*th)+ dlen, 0, (struct sockaddr *)&sin, sizeof(sin))<0) {
		printf("Error sending syn packet.\n"); perror("");		
	}
	close(ssock);	
}




/* 
 * reassembling ack
 */
void assemble_ack
(u_int32_t s_ip, u_int32_t d_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size){

	struct iphdr ih;
	struct tcphdr th;

	/* ip header */
	ih.version=4;
	ih.ihl=5;
	ih.tos=0;			/* XXX is this normal? */
	ih.tot_len=sizeof(ih)+sizeof(th)+size;	/* total length, from ip header to data */
	ih.id=htons(random());
	ih.frag_off=0;
	ih.ttl=30;
	ih.protocol=IPPROTO_TCP;
	ih.check=0;
	ih.saddr=s_ip;
	ih.daddr=d_ip;
	
	/* tcp header */
	th.source=sport;
	th.dest=dport;
	th.seq=seq;
	th.doff=sizeof(th)/4;
	th.ack_seq=htonl(ack);
	th.res1=0;
	th.fin=0;
	th.syn=0;
	th.rst=0;
	th.psh=0;
	th.ack=1;
	th.urg=0;
	th.res2=0;
	th.window=htons(65253);
	th.check=0;
	th.urg_ptr=0;

	send_tcp_segment(&ih, &th, "", 0); 
}




/* 
 * reassembling psh (psh, fin, ack)
 */
void assemble_psh
(u_int32_t s_ip, u_int32_t d_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size){

	struct iphdr ih;
	struct tcphdr th;
	
	/* ip header */
	ih.version=4;
	ih.ihl=5;
	ih.tos=0;			/* XXX is this normal? */
	ih.tot_len=sizeof(ih)+sizeof(th)+size;
	ih.id=htons(random());
	ih.frag_off=0;
	ih.ttl=30;
	ih.protocol=IPPROTO_TCP;
	ih.check=0;
	ih.saddr=s_ip;
	ih.daddr=d_ip;
	
	/* tcp header */
	th.source=sport;
	th.dest=dport;
	th.seq=seq;
	th.doff=sizeof(th)/4;
	th.ack_seq=htonl(ack);
	th.res1=0;
	th.fin=1;
	th.syn=0;
	th.rst=0;
	th.psh=1;	//PUSH
	th.ack=1;	//ACK
	th.urg=0;
	th.res2=0;
	th.window=htons(65253);
	th.check=0;
	th.urg_ptr=0;

	send_tcp_segment(&ih, &th, (char *)html, strlen(html)+1); 
}


/* 
 * reassembling port (psh, ack)
 */
void assemble_port
(u_int32_t s_ip, u_int32_t d_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size){

	struct iphdr ih;
	struct tcphdr th;

	/* ip header */
	ih.version=4;
	ih.ihl=5;
	ih.tos=0;			/* XXX is this normal? */
	ih.tot_len=sizeof(ih)+sizeof(th)+size;
	ih.id=htons(random());
	ih.frag_off=0;
	ih.ttl=30;
	ih.protocol=IPPROTO_TCP;
	ih.check=0;
	ih.saddr=s_ip;
	ih.daddr=d_ip;
	
	/* tcp header */
	th.source=sport;
	th.dest=dport;
	th.seq=seq;
	th.doff=sizeof(th)/4;
	th.ack_seq=htonl(ack);
	th.res1=0;
	th.fin=0;
	th.syn=0;
	th.rst=0;
	th.psh=1;	//PUSH
	th.ack=1;	//ACK
	th.urg=0;
	th.res2=0;
	th.window=htons(65253);
	th.check=0;
	th.urg_ptr=0;

	send_tcp_segment(&ih, &th, (char *)html, strlen(html)+1); 
}

/* 
 * reassembling rst (rst)
 */
void assemble_rst
(u_int32_t s_ip, u_int32_t d_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size){

	struct iphdr ih;
	struct tcphdr th;
	

	/* ip header */
	ih.version=4;
	ih.ihl=5;
	ih.tos=0;			/* XXX is this normal? */
	ih.tot_len=sizeof(ih)+sizeof(th)+size;
	ih.id=htons(random());
	ih.frag_off=0;
	ih.ttl=30;
	ih.protocol=IPPROTO_TCP;
	ih.check=0;
	ih.saddr=s_ip;
	ih.daddr=d_ip;
	
	/* tcp header */
	th.source=sport;
	th.dest=dport;
	th.seq=seq;
	th.doff=sizeof(th)/4;
	th.ack_seq=htonl(ack);
	th.res1=0;
	th.fin=0;
	th.syn=0;
	th.rst=1;
	th.psh=0;
	th.ack=0;
	th.urg=0;
	th.res2=0;
	th.window=htons(65253);
	th.check=0;
	th.urg_ptr=0;
		
	send_tcp_segment(&ih, &th, "", 0); 
}




/* 
 * pcap callback(차단 프로세스를 위한 패킷 캡쳐)
 */ 
void f_callback(  u_char *user, const struct pcap_pkthdr *h, const u_char *packet) {	
			
    struct tcphdr *tcph;
    struct ip  *iph;
    struct iphdr *iphiph;
    struct ethhdr *eh = (struct ethhdr *) packet;
    u_char * http;
    u_char host;
    char *cat;
    
    char *p1, *p2;
    char buf_url[http_header_size];    
    int type;    

    int packet_len =h->caplen;
    int ethhdr_len = sizeof(struct ethhdr);
    int iphlen;
    
    
    if(OPERATION == O_START)
    {    	
    	iphiph = (struct iphdr *) (packet + ethhdr_len);	
    	iph =  (struct ip *) (packet + sizeof(struct ethhdr));
    
    	// IP 헤더의 길이
    	iphlen = iph->ip_hl*4;

    	// 이더넷 프레임 헤더+ IP 헤더의 다음이 TCP 헤더임
    	tcph = (struct tcphdr *) (packet + sizeof(struct ethhdr) + iphlen) ;    
    	http = (u_char *)packet + ethhdr_len + iphlen + sizeof(struct tcphdr);    
    	type = type_request((char *)http);
    	
    
			// 3-way handshaking 이후에 패킷을 다룸.
			if( tcph->psh == 1 && tcph->ack ==1 ){
				
				// 각 쓰레드로 패킷 복사
				capture_data.check = 1;		
				capture_data.saddr = iphiph->saddr;
				capture_data.daddr = iphiph->daddr;
				capture_data.source = tcph->source;
				capture_data.dest = tcph->dest;
				capture_data.ack_seq = tcph->ack_seq;
				capture_data.seq = tcph->seq;
				capture_data.packet_len = packet_len;
				capture_data.type = type;
				memcpy(&ip_data, &capture_data, sizeof(capture_data));
			
			
				strncpy(capture_data.data, http, http_header_size);				
				memcpy(&keyword_data, &capture_data, sizeof(capture_data));
				
				/*********************************************************************************************************************************************
				// PORT 차단				
				if(tcph->dest == htons(1033)){					
					assemble_ack(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), 0);
  	  		assemble_port(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), packet_len-40);
  	  		assemble_rst(iphiph->saddr, iphiph->daddr, tcph->source, tcph->dest, tcph->seq+1, tcph->ack_seq, 0);
				}			
				***********************************************************************************************************************************************/
			
				switch(type){
			
					case IS_GET :
						///////////////////////////////////////////////////////////////////
						//        150 <- 이 값을 조정하여 차단률을 높일 수 있다.
						//        Host 필드까지 접근
						///////////////////////////////////////////////////////////////////
						if((p2 = strstr(http+150, "Host: ")) != NULL){
							unsigned long no; // url's key
							register int i=0;
							unsigned long *p_no = &no;

							p2 += 6;
					
							while(*p2 != '\r' && *p2 != 0){
								buf_url[i++] = *p2++;
							}
				
							buf_url[i] = 0;
							
							// url 해쉬 테이블에 있는지 찾음
							if( no = URL_serach(buf_url, url_node) ) // find url, return key
							{								
								// 베이스 테이블에 존재
								if( bistree_lookup(&url_base, (void **)&p_no) != -1) //exist in base
								{
									puts("URL베이스 테이블에 존재");
									host = ntochar(capture_data.saddr); // octec 4th
									
									// 그룹 오브젝트 배열에 source ip에 해당하는 인덱스에 정책이 설정되었는지 검색
									if( url_obj_group[ host ] == 1 )
									{																				
										// url 네트워크 오브젝트 검색 - 트리 검색
										// exist in object
										if (bistree_lookup( &(url_obj[host].tree), (void **)&p_no ) != -1) // exist in object
										{ 	
											//DENY
											puts("URL베이스 테이블에 있고 그룹에 존재하고 호스트에 있고 차단됨");
											assemble_ack(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), 0);
											assemble_psh(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), packet_len-40);
										}
										// 오브젝트에서 차단 안됨
										else
										{
											puts("URL베이스 테이블에 있고 그룹에 존재하고 호스트에 없고 차단 안됨");
											break;
										}
									}
									// 오브젝트 그룹에 존재하지 않음.
									else	// empty group
									{	
										// DENY
										puts("URL베이스 테이블에 있고 그룹에 없어 차단됨");
										assemble_ack(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), 0);
										assemble_psh(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), packet_len-40);
									}
								}
								
								// 베이스 테이블에 존재 안함
								else // Didn't exist base
								{									
									host = ntochar(capture_data.saddr); // octec 4th
									
									// 그룹 오브젝트 배열에 source ip에 해당하는 인덱스에 정책이 설정되었는지 검색
									if( url_obj_group[ host ] == 1 ) //apply group
									{
										if (bistree_lookup( &(url_obj[host].tree), (void **)&p_no ) != -1) // exist in object
										{	
											puts("URL베이스 테이블에 없고 그룹에 있고 호스트에 있어 차단됨");
											assemble_ack(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), 0);
											assemble_psh(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), packet_len-40);
										}
										else
										{
											puts("URL베이스 테이블에 없고 그룹에 있고 호스트에 없어  차단 안됨");
											break;
										}
									}
									
									// 오브젝트 그룹에 존재하지 않음
									else // empty group
									{
										puts("URL베이스 테이블에 없고 그룹에 없어 차단 안됨");
										break;
									}
								}
							}
						}				
						break;
				
					case IS_POST :
						
						break;
					
					default: 
						break;
				}
			}
	}
}




// 차단 프로세스 메인 함수
void *fake_page_process(char *path)
{
	
		char cmd[128];
	
    pcap_t  *pd;                     // pcap  디스크립터
    char     ebuf[PCAP_ERRBUF_SIZE], // error buffer
            *dev;                    // device name
    
    struct in_addr      netmask,     // 넷마스크
                        network;     // 네트워크 주소
    struct bpf_program  fcode;       // 패킷필터링 프로그램
    
    pthread_t ip_filter_thread;				// ip 차단 쓰레드
    pthread_t keyword_filter_thread;	// 키워드 차단 쓰레드
    pthread_t apply_thread;						// 정책 적용 쓰레드
    pthread_t referer_filter_thread;	// url-referer 쓰레드
    
    int status;
    
    // 정책이 적용된 파일 생성        
    sprintf(cmd,"touch %s",URL_PATH);    			system(cmd);
    sprintf(cmd,"touch %s",KEYWORD_PATH);    	system(cmd);
    sprintf(cmd,"touch %s",IP_PATH);			    system(cmd);
    
    sprintf(cmd,"touch %s",URL_OBJ_PATH);    	system(cmd);
    sprintf(cmd,"touch %s",KEYWORD_OBJ_PATH); system(cmd);
    sprintf(cmd,"touch %s",IP_OBJ_PATH);	    system(cmd);
    
    sprintf(cmd,"touch %s",URL_BASE_PATH);    	system(cmd);
    sprintf(cmd,"touch %s",KEYWORD_BASE_PATH); 	system(cmd);
    sprintf(cmd,"touch %s",IP_BASE_PATH);    		system(cmd);
    
    
    // block page 불러오기
    get_webpage(html, path);
    
    // 정책을 메모리에 Load
    if( policy_load() != 0 ){
    	puts("[FAKE] hashtable unload");
    	exit(1);
    }
    
    // 모든 쓰레드 상태 - 시작
    OPERATION = O_START;
    
    // 정책 반영 쓰레드
    if(( status = pthread_create( &apply_thread, NULL, &apply_thread_func, NULL)) != 0 ) {
			printf("apply_thread error : %s\n", strerror(status));			
		}
		
		// 키워드 차단 쓰레드
		if(( status = pthread_create( &keyword_filter_thread, NULL, &keyword_filter_thread_func, NULL)) != 0 ) {
			printf("keyword_filter_thread error : %s\n", strerror(status));			
		}
		
		// ip 차단 쓰레드
		if(( status = pthread_create( &ip_filter_thread, NULL, &ip_filter_thread_func, NULL)) != 0 ) {
			printf("ip_filter_thread error : %s\n", strerror(status));			
		}
		
		// url(referer)
		if(( status = pthread_create( &referer_filter_thread, NULL, &referer_filter_thread_func, NULL)) != 0 ) {
			printf("referer_filter_thread error : %s\n", strerror(status));			
		}
		
		    
    pd=pcap_open_live(answer[1],snapsize,1,1000,ebuf); // 디바이스 열기
    if(pd == NULL) {
        fprintf(stderr, "pcap_open_live fail: %s", ebuf);        
    }
    

    // 디바이스에 해당하는 localnet과 netmask를 얻음
    pcap_lookupnet(answer[1], &network.s_addr, &netmask.s_addr, ebuf);

    // 필터링 규칙 컴파일    
    pcap_compile(pd, &fcode, fake_rule,0, netmask.s_addr);

    pcap_setfilter(pd, &fcode); // 디스크립터에 필터링 규칙적용

    printf("[FAKE] CAPUTRE START Dev='%s'[net=%s]\n", answer[1], inet_ntoa(network));

    // f_callback : url 차단 루틴
    if(pcap_loop(pd, -1, f_callback, NULL)<0) {
        fprintf(stderr, "pcap_loop fail: %s\n", pcap_geterr(pd));
        exit(1);
    }
        
    
    pthread_join(apply_thread, NULL);	
    pthread_join(keyword_filter_thread, NULL);
		pthread_join(ip_filter_thread, NULL);	
		pcap_close(pd);
   return ; 
}



void *apply_thread_func(void *data)
{
	
	while(1){
		
		// pipeline[1] : fake가 받는 파이프
		// pipeline[0] : fake가 보내는 파이프
		if( read(pipeline[0], &ipc_buf, sizeof(ipc_buf)) != -1)
		{			
			if(ipc_buf == PLY_CHN){
				
				// 작동 중지 해쉬 테이블 다시 로드
				OPERATION = O_STOP;
				
				// 정책 다운로드				
				if (get_policy(agent_num) != 0){
					puts("[POLICY] download policy error");
				}
				puts("[POLICY] download policy");
				
				
				// 네트워크 객체 다운로드
				if(get_netobj(agent_num) != 0){
					puts("[POLICY] download network object error");
				}
				puts("[POLICY] download network object");
				
				// 해쉬 테이블 재 등록
				if( policy_load() != 0 ){
		    			puts("[FAKE] hashtable Reload error");    			
    				}
		    	puts("[FAKE] hashtable Reload");
    		
    			// 작동 재시작
		    	OPERATION = O_START;
    		
    			// 정책 적용 완료
		    	ipc_buf = PLY_APP;
    		
    			write(pipeline2[1], &ipc_buf, sizeof(ipc_buf));    		
			}
			else{
				puts("[PIPE] Not found COMMAND Define");				
			}
		}		
	}	
}




int get_webpage(char *buf, char *path)
{
        int fd;
        int cnt;
        
        int i=0;
        fd = open(path, O_RDONLY);

        if(fd == -1){
                printf("FILE open() error : %s\n",path);
                return -1;
        }

        while(cnt = read(fd, &html[i++], 1));
        html[i] = 0;
        close(fd);
        return 0;
}



// 정책을 메모리에 Load
int policy_load()
{	
	
	// 정책 파일열고 해쉬 테이블 생성
	puts("[HASH] policy_loading....");
	
	OBJ_group_init();	
	puts("[HASH] OBJ_group_init");
	
	URLToTable(URL_PATH);	
	puts("[HASH] URL DOWNLOAD");
	
	IPToTable(IP_PATH);	
  puts("[HASH] IP DOUNLOAD");
  
	KEYToTable(KEYWORD_PATH);	
  puts("[HASH] KEYWORD DOWNLOAD");

	BASE_create(&url_base, URL_BASE_PATH);
  puts("[HASH] URL BASE OBJECT DOWNLOAD");
  
	BASE_create(&ip_base, IP_BASE_PATH);
  puts("[HASH] IP BASE OBJECT DOWNLOAD");
  
	BASE_create(&key_base, KEYWORD_BASE_PATH);
  puts("[HASH] KEYWORD BASE OBJECT DOWNLOAD");
	
	OBJ_create(url_obj, URL_OBJ_PATH);
  puts("[HASH] URL OBJECT DOWNLOAD");
  
	OBJ_create(ip_obj, IP_OBJ_PATH);
  puts("[HASH] IP OBJECT DOWNLOAD");
  
	OBJ_create(key_obj, KEYWORD_OBJ_PATH);
  puts("[HASH] KEYWORD OBJECT DOWNLOAD");
  
  //오브젝트 그룹 set
  group_set(agent_num);

  return 0;	
}



// Master(DBMS)로 url, ip, keyword 정보를 가져온다 - 해쉬 테이블 만들기 .
int get_policy(int agent_num)
{
	
	MYSQL       *ply_connection=NULL, ply_conn;
	int query_stat;	
	char query[450];
	char buf[300];
	
	MYSQL_RES   *sql_result;
	MYSQL_ROW   sql_row;
	
	FILE * url_fp, * keyword_fp, * ip_fp;
	
	// 기존 정보 파일 삭제
	sprintf(buf,"rm -fr %s",URL_PATH);
	system(buf);
	sprintf(buf,"rm -fr %s",IP_PATH);
	system(buf);
	sprintf(buf,"rm -fr %s",KEYWORD_PATH);
	system(buf);
	
	
	puts(URL_PATH);
	puts(IP_PATH);
	puts(KEYWORD_PATH);
	
	// 새로운 정보 파일 생성
	if( !(url_fp = fopen(URL_PATH, "w")) )
	{
		puts(" fopen error");
		return -1;
	}
	if( !(keyword_fp = fopen(KEYWORD_PATH, "w")) )
	{
		puts(" fopen error");
		return -1;
	}
	if( !(ip_fp = fopen(IP_PATH, "w")) ){
		puts(" fopen error");
		return -1;
	}
	
	
	// 디비 접속
	mysql_init(&ply_conn);	
	
	ply_connection = mysql_real_connect(&ply_conn, answer[2], USER, answer[3], NULL, 0, (char *)NULL, 1);
	if (ply_connection == NULL){
			printf("%s\n", mysql_error(&ply_conn));
			return -1;
	}
	
	query_stat = mysql_select_db( ply_connection, "policy" );
	
	// SELECT distinct url.no, url.content FROM agent1_policy, url
	// where agent1_policy.type=1 and agent1_policy.catagory = url.catagory
	// and agent1_policy.no = url.no order by no
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// URL 가져오기
	////////////////////////////////////////////////////////////////////////////////////////////////
	sprintf(query,"SELECT distinct url.no, url.content FROM agent%d_policy, url WHERE agent%d_policy.type=1 and agent%d_policy.catagory = url.catagory and agent%d_policy.no = url.no" , 	agent_num, agent_num, agent_num, agent_num);	
	//puts(query);
	query_stat = mysql_real_query( ply_connection, query, strlen(query) );
	if( query_invalied( ply_conn, query_stat ) ){
		puts("URL POLICY DOWNLOAD FAIL");
		return -1;
	}
		
	sql_result = mysql_store_result( ply_connection );	
	
	puts("URL POLICY DOWNLOAD OK");
	
	while( sql_row = mysql_fetch_row(sql_result) ){
		
		sprintf(buf, "%s	%s\n", sql_row[0], sql_row[1]);		
		fputs(buf, url_fp);
	}
	fclose(url_fp);	
	mysql_free_result(sql_result);
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// IP 가져오기
	////////////////////////////////////////////////////////////////////////////////////////////////
	sprintf(query,"SELECT distinct %s.no, %s.content FROM agent%d_policy, %s  WHERE agent%d_policy.type=3 and agent%d_policy.catagory = %s.catagory and agent%d_policy.no = %s.no" , 	POLICY_ip, POLICY_ip, agent_num, POLICY_ip, agent_num, agent_num, POLICY_ip, agent_num, POLICY_ip);
	query_stat = mysql_real_query( ply_connection, query, strlen(query) );	
	//puts(query);
	if( query_invalied( ply_conn, query_stat ) ){
		puts("IP POLICY DOWNLOAD FAIL");
		return -1;
	}
	
	sql_result = mysql_store_result( ply_connection );
	
	puts("IP POLICY DOWNLOAD OK");
	
	while( sql_row = mysql_fetch_row(sql_result) ){
		
		sprintf(buf, "%s	%s\n", sql_row[0], sql_row[1]);
		
		fputs(buf, ip_fp);
	}
	fclose(ip_fp);	
	mysql_free_result(sql_result);
	
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// KEYWORD 가져오기
	////////////////////////////////////////////////////////////////////////////////////////////////
	sprintf(query,"SELECT distinct %s.no, %s.content FROM agent%d_policy, %s  WHERE agent%d_policy.type=2 and agent%d_policy.catagory = %s.catagory and agent%d_policy.no = %s.no" , 	POLICY_keyword, POLICY_keyword, agent_num, POLICY_keyword, agent_num, agent_num, POLICY_keyword, agent_num, POLICY_keyword);	
	//puts(query);
	query_stat = mysql_real_query( ply_connection, query, strlen(query) );
	if( query_invalied( ply_conn, query_stat ) ){
		puts("KEYWORD POLICY DOWNLOAD FAIL");
		return -1;
	}
	
	sql_result = mysql_store_result( ply_connection );
	
	puts("KEYWORD POLICY DOWNLOAD OK");
	
	while( sql_row = mysql_fetch_row(sql_result) ){
		
		sprintf(buf, "%s	%s\n", sql_row[0], sql_row[1]);
		
		fputs(buf, keyword_fp);
	}
	fclose(keyword_fp);

	mysql_free_result(sql_result);		
	mysql_close(ply_connection);

	return 0;	
	
}


// Master(DBMS)로 부터 정책을 가져온다 - 네트워크 오브젝트 만들기 .
int get_netobj(int agent_num)
{
	
	MYSQL       *obj_connection=NULL, obj_conn;
	int query_stat;	
	char query[600];
	char buf[300];	
		
	MYSQL_RES   *sql_result;
	MYSQL_ROW   sql_row;	
	
	FILE * url_fp, * keyword_fp, * ip_fp;
	FILE	*base_url,* base_ip, * base_keyword;
	
	// 기존 정책(네트워크 오브젝트) 파일 삭제
	unlink(URL_OBJ_PATH);
	unlink(IP_OBJ_PATH);
	unlink(KEYWORD_OBJ_PATH);
	
	// 기존 정책(기본 오브젝트) 파일 삭제
	unlink(URL_BASE_PATH);
	unlink(IP_BASE_PATH);
	unlink(KEYWORD_BASE_PATH);
	
	// 새로운 정책(네트워크 오브젝트) 파일 생성
	if( !(url_fp = fopen(URL_OBJ_PATH, "w")) )
	{
		puts(" fopen error");
		return -1;
	}
	if( !(keyword_fp = fopen(KEYWORD_OBJ_PATH, "w")) )
	{
		puts(" fopen error");
		return -1;
	}
	if( !(ip_fp = fopen(IP_OBJ_PATH, "w")) ){
		puts(" fopen error");
		return -1;
	}
	
	// 새로운 정책(기본 오브젝트) 파일 생성
	if( !(base_url = fopen(URL_BASE_PATH, "w")) )
	{
		puts(" fopen error");
		return -1;
	}
	if( !(base_keyword = fopen(KEYWORD_BASE_PATH, "w")) )
	{
		puts(" fopen error");
		return -1;
	}
	if( !(base_ip = fopen(IP_BASE_PATH, "w")) ){
		puts(" fopen error");
		return -1;
	}
	
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// DB 접속
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	mysql_init(&obj_conn);
	
	obj_connection = mysql_real_connect(&obj_conn, answer[2], USER, answer[3], NULL, 0, (char *)NULL, 1);
	if (obj_connection == NULL){
			printf("%s\n", mysql_error(&obj_conn));
			return -1;
	}
	
	// "use policy" //
	query_stat = mysql_select_db( obj_connection, "policy" );
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// URL 네트워크 객체 가져오기
	////////////////////////////////////////////////////////////////////////////////////////////////		
	sprintf(query,"SELECT distinct agent%d_policy.no, agent%d_group.host FROM agent%d_policy, agent%d_group WHERE agent%d_policy.netobj = agent%d_group.netobj and agent%d_policy.type=1",	agent_num, agent_num, agent_num, agent_num, agent_num, agent_num, agent_num);	
	query_stat = mysql_real_query( obj_connection, query, strlen(query) );
	//puts(query);
	if( query_invalied( obj_conn, query_stat ) ){
		puts("URL OBJECT DOWNLOAD FAIL");
		puts("URL BASE OBJECT DOWNLOAD FAIL");
		return -1;
	}
		
	sql_result = mysql_store_result( obj_connection );	
	
	while( sql_row = mysql_fetch_row(sql_result) ){
		
		////////////////////////////////////////////////////////////////////////
		// 네트워크 전체 선택(0.0.0.0) 이면 base 파일에 네트워크 객체 저장 -? 메모리 사용 효율
		////////////////////////////////////////////////////////////////////////
		if(!strcmp(sql_row[1],TOTAL_NETWORK)){
			sprintf(buf, "%s\n", sql_row[0]);
			fputs(buf, base_url);				
		}
		else{
			sprintf(buf, "%s	%s\n", sql_row[0], sql_row[1]);
			fputs(buf, url_fp);
		}
	}
	
	puts("URL OBJECT DOWNLOAD OK");
	puts("URL BASE OBJECT DOWNLOAD OK");
	fclose(url_fp);
	fclose(base_url);
	
	mysql_free_result(sql_result);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// IP 네트워크 객체 가져오기
	////////////////////////////////////////////////////////////////////////////////////////////////	
	sprintf(query,"SELECT distinct agent%d_policy.no, agent%d_group.host FROM agent%d_policy, agent%d_group WHERE agent%d_policy.netobj = agent%d_group.netobj and agent%d_policy.type=3",	agent_num, agent_num, agent_num, agent_num, agent_num, agent_num, agent_num);	
	query_stat = mysql_real_query( obj_connection, query, strlen(query) );
	//puts(query);
	if( query_invalied( obj_conn, query_stat ) ){
		puts("IP OBJECT DOWNLOAD FAIL");
		puts("IP BASE OBJECT DOWNLOAD FAIL");
		return -1;
	}
	
	sql_result = mysql_store_result( obj_connection );	
	
	while( sql_row = mysql_fetch_row(sql_result) ){
		
		////////////////////////////////////////////////////////////////////////
		// 네트워크 전체 선택(0.0.0.0) 이면 base 파일에 네트워크 객체 저장
		////////////////////////////////////////////////////////////////////////
		if(!strcmp(sql_row[1],TOTAL_NETWORK)){
			sprintf(buf, "%s\n", sql_row[0]);
			fputs(buf, base_ip);				
		}
		else{		
			sprintf(buf, "%s	%s\n", sql_row[0], sql_row[1]);		
			fputs(buf, ip_fp);
		}
	}
	puts("IP OBJECT DOWNLOAD OK");
	puts("IP BASE OBJECT DOWNLOAD OK");
	fclose(ip_fp);
	fclose(base_ip);
	
	mysql_free_result(sql_result);
	
	////////////////////////////////////////////////////////////////////////////////////////////////
	// KEYWORD 네트워크 객체 가져오기
	////////////////////////////////////////////////////////////////////////////////////////////////
		
	sprintf(query,"SELECT distinct agent%d_policy.no, agent%d_group.host FROM agent%d_policy, agent%d_group WHERE agent%d_policy.netobj = agent%d_group.netobj and agent%d_policy.type=2",	agent_num, agent_num, agent_num, agent_num, agent_num, agent_num, agent_num);	
	query_stat = mysql_real_query( obj_connection, query, strlen(query) );
	//puts(query);
	if( query_invalied( obj_conn, query_stat ) ){
		puts("KEYWORD OBJECT DOWNLOAD FAIL");
		puts("KEYWORD BASE OBJECT DOWNLOAD FAIL");
		return -1;
	}
	
	sql_result = mysql_store_result( obj_connection );
	
	while( sql_row = mysql_fetch_row(sql_result) ){
		
		////////////////////////////////////////////////////////////////////////
		// 네트워크 전체 선택(0.0.0.0) 이면 base 파일에 네트워크 객체 저장
		////////////////////////////////////////////////////////////////////////		
		if(!strcmp(sql_row[1],TOTAL_NETWORK)){
			sprintf(buf, "%s\n", sql_row[0]);
			fputs(buf, base_keyword);				
		}
		else{	
			sprintf(buf, "%s	%s\n", sql_row[0], sql_row[1]);		
			fputs(buf, keyword_fp);
		}
		
	}
	puts("KEYWORD OBJECT DOWNLOAD OK");
	puts("KEYWORD BASE OBJECT DOWNLOAD OK");
	fclose(keyword_fp);
	fclose(base_keyword);
	
	mysql_free_result(sql_result);
	
	mysql_close(obj_connection);
	
	return 0;
}


// unsigned long 정수를 unsigned char 로 변환
unsigned char ntochar(unsigned long p)
{
	u_char * index = (u_char *)&p;  // last octec's ipaddress 1 byte pointer
	u_char ret = *(index+3);        // last octec's ipaddress is hash index	
	return ret;
}



// 그룹 오브젝트 타입 판단 후 
// 그룹 오브젝트 배열에 개별 정책이 존재하므로 TRUE(1)로 셋팅 해준다.
int group_set(int num)
{
	MYSQL       *set_connection=NULL, set_conn;
	int query_stat;	
	MYSQL_RES   *sql_result;
	MYSQL_ROW   sql_row;	
	char query[100];	
	unsigned long ip_addr;
	unsigned char octec_4;
	
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// DB 접속
	/////////////////////////////////////////////////////////////////////////////////////////////////////	
	mysql_init(&set_conn);
	
	set_connection = mysql_real_connect(&set_conn, answer[2], USER, answer[3], NULL, 0, (char *)NULL, 1);
	if (set_connection == NULL){
			printf("%s\n", mysql_error(&set_conn));
			return -1;
	}
	
	sprintf(query, "select host from policy.agent%d_group where host != '0.0.0.0'", num);	
	query_stat = mysql_real_query( set_connection, query, strlen(query) );
	if( query_invalied(set_conn, query_stat) !=0){
		return -1;
	}
	
	sql_result = mysql_store_result( set_connection );	
	
	while( sql_row = mysql_fetch_row(sql_result))
	{			
		if( strcmp(sql_row[0],TOTAL_NETWORK))
		{
			ip_addr = inet_addr(sql_row[0]);
			octec_4 = ntochar(ip_addr);
			
			url_obj_group[octec_4] = 1;   // Valid policy network url check
			ip_obj_group[octec_4] = 1;   // Valid policy network ip check
			key_obj_group[octec_4] = 1;   // Valid policy network keyword check
		}
	}
	mysql_free_result(sql_result);	
	mysql_close(set_connection);
	
	return 0;
	
}

