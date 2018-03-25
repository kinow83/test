
/* fake paaket for access deny
 * date : 2008 . 8. 4
 * id   : kinow
 * doc  : 1. 3 way-handshake ���� �ܰ�
 *        2. HOST�� "GET" �޼ҵ��� �������� ��û�ϴ� PSH�� ACK �÷��װ� ���Ե� ��Ŷ�� �������� ���� ���̴�. (�������� ����...)
 *        3. �� �� AGENT�� �������� ����� GET ��û�� ���� ACK ��Ŷ�� HOST���� ���� ��, �� �ٷ�
 *        4. �������� ����� ���� �������� ������.
 *        5. �� �ʰ� ������ ���� �������� �������� ��ȿ ���� �ʰ� �ȴ�.

 */

#include "http_filter.h"
#include "pipe.h"

#include "table.h"
#include "obj.h"

#define snapsize 1024          				// capture Length
#define http_header_size snapsize-40  // Snap Length
#define fake_rule		" ! arp "					// ĸ�� �� arp ����
#define TOTAL_NETWORK	"0.0.0.0"				// ��ü ��Ʈ��ũ


int ssock;	// raw socket ���� ����

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

// url ���� ������, ip ���� ������, keyword ���� ������� �� �� ����ȴ�.
// �� ������� ����� �����ͷ� ���� ��å�� �˻��ϰ� �ȴ�.
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

char http_header[http_header_size];	// ��������

/**************************
*			���� ���μ���				*
***************************/
void *fake_page_process(char *path);



static struct packet_data 	ip_data;			//ip ���� ����ü
static struct packet_data  keyword_data;	//keyword ���� ����ü
static struct packet_data  capture_data;	//url ���� ����ü
static struct packet_data  referer_data;	//referer url ����ü

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

// unsigned long ������ unsigned char ������ ��ȯ
unsigned char ntochar(unsigned long p);
// ���� ���μ����� ���� ��Ŷ ĸ��
void f_callback(  u_char *user, const struct pcap_pkthdr *h, const u_char *packet);
// ip check sum
unsigned short ip_sum (u_short *addr, int len);

// ��Ŷ�� ���� �Լ�
void send_tcp_segment(struct iphdr *ih, struct tcphdr *th, char *data, int dlen);
// ��Ʈ�� ������ ��Ŷ ����
void assemble_port(u_int32_t my_ip, u_int32_t their_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size);
// ���� ��Ŷ ����
void assemble_ack(u_int32_t my_ip, u_int32_t their_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size);
// �����Ͱ� ������ ��Ŷ ����
void assemble_psh(u_int32_t my_ip, u_int32_t their_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size);
// ���� ��Ŷ ����
void assemble_rst(u_int32_t s_ip, u_int32_t d_ip, u_int16_t sport, u_int16_t dport, u_int32_t seq, u_int32_t ack, u_int32_t size);

// �� �������� ���Ͽ��� �о�� �޸𸮿� �ø�
int get_webpage(char *buf, char *path);
// ��å�� �޸𸮿� LOAD
int policy_load(void);
// MASTER�� ���� ��å�� ������
int get_policy(int agent_num);
// ��å ���� ������
void *apply_thread_func(void *data);
// �׷� ������Ʈ�� ����
int group_set(int num);



// url ���� ������(referer �ʵ�)
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
				
				// url �ؽ� ���̺� �ִ��� ã��
				if( no = URL_serach(buf_url, url_node) ) // find url, return key
				{
								
					// ���̽� ���̺� ����
					if( bistree_lookup(&url_base, (void **)&p_no) != -1) //exist in base
					{
						puts("URL���̽� ���̺� ����");
						host = ntochar(capture_data.saddr); // octec 4th
									
						// �׷� ������Ʈ �迭�� source ip�� �ش��ϴ� �ε����� ��å�� �����Ǿ����� �˻�
						if( url_obj_group[ host ] == 1 ) //apply group
						{
							// url ��Ʈ��ũ ������Ʈ �˻� - Ʈ�� �˻�
							if (bistree_lookup( &(url_obj[host].tree), (void **)&p_no ) != -1) // exist in object
							{ 	
								//DENY
								puts("URL���̽� ���̺� �ְ� �׷쿡 �����ϰ� ȣ��Ʈ�� �ְ� ���ܵ�");
								assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
			    			assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
							
							// ������Ʈ���� ���� �ȵ�
							}
							else
							{
								puts("URL���̽� ���̺� �ְ� �׷쿡 �����ϰ� ȣ��Ʈ�� ���� ���� �ȵ�");
								break;
							}
						}
						// ������Ʈ �׷쿡 �������� ����.
						else	// empty group
						{	
							// DENY
							puts("URL���̽� ���̺� �ְ� �׷쿡 ���� ���ܵ�");
							assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
			    		assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
						}
					}
								
					// ���̽� ���̺� ���� ����
					else // Didn't exist base
					{									
						host = ntochar(capture_data.saddr); // octec 4th
									
						// �׷� ������Ʈ �迭�� source ip�� �ش��ϴ� �ε����� ��å�� �����Ǿ����� �˻�
						if( url_obj_group[ host ] == 1 ) //apply group
						{
							// url ��Ʈ��ũ ������Ʈ �˻� - Ʈ�� �˻�
							if (bistree_lookup( &(url_obj[host].tree), (void **)&p_no ) != -1) // exist in object
							{	
								puts("URL���̽� ���̺� ���� �׷쿡 �ְ� ȣ��Ʈ�� �־� ���ܵ�");
								assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
			    			assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
							}
							else
							{
								puts("URL���̽� ���̺� ���� �׷쿡 �ְ� ȣ��Ʈ�� ����  ���� �ȵ�");
								break;
							}
						}
									
						// ������Ʈ �׷쿡 �������� ����
						else // empty group
						{
							puts("URL���̽� ���̺� ���� �׷쿡 ���� ���� �ȵ�");
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



// ip ���� ������
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
			
			// ip ���̺� �ִ��� ã��
			if( no = IP_serach(send_data.daddr, ip_node) ) // find ip, return key
			{
				puts("IP ���̺� �ִ��� ã��");
				
				// ���̽� ���̺� ���� ����
				//exist in base
				if( bistree_lookup(&ip_base, (void **)&p_no) != -1)
				{
					// octec 4th
					host = ntochar(send_data.saddr);
					
					// �׷� ������Ʈ �迭�� source ip�� �ش��ϴ� �ε����� ��å�� �����Ǿ����� �˻�
					if( ip_obj_group[ host ] == 1 )
					{
						// ip ��Ʈ��ũ ������Ʈ �˻� - Ʈ�� �˻�
						if (bistree_lookup( &(ip_obj[host].tree), (void **)&p_no ) != -1) // exist in object
						{ 
							//DENY
							puts("IP ���̽��� �ְ� �׷쿡 �ְ� ȣ��Ʈ�� �־ ���� ��");
							assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
			    		assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);			    		
						}
						else
						{
							puts("IP ���̽��� �ְ�  �׷쿡 �ְ� ȣ��Ʈ�� ��� ���� �ȵ�");
							goto IP_OUT;
						}
					}
					
					// empty group
					else
					{	
						// DENY
						puts("IP ���̽��� �ְ� �׷��� ���� ��");						
						assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
    				assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);    				
					}
				}
				
				// Didn't exist base
				else 
				{
					// octec 4th
					host = ntochar(send_data.saddr);
					
					// �׷� ������Ʈ �迭�� source ip�� �ش��ϴ� �ε����� ��å�� �����Ǿ����� �˻�
					if( ip_obj_group[ host ] == 1 )
					{
						// ip ��Ʈ��ũ ������Ʈ �˻� - Ʈ�� �˻�
						if (bistree_lookup( &(ip_obj[host].tree), (void **)&p_no ) != -1) // exist in object
						{	
							//DENY
							puts("IP ���̽��� ���� �׷쿡 �ְ� ȣ��Ʈ�� �־ ���� ��");
							assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
	    				assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);	    				
						}
						else
						{
							puts("IP ���̽��� ���� �׷쿡 �ְ� ȣ��Ʈ�� ��� ���� �ȵ�");
							goto IP_OUT;
						}
					}
					
					// empty group
					else
					{
						puts("IP ���̽��� ���� �׷��� ���� �ȵ�");
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



// Ű���� ���� ������
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
				
				// Ű���� ���̺� Ű����� ��ġ�ϴ� ���� ã��
				// exist in key table
				if ( no = KEY_serach(send_data.data, key_node) )
				{					
					// ���̽� ���̺� ���� ����
					//exist in base
					if( bistree_lookup(&key_base, (void **)&p_no) != -1)
					{						
						host = ntochar(send_data.saddr); // octec 4th
						
						// �׷� ������Ʈ �迭�� source ip�� �ش��ϴ� �ε����� ��å�� �����Ǿ����� �˻�
						if( key_obj_group[ host ] == 1 ) //apply group
						{
							// Ű���� ��Ʈ��ũ ������Ʈ �˻� - Ʈ�� �˻�
							if (bistree_lookup( &(key_obj[host].tree), (void **)&p_no ) != -1) // exist in object
							{ 	
								//DENY
								puts("KEY���̽��� �ְ� �׷쿡 �ְ� ȣ��Ʈ�� �־ ����");
								assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
  				  		assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
    						assemble_rst(send_data.saddr, send_data.daddr, send_data.source, send_data.dest, send_data.seq+1, send_data.ack_seq, 0);
							}
							// ȣ��Ʈ�� ���� ���� ����
							else
							{
								puts("KEY���̽��� �ְ� �׷쿡 �ְ� ȣ��Ʈ�� ��� ���� �ȵ�");
								goto KEY_OUT;
							}
						}
						// ������Ʈ �׷� ���̺� ���� ���� ����
						else	// empty group
						{	
							// DENY							
							puts("KEY���̽��� �ְ� �׷쿡 ��� ����");
							assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
	  				  assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
							assemble_rst(send_data.saddr, send_data.daddr, send_data.source, send_data.dest, send_data.seq+1, send_data.ack_seq, 0);
						}
					}
					
					// ���̽� ���̺� ���� ���� ����
					else // Didn't exist base
					{
						// octec 4th
						host = ntochar(send_data.saddr);
					
						// �׷� ������Ʈ �迭�� source ip�� �ش��ϴ� �ε����� ��å�� �����Ǿ����� �˻�
						if( key_obj_group[ host ] == 1 ) //apply group
						{
							// Ű���� ��Ʈ��ũ ������Ʈ �˻� - Ʈ�� �˻�
							// exist in object
							if (bistree_lookup( &(key_obj[host].tree), (void **)&p_no ) != -1)
							{	
								//DENY								
								puts("KEY���̽��� ���� �׷쿡 �ְ� ȣ��Ʈ�� �־ ����");
								assemble_ack(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), 0);
		  					assemble_psh(send_data.daddr, send_data.saddr, send_data.dest, send_data.source, send_data.ack_seq, ntohl(send_data.seq) + send_data.packet_len-(40+14), send_data.packet_len-40);
								assemble_rst(send_data.saddr, send_data.daddr, send_data.source, send_data.dest, send_data.seq+1, send_data.ack_seq, 0);
							}
							// ȣ��Ʈ�� ���� ���� ����
							else
							{
								puts("KEY���̽��� ���� �׷쿡 �ְ� ȣ��Ʈ ��� ���� �ȵ�");
								goto KEY_OUT;
							}
						}
						
						// ������Ʈ �׷� ���̺� ���� ���� ����
						else // empty group
						{
							puts("KEY���̽��� ���� �׷쿡 ��� ���� �ȵ�");
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
 * pcap callback(���� ���μ����� ���� ��Ŷ ĸ��)
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
    
    	// IP ����� ����
    	iphlen = iph->ip_hl*4;

    	// �̴��� ������ ���+ IP ����� ������ TCP �����
    	tcph = (struct tcphdr *) (packet + sizeof(struct ethhdr) + iphlen) ;    
    	http = (u_char *)packet + ethhdr_len + iphlen + sizeof(struct tcphdr);    
    	type = type_request((char *)http);
    	
    
			// 3-way handshaking ���Ŀ� ��Ŷ�� �ٷ�.
			if( tcph->psh == 1 && tcph->ack ==1 ){
				
				// �� ������� ��Ŷ ����
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
				// PORT ����				
				if(tcph->dest == htons(1033)){					
					assemble_ack(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), 0);
  	  		assemble_port(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), packet_len-40);
  	  		assemble_rst(iphiph->saddr, iphiph->daddr, tcph->source, tcph->dest, tcph->seq+1, tcph->ack_seq, 0);
				}			
				***********************************************************************************************************************************************/
			
				switch(type){
			
					case IS_GET :
						///////////////////////////////////////////////////////////////////
						//        150 <- �� ���� �����Ͽ� ���ܷ��� ���� �� �ִ�.
						//        Host �ʵ���� ����
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
							
							// url �ؽ� ���̺� �ִ��� ã��
							if( no = URL_serach(buf_url, url_node) ) // find url, return key
							{								
								// ���̽� ���̺� ����
								if( bistree_lookup(&url_base, (void **)&p_no) != -1) //exist in base
								{
									puts("URL���̽� ���̺� ����");
									host = ntochar(capture_data.saddr); // octec 4th
									
									// �׷� ������Ʈ �迭�� source ip�� �ش��ϴ� �ε����� ��å�� �����Ǿ����� �˻�
									if( url_obj_group[ host ] == 1 )
									{																				
										// url ��Ʈ��ũ ������Ʈ �˻� - Ʈ�� �˻�
										// exist in object
										if (bistree_lookup( &(url_obj[host].tree), (void **)&p_no ) != -1) // exist in object
										{ 	
											//DENY
											puts("URL���̽� ���̺� �ְ� �׷쿡 �����ϰ� ȣ��Ʈ�� �ְ� ���ܵ�");
											assemble_ack(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), 0);
											assemble_psh(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), packet_len-40);
										}
										// ������Ʈ���� ���� �ȵ�
										else
										{
											puts("URL���̽� ���̺� �ְ� �׷쿡 �����ϰ� ȣ��Ʈ�� ���� ���� �ȵ�");
											break;
										}
									}
									// ������Ʈ �׷쿡 �������� ����.
									else	// empty group
									{	
										// DENY
										puts("URL���̽� ���̺� �ְ� �׷쿡 ���� ���ܵ�");
										assemble_ack(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), 0);
										assemble_psh(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), packet_len-40);
									}
								}
								
								// ���̽� ���̺� ���� ����
								else // Didn't exist base
								{									
									host = ntochar(capture_data.saddr); // octec 4th
									
									// �׷� ������Ʈ �迭�� source ip�� �ش��ϴ� �ε����� ��å�� �����Ǿ����� �˻�
									if( url_obj_group[ host ] == 1 ) //apply group
									{
										if (bistree_lookup( &(url_obj[host].tree), (void **)&p_no ) != -1) // exist in object
										{	
											puts("URL���̽� ���̺� ���� �׷쿡 �ְ� ȣ��Ʈ�� �־� ���ܵ�");
											assemble_ack(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), 0);
											assemble_psh(iphiph->daddr, iphiph->saddr, tcph->dest, tcph->source, tcph->ack_seq, ntohl(tcph->seq) + packet_len-(40+14), packet_len-40);
										}
										else
										{
											puts("URL���̽� ���̺� ���� �׷쿡 �ְ� ȣ��Ʈ�� ����  ���� �ȵ�");
											break;
										}
									}
									
									// ������Ʈ �׷쿡 �������� ����
									else // empty group
									{
										puts("URL���̽� ���̺� ���� �׷쿡 ���� ���� �ȵ�");
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




// ���� ���μ��� ���� �Լ�
void *fake_page_process(char *path)
{
	
		char cmd[128];
	
    pcap_t  *pd;                     // pcap  ��ũ����
    char     ebuf[PCAP_ERRBUF_SIZE], // error buffer
            *dev;                    // device name
    
    struct in_addr      netmask,     // �ݸ���ũ
                        network;     // ��Ʈ��ũ �ּ�
    struct bpf_program  fcode;       // ��Ŷ���͸� ���α׷�
    
    pthread_t ip_filter_thread;				// ip ���� ������
    pthread_t keyword_filter_thread;	// Ű���� ���� ������
    pthread_t apply_thread;						// ��å ���� ������
    pthread_t referer_filter_thread;	// url-referer ������
    
    int status;
    
    // ��å�� ����� ���� ����        
    sprintf(cmd,"touch %s",URL_PATH);    			system(cmd);
    sprintf(cmd,"touch %s",KEYWORD_PATH);    	system(cmd);
    sprintf(cmd,"touch %s",IP_PATH);			    system(cmd);
    
    sprintf(cmd,"touch %s",URL_OBJ_PATH);    	system(cmd);
    sprintf(cmd,"touch %s",KEYWORD_OBJ_PATH); system(cmd);
    sprintf(cmd,"touch %s",IP_OBJ_PATH);	    system(cmd);
    
    sprintf(cmd,"touch %s",URL_BASE_PATH);    	system(cmd);
    sprintf(cmd,"touch %s",KEYWORD_BASE_PATH); 	system(cmd);
    sprintf(cmd,"touch %s",IP_BASE_PATH);    		system(cmd);
    
    
    // block page �ҷ�����
    get_webpage(html, path);
    
    // ��å�� �޸𸮿� Load
    if( policy_load() != 0 ){
    	puts("[FAKE] hashtable unload");
    	exit(1);
    }
    
    // ��� ������ ���� - ����
    OPERATION = O_START;
    
    // ��å �ݿ� ������
    if(( status = pthread_create( &apply_thread, NULL, &apply_thread_func, NULL)) != 0 ) {
			printf("apply_thread error : %s\n", strerror(status));			
		}
		
		// Ű���� ���� ������
		if(( status = pthread_create( &keyword_filter_thread, NULL, &keyword_filter_thread_func, NULL)) != 0 ) {
			printf("keyword_filter_thread error : %s\n", strerror(status));			
		}
		
		// ip ���� ������
		if(( status = pthread_create( &ip_filter_thread, NULL, &ip_filter_thread_func, NULL)) != 0 ) {
			printf("ip_filter_thread error : %s\n", strerror(status));			
		}
		
		// url(referer)
		if(( status = pthread_create( &referer_filter_thread, NULL, &referer_filter_thread_func, NULL)) != 0 ) {
			printf("referer_filter_thread error : %s\n", strerror(status));			
		}
		
		    
    pd=pcap_open_live(answer[1],snapsize,1,1000,ebuf); // ����̽� ����
    if(pd == NULL) {
        fprintf(stderr, "pcap_open_live fail: %s", ebuf);        
    }
    

    // ����̽��� �ش��ϴ� localnet�� netmask�� ����
    pcap_lookupnet(answer[1], &network.s_addr, &netmask.s_addr, ebuf);

    // ���͸� ��Ģ ������    
    pcap_compile(pd, &fcode, fake_rule,0, netmask.s_addr);

    pcap_setfilter(pd, &fcode); // ��ũ���Ϳ� ���͸� ��Ģ����

    printf("[FAKE] CAPUTRE START Dev='%s'[net=%s]\n", answer[1], inet_ntoa(network));

    // f_callback : url ���� ��ƾ
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
		
		// pipeline[1] : fake�� �޴� ������
		// pipeline[0] : fake�� ������ ������
		if( read(pipeline[0], &ipc_buf, sizeof(ipc_buf)) != -1)
		{			
			if(ipc_buf == PLY_CHN){
				
				// �۵� ���� �ؽ� ���̺� �ٽ� �ε�
				OPERATION = O_STOP;
				
				// ��å �ٿ�ε�				
				if (get_policy(agent_num) != 0){
					puts("[POLICY] download policy error");
				}
				puts("[POLICY] download policy");
				
				
				// ��Ʈ��ũ ��ü �ٿ�ε�
				if(get_netobj(agent_num) != 0){
					puts("[POLICY] download network object error");
				}
				puts("[POLICY] download network object");
				
				// �ؽ� ���̺� �� ���
				if( policy_load() != 0 ){
		    			puts("[FAKE] hashtable Reload error");    			
    				}
		    	puts("[FAKE] hashtable Reload");
    		
    			// �۵� �����
		    	OPERATION = O_START;
    		
    			// ��å ���� �Ϸ�
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



// ��å�� �޸𸮿� Load
int policy_load()
{	
	
	// ��å ���Ͽ��� �ؽ� ���̺� ����
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
  
  //������Ʈ �׷� set
  group_set(agent_num);

  return 0;	
}



// Master(DBMS)�� url, ip, keyword ������ �����´� - �ؽ� ���̺� ����� .
int get_policy(int agent_num)
{
	
	MYSQL       *ply_connection=NULL, ply_conn;
	int query_stat;	
	char query[450];
	char buf[300];
	
	MYSQL_RES   *sql_result;
	MYSQL_ROW   sql_row;
	
	FILE * url_fp, * keyword_fp, * ip_fp;
	
	// ���� ���� ���� ����
	sprintf(buf,"rm -fr %s",URL_PATH);
	system(buf);
	sprintf(buf,"rm -fr %s",IP_PATH);
	system(buf);
	sprintf(buf,"rm -fr %s",KEYWORD_PATH);
	system(buf);
	
	
	puts(URL_PATH);
	puts(IP_PATH);
	puts(KEYWORD_PATH);
	
	// ���ο� ���� ���� ����
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
	
	
	// ��� ����
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
	// URL ��������
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
	// IP ��������
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
	// KEYWORD ��������
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


// Master(DBMS)�� ���� ��å�� �����´� - ��Ʈ��ũ ������Ʈ ����� .
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
	
	// ���� ��å(��Ʈ��ũ ������Ʈ) ���� ����
	unlink(URL_OBJ_PATH);
	unlink(IP_OBJ_PATH);
	unlink(KEYWORD_OBJ_PATH);
	
	// ���� ��å(�⺻ ������Ʈ) ���� ����
	unlink(URL_BASE_PATH);
	unlink(IP_BASE_PATH);
	unlink(KEYWORD_BASE_PATH);
	
	// ���ο� ��å(��Ʈ��ũ ������Ʈ) ���� ����
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
	
	// ���ο� ��å(�⺻ ������Ʈ) ���� ����
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
	// DB ����
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
	// URL ��Ʈ��ũ ��ü ��������
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
		// ��Ʈ��ũ ��ü ����(0.0.0.0) �̸� base ���Ͽ� ��Ʈ��ũ ��ü ���� -? �޸� ��� ȿ��
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
	// IP ��Ʈ��ũ ��ü ��������
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
		// ��Ʈ��ũ ��ü ����(0.0.0.0) �̸� base ���Ͽ� ��Ʈ��ũ ��ü ����
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
	// KEYWORD ��Ʈ��ũ ��ü ��������
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
		// ��Ʈ��ũ ��ü ����(0.0.0.0) �̸� base ���Ͽ� ��Ʈ��ũ ��ü ����
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


// unsigned long ������ unsigned char �� ��ȯ
unsigned char ntochar(unsigned long p)
{
	u_char * index = (u_char *)&p;  // last octec's ipaddress 1 byte pointer
	u_char ret = *(index+3);        // last octec's ipaddress is hash index	
	return ret;
}



// �׷� ������Ʈ Ÿ�� �Ǵ� �� 
// �׷� ������Ʈ �迭�� ���� ��å�� �����ϹǷ� TRUE(1)�� ���� ���ش�.
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
	// DB ����
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

