/*
* @file: ifconfig.h
* @brief : GET ifconfig information
*/
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/if_ether.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <sys/ioctl.h>

// struct ifconfig
struct ifconfig{
	char eth[5];
	char mac[18];
	char ip[16];
	char broadcast[16];
	char netmask[16];
	int mtu;	
};

struct ifconfig ifcg;


void get_ifconfig(struct ifconfig *ifc, char *dev)
{ 
	// 이더넷 데이터 구조체  
	struct ifreq *ifr; 
  struct sockaddr_in *sin; 
  struct sockaddr *sa; 
 
  // 이더넷 설정 구조체 
  struct ifconf ifcfg; 
  int fd; 
  int n; 
  int numreqs = 30; 
  fd = socket(AF_INET, SOCK_DGRAM, 0); 
 
 
  memset(&ifcfg, 0, sizeof(ifcfg)); 
  ifcfg.ifc_buf = NULL; 
  ifcfg.ifc_len = sizeof(struct ifreq) * numreqs; 
  ifcfg.ifc_buf = malloc(ifcfg.ifc_len); 
 
  for(;;) 
  { 
  	ifcfg.ifc_len = sizeof(struct ifreq) * numreqs; 
    ifcfg.ifc_buf = realloc(ifcfg.ifc_buf, ifcfg.ifc_len); 
    if (ioctl(fd, SIOCGIFCONF, (char *)&ifcfg) < 0) 
    { 
    	perror("SIOCGIFCONF ");
    }
      
    break; 
  } 
 
  ifr = ifcfg.ifc_req; 
  for (n = 0; n < ifcfg.ifc_len; n+= sizeof(struct ifreq)) 
  { 
  	if (!strcmp(ifr->ifr_name, dev) ){
    			
   	 //IP
   	 sin = (struct sockaddr_in *)&ifr->ifr_addr; 
   	 strcpy(ifc->ip, inet_ntoa(sin->sin_addr));
    			
	   //MAC
  	 ioctl(fd, SIOCGIFHWADDR, (char *)ifr); 
     sa = &ifr->ifr_hwaddr; 
	   strcpy(ifc->mac, ether_ntoa((struct ether_addr *)sa->sa_data));
	   
	   // broadcast 
	   ioctl(fd,  SIOCGIFBRDADDR, (char *)ifr); 
	   sin = (struct sockaddr_in *)&ifr->ifr_broadaddr; 
  	 strcpy(ifc->broadcast, inet_ntoa(sin->sin_addr));
        	
	   // netmask
  	 ioctl(fd, SIOCGIFNETMASK, (char *)ifr); 
  	 sin = (struct sockaddr_in *)&ifr->ifr_addr; 
  	 strcpy(ifc->netmask, inet_ntoa(sin->sin_addr));
  	 
  	 // MTU값 
  	 ioctl(fd, SIOCGIFMTU, (char *)ifr);
  	 ifc->mtu =  ifr->ifr_mtu;
    
   	 break;
    }
    ifr++;     
  }
}


