
/*
* @ file 	: pcap.h
*	@ brief : ��Ŷ ĸ��
*/
#include <pcap.h>

#define PROMISCUOUS		1	//������ ���
#define NONPROMISCUOUS	0
#define SNAPSIZE		1024	//ĸ�� ������


#define log_dir 				"log"					// LOG ���丮
#define live_log				log_dir"/LIVE_LOG"			// �ǽð� �α� ���� ����
#define LIVE_LOG_FILE		log_dir"/LIVE_LOG_FILE"	// �α� ���� ����
#define TRAFFIC_FILE		log_dir"/TRAFFIC_FILE"	// Ʈ���� ���� ����

#define LIVE_INTERVAL 5	// �ǽð� ��û�� 5�� ���� �̷������.

#define pcap_filter	" ! arp "	// ��Ŷ ���� ��

// Ʈ���� ����ü
typedef struct size{
	int Byte;
	int KByte;
	int MByte;
	int GByte;
	int TByte;
}SIZE;

// �ǽð� �α׸� DBMS�� �����ϱ� ���� ������ ���� ����
struct argument{
  	int end_time;	// �ǽð� �Ϸ� �ð�
  	int agent_num;// ������Ʈ ��ȣ
  	char *host;		// ��� ȣ��Ʈ
  	char *user;		// ��� ����
  	char *password;	// ��� ��� ��ȣ
  	int sock;	// ���� ����
};


char* itoa(unsigned int val);	// ������ char�� ��ȯ
char * _strcat(char * src, char * dst);	// ���� strcat �Լ�
//char * _strnstr(char *big, char *little, int len);
void pcap_callback(u_char *user, const struct pcap_pkthdr *h, const u_char *packet) ;	// �ݹ� �Լ�
void Log_write(char *pbuf, char *url);	// LOG ���
void Log_live_write(char *pbuf, char *url, const struct pcap_pkthdr *h);	// ���Ͽ� LOG ���
void cal_traffic();	// Ʈ���� ���
void pcap_capture(char *dev, char *filter);	// ��Ŷ ĸ��
void live_pcap_callback(u_char *user, const struct pcap_pkthdr *h, const u_char *packet);	// �ǽð� ��Ŷ ĸ�� �ݹ� �Լ�
void live_pcap_capture(char *dev, char *filter, int agent_num, char *host, char *user, char *password, int sock);	// �ǽð� ��Ŷ ĸ��
void cleanup(void *myarg);	// ������ ���� �� ����Ǵ� �Լ� - �ǽð� ��Ŷ ĸ�� ���� �� DB�� �����Ѵ�(live_log_send �Լ� ȣ��)
int live_log_send(char *host, char *user, char *password, int agent_num);	//�ǽð� ��Ŷ ĸ�� ���� �� DB�� �����Ѵ�
void live_sort_func();	// �ǽð� LOG�� �ߺ� �����ϴ� sort�Լ�


SIZE traffic;	// Ʈ����

// �α� ���� ����
struct ether_header *ep;	// ethernet
struct ip  *iph;					// ip
struct tcphdr *tcph;			// tcp
char buf_header[128];			// packet_data
int fd=0;									// LOG ���� ��ũ����
unsigned short ether_type;
char url_host[128];				// url ���ڿ�
char *Host;								// "Host : " ã�� ������
char save_file[24];



// �ǽð� �α� ���� ����
struct ether_header *live_ep;	// ethernet
struct ip  *live_iph;					// ip
struct tcphdr *live_tcph;			// tcp
char live_buf_header[128];			// packet_data
int live_fd=0;									// LIVE_LOG ���� ��ũ����
unsigned short live_ether_type;
char live_url_host[128];				// url ���ڿ�
char *live_Host;								// "Host : " ã�� ������


uint32_t pre_ip_dst;
uint32_t pre_ip_src;
uint16_t pre_port_dst;
uint32_t pre_tcp_dst;
uint32_t pre_udp_dst;


int s;


// Ʈ���� ���
// byte ~ t-byte ���� ���
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

// �ǽð� ��Ŷ ĸ��
void live_pcap_capture(char *dev, char *filter, int agent_num, char *host, char *user, char *password, int sock)
{
	
	pcap_t *pd;
	char	ebuf[PCAP_ERRBUF_SIZE]; 	// error buffer
  struct in_addr      netmask, 		// �ݸ���ũ
                      network;		// ��Ʈ��ũ �ּ�
  struct bpf_program  fcode;  	 	// ��Ŷ���͸� ���α׷�  
  int status;
  struct argument arg;
  
  pd = pcap_open_live(dev, SNAPSIZE, PROMISCUOUS, 0, ebuf); // ����̽� ����
  if(pd == NULL) {
  	fprintf(stderr, "pcap_open_live fail: %s", ebuf);
    exit(0);
  }

  // device localnet��, netmask
  pcap_lookupnet(dev, &network.s_addr, &netmask.s_addr, ebuf);

		
	// ���͸� ��Ģ ������
	pcap_compile(pd, &fcode, filter, 0, netmask.s_addr);
  pcap_setfilter(pd, &fcode); // ��ũ���Ϳ� ���͸� ��Ģ����

  printf("[LIVE] CAPUTRE START Dev='%s'[net=%s]\n", dev, inet_ntoa(network), inet_ntoa(netmask));

	
	//������Ʈ  ���� ����
	arg.host = host;
	arg.user = user;
	arg.password = password;
	arg.agent_num = agent_num;
	arg.sock = sock;
	
	//live pcap capure �� �� �� �ð��� ���
	arg.end_time=tz.t + LIVE_INTERVAL;	
	
	// LIVE_LOG ���� ����
	live_fd = open(live_log, O_RDWR | O_CREAT);
	
	
	if(pcap_loop(pd, -1, live_pcap_callback, (void *)&arg)<0) {
		fprintf(stderr, "pcap_loop fail: %s\n", pcap_geterr(pd));
        exit(-1);
  }
    
  pcap_close(pd);	// close the packet capture discriptor
}


// �ǽð� ��Ŷ ĸ�� �ݹ� �Լ�
void live_pcap_callback(u_char *arg, const struct pcap_pkthdr *h, const u_char *packet)
{
	char * pbuf = live_buf_header;	//��� ����	
	register int i = 0;
	
	
	pthread_cleanup_push(cleanup, arg);
	// �ð��� �� �Ǹ� ������ �ױ� - ������ ��� ����
	if(tz.t >= ((struct argument *)arg)->end_time){
		pthread_exit(0);
	}
	pthread_cleanup_pop(0);
	
	
	memset(live_buf_header, 0, sizeof(live_buf_header));
	
	// �̴��� ���
	live_ep = (struct ether_header *)packet;
	
	// IP��� �������� ���� �̴��� ��� ��ŭ offset
	packet += sizeof(struct ether_header);
	
	// �̴��� ��� Ÿ��
  live_ether_type = ntohs(live_ep->ether_type);
  
	
	// @ IP protocol
	live_iph = (struct ip *)packet;
	
	// �̴���
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
    		
    		// url ������ ���
    		if((live_Host = strstr(packet+150, "Host: ")) != NULL){
    			
    			int i = 0;
    			live_Host += 6;
    			while( *live_Host != '\r' && *live_Host != 0 )
    				live_url_host[i++] = *live_Host++;    				
    			
    			url_host[i] = 0;
    			
    			// LOG ���
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




// ��Ŷ ĸ��
void pcap_capture(char *dev, char *filter)
{
	pcap_t *pd;
	char	ebuf[PCAP_ERRBUF_SIZE]; 	// error buffer
  struct in_addr      netmask, 		// �ݸ���ũ
                      network;		// ��Ʈ��ũ �ּ�
  struct bpf_program  fcode;  	 	// ��Ŷ���͸� ���α׷�  
  int status;
  
  

  pd = pcap_open_live(dev, SNAPSIZE, PROMISCUOUS, 0, ebuf); // ����̽� ����
  if(pd == NULL) {
  	fprintf(stderr, "pcap_open_live fail: %s", ebuf);
    exit(0);
  }

  // device localnet��, netmask
  pcap_lookupnet(dev, &network.s_addr, &netmask.s_addr, ebuf);

		
	// ���͸� ��Ģ ������
	pcap_compile(pd, &fcode, filter, 0, netmask.s_addr);
  pcap_setfilter(pd, &fcode); // ��ũ���Ϳ� ���͸� ��Ģ����

  printf("[PCAP] CAPUTRE START Dev='%s'[net=%s]\n", dev, inet_ntoa(network), inet_ntoa(netmask));

	// LOG ���� ���� - LOG�� : AG0913 - 9�� 13�� �α�����
	sprintf(save_file, "%s/AG%02d%02d",log_dir, tz.mday, tz.hour);	
	fd = open(save_file, O_RDWR | O_CREAT);		// ��� ���� open
	
	
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
	
	char * pbuf = buf_header;	//��� ����	
	register int i = 0;	
	
	
	if(STATUS == STOP){
		puts("[EXIT] pcap_capture");
		pthread_exit(0);
	}
	
	
	memset(buf_header, 0, sizeof(buf_header));
	
	// Ʈ���� ���
	traffic.Byte += h->len;	
	cal_traffic();
	
	
	// �̴��� ���
	ep = (struct ether_header *)packet;
	
	// IP��� �������� ���� �̴��� ��� ��ŭ offset
	packet += sizeof(struct ether_header);
	
	// �̴��� ��� Ÿ��
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
    	
    	// url ���� ���
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


// LOG ���
void Log_write(char *pbuf, char *url)
{
	
	pbuf = (char *)_strcat(pbuf, (char *)inet_ntoa(iph->ip_src));	// ����� ip
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, itoa(ntohs(tcph->source)));	// ����� port
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, (char *)inet_ntoa(iph->ip_dst));	// ������ ip
	pbuf = (char *)_strcat(pbuf, "\t");
	pbuf = (char *)_strcat(pbuf, itoa(ntohs(tcph->dest)));		// ������ port
	pbuf = (char *)_strcat(pbuf, "\t");	
	pbuf = (char *)_strcat(pbuf, url);												//url
	pbuf = (char *)_strcat(pbuf, "\t");
	
	// ��Ŷ �ð� ����
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


// �ǽð� �α� ���
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
	pbuf = (char *)_strcat(pbuf, itoa(h->len));	// ��Ŷ ũ��
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


// �ӵ� ���� strcat �Լ�
char * _strcat(char * src, char * dst)
{
	while(*src++); src--;
	while(*src++ = *dst++);	src--;
	return src;
}


/*  strstr �Լ��� �˻� ���� ���� �߰�  */
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


// �ǽð� ������ ���� �� ȣ���. - �ǽð� �α׸� DB�� ��� �ϰԵ�
void cleanup(void *arg)
{
	struct argument *info = (struct argument *)arg;	
	COMMAND cmd;
	
	
	live_log_send( info->host, info->user, info->password, info->agent_num );
	
	cmd.signal = RPL_LIVE;
	cmd.data = 0;
	
	// �۾��� ��� �������Ƿ� MASTER ���� �˸���.
	write(info->sock, &cmd, sizeof(cmd));
	printf("cleanup   sock:%d\n",info->sock);
	puts("[COMMAND] SEND : AGENT -> RECV : MASTER [RPL_LIVE]");
	return;
    
}


// �ǽð� �α׸� DB�� ���
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


// �ǽð� �α��� �ߺ��� ���� �ϴ� sort �Լ� - ����
void live_sort_func()
{
	char system_sort[64];
	sprintf(system_sort,"sort -u %s | sort -k 8 > %s", live_log, LIVE_LOG_FILE);
	system(system_sort);
	unlink(LIVE_LOG_FILE);
	return;
}


